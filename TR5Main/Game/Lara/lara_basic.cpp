#include "framework.h"
#include "lara.h"
#include "lara_basic.h"
#include "lara_tests.h"
#include "lara_collide.h"
#include "lara_slide.h"
#include "lara_monkey.h"
#include "lara_helpers.h"
#include "input.h"
#include "level.h"
#include "setup.h"
#include "health.h"
#include "Sound/sound.h"
#include "animation.h"
#include "pickup.h"
#include "collide.h"
#include "items.h"
#include "camera.h"
#include "GameFlowScript.h"

// ------------------------------
// BASIC MOVEMENT & MISCELLANEOUS
// Control & Collision Functions
// ------------------------------

// --------------
// MISCELLANEOUS:
// --------------

void lara_void_func(ITEM_INFO* item, COLL_INFO* coll)
{
	return;
}

void lara_default_col(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesArePits = true;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);
	LaraResetGravityStatus(item, coll);
}

void lara_as_special(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(170.0f);
	Camera.targetElevation = -ANGLE(25.0f);
}

void lara_as_null(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
}

void lara_as_controlled(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

	if (item->frameNumber == TestLastFrame(item, item->animNumber) - 1)
	{
		info->gunStatus = LG_HANDS_FREE;

		if (UseForcedFixedCamera)
			UseForcedFixedCamera = 0;
	}
}

void lara_as_controlledl(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
}

// ---------------
// BASIC MOVEMENT:
// ---------------

// State:		LS_WALK_FORWARD (0)
// Collision:	lara_col_walk_forward()
void lara_as_walk_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->jumpCount++;
	if (info->jumpCount > LARA_JUMP_TIME / 2 + 4) // If anyone complains about the walk jump being nerfed, increase this value.
		info->jumpCount = LARA_JUMP_TIME / 2 + 4;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	// TODO: Implement item alignment properly someday. @Sezz 2021.11.01
	if (info->isMoving)
		return;

	// TODO: If stopping and holding WALK without FORWARD, Lara can't turn. @Sezz 2021.10.09
	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN)
			info->turnRate = -LARA_SLOW_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 6);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN)
			info->turnRate = LARA_SLOW_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 6);
	}

	if (TrInput & IN_FORWARD)
	{
		if (info->waterStatus == LW_WADE)
			item->goalAnimState = LS_WADE_FORWARD;
		else if (TrInput & IN_WALK) [[likely]]
			item->goalAnimState = LS_WALK_FORWARD;
		else
			item->goalAnimState = LS_RUN_FORWARD;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LA_WALK_FORWARD (0)
// Control:		lara_as_walk_forward()
void lara_col_walk_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.BadHeightDown = STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.SlopesArePits = true;
	coll->Setup.DeathFlagIsPit = true;
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
		return;

	if (TestLaraVault(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		item->goalAnimState = LS_SPLAT;
		if (GetChange(item, &g_Level.Anims[item->animNumber]))
			return;

		LaraCollideStop(item, coll);
	}

	if (TestLaraStep(coll) &&
		coll->CollisionType != CT_FRONT)
	{
		DoLaraStep(item, coll);

		return;
	}
}

// State:		LS_RUN_FORWARD (1)
// Collision:	lara_col_run_forward()
void lara_as_run_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->jumpCount++;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_FAST_TURN)
			info->turnRate = -LARA_FAST_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_FAST_TURN)
			info->turnRate = LARA_FAST_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE);
	}

	// Pseudo action queue for running jump.
	// Creates a committal lock when JUMP is pressed and released while allowJump isn't true yet.
	static bool commitJump = false;

	if ((TrInput & IN_JUMP || commitJump) &&
		!item->gravityStatus &&
		info->waterStatus != LW_WADE)
	{
		commitJump = TrInput & IN_FORWARD;

		if (info->jumpCount >= LARA_JUMP_TIME)
		{
			commitJump = false;
			item->goalAnimState = LS_JUMP_FORWARD;
		}

		return;
	}

	if (TrInput & IN_SPRINT &&
		info->sprintTimer &&
		info->waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_SPRINT;

		return;
	}

	if (TrInput & IN_ROLL &&
		info->waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_ROLL_FORWARD;

		return;
	}

	if (TrInput & IN_DUCK &&
		(info->gunStatus == LG_HANDS_FREE || !IsStandingWeapon(info->gunType)) &&
		info->waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_CROUCH_IDLE;

		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (info->waterStatus == LW_WADE)
			item->goalAnimState = LS_WADE_FORWARD;
		else if (TrInput & IN_WALK)
			item->goalAnimState = LS_WALK_FORWARD;
		else [[likely]]
			item->goalAnimState = LS_RUN_FORWARD;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_RUN_FORWARD (1)
// Control:		lara_as_run_forward()
void lara_col_run_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);
	LaraResetGravityStatus(item, coll);

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
		return;

	if (TestLaraVault(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		item->pos.zRot = 0;

		if (coll->HitTallObject || TestLaraWall(item, 256, 0, -640) != SPLAT_COLL::NONE)
		{
			item->goalAnimState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->animNumber]))
			{
				item->currentAnimState = LS_SPLAT;

				return;
			}
		}

		LaraCollideStop(item, coll);
	}

	if (TestLaraStep(coll) &&
		coll->CollisionType != CT_FRONT)
	{
		DoLaraStep(item, coll);

		return;
	}
}

