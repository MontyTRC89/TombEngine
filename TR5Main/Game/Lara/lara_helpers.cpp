#include "framework.h"
#include "Game/Lara/lara_helpers.h"

#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/lara_collide.h"
#include "Scripting/GameFlowScript.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

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

// TODO: Some states can't make the most of this function due to missing step up/down animations.
// Try implementing leg IK as a substitute to make step animations obsolete. @Sezz 2021.10.09
void DoLaraStep(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		if (TestLaraStepUp(item, coll))
		{
			item->goalAnimState = LS_STEP_UP;
			if (GetChange(item, &g_Level.Anims[item->animNumber]))
			{
				item->pos.yPos += coll->Middle.Floor;
				return;
			}
		}
		else if (TestLaraStepDown(item, coll))
		{
			item->goalAnimState = LS_STEP_DOWN;
			if (GetChange(item, &g_Level.Anims[item->animNumber]))
			{
				item->pos.yPos += coll->Middle.Floor;
				return;
			}
		}
	}

	// Height difference is below threshold for step dispatch OR step animation doesn't exist; translate Lara to new floor height.
	// TODO: This approach might cause underirable artefacts where an object pushes Lara rapidly up/down a slope or a platform rapidly ascends/descends.
	constexpr int rate = 50;
	int threshold = std::max(abs(item->speed) / 3 * 2, STEP_SIZE / 16);
	int sign = std::copysign(1, coll->Middle.Floor);
	
	if (TestEnvironment(ENV_FLAG_SWAMP, item) && coll->Middle.Floor > 0)
		item->pos.yPos += SWAMP_GRAVITY;
	else if (abs(coll->Middle.Floor) > (STEPUP_HEIGHT / 2))		// Outer range.
		item->pos.yPos += rate * sign;
	else if (abs(coll->Middle.Floor) <= (STEPUP_HEIGHT / 2) &&	// Inner range.
		abs(coll->Middle.Floor) >= threshold)
	{
		item->pos.yPos += std::max<int>(abs(coll->Middle.Floor / 2.75), threshold) * sign;
	}
	else
		item->pos.yPos += coll->Middle.Floor;
}

void DoLaraMonkeyStep(ITEM_INFO* item, COLL_INFO* coll)
{
	constexpr int rate = 50;
	int threshold = std::max<int>(abs(item->speed) / 3 * 2, CLICK(1.25f) / 16);
	int sign = std::copysign(1, coll->Middle.Ceiling);

	if (abs(coll->Middle.Ceiling) > (CLICK(1.25f) / 2))			// Outer range.
		item->pos.yPos += rate * sign;
	else if (abs(coll->Middle.Ceiling) <= (CLICK(1.25f) / 2) &&	// Inner range.
		abs(coll->Middle.Ceiling) >= threshold)
	{
		item->pos.yPos += std::max<int>(abs(coll->Middle.Ceiling / 2.75), threshold) * sign;
	}
	else
		item->pos.yPos += coll->Middle.Ceiling;
}

void DoLaraCrawlVault(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	ResetLaraFlex(item);

	if (TestLaraCrawlExitDownStep(item, coll))
	{
		if (TrInput & IN_CROUCH && TestLaraCrawlDownStep(item, coll))
			item->goalAnimState = LS_CRAWL_STEP_DOWN;
		else [[likely]]
			item->goalAnimState = LS_CRAWL_EXIT_STEP_DOWN;

		return;
	}

	if (TestLaraCrawlExitJump(item, coll))
	{
		if (TrInput & IN_WALK)
			item->goalAnimState = LS_CRAWL_EXIT_FLIP;
		else [[likely]]
			item->goalAnimState = LS_CRAWL_EXIT_JUMP;

		return;
	}

	if (TestLaraCrawlUpStep(item, coll))
	{
		item->goalAnimState = LS_CRAWL_STEP_UP;
		return;
	}

	if (TestLaraCrawlDownStep(item, coll))
	{
		item->goalAnimState = LS_CRAWL_STEP_DOWN;
		return;
	}
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

	info->torsoZrot += std::min(abs(rate), abs(maxAngle - info->torsoZrot) / 6) * sign;

	if (!(TrInput & IN_LOOK) &&
		item->currentAnimState != LS_CRAWL_BACK)
	{
		info->headZrot = info->torsoZrot / 2;
		info->headYrot = info->headZrot;
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
		info->jumpDirection = LaraJumpDirection::Forward;
	}
	else if (TrInput & IN_BACK &&
		TestLaraJumpBack(item, coll))
	{
		info->jumpDirection = LaraJumpDirection::Back;
	}
	else if (TrInput & IN_LEFT &&
		TestLaraJumpLeft(item, coll))
	{
		info->jumpDirection = LaraJumpDirection::Left;
	}
	else if (TrInput & IN_RIGHT &&
		TestLaraJumpRight(item, coll))
	{
		info->jumpDirection = LaraJumpDirection::Right;
	}
	else if (TestLaraJumpUp(item, coll)) [[likely]]
		info->jumpDirection = LaraJumpDirection::Up;
	else
		info->jumpDirection = LaraJumpDirection::None;
}

