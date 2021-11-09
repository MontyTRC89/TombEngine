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
	Lara.moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesArePits = true;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = Lara.moveAngle;
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
	Lara.look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

	if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd - 1)
	{
		Lara.gunStatus = LG_NO_ARMS;

		if (UseForcedFixedCamera)
			UseForcedFixedCamera = 0;
	}
}

void lara_as_controlledl(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.look = false;
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
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	// TODO: Implement item alignment properly. @Sezz 2021.11.01
	if (Lara.isMoving)
		return;

	// TODO: If stopping and holding WALK without FORWARD, Lara can't turn. @Sezz 2021.10.09
	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_SLOW_TURN)
			Lara.turnRate = -LARA_SLOW_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, 10);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_SLOW_TURN)
			Lara.turnRate = LARA_SLOW_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, 10);
	}

	if (TrInput & IN_FORWARD)
	{
		if (Lara.waterStatus == LW_WADE)
			item->goalAnimState = LS_WADE_FORWARD;
		else if (TrInput & IN_WALK) [[likely]]
			item->goalAnimState = LS_WALK_FORWARD;
		else
			item->goalAnimState = LS_RUN_FORWARD;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// LEGACY
void old_lara_as_walk(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 0*/
	/*collision: lara_col_walk*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (!Lara.isMoving)
	{
		if (TrInput & IN_LEFT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -LARA_SLOW_TURN)
				Lara.turnRate = -LARA_SLOW_TURN;
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > LARA_SLOW_TURN)
				Lara.turnRate = LARA_SLOW_TURN;
		}

		if (TrInput & IN_FORWARD)
		{
			if (Lara.waterStatus == LW_WADE)
			{
				item->goalAnimState = LS_WADE_FORWARD;
			}
			else if (TrInput & IN_WALK)
			{
				item->goalAnimState = LS_WALK_FORWARD;
			}
			else
			{
				item->goalAnimState = LS_RUN_FORWARD;
			}
		}
		else
		{
			item->goalAnimState = LS_STOP;
		}
	}
}

// State:		LA_WALK_FORWARD (0)
// Control:		lara_as_walk_forward()
void lara_col_walk(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot;
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.BadHeightDown = STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.SlopesArePits = true;
	coll->Setup.DeathFlagIsPit = true;
	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);

		return;
	}

	if (LaraDeflectEdge(item, coll))
	{
		item->goalAnimState = LS_SPLAT;
		if (GetChange(item, &g_Level.Anims[item->animNumber]))
			return;

		LaraCollideStop(item, coll);
	}

	if (TestLaraFall(coll))
	{
		SetLaraFallState(item);

		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	if (TestLaraVault(item, coll))
		return;

	if (TestLaraStep(coll) &&
		coll->CollisionType != CT_FRONT)
	{
		DoLaraStep(item, coll);

		return;
	}

	// LEGACY step
	/*if (coll->Middle.Floor > STEP_SIZE / 2)
	{
		if (coll->Front.Floor == NO_HEIGHT || coll->Front.Floor <= STEP_SIZE / 2)
		{
			coll->Middle.Floor = 0;
		}
		else
		{
			item->goalAnimState = LS_STEP_DOWN;
			GetChange(item, &g_Level.Anims[item->animNumber]);
		}
	}
	if (coll->Middle.Floor >= -STEPUP_HEIGHT && coll->Middle.Floor < -STEP_SIZE / 2)
	{
		if (coll->Front.Floor == NO_HEIGHT ||
			coll->Front.Floor < -STEPUP_HEIGHT ||
			coll->Front.Floor >= -STEP_SIZE / 2)
		{
			coll->Middle.Floor = 0;
		}
		else
		{
			item->goalAnimState = LS_STEP_UP;
			GetChange(item, &g_Level.Anims[item->animNumber]);
		}
	}*/
}

// LEGACY
void old_lara_col_walk(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 0*/
	/*state code: lara_as_walk*/
	item->gravityStatus = false;
	item->fallspeed = 0;

	Lara.moveAngle = item->pos.yRot;

	coll->Setup.BadHeightDown = STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;

	coll->Setup.SlopesAreWalls = true;
	coll->Setup.SlopesArePits = true;
	coll->Setup.DeathFlagIsPit = 1;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (!LaraHitCeiling(item, coll) && !TestLaraVault(item, coll))
	{
		if (LaraDeflectEdge(item, coll))
		{
			item->goalAnimState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->animNumber]))
				return;

			LaraCollideStop(item, coll);
		}

		if (!LaraFallen(item, coll))
		{
			if (coll->Middle.Floor > STEP_SIZE / 2)
			{
				if (coll->Front.Floor == NO_HEIGHT || coll->Front.Floor <= STEP_SIZE / 2)
				{
					coll->Middle.Floor = 0;
				}
				else
				{
					item->goalAnimState = LS_STEP_DOWN;
					GetChange(item, &g_Level.Anims[item->animNumber]);
				}
			}
			if (coll->Middle.Floor >= -STEPUP_HEIGHT && coll->Middle.Floor < -STEP_SIZE / 2)
			{
				if (coll->Front.Floor == NO_HEIGHT ||
					coll->Front.Floor < -STEPUP_HEIGHT ||
					coll->Front.Floor >= -STEP_SIZE / 2)
				{
					coll->Middle.Floor = 0;
				}
				else
				{
					item->goalAnimState = LS_STEP_UP;
					GetChange(item, &g_Level.Anims[item->animNumber]);
				}
			}

			if (!TestLaraSlide(item, coll) && coll->Middle.Floor != NO_HEIGHT && coll->CollisionType != CT_FRONT)
				item->pos.yPos += coll->Middle.Floor;
		}
	}
}