// State:		LS_IDLE (2)
// Collision:	lara_col_idle()
void lara_as_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = ((TestLaraSwamp(item) && info->waterStatus == LW_WADE) || item->animNumber == LA_SWANDIVE_ROLL) ? false : true;

	// TODO: Hardcoding
	if (item->animNumber != LA_SPRINT_TO_STAND_RIGHT &&
		item->animNumber != LA_SPRINT_TO_STAND_LEFT)
		StopSoundEffect(SFX_TR4_LARA_SLIPPING);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	// Handles waterskin and clockwork beetle.
	// TODO: Hardcoding.
	if (UseSpecialItem(item))
		return;

	if (TrInput & IN_LOOK && info->look)
		LookUpDown();

	if (TrInput & IN_LEFT &&
		!(TrInput & IN_JUMP))		// JUMP locks y rotation.
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN)
			info->turnRate = -LARA_SLOW_TURN;
	}
	else if (TrInput & IN_RIGHT &&
		!(TrInput & IN_JUMP))
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN)
			info->turnRate = LARA_SLOW_TURN;
	}

	if (info->waterStatus == LW_WADE)
	{
		if (TestLaraSwamp(item))
			pseudo_lara_as_swamp_idle(item, coll);
		else [[likely]]
			pseudo_lara_as_wade_idle(item, coll);

		return;
	}

	if (TrInput & IN_JUMP &&
		coll->Middle.Ceiling < -(LARA_HEADROOM * 0.7f))
	{
		item->goalAnimState = LS_JUMP_PREPARE;

		return;
	}

	if (TrInput & IN_ROLL)
	{
		item->goalAnimState = LS_ROLL_FORWARD;

		return;
	}

	if (TrInput & IN_DUCK &&
		(info->gunStatus == LG_HANDS_FREE || !IsStandingWeapon(info->gunType)))
	{
		item->goalAnimState = LS_CROUCH_IDLE;

		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->goalAnimState = LS_WALK_FORWARD;

				return;
			}
		}
		else if (TrInput & IN_SPRINT &&
			TestLaraRunForward(item, coll))
		{
			item->goalAnimState = LS_SPRINT;

			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->goalAnimState = LS_RUN_FORWARD;

			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->goalAnimState = LS_WALK_BACK;

				return;
			}
		}
		else if (TestLaraHopBack(item, coll)) [[likely]]
		{
			item->goalAnimState = LS_RUN_BACK;

			return;
		}
	}

	if (TrInput & IN_LSTEP &&
		TestLaraStepLeft(item, coll))
	{
		item->goalAnimState = LS_STEP_LEFT;

		return;
	}
	else if (TrInput & IN_RSTEP &&
		TestLaraStepRight(item, coll))
	{
		item->goalAnimState = LS_STEP_RIGHT;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		if (TrInput & IN_SPRINT ||
			info->turnRate <= -LARA_SLOW_TURN ||
			(info->gunStatus == LG_READY && info->gunType != WEAPON_TORCH) ||
			(info->gunStatus == LG_DRAW_GUNS && info->gunType != WEAPON_FLARE))
		{
			item->goalAnimState = LS_TURN_LEFT_FAST;
		}
		else [[likely]]
			item->goalAnimState = LS_TURN_LEFT_SLOW;

		return;
	}
	else if (TrInput & IN_RIGHT)
	{
		if (TrInput & IN_SPRINT ||
			info->turnRate >= LARA_SLOW_TURN ||
			(info->gunStatus == LG_READY && info->gunType != WEAPON_TORCH) ||
			(info->gunStatus == LG_DRAW_GUNS && info->gunType != WEAPON_FLARE))
		{
			item->goalAnimState = LS_TURN_RIGHT_FAST;
		}
		else [[likely]]
			item->goalAnimState = LS_TURN_RIGHT_SLOW;

		return;
	}

	// TODO: Without animation blending, the AFK state's
	// movement lock will be rather obnoxious.
	// Adding some idle breathing would also be nice. @Sezz 2021.10.31
	if (info->poseCount >= LARA_POSE_TIME &&
		TestLaraPose(item, coll) &&
		g_GameFlow->Animations.Pose)
	{
		item->goalAnimState = LS_POSE;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// TODO: Future-proof for rising water.
// TODO: Make these into true states someday? It may take a bit of work. @Sezz 2021.10.13
// Pseudo-state for idling in wade-height water.
void pseudo_lara_as_wade_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_JUMP &&
		coll->Middle.Ceiling < -(LARA_HEADROOM * 0.7f))
	{
		item->goalAnimState = LS_JUMP_PREPARE;

		return;
	}

	if (TrInput & IN_FORWARD &&
		TestLaraRunForward(item, coll))
	{
		item->goalAnimState = LS_WADE_FORWARD;
		
		return;
	}

	if (TrInput & IN_BACK &&
		TestLaraWalkBack(item, coll))
	{
		item->goalAnimState = LS_WALK_BACK;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->goalAnimState = LS_TURN_LEFT_SLOW;

		return;
	}
	else if (TrInput & IN_RIGHT)
	{
		item->goalAnimState = LS_TURN_RIGHT_SLOW;

		return;
	}

	if (TrInput & IN_LSTEP &&
		TestLaraStepLeft(item, coll))
	{
		item->goalAnimState = LS_STEP_LEFT;

		return;
	}
	else if (TrInput & IN_RSTEP &&
		TestLaraStepRight(item, coll))
	{
		item->goalAnimState = LS_STEP_RIGHT;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// Pseudo-state for idling in swamps.
void pseudo_lara_as_swamp_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_FORWARD &&
		TestLaraRunForward(item, coll))
	{
		item->goalAnimState = LS_WADE_FORWARD;
		
		return;
	}

	if (TrInput & IN_BACK &&
		TestLaraWalkBackSwamp(item, coll))
	{
		item->goalAnimState = LS_WALK_BACK;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->goalAnimState = LS_TURN_LEFT_SLOW;

		return;
	}
	else if (TrInput & IN_RIGHT)
	{
		item->goalAnimState = LS_TURN_RIGHT_SLOW;

		return;
	}

	if (TrInput & IN_LSTEP &&
		TestLaraStepLeftSwamp(item, coll))
	{
		item->goalAnimState = LS_STEP_LEFT;

		return;
	}
	else if (TrInput & IN_RSTEP &&
		TestLaraStepRightSwamp(item, coll))
	{
		item->goalAnimState = LS_STEP_RIGHT;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_IDLE (2)
// Control:		lara_as_idle()
void lara_col_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = TestLaraSwamp(item) ? NO_BAD_POS : STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	item->gravityStatus = false;
	item->fallspeed = 0;
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
		return;

	ShiftItem(item, coll);

	// TODO: Vaulting from this state.

	if (TestLaraStep(coll))
	{
		DoLaraStep(item, coll);

		return;
	}
}

