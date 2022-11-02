#include "framework.h"
#include "Game/Lara/lara_helpers.h"

#include "Flow/ScriptInterfaceFlowHandler.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/control/volume.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_tests.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#include "Objects/TR2/Vehicles/skidoo.h"
#include "Objects/TR3/Vehicles/big_gun.h"
#include "Objects/TR3/Vehicles/kayak.h"
#include "Objects/TR3/Vehicles/minecart.h"
#include "Objects/TR3/Vehicles/quad_bike.h"
#include "Objects/TR3/Vehicles/upv.h"
#include "Objects/TR4/Vehicles/jeep.h"
#include "Objects/TR4/Vehicles/motorbike.h"

using namespace TEN::Control::Volumes;
using namespace TEN::Input;
using namespace TEN::Math;
using namespace TEN::Renderer;

// -----------------------------
// HELPER FUNCTIONS
// For State Control & Collision
// -----------------------------

void HandleLaraMovementParameters(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	// Update AFK pose timer.
	if (lara->Control.Count.Pose < LARA_POSE_TIME && TestLaraPose(item, coll) &&
		!(TrInput & IN_LOOK || IsOpticActionHeld()) &&
		g_GameFlow->HasAFKPose())
	{
		lara->Control.Count.Pose++;
	}
	else
		lara->Control.Count.Pose = 0;

	// Reset running jump timer.
	if (!IsRunJumpCountableState((LaraState)item->Animation.ActiveState))
		lara->Control.Count.Run = 0;

	// Reset running jump action queue.
	if (!IsRunJumpQueueableState((LaraState)item->Animation.ActiveState))
		lara->Control.RunJumpQueued = false;

	// Reset lean.
	if ((!lara->Control.IsMoving || (lara->Control.IsMoving && !(TrInput & (IN_LEFT | IN_RIGHT)))) &&
		(!lara->Control.IsLow && item->Animation.ActiveState != LS_DEATH)) // HACK: Don't interfere with surface alignment in crouch, crawl, and death states.
	{
		ResetLaraLean(item, 6.0f);
	}

	// Reset crawl flex.
	if (!(TrInput & IN_LOOK) && coll->Setup.Height > LARA_HEIGHT - LARA_HEADROOM &&	// HACK
		(!item->Animation.Velocity.z || (item->Animation.Velocity.z && !(TrInput & (IN_LEFT | IN_RIGHT)))))
	{
		ResetLaraFlex(item, 12.0f);
	}

	// Apply and reset turn rate.
	item->Pose.Orientation.y += lara->Control.TurnRate;
	if (!(TrInput & (IN_LEFT | IN_RIGHT)))
		lara->Control.TurnRate = 0;

	lara->Control.IsLow = false;
}

bool HandleLaraVehicle(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Vehicle == NO_ITEM)
		return false;

	TestVolumes(lara->Vehicle);

	switch (g_Level.Items[lara->Vehicle].ObjectNumber)
	{
	case ID_QUAD:
		QuadBikeControl(item, coll);
		break;

	case ID_JEEP:
		JeepControl(item);
		break;

	case ID_MOTORBIKE:
		MotorbikeControl(item, coll);
		break;

	case ID_KAYAK:
		KayakControl(item);
		break;

	case ID_SNOWMOBILE:
		SkidooControl(item, coll);
		break;

	case ID_UPV:
		UPVControl(item, coll);
		break;

	case ID_MINECART:
		MinecartControl(item);
		break;

	case ID_BIGGUN:
		BigGunControl(item, coll);
		break;

		// Boats are processed like normal items in loop.
	default:
		LaraGun(item);
	}

	return true;
}