// State:		LS_RUN_FORWARD (1)
// Collision:	lara_col_run()
void lara_as_run(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_FAST_TURN)
			Lara.turnRate = -LARA_FAST_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, 7);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_FAST_TURN)
			Lara.turnRate = LARA_FAST_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX, 7);
	}

	static bool allowJump = false;

	if (item->animNumber == LA_STAND_TO_RUN)
		allowJump = false;
	else if (item->animNumber == LA_RUN)
	{
		if (item->frameNumber == 4)
			allowJump = true;
	}
	else
		allowJump = true;

	// TODO: Do something about wade checks. @Sezz 2021.10.17

	// Pseudo action queue which makes JUMP input take complete precedence.
	// Creates a committal lock to perform a forward jump when JUMP is pressed and released while allowJump isn't true yet.
	static bool commitToJump = false;

	if ((TrInput & IN_JUMP || commitToJump) &&
		!item->gravityStatus &&
		Lara.waterStatus != LW_WADE)
	{
		commitToJump = TrInput & IN_FORWARD;

		if (allowJump)
		{
			commitToJump = false;
			item->goalAnimState = LS_JUMP_FORWARD;
		}

		return;
	}

	if (TrInput & IN_SPRINT &&
		DashTimer &&
		Lara.waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_SPRINT;

		return;
	}

	if (TrInput & IN_ROLL &&
		Lara.waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_ROLL_FORWARD;

		return;
	}

	if (TrInput & IN_DUCK &&
		(Lara.gunStatus == LG_NO_ARMS || !IsStandingWeapon(Lara.gunType)) &&
		Lara.waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_CROUCH_IDLE;

		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (Lara.waterStatus == LW_WADE)
			item->goalAnimState = LS_WADE_FORWARD;
		else if (TrInput & IN_WALK)
			item->goalAnimState = LS_WALK_FORWARD;
		else [[likely]]
			item->goalAnimState = LS_RUN_FORWARD;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// LEGACY
void old_lara_as_run(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 1*/
	/*collision: lara_col_run*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;
		return;
	}

	if (TrInput & IN_ROLL)
	{
		item->animNumber = LA_ROLL_180_START;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 2;
		item->currentAnimState = LS_ROLL_FORWARD;
		item->goalAnimState = LS_STOP;
		return;
	}

	if (TrInput & IN_SPRINT && DashTimer)
	{
		item->goalAnimState = LS_SPRINT;
		return;
	}

	if (TrInput & IN_DUCK &&
		Lara.waterStatus != LW_WADE &&
		(Lara.gunStatus == LG_NO_ARMS ||
			Lara.gunType == WEAPON_NONE ||
			Lara.gunType == WEAPON_PISTOLS ||
			Lara.gunType == WEAPON_REVOLVER ||
			Lara.gunType == WEAPON_UZI ||
			Lara.gunType == WEAPON_FLARE))
	{
		item->goalAnimState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_FAST_TURN)
			Lara.turnRate = -LARA_FAST_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, 7);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_FAST_TURN)
			Lara.turnRate = LARA_FAST_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX, 7);
	}

	static bool doJump = false;

	if (item->animNumber == LA_STAND_TO_RUN)
	{
		doJump = false;
	}
	else if (item->animNumber == LA_RUN)
	{
		if (item->frameNumber == 4)
			doJump = true;
	}
	else
	{
		doJump = true;
	}

	if (TrInput & IN_JUMP && doJump && !item->gravityStatus)
	{
		item->goalAnimState = LS_JUMP_FORWARD;
	}
	else if (TrInput & IN_FORWARD)
	{
		if (Lara.waterStatus == LW_WADE)
		{
			item->goalAnimState = LS_WADE_FORWARD;
		}
		else
		{
			if (TrInput & IN_WALK)
				item->goalAnimState = LS_WALK_FORWARD;
			else
				item->goalAnimState = LS_RUN_FORWARD;
		}
	}
	else
	{
		item->goalAnimState = LS_STOP;
	}
}

// State:		LS_RUN_FORWARD (1)
// Control:		lara_as_run()
void lara_col_run(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);
	LaraResetGravityStatus(item, coll);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);

		return;
	}

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

	if (TestLaraFall(coll))
	{
		SetLaraFallState(item);

		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	if (TestLaraVault(item, coll))
		return;

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

// LEGACY
void old_lara_col_run(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 1*/
	/*state code: lara_col_run*/
	Lara.moveAngle = item->pos.yRot;

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = Lara.moveAngle;

	GetCollisionInfo(coll, item);
	LaraResetGravityStatus(item, coll);

	if (!LaraHitCeiling(item, coll) && !TestLaraVault(item, coll))
	{
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

		if (!LaraFallen(item, coll))
		{
			if (coll->Middle.Floor >= -STEPUP_HEIGHT && coll->Middle.Floor < -STEP_SIZE / 2)
			{
				if (coll->Front.Floor == NO_HEIGHT || coll->Front.Floor < -STEPUP_HEIGHT || coll->Front.Floor >= -STEP_SIZE / 2)
				{
					coll->Middle.Floor = 0;
				}
				else
				{
					item->goalAnimState = LS_STEP_UP;
					GetChange(item, &g_Level.Anims[item->animNumber]);
				}
			}

			if (!TestLaraSlide(item, coll) && coll->CollisionType != CT_FRONT)
			{
				if (coll->Middle.Floor < 50)
				{
					if (coll->Middle.Floor != NO_HEIGHT)
						item->pos.yPos += coll->Middle.Floor;
				}
				else
				{
					item->goalAnimState = LS_STEP_DOWN; // for theoretical running stepdown anims, not in default anims
					if (GetChange(item, &g_Level.Anims[item->animNumber]))
						item->pos.yPos += coll->Middle.Floor; // move Lara to middle.Floor
					else
						item->pos.yPos += 50; // do the default aligment
				}
			}
		}
	}
}

// State:		LS_STOP (2)
// Collision:	lara_col_stop()
void lara_as_stop(ITEM_INFO* item, COLL_INFO* coll)
{
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

	if (TrInput & IN_LOOK)
		LookUpDown();

	// Permit turning when Lara is stationary and cannot dispatch into a true turn.
	if (TrInput & IN_LEFT &&
		!(TrInput & IN_JUMP))		// JUMP locks y rotation.
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_SLOW_TURN)
			Lara.turnRate = -LARA_SLOW_TURN;
	}
	else if (TrInput & IN_RIGHT &&
		!(TrInput & IN_JUMP))
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_SLOW_TURN)
			Lara.turnRate = LARA_SLOW_TURN;
	}

	// TODO: Refine this handling. Create LS_WADE_IDLE state? Might complicate things. @Sezz 2021.09.28
	if (Lara.waterStatus == LW_WADE)
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
		(Lara.gunStatus == LG_NO_ARMS || !IsStandingWeapon(Lara.gunType)))
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
		if (TrInput & IN_WALK &&
			TestLaraWalkBack(item, coll))
		{
			item->goalAnimState = LS_WALK_BACK;

			return;
		}
		else if (!(TrInput & IN_WALK) &&
			TestLaraHopBack(item, coll)) [[likely]]
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
			(Lara.gunStatus == LG_READY && Lara.gunType != WEAPON_TORCH) ||
			(Lara.gunStatus == LG_DRAW_GUNS && Lara.gunType != WEAPON_FLARE)) // Why are these weapons??? @Sezz 2021.10.10
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
			(Lara.gunStatus == LG_READY && Lara.gunType != WEAPON_TORCH) ||
			(Lara.gunStatus == LG_DRAW_GUNS && Lara.gunType != WEAPON_FLARE))
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
	Lara.NewAnims.Pose = false;

	if (Lara.poseCount == LARA_POSE_TIME &&
		TestLaraPose(item, coll) &&
		Lara.NewAnims.Pose)
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

// LEGACY
void old_lara_as_stop(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 2*/
	/*collision: lara_col_stop*/
	COLL_RESULT fheight = {}; fheight.Position.Floor = NO_HEIGHT;
	COLL_RESULT rheight = {}; rheight.Position.Floor = NO_HEIGHT;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;
		return;
	}

	if (item->animNumber != LA_SPRINT_TO_STAND_RIGHT && item->animNumber != LA_SPRINT_TO_STAND_LEFT)
		StopSoundEffect(SFX_TR4_LARA_SLIPPING);

	// Handles waterskin and clockwork beetle
	if (UseSpecialItem(item))
		return;

	if (TrInput & IN_ROLL && Lara.waterStatus != LW_WADE)
	{
		item->animNumber = LA_ROLL_180_START;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 2;
		item->currentAnimState = LS_ROLL_FORWARD;
		item->goalAnimState = LS_STOP;
		return;
	}

	if (TrInput & IN_DUCK
		&& Lara.waterStatus != LW_WADE
		&& item->currentAnimState == LS_STOP
		&& (Lara.gunStatus == LG_NO_ARMS
			|| Lara.gunType == WEAPON_NONE
			|| Lara.gunType == WEAPON_PISTOLS
			|| (Lara.gunType == WEAPON_REVOLVER && !LaserSight)
			|| Lara.gunType == WEAPON_UZI
			|| Lara.gunType == WEAPON_FLARE))
	{
		item->goalAnimState = LS_CROUCH_IDLE;
		return;
	}

	item->goalAnimState = LS_STOP;

	if (TrInput & IN_LOOK)
		LookUpDown();

	if (TrInput & IN_LSTEP)
	{
		auto collFloorResult = LaraCollisionFront(item, item->pos.yRot - ANGLE(90.0f), LARA_RAD + 48);
		auto collCeilingResult = LaraCeilingCollisionFront(item, item->pos.yRot - ANGLE(90.0f), LARA_RAD + 48, LARA_HEIGHT);

		if ((collFloorResult.Position.Floor < 128 && collFloorResult.Position.Floor > -128) && !collFloorResult.Position.Slope && collCeilingResult.Position.Ceiling <= 0)
			item->goalAnimState = LS_STEP_LEFT;
	}
	else if (TrInput & IN_RSTEP)
	{
		auto collFloorResult = LaraCollisionFront(item, item->pos.yRot + ANGLE(90.0f), LARA_RAD + 48);
		auto collCeilingResult = LaraCeilingCollisionFront(item, item->pos.yRot + ANGLE(90.0f), LARA_RAD + 48, LARA_HEIGHT);

		if ((collFloorResult.Position.Floor < 128 && collFloorResult.Position.Floor > -128) && !collFloorResult.Position.Slope && collCeilingResult.Position.Ceiling <= 0)
			item->goalAnimState = LS_STEP_RIGHT;
	}
	else if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_FAST_TURN)
			Lara.turnRate = -LARA_FAST_TURN;

		item->goalAnimState = LS_TURN_LEFT_SLOW;
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_FAST_TURN)
			Lara.turnRate = LARA_FAST_TURN;

		item->goalAnimState = LS_TURN_RIGHT_SLOW;
	}

	if (TrInput & IN_FORWARD)
		fheight = LaraCollisionFront(item, item->pos.yRot, LARA_RAD + 4);
	else if (TrInput & IN_BACK)
		rheight = LaraCollisionFront(item, item->pos.yRot - ANGLE(180.0f), LARA_RAD + 4); // TR3: item->pos.yRot + ANGLE(180)?

	if (Lara.waterStatus == LW_WADE)
	{
		if (TrInput & IN_JUMP && !TestLaraSwamp(item))
			item->goalAnimState = LS_JUMP_PREPARE;

		if (TrInput & IN_FORWARD)
		{
			bool wade = false;

			if (TestLaraSwamp(item))
			{
				if (fheight.Position.Floor > -(STEPUP_HEIGHT - 1))
					wade = true;
			}
			else
			{
				if ((fheight.Position.Floor < (STEPUP_HEIGHT - 1)) && (fheight.Position.Floor > -(STEPUP_HEIGHT - 1)))
					wade = true;
			}

			if (!wade)
			{
				Lara.moveAngle = item->pos.yRot;
				coll->Setup.BadHeightDown = NO_BAD_POS;
				coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
				coll->Setup.BadCeilingHeight = 0;
				coll->Setup.SlopesAreWalls = true;
				coll->Setup.Radius = LARA_RAD + 2;
				coll->Setup.ForwardAngle = Lara.moveAngle;

				GetCollisionInfo(coll, item);
				if (TestLaraVault(item, coll))
					return;

				coll->Setup.Radius = LARA_RAD;
			}
			else
				item->goalAnimState = LS_WADE_FORWARD;
		}
		else if (TrInput & IN_BACK)
		{
			if (TestLaraSwamp(item) || (TrInput & IN_WALK))
			{
				if ((rheight.Position.Floor < (STEPUP_HEIGHT - 1)) && (rheight.Position.Floor > -(STEPUP_HEIGHT - 1)) && !rheight.Position.Slope)
					item->goalAnimState = LS_WALK_BACK;
			}
			else if (rheight.Position.Floor > -(STEPUP_HEIGHT - 1))
			{
				item->goalAnimState = LS_HOP_BACK;
			}
		}
	}
	else
	{
		if (TrInput & IN_JUMP)
		{
			if (!TestLaraSwamp(item) && coll->Middle.Ceiling < -LARA_HEADROOM * 0.7f)
				item->goalAnimState = LS_JUMP_PREPARE;
		}
		else if (TrInput & IN_FORWARD)
		{
			auto cheight = LaraCeilingCollisionFront(item, item->pos.yRot, LARA_RAD + 4, LARA_HEIGHT);

			// Don't try to move if there is slope in front
			if (fheight.Position.Slope && (fheight.Position.Floor < 0 || cheight.Position.Ceiling > 0))
				return; // item->goalAnimState = LS_STOP was removed here because it prevented Lara from rotating while still holding forward. -- Lwmte, 17.09.2021

			if (TestLaraVault(item, coll))
				return;

			if (TrInput & IN_WALK)
			{
				if (coll->CollisionType == CT_FRONT) // Never try to walk if there's collision in front
					return;
				item->goalAnimState = LS_WALK_FORWARD;
			}
			else
			{
				if (cheight.Position.Ceiling > 0) // Only try to run if there's no overhang ceiling in front
					return;
				item->goalAnimState = LS_RUN_FORWARD;
			}
		}
		else if (TrInput & IN_BACK)
		{
			if (TrInput & IN_WALK)
			{
				if ((rheight.Position.Floor < (STEPUP_HEIGHT - 1)) && (rheight.Position.Floor > -(STEPUP_HEIGHT - 1)) && !rheight.Position.Slope)
					item->goalAnimState = LS_WALK_BACK;
			}
			else if (rheight.Position.Floor > -(STEPUP_HEIGHT - 1))
			{
				item->goalAnimState = LS_HOP_BACK;
			}
		}
	}
}

