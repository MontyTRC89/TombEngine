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

void lara_as_forwardjump(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	// Update running jump counter in preparation for possible dispatch soon after landing.
	info->jumpCount++;
	if (info->jumpCount > LARA_JUMP_TIME / 2)
		info->jumpCount = LARA_JUMP_TIME / 2;

	/*state 3*/
	/*collision: */
	if (item->goalAnimState == LS_SWANDIVE_START ||
		item->goalAnimState == LS_REACH)
		item->goalAnimState = LS_JUMP_FORWARD;

	if (item->goalAnimState != LS_DEATH &&
		item->goalAnimState != LS_IDLE &&
		item->goalAnimState != LS_RUN_FORWARD)
	{
		if (info->gunStatus == LG_HANDS_FREE && TrInput & IN_ACTION)
			item->goalAnimState = LS_REACH;

		if (TrInput & IN_BACK || TrInput & IN_ROLL)
			item->goalAnimState = LS_JUMP_ROLL_180;

		if (info->gunStatus == LG_HANDS_FREE && TrInput & IN_WALK)
			item->goalAnimState = LS_SWANDIVE_START;

		if (item->fallspeed > LARA_FREEFALL_SPEED)
			item->goalAnimState = LS_FREEFALL;
	}

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
}

void lara_col_forwardjump(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 3*/
	/*state code: lara_as_forwardjump*/
	if (item->speed < 0)
		info->moveAngle = item->pos.yRot + ANGLE(180);
	else
		info->moveAngle = item->pos.yRot;

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
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
		{
			if (info->waterStatus == LW_WADE)
				item->goalAnimState = LS_IDLE;
			else
			{
				if (TrInput & IN_FORWARD && !(TrInput & IN_WALK))
					SetAnimation(item, LA_LAND_TO_RUN);
				else
					item->goalAnimState = LS_IDLE;
			}
		}

		item->gravityStatus = false;
		item->fallspeed = 0;
		item->speed = 0;

		LaraSnapToHeight(item, coll);
	}
}

void lara_as_fastfall(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 9*/
	/*collision: lara_col_fastfall*/
	item->speed = (item->speed * 95) / 100;
	if (item->fallspeed == 154)
		SoundEffect(SFX_TR4_LARA_FALL, &item->pos, 0);
}

void lara_col_fastfall(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 9*/
	/*state code: lara_as_fastfall*/
	item->gravityStatus = true;

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;

	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);
	LaraSlideEdgeJump(item, coll);

	if (coll->Middle.Floor <= 0 || TestLaraSwamp(item))
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			SetAnimation(item, LA_FREEFALL_LAND);

		StopSoundEffect(SFX_TR4_LARA_FALL);

		item->fallspeed = 0;
		item->gravityStatus = false;

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
		return;

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_REACH_TURN_MAX)
			info->turnRate = -LARA_REACH_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_REACH_TURN_MAX)
			info->turnRate = LARA_REACH_TURN_MAX;
	}

	if (TestLaraLand(item))
	{
		item->goalAnimState = LS_IDLE;
		item->fallspeed = 0;
		item->gravityStatus = false;
		return;
	}

	if (item->fallspeed > LARA_FREEFALL_SPEED)
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
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);
	ShiftItem(item, coll);

	if (item->fallspeed > 0 && coll->Middle.Floor <= 0)
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
		{
			if (coll->Middle.Floor != NO_HEIGHT)
				item->pos.yPos += coll->Middle.Floor;
		}
	}
}

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
		return;

	if (info->waterStatus == LW_WADE)
	{
		item->goalAnimState = LS_JUMP_UP;
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

	if ((TrInput & IN_FORWARD ||
			info->jumpDirection == LaraJumpDirection::Forward && !(TrInput & IN_DIRECTION)) &&
		TestLaraStandingJump(item, coll, item->pos.yRot))
	{
		item->goalAnimState = LS_JUMP_FORWARD;
		info->jumpDirection = LaraJumpDirection::Forward;
		info->moveAngle = item->pos.yRot;
		return;
	}
	else if ((TrInput & IN_BACK ||
			info->jumpDirection == LaraJumpDirection::Back && !(TrInput & IN_DIRECTION)) &&
		TestLaraStandingJump(item, coll, item->pos.yRot + ANGLE(180.0f)))
	{
		item->goalAnimState = LS_JUMP_BACK;
		info->jumpDirection = LaraJumpDirection::Back;
		info->moveAngle = item->pos.yRot + ANGLE(180.0f);
		return;
	}

	if ((TrInput & IN_LEFT ||
			info->jumpDirection == LaraJumpDirection::Left && !(TrInput & IN_DIRECTION)) &&
		TestLaraStandingJump(item, coll, item->pos.yRot - ANGLE(90.0f)))
	{
		item->goalAnimState = LS_JUMP_LEFT;
		info->jumpDirection = LaraJumpDirection::Left;
		info->moveAngle = item->pos.yRot - ANGLE(90.0f);
		return;
	}
	else if ((TrInput & IN_RIGHT ||
			info->jumpDirection == LaraJumpDirection::Right && !(TrInput & IN_DIRECTION)) &&
		TestLaraStandingJump(item, coll, item->pos.yRot + ANGLE(90.0f)))
	{
		item->goalAnimState = LS_JUMP_RIGHT;
		info->jumpDirection = LaraJumpDirection::Right;
		info->moveAngle = item->pos.yRot + ANGLE(90.0f);
		return;
	}
	
	item->goalAnimState = LS_JUMP_UP;
	info->jumpDirection = LaraJumpDirection::Up;
}

