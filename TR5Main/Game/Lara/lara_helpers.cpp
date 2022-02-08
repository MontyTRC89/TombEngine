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
	if (!item->speed)
		return;

	int sign = copysign(1, maxAngle);

	if (coll->CollisionType == CT_LEFT || coll->CollisionType == CT_RIGHT)
		item->pos.zRot += std::min(rate, (short)(abs((maxAngle * 3) / 5 - item->pos.zRot) / 3)) * sign;
	else
		item->pos.zRot += std::min(rate, (short)(abs(maxAngle - item->pos.zRot) / 3)) * sign;
}

void EaseOutLaraHeight(ITEM_INFO* item, int height)
{
	if (height == NO_HEIGHT)
		return;

	// Translate Lara to new height.
	// TODO: This approach may cause undesirable artefacts where an object pushes Lara rapidly up/down a slope or a platform rapidly ascends/descends.
	constexpr int rate = 50;
	int threshold = std::max(abs(item->speed) / 3 * 2, STEP_SIZE / 16);
	int sign = std::copysign(1, height);
	
	if (TestEnvironment(ENV_FLAG_SWAMP, item) && height > 0)
		item->pos.yPos += SWAMP_GRAVITY;
	else if (abs(height) > (STEPUP_HEIGHT / 2))		// Outer range.
		item->pos.yPos += rate * sign;
	else if (abs(height) <= (STEPUP_HEIGHT / 2) &&	// Inner range.
		abs(height) >= threshold)
	{
		item->pos.yPos += std::max<int>(abs(height / 2.75), threshold) * sign;
	}
	else
		item->pos.yPos += height;
}