// State:		LS_STOP (2)
// Control:		lara_as_stop()
void lara_col_stop(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = TestLaraSwamp(item) ? NO_BAD_POS : STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.SlopesArePits = true;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);

		return;
	}

	if (TestLaraFall(coll))
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

// LEGACY
void old_lara_col_stop(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 2*/
	/*state code: lara_as_stop*/
	Lara.moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.SlopesArePits = true;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (LaraHitCeiling(item, coll))
		return;

	if (LaraFallen(item, coll))
		return;

	if (TestLaraSlide(item, coll))
		return;

	ShiftItem(item, coll);
	LaraSnapToHeight(item, coll);
}

void lara_as_forwardjump(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 3*/
	/*collision: */
	if (item->goalAnimState == LS_SWANDIVE_START ||
		item->goalAnimState == LS_REACH)
		item->goalAnimState = LS_JUMP_FORWARD;

	if (item->goalAnimState != LS_DEATH &&
		item->goalAnimState != LS_STOP &&
		item->goalAnimState != LS_RUN_FORWARD)
	{
		if (Lara.gunStatus == LG_NO_ARMS && TrInput & IN_ACTION)
			item->goalAnimState = LS_REACH;

		if (TrInput & IN_BACK || TrInput & IN_ROLL)
			item->goalAnimState = LS_JUMP_ROLL_180;

		if (Lara.gunStatus == LG_NO_ARMS && TrInput & IN_WALK)
			item->goalAnimState = LS_SWANDIVE_START;

		if (item->fallspeed > LARA_FREEFALL_SPEED)
			item->goalAnimState = LS_FREEFALL;
	}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;

		if (Lara.turnRate < -LARA_JUMP_TURN)
			Lara.turnRate = -LARA_JUMP_TURN;
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;

		if (Lara.turnRate > LARA_JUMP_TURN)
			Lara.turnRate = LARA_JUMP_TURN;
	}
}

void lara_col_forwardjump(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 3*/
	/*state code: lara_as_forwardjump*/
	if (item->speed < 0)
		Lara.moveAngle = item->pos.yRot + ANGLE(180);
	else
		Lara.moveAngle = item->pos.yRot;

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);
	LaraDeflectEdgeJump(item, coll);

	if (item->speed < 0)
		Lara.moveAngle = item->pos.yRot;

	if (item->fallspeed > 0 && (coll->Middle.Floor <= 0 || TestLaraSwamp(item)))
	{
		if (LaraLandedBad(item, coll))
		{
			item->goalAnimState = LS_DEATH;
		}
		else
		{
			if (Lara.waterStatus == LW_WADE)
			{
				item->goalAnimState = LS_STOP;
			}
			else
			{
				if (TrInput & IN_FORWARD && !(TrInput & IN_STEPSHIFT))
					item->goalAnimState = LS_RUN_FORWARD;
				else
					item->goalAnimState = LS_STOP;
			}
		}

		item->gravityStatus = false;
		item->fallspeed = 0;
		item->speed = 0;

		LaraSnapToHeight(item, coll);
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
	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_MED_TURN)
			Lara.turnRate = -LARA_MED_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 2, 10);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_MED_TURN)
			Lara.turnRate = LARA_MED_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 2, 10);
	}

	if (TrInput & IN_ROLL)
	{
		item->goalAnimState = LS_ROLL_FORWARD;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// LEGACY
void old_lara_as_fastback(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state: 5*/
	/*collision: lara_col_fastback*/
	item->goalAnimState = LS_STOP;
	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_MED_TURN)
			Lara.turnRate = -LARA_MED_TURN;
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_MED_TURN)
			Lara.turnRate = LARA_MED_TURN;
	}
}

// State:		LS_HOP_BACK (5)
// Control:		lara_as_fastback()
void lara_col_fastback(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot + ANGLE(180.0f);
	item->fallspeed = 0;
	item->gravityStatus = false;
	coll->Setup.SlopesAreWalls = false;
	coll->Setup.SlopesArePits = true;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);

		return;
	}

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (coll->Middle.Floor > STEPUP_HEIGHT / 2)
	{
		SetLaraFallBackState(item);

		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	if (TestLaraStep(coll))
	{
		DoLaraStep(item, coll);

		return;
	}

	// LEGACY step
	/*if (coll->Middle.Floor <= 200)
	{
		if (LaraDeflectEdge(item, coll))
			LaraCollideStop(item, coll);

		if (!TestLaraSlide(item, coll) && coll->Middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->Middle.Floor;
	}
	else
	{
		item->fallspeed = 0;

		item->animNumber = LA_FALL_BACK;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->currentAnimState = LS_FALL_BACK;
		item->goalAnimState = LS_FALL_BACK;

		item->gravityStatus = true;
	}*/
}

// LEGACY
void old_lara_col_fastback(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state: 5*/
	/*state code: lara_as_fastback*/
	item->fallspeed = 0;
	item->gravityStatus = false;

	Lara.moveAngle = item->pos.yRot + ANGLE(180);

	coll->Setup.SlopesAreWalls = false;
	coll->Setup.SlopesArePits = true;

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (!LaraHitCeiling(item, coll))
	{
		if (coll->Middle.Floor <= 200)
		{
			if (LaraDeflectEdge(item, coll))
				LaraCollideStop(item, coll);

			if (!TestLaraSlide(item, coll) && coll->Middle.Floor != NO_HEIGHT)
				item->pos.yPos += coll->Middle.Floor;
		}
		else
		{
			item->fallspeed = 0;

			item->animNumber = LA_FALL_BACK;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = LS_FALL_BACK;
			item->goalAnimState = LS_FALL_BACK;

			item->gravityStatus = true;
		}
	}
}

