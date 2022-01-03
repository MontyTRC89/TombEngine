#include "framework.h"
#include "Game/Lara/lara_jump.h"

#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_basic.h"
#include "Scripting/GameFlowScript.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

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
		return;

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
		item->goalAnimState = LS_SWAN_DIVE_START;
		return;
	}

	if (item->fallspeed >= LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
		return;
	}
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

	if (TestLaraLand(item, coll))
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

		// TODO: It's still possible to achieve a softlock.
		DoLaraLand(item, coll);
	}
}

// State:		LS_FREEFALL (9)
// Collision:	lara_col_freefall()
void lara_as_freefall(ITEM_INFO* item, COLL_INFO* coll)
{
	item->speed = item->speed * 0.95f;

	if (item->fallspeed == LARA_FREEFALL_SCREAM_SPEED &&
		item->hitPoints > 0)
	{
		SoundEffect(SFX_TR4_LARA_FALL, &item->pos, 0);
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

	if (TestLaraLand(item, coll))
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			item->goalAnimState = LS_IDLE;

		StopSoundEffect(SFX_TR4_LARA_FALL);
		DoLaraLand(item, coll);
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
		if (info->turnRate < -LARA_JUMP_TURN_MAX / 2)
			info->turnRate = -LARA_JUMP_TURN_MAX / 2;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_JUMP_TURN_MAX / 2)
			info->turnRate = LARA_JUMP_TURN_MAX / 2;
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

	if (TestLaraLand(item, coll))
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			item->goalAnimState = LS_IDLE;

		DoLaraLand(item, coll);
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
		if (TestLaraJumpUp(item, coll))
			item->goalAnimState = LS_JUMP_UP;
		else
			item->goalAnimState = LS_IDLE;

		return;
	}

	// JUMP key repressed without directional key; cancel directional jump lock.
	if (DbInput & IN_JUMP && !(TrInput & IN_DIRECTION))
	{
		if (TestLaraJumpUp(item, coll))
		{
			item->goalAnimState = LS_JUMP_UP;
			info->jumpDirection = LaraJumpDirection::Up;
		}
		else
		{
			item->goalAnimState = LS_IDLE;
			info->jumpDirection = LaraJumpDirection::None;
		}

		return;
	}

	if ((TrInput & IN_FORWARD ||
			!(TrInput & IN_DIRECTION) && info->jumpDirection == LaraJumpDirection::Forward) &&
		TestLaraJumpForward(item, coll))
	{
		item->goalAnimState = LS_JUMP_FORWARD;
		info->jumpDirection = LaraJumpDirection::Forward;
		return;
	}
	else if ((TrInput & IN_BACK ||
			!(TrInput & IN_DIRECTION) && info->jumpDirection == LaraJumpDirection::Back) &&
		TestLaraJumpBack(item, coll))
	{
		item->goalAnimState = LS_JUMP_BACK;
		info->jumpDirection = LaraJumpDirection::Back;
		return;
	}
	
	if ((TrInput & IN_LEFT ||
			!(TrInput & IN_DIRECTION) && info->jumpDirection == LaraJumpDirection::Left) &&
		TestLaraJumpLeft(item, coll))
	{
		item->goalAnimState = LS_JUMP_LEFT;
		info->jumpDirection = LaraJumpDirection::Left;
		return;
	}
	else if ((TrInput & IN_RIGHT ||
			!(TrInput & IN_DIRECTION) && info->jumpDirection == LaraJumpDirection::Right) &&
		TestLaraJumpRight(item, coll))
	{
		item->goalAnimState = LS_JUMP_RIGHT;
		info->jumpDirection = LaraJumpDirection::Right;
		return;
	}
	
	// No directional key pressed and no directional lock; commit to jump up.
	if (TestLaraJumpUp(item, coll))
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
		return;

	if (TestLaraLand(item, coll))
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			item->goalAnimState = LS_IDLE;

		return;
	}

	// TODO: Core appears to have planned this feature. Add an animation to make it possible.
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
		return;

	// TODO: Core appears to have planned this feature. Add an animation to make it possible.
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
		return;

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

	if (TestLaraLand(item, coll))
	{
		item->goalAnimState = LS_IDLE;

		DoLaraLand(item, coll);
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

	if (TestLaraLand(item, coll))
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			item->goalAnimState = LS_IDLE;

		DoLaraLand(item, coll);
	}
}

// State:		LS_SWAN_DIVE_START (52)
// Collision:	lara_col_swan_dive()
void lara_as_swan_dive(ITEM_INFO* item, COLL_INFO* coll)
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

	if (item->fallspeed >= LARA_FREEFALL_SPEED && item->goalAnimState != LS_FREEFALL_DIVE)
		item->goalAnimState = LS_SWAN_DIVE_END;
}

// State:		LS_SWAN_DIVE_START (52)
// Control:		lara_as_swan_dive()
void lara_col_swan_dive(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;
	auto bounds = GetBoundsAccurate(item);
	auto realHeight = bounds->Y2 - bounds->Y1;

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

// State:		LS_FREEFALL_DIVE (53)
// Collision:	lara_col_freefall_dive()
void lara_as_freefall_dive(ITEM_INFO* item, COLL_INFO* coll)
{
	item->speed = item->speed * 0.95f;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpaz = false;

	if (TrInput & IN_ROLL &&
		item->goalAnimState == LS_SWAN_DIVE_END)
	{
		item->goalAnimState = LS_JUMP_ROLL_180;
		return;
	}
}

// State:		LS_FREEFALL_DIVE (53)
// Control:		lara_as_freefall_dive()
void lara_col_freefall_dive(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);

	LaraDeflectEdgeJump(item, coll);

	if (TestLaraLand(item, coll))
	{
		if (item->fallspeed >= LARA_FREEFALL_DIVE_DEATH_SPEED)
			item->goalAnimState = LS_DEATH;
		else
			item->goalAnimState = LS_IDLE;

		DoLaraLand(item, coll);
	}
}