// TODO: Some states can't make the most of this function due to missing step up/down animations.
// Try implementing leg IK as a substitute to make step animations obsolete. @Sezz 2021.10.09
void DoLaraStep(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		if (TestLaraStepUp(item, coll))
		{
			item->targetState = LS_STEP_UP;
			if (GetChange(item, &g_Level.Anims[item->animNumber]))
			{
				item->pos.yPos += coll->Middle.Floor;
				return;
			}
		}
		else if (TestLaraStepDown(item, coll))
		{
			item->targetState = LS_STEP_DOWN;
			if (GetChange(item, &g_Level.Anims[item->animNumber]))
			{
				item->pos.yPos += coll->Middle.Floor;
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
	coll->Setup.ForwardAngle = item->pos.yRot + ANGLE(180.0f);
	GetCollisionInfo(coll, item);
	SnapItemToLedge(item, coll);
	MoveItem(item, item->pos.yRot, -LARA_RAD_CRAWL);
	item->pos.yRot += ANGLE(180.0f);
	LaraResetGravityStatus(item, coll);
}

void DoLaraCrawlFlex(ITEM_INFO* item, COLL_INFO* coll, short maxAngle, short rate)
{
	LaraInfo*& info = item->data;

	if (!item->speed)
		return;

	int sign = copysign(1, maxAngle);
	rate = copysign(rate, maxAngle);

	info->extraTorsoRot.z += std::min(abs(rate), abs(maxAngle - info->extraTorsoRot.z) / 6) * sign;

	if (!(TrInput & IN_LOOK) &&
		item->activeState != LS_CRAWL_BACK)
	{
		info->extraHeadRot.z = info->extraTorsoRot.z / 2;
		info->extraHeadRot.y = info->extraHeadRot.z;
	}
}

void DoLaraFallDamage(ITEM_INFO* item)
{
	// TODO: Demagic more of these numbers.
	int landSpeed = item->fallspeed - 140;

	if (landSpeed > 0)
	{
		if (landSpeed <= 14)
			item->hitPoints -= LARA_HEALTH_MAX * pow(landSpeed, 2) / 196;
		else
			item->hitPoints = 0;
	}
}

short GetLaraSlideDirection(ITEM_INFO* item, COLL_INFO* coll)
{
	short direction = item->pos.yRot;

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
	LaraInfo*& info = item->data;

	if (TrInput & IN_FORWARD &&
		TestLaraJumpForward(item, coll))
	{
		info->jumpDirection = JumpDirection::Forward;
	}
	else if (TrInput & IN_BACK &&
		TestLaraJumpBack(item, coll))
	{
		info->jumpDirection = JumpDirection::Back;
	}
	else if (TrInput & IN_LEFT &&
		TestLaraJumpLeft(item, coll))
	{
		info->jumpDirection = JumpDirection::Left;
	}
	else if (TrInput & IN_RIGHT &&
		TestLaraJumpRight(item, coll))
	{
		info->jumpDirection = JumpDirection::Right;
	}
	else if (TestLaraJumpUp(item, coll)) [[likely]]
		info->jumpDirection = JumpDirection::Up;
	else
		info->jumpDirection = JumpDirection::NoDirection;
}

// TODO: Add a timeout? Imagine a small, sad rain cloud with the properties of a ceiling following Lara overhead.
// runJumpQueued will never reset, and when the sad cloud flies away after an indefinite amount of time, Lara will jump. @Sezz 2022.01.22
void SetLaraRunJumpQueue(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	int y = item->pos.yPos;
	int dist = WALL_SIZE;
	auto probe = GetCollisionResult(item, item->pos.yRot, dist, -coll->Setup.Height);

	if ((TestLaraRunJumpForward(item, coll) ||													// Area close ahead is permissive...
			(probe.Position.Ceiling - y) < -(coll->Setup.Height + (LARA_HEADROOM * 0.8f)) ||		// OR ceiling height is permissive far ahead
			(probe.Position.Floor - y) >= CLICK(0.5f)) &&											// OR there is a drop below far ahead.
		probe.Position.Floor != NO_HEIGHT)
	{
		info->runJumpQueued = IsRunJumpQueueableState((LARA_STATE)item->targetState);
	}
	else
		info->runJumpQueued = false;
}

void SetLaraVault(ITEM_INFO* item, COLL_INFO* coll, VaultTestResult vaultResult)
{
	LaraInfo*& info = item->data;

	info->projectedFloorHeight = vaultResult.Height;
	info->gunStatus = vaultResult.SetBusyHands ? LG_HANDS_BUSY : info->gunStatus;
	info->turnRate = 0;

	if (vaultResult.SnapToLedge)
		SnapItemToLedge(item, coll);
}

void SetLaraLand(ITEM_INFO* item, COLL_INFO* coll)
{
	item->speed = 0;
	item->fallspeed = 0;
	item->airborne = false;

	LaraSnapToHeight(item, coll);
}

void SetLaraFallState(ITEM_INFO* item)
{
	SetAnimation(item, LA_FALL_START);
	item->fallspeed = 0;
	item->airborne = true;
}

void SetLaraFallBackState(ITEM_INFO* item)
{
	SetAnimation(item, LA_FALL_BACK);
	item->fallspeed = 0;
	item->airborne = true;
}

void SetLaraMonkeyFallState(ITEM_INFO* item)
{
	// Hack.
	if (item->activeState == LS_MONKEY_TURN_180)
		return;

	SetAnimation(item, LA_MONKEY_TO_FREEFALL);
	SetLaraMonkeyRelease(item);
}

void SetLaraMonkeyRelease(ITEM_INFO* item)
{
	LaraInfo*& info = item->data;

	item->speed = 2;
	item->fallspeed = 1;
	item->airborne = true;
	info->gunStatus = LG_HANDS_FREE;
}

void SetLaraSlideState(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	short direction = GetLaraSlideDirection(item, coll);
	short delta = direction - item->pos.yRot;
	static short oldAngle = 1;

	ShiftItem(item, coll);

	if (delta < -ANGLE(90.0f) || delta > ANGLE(90.0f))
	{
		if (item->activeState == LS_SLIDE_BACK && oldAngle == direction)
			return;

		SetAnimation(item, LA_SLIDE_BACK_START);
		item->pos.yRot = direction + ANGLE(180.0f);
	}
	else
	{
		if (item->activeState == LS_SLIDE_FORWARD && oldAngle == direction)
			return;

		SetAnimation(item, LA_SLIDE_FORWARD);
		item->pos.yRot = direction;
	}

	LaraSnapToHeight(item, coll);
	info->moveAngle = direction;
	oldAngle = direction;
}

void ResetLaraFlex(ITEM_INFO* item, float rate)
{
	LaraInfo*& info = item->data;

	// Reset head.
	if (abs(info->extraHeadRot.x) > ANGLE(0.1f))
		info->extraHeadRot.x += info->extraHeadRot.x / -rate;
	else
		info->extraHeadRot.x = 0;

	if (abs(info->extraHeadRot.y) > ANGLE(0.1f))
		info->extraHeadRot.y += info->extraHeadRot.y / -rate;
	else
		info->extraHeadRot.y = 0;

	if (abs(info->extraHeadRot.z) > ANGLE(0.1f))
		info->extraHeadRot.z += info->extraHeadRot.z / -rate;
	else
		info->extraHeadRot.z = 0;

	// Reset torso.
	if (abs(info->extraTorsoRot.x) > ANGLE(0.1f))
		info->extraTorsoRot.x += info->extraTorsoRot.x / -rate;
	else
		info->extraTorsoRot.x = 0;

	if (abs(info->extraTorsoRot.y) > ANGLE(0.1f))
		info->extraTorsoRot.y += info->extraTorsoRot.y / -rate;
	else
		info->extraTorsoRot.y = 0;

	if (abs(info->extraTorsoRot.z) > ANGLE(0.1f))
		info->extraTorsoRot.z += info->extraTorsoRot.z / -rate;
	else
		info->extraTorsoRot.z = 0;
}

void HandleLaraMovementParameters(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	// Reset running jump timer.
	if (!IsRunJumpCountableState((LARA_STATE)item->activeState))
		info->runJumpCount = 0;

	// Reset running jump action queue.
	if (!IsRunJumpQueueableState((LARA_STATE)item->activeState))
		info->runJumpQueued = false;

	// Reset projected height value used by step function.
	//if (!IsVaultState((LARA_STATE)item->activeState))
	//	info->projectedFloorHeight = NO_HEIGHT;

	// Reset calculated auto jump velocity.
	//if (item->activeState != LS_AUTO_JUMP)
	//	info->calcJumpVelocity = 0;

	// Increment/reset AFK pose timer.
	if (info->poseCount < LARA_POSE_TIME &&
		TestLaraPose(item, coll) &&
		!(TrInput & (IN_WAKE | IN_LOOK)) &&
		g_GameFlow->Animations.Pose)
	{
		info->poseCount++;
	}
	else
		info->poseCount = 0;

	// Reset lean.
	if (!info->isMoving || (info->isMoving && !(TrInput & (IN_LEFT | IN_RIGHT))))
	{
		if (abs(item->pos.zRot) > ANGLE(0.1f))
			item->pos.zRot += item->pos.zRot / -6;
		else
			item->pos.zRot = 0;
	}

	// Temp.
	if (abs(item->pos.xRot) > ANGLE(0.1f))
		item->pos.xRot += item->pos.xRot / -6;
	else
		item->pos.xRot = 0;

	// Reset crawl flex.
	if (!(TrInput & IN_LOOK) &&
		coll->Setup.Height > LARA_HEIGHT - LARA_HEADROOM &&
		(!item->speed || (item->speed && !(TrInput & (IN_LEFT | IN_RIGHT)))))
	{
		ResetLaraFlex(item, 12);
	}

	// Reset turn rate.
	int sign = copysign(1, info->turnRate);
	if (abs(info->turnRate) > ANGLE(2.0f))
		info->turnRate -= ANGLE(2.0f) * sign;
	else if (abs(info->turnRate) > ANGLE(0.5f))
		info->turnRate -= ANGLE(0.5f) * sign;
	else
		info->turnRate = 0;
	item->pos.yRot += info->turnRate;
}

void HandleLaraVehicle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (info->Vehicle != NO_ITEM)
	{
		switch (g_Level.Items[info->Vehicle].objectNumber)
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