// State:		LS_TURN_RIGHT_SLOW (6)
// Collision:	lara_col_turn_r()
void lara_as_turn_r(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	// TODO: This can't be anywhere below the run dispatch because tests to prevent forward/back movement currently don't exist.
	Lara.turnRate += LARA_TURN_RATE;

	if (Lara.waterStatus == LW_WADE)
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

	if ((TrInput & IN_DUCK/* || TestLaraKeepDucked(coll)*/) &&
		(Lara.gunStatus == LG_NO_ARMS || !IsStandingWeapon(Lara.gunType)))
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
		if (TrInput & IN_WALK &&
			TestLaraWalkBack(item, coll))
		{
			item->goalAnimState = LS_WALK_BACK;

			return;
		}
		else if (!(TrInput & IN_WALK) &&
			TestLaraHopBack(item, coll)) [[likely]]
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
		if (TrInput & IN_WALK) // TODO: This hasn't worked since TR1.
		{
			if (Lara.turnRate > LARA_SLOW_TURN)
				Lara.turnRate = LARA_SLOW_TURN;

			item->goalAnimState = LS_TURN_RIGHT_SLOW;
		}
		else if (Lara.turnRate > LARA_SLOW_TURN)
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
		if (Lara.turnRate > LARA_SLOW_TURN)
			Lara.turnRate = LARA_SLOW_TURN;

		item->goalAnimState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// Pseudo-state for turning right in swamps.
void LaraSwampTurnRight(ITEM_INFO* item, COLL_INFO* coll)
{
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
		if (Lara.turnRate > LARA_SLOW_TURN)
			Lara.turnRate = LARA_SLOW_TURN;

		item->goalAnimState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// LEGACY
void old_lara_as_turn_r(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state: 6*/
	/*collision: */
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;

		return;
	}

	Lara.turnRate += LARA_TURN_RATE;

	if (Lara.gunStatus != LG_READY || Lara.waterStatus == LW_WADE)
	{
		if (Lara.turnRate > LARA_SLOW_TURN)
		{
			if (TrInput & IN_WALK || Lara.waterStatus == LW_WADE)
				Lara.turnRate = LARA_SLOW_TURN;
			else
				item->goalAnimState;// = LS_TURN_FAST;
		}
	}
	else
	{
		item->goalAnimState;// = LS_TURN_FAST;
	}

	// Don't try to move forward if button isn't pressed or there's no headroom in front
	if (!(TrInput & IN_FORWARD))
	{
		if (!(TrInput & IN_RIGHT))
			item->goalAnimState = LS_STOP;

		return;
	}

	if (Lara.waterStatus == LW_WADE)
	{
		item->goalAnimState = LS_WADE_FORWARD;
	}
	else if (TrInput & IN_WALK)
	{
		item->goalAnimState = LS_WALK_FORWARD;
	}
	else
	{
		item->goalAnimState = LS_RUN_FORWARD;
	}
}

// State:		LS_TURN_RIGHT_SLOW (6)
// Control:		lara_as_turn_r()
void lara_col_turn_r(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_stop(item, coll);
}

// LEGACY
void old_lara_col_turn_r(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state: 6*/
	/*state code: lara_as_turn_r*/
	item->fallspeed = 0;
	item->gravityStatus = false;
	Lara.moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.SlopesArePits = true;
	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (coll->Middle.Floor > 100 && !TestLaraSwamp(item))
	{
		item->fallspeed = 0;
		item->animNumber = LA_FALL_START;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->currentAnimState = LS_JUMP_FORWARD;
		item->goalAnimState = LS_JUMP_FORWARD;
		item->gravityStatus = true;
		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	LaraSnapToHeight(item, coll);
}

// State:		LS_TURN_LEFT_SLOW (7)
// Collision:	lara_col_turn_l()
void lara_as_turn_l(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	Lara.turnRate -= LARA_TURN_RATE;

	if (Lara.waterStatus == LW_WADE)
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
		(Lara.gunStatus == LG_NO_ARMS || !IsStandingWeapon(Lara.gunType)))
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
		if (TrInput & IN_WALK &&
			TestLaraWalkBack(item, coll))
		{
			item->goalAnimState = LS_WALK_BACK;

			return;
		}
		else if (!(TrInput & IN_WALK) &&
			TestLaraHopBack(item, coll)) [[likely]]
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
		if (TrInput & IN_WALK)
		{
			if (Lara.turnRate < -LARA_SLOW_TURN)
				Lara.turnRate = -LARA_SLOW_TURN;

			item->goalAnimState = LS_TURN_LEFT_SLOW;
		}
		else if (Lara.turnRate < -LARA_SLOW_TURN)
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
		if (Lara.turnRate < -LARA_SLOW_TURN)
			Lara.turnRate = -LARA_SLOW_TURN;

		item->goalAnimState = LS_TURN_LEFT_SLOW;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// Pseudo-state for turning left in swamps.
void LaraSwampTurnLeft(ITEM_INFO* item, COLL_INFO* coll)
{
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
		if (Lara.turnRate < -LARA_SLOW_TURN)
			Lara.turnRate = -LARA_SLOW_TURN;

		item->goalAnimState = LS_TURN_LEFT_SLOW;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// LEGACY
void old_lara_as_turn_l(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 7*/
	/*collision: lara_col_turn_l*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	Lara.turnRate -= LARA_TURN_RATE;

	if (Lara.gunStatus != LG_READY || Lara.waterStatus == LW_WADE)
	{
		if (Lara.turnRate < -LARA_SLOW_TURN)
		{
			if (TrInput & IN_WALK || Lara.waterStatus == LW_WADE)
				Lara.turnRate = -LARA_SLOW_TURN;
			else
				item->goalAnimState;// = LS_TURN_FAST; // Dead state.
		}
	}
	else
	{
		item->goalAnimState;// = LS_TURN_FAST; // Dead state.
	}

	// Don't try to move forward if button isn't pressed or there's no headroom in front
	if (!(TrInput & IN_FORWARD) || coll->CollisionType == CT_FRONT)
	{
		if (!(TrInput & IN_LEFT))
			item->goalAnimState = LS_STOP;

		return;
	}

	if (Lara.waterStatus == LW_WADE)
	{
		item->goalAnimState = LS_WADE_FORWARD;
	}
	else if (TrInput & IN_WALK)
	{
		item->goalAnimState = LS_WALK_FORWARD;
	}
	else
	{
		item->goalAnimState = LS_RUN_FORWARD;
	}
}

// State:		LS_TURN_LEFT_SLOW (7)
// Control:		lara_as_turn_l()
void lara_col_turn_l(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_turn_r(item, coll);
}

void lara_as_death(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 8*/
	/*collision: lara_col_death*/
	Lara.look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	if (BinocularRange)
	{
		BinocularRange = 0;
		LaserSight = false;
		AlterFOV(ANGLE(80.0f));
		LaraItem->meshBits = -1;
		Lara.busy = false;
	}
}

void lara_col_death(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 8*/
	/*state code: lara_as_death*/
	StopSoundEffect(SFX_TR4_LARA_FALL);

	Lara.moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.Radius = LARA_RAD_DEATH;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);
	ShiftItem(item, coll);

	item->hitPoints = -1;
	Lara.air = -1;

	if (coll->Middle.Floor != NO_HEIGHT)
		item->pos.yPos += coll->Middle.Floor;
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
	/*state 9*/
	/*state code: lara_as_fastfall*/
	item->gravityStatus = true;

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);
	LaraSlideEdgeJump(item, coll);

	if (coll->Middle.Floor <= 0 || TestLaraSwamp(item))
	{
		if (LaraLandedBad(item, coll))
		{
			item->goalAnimState = LS_DEATH;
		}
		else
		{
			item->goalAnimState = LS_STOP;
			item->currentAnimState = LS_STOP;
			item->animNumber = LA_FREEFALL_LAND;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}

		StopSoundEffect(SFX_TR4_LARA_FALL);

		item->fallspeed = 0;
		item->gravityStatus = false;


		LaraSnapToHeight(item, coll);
	}
}

void lara_as_reach(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 11*/
	/*collision: lara_col_reach*/
	Camera.targetAngle = ANGLE(85.0f);
	if (item->fallspeed > LARA_FREEFALL_SPEED)
		item->goalAnimState = LS_FREEFALL;
}

void lara_col_reach(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 11*/
	/*state code: lara_as_reach*/
	if (Lara.ropePtr == -1)
		item->gravityStatus = true;

	Lara.moveAngle = item->pos.yRot;

	coll->Setup.Height = LARA_HEIGHT_STRETCH;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = 0;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = Lara.moveAngle;
	coll->Setup.Radius = coll->Setup.Radius * 1.2f;
	coll->Setup.Mode = COLL_PROBE_MODE::FREE_FORWARD;

	GetCollisionInfo(coll, item);

	if (TestLaraHangJump(item, coll))
		return;

	LaraSlideEdgeJump(item, coll);
	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);
	ShiftItem(item, coll);

	if (item->fallspeed > 0 && coll->Middle.Floor <= 0)
	{
		if (LaraLandedBad(item, coll))
		{
			item->goalAnimState = LS_DEATH;
		}
		else
		{
			item->goalAnimState = LS_STOP;
			item->fallspeed = 0;
			item->gravityStatus = false;
			if (coll->Middle.Floor != NO_HEIGHT)
				item->pos.yPos += coll->Middle.Floor;
		}
	}
}

void lara_as_splat(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.look = false;
}

void lara_col_splat(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot;

	coll->Setup.SlopesAreWalls = true;
	coll->Setup.SlopesArePits = true;

	coll->Setup.BadHeightDown = STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);
	ShiftItem(item, coll);

	if (coll->Middle.Floor >= -256 && coll->Middle.Floor <= 256)
		item->pos.yPos += coll->Middle.Floor;
}

