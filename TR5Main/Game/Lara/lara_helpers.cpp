#include "framework.h"
#include "Game/Lara/lara_helpers.h"

#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Scripting/Flow/ScriptInterfaceFlowHandler.h"
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
		ResetLaraLean(item, 0.15f);

	// Reset crawl flex.
	if (!(TrInput & IN_LOOK) && coll->Setup.Height > LARA_HEIGHT - LARA_HEADROOM &&	// HACK
		(!item->Animation.Velocity || (item->Animation.Velocity && !(TrInput & (IN_LEFT | IN_RIGHT)))))
	{
		ResetLaraFlex(item, 0.15f);
	}

	// Reset turn rate.
	// TODO: Make it less stupid in the future. Do it according to a curve?
	int sign = copysign(1, lara->Control.TurnRate);
	if (abs(lara->Control.TurnRate) > Angle::DegToRad(2.0f))
		lara->Control.TurnRate -= Angle::DegToRad(2.0f) * sign;
	else if (abs(lara->Control.TurnRate) > Angle::DegToRad(0.5f))
		lara->Control.TurnRate -= Angle::DegToRad(0.5f) * sign;
	else
		lara->Control.TurnRate = 0;
	item->Pose.Orientation.SetY(item->Pose.Orientation.GetY() + lara->Control.TurnRate);
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
void DoLaraLean(ItemInfo* item, CollisionInfo* coll, float maxAngle, float rate)
{
	if (!item->Animation.Velocity)
		return;

	rate = abs(rate);
	int sign = copysign(1, maxAngle);

	if (coll->CollisionType == CT_LEFT || coll->CollisionType == CT_RIGHT)
		item->Pose.Orientation.SetZ(item->Pose.Orientation.GetZ() + std::min(rate, abs((maxAngle * 3) / 5 - item->Pose.Orientation.GetZ()) / 3) * sign);
	else
		item->Pose.Orientation.SetZ(item->Pose.Orientation.GetZ() + std::min(rate, abs(maxAngle - item->Pose.Orientation.GetZ()) / 3) * sign);
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
	coll->Setup.ForwardAngle = Angle::Normalize(item->Pose.Orientation.GetY() + Angle::DegToRad(180.0f));
	GetCollisionInfo(coll, item);

	SnapItemToLedge(item, coll);
	LaraResetGravityStatus(item, coll);

	// Bridges behave differently.
	if (coll->Middle.Bridge < 0)
	{
		MoveItem(item, item->Pose.Orientation.GetY(), -LARA_RADIUS_CRAWL);
		item->Pose.Orientation.SetY(item->Pose.Orientation.GetY() + Angle::DegToRad(180.0f));
	}
}

void DoLaraCrawlFlex(ItemInfo* item, CollisionInfo* coll, float maxAngle, float rate)
{
	auto* lara = GetLaraInfo(item);

	if (!item->Animation.Velocity)
		return;

	int sign = copysign(1, maxAngle);
	rate = copysign(rate, maxAngle);

	lara->ExtraTorsoRot.SetZ(lara->ExtraTorsoRot.GetZ() + std::min(abs(rate), abs(maxAngle - lara->ExtraTorsoRot.GetZ()) / 6) * sign);

	if (!(TrInput & IN_LOOK) &&
		item->Animation.ActiveState != LS_CRAWL_BACK)
	{
		lara->ExtraHeadRot.SetZ(lara->ExtraTorsoRot.GetZ() / 2);
		lara->ExtraHeadRot.SetY(lara->ExtraHeadRot.GetZ());
	}
}

void DoLaraTightropeBalance(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);
	const int factor = ((lara->Control.Tightrope.TimeOnTightrope >> 7) & 0xFF) * 128;

	if (TrInput & IN_LEFT)
		lara->Control.Tightrope.Balance += Angle::DegToRad(1.4f);
	if (TrInput & IN_RIGHT)
		lara->Control.Tightrope.Balance -= Angle::DegToRad(1.4f);

	if (lara->Control.Tightrope.Balance < 0)
	{
		lara->Control.Tightrope.Balance -= factor;
		if (lara->Control.Tightrope.Balance <= Angle::DegToRad(-45.0f))
			lara->Control.Tightrope.Balance = Angle::DegToRad(-45.0f);

	}
	else if (lara->Control.Tightrope.Balance > 0)
	{
		lara->Control.Tightrope.Balance += factor;
		if (lara->Control.Tightrope.Balance >= Angle::DegToRad(45.0f))
			lara->Control.Tightrope.Balance = Angle::DegToRad(45.0f);
	}
	else
		lara->Control.Tightrope.Balance = GetRandomControl() & 1 ? -1 : 1;
}