// State:		LS_POSE (4)
// Control:		lara_col_pose()
void lara_as_pose(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	if (TestLaraPose(item, coll))
	{
		if (TrInput & IN_ROLL)
		{
			item->goalAnimState = LS_ROLL_FORWARD;

			return;
		}

		if (TrInput & IN_WAKE)
		{
			item->goalAnimState = LS_IDLE;

			return;
		}

		item->goalAnimState = LS_POSE;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_POSE (4)
// Control:		lara_as_pose()
void lara_col_pose(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_RUN_BACK (5)
// Collision:	lara_col_run_back()
void lara_as_run_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_MED_TURN)
			info->turnRate = -LARA_MED_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_MED_TURN)
			info->turnRate = LARA_MED_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);
	}

	if (TrInput & IN_ROLL)
	{
		item->goalAnimState = LS_ROLL_FORWARD;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_RUN_BACK (5)
// Control:		lara_as_run_back()
void lara_col_run_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot + ANGLE(180.0f);
	item->fallspeed = 0;
	item->gravityStatus = false;
	coll->Setup.SlopesAreWalls = false;
	coll->Setup.SlopesArePits = true;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);

		return;
	}

	if (coll->Middle.Floor > (STEPUP_HEIGHT / 2))
	{
		SetLaraFallBackState(item);

		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (TestLaraStep(coll))
	{
		DoLaraStep(item, coll);

		return;
	}
}