void lara_col_land(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 14*/
	/*state code: lara_void_func*/
	lara_col_stop(item, coll);
}

// State:		LS_JUMP_PREPARE (15)
// Collision:	lara_col_compress()
void lara_as_compress(ITEM_INFO* item, COLL_INFO* coll)
{
	// TODO: dispatch
	/*if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}*/

	if (Lara.waterStatus == LW_WADE)
	{
		item->goalAnimState = LS_JUMP_UP;

		return;
	}

	if (TrInput & IN_LEFT &&
		TrInput & (IN_FORWARD | IN_BACK))
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_SLOW_TURN)
			Lara.turnRate = -LARA_SLOW_TURN;
	}
	else if (TrInput & IN_RIGHT &&
		TrInput & (IN_FORWARD | IN_BACK))
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_SLOW_TURN)
			Lara.turnRate = LARA_SLOW_TURN;
	}

	if (TrInput & IN_FORWARD &&
		TestLaraStandingJump(item, coll, item->pos.yRot))
	{
		Lara.moveAngle = coll->Setup.ForwardAngle;
		item->goalAnimState = LS_JUMP_FORWARD;

		return;
	}
	else if (TrInput & IN_BACK &&
		TestLaraStandingJump(item, coll, item->pos.yRot + ANGLE(180.0f)))
	{
		Lara.moveAngle = coll->Setup.ForwardAngle + ANGLE(180.0f);
		item->goalAnimState = LS_JUMP_BACK;

		return;
	}

	if (TrInput & IN_LEFT &&
		TestLaraStandingJump(item, coll, item->pos.yRot - ANGLE(90.0f)))
	{
		Lara.moveAngle = coll->Setup.ForwardAngle - ANGLE(90.0f);
		item->goalAnimState = LS_JUMP_LEFT;

		return;
	}
	else if (TrInput & IN_RIGHT &&
		TestLaraStandingJump(item, coll, item->pos.yRot + ANGLE(90.0f)))
	{
		Lara.moveAngle = coll->Setup.ForwardAngle + ANGLE(90.0f);
		item->goalAnimState = LS_JUMP_RIGHT;

		return;
	}

	item->goalAnimState = LS_JUMP_UP;
}

// LEGACY
void old_lara_as_compress(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 15*/
	/*collision: lara_col_compress*/
	if (Lara.waterStatus != LW_WADE)
	{
		if (TrInput & IN_FORWARD && !TestLaraFacingCorner(item, item->pos.yRot, 256) && LaraFloorFront(item, item->pos.yRot, 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_FORWARD;
			Lara.moveAngle = item->pos.yRot;
		}
		else if (TrInput & IN_LEFT && !TestLaraFacingCorner(item, item->pos.yRot - ANGLE(90.0f), 256) && LaraFloorFront(item, item->pos.yRot - ANGLE(90.0f), 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_LEFT;
			Lara.moveAngle = item->pos.yRot - ANGLE(90);
		}
		else if (TrInput & IN_RIGHT && !TestLaraFacingCorner(item, item->pos.yRot + ANGLE(90.0f), 256) && LaraFloorFront(item, item->pos.yRot + ANGLE(90.0f), 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_RIGHT;
			Lara.moveAngle = item->pos.yRot + ANGLE(90);
		}
		else if (TrInput & IN_BACK && !TestLaraFacingCorner(item, item->pos.yRot - ANGLE(180.0f), 256) && LaraFloorFront(item, item->pos.yRot - ANGLE(180.0f), 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_BACK;
			Lara.moveAngle = item->pos.yRot + ANGLE(180);
		}
	}

	if (item->fallspeed > LARA_FREEFALL_SPEED)
		item->goalAnimState = LS_FREEFALL;
}

// State:		LS_JUMP_PREPARE (15)
// Collision:	lara_as_compress()
void lara_col_compress(ITEM_INFO* item, COLL_INFO* coll)
{
	item->fallspeed = 0;
	item->gravityStatus = false;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = NO_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraFall(coll))
	{
		SetLaraFallState(item);

		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	// TODO: Better handling.
	if (coll->Middle.Ceiling > -100)
	{
		item->animNumber = LA_STAND_SOLID;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = LS_STOP;
		item->currentAnimState = LS_STOP;

		item->speed = 0;
		item->fallspeed = 0;
		item->gravityStatus = false;

		item->pos.xPos = coll->Setup.OldPosition.x;
		item->pos.yPos = coll->Setup.OldPosition.y;
		item->pos.zPos = coll->Setup.OldPosition.z;
	}

	if (coll->Middle.Floor > -STEP_SIZE && coll->Middle.Floor < STEP_SIZE)
		item->pos.yPos += coll->Middle.Floor;
}

// LEGACY
void old_lara_col_compress(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 15*/
	/*state code: lara_as_compress*/
	item->fallspeed = 0;
	item->gravityStatus = false;

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = NO_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (!LaraFallen(item, coll))
	{
		if (coll->Middle.Ceiling > -100)
		{
			item->animNumber = LA_STAND_SOLID;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_STOP;
			item->currentAnimState = LS_STOP;

			item->speed = 0;
			item->fallspeed = 0;
			item->gravityStatus = false;

			item->pos.xPos = coll->Setup.OldPosition.x;
			item->pos.yPos = coll->Setup.OldPosition.y;
			item->pos.zPos = coll->Setup.OldPosition.z;
		}

		if (coll->Middle.Floor > -256 && coll->Middle.Floor < 256)
			item->pos.yPos += coll->Middle.Floor;
	}
}

// State:		LS_WALK_BACK (16)
// Collision:	lara_col_back()
void lara_as_back(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP; // TODO dispatch

		return;
	}

	if (Lara.isMoving)
		return;

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_SLOW_TURN)
			Lara.turnRate = -LARA_SLOW_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, 10);

	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_SLOW_TURN)
			Lara.turnRate = LARA_SLOW_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, 10);
	}

	if (TrInput & IN_BACK &&
		(TrInput & IN_WALK || Lara.waterStatus == LW_WADE))
	{
		item->goalAnimState = LS_WALK_BACK;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// LEGACY
void old_lara_as_back(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 16*/
	/*collision: lara_col_back*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (!Lara.isMoving)
	{
		if ((TrInput & IN_BACK) && ((TrInput & IN_WALK) || Lara.waterStatus == LW_WADE))
			item->goalAnimState = LS_WALK_BACK;
		else
			item->goalAnimState = LS_STOP;

		if (TrInput & IN_LEFT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -LARA_SLOW_TURN)
				Lara.turnRate = -LARA_SLOW_TURN;
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > LARA_SLOW_TURN)
				Lara.turnRate = LARA_SLOW_TURN;
		}
	}
}

// State:		LS_WALK_BACK (16)
// Control:		lara_as_back()
void lara_col_back(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot + ANGLE(180);
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.BadHeightDown = Lara.waterStatus == LW_WADE ? NO_BAD_POS : STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesArePits = true;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);

		return;
	}

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (TestLaraFall(coll))
	{
		SetLaraFallState(item);

		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	if (TestLaraStep(coll))
	{
		DoLaraStep(item, coll);

		return;
	}

	// LEGACY step
	/*if (coll->Middle.Floor > STEP_SIZE / 2 && coll->Middle.Floor < STEPUP_HEIGHT)
	{
		item->goalAnimState = LS_STEP_BACK_DOWN;
		GetChange(item, &g_Level.Anims[item->animNumber]);
	}*/
}

// LEGACY
void old_lara_col_back(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 16*/
	/*state code: lara_as_back*/
	item->gravityStatus = false;
	item->fallspeed = 0;
	Lara.moveAngle = item->pos.yRot + ANGLE(180);

	if (Lara.waterStatus == LW_WADE)
		coll->Setup.BadHeightDown = NO_BAD_POS;
	else
		coll->Setup.BadHeightDown = STEPUP_HEIGHT;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesArePits = true;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (LaraHitCeiling(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (LaraFallen(item, coll))
		return;

	if (coll->Middle.Floor > STEP_SIZE / 2 && coll->Middle.Floor < STEPUP_HEIGHT)
	{
		item->goalAnimState = LS_STEP_BACK_DOWN;
		GetChange(item, &g_Level.Anims[item->animNumber]);
	}

	if (TestLaraSlide(item, coll))
		return;

	LaraSnapToHeight(item, coll);
}

// State:		LS_TURN_RIGHT_FAST (20)
// Collision:	lara_col_turn_right_fast()
void lara_as_turn_right_fast(ITEM_INFO* item, COLL_INFO* coll)
{
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
		Lara.waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_ROLL_FORWARD;

		return;
	}

	if ((TrInput & IN_DUCK/* || TestLaraKeepDucked(coll)*/) &&
		(Lara.gunStatus == LG_NO_ARMS || !IsStandingWeapon(Lara.gunType)) &&
		Lara.waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_CROUCH_IDLE;

		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (Lara.waterStatus == LW_WADE)		// Should not be possible, but here for security.
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
		if (TrInput & IN_WALK &&
			TestLaraWalkBack(item, coll))
		{
			item->goalAnimState = LS_WALK_BACK;

			return;
		}
		else if (!(TrInput & IN_WALK) &&
			TestLaraHopBack(item, coll)) [[likely]]
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
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate < LARA_MED_TURN)
			Lara.turnRate = LARA_MED_TURN;
		else if (Lara.turnRate > LARA_FAST_TURN)
			Lara.turnRate = LARA_FAST_TURN;

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

// LEGACY
void lara_as_fastturn(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 20*/
	/*collision: lara_col_fastturn*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (Lara.turnRate < 0)
	{
		Lara.turnRate = -LARA_FAST_TURN;

		if (!(TrInput & IN_LEFT))
			item->goalAnimState = LS_STOP;
	}
	else
	{
		Lara.turnRate = LARA_FAST_TURN;

		if (!(TrInput & IN_RIGHT))
			item->goalAnimState = LS_STOP;
	}
}

// LEGACY
void lara_col_fastturn(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 20*/
	/*state code: lara_as_fastturn*/
	lara_col_stop(item, coll);
}

// State:		LS_TURN_LEFT_FAST (152)
// Collision:	lara_col_turn_left_fast()
void lara_as_turn_left_fast(ITEM_INFO* item, COLL_INFO* coll)
{
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
		Lara.waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_ROLL_FORWARD;

		return;
	}

	if ((TrInput & IN_DUCK/* || TestLaraKeepDucked(coll)*/) &&
		(Lara.gunStatus == LG_NO_ARMS || !IsStandingWeapon(Lara.gunType)) &&
		Lara.waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_CROUCH_IDLE;

		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (Lara.waterStatus == LW_WADE)		// Should not be possible, but here for security.
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
		if (Lara.waterStatus == LW_WADE)
		{
			item->goalAnimState = LS_WADE_FORWARD;

			return;
		}
		else if (TrInput & IN_WALK &&
			TestLaraWalkBack(item, coll))
		{
			item->goalAnimState = LS_WALK_BACK;

			return;
		}
		else if (!(TrInput & IN_WALK) &&
			TestLaraHopBack(item, coll)) [[likely]]
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
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_FAST_TURN)
			Lara.turnRate = -LARA_FAST_TURN;
		else if (Lara.turnRate > -LARA_MED_TURN)
			Lara.turnRate = -LARA_MED_TURN;

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
	Lara.look = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;

		return;
	}

	if (Lara.isMoving)
		return;

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_SLOW_TURN)
			Lara.turnRate = -LARA_SLOW_TURN;
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_SLOW_TURN)
			Lara.turnRate = LARA_SLOW_TURN;
	}

	if (TrInput & IN_RSTEP)
	{
		item->goalAnimState = LS_STEP_RIGHT;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// LEGACY
void old_lara_as_stepright(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 21*/
	/*collision: lara_col_stepright*/
	Lara.look = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (!Lara.isMoving)
	{
		if (!(TrInput & IN_RSTEP))
		{
			item->goalAnimState = LS_STOP;
		}

		if (TrInput & IN_LEFT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -LARA_SLOW_TURN)
				Lara.turnRate = -LARA_SLOW_TURN;
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > LARA_SLOW_TURN)
				Lara.turnRate = LARA_SLOW_TURN;
		}
	}
}