void DoLaraTightropeLean(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	item->Pose.Orientation.SetZ(lara->Control.Tightrope.Balance / 4);
	lara->ExtraTorsoRot.SetZ(lara->Control.Tightrope.Balance + Angle::DegToRad(180.0f));
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
		if (lara->Control.Tightrope.Balance <= Angle::DegToRad(0.75f))
			lara->Control.Tightrope.Balance = 0;
		else
			lara->Control.Tightrope.Balance -= Angle::DegToRad(0.75f);
	}

	if (lara->Control.Tightrope.Balance < 0)
	{
		if (lara->Control.Tightrope.Balance >= Angle::DegToRad(-0.75f))
			lara->Control.Tightrope.Balance = 0;
		else
			lara->Control.Tightrope.Balance += Angle::DegToRad(0.75f);
	}
}

void DoLaraFallDamage(ItemInfo* item)
{
	if (item->Animation.VerticalVelocity >= LARA_DAMAGE_VELOCITY)
	{
		if (item->Animation.VerticalVelocity >= LARA_DEATH_VELOCITY)
			item->HitPoints = 0;
		else [[likely]]
		{
			float base = item->Animation.VerticalVelocity - (LARA_DAMAGE_VELOCITY - 1);
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

float GetLaraSlideDirection(ItemInfo* item, CollisionInfo* coll)
{
	float direction = coll->Setup.ForwardAngle;
	auto probe = GetCollision(item);

	// Ground is flat.
	if (!probe.FloorTilt.x && !probe.FloorTilt.y)
		return direction;

	direction = GetSurfaceAspectAngle(probe.FloorTilt.x, probe.FloorTilt.y);

	// Determine nearest cardinal direction of surface aspect.
	if (!g_GameFlow->HasSlideExtended())
		direction = GetQuadrant(direction) * Angle::DegToRad(90.0f);

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
		float minSlideAngle = Angle::DegToRad(33.75f);
		float steepness = GetSurfaceSteepnessAngle(probe.FloorTilt.x, probe.FloorTilt.y);
		float direction = GetSurfaceAspectAngle(probe.FloorTilt.x, probe.FloorTilt.y);

		float velocityMultiplier = 1 / Angle::DegToRad(33.75f);
		int slideVelocity = std::min<int>(minVelocity + 10 * (steepness * velocityMultiplier), LARA_TERMINAL_VELOCITY);
		float deltaAngle = Angle::ShortestAngle(direction, item->Pose.Orientation.GetY());

		g_Renderer.PrintDebugMessage("%d", slideVelocity);

		lara->ExtraVelocity.x += slideVelocity;
		lara->ExtraVelocity.y += slideVelocity * sin(steepness);
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

			lara->Control.Subsuit.XRot -= Angle::DegToRad(2.0f);
			if (lara->Control.Subsuit.XRot < lara->Control.Subsuit.DXRot)
				lara->Control.Subsuit.XRot = lara->Control.Subsuit.DXRot;
		}
	}
	else
	{
		if (lara->Control.Subsuit.XRot < 0 && lara->Control.Subsuit.DXRot > 0)
			lara->Control.Subsuit.XRot = ceil(0.75 * lara->Control.Subsuit.XRot);

		lara->Control.Subsuit.XRot += Angle::DegToRad(2.0f);
		if (lara->Control.Subsuit.XRot > lara->Control.Subsuit.DXRot)
			lara->Control.Subsuit.XRot = lara->Control.Subsuit.DXRot;
	}

	if (lara->Control.Subsuit.DXRot != 0)
	{
		float rotation = lara->Control.Subsuit.DXRot / 8;
		if (rotation < Angle::DegToRad(-2.0f))
			rotation = Angle::DegToRad(-2.0f);
		else if (rotation > Angle::DegToRad(2.0f))
			rotation = Angle::DegToRad(2.0f);

		item->Pose.Orientation.SetX(item->Pose.Orientation.GetX() + rotation);
	}

	lara->Control.Subsuit.Velocity[0] += abs(lara->Control.Subsuit.XRot / 8);
	lara->Control.Subsuit.Velocity[1] += abs(lara->Control.Subsuit.XRot / 8);

	if (lara->Control.TurnRate > 0)
		lara->Control.Subsuit.Velocity[0] += abs(lara->Control.TurnRate) * 2;
	else if (lara->Control.TurnRate < 0)
		lara->Control.Subsuit.Velocity[1] += abs(lara->Control.TurnRate) * 2;

	if (lara->Control.Subsuit.Velocity[0] > SECTOR(1.5f))
		lara->Control.Subsuit.Velocity[0] = SECTOR(1.5f);

	if (lara->Control.Subsuit.Velocity[1] > SECTOR(1.5f))
		lara->Control.Subsuit.Velocity[1] = SECTOR(1.5f);

	if (lara->Control.Subsuit.Velocity[0] != 0 || lara->Control.Subsuit.Velocity[1] != 0)
		SoundEffect(SFX_TR5_DIVE_SUIT_ENGINE, &item->Pose, (((lara->Control.Subsuit.Velocity[0] + lara->Control.Subsuit.Velocity[1]) * 4) & 0x1F00) + 10);
}

void ModulateLaraSubsuitSwimTurn(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	if (item->Pose.Position.y < 14080)
		lara->Control.Subsuit.VerticalVelocity += (14080 - item->Pose.Position.y) >> 4;

	if (TrInput & IN_FORWARD && item->Pose.Orientation.GetX() > Angle::DegToRad(-85.0f))
		lara->Control.Subsuit.DXRot = Angle::DegToRad(-45.0f);
	else if (TrInput & IN_BACK && item->Pose.Orientation.GetX() < Angle::DegToRad(85.0f))
		lara->Control.Subsuit.DXRot = Angle::DegToRad(45.0f);
	else
		lara->Control.Subsuit.DXRot = 0;

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_SUBSUIT_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_MED_TURN_MAX)
			lara->Control.TurnRate = -LARA_MED_TURN_MAX;

		DoLaraLean(item, &LaraCollision, -LARA_LEAN_MAX, LARA_LEAN_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_SUBSUIT_TURN_RATE;
		if (lara->Control.TurnRate > LARA_MED_TURN_MAX)
			lara->Control.TurnRate = LARA_MED_TURN_MAX;

		DoLaraLean(item, &LaraCollision, LARA_LEAN_MAX, LARA_LEAN_RATE);
	}
}