// State:		LS_JUMP_PREPARE (15)
// Collision:	lara_as_jump_prepare()
void lara_col_jump_prepare(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	item->fallspeed = 0;
	item->gravityStatus = false;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = NO_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);

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

	if (coll->Middle.Floor > -STEP_SIZE && coll->Middle.Floor < STEP_SIZE)
		item->pos.yPos += coll->Middle.Floor;
}

void lara_as_backjump(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 25*/
	/*collision: lara_col_backjump*/
	info->look = false;

	Camera.targetAngle = ANGLE(135.0f);
	if (item->fallspeed <= LARA_FREEFALL_SPEED)
	{
		if (item->goalAnimState == LS_RUN_FORWARD)
		{
			item->goalAnimState = LS_IDLE;
		}
		else if ((TrInput & IN_FORWARD || TrInput & IN_ROLL) && item->goalAnimState != LS_IDLE)
		{
			item->goalAnimState = LS_JUMP_ROLL_180;
		}
	}
	else
	{
		item->goalAnimState = LS_FREEFALL;
	}
}

void lara_col_backjump(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 25*/
	/*state code: lara_as_backjump*/
	info->moveAngle = item->pos.yRot + ANGLE(180);
	LaraJumpCollision(item, coll);
}

void lara_as_rightjump(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 26*/
	/*collision: lara_col_rightjump*/
	info->look = false;
	if (item->fallspeed > LARA_FREEFALL_SPEED)
		item->goalAnimState = LS_FREEFALL;
	else if (TrInput & IN_LEFT && item->goalAnimState != LS_IDLE)
		item->goalAnimState = LS_JUMP_ROLL_180;
}

void lara_col_rightjump(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 26*/
	/*state code: lara_as_rightjump*/
	info->moveAngle = item->pos.yRot + ANGLE(90);
	LaraJumpCollision(item, coll);
}

void lara_as_leftjump(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 27*/
	/*collision: lara_col_leftjump*/
	info->look = false;
	if (item->fallspeed > LARA_FREEFALL_SPEED)
		item->goalAnimState = LS_FREEFALL;
	else if (TrInput & IN_RIGHT && item->goalAnimState != LS_IDLE)
		item->goalAnimState = LS_JUMP_ROLL_180;
}

void lara_col_leftjump(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 27*/
	/*state code: lara_as_leftjump*/
	info->moveAngle = item->pos.yRot - ANGLE(90);
	LaraJumpCollision(item, coll);
}

void lara_as_upjump(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;

	/*state 28*/
	/*collision: lara_col_upjump*/
	if (item->fallspeed > LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
	}
}

void lara_col_upjump(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 28*/
	/*state code: lara_as_upjump*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_IDLE;
		return;
	}

	info->moveAngle = item->pos.yRot;

	coll->Setup.Height = LARA_HEIGHT_STRETCH;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = item->speed < 0 ? info->moveAngle + ANGLE(180.0f) : info->moveAngle;
	coll->Setup.Mode = COLL_PROBE_MODE::FREE_FORWARD;

	GetCollisionInfo(coll, item);

	if (TestLaraHangJumpUp(item, coll))
		return;

	if (coll->CollisionType == CT_CLAMP ||
		coll->CollisionType == CT_TOP ||
		coll->CollisionType == CT_TOP_FRONT)
		item->fallspeed = 1;

	ShiftItem(item, coll);

	if (coll->CollisionType == CT_NONE)
	{
		if (item->fallspeed < -70)
		{
			if (TrInput & IN_FORWARD && item->speed < 5)
			{
				item->speed++;
			}
			else if (TrInput & IN_BACK && item->speed > -5)
			{
				item->speed -= 2;
			}
		}
	}
	else
	{
		item->speed = item->speed <= 0 ? -2 : 2;
	}

	if (item->fallspeed > 0 && coll->Middle.Floor <= 0)
	{
		item->goalAnimState = LaraLandedBad(item, coll) ? LS_DEATH : LS_IDLE;

		item->gravityStatus = false;
		item->fallspeed = 0;

		if (coll->Middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->Middle.Floor;
	}
}

// State:		LS_FALL_BACK (29)
// Collision:	lara_col_fall_back()
void lara_as_fall_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (item->hitPoints <= 0)
		return;

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

	if (item->fallspeed > LARA_FREEFALL_SPEED)
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
