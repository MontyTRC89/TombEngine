#include "framework.h"
#include "control.h"
#include "input.h"
#include "level.h"
#include "setup.h"
#include "sound.h"
#include "camera.h"
#include "lara.h"
#include "lara_collide.h"
#include "lara_tests.h"
#include "lara_helpers.h"
#include "lara_jump.h"
#include "lara_basic.h"
#include "Scripting/GameFlowScript.h"

// -----------------------------
// JUMP
// Control & Collision Functions
// -----------------------------

// State:		LS_JUMP_FORWARD (3)
// Collision:	lara_col_jump_forward()
void lara_as_jump_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (item->hitPoints <= 0)
	{
		if (TestLaraLand(item))
			item->goalAnimState = LS_IDLE;

		return;
	}

	// runJumpCount?
	// Update running jump counter in preparation for possible dispatch soon after landing.
	info->jumpCount++;
	if (info->jumpCount > LARA_JUMP_TIME / 2)
		info->jumpCount = LARA_JUMP_TIME / 2;

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_JUMP_TURN_MAX)
			info->turnRate = -LARA_JUMP_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_JUMP_TURN_MAX)
			info->turnRate = LARA_JUMP_TURN_MAX;
	}

	if (TestLaraLand(item))
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else if (TrInput & IN_FORWARD && !(TrInput & IN_WALK) &&
			info->waterStatus != LW_WADE) [[likely]]
		{
			item->goalAnimState = LS_RUN_FORWARD;
		}
		else
			item->goalAnimState = LS_IDLE;

		return;
	}

	if (TrInput & IN_ACTION &&
		info->gunStatus == LG_HANDS_FREE)
	{
		item->goalAnimState = LS_REACH;
		return;
	}

	if (TrInput & (IN_ROLL | IN_BACK))
	{
		item->goalAnimState = LS_JUMP_ROLL_180;
		return;
	}

	if (TrInput & IN_WALK &&
		info->gunStatus == LG_HANDS_FREE)
	{
		item->goalAnimState = LS_SWANDIVE_START;
		return;
	}

	if (item->fallspeed >= LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
		return;
	}

	item->goalAnimState = LS_JUMP_FORWARD;
}

// State:		LS_JUMP_FORWARD (3)
// Control:		lara_as_jump_forward()
void lara_col_jump_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = (item->speed > 0) ? item->pos.yRot : item->pos.yRot + ANGLE(180.0f);
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);

	LaraDeflectEdgeJump(item, coll);

	if (item->speed < 0)
		info->moveAngle = item->pos.yRot;

	if (item->fallspeed > 0 && (coll->Middle.Floor <= 0 || TestLaraSwamp(item)))
	{
		LaraResetGravityStatus(item, coll);
		LaraSnapToHeight(item, coll);
	}
}

// State:		LS_FREEFALL (9)
// Collision:	lara_col_freefall()
void lara_as_freefall(ITEM_INFO* item, COLL_INFO* coll)
{
	item->speed = (item->speed * 95) / 100;

	if (item->fallspeed == LARA_FREEFALL_SCREAM_SPEED &&
		item->hitPoints > 0)
	{
		SoundEffect(SFX_TR4_LARA_FALL, &item->pos, 0);
	}

	if (TestLaraLand(item))
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			SetAnimation(item, LA_FREEFALL_LAND);

		StopSoundEffect(SFX_TR4_LARA_FALL);
		return;
	}

	item->goalAnimState = LS_FREEFALL;
}

// State:		LS_FREEFALL (9)
// Control:		lara_as_freefall()
void lara_col_freefall(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	item->gravityStatus = true;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);

	LaraSlideEdgeJump(item, coll);

	if (item->fallspeed > 0 && (coll->Middle.Floor <= 0 || TestLaraSwamp(item)))
	{
		LaraResetGravityStatus(item, coll);
		LaraSnapToHeight(item, coll);
	}
}

// State:		LS_REACH (11)
// Collision:	lara_col_reach()
void lara_as_reach(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	Camera.targetAngle = ANGLE(85.0f);

	if (item->hitPoints <= 0)
	{
		if (TestLaraLand(item))
			item->goalAnimState = LS_IDLE;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_JUMP_TURN_MAX / 2)
			info->turnRate = -LARA_JUMP_TURN_MAX / 2;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_JUMP_TURN_MAX / 2)
			info->turnRate = LARA_JUMP_TURN_MAX / 2;
	}

	if (TestLaraLand(item))
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			item->goalAnimState = LS_IDLE;

		return;
	}

	if (item->fallspeed >= LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
		return;
	}

	item->goalAnimState = LS_REACH;
}