void ModulateLaraSwimTurn(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_FORWARD)
		item->Pose.Orientation.SetX(item->Pose.Orientation.GetX() - Angle::DegToRad(2.0f));
	else if (TrInput & IN_BACK)
		item->Pose.Orientation.SetX(item->Pose.Orientation.GetX() + Angle::DegToRad(2.0f));

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_MED_TURN_MAX)
			lara->Control.TurnRate = -LARA_MED_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_MED_TURN_MAX)
			lara->Control.TurnRate = LARA_MED_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE);
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
	else if (TestLaraJumpUp(item, coll)) [[likely]]
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
	auto probe = GetCollision(item, item->Pose.Orientation.GetY(), distance, -coll->Setup.Height);

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
	//item->Airborne = false; // TODO: Removing this avoids an unusual landing bug Core had worked around in an obscure way. I hope to find a proper solution. @Sezz 2022.02.18
	item->Animation.Velocity = 0;
	item->Animation.VerticalVelocity = 0;

	LaraSnapToHeight(item, coll);
}

void SetLaraFallAnimation(ItemInfo* item)
{
	SetAnimation(item, LA_FALL_START);
	item->Animation.Airborne = true;
	item->Animation.VerticalVelocity = 0;
}

void SetLaraFallBackAnimation(ItemInfo* item)
{
	SetAnimation(item, LA_FALL_BACK);
	item->Animation.Airborne = true;
	item->Animation.VerticalVelocity = 0;
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

	item->Animation.Airborne = true;
	item->Animation.Velocity = 2;
	item->Animation.VerticalVelocity = 1;
	lara->Control.HandStatus = HandStatus::Free;
}