// State:		LS_STEP_RIGHT (21)
// Control:		lara_as_stepright()
void lara_col_stepright(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot + ANGLE(90);
	coll->Setup.BadHeightDown = (Lara.waterStatus == LW_WADE) ? NO_BAD_POS : STEP_SIZE / 2;
	coll->Setup.BadHeightUp = -STEP_SIZE / 2;
	coll->Setup.BadCeilingHeight = 0;
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.SlopesArePits = true;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);

		return;
	}

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (TestLaraFall(coll))
	{
		SetLaraFallState(item);

		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	if (TestLaraStep(coll) || TestLaraSwamp(item))
	{
		DoLaraStep(item, coll);

		return;
	}

	// LEGACY step
	/*if (coll->Middle.Floor != NO_HEIGHT)
		item->pos.yPos += coll->Middle.Floor;*/
}

// LEGACY
void old_lara_col_stepright(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 21*/
	/*state code: lara_as_stepright*/
	if (item->currentAnimState == LS_STEP_RIGHT)
		Lara.moveAngle = item->pos.yRot + ANGLE(90);
	else
		Lara.moveAngle = item->pos.yRot - ANGLE(90);

	item->gravityStatus = false;
	item->fallspeed = 0;

	if (Lara.waterStatus == LW_WADE)
		coll->Setup.BadHeightDown = NO_BAD_POS;
	else
		coll->Setup.BadHeightDown = 128;

	coll->Setup.SlopesAreWalls = true;
	coll->Setup.SlopesArePits = true;

	coll->Setup.BadHeightUp = -128;
	coll->Setup.BadCeilingHeight = 0;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (!LaraHitCeiling(item, coll))
	{
		if (LaraDeflectEdge(item, coll))
			LaraCollideStop(item, coll);

		if (!LaraFallen(item, coll) && !TestLaraSlide(item, coll) && coll->Middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->Middle.Floor;
	}
}

// State:		LS_SIDESTEP_LEFT (22)
// Collision:	lara_col_stepleft()
void lara_as_stepleft(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.look = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;

		return;
	}

	if (Lara.isMoving)
		return;

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_SLOW_TURN)
			Lara.turnRate = -LARA_SLOW_TURN;
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_SLOW_TURN)
			Lara.turnRate = LARA_SLOW_TURN;
	}

	if (TrInput & IN_LSTEP)
	{
		item->goalAnimState = LS_STEP_LEFT;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// LEGACY
void old_lara_as_stepleft(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 22*/
	/*collision: lara_col_stepleft*/
	Lara.look = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (!Lara.isMoving)
	{
		if (!(TrInput & IN_LSTEP))
		{
			item->goalAnimState = LS_STOP;
		}

		if (TrInput & IN_LEFT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -LARA_SLOW_TURN)
				Lara.turnRate = -LARA_SLOW_TURN;
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > LARA_SLOW_TURN)
				Lara.turnRate = LARA_SLOW_TURN;
		}
	}
}

// State:		LS_STEP_LEFT (22)
// Control:		lara_as_stepleft()
void lara_col_stepleft(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot - ANGLE(90);
	coll->Setup.BadHeightDown = (Lara.waterStatus == LW_WADE) ? NO_BAD_POS : STEP_SIZE / 2;
	coll->Setup.BadHeightUp = -STEP_SIZE / 2;
	coll->Setup.BadCeilingHeight = 0;
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.SlopesArePits = true;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);

		return;
	}

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (TestLaraFall(coll))
	{
		SetLaraFallState(item);

		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	if (TestLaraStep(coll) || TestLaraSwamp(item))
	{
		DoLaraStep(item, coll);

		return;
	}

	// LEGACY step
	/*if (coll->Middle.Floor != NO_HEIGHT)
		item->pos.yPos += coll->Middle.Floor;*/
}

// LEGACY
void old_lara_col_stepleft(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 22*/
	/*state code: lara_as_stepleft*/
	lara_col_stepright(item, coll);
}

// State:		LS_ROLL_FORWARD (23)
// Collision:	lara_col_roll2()
void lara_as_roll2(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_LEFT &&
		!(TrInput & IN_JUMP))		// JUMP locks y rotation.
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_MED_TURN)
			Lara.turnRate = -LARA_MED_TURN;
	}
	else if (TrInput & IN_RIGHT &&
		!(TrInput & IN_JUMP))
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_MED_TURN)
			Lara.turnRate = LARA_MED_TURN;
	}

	item->goalAnimState = LS_ROLL_BACK;
}

// State:		LS_ROLL_FORWARD (23)
// Control:		lara_as_roll2()
void lara_col_roll2(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.laraNode = 0;
	Lara.moveAngle = item->pos.yRot + ANGLE(180);
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = Lara.moveAngle;
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

	// LEGACY step
	/*if (coll->Middle.Floor != NO_HEIGHT)
		item->pos.yPos += coll->Middle.Floor;*/
}

// LEGACY
void old_lara_col_roll2(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 23*/
	/*state code: lara_void_func*/
	Camera.laraNode = 0;
	Lara.moveAngle = item->pos.yRot + ANGLE(180);

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;

	item->gravityStatus = false;
	item->fallspeed = 0;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (LaraHitCeiling(item, coll))
		return;
	if (TestLaraSlide(item, coll))
		return;

	if (coll->Middle.Floor > 200)
	{
		item->animNumber = LA_FALL_BACK;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->currentAnimState = LS_FALL_BACK;
		item->goalAnimState = LS_FALL_BACK;
		item->fallspeed = 0;
		item->gravityStatus = true;
		return;
	}

	ShiftItem(item, coll);

	if (coll->Middle.Floor != NO_HEIGHT)
		item->pos.yPos += coll->Middle.Floor;
}

