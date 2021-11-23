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
		info->gunStatus = LG_NO_ARMS;

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
// Collision:	lara_col_walk()
void lara_as_walk(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->jumpCount++;
	if (info->jumpCount > LARA_JUMP_TIME / 2 + 2) // TODO: Remove the "+ 2" when anim blending becomes a feature; right now, it is a temporary measure to avoid stuttering. @Sezz 2021.11.19 
		info->jumpCount = LARA_JUMP_TIME / 2 + 2;

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

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN)
			info->turnRate = LARA_SLOW_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);
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

	item->goalAnimState = LS_STOP;
}

// State:		LA_WALK_FORWARD (0)
// Control:		lara_as_walk_forward()
void lara_col_walk(ITEM_INFO* item, COLL_INFO* coll)
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
// Collision:	lara_col_run()
void lara_as_run(ITEM_INFO* item, COLL_INFO* coll)
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

	// TODO: Do something about wade checks someday. @Sezz 2021.10.17

	// Pseudo action queue which makes JUMP input take complete precedence.
	// Creates a committal lock to perform a forward jump when JUMP is pressed and released while allowJump isn't true yet.
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
		(info->gunStatus == LG_NO_ARMS || !IsStandingWeapon(info->gunType)) &&
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

	item->goalAnimState = LS_STOP;
}

// State:		LS_RUN_FORWARD (1)
// Control:		lara_as_run()
void lara_col_run(ITEM_INFO* item, COLL_INFO* coll)
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

	// LEGACY step
	//if (coll->Front.Floor == NO_HEIGHT || coll->Front.Floor < -STEPUP_HEIGHT || coll->Front.Floor >= -STEP_SIZE / 2)
	//{
	//	coll->Middle.Floor = 0;
	//}
	//else
	//{
	//	item->goalAnimState = LS_STEP_UP;
	//	GetChange(item, &g_Level.Anims[item->animNumber]);
	//}
	//if (coll->Middle.Floor < 50)
	//{
	//	if (coll->Middle.Floor != NO_HEIGHT)
	//		item->pos.yPos += coll->Middle.Floor;
	//}
	//else
	//{
	//	item->goalAnimState = LS_STEP_DOWN; // for theoretical running stepdown anims, not in default anims
	//	if (GetChange(item, &g_Level.Anims[item->animNumber]))
	//		item->pos.yPos += coll->Middle.Floor; // move Lara to middle.Floor
	//	else
	//		item->pos.yPos += 50; // do the default aligment
	//}
}

// State:		LS_STOP (2)
// Collision:	lara_col_stop()
void lara_as_stop(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = ((TestLaraSwamp(item) && info->waterStatus == LW_WADE) || item->animNumber == LA_SWANDIVE_ROLL) ? false : true;

	// TODO: Hardcoding. @Sezz 2021.09.28
	if (item->animNumber != LA_SPRINT_TO_STAND_RIGHT &&
		item->animNumber != LA_SPRINT_TO_STAND_LEFT)
		StopSoundEffect(SFX_TR4_LARA_SLIPPING);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	// TODO: Hardcoding. @Sezz 2021.09.28
	// Handles waterskin and clockwork beetle.
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

	// TODO: Refine this handling. Create LS_WADE_IDLE state? Might complicate things. @Sezz 2021.09.28
	if (info->waterStatus == LW_WADE)
	{
		if (TestLaraSwamp(item))
			LaraSwampStop(item, coll);
		else [[likely]]
			LaraWadeStop(item, coll);

		return;
	}

	if (TrInput & IN_JUMP &&
		coll->Middle.Ceiling < -LARA_HEADROOM * 0.7f)
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
		(info->gunStatus == LG_NO_ARMS || !IsStandingWeapon(info->gunType)))
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
	// TODO: Create new LS_WADE_BACK state? Its function would make a direct call to lara_as_back(). @Sezz 2021.06.27
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
			item->goalAnimState = LS_HOP_BACK;

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
			(info->gunStatus == LG_DRAW_GUNS && info->gunType != WEAPON_FLARE)) // Why are these weapons??? @Sezz 2021.10.10
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

	// TODO: LUA.
	// TODO: Without animation blending, the AFK pose animation's
	// movement lock will be rather obnoxious.
	// TODO: Adding some idle breathing would be nice. @Sezz 2021.10.31
	info->NewAnims.Pose = false;

	if (info->poseCount >= LARA_POSE_TIME &&
		TestLaraPose(item, coll) &&
		info->NewAnims.Pose)
	{
		item->goalAnimState = LS_POSE;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// TODO: Future-proof for raising water.
// TODO: See if making these into true states would be beneficial. @Sezz 2021.10.13
// Pseudo-state for idling in wade-height water.
void LaraWadeStop(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_JUMP)
	{
		item->goalAnimState = LS_JUMP_PREPARE;

		return;
	}

	if (TrInput & IN_FORWARD &&
		coll->CollisionType != CT_FRONT &&
		coll->CollisionType != CT_TOP_FRONT)
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

	item->goalAnimState = LS_STOP;
}

