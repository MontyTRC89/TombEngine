#include "framework.h"
#include "Game/Lara/lara_helpers.h"

#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Flow/ScriptInterfaceFlowHandler.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_tests.h"
#include "Renderer/Renderer11.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#include "Objects/TR2/Vehicles/skidoo.h"
#include "Objects/TR3/Vehicles/biggun.h"
#include "Objects/TR3/Vehicles/kayak.h"
#include "Objects/TR3/Vehicles/minecart.h"
#include "Objects/TR3/Vehicles/quad.h"
#include "Objects/TR3/Vehicles/upv.h"
#include "Objects/TR4/Vehicles/jeep.h"
#include "Objects/TR4/Vehicles/motorbike.h"

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
		!(TrInput & (IN_WAKE | IN_LOOK)) &&
		g_GameFlow->HasAFKPose())
	{
		lara->Control.Count.Pose++;
	}
	else
		lara->Control.Count.Pose = 0;

	// Reset running jump timer.
	if (!IsRunJumpCountableState((LaraState)item->Animation.ActiveState))
		lara->Control.Count.RunJump = 0;

	// Reset running jump action queue.
	if (!IsRunJumpQueueableState((LaraState)item->Animation.ActiveState))
		lara->Control.RunJumpQueued = false;

	// Reset lean.
	if (!lara->Control.IsMoving || (lara->Control.IsMoving && !(TrInput & (IN_LEFT | IN_RIGHT))))
		ResetLaraLean(item, 6.0f);

	// Reset crawl flex.
	if (!(TrInput & IN_LOOK) && coll->Setup.Height > LARA_HEIGHT - LARA_HEADROOM &&	// HACK
		(!item->Animation.Velocity || (item->Animation.Velocity && !(TrInput & (IN_LEFT | IN_RIGHT)))))
	{
		ResetLaraFlex(item, 12.0f);
	}

	// Reset turn rate.
	// TODO: Make it less stupid in the future. Do it according to a curve?
	int sign = copysign(1, lara->Control.TurnRate);
	if (abs(lara->Control.TurnRate) > ANGLE(2.0f))
		lara->Control.TurnRate -= ANGLE(2.0f) * sign;
	else if (abs(lara->Control.TurnRate) > ANGLE(0.5f))
		lara->Control.TurnRate -= ANGLE(0.5f) * sign;
	else
		lara->Control.TurnRate = 0;
	item->Pose.Orientation.y += lara->Control.TurnRate;
}

bool HandleLaraVehicle(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Vehicle != NO_ITEM)
	{
		switch (g_Level.Items[lara->Vehicle].ObjectNumber)
		{
		case ID_QUAD:
			QuadBikeControl(item, coll);
			break;

		case ID_JEEP:
			JeepControl();
			break;

		case ID_MOTORBIKE:
			MotorbikeControl();
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
			MineCartControl(item);
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

	return false;
}

void ApproachLaraTargetOrientation(ItemInfo* item, Vector3Shrt targetOrient, float rate)
{
	auto* lara = GetLaraInfo(item);

	if (!rate)
	{
		TENLog(std::string("ApproachLaraTargetOrientation() attempted division by zero."), LogLevel::Warning);
		return;
	}

	rate = abs(rate);

	if (abs((short)(targetOrient.x - item->Pose.Orientation.x)) > ANGLE(0.1f))
		item->Pose.Orientation.x += (short)(targetOrient.x - item->Pose.Orientation.x) / rate;
	else
		item->Pose.Orientation.x = targetOrient.x;

	if (abs((short)(targetOrient.y - item->Pose.Orientation.y)) > ANGLE(0.1f))
		item->Pose.Orientation.y += (short)(targetOrient.y - item->Pose.Orientation.y) / rate;
	else
		item->Pose.Orientation.y = targetOrient.y;

	if (abs((short)(targetOrient.z - item->Pose.Orientation.z)) > ANGLE(0.1f))
		item->Pose.Orientation.z += (short)(targetOrient.z - item->Pose.Orientation.z) / rate;
	else
		item->Pose.Orientation.z = targetOrient.z;
}

// TODO: This approach may cause undesirable artefacts where an object pushes Lara rapidly up/down a slope or a platform rapidly ascends/descends.
// Nobody panic. I have ideas. @Sezz 2022.03.24
void EaseOutLaraHeight(ItemInfo* item, int height)
{
	if (height == NO_HEIGHT)
		return;

	// Translate Lara to new height.
	static constexpr int rate = 50;
	int threshold = std::max(abs(item->Animation.Velocity) * 1.5f, CLICK(0.25f) / 4);
	int sign = std::copysign(1, height);
	
	if (TestEnvironment(ENV_FLAG_SWAMP, item) && height > 0)
		item->Pose.Position.y += SWAMP_GRAVITY;
	else if (abs(height) > (STEPUP_HEIGHT / 2))		// Outer range.
		item->Pose.Position.y += rate * sign;
	else if (abs(height) <= (STEPUP_HEIGHT / 2) &&	// Inner range.
		abs(height) >= threshold)
	{
		item->Pose.Position.y += std::max<int>(abs(height / 2.75), threshold) * sign;
	}
	else
		item->Pose.Position.y += height;
}

// TODO: Make lean rate proportional to the turn rate, allowing for nicer aesthetics with future analog stick input.
void DoLaraLean(ItemInfo* item, CollisionInfo* coll, short maxAngle, short rate)
{
	if (!item->Animation.Velocity)
		return;

	rate = abs(rate);
	int sign = copysign(1, maxAngle);

	if (coll->CollisionType == CT_LEFT || coll->CollisionType == CT_RIGHT)
		item->Pose.Orientation.z += std::min<short>(rate, abs((maxAngle * 3) / 5 - item->Pose.Orientation.z) / 3) * sign;
	else
		item->Pose.Orientation.z += std::min<short>(rate, abs(maxAngle - item->Pose.Orientation.z) / 3) * sign;
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
			if (GetChange(item, &g_Level.Anims[item->Animation.AnimNumber]))
			{
				item->Pose.Position.y += coll->Middle.Floor;
				return;
			}
		}
		else if (TestLaraStepDown(item, coll))
		{
			item->Animation.TargetState = LS_STEP_DOWN;
			if (GetChange(item, &g_Level.Anims[item->Animation.AnimNumber]))
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
		MoveItem(item, item->Pose.Orientation.y, -LARA_RADIUS_CRAWL);
		item->Pose.Orientation.y += ANGLE(180.0f);
	}
}