void lara_as_backjump(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 25*/
	/*collision: lara_col_backjump*/
	Camera.targetAngle = ANGLE(135.0f);
	if (item->fallspeed <= LARA_FREEFALL_SPEED)
	{
		if (item->goalAnimState == LS_RUN_FORWARD)
		{
			item->goalAnimState = LS_STOP;
		}
		else if ((TrInput & IN_FORWARD || TrInput & IN_ROLL) && item->goalAnimState != LS_STOP)
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
	/*state 25*/
	/*state code: lara_as_backjump*/
	Lara.moveAngle = item->pos.yRot + ANGLE(180);
	lara_col_jumper(item, coll);
}

void lara_as_rightjump(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 26*/
	/*collision: lara_col_rightjump*/
	Lara.look = false;
	if (item->fallspeed > LARA_FREEFALL_SPEED)
		item->goalAnimState = LS_FREEFALL;
	else if (TrInput & IN_LEFT && item->goalAnimState != LS_STOP)
		item->goalAnimState = LS_JUMP_ROLL_180;
}

void lara_col_rightjump(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 26*/
	/*state code: lara_as_rightjump*/
	Lara.moveAngle = item->pos.yRot + ANGLE(90);
	lara_col_jumper(item, coll);
}

void lara_as_leftjump(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 27*/
	/*collision: lara_col_leftjump*/
	Lara.look = false;
	if (item->fallspeed > LARA_FREEFALL_SPEED)
		item->goalAnimState = LS_FREEFALL;
	else if (TrInput & IN_RIGHT && item->goalAnimState != LS_STOP)
		item->goalAnimState = LS_JUMP_ROLL_180;
}

void lara_col_leftjump(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 27*/
	/*state code: lara_as_leftjump*/
	Lara.moveAngle = item->pos.yRot - ANGLE(90);
	lara_col_jumper(item, coll);
}

void lara_col_jumper(ITEM_INFO* item, COLL_INFO* coll)
{
	/*states 25, 26, 27*/
	/*state code: none, but is called in lara_col_backjump, lara_col_rightjump and lara_col_leftjump*/
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = Lara.moveAngle;

	GetCollisionInfo(coll, item);
	LaraDeflectEdgeJump(item, coll);

	if (item->fallspeed > 0 && coll->Middle.Floor <= 0)
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			item->goalAnimState = LS_STOP;

		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->Middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->Middle.Floor;
	}
}

void lara_as_upjump(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 28*/
	/*collision: lara_col_upjump*/
	if (item->fallspeed > LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
	}
}

void lara_col_upjump(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 28*/
	/*state code: lara_as_upjump*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	Lara.moveAngle = item->pos.yRot;

	coll->Setup.Height = LARA_HEIGHT_STRETCH;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = item->speed < 0 ? Lara.moveAngle + ANGLE(180.0f) : Lara.moveAngle;
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
		item->goalAnimState = LaraLandedBad(item, coll) ? LS_DEATH : LS_STOP;

		item->gravityStatus = false;
		item->fallspeed = 0;

		if (coll->Middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->Middle.Floor;
	}
}

void lara_as_fallback(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 29*/
	/*collision: lara_col_fallback*/
	if (item->fallspeed > LARA_FREEFALL_SPEED)
		item->goalAnimState = LS_FREEFALL;

	if (TrInput & IN_ACTION)
		if (Lara.gunStatus == LG_NO_ARMS)
			item->goalAnimState = LS_REACH;
}

void lara_col_fallback(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 29*/
	/*state code: lara_as_fallback*/
	Lara.moveAngle = item->pos.yRot + ANGLE(180);

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);
	LaraDeflectEdgeJump(item, coll);

	if (item->fallspeed > 0 &&  (coll->Middle.Floor <= 0 || TestLaraSwamp(item)))
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			item->goalAnimState = LS_STOP;

		LaraResetGravityStatus(item, coll);
		LaraSnapToHeight(item, coll);
	}
}

// State:		LS_ROLL_FORWARD (45)
// Collision:	lara_col_roll()
void lara_as_roll(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_LEFT &&
		!(TrInput & IN_JUMP))		// JUMP locks y rotation.
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_MED_TURN)
			Lara.turnRate = -LARA_MED_TURN;
	}
	else if (TrInput & IN_RIGHT &&
		!(TrInput & IN_JUMP))
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_MED_TURN)
			Lara.turnRate = LARA_MED_TURN;
	}

	// TODO: Interpolation instead.
	Lara.NewAnims.SwandiveRollRun = true;

	if (TrInput & IN_FORWARD &&
		item->animNumber == LA_SWANDIVE_ROLL &&
		Lara.NewAnims.SwandiveRollRun)
	{
		item->goalAnimState = LS_RUN_FORWARD;

		return;
	}

	item->goalAnimState = LS_ROLL_FORWARD; //?
}

// State:		LS_ROLL_FORWARD (45)
// Control:		lara_as_roll()
void lara_col_roll(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot;
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesArePits = false;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);

		return;
	}

	if (TestLaraFall(coll))
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

	// LEGACY step
	/*if (coll->Middle.Floor != NO_HEIGHT)
		item->pos.yPos += coll->Middle.Floor;*/
}

// LEGACY
void old_lara_col_roll(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 45*/
	/*state code: lara_void_func*/
	Lara.moveAngle = item->pos.yRot;

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesArePits = false;
	coll->Setup.SlopesAreWalls = true;

	item->gravityStatus = false;
	item->fallspeed = 0;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (LaraHitCeiling(item, coll))
		return;
	if (TestLaraSlide(item, coll))
		return;
	if (LaraFallen(item, coll))
		return;

	//##LUA debug etc.
	Lara.NewAnims.SwandiveRollRun = 1;

	if (TrInput & IN_FORWARD && item->animNumber == LA_SWANDIVE_ROLL && Lara.NewAnims.SwandiveRollRun)
	{
		item->goalAnimState = LS_RUN_FORWARD;
	}

	ShiftItem(item, coll);

	if (coll->Middle.Floor != NO_HEIGHT)
		item->pos.yPos += coll->Middle.Floor;
}

void lara_as_swandive(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 52*/
	/*collision: lara_col_swandive*/
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpaz = false;

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_JUMP_TURN)
			Lara.turnRate = -LARA_JUMP_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, 16);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_JUMP_TURN)
			Lara.turnRate = LARA_JUMP_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX, 16);
	}

	if (item->fallspeed > LARA_FREEFALL_SPEED && item->goalAnimState != LS_DIVE)
		item->goalAnimState = LS_SWANDIVE_END;
}

void lara_col_swandive(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 52*/
	/*state code: lara_as_swandive*/
	Lara.moveAngle = item->pos.yRot;

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);
	LaraDeflectEdgeJump(item, coll);

	if (coll->Middle.Floor <= 0 && item->fallspeed > 0)
	{
		item->goalAnimState = LS_STOP;
		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->Middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->Middle.Floor;
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
	/*state 53*/
	/*state code: lara_as_fastdive*/
	Lara.moveAngle = item->pos.yRot;

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);
	LaraDeflectEdgeJump(item, coll);

	if (coll->Middle.Floor <= 0 && item->fallspeed > 0)
	{
		if (item->fallspeed <= 133)
			item->goalAnimState = LS_STOP;
		else
			item->goalAnimState = LS_DEATH;

		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->Middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->Middle.Floor;
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
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_FAST_TURN)
			Lara.turnRate = -LARA_FAST_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, 10);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_FAST_TURN)
			Lara.turnRate = LARA_FAST_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX, 10);
	}

	if (TrInput & IN_FORWARD)
	{
		if (Lara.waterStatus == LW_ABOVE_WATER)
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
	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_SLOW_TURN)
			Lara.turnRate = -LARA_SLOW_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 5 * 3, 12);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_SLOW_TURN)
			Lara.turnRate = LARA_SLOW_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 5 * 3, 12);
	}

	if (TrInput & IN_FORWARD)
	{
		item->goalAnimState = LS_WADE_FORWARD;
		
		return;
	}

	item->goalAnimState = LS_STOP;
}

