#include "framework.h"
#include "Game/Lara/lara_helpers.h"

#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_tests.h"
#include "Scripting/GameFlowScript.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#include "Objects/TR2/Vehicles/snowmobile.h"
#include "Objects/TR3/Vehicles/biggun.h"
#include "Objects/TR3/Vehicles/kayak.h"
#include "Objects/TR3/Vehicles/minecart.h"
#include "Objects/TR3/Vehicles/quad.h"
#include "Objects/TR3/Vehicles/upv.h"
#include "Objects/TR4/Vehicles/jeep.h"
#include "Objects/TR4/Vehicles/motorbike.h"

// -----------------------------
// HELPER FUNCTIONS
// For State Control & Collision
// -----------------------------

// TODO: Make lean rate proportional to the turn rate, allowing for nicer aesthetics with future analog stick input.
void DoLaraLean(ITEM_INFO* item, COLL_INFO* coll, short maxAngle, short rate)
{
	if (!item->Velocity)
		return;

	int sign = copysign(1, maxAngle);

	rate = abs(rate);

	if (coll->CollisionType == CT_LEFT || coll->CollisionType == CT_RIGHT)
		item->Position.zRot += std::min<short>(rate, abs((maxAngle * 3) / 5 - item->Position.zRot) / 3) * sign;
	else
		item->Position.zRot += std::min<short>(rate, abs(maxAngle - item->Position.zRot) / 3) * sign;
}

// Works, but disabled for the time being. @Sezz 2022.02.13
void ApproachLaraTargetAngle(ITEM_INFO* item, short targetAngle, float rate)
{
	auto* info = GetLaraInfo(item);

	if (!rate)
		return;

	rate = abs(rate);

	if (abs((short)(targetAngle - item->Position.yRot)) > ANGLE(0.1f))
		item->Position.yRot += (short)(targetAngle - item->Position.yRot) / rate;
	else
		item->Position.yRot = targetAngle;
}

void EaseOutLaraHeight(ITEM_INFO* item, int height)
{
	if (height == NO_HEIGHT)
		return;

	// Translate Lara to new height.
	// TODO: This approach may cause undesirable artefacts where an object pushes Lara rapidly up/down a slope or a platform rapidly ascends/descends.
	static constexpr int rate = 50;
	int threshold = std::max(abs(item->Velocity) / 3 * 2, CLICK(1) / 16);
	int sign = std::copysign(1, height);
	
	if (TestEnvironment(ENV_FLAG_SWAMP, item) && height > 0)
		item->Position.yPos += SWAMP_GRAVITY;
	else if (abs(height) > (STEPUP_HEIGHT / 2))		// Outer range.
		item->Position.yPos += rate * sign;
	else if (abs(height) <= (STEPUP_HEIGHT / 2) &&	// Inner range.
		abs(height) >= threshold)
	{
		item->Position.yPos += std::max<int>(abs(height / 2.75), threshold) * sign;
	}
	else
		item->Position.yPos += height;
}

// TODO: Some states can't make the most of this function due to missing step up/down animations.
// Try implementing leg IK as a substitute to make step animations obsolete. @Sezz 2021.10.09
void DoLaraStep(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		if (TestLaraStepUp(item, coll))
		{
			item->TargetState = LS_STEP_UP;
			if (GetChange(item, &g_Level.Anims[item->AnimNumber]))
			{
				item->Position.yPos += coll->Middle.Floor;
				return;
			}
		}
		else if (TestLaraStepDown(item, coll))
		{
			item->TargetState = LS_STEP_DOWN;
			if (GetChange(item, &g_Level.Anims[item->AnimNumber]))
			{
				item->Position.yPos += coll->Middle.Floor;
				return;
			}
		}
	}

	EaseOutLaraHeight(item, coll->Middle.Floor);
}

void DoLaraMonkeyStep(ITEM_INFO* item, COLL_INFO* coll)
{
	EaseOutLaraHeight(item, coll->Middle.Ceiling);
}