void DoLaraCrawlFlex(ItemInfo* item, CollisionInfo* coll, short maxAngle, short rate)
{
	auto* lara = GetLaraInfo(item);

	if (!item->Animation.Velocity)
		return;

	int sign = copysign(1, maxAngle);
	rate = copysign(rate, maxAngle);

	lara->ExtraTorsoRot.z += std::min(abs(rate), abs(maxAngle - lara->ExtraTorsoRot.z) / 6) * sign;

	if (!(TrInput & IN_LOOK) &&
		item->Animation.ActiveState != LS_CRAWL_BACK)
	{
		lara->ExtraHeadRot.z = lara->ExtraTorsoRot.z / 2;
		lara->ExtraHeadRot.y = lara->ExtraHeadRot.z;
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

void DealLaraFallDamage(ItemInfo* item)
{
	if (item->Animation.VerticalVelocity >= LARA_DAMAGE_VELOCITY)
	{
		if (item->Animation.VerticalVelocity >= LARA_DEATH_VELOCITY)
			item->HitPoints = 0;
		else USE_FEATURE_IF_CPP20([[likely]])
		{
			int base = item->Animation.VerticalVelocity - (LARA_DAMAGE_VELOCITY - 1);
			item->HitPoints -= LARA_HEALTH_MAX * (pow(base, 2) / 196);
		}
	}
}

LaraInfo*& GetLaraInfo(ItemInfo* item)
{
	if (item->ObjectNumber == ID_LARA)
		return (LaraInfo*&)item->Data;

	TENLog(std::string("Attempted to fetch LaraInfo data from entity with object ID ") + std::to_string(item->ObjectNumber), LogLevel::Warning);

	auto* firstLaraItem = FindItem(ID_LARA);
	return (LaraInfo*&)firstLaraItem->Data;
}

short GetLaraSlideDirection(ItemInfo* item, CollisionInfo* coll)
{
	short direction = coll->Setup.ForwardAngle;
	auto probe = GetCollision(item);

	// Ground is flat.
	if (!probe.FloorTilt.x && !probe.FloorTilt.y)
		return direction;

	direction = GetSurfaceAspectAngle(probe.FloorTilt.x, probe.FloorTilt.y);

	// Determine nearest cardinal direction of surface aspect.
	if (!g_GameFlow->HasSlideExtended())
		direction = GetQuadrant(direction) * ANGLE(90.0f);

	return direction;
}

void ModulateLaraSlideVelocity(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	constexpr int minVelocity = 50;
	constexpr int maxVelocity = LARA_TERMINAL_VELOCITY;

	if (g_GameFlow->HasSlideExtended())
	{
		auto probe = GetCollision(item);
		short minSlideAngle = ANGLE(33.75f);
		short steepness = GetSurfaceSteepnessAngle(probe.FloorTilt.x, probe.FloorTilt.y);
		short direction = GetSurfaceAspectAngle(probe.FloorTilt.x, probe.FloorTilt.y);

		float velocityMultiplier = 1 / (float)ANGLE(33.75f);
		int slideVelocity = std::min<int>(minVelocity + 10 * (steepness * velocityMultiplier), LARA_TERMINAL_VELOCITY);
		//short deltaAngle = abs((short)(direction - item->Pose.Orientation.y));

		g_Renderer.PrintDebugMessage("%d", slideVelocity);

		lara->ExtraVelocity.x += slideVelocity;
		lara->ExtraVelocity.y += slideVelocity * phd_sin(steepness);
	}
	else
		lara->ExtraVelocity.x += minVelocity;
}

void UpdateLaraSubsuitAngles(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.Subsuit.VerticalVelocity != 0)
	{
		item->Pose.Position.y += lara->Control.Subsuit.VerticalVelocity / 4;
		lara->Control.Subsuit.VerticalVelocity = ceil((15 / 16) * lara->Control.Subsuit.VerticalVelocity - 1);
	}

	lara->Control.Subsuit.Velocity[0] = -4 * item->Animation.VerticalVelocity;
	lara->Control.Subsuit.Velocity[1] = -4 * item->Animation.VerticalVelocity;

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
		SoundEffect(SFX_TR5_DIVE_SUIT_ENGINE, &item->Pose, SoundEnvironment::Water, 1.0f + (mul1 + mul2), vol);
	}
}