// TODO: Add a timeout? Imagine a small, sad rain cloud with the properties of a ceiling following Lara overhead.
// runJumpQueued will never reset, and when the sad cloud flies away, Lara will jump. @Sezz 2022.01.22
void SetLaraRunJumpQueue(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	int y = item->pos.yPos;
	int dist = WALL_SIZE;
	auto probe = GetCollisionResult(item, item->pos.yRot, dist, -coll->Setup.Height);

	if ((TestLaraRunJumpForward(item, coll) ||													// Area close ahead is permissive...
			(probe.Position.Ceiling - y) < -(coll->Setup.Height + (LARA_HEADROOM * 0.7f)) ||	// OR ceiling height is permissive far ahead
			(probe.Position.Floor - y) >= CLICK(0.5f)) &&										// OR there is a drop below far ahead.
		probe.Position.Floor != NO_HEIGHT)
	{
		info->runJumpQueued = (item->goalAnimState == LS_RUN_FORWARD);
	}
	else
		info->runJumpQueued = false;
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
	LaraInfo*& info = item->data;

	// Hack.
	if (item->currentAnimState == LS_MONKEY_TURN_180)
		return;

	SetAnimation(item, LA_MONKEY_TO_FREEFALL);
	SetLaraMonkeyRelease(item);
}

void SetLaraMonkeyRelease(ITEM_INFO* item)
{
	LaraInfo*& info = item->data;

	info->gunStatus = LG_HANDS_FREE;
	item->speed = 2;
	item->fallspeed = 1;
	item->airborne = true;
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
		if (item->currentAnimState == LS_SLIDE_BACK && oldAngle == direction)
			return;

		SetAnimation(item, LA_SLIDE_BACK_START);
		item->pos.yRot = direction + ANGLE(180.0f);
	}
	else
	{
		if (item->currentAnimState == LS_SLIDE_FORWARD && oldAngle == direction)
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
	if (abs(info->headXrot) > ANGLE(0.1f))
		info->headXrot += info->headXrot / -rate;
	else
		info->headXrot = 0;

	if (abs(info->headYrot) > ANGLE(0.1f))
		info->headYrot += info->headYrot / -rate;
	else
		info->headYrot = 0;

	if (abs(info->headZrot) > ANGLE(0.1f))
		info->headZrot += info->headZrot / -rate;
	else
		info->headZrot = 0;

	// Reset torso.
	if (abs(info->torsoXrot) > ANGLE(0.1f))
		info->torsoXrot += info->torsoXrot / -rate;
	else
		info->torsoXrot = 0;

	if (abs(info->torsoYrot) > ANGLE(0.1f))
		info->torsoYrot += info->torsoYrot / -rate;
	else
		info->torsoYrot = 0;

	if (abs(info->torsoZrot) > ANGLE(0.1f))
		info->torsoZrot += info->torsoZrot / -rate;
	else
		info->torsoZrot = 0;
}

void HandleLaraMovementParameters(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	// Reset running jump timer.
	if (item->currentAnimState != LS_RUN_FORWARD &&
		item->currentAnimState != LS_WALK_FORWARD &&
		item->currentAnimState != LS_JUMP_FORWARD &&
		item->currentAnimState != LS_SPRINT &&
		item->currentAnimState != LS_SPRINT_DIVE)
	{
		info->runJumpCount = 0;
	}

	// Reset running jump action queue.
	if (item->currentAnimState != LS_RUN_FORWARD)
		info->runJumpQueued = false;

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
