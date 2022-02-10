#include "framework.h"
#include "Game/Lara/lara_helpers.h"

#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/lara_collide.h"
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

	if (coll->CollisionType == CT_LEFT || coll->CollisionType == CT_RIGHT)
		item->Position.zRot += std::min(rate, (short)(abs((maxAngle * 3) / 5 - item->Position.zRot) / 3)) * sign;
	else
		item->Position.zRot += std::min(rate, (short)(abs(maxAngle - item->Position.zRot) / 3)) * sign;
}

void EaseOutLaraHeight(ITEM_INFO* item, int height)
{
	if (height == NO_HEIGHT)
		return;

	// Translate Lara to new height.
	// TODO: This approach may cause undesirable artefacts where an object pushes Lara rapidly up/down a slope or a platform rapidly ascends/descends.
	constexpr int rate = 50;
	int threshold = std::max(abs(item->Velocity) / 3 * 2, STEP_SIZE / 16);
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
	LaraInfo*& info = item->Data;

	if (!item->Velocity)
		return;

	int sign = copysign(1, maxAngle);
	rate = copysign(rate, maxAngle);

	info->Control.ExtraTorsoRot.zRot += std::min(abs(rate), abs(maxAngle - info->Control.ExtraTorsoRot.zRot) / 6) * sign;

	if (!(TrInput & IN_LOOK) &&
		item->ActiveState != LS_CRAWL_BACK)
	{
		info->Control.ExtraHeadRot.zRot = info->Control.ExtraTorsoRot.zRot / 2;
		info->Control.ExtraHeadRot.yRot = info->Control.ExtraHeadRot.zRot;
	}
}

void DoLaraFallDamage(ITEM_INFO* item)
{
	// TODO: Demagic more of these numbers.
	int landSpeed = item->VerticalVelocity - 140;

	if (landSpeed > 0)
	{
		if (landSpeed <= 14)
			item->HitPoints -= LARA_HEALTH_MAX * pow(landSpeed, 2) / 196;
		else
			item->HitPoints = 0;
	}
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
			direction = ANGLE(0.0f);
	}

	return direction;
}

void SetLaraJumpDirection(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

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
	LaraInfo*& info = item->Data;

	int y = item->Position.yPos;
	int dist = WALL_SIZE;
	auto probe = GetCollisionResult(item, item->Position.yRot, dist, -coll->Setup.Height);

	if ((TestLaraRunJumpForward(item, coll) ||													// Area close ahead is permissive...
			(probe.Position.Ceiling - y) < -(coll->Setup.Height + (LARA_HEADROOM * 0.8f)) ||		// OR ceiling height is permissive far ahead
			(probe.Position.Floor - y) >= CLICK(0.5f)) &&											// OR there is a drop below far ahead.
		probe.Position.Floor != NO_HEIGHT)
	{
		info->Control.RunJumpQueued = IsRunJumpQueueableState((LARA_STATE)item->TargetState);
	}
	else
		info->Control.RunJumpQueued = false;
}

void SetLaraVault(ITEM_INFO* item, COLL_INFO* coll, VaultTestResult vaultResult)
{
	LaraInfo*& info = item->Data;

	info->Control.ProjectedFloorHeight = vaultResult.Height;
	info->gunStatus = vaultResult.SetBusyHands ? LG_HANDS_BUSY : info->gunStatus;
	info->Control.TurnRate = 0;

	if (vaultResult.SnapToLedge)
		SnapItemToLedge(item, coll);
}

void SetLaraLand(ITEM_INFO* item, COLL_INFO* coll)
{
	item->Velocity = 0;
	item->VerticalVelocity = 0;
	item->Airborne = false;

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
	// Hack.
	if (item->ActiveState == LS_MONKEY_TURN_180)
		return;

	SetAnimation(item, LA_MONKEY_TO_FREEFALL);
	SetLaraMonkeyRelease(item);
}

void SetLaraMonkeyRelease(ITEM_INFO* item)
{
	LaraInfo*& info = item->Data;

	item->Velocity = 2;
	item->VerticalVelocity = 1;
	item->Airborne = true;
	info->gunStatus = LG_HANDS_FREE;
}