void ModulateLaraSubsuitSwimTurn(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_FORWARD && item->Pose.Orientation.x > -ANGLE(85.0f))
		lara->Control.Subsuit.DXRot = -ANGLE(45.0f);
	else if (TrInput & IN_BACK && item->Pose.Orientation.x < ANGLE(85.0f))
		lara->Control.Subsuit.DXRot = ANGLE(45.0f);
	else
		lara->Control.Subsuit.DXRot = 0;

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_SUBSUIT_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_MED_TURN_MAX)
			lara->Control.TurnRate = -LARA_MED_TURN_MAX;

		item->Pose.Orientation.z -= LARA_LEAN_RATE;
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_SUBSUIT_TURN_RATE;
		if (lara->Control.TurnRate > LARA_MED_TURN_MAX)
			lara->Control.TurnRate = LARA_MED_TURN_MAX;

		item->Pose.Orientation.z += LARA_LEAN_RATE;
	}
}

void ModulateLaraSwimTurn(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_FORWARD)
		item->Pose.Orientation.x -= ANGLE(2.0f);
	else if (TrInput & IN_BACK)
		item->Pose.Orientation.x += ANGLE(2.0f);

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_MED_TURN_MAX)
			lara->Control.TurnRate = -LARA_MED_TURN_MAX;

		item->Pose.Orientation.z -= LARA_LEAN_RATE;
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_MED_TURN_MAX)
			lara->Control.TurnRate = LARA_MED_TURN_MAX;

		item->Pose.Orientation.z += LARA_LEAN_RATE;
	}
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
// RunJumpQueued will never reset, and when the sad cloud flies away after an indefinite amount of time, Lara will jump. @Sezz 2022.01.22
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
		lara->TargetOrientation = Vector3Shrt(0, coll->NearestLedgeAngle, 0);
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
	item->Animation.Velocity = 0;
	item->Animation.VerticalVelocity = 0;
	//item->Airborne = false; // TODO: Removing this avoids an unusual landing bug Core had worked around in an obscure way. I hope to find a proper solution. @Sezz 2022.02.18

	LaraSnapToHeight(item, coll);
}

void SetLaraFallAnimation(ItemInfo* item)
{
	SetAnimation(item, LA_FALL_START);
	item->Animation.VerticalVelocity = 0;
	item->Animation.Airborne = true;
}

void SetLaraFallBackAnimation(ItemInfo* item)
{
	SetAnimation(item, LA_FALL_BACK);
	item->Animation.VerticalVelocity = 0;
	item->Animation.Airborne = true;
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

	item->Animation.Velocity = 2;
	item->Animation.VerticalVelocity = 1;
	item->Animation.Airborne = true;
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

	lara->Control.MoveAngle = angle;
	oldAngle = angle;
}

// TODO: Do it later.
void newSetLaraSlideAnimation(ItemInfo* item, CollisionInfo* coll)
{
	short direction = GetLaraSlideDirection(item, coll);
	short deltaAngle = direction - item->Pose.Orientation.y;

	if (!g_GameFlow->HasSlideExtended())
		item->Pose.Orientation.y = direction;

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
		item->Pose.Position.y += CLICK(1);
		item->Pose.Orientation.y += lara->NextCornerPos.Orientation.y / 2;
		item->Animation.Velocity = 2;
		item->Animation.VerticalVelocity = 1;
		item->Animation.Airborne = true;
		lara->Control.HandStatus = HandStatus::Free;
		return;
	}

	if (flip)
	{
		if (lara->Control.IsClimbingLadder)
			SetAnimation(item, LA_LADDER_IDLE);
		else
			SetAnimation(item, LA_HANG_IDLE);

		coll->Setup.OldPosition.x = item->Pose.Position.x = lara->NextCornerPos.Position.x;
		coll->Setup.OldPosition.y = item->Pose.Position.y = lara->NextCornerPos.Position.y;
		coll->Setup.OldPosition.z = item->Pose.Position.z = lara->NextCornerPos.Position.z;
		item->Pose.Orientation.y = lara->NextCornerPos.Orientation.y;
	}
}

void SetLaraSwimDiveAnimation(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	SetAnimation(item, LA_ONWATER_DIVE);
	item->Animation.TargetState = LS_UNDERWATER_SWIM_FORWARD;
	item->Animation.VerticalVelocity = LARA_SWIM_VELOCITY_MAX * 0.4f;
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