// State:		LS_REACH (11)
// Control:		lara_as_reach()
void lara_col_reach(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	item->gravityStatus = (info->ropePtr == -1) ? true : false;
	coll->Setup.Height = LARA_HEIGHT_STRETCH;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = 0;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = info->moveAngle;
	coll->Setup.Radius = coll->Setup.Radius * 1.2f;
	coll->Setup.Mode = COLL_PROBE_MODE::FREE_FORWARD;
	GetCollisionInfo(coll, item);

	if (TestLaraHangJump(item, coll))
		return;

	LaraSlideEdgeJump(item, coll);

	GetCollisionInfo(coll, item);

	ShiftItem(item, coll);

	if (item->fallspeed > 0 && (coll->Middle.Floor <= 0 || TestLaraSwamp(item)))
	{
		LaraResetGravityStatus(item, coll);
		LaraSnapToHeight(item, coll);
	}
}

// TODO: Unused? Naming is also completely mismatched; enum calls it LS_GRAB_TO_FALL.
void lara_col_land(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 14*/
	/*state code: lara_void_func*/
	lara_col_idle(item, coll);
}

// State:		LS_JUMP_PREPARE (15)
// Collision:	lara_col_jump_prepare()
void lara_as_jump_prepare(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_IDLE;
		return;
	}

	if (TrInput & IN_LEFT &&
		TrInput & (IN_FORWARD | IN_BACK))
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN_MAX)
			info->turnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT &&
		TrInput & (IN_FORWARD | IN_BACK))
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN_MAX)
			info->turnRate = LARA_SLOW_TURN_MAX;
	}

	if (info->waterStatus == LW_WADE)
	{
		if (TestLaraStandingJump(item, coll, item->pos.yRot, 0))
			item->goalAnimState = LS_JUMP_UP;
		else
			item->goalAnimState = LS_IDLE;

		return;
	}

	if ((TrInput & IN_FORWARD ||
			!(TrInput & IN_DIRECTION) && info->jumpDirection == LaraJumpDirection::Forward) &&
		TestLaraStandingJump(item, coll, item->pos.yRot))
	{
		item->goalAnimState = LS_JUMP_FORWARD;
		info->jumpDirection = LaraJumpDirection::Forward;
		return;
	}
	else if ((TrInput & IN_BACK ||
			!(TrInput & IN_DIRECTION) && info->jumpDirection == LaraJumpDirection::Back) &&
		TestLaraStandingJump(item, coll, item->pos.yRot + ANGLE(180.0f)))
	{
		item->goalAnimState = LS_JUMP_BACK;
		info->jumpDirection = LaraJumpDirection::Back;
		return;
	}
	
	if ((TrInput & IN_LEFT ||
			!(TrInput & IN_DIRECTION) && info->jumpDirection == LaraJumpDirection::Left) &&
		TestLaraStandingJump(item, coll, item->pos.yRot - ANGLE(90.0f)))
	{
		item->goalAnimState = LS_JUMP_LEFT;
		info->jumpDirection = LaraJumpDirection::Left;
		return;
	}
	else if ((TrInput & IN_RIGHT ||
			!(TrInput & IN_DIRECTION) && info->jumpDirection == LaraJumpDirection::Right) &&
		TestLaraStandingJump(item, coll, item->pos.yRot + ANGLE(90.0f)))
	{
		item->goalAnimState = LS_JUMP_RIGHT;
		info->jumpDirection = LaraJumpDirection::Right;
		return;
	}
	
	if (TestLaraStandingJump(item, coll, item->pos.yRot, 0))
	{
		item->goalAnimState = LS_JUMP_UP;
		info->jumpDirection = LaraJumpDirection::Up;
		return;
	}

	item->goalAnimState = LS_IDLE;
	info->jumpDirection = LaraJumpDirection::None;
}

// State:		LS_JUMP_PREPARE (15)
// Collision:	lara_as_jump_prepare()
void lara_col_jump_prepare(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	item->gravityStatus = false;
	item->fallspeed = 0;

	info->moveAngle = item->pos.yRot;
	switch (info->jumpDirection)
	{
	case LaraJumpDirection::Back:
		info->moveAngle += ANGLE(180.0f);
		break;

	case LaraJumpDirection::Left:
		info->moveAngle -= ANGLE(90.0f);
		break;

	case LaraJumpDirection::Right:
		info->moveAngle += ANGLE(90.0f);
		break;

	default:
		break;
	}
	
	coll->Setup.BadHeightDown = TestLaraSwamp(item) ? NO_BAD_POS : STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesArePits = TestLaraSwamp(item) ? false : true;
	coll->Setup.SlopesAreWalls = TestLaraSwamp(item) ? false : true;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraFall(item, coll))
	{
		SetLaraFallState(item);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideState(item, coll);
		return;
	}

	ShiftItem(item, coll);

	if (TestLaraStep(coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_JUMP_BACK (25)
// Collision:	lara_col_jump_back()
void lara_as_jump_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;
	Camera.targetAngle = ANGLE(135.0f);

	if (item->hitPoints <= 0)
	{
		if (TestLaraLand(item))
			item->goalAnimState = LS_IDLE;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_JUMP_TURN_MAX / 2)
			info->turnRate = -LARA_JUMP_TURN_MAX / 2;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_JUMP_TURN_MAX / 2)
			info->turnRate = LARA_JUMP_TURN_MAX / 2;
	}

	if (TestLaraLand(item))
	{
		item->goalAnimState = LS_IDLE;
		return;
	}

	if (TrInput & (IN_ROLL | IN_FORWARD))
	{
		item->goalAnimState = LS_JUMP_ROLL_180;
		return;
	}

	if (item->fallspeed >= LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
		return;
	}

	item->goalAnimState = LS_JUMP_BACK;
}