// TODO: This approach may cause undesirable artefacts where a platform rapidly ascends/descends or the player gets pushed.
// Potential solutions:
// 1. Consider floor tilt when translating objects.
// 2. Object parenting. -- Sezz 2022.10.28
void EaseOutLaraHeight(ItemInfo* item, int height)
{
	static constexpr int   rate				 = 50;
	static constexpr float easingAlpha		 = 0.35f;
	static constexpr int   constantThreshold = STEPUP_HEIGHT / 2;

	// Check for walls.
	if (height == NO_HEIGHT)
		return;

	// Swamp case.
	if (TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		item->Pose.Position.y += (height > 0) ? SWAMP_GRAVITY : height;
		return;
	}

	int easingThreshold = std::max(abs(item->Animation.Velocity.z) * 1.5f, BLOCK(1.0f / 64));

	// Regular case.
	if (abs(height) > constantThreshold)
	{
		int sign = std::copysign(1, height);
		item->Pose.Position.y += rate * sign;
	}
	else if (abs(height) > easingThreshold)
	{
		int vPos = item->Pose.Position.y;
		item->Pose.Position.y = (int)round(Lerp(vPos, vPos + height, easingAlpha));
	}
	else
		item->Pose.Position.y += height;
}

// TODO: Some states can't make the most of this function due to missing step up/down animations.
// Try implementing leg IK as a substitute to make step animations obsolete. @Sezz 2021.10.09
void DoLaraStep(ItemInfo* item, CollisionInfo* coll)
{
	if (!TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		if (TestLaraStepUp(item, coll))
		{
			item->Animation.TargetState = LS_STEP_UP;
			if (GetStateDispatch(item, g_Level.Anims[item->Animation.AnimNumber]))
			{
				item->Pose.Position.y += coll->Middle.Floor;
				return;
			}
		}
		else if (TestLaraStepDown(item, coll))
		{
			item->Animation.TargetState = LS_STEP_DOWN;
			if (GetStateDispatch(item, g_Level.Anims[item->Animation.AnimNumber]))
			{
				item->Pose.Position.y += coll->Middle.Floor;
				return;
			}
		}
	}

	EaseOutLaraHeight(item, coll->Middle.Floor);
}

void DoLaraMonkeyStep(ItemInfo* item, CollisionInfo* coll)
{
	EaseOutLaraHeight(item, coll->Middle.Ceiling);
}

void DoLaraCrawlToHangSnap(ItemInfo* item, CollisionInfo* coll)
{
	coll->Setup.ForwardAngle = item->Pose.Orientation.y + ANGLE(180.0f);
	GetCollisionInfo(coll, item);

	SnapItemToLedge(item, coll);
	LaraResetGravityStatus(item, coll);

	// Bridges behave differently.
	if (coll->Middle.Bridge < 0)
	{
		TranslateItem(item, item->Pose.Orientation.y, -LARA_RADIUS_CRAWL);
		item->Pose.Orientation.y += ANGLE(180.0f);
	}
}

void DoLaraTightropeBalance(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);
	const int factor = ((lara->Control.Tightrope.TimeOnTightrope >> 7) & 0xFF) * 128;

	if (TrInput & IN_LEFT)
		lara->Control.Tightrope.Balance += ANGLE(1.4f);
	if (TrInput & IN_RIGHT)
		lara->Control.Tightrope.Balance -= ANGLE(1.4f);

	if (lara->Control.Tightrope.Balance < 0)
	{
		lara->Control.Tightrope.Balance -= factor;
		if (lara->Control.Tightrope.Balance <= -ANGLE(45.0f))
			lara->Control.Tightrope.Balance = -ANGLE(45.0f);

	}
	else if (lara->Control.Tightrope.Balance > 0)
	{
		lara->Control.Tightrope.Balance += factor;
		if (lara->Control.Tightrope.Balance >= ANGLE(45.0f))
			lara->Control.Tightrope.Balance = ANGLE(45.0f);
	}
	else
		lara->Control.Tightrope.Balance = GetRandomControl() & 1 ? -1 : 1;
}

void DoLaraTightropeLean(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	item->Pose.Orientation.z = lara->Control.Tightrope.Balance / 4;
	lara->ExtraTorsoRot.z = -lara->Control.Tightrope.Balance;
}

void DoLaraTightropeBalanceRegen(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.Tightrope.TimeOnTightrope <= 32)
		lara->Control.Tightrope.TimeOnTightrope = 0;
	else
		lara->Control.Tightrope.TimeOnTightrope -= 32;

	if (lara->Control.Tightrope.Balance > 0)
	{
		if (lara->Control.Tightrope.Balance <= ANGLE(0.75f))
			lara->Control.Tightrope.Balance = 0;
		else
			lara->Control.Tightrope.Balance -= ANGLE(0.75f);
	}

	if (lara->Control.Tightrope.Balance < 0)
	{
		if (lara->Control.Tightrope.Balance >= -ANGLE(0.75f))
			lara->Control.Tightrope.Balance = 0;
		else
			lara->Control.Tightrope.Balance += ANGLE(0.75f);
	}
}