// TODO: Doesn't always work on bridges.
void DoLaraCrawlToHangSnap(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.ForwardAngle = item->Position.yRot + ANGLE(180.0f);
	GetCollisionInfo(coll, item);
	SnapItemToLedge(item, coll);
	MoveItem(item, item->Position.yRot, -LARA_RAD_CRAWL);
	item->Position.yRot += ANGLE(180.0f);
	LaraResetGravityStatus(item, coll);
}

void DoLaraCrawlFlex(ITEM_INFO* item, COLL_INFO* coll, short maxAngle, short rate)
{
	auto* info = GetLaraInfo(item);

	if (!item->Velocity)
		return;

	int sign = copysign(1, maxAngle);
	rate = copysign(rate, maxAngle);

	info->ExtraTorsoRot.zRot += std::min(abs(rate), abs(maxAngle - info->ExtraTorsoRot.zRot) / 6) * sign;

	if (!(TrInput & IN_LOOK) &&
		item->ActiveState != LS_CRAWL_BACK)
	{
		info->ExtraHeadRot.zRot = info->ExtraTorsoRot.zRot / 2;
		info->ExtraHeadRot.yRot = info->ExtraHeadRot.zRot;
	}
}

void DoLaraFallDamage(ITEM_INFO* item)
{
	// TODO: Demagic more of these numbers.
	int landingVelocity = item->VerticalVelocity - 140;

	if (landingVelocity > 0)
	{
		if (landingVelocity <= 14)
			item->HitPoints -= LARA_HEALTH_MAX * pow(landingVelocity, 2) / 196;
		else
			item->HitPoints = 0;
	}
}

void DoLaraTightropeBalance(ITEM_INFO* item)
{
	auto* info = GetLaraInfo(item);
	const int factor = ((info->Control.TightropeControl.TimeOnTightrope >> 7) & 0xFF) * 128;

	if (TrInput & IN_LEFT)
		info->Control.TightropeControl.Balance += ANGLE(1.4f);
	if (TrInput & IN_RIGHT)
		info->Control.TightropeControl.Balance -= ANGLE(1.4f);

	if (info->Control.TightropeControl.Balance < 0)
	{
		info->Control.TightropeControl.Balance -= factor;
		if (info->Control.TightropeControl.Balance <= -ANGLE(45.0f))
			info->Control.TightropeControl.Balance = ANGLE(45.0f);

	}
	else if (info->Control.TightropeControl.Balance > 0)
	{
		info->Control.TightropeControl.Balance += factor;
		if (info->Control.TightropeControl.Balance >= ANGLE(45.0f))
			info->Control.TightropeControl.Balance = ANGLE(45.0f);
	}
	else
		info->Control.TightropeControl.Balance = GetRandomControl() & 1 ? -1 : 1;
}

void DoLaraTightropeLean(ITEM_INFO* item)
{
	auto* info = GetLaraInfo(item);

	item->Position.zRot = info->Control.TightropeControl.Balance / 4;
	info->ExtraTorsoRot.zRot = -info->Control.TightropeControl.Balance;
}

void DoLaraTightropeBalanceRegen(ITEM_INFO* item)
{
	auto* info = GetLaraInfo(item);

	if (info->Control.TightropeControl.TimeOnTightrope <= 32)
		info->Control.TightropeControl.TimeOnTightrope = 0;
	else
		info->Control.TightropeControl.TimeOnTightrope -= 32;

	if (info->Control.TightropeControl.Balance > 0)
	{
		if (info->Control.TightropeControl.Balance <= ANGLE(0.75f))
			info->Control.TightropeControl.Balance = 0;
		else
			info->Control.TightropeControl.Balance -= ANGLE(0.75f);
	}

	if (info->Control.TightropeControl.Balance < 0)
	{
		if (info->Control.TightropeControl.Balance >= -ANGLE(0.75f))
			info->Control.TightropeControl.Balance = 0;
		else
			info->Control.TightropeControl.Balance += ANGLE(0.75f);
	}
}