// State:		LS_TURN_RIGHT_SLOW (6)
// Collision:	lara_col_turn_right_slow()
void lara_as_turn_right_slow(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = (TestLaraSwamp(item) && info->waterStatus == LW_WADE) ? false : true;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	info->turnRate += LARA_TURN_RATE;
	if (info->turnRate < 0)
		info->turnRate = 0;
	else if (info->turnRate > LARA_MED_FAST_TURN)
		info->turnRate = LARA_MED_FAST_TURN;

	if (info->waterStatus == LW_WADE)
	{
		if (TestLaraSwamp(item))
			pseudo_lara_as_swamp_turn_right_slow(item, coll);
		else [[likely]]
			pseudo_lara_as_wade_turn_right_slow(item, coll);

		return;
	}

	if (TrInput & IN_JUMP &&
		coll->Middle.Ceiling < -(LARA_HEADROOM * 0.7f))
	{
		item->goalAnimState = LS_JUMP_PREPARE;

		return;
	}

	if (TrInput & IN_ROLL)
	{
		item->goalAnimState = LS_ROLL_FORWARD;

		return;
	}

	if (TrInput & IN_DUCK &&
		(info->gunStatus == LG_HANDS_FREE || !IsStandingWeapon(info->gunType)))
	{
		item->goalAnimState = LS_CROUCH_IDLE;

		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->goalAnimState = LS_WALK_FORWARD;

				return;
			}
		}
		else if (TrInput & IN_SPRINT &&
			TestLaraRunForward(item, coll))
		{
			item->goalAnimState = LS_SPRINT;

			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->goalAnimState = LS_RUN_FORWARD;

			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->goalAnimState = LS_WALK_BACK;

				return;
			}
		}
		else if (TestLaraHopBack(item, coll)) [[likely]]
		{
			item->goalAnimState = LS_RUN_BACK;

			return;
		}
	}

	if (TrInput & IN_LSTEP &&
		TestLaraStepLeft(item, coll))
	{
		item->goalAnimState = LS_STEP_LEFT;

		return;
	}
	else if (TrInput & IN_RSTEP &&
		TestLaraStepRight(item, coll))
	{
		item->goalAnimState = LS_STEP_RIGHT;

		return;
	}

	if (TrInput & IN_RIGHT)
	{
		if (TrInput & IN_WALK) // TODO: This hasn't worked since TR1.
		{
			if (info->turnRate > LARA_SLOW_TURN)
				info->turnRate = LARA_SLOW_TURN;

			item->goalAnimState = LS_TURN_RIGHT_SLOW;
		}
		else if (info->turnRate > LARA_SLOW_MED_TURN)
			item->goalAnimState = LS_TURN_RIGHT_FAST;
		else [[likely]]
			item->goalAnimState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// Pseudo-state for turning right slowly in wade-height water.
void pseudo_lara_as_wade_turn_right_slow(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (info->turnRate > LARA_WADE_TURN_MAX)
		info->turnRate = LARA_WADE_TURN_MAX;

	if (TrInput & IN_JUMP &&
		coll->Middle.Ceiling < -(LARA_HEADROOM * 0.7f))
	{
		item->goalAnimState = LS_JUMP_PREPARE;

		return;
	}

	if (TrInput & IN_FORWARD &&
		TestLaraRunForward(item, coll))
	{
		item->goalAnimState = LS_WADE_FORWARD;

		return;
	}
	else if (TrInput & IN_BACK &&
		TestLaraWalkBack(item, coll))
	{
		item->goalAnimState = LS_WALK_BACK;

		return;
	}

	if (TrInput & IN_LSTEP &&
		TestLaraStepLeft(item, coll))
	{
		item->goalAnimState = LS_STEP_LEFT;

		return;
	}
	else if (TrInput & IN_RSTEP &&
		TestLaraStepRight(item, coll))
	{
		item->goalAnimState = LS_STEP_RIGHT;

		return;
	}

	if (TrInput & IN_RIGHT)
	{
		item->goalAnimState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// Pseudo-state for turning right slowly in swamps.
void pseudo_lara_as_swamp_turn_right_slow(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (info->turnRate > LARA_SWAMP_TURN_MAX)
		info->turnRate = LARA_SWAMP_TURN_MAX;

	if (TrInput & IN_FORWARD &&
		TestLaraRunForward(item, coll))
	{
		item->goalAnimState = LS_WADE_FORWARD;

		return;
	}
	else if (TrInput & IN_BACK &&
		TestLaraWalkBackSwamp(item, coll))
	{
		item->goalAnimState = LS_WALK_BACK;

		return;
	}

	if (TrInput & IN_LSTEP &&
		TestLaraStepLeftSwamp(item, coll))
	{
		item->goalAnimState = LS_STEP_LEFT;

		return;
	}
	else if (TrInput & IN_RSTEP &&
		TestLaraStepRightSwamp(item, coll))
	{
		item->goalAnimState = LS_STEP_RIGHT;

		return;
	}

	if (TrInput & IN_RIGHT)
	{
		item->goalAnimState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_TURN_RIGHT_SLOW (6)
// Control:		lara_as_turn_right_slow()
void lara_col_turn_right_slow(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_TURN_LEFT_SLOW (7)
// Collision:	lara_col_turn_left_slow()
void lara_as_turn_left_slow(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = (TestLaraSwamp(item) && info->waterStatus == LW_WADE) ? false : true;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	info->turnRate -= LARA_TURN_RATE;
	if (info->turnRate > 0)
		info->turnRate = 0;
	else if (info->turnRate < -LARA_MED_FAST_TURN)
		info->turnRate = -LARA_MED_FAST_TURN;

	if (info->waterStatus == LW_WADE)
	{
		if (TestLaraSwamp(item))
			pseudo_lara_as_swamp_turn_left_slow(item, coll);
		else [[likely]]
			pseudo_lara_as_wade_turn_left_slow(item, coll);

		return;
	}

	if (TrInput & IN_JUMP &&
		coll->Middle.Ceiling < -(LARA_HEADROOM * 0.7f))
	{
		item->goalAnimState = LS_JUMP_PREPARE;

		return;
	}

	if (TrInput & IN_ROLL)
	{
		item->goalAnimState = LS_ROLL_FORWARD;

		return;
	}

	if (TrInput & IN_DUCK &&
		(info->gunStatus == LG_HANDS_FREE || !IsStandingWeapon(info->gunType)))
	{
		item->goalAnimState = LS_CROUCH_IDLE;

		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->goalAnimState = LS_WALK_FORWARD;

				return;
			}
		}
		else if (TrInput & IN_SPRINT &&
			TestLaraRunForward(item, coll))
		{
			item->goalAnimState = LS_SPRINT;

			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->goalAnimState = LS_RUN_FORWARD;

			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->goalAnimState = LS_WALK_BACK;

				return;
			}
		}
		else if (TestLaraHopBack(item, coll)) [[likely]]
		{
			item->goalAnimState = LS_RUN_BACK;

			return;
		}
	}

	if (TrInput & IN_LSTEP &&
		TestLaraStepLeft(item, coll))
	{
		item->goalAnimState = LS_STEP_LEFT;

		return;
	}
	// TODO: Lara steps left. Why?? @Sezz 2021.10.08
	else if (TrInput & IN_RSTEP &&
		TestLaraStepRight(item, coll))
	{
		item->goalAnimState = LS_STEP_RIGHT;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		if (TrInput & IN_WALK)
		{
			if (info->turnRate < -LARA_SLOW_TURN)
				info->turnRate = -LARA_SLOW_TURN;

			item->goalAnimState = LS_TURN_LEFT_SLOW;
		}
		else if (info->turnRate < -LARA_SLOW_MED_TURN)
			item->goalAnimState = LS_TURN_LEFT_FAST;
		else [[likely]]
			item->goalAnimState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// Pseudo-state for turning left slowly in wade-height water.
void pseudo_lara_as_wade_turn_left_slow(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (info->turnRate < -LARA_WADE_TURN_MAX)
		info->turnRate = -LARA_WADE_TURN_MAX;

	if (TrInput & IN_JUMP &&
		coll->Middle.Ceiling < -(LARA_HEADROOM * 0.7f))
	{
		item->goalAnimState = LS_JUMP_PREPARE;

		return;
	}

	if (TrInput & IN_FORWARD &&
		TestLaraRunForward(item, coll))
	{
		item->goalAnimState = LS_WADE_FORWARD;

		return;
	}
	else if (TrInput & IN_BACK &&
		TestLaraWalkBack(item, coll))
	{
		item->goalAnimState = LS_WALK_BACK;

		return;
	}

	if (TrInput & IN_LSTEP &&
		TestLaraStepLeft(item, coll))
	{
		item->goalAnimState = LS_STEP_LEFT;

		return;
	}
	else if (TrInput & IN_RSTEP &&
		TestLaraStepRight(item, coll))
	{
		item->goalAnimState = LS_STEP_RIGHT;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->goalAnimState = LS_TURN_LEFT_SLOW;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// Pseudo-state for turning left slowly in swamps.
void pseudo_lara_as_swamp_turn_left_slow(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (info->turnRate < -LARA_SWAMP_TURN_MAX)
		info->turnRate = -LARA_SWAMP_TURN_MAX;

	if (TrInput & IN_FORWARD &&
		TestLaraRunForward(item, coll))
	{
		item->goalAnimState = LS_WADE_FORWARD;

		return;
	}
	else if (TrInput & IN_BACK &&
		TestLaraWalkBack(item, coll))
	{
		item->goalAnimState = LS_WALK_BACK;

		return;
	}

	if (TrInput & IN_LSTEP &&
		TestLaraStepLeft(item, coll))
	{
		item->goalAnimState = LS_STEP_LEFT;

		return;
	}
	else if (TrInput & IN_RSTEP &&
		TestLaraStepRight(item, coll))
	{
		item->goalAnimState = LS_STEP_RIGHT;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->goalAnimState = LS_TURN_LEFT_SLOW;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_TURN_LEFT_SLOW (7)
// Control:		lara_as_turn_left_slow()
void lara_col_turn_left_slow(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_turn_right_slow(item, coll);
}

void lara_as_death(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 8*/
	/*collision: lara_col_death*/
	info->look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

	if (BinocularRange)
	{
		BinocularRange = 0;
		LaserSight = false;
		AlterFOV(ANGLE(80.0f));
		LaraItem->meshBits = -1;
		info->busy = false;
	}
}

void lara_col_death(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 8*/
	/*state code: lara_as_death*/
	StopSoundEffect(SFX_TR4_LARA_FALL);

	info->moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.Radius = LARA_RAD_DEATH;

	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);
	ShiftItem(item, coll);

	item->hitPoints = -1;
	info->air = -1;

	if (coll->Middle.Floor != NO_HEIGHT)
		item->pos.yPos += coll->Middle.Floor;
}

void lara_as_splat(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;
}

void lara_col_splat(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;

	coll->Setup.SlopesAreWalls = true;
	coll->Setup.SlopesArePits = true;

	coll->Setup.BadHeightDown = STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;

	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);
	ShiftItem(item, coll);

	if (coll->Middle.Floor >= -256 && coll->Middle.Floor <= 256)
		item->pos.yPos += coll->Middle.Floor;
}

// State:		LS_WALK_BACK (16)
// Collision:	lara_col_walk_back()
void lara_as_walk_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = (TestLaraSwamp(item) && info->waterStatus == LW_WADE) ? false : true;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	if (info->isMoving)
		return;

	if (TestLaraSwamp(item) &&
		info->waterStatus == LW_WADE)
	{
		pseudo_lara_as_swamp_walk_back(item, coll);

		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN)
			info->turnRate = -LARA_SLOW_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);

	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN)
			info->turnRate = LARA_SLOW_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);
	}

	if (TrInput & IN_BACK &&
		(TrInput & IN_WALK || info->waterStatus == LW_WADE))
	{
		item->goalAnimState = LS_WALK_BACK;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// Pseudo-state for walking back in swamps.
void pseudo_lara_as_swamp_walk_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN / 3)
			info->turnRate = -LARA_SLOW_TURN / 3;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 3);

	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN / 3)
			info->turnRate = LARA_SLOW_TURN / 3;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 3);
	}

	if (TrInput & IN_BACK)
	{
		item->goalAnimState = LS_WALK_BACK;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_WALK_BACK (16)
// Control:		lara_as_walk_back()
void lara_col_walk_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot + ANGLE(180.0f);
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.BadHeightDown = (info->waterStatus == LW_WADE) ? NO_BAD_POS : STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesArePits = TestLaraSwamp(item) ? false : true;
	coll->Setup.SlopesAreWalls = TestLaraSwamp(item) ? false : true;
	coll->Setup.DeathFlagIsPit = true;
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
		return;

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (TestLaraStep(coll))
	{
		DoLaraStep(item, coll);

		return;
	}
}