// State:		LS_JUMP_BACK (25)
// Control:		lara_as_jump_back()
void lara_col_jump_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraJumpCollision(item, coll, item->pos.yRot + ANGLE(180.0f));
}

// State:		LS_JUMP_RIGHT (26)
// Collision:	lara_col_jump_right()
void lara_as_jump_right(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;

	if (item->hitPoints <= 0)
	{
		if (TestLaraLand(item))
			item->goalAnimState = LS_IDLE;

		return;
	}

	if (TestLaraLand(item))
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			item->goalAnimState = LS_IDLE;

		return;
	}

	// TODO: Core seems to have planned this feature. Add an animation to make it possible.
	/*if (TrInput & (IN_ROLL | IN_LEFT))
	{
		item->goalAnimState = LS_JUMP_ROLL_180;
		return;
	}*/

	if (item->fallspeed >= LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
		return;
	}

	item->goalAnimState = LS_JUMP_RIGHT;
}

// State:		LS_JUMP_RIGHT (26)
// Control:		lara_as_jump_right()
void lara_col_jump_right(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraJumpCollision(item, coll, item->pos.yRot + ANGLE(90.0f));
}

// State:		LS_JUMP_LEFT (27)
// Collision:	lara_as_jump_left()
void lara_as_jump_left(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;

	if (item->hitPoints <= 0)
	{
		if (TestLaraLand(item))
			item->goalAnimState = LS_IDLE;

		return;
	}

	if (TestLaraLand(item))
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			item->goalAnimState = LS_IDLE;

		return;
	}

	// TODO: Core seems to have planned this feature. Add an animation to make it possible.
	/*if (TrInput & (IN_ROLL | IN_RIGHT))
	{
		item->goalAnimState = LS_JUMP_ROLL_180;
		return;
	}*/

	if (item->fallspeed >= LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
		return;
	}

	item->goalAnimState = LS_JUMP_LEFT;
}

// State:		LS_JUMP_LEFT (27)
// Control:		lara_as_jump_left()
void lara_col_jump_left(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraJumpCollision(item, coll, item->pos.yRot - ANGLE(90.0f));
}

// State:		LS_JUMP_UP (27)
// Collision:	lara_col_jump_up()
void lara_as_jump_up(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;

	if (item->hitPoints <= 0)
	{
		if (TestLaraLand(item))
			item->goalAnimState = LS_IDLE;

		return;
	}

	if (TestLaraLand(item))
	{
		item->goalAnimState = LS_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		item->speed += 2;
		if (item->speed > 5)
			item->speed = 5;

		item->pos.xRot -= std::min((short)(LARA_LEAN_RATE / 3), (short)(abs(ANGLE(item->speed) - item->pos.xRot) / 3)) * 1;
		info->headXrot -= (ANGLE(5.0f) - item->pos.zRot) / 3;

	}
	else if (TrInput & IN_BACK)
	{
		item->speed -= 2;
		if (item->speed < -5)
			item->speed = -5;

		item->pos.xRot += std::min((short)(LARA_LEAN_RATE / 3), (short)(abs(ANGLE(item->speed) - item->pos.xRot) / 3)) * 1;
		info->headYrot += (ANGLE(10.0f) - item->pos.zRot) / 3;
	}
	else
		item->speed = item->speed <= 0 ? -2 : 2;

	if (item->fallspeed >= LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
		return;
	}

	item->goalAnimState = LS_JUMP_UP;
}