void DoLaraFallDamage(ItemInfo* item)
{
	if (item->Animation.Velocity.y >= LARA_DAMAGE_VELOCITY)
	{
		if (item->Animation.Velocity.y >= LARA_DEATH_VELOCITY)
			item->HitPoints = 0;
		else USE_FEATURE_IF_CPP20([[likely]])
		{
			float base = item->Animation.Velocity.y - (LARA_DAMAGE_VELOCITY - 1);
			item->HitPoints -= LARA_HEALTH_MAX * (pow(base, 2) / 196);
		}

		float rumblePower = ((float)item->Animation.Velocity.y / (float)LARA_DEATH_VELOCITY) * 0.7f;
		Rumble(rumblePower, 0.3f);
	}
}

LaraInfo*& GetLaraInfo(ItemInfo* item)
{
	if (item->ObjectNumber == ID_LARA)
		return (LaraInfo*&)item->Data;
	else
	{
		TENLog(std::string("Attempted to fetch LaraInfo data from entity with object ID ") + std::to_string(item->ObjectNumber), LogLevel::Warning);

		auto* firstLaraItem = FindItem(ID_LARA);
		return (LaraInfo*&)firstLaraItem->Data;
	}
}

short GetLaraSlideDirection(ItemInfo* item, CollisionInfo* coll)
{
	short headingAngle = coll->Setup.ForwardAngle;
	auto probe = GetCollision(item);

	// Ground is flat.
	if (probe.FloorTilt == Vector2::Zero)
		return headingAngle;

	// Get either:
	// a) the surface aspect angle (extended slides), or
	// b) the derived nearest cardinal direction from it (original slides).
	headingAngle = Geometry::GetSurfaceAspectAngle(probe.FloorTilt);
	if (g_GameFlow->HasSlideExtended())
		return headingAngle;
	else
		return (GetQuadrant(headingAngle) * ANGLE(90.0f));
}

short ModulateLaraTurnRate(short turnRate, short accelRate, short minTurnRate, short maxTurnRate, float axisCoeff, bool invert)
{
	axisCoeff *= invert ? -1 : 1;
	int sign = std::copysign(1, axisCoeff);

	short minTurnRateNorm = minTurnRate * abs(axisCoeff);
	short maxTurnRateNorm = maxTurnRate * abs(axisCoeff);

	short newTurnRate = (turnRate + (accelRate * sign)) * sign;
	newTurnRate = std::clamp(newTurnRate, minTurnRateNorm, maxTurnRateNorm);
	return (newTurnRate * sign);
}

// TODO: Make these two functions methods of LaraInfo someday. -- Sezz 2022.06.26
void ModulateLaraTurnRateX(ItemInfo* item, short accelRate, short minTurnRate, short maxTurnRate, bool invert)
{
	auto* lara = GetLaraInfo(item);

	//lara->Control.TurnRate.x = ModulateLaraTurnRate(lara->Control.TurnRate.x, accelRate, minTurnRate, maxTurnRate, AxisMap[InputAxis::MoveVertical], invert);
}

void ModulateLaraTurnRateY(ItemInfo* item, short accelRate, short minTurnRate, short maxTurnRate, bool invert)
{
	auto* lara = GetLaraInfo(item);

	float axisCoeff = AxisMap[InputAxis::MoveHorizontal];
	if (item->Animation.IsAirborne)
	{
		int sign = std::copysign(1, axisCoeff);
		axisCoeff = std::min(1.2f, abs(axisCoeff)) * sign;
	}

	lara->Control.TurnRate/*.y*/ = ModulateLaraTurnRate(lara->Control.TurnRate/*.y*/, accelRate, minTurnRate, maxTurnRate, axisCoeff, invert);
}