// State:		LS_TURN_RIGHT_FAST (20)
// Collision:	lara_col_turn_right_fast()
void lara_as_turn_right_fast(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	info->turnRate += LARA_TURN_RATE;
	if (info->turnRate < LARA_MED_TURN)
		info->turnRate = LARA_MED_TURN;
	else if (info->turnRate > LARA_FAST_TURN)
		info->turnRate = LARA_FAST_TURN;

	if (TrInput & IN_JUMP &&
		coll->Middle.Ceiling < -(LARA_HEADROOM * 0.7f))
	{
		item->goalAnimState = LS_JUMP_PREPARE;

		return;
	}

	if (TrInput & IN_ROLL &&
		info->waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_ROLL_FORWARD;

		return;
	}

	if (TrInput & IN_DUCK &&
		(info->gunStatus == LG_HANDS_FREE || !IsStandingWeapon(info->gunType)) &&
		info->waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_CROUCH_IDLE;

		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (info->waterStatus == LW_WADE)		// Should not be possible, but here for security.
		{
			if (TestLaraRunForward(item, coll))
			{
				item->goalAnimState = LS_WADE_FORWARD;

				return;
			}
		}
		else if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->goalAnimState = LS_WALK_FORWARD;

				return;
			}
		}
		else if (TrInput & IN_SPRINT &&
			TestLaraRunForward(item, coll))
		{
			item->goalAnimState = LS_SPRINT;

			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->goalAnimState = LS_RUN_FORWARD;

			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->goalAnimState = LS_WALK_BACK;

				return;
			}
		}
		else if (TestLaraHopBack(item, coll)) [[likely]]
		{
			item->goalAnimState = LS_RUN_BACK;

			return;
		}
	}

	if (TrInput & IN_LSTEP &&
		TestLaraStepLeft(item, coll))
	{
		item->goalAnimState = LS_STEP_LEFT;

		return;
	}
	else if (TrInput & IN_RSTEP &&
		TestLaraStepRight(item, coll))
	{
		item->goalAnimState = LS_STEP_RIGHT;

		return;
	}

	// TODO: Hold WALK to slow down again.
	if (TrInput & IN_RIGHT)
	{
		item->goalAnimState = LS_TURN_RIGHT_FAST;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_TURN_RIGHT_FAST (20)
// Control:		lara_as_turn_right_fast()
void lara_col_turn_right_fast(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_TURN_LEFT_FAST (152)
// Collision:	lara_col_turn_left_fast()
void lara_as_turn_left_fast(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	info->turnRate -= LARA_TURN_RATE;
	if (info->turnRate > -LARA_MED_TURN)
		info->turnRate = -LARA_MED_TURN;
	else if (info->turnRate < -LARA_FAST_TURN)
		info->turnRate = -LARA_FAST_TURN;

	if (TrInput & IN_JUMP &&
		coll->Middle.Ceiling < -(LARA_HEADROOM * 0.7f))
	{
		item->goalAnimState = LS_JUMP_PREPARE;

		return;
	}

	if (TrInput & IN_ROLL &&
		info->waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_ROLL_FORWARD;

		return;
	}

	if (TrInput & IN_DUCK &&
		(info->gunStatus == LG_HANDS_FREE || !IsStandingWeapon(info->gunType)) &&
		info->waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_CROUCH_IDLE;

		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (info->waterStatus == LW_WADE)		// Should not be possible, but here for security.
		{
			if (TestLaraRunForward(item, coll))
			{
				item->goalAnimState = LS_WADE_FORWARD;

				return;
			}
		}
		else if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->goalAnimState = LS_WALK_FORWARD;

				return;
			}
		}
		else if (TrInput & IN_SPRINT &&
			TestLaraRunForward(item, coll))
		{
			item->goalAnimState = LS_SPRINT;

			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->goalAnimState = LS_RUN_FORWARD;

			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->goalAnimState = LS_WALK_BACK;

				return;
			}
		}
		else if (TestLaraHopBack(item, coll)) [[likely]]
		{
			item->goalAnimState = LS_RUN_BACK;

			return;
		}
	}

	if (TrInput & IN_LSTEP &&
		TestLaraStepLeft(item, coll))
	{
		item->goalAnimState = LS_STEP_LEFT;

		return;
	}
	// TODO: Lara steps left. Why?? @Sezz 2021.10.08
	else if (TrInput & IN_RSTEP &&
		TestLaraStepRight(item, coll))
	{
		item->goalAnimState = LS_STEP_RIGHT;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->goalAnimState = LS_TURN_LEFT_FAST;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_TURN_LEFT_FAST (152)
// Control:		lara_as_turn_left_fast()
void lara_col_turn_left_fast(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_STEP_RIGHT (21)
// Collision:	lara_col_step_right()
void lara_as_step_right(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_IDLE;

		return;
	}

	if (info->isMoving)
		return;

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN)
			info->turnRate = -LARA_SLOW_TURN;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN)
			info->turnRate = LARA_SLOW_TURN;
	}

	if (TrInput & IN_RSTEP)
	{
		item->goalAnimState = LS_STEP_RIGHT;

		return;
	}
	
	item->goalAnimState = LS_IDLE;
}