void SetLaraSlideState(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	short direction = GetLaraSlideDirection(item, coll);
	short delta = direction - item->Position.yRot;
	static short oldAngle = 1;

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

void ResetLaraFlex(ITEM_INFO* item, float rate)
{
	LaraInfo*& info = item->Data;

	// Reset head.
	if (abs(info->Control.ExtraHeadRot.xRot) > ANGLE(0.1f))
		info->Control.ExtraHeadRot.xRot += info->Control.ExtraHeadRot.xRot / -rate;
	else
		info->Control.ExtraHeadRot.xRot = 0;

	if (abs(info->Control.ExtraHeadRot.yRot) > ANGLE(0.1f))
		info->Control.ExtraHeadRot.yRot += info->Control.ExtraHeadRot.yRot / -rate;
	else
		info->Control.ExtraHeadRot.yRot = 0;

	if (abs(info->Control.ExtraHeadRot.zRot) > ANGLE(0.1f))
		info->Control.ExtraHeadRot.zRot += info->Control.ExtraHeadRot.zRot / -rate;
	else
		info->Control.ExtraHeadRot.zRot = 0;

	// Reset torso.
	if (abs(info->Control.ExtraTorsoRot.zRot) > ANGLE(0.1f))
		info->Control.ExtraTorsoRot.zRot += info->Control.ExtraTorsoRot.zRot / -rate;
	else
		info->Control.ExtraTorsoRot.zRot = 0;

	if (abs(info->Control.ExtraTorsoRot.yRot) > ANGLE(0.1f))
		info->Control.ExtraTorsoRot.yRot += info->Control.ExtraTorsoRot.yRot / -rate;
	else
		info->Control.ExtraTorsoRot.yRot = 0;

	if (abs(info->Control.ExtraTorsoRot.zRot) > ANGLE(0.1f))
		info->Control.ExtraTorsoRot.zRot += info->Control.ExtraTorsoRot.zRot / -rate;
	else
		info->Control.ExtraTorsoRot.zRot = 0;
}

void HandleLaraMovementParameters(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	// Reset running jump timer.
	if (!IsRunJumpCountableState((LARA_STATE)item->ActiveState))
		info->Control.RunJumpCount = 0;

	// Reset running jump action queue.
	if (!IsRunJumpQueueableState((LARA_STATE)item->ActiveState))
		info->Control.RunJumpQueued = false;

	// Reset projected height value used by step function.
	//if (!IsVaultState((LARA_STATE)item->ActiveState))
	//	info->Control.ProjectedFloorHeight = NO_HEIGHT;

	// Reset calculated auto jump velocity.
	//if (item->ActiveState != LS_AUTO_JUMP)
	//	info->Control.CalculatedJumpVelocity = 0;

	// Increment/reset AFK pose timer.
	if (info->Control.PoseCount < LARA_POSE_TIME &&
		TestLaraPose(item, coll) &&
		!(TrInput & (IN_WAKE | IN_LOOK)) &&
		g_GameFlow->Animations.Pose)
	{
		info->Control.PoseCount++;
	}
	else
		info->Control.PoseCount = 0;

	// Reset lean.
	if (!info->Control.IsMoving || (info->Control.IsMoving && !(TrInput & (IN_LEFT | IN_RIGHT))))
	{
		if (abs(item->Position.zRot) > ANGLE(0.1f))
			item->Position.zRot += item->Position.zRot / -6;
		else
			item->Position.zRot = 0;
	}

	// Temp.
	if (abs(item->Position.xRot) > ANGLE(0.1f))
		item->Position.xRot += item->Position.xRot / -6;
	else
		item->Position.xRot = 0;

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

void HandleLaraVehicle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	if (info->Vehicle != NO_ITEM)
	{
		switch (g_Level.Items[info->Vehicle].ObjectNumber)
		{
		case ID_QUAD:
			if (QuadBikeControl(item, coll))
				return;
			break;

		case ID_JEEP:
			if (JeepControl())
				return;
			break;

		case ID_MOTORBIKE:
			if (MotorbikeControl())
				return;
			break;

		case ID_KAYAK:
			if (KayakControl(item))
				return;
			break;

		case ID_SNOWMOBILE:
			if (SkidooControl(item, coll))
				return;
			break;

		case ID_UPV:
			if (SubControl(item, coll))
				return;
			break;

		case ID_MINECART:
			if (MineCartControl())
				return;
			break;

		case ID_BIGGUN:
			if (BigGunControl(item, coll))
				return;
			break;

		// Boats are processed like normal items in loop.
		default:
			LaraGun(item);
			return;
		}
	}
}