void ModulateLaraSwimTurnRates(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	/*if (TrInput & (IN_FORWARD | IN_BACK))
		ModulateLaraTurnRateX(item, 0, 0, 0);*/

	if (TrInput & IN_FORWARD)
		item->Pose.Orientation.x -= ANGLE(3.0f);
	else if (TrInput & IN_BACK)
		item->Pose.Orientation.x += ANGLE(3.0f);

	if (TrInput & (IN_LEFT | IN_RIGHT))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_MED_TURN_RATE_MAX);

		// TODO: ModulateLaraLean() doesn't really work here. -- Sezz 2022.06.22
		if (TrInput & IN_LEFT)
			item->Pose.Orientation.z -= LARA_LEAN_RATE;
		else if (TrInput & IN_RIGHT)
			item->Pose.Orientation.z += LARA_LEAN_RATE;
	}
}

void ModulateLaraSubsuitSwimTurnRates(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_FORWARD && item->Pose.Orientation.x > -ANGLE(85.0f))
		lara->Control.Subsuit.DXRot = -ANGLE(45.0f);
	else if (TrInput & IN_BACK && item->Pose.Orientation.x < ANGLE(85.0f))
		lara->Control.Subsuit.DXRot = ANGLE(45.0f);
	else
		lara->Control.Subsuit.DXRot = 0;

	if (TrInput & (IN_LEFT | IN_RIGHT))
	{
		ModulateLaraTurnRateY(item, LARA_SUBSUIT_TURN_RATE_ACCEL, 0, LARA_MED_TURN_RATE_MAX);

		if (TrInput & IN_LEFT)
			item->Pose.Orientation.z -= LARA_LEAN_RATE;
		else if (TrInput & IN_RIGHT)
			item->Pose.Orientation.z += LARA_LEAN_RATE;
	}
}

// TODO: Simplify this function. Some members of SubsuitControlData may be unnecessary. -- Sezz 2022.06.22
void UpdateLaraSubsuitAngles(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.Subsuit.VerticalVelocity != 0)
	{
		item->Pose.Position.y += lara->Control.Subsuit.VerticalVelocity / 4;
		lara->Control.Subsuit.VerticalVelocity = ceil((15 / 16) * lara->Control.Subsuit.VerticalVelocity - 1);
	}

	lara->Control.Subsuit.Velocity[0] = -4 * item->Animation.Velocity.y;
	lara->Control.Subsuit.Velocity[1] = -4 * item->Animation.Velocity.y;

	if (lara->Control.Subsuit.XRot >= lara->Control.Subsuit.DXRot)
	{
		if (lara->Control.Subsuit.XRot > lara->Control.Subsuit.DXRot)
		{
			if (lara->Control.Subsuit.XRot > 0 && lara->Control.Subsuit.DXRot < 0)
				lara->Control.Subsuit.XRot = ceil(0.75 * lara->Control.Subsuit.XRot);

			lara->Control.Subsuit.XRot -= ANGLE(2.0f);
			if (lara->Control.Subsuit.XRot < lara->Control.Subsuit.DXRot)
				lara->Control.Subsuit.XRot = lara->Control.Subsuit.DXRot;
		}
	}
	else
	{
		if (lara->Control.Subsuit.XRot < 0 && lara->Control.Subsuit.DXRot > 0)
			lara->Control.Subsuit.XRot = ceil(0.75 * lara->Control.Subsuit.XRot);

		lara->Control.Subsuit.XRot += ANGLE(2.0f);
		if (lara->Control.Subsuit.XRot > lara->Control.Subsuit.DXRot)
			lara->Control.Subsuit.XRot = lara->Control.Subsuit.DXRot;
	}

	if (lara->Control.Subsuit.DXRot != 0)
	{
		short rotation = lara->Control.Subsuit.DXRot >> 3;
		if (rotation < -ANGLE(2.0f))
			rotation = -ANGLE(2.0f);
		else if (rotation > ANGLE(2.0f))
			rotation = ANGLE(2.0f);

		item->Pose.Orientation.x += rotation;
	}

	lara->Control.Subsuit.Velocity[0] += abs(lara->Control.Subsuit.XRot >> 3);
	lara->Control.Subsuit.Velocity[1] += abs(lara->Control.Subsuit.XRot >> 3);

	if (lara->Control.TurnRate > 0)
		lara->Control.Subsuit.Velocity[0] += 2 * abs(lara->Control.TurnRate);
	else if (lara->Control.TurnRate < 0)
		lara->Control.Subsuit.Velocity[1] += 2 * abs(lara->Control.TurnRate);

	if (lara->Control.Subsuit.Velocity[0] > SECTOR(1.5f))
		lara->Control.Subsuit.Velocity[0] = SECTOR(1.5f);

	if (lara->Control.Subsuit.Velocity[1] > SECTOR(1.5f))
		lara->Control.Subsuit.Velocity[1] = SECTOR(1.5f);

	if (lara->Control.Subsuit.Velocity[0] != 0 || lara->Control.Subsuit.Velocity[1] != 0)
	{
		auto mul1 = (float)abs(lara->Control.Subsuit.Velocity[0]) / SECTOR(8);
		auto mul2 = (float)abs(lara->Control.Subsuit.Velocity[1]) / SECTOR(8);
		auto vol = ((mul1 + mul2) * 5.0f) + 0.5f;
		SoundEffect(SFX_TR5_VEHICLE_DIVESUIT_ENGINE, &item->Pose, SoundEnvironment::Water, 1.0f + (mul1 + mul2), vol);
	}
}