// Pseudo-state for idling in swamps.
void LaraSwampStop(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_FORWARD &&
		coll->CollisionType != CT_FRONT &&
		coll->CollisionType != CT_TOP_FRONT)
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

	item->goalAnimState = LS_STOP;
}

// State:		LS_STOP (2)
// Control:		lara_as_stop()
void lara_col_stop(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = TestLaraSwamp(item) ? NO_BAD_POS : STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.SlopesArePits = true;
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

	// TODO: Vaulting from this state.

	LaraSnapToHeight(item, coll);
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
			item->goalAnimState = LS_STOP;

			return;
		}

		item->goalAnimState = LS_POSE;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// State:		LS_POSE (4)
// Control:		lara_as_pose()
void lara_col_pose(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_stop(item, coll);
}

// State:		LS_HOP_BACK (5)
// Collision:	lara_col_fastback()
void lara_as_fastback(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_MED_TURN)
			info->turnRate = -LARA_MED_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 2, LARA_LEAN_RATE / 2);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_MED_TURN)
			info->turnRate = LARA_MED_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 2, LARA_LEAN_RATE / 2);
	}

	if (TrInput & IN_ROLL)
	{
		item->goalAnimState = LS_ROLL_FORWARD;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// State:		LS_HOP_BACK (5)
// Control:		lara_as_fastback()
void lara_col_fastback(ITEM_INFO* item, COLL_INFO* coll)
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

	if (coll->Middle.Floor > STEPUP_HEIGHT / 2)
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
// Collision:	lara_col_turn_r()
void lara_as_turn_r(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = (TestLaraSwamp(item) && info->waterStatus == LW_WADE) ? false : true;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	// TODO: This can't be anywhere below the run dispatch because a test to prevent forward movement without embedding currently can't exist. @Sezz 2021.11.12
	info->turnRate += LARA_TURN_RATE;

	if (info->waterStatus == LW_WADE)
	{
		if (TestLaraSwamp(item))
			LaraSwampTurnRight(item, coll);
		else [[likely]]
			LaraWadeTurnRight(item, coll);

		return;
	}

	if (TrInput & IN_JUMP &&
		coll->Middle.Ceiling < -LARA_HEADROOM * 0.7f)
	{
		item->goalAnimState = LS_JUMP_PREPARE;

		return;
	}

	if (TrInput & IN_ROLL)
	{
		item->goalAnimState = LS_ROLL_FORWARD;

		return;
	}

	if ((TrInput & IN_DUCK/* || TestLaraKeepCrouched(coll)*/) &&
		(info->gunStatus == LG_NO_ARMS || !IsStandingWeapon(info->gunType)))
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
			item->goalAnimState = LS_HOP_BACK;

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

	// TODO: Lara can get locked in the turn right/left animation when, holding forward against a wall,
	// the player presses and holds the button to turn the opposite way. @Sezz 2021.10.16
	if (TrInput & IN_RIGHT)
	{
		if (info->turnRate < 0)
			info->turnRate = 0;
		else if (info->turnRate > (LARA_MED_TURN + ANGLE(1.0f)))
			info->turnRate = LARA_MED_TURN + ANGLE(1.0f);

		if (TrInput & IN_WALK) // TODO: This hasn't worked since TR1.
		{
			if (info->turnRate > LARA_SLOW_TURN)
				info->turnRate = LARA_SLOW_TURN;

			item->goalAnimState = LS_TURN_RIGHT_SLOW;
		}
		else if (info->turnRate > LARA_SLOW_TURN)
			item->goalAnimState = LS_TURN_RIGHT_FAST;
		else [[likely]]
			item->goalAnimState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// Pseudo-state for turning right in wade-height water.
void LaraWadeTurnRight(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (TrInput & IN_JUMP)
	{
		item->goalAnimState = LS_JUMP_PREPARE;

		return;
	}

	if (TrInput & IN_FORWARD &&
		coll->CollisionType != CT_FRONT &&
		coll->CollisionType != CT_TOP_FRONT)
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
		if (info->turnRate > (LARA_SLOW_TURN + ANGLE(1.5f)))
			info->turnRate = LARA_SLOW_TURN + ANGLE(1.5f);

		item->goalAnimState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// Pseudo-state for turning right in swamps.
void LaraSwampTurnRight(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (TrInput & IN_FORWARD &&
		coll->CollisionType != CT_FRONT &&
		coll->CollisionType != CT_TOP_FRONT)
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
		if (info->turnRate > LARA_SLOW_TURN / 2)
			info->turnRate = LARA_SLOW_TURN / 2;

		item->goalAnimState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// State:		LS_TURN_RIGHT_SLOW (6)
// Control:		lara_as_turn_r()
void lara_col_turn_r(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_stop(item, coll);
}

// State:		LS_TURN_LEFT_SLOW (7)
// Collision:	lara_col_turn_l()
void lara_as_turn_l(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = (TestLaraSwamp(item) && info->waterStatus == LW_WADE) ? false : true;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	info->turnRate -= LARA_TURN_RATE;

	if (info->waterStatus == LW_WADE)
	{
		if (TestLaraSwamp(item))
			LaraSwampTurnLeft(item, coll);
		else [[likely]]
			LaraWadeTurnLeft(item, coll);

		return;
	}

	if (TrInput & IN_JUMP &&
		coll->Middle.Ceiling < -LARA_HEADROOM * 0.7f)
	{
		item->goalAnimState = LS_JUMP_PREPARE;

		return;
	}

	if (TrInput & IN_ROLL)
	{
		item->goalAnimState = LS_ROLL_FORWARD;

		return;
	}

	if ((TrInput & IN_DUCK) &&
		(info->gunStatus == LG_NO_ARMS || !IsStandingWeapon(info->gunType)))
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
			item->goalAnimState = LS_HOP_BACK;

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
		if (info->turnRate > 0)
			info->turnRate = 0;
		else if (info->turnRate < -(LARA_MED_TURN + ANGLE(1.0f)))
			info->turnRate = -(LARA_MED_TURN + ANGLE(1.0f));

		if (TrInput & IN_WALK)
		{
			if (info->turnRate < -LARA_SLOW_TURN)
				info->turnRate = -LARA_SLOW_TURN;

			item->goalAnimState = LS_TURN_LEFT_SLOW;
		}
		else if (info->turnRate < -LARA_MED_TURN)
			item->goalAnimState = LS_TURN_LEFT_FAST;
		else [[likely]]
			item->goalAnimState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// Pseudo-state for turning left in wade-height water.
void LaraWadeTurnLeft(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (TrInput & IN_JUMP)
	{
		item->goalAnimState = LS_JUMP_PREPARE;

		return;
	}

	if (TrInput & IN_FORWARD &&
		coll->CollisionType != CT_FRONT &&
		coll->CollisionType != CT_TOP_FRONT)
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
		if (info->turnRate < -(LARA_SLOW_TURN + ANGLE(1.5f)))
			info->turnRate = -(LARA_SLOW_TURN + ANGLE(1.5f));

		item->goalAnimState = LS_TURN_LEFT_SLOW;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// Pseudo-state for turning left in swamps.
void LaraSwampTurnLeft(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (TrInput & IN_FORWARD &&
		coll->CollisionType != CT_FRONT &&
		coll->CollisionType != CT_TOP_FRONT)
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
		if (info->turnRate < -LARA_SLOW_TURN / 2)
			info->turnRate = -LARA_SLOW_TURN / 2;

		item->goalAnimState = LS_TURN_LEFT_SLOW;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// State:		LS_TURN_LEFT_SLOW (7)
// Control:		lara_as_turn_l()
void lara_col_turn_l(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_turn_r(item, coll);
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
// Collision:	lara_col_back()
void lara_as_back(ITEM_INFO* item, COLL_INFO* coll)
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
		LaraSwampWalkBack(item, coll);

		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN)
			info->turnRate = -LARA_SLOW_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 2);

	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN)
			info->turnRate = LARA_SLOW_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 2);
	}

	if (TrInput & IN_BACK &&
		(TrInput & IN_WALK || info->waterStatus == LW_WADE))
	{
		item->goalAnimState = LS_WALK_BACK;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// Pseudo-state for walking back in swamps.
void LaraSwampWalkBack(ITEM_INFO* item, COLL_INFO* coll)
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

	item->goalAnimState = LS_STOP;
}

// State:		LS_WALK_BACK (16)
// Control:		lara_as_back()
void lara_col_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot + ANGLE(180);
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.BadHeightDown = info->waterStatus == LW_WADE ? NO_BAD_POS : STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesArePits = true;
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

	// TODO: Wade handling. @Sezz 2021.10.13

	if (TrInput & IN_JUMP &&
		coll->Middle.Ceiling < -LARA_HEADROOM * 0.7f)
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

	if ((TrInput & IN_DUCK/* || TestLaraKeepCrouched(coll)*/) &&
		(info->gunStatus == LG_NO_ARMS || !IsStandingWeapon(info->gunType)) &&
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
			item->goalAnimState = LS_HOP_BACK;

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
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate < LARA_MED_TURN)
			info->turnRate = LARA_MED_TURN;
		else if (info->turnRate > LARA_FAST_TURN)
			info->turnRate = LARA_FAST_TURN;

		item->goalAnimState = LS_TURN_RIGHT_FAST;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// State:		LS_TURN_RIGHT_FAST (20)
// Control:		lara_as_turn_right_fast()
void lara_col_turn_right_fast(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_stop(item, coll);
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

	if (TrInput & IN_JUMP &&
		coll->Middle.Ceiling < -LARA_HEADROOM * 0.7f)
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

	if ((TrInput & IN_DUCK/* || TestLaraKeepCrouched(coll)*/) &&
		(info->gunStatus == LG_NO_ARMS || !IsStandingWeapon(info->gunType)) &&
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
			item->goalAnimState = LS_HOP_BACK;

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
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate > -LARA_MED_TURN)
			info->turnRate = -LARA_MED_TURN;
		else if (info->turnRate < -LARA_FAST_TURN)
			info->turnRate = -LARA_FAST_TURN;

		item->goalAnimState = LS_TURN_LEFT_FAST;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// State:		LS_TURN_LEFT_FAST (152)
// Control:		lara_as_turn_left_fast()
void lara_col_turn_left_fast(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_stop(item, coll);
}

// State:		LS_SIDESTEP_RIGHT (21)
// Collision:	lara_col_stepright()
void lara_as_stepright(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;

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

	item->goalAnimState = LS_STOP;
}

// State:		LS_STEP_RIGHT (21)
// Control:		lara_as_stepright()
void lara_col_stepright(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot + ANGLE(90.0f);
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.BadHeightDown = (info->waterStatus == LW_WADE) ? NO_BAD_POS : STEP_SIZE / 2;
	coll->Setup.BadHeightUp = -STEP_SIZE / 2;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesArePits = true;
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
		LaraCollideStop(item, coll);

	if (TestLaraStep(coll) || TestLaraSwamp(item))
	{
		DoLaraStep(item, coll);

		return;
	}
}

// State:		LS_SIDESTEP_LEFT (22)
// Collision:	lara_col_stepleft()
void lara_as_stepleft(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;

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

	item->goalAnimState = LS_STOP;
}

// State:		LS_STEP_LEFT (22)
// Control:		lara_as_stepleft()
void lara_col_stepleft(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot - ANGLE(90.0f);
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.BadHeightDown = (info->waterStatus == LW_WADE) ? NO_BAD_POS : STEP_SIZE / 2;
	coll->Setup.BadHeightUp = -STEP_SIZE / 2;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesArePits = true;
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
		LaraCollideStop(item, coll);

	if (TestLaraStep(coll) || TestLaraSwamp(item))
	{
		DoLaraStep(item, coll);

		return;
	}
}

// State:		LS_ROLL_BACK(23)
// Collision:	lara_col_roll2()
void lara_as_roll2(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;

	if (TrInput & IN_ROLL)
	{
		item->goalAnimState = LS_ROLL_BACK;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// State:		LS_ROLL_BACK (23)
// Control:		lara_as_roll2()
void lara_col_roll2(ITEM_INFO* item, COLL_INFO* coll)
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
// Collision:	lara_col_roll()
void lara_as_roll(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;

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

	item->goalAnimState = LS_STOP;
}

// State:		LS_ROLL_FORWARD (45)
// Control:		lara_as_roll()
void lara_col_roll(ITEM_INFO* item, COLL_INFO* coll)
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

void lara_as_gymnast(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 54*/
	/*collision: lara_default_col*/
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
}

// State:		LS_WADE_FORWARD (65)
// Collision:	lara_col_wade()
void lara_as_wade(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = (TestLaraSwamp(item) && info->waterStatus == LW_WADE) ? false : true;
	Camera.targetElevation = -ANGLE(22.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;

		return;
	}

	if (TestLaraSwamp(item))
	{
		LaraWadeSwamp(item, coll);

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

	item->goalAnimState = LS_STOP;
}

// Pseudo-state for wading in a swamp.
void LaraWadeSwamp(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN / 2)
			info->turnRate = -LARA_SLOW_TURN / 2;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 5 * 3, LARA_LEAN_RATE / 3);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN / 2)
			info->turnRate = LARA_SLOW_TURN / 2;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 5 * 3, LARA_LEAN_RATE / 3);
	}

	if (TrInput & IN_FORWARD)
	{
		item->goalAnimState = LS_WADE_FORWARD;
		
		return;
	}

	item->goalAnimState = LS_STOP;
}

// State:		LS_WADE_FORWARD (65)
// Control:		lara_as_wade()
void lara_col_wade(ITEM_INFO* item, COLL_INFO* coll)
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

	// LEGACY step
	/*if (coll->Middle.Floor >= -STEPUP_HEIGHT && coll->Middle.Floor < -STEP_SIZE / 2 && !TestLaraSwamp(item))
	{
		item->goalAnimState = LS_STEP_UP;
		GetChange(item, &g_Level.Anims[item->animNumber]);
	}

	if (coll->Middle.Floor >= 50 && !TestLaraSwamp(item))
	{
		item->pos.yPos += 50;
		return;
	}
	
	LaraSnapToHeight(item, coll);*/
}

// State:		LS_SPRINT (73)
// Collision:	lara_col_dash()
void lara_as_dash(ITEM_INFO* item, COLL_INFO* coll)
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

	if ((TrInput & IN_DUCK/* || TestLaraKeepCrouched(coll)*/) &&
		(info->gunStatus == LG_NO_ARMS || !IsStandingWeapon(info->gunType)))
	{
		item->goalAnimState = LS_CROUCH_IDLE;

		return;
	}

	// TODO: Supposedly there is a bug wherein sprinting into the boundary between shallow and deep water
	// under some condition allows Lara to run around in the water room. Investigate. @Sezz 2021.09.29
	if (TrInput & IN_FORWARD)
	{
		if (info->waterStatus == LW_WADE)
			item->goalAnimState = LS_RUN_FORWARD;	// TODO: Dispatch to wade forward state. @Sezz 2021.09.29
		else if (TrInput & IN_WALK)
			item->goalAnimState = LS_WALK_FORWARD;
		else if (TrInput & IN_SPRINT && info->sprintTimer > 0) [[likely]]
			item->goalAnimState = LS_SPRINT;
		else
			item->goalAnimState = LS_RUN_FORWARD;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// State:		LS_SPRINT (73)
// Control:		lara_as_dash()
void lara_col_dash(ITEM_INFO* item, COLL_INFO* coll)
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

	if (TestLaraStep(coll))
	{
		DoLaraStep(item, coll);

		return;
	}

	// LEGACY step
	//if (coll->Middle.Floor >= -STEPUP_HEIGHT && coll->Middle.Floor < -STEP_SIZE / 2)
	//{
	//	item->goalAnimState = LS_STEP_UP;
	//	GetChange(item, &g_Level.Anims[item->animNumber]);
	//}
	//
	//if (coll->Middle.Floor < 50)
	//{
	//	if (coll->Middle.Floor != NO_HEIGHT)
	//		item->pos.yPos += coll->Middle.Floor;
	//}
	//else
	//{
	//	item->goalAnimState = LS_STEP_DOWN; // for theoretical sprint stepdown anims, not in default anims
	//	if (GetChange(item, &g_Level.Anims[item->animNumber]))
	//		item->pos.yPos += coll->Middle.Floor; // move Lara to middle.Floor
	//	else
	//		item->pos.yPos += 50; // do the default aligment
	//}
}

// State:		LS_SPRINT_ROLL (74)
// Collision:	lara_col_dashdive()
void lara_as_dashdive(ITEM_INFO* item, COLL_INFO* coll)
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
		item->goalAnimState != LS_STOP &&
		item->goalAnimState != LS_RUN_FORWARD &&
		item->fallspeed > LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
	}
}

void lara_col_dashdive(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	/*state 74*/
	/*state code: lara_as_dashdive*/
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
				item->goalAnimState = LS_STOP;
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