// temp
void SetLaraSlideAnimation(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TestEnvironment(ENV_FLAG_SWAMP, item))
		return;

	static float oldAngle = 1;

	if (abs(coll->FloorTilt.x) <= 2 && abs(coll->FloorTilt.y) <= 2)
		return;

	float angle = Angle::DegToRad(0.0f);
	if (coll->FloorTilt.x > 2)
		angle = Angle::DegToRad(-90.0f);
	else if (coll->FloorTilt.x < -2)
		angle = Angle::DegToRad(90.0f);

	if (coll->FloorTilt.y > 2 && coll->FloorTilt.y > abs(coll->FloorTilt.x))
		angle = Angle::DegToRad(180.0f);
	else if (coll->FloorTilt.y < -2 && -coll->FloorTilt.y > abs(coll->FloorTilt.x))
		angle = Angle::DegToRad(0.0f);

	float delta = Angle::Normalize(angle - item->Pose.Orientation.GetY());

	ShiftItem(item, coll);

	if (delta < Angle::DegToRad(-90.0f) || delta > Angle::DegToRad(90.0f))
	{
		if (item->Animation.ActiveState == LS_SLIDE_BACK && oldAngle == angle)
			return;

		SetAnimation(item, LA_SLIDE_BACK_START);
		item->Pose.Orientation.SetY(angle + Angle::DegToRad(180.0f));
	}
	else
	{
		if (item->Animation.ActiveState == LS_SLIDE_FORWARD && oldAngle == angle)
			return;

		SetAnimation(item, LA_SLIDE_FORWARD);
		item->Pose.Orientation.SetY(angle);
	}

	lara->Control.MoveAngle = angle;
	oldAngle = angle;
}

// TODO: Do it later.
void newSetLaraSlideAnimation(ItemInfo* item, CollisionInfo* coll)
{
	float direction = GetLaraSlideDirection(item, coll);
	float deltaAngle = direction - item->Pose.Orientation.GetY();

	if (!g_GameFlow->HasSlideExtended())
		item->Pose.Orientation.SetY(direction);

	// Snap to height upon slide entrance.
	if (item->Animation.ActiveState != LS_SLIDE_FORWARD &&
		item->Animation.ActiveState != LS_SLIDE_BACK)
	{
		LaraSnapToHeight(item, coll);
	}

	// Slide forward.
	if (abs(deltaAngle) <= Angle::DegToRad(90.0f))
	{
		if (item->Animation.ActiveState == LS_SLIDE_FORWARD && abs(deltaAngle) <= Angle::DegToRad(180.0f))
			return;

		SetAnimation(item, LA_SLIDE_FORWARD);
	}
	// Slide backward.
	else
	{
		if (item->Animation.ActiveState == LS_SLIDE_BACK && abs((short)(deltaAngle - Angle::DegToRad(180.0f))) <= Angle::DegToRad(-180.0f))
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
		item->Pose.Orientation.SetY(item->Pose.Orientation.GetY() + lara->NextCornerPos.Orientation.GetY() / 2);
		item->Animation.Airborne = true;
		item->Animation.Velocity = 2;
		item->Animation.VerticalVelocity = 1;
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
		item->Pose.Orientation.SetY(lara->NextCornerPos.Orientation.GetY());
		coll->Setup.OldPosition = lara->NextCornerPos.Position;
	}
}

void SetLaraSwimDiveAnimation(ItemInfo* item)
{
	auto* lara = GetLaraInfo(item);

	SetAnimation(item, LA_ONWATER_DIVE);
	item->Animation.TargetState = LS_UNDERWATER_SWIM_FORWARD;
	item->Animation.VerticalVelocity = LARA_SWIM_VELOCITY_MAX * 0.4f;
	item->Pose.Orientation.SetX(Angle::DegToRad(-45.0f));
	lara->Control.WaterStatus = WaterStatus::Underwater;
}

void ResetLaraLean(ItemInfo* item, float rate, bool resetRoll, bool resetPitch)
{
	if (resetPitch)
		item->Pose.Orientation.SetX(Angle::Interpolate(item->Pose.Orientation.GetX(), 0, rate, Angle::DegToRad(0.1f)));

	if (resetRoll)
		item->Pose.Orientation.SetZ(Angle::Interpolate(item->Pose.Orientation.GetZ(), 0, rate, Angle::DegToRad(0.1f)));
}

void ResetLaraFlex(ItemInfo* item, float rate)
{
	auto* lara = GetLaraInfo(item);

	lara->ExtraHeadRot.Interpolate(EulerAngles::Zero, rate, Angle::DegToRad(0.1f));
	lara->ExtraTorsoRot.Interpolate(EulerAngles::Zero, rate, Angle::DegToRad(0.1f));
}