void ModulateLaraLean(ItemInfo* item, CollisionInfo* coll, short baseRate, short maxAngle)
{
	if (!item->Animation.Velocity.z)
		return;

	float axisCoeff = AxisMap[InputAxis::MoveHorizontal];
	int sign = copysign(1, axisCoeff);
	short maxAngleNormalized = maxAngle * axisCoeff;

	if (coll->CollisionType == CT_LEFT || coll->CollisionType == CT_RIGHT)
		maxAngleNormalized *= 0.6f;

	item->Pose.Orientation.z += std::min<short>(baseRate, abs(maxAngleNormalized - item->Pose.Orientation.z) / 3) * sign;
}

void ModulateLaraCrawlFlex(ItemInfo* item, short baseRate, short maxAngle)
{
	auto* lara = GetLaraInfo(item);

	if (!item->Animation.Velocity.z)
		return;

	float axisCoeff = AxisMap[InputAxis::MoveHorizontal];
	int sign = copysign(1, axisCoeff);
	short maxAngleNormalized = maxAngle * axisCoeff;

	if (abs(lara->ExtraTorsoRot.z) < LARA_CRAWL_FLEX_MAX)
		lara->ExtraTorsoRot.z += std::min<short>(baseRate, abs(maxAngleNormalized - lara->ExtraTorsoRot.z) / 6) * sign;

	if (!(TrInput & IN_LOOK) &&
		item->Animation.ActiveState != LS_CRAWL_BACK)
	{
		lara->ExtraHeadRot.z = lara->ExtraTorsoRot.z / 2;
		lara->ExtraHeadRot.y = lara->ExtraHeadRot.z;
	}
}

// TODO: Unused; I will pick this back up later. -- Sezz 2022.06.22
void ModulateLaraSlideVelocity(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	constexpr int minVelocity = 50;
	constexpr int maxVelocity = LARA_TERMINAL_VELOCITY;

	if (g_GameFlow->HasSlideExtended())
	{
		auto probe = GetCollision(item);
		short minSlideAngle = ANGLE(33.75f);
		short steepness = Geometry::GetSurfaceSteepnessAngle(probe.FloorTilt);
		short direction = Geometry::GetSurfaceAspectAngle(probe.FloorTilt);

		float velocityMultiplier = 1 / (float)ANGLE(33.75f);
		int slideVelocity = std::min<int>(minVelocity + 10 * (steepness * velocityMultiplier), LARA_TERMINAL_VELOCITY);
		//short deltaAngle = abs((short)(direction - item->Pose.Orientation.y));

		g_Renderer.PrintDebugMessage("%d", slideVelocity);

		//lara->ExtraVelocity.x += slideVelocity;
		//lara->ExtraVelocity.y += slideVelocity * phd_sin(steepness);
	}
	//else
		//lara->ExtraVelocity.x += minVelocity;
}