// LEGACY
void old_lara_as_wade(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 65*/
	/*collision: lara_col_wade*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	Camera.targetElevation = -ANGLE(22.0f);

	if (TestLaraSwamp(item))
	{
		if (TrInput & IN_LEFT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -LARA_SLOW_TURN)
				Lara.turnRate = -LARA_SLOW_TURN;

			DoLaraLean(item, coll, -LARA_LEAN_MAX / 2, 16);
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > LARA_SLOW_TURN)
				Lara.turnRate = LARA_SLOW_TURN;

			DoLaraLean(item, coll, LARA_LEAN_MAX / 2, 16);
		}

		if (TrInput & IN_FORWARD)
			item->goalAnimState = LS_WADE_FORWARD;
		else
			item->goalAnimState = LS_STOP;
	}
	else
	{
		if (TrInput & IN_LEFT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -LARA_FAST_TURN)
				Lara.turnRate = -LARA_FAST_TURN;

			DoLaraLean(item, coll, -LARA_LEAN_MAX, 7);
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > LARA_FAST_TURN)
				Lara.turnRate = LARA_FAST_TURN;

			DoLaraLean(item, coll, LARA_LEAN_MAX, 7);
		}

		if (TrInput & IN_FORWARD)
		{
			if (Lara.waterStatus == LW_ABOVE_WATER)
				item->goalAnimState = LS_RUN_FORWARD;
			else
				item->goalAnimState = LS_WADE_FORWARD;
		}
		else
		{
			item->goalAnimState = LS_STOP;
		}
	}
}

// State:		LS_WADE_FORWARD (65)
// Control:		lara_as_wade()
void lara_col_wade(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);

		return;
	}

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

	if (TestLaraVault(item, coll))
		return;

	// TODO: This check can be simpler. @Sezz 2021.11.08
	if (TestLaraStep(coll) || TestLaraSwamp(item))
	{
		DoLaraStep(item, coll);

		return;
	}
}

// LEGACY
void old_lara_col_wade(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 65*/
	/*state code: lara_as_wade*/
	Lara.moveAngle = item->pos.yRot;

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;

	coll->Setup.SlopesAreWalls = true;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (LaraHitCeiling(item, coll))
		return;

	if (TestLaraVault(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		item->pos.zRot = 0;


		if (!coll->Front.Slope && coll->Front.Floor < -((STEP_SIZE * 5) / 2) && !TestLaraSwamp(item))
		{
			item->goalAnimState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->animNumber]))
				return;
		}

		LaraCollideStop(item, coll);
	}


	if (coll->Middle.Floor >= -STEPUP_HEIGHT && coll->Middle.Floor < -STEP_SIZE / 2 && !TestLaraSwamp(item))
	{
		item->goalAnimState = LS_STEP_UP;
		GetChange(item, &g_Level.Anims[item->animNumber]);
	}

	if (coll->Middle.Floor >= 50 && !TestLaraSwamp(item))
	{
		item->pos.yPos += 50;
		return;
	}

	LaraSnapToHeight(item, coll);
}

// State:		LS_SPRINT (73)
// Collision:	lara_col_dash()
void lara_as_dash(ITEM_INFO* item, COLL_INFO* coll)
{
	DashTimer--; // TODO: Move global to LaraItem? health.cpp needs deglobalisation in general. @Sezz 2021.09.29

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_SLOW_TURN)
			Lara.turnRate = -LARA_SLOW_TURN;

		DoLaraLean(item, coll, -(LARA_LEAN_MAX * 3) / 5, 10);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_SLOW_TURN)
			Lara.turnRate = LARA_SLOW_TURN;

		DoLaraLean(item, coll, (LARA_LEAN_MAX * 3) / 5, 10);
	}

	if (TrInput & IN_JUMP)
	{
		item->goalAnimState = LS_SPRINT_ROLL;

		return;
	}

	if ((TrInput & IN_DUCK/* || TestLaraKeepDucked(coll)*/) &&
		(Lara.gunStatus == LG_NO_ARMS || !IsStandingWeapon(Lara.gunType)))
	{
		item->goalAnimState = LS_CROUCH_IDLE;

		return;
	}

	// TODO: Supposedly there is a bug wherein sprinting into the boundary between shallow and deep water
	// under some condition allows Lara to run around in the water room. Investigate. @Sezz 2021.09.29
	if (TrInput & IN_FORWARD)
	{
		if (Lara.waterStatus == LW_WADE)
			item->goalAnimState = LS_RUN_FORWARD;	// TODO: Dispatch to wade forward state. @Sezz 2021.09.29
		else if (TrInput & IN_WALK)
			item->goalAnimState = LS_WALK_FORWARD;
		else if (TrInput & IN_SPRINT && DashTimer > 0) [[likely]]
			item->goalAnimState = LS_SPRINT;
		else
			item->goalAnimState = LS_RUN_FORWARD;

		return;
	}

	item->goalAnimState = LS_STOP;
}

// LEGACY
void old_lara_as_dash(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 73*/
	/*collision: lara_col_dash*/
	if (item->hitPoints <= 0 || !DashTimer || !(TrInput & IN_SPRINT) || Lara.waterStatus == LW_WADE)
	{
		item->goalAnimState = LS_RUN_FORWARD;
		return;
	}

	DashTimer--;

	if (TrInput & IN_DUCK
		&& (Lara.gunStatus == LG_NO_ARMS
			|| Lara.gunType == WEAPON_NONE
			|| Lara.gunType == WEAPON_PISTOLS
			|| Lara.gunType == WEAPON_REVOLVER
			|| Lara.gunType == WEAPON_UZI
			|| Lara.gunType == WEAPON_FLARE))
	{
		item->goalAnimState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_SLOW_TURN)
			Lara.turnRate = -LARA_SLOW_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, 7);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_SLOW_TURN)
			Lara.turnRate = LARA_SLOW_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX, 7);
	}

	if (!(TrInput & IN_JUMP) || item->gravityStatus)
	{
		if (TrInput & IN_FORWARD)
		{
			if (TrInput & IN_WALK)
				item->goalAnimState = LS_WALK_FORWARD;
			else
				item->goalAnimState = LS_SPRINT;
		}
		else if (!(TrInput & IN_LEFT) && !(TrInput & IN_RIGHT))
		{
			item->goalAnimState = LS_STOP;
		}
	}
	else
	{
		item->goalAnimState = LS_SPRINT_ROLL;
	}
}

// State:		LS_SPRINT (73)
// Control:		lara_as_dash()
void lara_col_dash(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.ForwardAngle = Lara.moveAngle;
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

	if (TestLaraFall(coll))
	{
		SetLaraFallState(item);

		return;
	}

	if (TestLaraSlide(item, coll))
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

// LEGACY
void old_lara_col_dash(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 73*/
	/*state code: lara_as_dash*/
	Lara.moveAngle = item->pos.yRot;

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;

	coll->Setup.SlopesAreWalls = true;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (!LaraHitCeiling(item, coll) && !TestLaraVault(item, coll))
	{
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

		if (!LaraFallen(item, coll))
		{
			if (coll->Middle.Floor >= -STEPUP_HEIGHT && coll->Middle.Floor < -STEP_SIZE / 2)
			{
				item->goalAnimState = LS_STEP_UP;
				GetChange(item, &g_Level.Anims[item->animNumber]);
			}

			if (!TestLaraSlide(item, coll))
			{
				if (coll->Middle.Floor < 50)
				{
					if (coll->Middle.Floor != NO_HEIGHT)
						item->pos.yPos += coll->Middle.Floor;
				}
				else
				{
					item->goalAnimState = LS_STEP_DOWN; // for theoretical sprint stepdown anims, not in default anims
					if (GetChange(item, &g_Level.Anims[item->animNumber]))
						item->pos.yPos += coll->Middle.Floor; // move Lara to middle.Floor
					else
						item->pos.yPos += 50; // do the default aligment
				}
			}
		}
	}
}

// State:		LS_SPRINT_ROLL (74)
// Collision:	lara_col_dashdive()
void lara_as_dashdive(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_SLOW_TURN)
			Lara.turnRate = -LARA_SLOW_TURN;

		DoLaraLean(item, coll, -(LARA_LEAN_MAX * 3) / 5, 10);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_SLOW_TURN)
			Lara.turnRate = LARA_SLOW_TURN;

		DoLaraLean(item, coll, (LARA_LEAN_MAX * 3) / 5, 10);
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

// LEGACY
void old_lara_as_dashdive(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 74*/
	/*collision: lara_col_dashdive*/
	if (item->goalAnimState != LS_DEATH &&
		item->goalAnimState != LS_STOP &&
		item->goalAnimState != LS_RUN_FORWARD &&
		item->fallspeed > LARA_FREEFALL_SPEED)
		item->goalAnimState = LS_FREEFALL;
}

void lara_col_dashdive(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 74*/
	/*state code: lara_as_dashdive*/
	if (item->speed < 0)
		Lara.moveAngle = item->pos.yRot + ANGLE(180);
	else
		Lara.moveAngle = item->pos.yRot;

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;

	coll->Setup.SlopesAreWalls = true;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);
	LaraDeflectEdgeJump(item, coll);

	if (!LaraFallen(item, coll))
	{
		if (item->speed < 0)
			Lara.moveAngle = item->pos.yRot;

		if (coll->Middle.Floor <= 0 && item->fallspeed > 0)
		{
			if (LaraLandedBad(item, coll))
			{
				item->goalAnimState = LS_DEATH;
			}
			else if (Lara.waterStatus == LW_WADE || !(TrInput & IN_FORWARD) || TrInput & IN_WALK)
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