// State:		LS_STEP_RIGHT (21)
// Control:		lara_as_step_right()
void lara_col_step_right(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot + ANGLE(90.0f);
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.BadHeightDown = (info->waterStatus == LW_WADE) ? NO_BAD_POS : STEP_SIZE / 2;
	coll->Setup.BadHeightUp = -STEP_SIZE / 2;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesArePits = TestLaraSwamp(item) ? false : true;
	coll->Setup.SlopesAreWalls = TestLaraSwamp(item) ? false : true;
	coll->Setup.DeathFlagIsPit = true;
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
		return;

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (TestLaraStep(coll) || TestLaraSwamp(item))
	{
		DoLaraStep(item, coll);

		return;
	}
}

// State:		LS_STEP_LEFT (22)
// Collision:	lara_col_step_left()
void lara_as_step_left(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_IDLE;

		return;
	}

	if (info->isMoving)
		return;

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN)
			info->turnRate = -LARA_SLOW_TURN;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN)
			info->turnRate = LARA_SLOW_TURN;
	}

	if (TrInput & IN_LSTEP)
	{
		item->goalAnimState = LS_STEP_LEFT;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_STEP_LEFT (22)
// Control:		lara_as_step_left()
void lara_col_step_left(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot - ANGLE(90.0f);
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.BadHeightDown = (info->waterStatus == LW_WADE) ? NO_BAD_POS : STEP_SIZE / 2;
	coll->Setup.BadHeightUp = -STEP_SIZE / 2;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesArePits = TestLaraSwamp(item) ? false : true;
	coll->Setup.SlopesAreWalls = TestLaraSwamp(item) ? false : true;
	coll->Setup.DeathFlagIsPit = true;
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
		return;

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (TestLaraStep(coll) || TestLaraSwamp(item))
	{
		DoLaraStep(item, coll);

		return;
	}
}

// State:		LS_ROLL_BACK (23)
// Collision:	lara_col_roll_back()
void lara_as_roll_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;

	if (TrInput & IN_ROLL)
	{
		item->goalAnimState = LS_ROLL_BACK;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_ROLL_BACK (23)
// Control:		lara_as_roll_back()
void lara_col_roll_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	Camera.laraNode = 0;
	info->moveAngle = item->pos.yRot + ANGLE(180);
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);

		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	if (coll->Middle.Floor > STEPUP_HEIGHT / 2) // Was 200.
	{
		SetLaraFallBackState(item);

		return;
	}

	ShiftItem(item, coll);

	if (TestLaraStep(coll))
	{
		DoLaraStep(item, coll);

		return;
	}
}