void AlignLaraToSurface(ItemInfo* item, float alpha)
{
	// Calculate surface angles.
	auto floorTilt = GetCollision(item).FloorTilt;
	short aspectAngle = Geometry::GetSurfaceAspectAngle(floorTilt);
	short steepnessAngle = std::min(Geometry::GetSurfaceSteepnessAngle(floorTilt), ANGLE(70.0f));

	short deltaAngle = Geometry::GetShortestAngularDistance(item->Pose.Orientation.y, aspectAngle);
	float sinDeltaAngle = phd_sin(deltaAngle);
	float cosDeltaAngle = phd_cos(deltaAngle);

	// Calculate extra rotation required.
	auto extraRot = EulerAngles(
		-steepnessAngle * cosDeltaAngle,
		0,
		steepnessAngle * sinDeltaAngle
	) - EulerAngles(item->Pose.Orientation.x, 0, item->Pose.Orientation.z);

	// Apply extra rotation according to alpha.
	item->Pose.Orientation += extraRot * alpha;
}

void SetLaraJumpDirection(ItemInfo* item, CollisionInfo* coll) 
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_FORWARD &&
		TestLaraJumpForward(item, coll))
	{
		lara->Control.JumpDirection = JumpDirection::Forward;
	}
	else if (TrInput & IN_BACK &&
		TestLaraJumpBack(item, coll))
	{
		lara->Control.JumpDirection = JumpDirection::Back;
	}
	else if (TrInput & IN_LEFT &&
		TestLaraJumpLeft(item, coll))
	{
		lara->Control.JumpDirection = JumpDirection::Left;
	}
	else if (TrInput & IN_RIGHT &&
		TestLaraJumpRight(item, coll))
	{
		lara->Control.JumpDirection = JumpDirection::Right;
	}
	else if (TestLaraJumpUp(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		lara->Control.JumpDirection = JumpDirection::Up;
	else
		lara->Control.JumpDirection = JumpDirection::None;
}

// TODO: Add a timeout? Imagine a small, sad rain cloud with the properties of a ceiling following Lara overhead.
// RunJumpQueued will never reset, and when the sad cloud flies away after an indefinite amount of time, Lara will jump. -- Sezz 2022.01.22
void SetLaraRunJumpQueue(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int y = item->Pose.Position.y;
	int distance = SECTOR(1);
	auto probe = GetCollision(item, item->Pose.Orientation.y, distance, -coll->Setup.Height);

	if ((TestLaraRunJumpForward(item, coll) ||													// Area close ahead is permissive...
		(probe.Position.Ceiling - y) < -(coll->Setup.Height + (LARA_HEADROOM * 0.8f)) ||		// OR ceiling height far ahead is permissive
		(probe.Position.Floor - y) >= CLICK(0.5f)) &&											// OR there is a drop below far ahead.
		probe.Position.Floor != NO_HEIGHT)
	{
		lara->Control.RunJumpQueued = IsRunJumpQueueableState((LaraState)item->Animation.TargetState);
	}
	else
		lara->Control.RunJumpQueued = false;
}

void SetLaraVault(ItemInfo* item, CollisionInfo* coll, VaultTestResult vaultResult)
{
	auto* lara = GetLaraInfo(item);

	lara->ProjectedFloorHeight = vaultResult.Height;
	lara->Control.HandStatus = vaultResult.SetBusyHands ? HandStatus::Busy : lara->Control.HandStatus;
	lara->Control.TurnRate = 0;

	if (vaultResult.SnapToLedge)
	{
		SnapItemToLedge(item, coll, 0.2f, false);
		lara->TargetOrientation = EulerAngles(0, coll->NearestLedgeAngle, 0);
	}

	if (vaultResult.SetJumpVelocity)
	{
		int height = lara->ProjectedFloorHeight - item->Pose.Position.y;
		if (height > -CLICK(3.5f))
			height = -CLICK(3.5f);
		else if (height < -CLICK(7.5f))
			height = -CLICK(7.5f);

		lara->Control.CalculatedJumpVelocity = -3 - sqrt(-9600 - 12 * height); // TODO: Find a better formula for this that won't require the above block.
	}
}

void SetLaraLand(ItemInfo* item, CollisionInfo* coll)
{
	//item->IsAirborne = false; // TODO: Removing this avoids an unusual landing bug. I hope to find a proper solution later. -- Sezz 2022.02.18
	item->Animation.Velocity.y = 0.0f;
	LaraSnapToHeight(item, coll);
}

void SetLaraFallAnimation(ItemInfo* item)
{
	SetAnimation(item, LA_FALL_START);
	item->Animation.IsAirborne = true;
	item->Animation.Velocity.y = 0.0f;
}

void SetLaraFallBackAnimation(ItemInfo* item)
{
	SetAnimation(item, LA_FALL_BACK);
	item->Animation.IsAirborne = true;
	item->Animation.Velocity.y = 0.0f;
}

void SetLaraMonkeyFallAnimation(ItemInfo* item)
{
	// HACK: Disallow release during 180 turn action.
	if (item->Animation.ActiveState == LS_MONKEY_TURN_180)
		return;

	SetAnimation(item, LA_MONKEY_TO_FREEFALL);
	SetLaraMonkeyRelease(item);
}

void SetLaraMonkeyRelease(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	item->Animation.IsAirborne = true;
	item->Animation.Velocity.y = 1.0f;
	item->Animation.Velocity.z = 2.0f;
	lara->Control.TurnRate = 0;
	lara->Control.HandStatus = HandStatus::Free;
}

// temp
void SetLaraSlideAnimation(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TestEnvironment(ENV_FLAG_SWAMP, item))
		return;

	static short oldAngle = 1;

	if (abs(coll->FloorTilt.x) <= 2 && abs(coll->FloorTilt.y) <= 2)
		return;

	short angle = ANGLE(0.0f);
	if (coll->FloorTilt.x > 2)
		angle = -ANGLE(90.0f);
	else if (coll->FloorTilt.x < -2)
		angle = ANGLE(90.0f);

	if (coll->FloorTilt.y > 2 && coll->FloorTilt.y > abs(coll->FloorTilt.x))
		angle = ANGLE(180.0f);
	else if (coll->FloorTilt.y < -2 && -coll->FloorTilt.y > abs(coll->FloorTilt.x))
		angle = ANGLE(0.0f);

	short delta = angle - item->Pose.Orientation.y;

	ShiftItem(item, coll);

	if (delta < -ANGLE(90.0f) || delta > ANGLE(90.0f))
	{
		if (item->Animation.ActiveState == LS_SLIDE_BACK && oldAngle == angle)
			return;

		SetAnimation(item, LA_SLIDE_BACK_START);
		item->Pose.Orientation.y = angle + ANGLE(180.0f);
	}
	else
	{
		if (item->Animation.ActiveState == LS_SLIDE_FORWARD && oldAngle == angle)
			return;

		SetAnimation(item, LA_SLIDE_FORWARD);
		item->Pose.Orientation.y = angle;
	}

	LaraSnapToHeight(item, coll);
	lara->Control.MoveAngle = angle;
	lara->Control.TurnRate = 0;
	oldAngle = angle;
}