LaraInfo*& GetLaraInfo(ITEM_INFO* item)
{
	if (item->ObjectNumber != ID_LARA)
	{
		TENLog(std::string("Attempted to fetch LaraInfo data from item with object ID ") + std::to_string(item->ObjectNumber), LogLevel::Warning);
		
		auto* firstLaraItem = FindItem(ID_LARA);
		return (LaraInfo*&)firstLaraItem->Data;
	}

	return (LaraInfo*&)item->Data;
}

short GetLaraSlideDirection(ITEM_INFO* item, COLL_INFO* coll)
{
	short direction = item->Position.yRot;

	//if (g_GameFlow->Animations.SlideExtended)
	//{
	//	// TODO: Get true slope direction.
	//}
	//else
	{
		if (coll->FloorTiltX > 2)
			direction = -ANGLE(90.0f);
		else if (coll->FloorTiltX < -2)
			direction = ANGLE(90.0f);

		if (coll->FloorTiltZ > 2 && coll->FloorTiltZ > abs(coll->FloorTiltX))
			direction = ANGLE(180.0f);
		else if (coll->FloorTiltZ < -2 && -coll->FloorTiltZ > abs(coll->FloorTiltX))
			direction = 0;
	}

	return direction;
}

void SetLaraJumpDirection(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	if (TrInput & IN_FORWARD &&
		TestLaraJumpForward(item, coll))
	{
		info->Control.JumpDirection = JumpDirection::Forward;
	}
	else if (TrInput & IN_BACK &&
		TestLaraJumpBack(item, coll))
	{
		info->Control.JumpDirection = JumpDirection::Back;
	}
	else if (TrInput & IN_LEFT &&
		TestLaraJumpLeft(item, coll))
	{
		info->Control.JumpDirection = JumpDirection::Left;
	}
	else if (TrInput & IN_RIGHT &&
		TestLaraJumpRight(item, coll))
	{
		info->Control.JumpDirection = JumpDirection::Right;
	}
	else if (TestLaraJumpUp(item, coll)) [[likely]]
		info->Control.JumpDirection = JumpDirection::Up;
	else
		info->Control.JumpDirection = JumpDirection::None;
}

// TODO: Add a timeout? Imagine a small, sad rain cloud with the properties of a ceiling following Lara overhead.
// RunJumpQueued will never reset, and when the sad cloud flies away after an indefinite amount of time, Lara will jump. @Sezz 2022.01.22
void SetLaraRunJumpQueue(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	int y = item->Position.yPos;
	int dist = WALL_SIZE;
	auto probe = GetCollisionResult(item, item->Position.yRot, dist, -coll->Setup.Height);

	if ((TestLaraRunJumpForward(item, coll) ||													// Area close ahead is permissive...
			(probe.Position.Ceiling - y) < -(coll->Setup.Height + (LARA_HEADROOM * 0.8f)) ||		// OR ceiling height is permissive far ahead
			(probe.Position.Floor - y) >= CLICK(0.5f)) &&											// OR there is a drop below far ahead.
		probe.Position.Floor != NO_HEIGHT)
	{
		info->Control.RunJumpQueued = IsRunJumpQueueableState((LaraState)item->TargetState);
	}
	else
		info->Control.RunJumpQueued = false;
}

void SetLaraVault(ITEM_INFO* item, COLL_INFO* coll, VaultTestResult vaultResult)
{
	auto* info = GetLaraInfo(item);

	info->ProjectedFloorHeight = vaultResult.Height;
	info->Control.HandStatus = vaultResult.SetBusyHands ? HandStatus::Busy : info->Control.HandStatus;
	info->Control.TurnRate = 0;

	// Disable smooth angle adjustment for now.
	//info->Control.ApproachTargetAngle = vaultResult.ApproachLedgeAngle;

	if (vaultResult.SnapToLedge)
		SnapItemToLedge(item, coll, 0.2f);
}