// State:		LS_ROLL_FORWARD (45)
// Collision:	lara_col_roll_forward()
void lara_as_roll_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;

	// TODO: Change swandive roll anim state to something sensible.
	if (TrInput & IN_FORWARD &&
		item->animNumber == LA_SWANDIVE_ROLL)
	{
		item->goalAnimState = LS_RUN_FORWARD;

		return;
	}

	if (TrInput & IN_ROLL)
	{
		item->goalAnimState = LS_ROLL_FORWARD;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_ROLL_FORWARD (45)
// Control:		lara_as_roll_forward()
void lara_col_roll_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesArePits = false;
	coll->Setup.SlopesAreWalls = true;
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
		return;

	ShiftItem(item, coll);

	if (TestLaraStep(coll))
	{
		DoLaraStep(item, coll);

		return;
	}
}

// State:		LS_WADE_FORWARD (65)
// Collision:	lara_col_wade_forward()
void lara_as_wade_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = (TestLaraSwamp(item) && info->waterStatus == LW_WADE) ? false : true;
	Camera.targetElevation = -ANGLE(22.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_IDLE;

		return;
	}

	if (TestLaraSwamp(item))
	{
		pseudo_lara_as_swamp_wade_forward(item, coll);

		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_FAST_TURN)
			info->turnRate = -LARA_FAST_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE / 2);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_FAST_TURN)
			info->turnRate = LARA_FAST_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE / 2);
	}

	if (TrInput & IN_FORWARD)
	{
		if (info->waterStatus == LW_ABOVE_WATER)
			item->goalAnimState = LS_RUN_FORWARD;
		else [[likely]]
			item->goalAnimState = LS_WADE_FORWARD;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// Pseudo-state for wading in swamps.
void pseudo_lara_as_swamp_wade_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SWAMP_TURN_MAX)
			info->turnRate = -LARA_SWAMP_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 5 * 3, LARA_LEAN_RATE / 3);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SWAMP_TURN_MAX)
			info->turnRate = LARA_SWAMP_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 5 * 3, LARA_LEAN_RATE / 3);
	}

	if (TrInput & IN_FORWARD)
	{
		item->goalAnimState = LS_WADE_FORWARD;
		
		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_WADE_FORWARD (65)
// Control:		lara_as_wade_forward()
void lara_col_wade_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = TestLaraSwamp(item) ? false : true;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);

		return;
	}

	if (TestLaraVault(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		item->pos.zRot = 0;

		if (coll->Front.Floor < -STEPUP_HEIGHT &&
			!coll->Front.Slope &&
			!TestLaraSwamp(item))
		{
			item->goalAnimState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->animNumber]))
				return;
		}

		LaraCollideStop(item, coll);
	}

	if (TestLaraStep(coll) || TestLaraSwamp(item))
	{
		DoLaraStep(item, coll);

		return;
	}
}