// State:		LS_JUMP_UP (28)
// Control:		lara_as_jump_up()
void lara_col_jump_up(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.Height = LARA_HEIGHT_STRETCH;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = (item->speed >= 0) ? info->moveAngle : info->moveAngle + ANGLE(180.0f);
	coll->Setup.Mode = COLL_PROBE_MODE::FREE_FORWARD;
	GetCollisionInfo(coll, item);

	if (TestLaraHangJumpUp(item, coll))
		return;

	if (coll->CollisionType == CT_CLAMP ||
		coll->CollisionType == CT_TOP ||
		coll->CollisionType == CT_TOP_FRONT)
	{
		item->fallspeed = 1;
	}

	ShiftItem(item, coll);

	if (item->fallspeed > 0 && (coll->Middle.Floor <= 0 || TestLaraSwamp(item)))
	{
		LaraResetGravityStatus(item, coll);
		LaraSnapToHeight(item, coll);
	}
}

// State:		LS_FALL_BACK (29)
// Collision:	lara_col_fall_back()
void lara_as_fall_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (item->hitPoints <= 0)
		return;

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_JUMP_TURN_MAX / 2)
			info->turnRate = -LARA_JUMP_TURN_MAX / 2;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_JUMP_TURN_MAX / 2)
			info->turnRate = LARA_JUMP_TURN_MAX / 2;
	}

	if (TestLaraLand(item))
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			item->goalAnimState = LS_IDLE;

		return;
	}

	if (TrInput & IN_ACTION &&
		info->gunStatus == LG_HANDS_FREE)
	{
		item->goalAnimState = LS_REACH;
		return;
	}

	if (item->fallspeed >= LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
		return;
	}

	item->goalAnimState = LS_FALL_BACK;
}

// State:		LS_FALL_BACK (29)
// Collision:	lara_col_fall_back()
void lara_col_fall_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot + ANGLE(180.0f);
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);

	LaraDeflectEdgeJump(item, coll);

	if (item->fallspeed > 0 && (coll->Middle.Floor <= 0 || TestLaraSwamp(item)))
	{
		LaraResetGravityStatus(item, coll);
		LaraSnapToHeight(item, coll);
	}
}

// State:		LS_SWANDIVE_START (52)
// Control:		lara_col_swandive()
void lara_as_swandive(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->gunStatus = LG_HANDS_BUSY;
	info->look = false;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpaz = false;

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_JUMP_TURN_MAX)
			info->turnRate = -LARA_JUMP_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE / 2);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_JUMP_TURN_MAX)
			info->turnRate = LARA_JUMP_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE / 2);
	}

	// TODO: Why?
	if (item->fallspeed > LARA_FREEFALL_SPEED && item->goalAnimState != LS_DIVE)
		item->goalAnimState = LS_SWANDIVE_END;
}

void lara_col_swandive(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;
	auto bounds = GetBoundsAccurate(item);
	auto realHeight = bounds->Y2 - bounds->Y1;

	/*state 52*/
	/*state code: lara_as_swandive*/
	info->moveAngle = item->pos.yRot;
	info->keepCrouched = TestLaraKeepCrouched(item, coll);
	coll->Setup.Height = std::max(LARA_HEIGHT_CRAWL, (int)(realHeight * 0.7f));
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeJump(item, coll))
		info->gunStatus = LG_HANDS_FREE;

	if (coll->Middle.Floor <= 0 && item->fallspeed > 0)
	{
		auto probe = GetCollisionResult(item, coll->Setup.ForwardAngle, coll->Setup.Radius, 0);

		if (TestLaraSlide(item, coll))
			SetLaraSlideState(item, coll);
		else if (info->keepCrouched ||
			abs(probe.Position.Ceiling - probe.Position.Floor) < LARA_HEIGHT &&
			g_GameFlow->Animations.CrawlspaceSwandive)
		{
			SetAnimation(item, LA_SPRINT_TO_CROUCH_LEFT, 10);

			if (!info->keepCrouched) // HACK: If Lara landed on the edge, shift forward to avoid standing up or falling out.
				MoveItem(item, coll->Setup.ForwardAngle, STEP_SIZE / 2);
		}
		else [[likely]]
			SetAnimation(item, LA_SWANDIVE_ROLL, 0);

		item->fallspeed = 0;
		item->gravityStatus = false;
		info->gunStatus = LG_HANDS_FREE;

		LaraSnapToHeight(item, coll);
	}
}

void lara_as_fastdive(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 53*/
	/*collision: lara_col_fastdive*/
	if (TrInput & IN_ROLL && item->goalAnimState == LS_SWANDIVE_END)
		item->goalAnimState = LS_JUMP_ROLL_180;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpaz = false;
	item->speed = (item->speed * 95) / 100;
}

void lara_col_fastdive(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 53*/
	/*state code: lara_as_fastdive*/
	info->moveAngle = item->pos.yRot;

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;

	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);
	LaraDeflectEdgeJump(item, coll);

	if (coll->Middle.Floor <= 0 && item->fallspeed > 0)
	{
		if (item->fallspeed <= 133)
			item->goalAnimState = LS_IDLE;
		else
			item->goalAnimState = LS_DEATH;

		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->Middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->Middle.Floor;
	}
}