// TODO: Do it later.
void newSetLaraSlideAnimation(ItemInfo* item, CollisionInfo* coll)
{
	short headinAngle = GetLaraSlideDirection(item, coll);
	short deltaAngle = headinAngle - item->Pose.Orientation.y;

	if (!g_GameFlow->HasSlideExtended())
		item->Pose.Orientation.y = headinAngle;

	// Snap to height upon slide entrance.
	if (item->Animation.ActiveState != LS_SLIDE_FORWARD &&
		item->Animation.ActiveState != LS_SLIDE_BACK)
	{
		LaraSnapToHeight(item, coll);
	}

	// Slide forward.
	if (abs(deltaAngle) <= ANGLE(90.0f))
	{
		if (item->Animation.ActiveState == LS_SLIDE_FORWARD && abs(deltaAngle) <= ANGLE(180.0f))
			return;

		SetAnimation(item, LA_SLIDE_FORWARD);
	}
	// Slide backward.
	else
	{
		if (item->Animation.ActiveState == LS_SLIDE_BACK && abs((short)(deltaAngle - ANGLE(180.0f))) <= -ANGLE(180.0f))
			return;

		SetAnimation(item, LA_SLIDE_BACK_START);
	}
}

void SetLaraCornerAnimation(ItemInfo* item, CollisionInfo* coll, bool flip)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		SetAnimation(item, LA_FALL_START);
		item->Animation.IsAirborne = true;
		item->Animation.Velocity.z = 2;
		item->Animation.Velocity.y = 1;
		item->Pose.Position.y += CLICK(1);
		item->Pose.Orientation.y += lara->NextCornerPos.Orientation.y / 2;
		lara->Control.HandStatus = HandStatus::Free;
		return;
	}

	if (flip)
	{
		if (lara->Control.IsClimbingLadder)
			SetAnimation(item, LA_LADDER_IDLE);
		else
			SetAnimation(item, LA_HANG_IDLE);

		item->Pose.Position = lara->NextCornerPos.Position;
		item->Pose.Orientation.y = lara->NextCornerPos.Orientation.y;
		coll->Setup.OldPosition = lara->NextCornerPos.Position;
	}
}