void SetLaraLand(ITEM_INFO* item, COLL_INFO* coll)
{
	item->Velocity = 0;
	item->VerticalVelocity = 0;
	//item->Airborne = false; // TODO: Removing this addresses an unusual landing bug Core had worked around in an obscure way. I'd like to find a proper solution someday. @Sezz 2022.02.18

	LaraSnapToHeight(item, coll);
}

void SetLaraFallState(ITEM_INFO* item)
{
	SetAnimation(item, LA_FALL_START);
	item->VerticalVelocity = 0;
	item->Airborne = true;
}

void SetLaraFallBackState(ITEM_INFO* item)
{
	SetAnimation(item, LA_FALL_BACK);
	item->VerticalVelocity = 0;
	item->Airborne = true;
}

void SetLaraMonkeyFallState(ITEM_INFO* item)
{
	// HACK: Disallow release during 180 turn action.
	if (item->ActiveState == LS_MONKEY_TURN_180)
		return;

	SetAnimation(item, LA_MONKEY_TO_FREEFALL);
	SetLaraMonkeyRelease(item);
}

void SetLaraMonkeyRelease(ITEM_INFO* item)
{
	auto* info = GetLaraInfo(item);

	item->Velocity = 2;
	item->VerticalVelocity = 1;
	item->Airborne = true;
	info->Control.HandStatus = HandStatus::Free;
}

void SetLaraSlideState(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	short direction = GetLaraSlideDirection(item, coll);
	short delta = direction - item->Position.yRot;
	static short oldAngle = 1; // TODO: Remove this.

	ShiftItem(item, coll);

	if (delta < -ANGLE(90.0f) || delta > ANGLE(90.0f))
	{
		if (item->ActiveState == LS_SLIDE_BACK && oldAngle == direction)
			return;

		SetAnimation(item, LA_SLIDE_BACK_START);
		item->Position.yRot = direction + ANGLE(180.0f);
	}
	else
	{
		if (item->ActiveState == LS_SLIDE_FORWARD && oldAngle == direction)
			return;

		SetAnimation(item, LA_SLIDE_FORWARD);
		item->Position.yRot = direction;
	}

	LaraSnapToHeight(item, coll);
	info->Control.MoveAngle = direction;
	oldAngle = direction;
}

void SetCornerAnimation(ITEM_INFO* item, COLL_INFO* coll, bool flip)
{
	auto* info = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		SetAnimation(item, LA_FALL_START);

		item->Airborne = true;
		item->Velocity = 2;
		item->Position.yPos += STEP_SIZE;
		item->VerticalVelocity = 1;

		info->Control.HandStatus = HandStatus::Free;

		item->Position.yRot += info->NextCornerPos.yRot / 2;
		return;
	}

	if (flip)
	{
		if (info->Control.IsClimbingLadder)
			SetAnimation(item, LA_LADDER_IDLE);
		else
			SetAnimation(item, LA_HANG_IDLE);

		coll->Setup.OldPosition.x = item->Position.xPos = info->NextCornerPos.xPos;
		coll->Setup.OldPosition.y = item->Position.yPos = info->NextCornerPos.yPos;
		coll->Setup.OldPosition.z = item->Position.zPos = info->NextCornerPos.zPos;
		item->Position.yRot = info->NextCornerPos.yRot;
	}
}

void ResetLaraLean(ITEM_INFO* item, float rate, bool resetRoll, bool resetPitch)
{
	if (!rate)
		return;

	rate = abs(rate);

	if (resetPitch)
	{
		if (abs(item->Position.xRot) > ANGLE(0.1f))
			item->Position.xRot += item->Position.xRot / -rate;
		else
			item->Position.xRot = 0;
	}

	if (resetRoll)
	{
		if (abs(item->Position.zRot) > ANGLE(0.1f))
			item->Position.zRot += item->Position.zRot / -rate;
		else
			item->Position.zRot = 0;
	}
}