// State:		LS_SPRINT (73)
// Collision:	lara_col_sprint()
void lara_as_sprint(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->sprintTimer--;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN)
			info->turnRate = -LARA_SLOW_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN)
			info->turnRate = LARA_SLOW_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE);
	}

	if (TrInput & IN_JUMP)
	{
		item->goalAnimState = LS_SPRINT_ROLL;

		return;
	}

	if (TrInput & IN_DUCK &&
		(info->gunStatus == LG_HANDS_FREE || !IsStandingWeapon(info->gunType)))
	{
		item->goalAnimState = LS_CROUCH_IDLE;

		return;
	}

	// TODO: Supposedly there is a bug wherein sprinting into the boundary between shallow and deep water
	// while meeting some condition allows Lara to run around in the water room. Investigate. @Sezz 2021.09.29
	if (TrInput & IN_FORWARD)
	{
		if (info->waterStatus == LW_WADE)
			item->goalAnimState = LS_RUN_FORWARD;	// TODO: Dispatch to wade forward state directly. @Sezz 2021.09.29
		else if (TrInput & IN_WALK)
			item->goalAnimState = LS_WALK_FORWARD;
		else if (TrInput & IN_SPRINT && info->sprintTimer > 0) [[likely]]
			item->goalAnimState = LS_SPRINT;
		else
			item->goalAnimState = LS_RUN_FORWARD;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_SPRINT (73)
// Control:		lara_as_sprint()
void lara_col_sprint(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;
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
		return;

	if (LaraDeflectEdge(item, coll))
	{
		item->pos.zRot = 0;

		if (coll->HitTallObject || TestLaraWall(item, 256, 0, -640) != SPLAT_COLL::NONE)
		{
			item->goalAnimState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->animNumber]))
			{
				item->currentAnimState = LS_SPLAT;
				return;
			}
		}

		LaraCollideStop(item, coll);
	}

	if (TestLaraVault(item, coll))
		return;

	if (TestLaraStep(coll) &&
		coll->CollisionType != CT_FRONT)
	{
		DoLaraStep(item, coll);

		return;
	}
}

// State:		LS_SPRINT_ROLL (74)
// Collision:	lara_col_sprint_roll()
void lara_as_sprint_roll(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN)
			info->turnRate = -LARA_SLOW_TURN;

		DoLaraLean(item, coll, -(LARA_LEAN_MAX * 3) / 5, LARA_LEAN_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN)
			info->turnRate = LARA_SLOW_TURN;

		DoLaraLean(item, coll, (LARA_LEAN_MAX * 3) / 5, LARA_LEAN_RATE);
	}

	// TODO: What?
	if (item->goalAnimState != LS_DEATH &&
		item->goalAnimState != LS_IDLE &&
		item->goalAnimState != LS_RUN_FORWARD &&
		item->fallspeed > LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
	}
}

void lara_col_sprint_roll(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 74*/
	/*state code: lara_as_sprint_roll*/
	if (item->speed < 0)
		info->moveAngle = item->pos.yRot + ANGLE(180);
	else
		info->moveAngle = item->pos.yRot;

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;

	coll->Setup.SlopesAreWalls = true;

	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);
	LaraDeflectEdgeJump(item, coll);

	if (!LaraFallen(item, coll))
	{
		if (item->speed < 0)
			info->moveAngle = item->pos.yRot;

		if (coll->Middle.Floor <= 0 && item->fallspeed > 0)
		{
			if (LaraLandedBad(item, coll))
			{
				item->goalAnimState = LS_DEATH;
			}
			else if (info->waterStatus == LW_WADE || !(TrInput & IN_FORWARD) || TrInput & IN_WALK)
			{
				item->goalAnimState = LS_IDLE;
			}
			else
			{
				item->goalAnimState = LS_RUN_FORWARD;
			}

			item->gravityStatus = false;
			item->fallspeed = 0;
			item->pos.yPos += coll->Middle.Floor;
			item->speed = 0;

			AnimateLara(item);
		}

		ShiftItem(item, coll);

		if (coll->Middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->Middle.Floor;
	}
}