void SetLaraSwimDiveAnimation(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	SetAnimation(item, LA_ONWATER_DIVE);
	item->Animation.TargetState = LS_UNDERWATER_SWIM_FORWARD;
	item->Animation.Velocity.y = LARA_SWIM_VELOCITY_MAX * 0.4f;
	item->Pose.Orientation.x = -ANGLE(45.0f);
	lara->Control.WaterStatus = WaterStatus::Underwater;
}

void ResetLaraLean(ItemInfo* item, float rate, bool resetRoll, bool resetPitch)
{
	if (!rate)
	{
		TENLog(std::string("ResetLaraLean() attempted division by zero!"), LogLevel::Warning);
		return;
	}

	rate = abs(rate);

	if (resetPitch)
	{
		if (abs(item->Pose.Orientation.x) > ANGLE(0.1f))
			item->Pose.Orientation.x += item->Pose.Orientation.x / -rate;
		else
			item->Pose.Orientation.x = 0;
	}

	if (resetRoll)
	{
		if (abs(item->Pose.Orientation.z) > ANGLE(0.1f))
			item->Pose.Orientation.z += item->Pose.Orientation.z / -rate;
		else
			item->Pose.Orientation.z = 0;
	}
}

void ResetLaraFlex(ItemInfo* item, float rate)
{
	auto* lara = GetLaraInfo(item);

	if (!rate)
	{
		TENLog(std::string("ResetLaraFlex() attempted division by zero."), LogLevel::Warning);
		return;
	}

	rate = abs(rate);

	// Reset head.
	if (abs(lara->ExtraHeadRot.x) > ANGLE(0.1f))
		lara->ExtraHeadRot.x += lara->ExtraHeadRot.x / -rate;
	else
		lara->ExtraHeadRot.x = 0;

	if (abs(lara->ExtraHeadRot.y) > ANGLE(0.1f))
		lara->ExtraHeadRot.y += lara->ExtraHeadRot.y / -rate;
	else
		lara->ExtraHeadRot.y = 0;

	if (abs(lara->ExtraHeadRot.z) > ANGLE(0.1f))
		lara->ExtraHeadRot.z += lara->ExtraHeadRot.z / -rate;
	else
		lara->ExtraHeadRot.z = 0;

	// Reset torso.
	if (abs(lara->ExtraTorsoRot.x) > ANGLE(0.1f))
		lara->ExtraTorsoRot.x += lara->ExtraTorsoRot.x / -rate;
	else
		lara->ExtraTorsoRot.x = 0;

	if (abs(lara->ExtraTorsoRot.y) > ANGLE(0.1f))
		lara->ExtraTorsoRot.y += lara->ExtraTorsoRot.y / -rate;
	else
		lara->ExtraTorsoRot.y = 0;

	if (abs(lara->ExtraTorsoRot.z) > ANGLE(0.1f))
		lara->ExtraTorsoRot.z += lara->ExtraTorsoRot.z / -rate;
	else
		lara->ExtraTorsoRot.z = 0;
}

void RumbleLaraHealthCondition(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints > LARA_HEALTH_CRITICAL && !lara->PoisonPotency)
		return;

	bool doPulse = (GlobalCounter & 0x0F) % 0x0F == 1;
	if (doPulse)
		Rumble(0.2f, 0.1f);
}