void ResetLaraFlex(ITEM_INFO* item, float rate)
{
	auto* info = GetLaraInfo(item);

	if (!rate)
		return;

	rate = abs(rate);

	// Reset head.
	if (abs(info->ExtraHeadRot.xRot) > ANGLE(0.1f))
		info->ExtraHeadRot.xRot += info->ExtraHeadRot.xRot / -rate;
	else
		info->ExtraHeadRot.xRot = 0;

	if (abs(info->ExtraHeadRot.yRot) > ANGLE(0.1f))
		info->ExtraHeadRot.yRot += info->ExtraHeadRot.yRot / -rate;
	else
		info->ExtraHeadRot.yRot = 0;

	if (abs(info->ExtraHeadRot.zRot) > ANGLE(0.1f))
		info->ExtraHeadRot.zRot += info->ExtraHeadRot.zRot / -rate;
	else
		info->ExtraHeadRot.zRot = 0;

	// Reset torso.
	if (abs(info->ExtraTorsoRot.xRot) > ANGLE(0.1f))
		info->ExtraTorsoRot.xRot += info->ExtraTorsoRot.xRot / -rate;
	else
		info->ExtraTorsoRot.xRot = 0;

	if (abs(info->ExtraTorsoRot.yRot) > ANGLE(0.1f))
		info->ExtraTorsoRot.yRot += info->ExtraTorsoRot.yRot / -rate;
	else
		info->ExtraTorsoRot.yRot = 0;

	if (abs(info->ExtraTorsoRot.zRot) > ANGLE(0.1f))
		info->ExtraTorsoRot.zRot += info->ExtraTorsoRot.zRot / -rate;
	else
		info->ExtraTorsoRot.zRot = 0;
}

void HandleLaraMovementParameters(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	// Reset running jump timer.
	if (!IsRunJumpCountableState((LaraState)item->ActiveState))
		info->Control.Count.RunJump = 0;

	// Reset running jump action queue.
	if (!IsRunJumpQueueableState((LaraState)item->ActiveState))
		info->Control.RunJumpQueued = false;

	// Increment/reset AFK pose timer.
	if (info->Control.Count.Pose < LARA_POSE_TIME &&
		TestLaraPose(item, coll) &&
		!(TrInput & (IN_WAKE | IN_LOOK)) &&
		g_GameFlow->Animations.Pose)
	{
		info->Control.Count.Pose++;
	}
	else
		info->Control.Count.Pose = 0;

	// Reset lean.
	if (!info->Control.IsMoving || (info->Control.IsMoving && !(TrInput & (IN_LEFT | IN_RIGHT))))
		ResetLaraLean(item, 6);

	// Reset crawl flex.
	if (!(TrInput & IN_LOOK) &&
		coll->Setup.Height > LARA_HEIGHT - LARA_HEADROOM &&
		(!item->Velocity || (item->Velocity && !(TrInput & (IN_LEFT | IN_RIGHT)))))
	{
		ResetLaraFlex(item, 12);
	}

	// Reset turn rate.
	int sign = copysign(1, info->Control.TurnRate);
	if (abs(info->Control.TurnRate) > ANGLE(2.0f))
		info->Control.TurnRate -= ANGLE(2.0f) * sign;
	else if (abs(info->Control.TurnRate) > ANGLE(0.5f))
		info->Control.TurnRate -= ANGLE(0.5f) * sign;
	else
		info->Control.TurnRate = 0;
	item->Position.yRot += info->Control.TurnRate;
}

bool HandleLaraVehicle(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	if (info->Vehicle != NO_ITEM)
	{
		switch (g_Level.Items[info->Vehicle].ObjectNumber)
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
			SubControl(item, coll);
			break;

		case ID_MINECART:
			MineCartControl();
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
