#include "framework.h"
#include "lara.h"
#include "lara_basic.h"
#include "lara_tests.h"
#include "lara_collide.h"
#include "lara_slide.h"
#include "lara_monkey.h"
#include "input.h"
#include "health.h"
#include "sound.h"
#include "draw.h"
#include "pickup.h"

bool EnableBackKeyTurn = false;

bool EnableJump = false;

// BASIC MOVEMENT

// ------------------------------
// Auxiliary Functions
// ------------------------------

bool TestLaraStepDown(COLL_INFO* coll)
{
	if (coll->midFloor <= STEPUP_HEIGHT &&
		coll->midFloor > STEP_SIZE / 2)
	{
		return true;
	}

	return false;
}

bool TestLaraStepUp(COLL_INFO* coll)
{
	if (coll->frontFloor >= -STEPUP_HEIGHT &&
		coll->frontFloor < -STEP_SIZE / 2)
	{
		return true;
	}

	return false;
}

// TODO: Some states can't use this function yet due to missing step up/down anims.
void DoLaraStep(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TestLaraStepDown(coll))
	{
		item->pos.yPos += coll->midFloor;

		item->goalAnimState = LS_STEP_DOWN;
		GetChange(item, &g_Level.Anims[item->animNumber]);
	}
	else if (TestLaraStepUp(coll))
	{
		item->pos.yPos += coll->midFloor;

		item->goalAnimState = LS_STEP_UP;
		GetChange(item, &g_Level.Anims[item->animNumber]);
	}
	else if (abs(coll->midFloor) >= 50)
	{
		item->pos.yPos += 50 * GetSign(coll->midFloor);
	}
	else
	{
		item->pos.yPos += coll->midFloor;
	}
}

// ------------------------------
// GENERAL
// Control & Collision Functions
// ------------------------------

void lara_void_func(ITEM_INFO* item, COLL_INFO* coll)//19928(<), 19A5C(<) (F)
{
	return;
}

void lara_default_col(ITEM_INFO* item, COLL_INFO* coll)//1C80C(<), 1C940(<) (F)
{
	Lara.moveAngle = 0;

	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesArePits = true;
	coll->slopesAreWalls = true;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
}

void lara_as_special(ITEM_INFO* item, COLL_INFO* coll)//1ADDC(<), 1AF10(<) (F)
{
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(170.0f);
	Camera.targetElevation = ANGLE(-25.0f);
}

void lara_as_null(ITEM_INFO* item, COLL_INFO* coll)//1A5DC(<), 1A710(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
}

void lara_as_controlled(ITEM_INFO* item, COLL_INFO* coll)//1B0FC(<), 1B230(<) (F)
{
	Lara.look = false;

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd - 1)
	{
		Lara.gunStatus = LG_NO_ARMS;

		if (UseForcedFixedCamera)
		{
			UseForcedFixedCamera = 0;
		}
	}
}

void lara_as_controlledl(ITEM_INFO* item, COLL_INFO* coll)//1B180(<), 1B2B4(<) (F)
{
	Lara.look = false;

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
}

// ------------------------------
// BASIC MOVEMENT
// Control & Collision Functions
// ------------------------------

// State:		0
// Collision:	lara_col_walk_forward()
void lara_as_walk_forward(ITEM_INFO* item, COLL_INFO* coll)//191B8(<), 192EC(<) (F)
{
	// TODO: Looking while walking.
	// BUG: When looking while running Lara transitions into a walk, look mode locks.

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (Lara.isMoving)
	{
		return;
	}

	// TODO: Lara refuses to turn when performing walk-to-stand anims.
	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < ANGLE(-4.0f))
		{
			Lara.turnRate = ANGLE(-4.0f);
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > ANGLE(4.0f))
		{
			Lara.turnRate = ANGLE(4.0f);
		}
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

		return;
	}

	item->goalAnimState = LS_STOP;
}

// State:		0
// State code:	lara_as_walk_forward()
void lara_col_walk_forward(ITEM_INFO* item, COLL_INFO* coll)//1B3E8, 1B51C (F)
{
	Lara.moveAngle = 0;

	item->gravityStatus = false;
	item->fallspeed = 0;

	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = true;
	coll->slopesArePits = true;
	coll->lavaIsPit = 1;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (TrInput & IN_LEFT)
	{
		if (TestLaraLean(coll) && item->pos.zRot <= 0)
		{
			item->pos.zRot -= LARA_LEAN_RATE;
			if (item->pos.zRot < -LARA_LEAN_MAX / 2)
			{
				item->pos.zRot = -LARA_LEAN_MAX / 2;
			}
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		if (TestLaraLean(coll) && item->pos.zRot >= 0)
		{
			item->pos.zRot += LARA_LEAN_RATE;
			if (item->pos.zRot > LARA_LEAN_MAX / 2)
			{
				item->pos.zRot = LARA_LEAN_MAX / 2;
			}
		}
	}

	if (TestLaraVault(item, coll))
	{
		return;
	}

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraWallDeflect(coll))
	{
		SetLaraWallDeflect(item, coll);

		item->goalAnimState = LS_SPLAT;
		if (GetChange(item, &g_Level.Anims[item->animNumber]))
		{
			return;
		}

		LaraCollideStop(item, coll);
	}

	if (abs(coll->midFloor) > 0 && abs(coll->midFloor <= STEPUP_HEIGHT))
	{
		DoLaraStep(item, coll);
	}

	if (TestLaraFall(coll))
	{
		item->goalAnimState = LS_FALL;
		item->fallspeed = 0;
		item->gravityStatus = true;

		return;
	}

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}
}

// State:		1
// Collision:	lara_col_run()
void lara_as_run(ITEM_INFO* item, COLL_INFO* coll)//192EC, 19420 (F)
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
		{
			Lara.turnRate = -LARA_FAST_TURN;
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_FAST_TURN)
		{
			Lara.turnRate = LARA_FAST_TURN;
		}
	}

	if (item->animNumber == LA_STAND_TO_RUN)
	{
		EnableJump = false;
	}
	else if (item->animNumber == LA_RUN && item->frameNumber == 4)
	{
		EnableJump = true;
	}
	else
	{
		EnableJump = true;
	}

	if (TrInput & IN_JUMP && EnableJump && !item->gravityStatus)
	{
		item->goalAnimState = LS_JUMP_FORWARD;
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

	if ((TrInput & IN_ROLL ||
		(TrInput & IN_BACK && EnableBackKeyTurn))
		&& !(TrInput & IN_JUMP))	// A slightly hacky fix to prevent unintentional rolling when JUMP and ROLL are pressed and EnableJump isn't true yet.
	{
		item->goalAnimState = LS_ROLL_FORWARD;
		return;
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

		return;
	}

	item->goalAnimState = LS_STOP;
}

// State:		1
// State code:	lara_col_run()
void lara_col_run(ITEM_INFO* item, COLL_INFO* coll)//1B64C, 1B780 (F)
{
	Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = true;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (TrInput & IN_LEFT)
	{
		if (TestLaraLean(coll))
		{
			item->pos.zRot -= LARA_LEAN_RATE;
			if (item->pos.zRot < -LARA_LEAN_MAX)
			{
				item->pos.zRot = -LARA_LEAN_MAX;
			}
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		if (TestLaraLean(coll))
		{
			item->pos.zRot += LARA_LEAN_RATE;
			if (item->pos.zRot > LARA_LEAN_MAX)
			{
				item->pos.zRot = LARA_LEAN_MAX;
			}
		}
	}

	if (TestLaraVault(item, coll))
	{
		return;
	}

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (LaraDeflectEdge(item, coll))
	{
		item->pos.zRot = 0;

		if (TestWall(item, 256, 0, -640))
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

	if (abs(coll->midFloor) > 0 && abs(coll->midFloor <= STEPUP_HEIGHT))
	{
		//DoLaraStep(item, coll);	// Uncomment this line and remove everything below when a step down anim is created.

		/*if (TestLaraStepDown(coll))
		{
			item->pos.yPos += coll->midFloor;

			item->goalAnimState = LS_STEP_DOWN;
			GetChange(item, &g_Level.Anims[item->animNumber]);
		}
		else */if (TestLaraStepUp(coll))
		{
			item->pos.yPos += coll->midFloor;

			item->goalAnimState = LS_STEP_UP;
			GetChange(item, &g_Level.Anims[item->animNumber]);
		}
		else if (abs(coll->midFloor) >= 50)
		{
			item->pos.yPos += 50 * GetSign(coll->midFloor);
		}
		else
		{
			item->pos.yPos += coll->midFloor;
		}
	}

	if (TestLaraFall(coll))
	{
		//if (TrInput & IN_ACTION &&
		//	EnableSafetyDrop &&
		//	coll->midFloor >= 1024 &&
		//	Lara.gunStatus == LG_NO_ARMS)  // TODO: Also test for floor behind and static below.
		//{
		//	item->goalAnimState = LS_SAFE_DROP;
		//}
		//else
		//{
		item->goalAnimState = LS_FALL;
		item->fallspeed = 0;
		item->gravityStatus = true;
		//}

		return;
	}

	// LEGACY step code. Has NO_HEIGHT checks unaccounted for above; not sure what they do exactly. -Sezz
	//if (coll->midFloor >= -STEPUP_HEIGHT && coll->midFloor < -STEP_SIZE / 2)
	//{
	//	if (coll->frontFloor == NO_HEIGHT || coll->frontFloor < -STEPUP_HEIGHT || coll->frontFloor >= -STEP_SIZE / 2)
	//	{
	//		coll->midFloor = 0;
	//	}
	//	else
	//	{
	//		item->goalAnimState = LS_STEP_UP;
	//		GetChange(item, &g_Level.Anims[item->animNumber]);
	//	}
	//}
	//else
	//{
	//	if (coll->midFloor < 50)
	//	{
	//		if (coll->midFloor != NO_HEIGHT)
	//		{
	//			item->pos.yPos += coll->midFloor / 2;
	//		}
	//		else
	//		{
	//			item->goalAnimState = LS_STEP_DOWN; // for theoretical running stepdown anims, not in default anims

	//			if (GetChange(item, &g_Level.Anims[item->animNumber]))
	//			{
	//				item->pos.yPos += coll->midFloor; // move Lara to midFloor
	//			}
	//			else
	//			{
	//				item->pos.yPos += 50; // do the default aligment
	//			}
	//		}
	//	}
	//}

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}
}

// State:		2
// Collision:	lara_col_stop()
void lara_as_stop(ITEM_INFO* item, COLL_INFO* coll)
{
	int fHeight = NO_HEIGHT;
	int rHeight = NO_HEIGHT;

	if (item->animNumber != LA_SPRINT_TO_STAND_RIGHT && item->animNumber != LA_SPRINT_TO_STAND_LEFT)
	{
		StopSoundEffect(SFX_LARA_SLIPPING);
	}

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;
		return;
	}

	// Handles waterskin and clockwork beetle.
	if (UseSpecialItem(item))
	{
		return;
	}

	// TODO: This prevents locks and looping actions, but those should be caught by each state.
	item->goalAnimState = LS_STOP;

	if (TrInput & IN_LOOK)
	{
		LookUpDown();
	}

	if (TrInput & IN_FORWARD)
	{
		fHeight = LaraFloorFront(item, item->pos.yRot, LARA_RAD + 4);
	}
	else if (TrInput & IN_BACK)
	{
		rHeight = LaraFloorFront(item, item->pos.yRot - ANGLE(180.0f), LARA_RAD + 4); // TR3: item->pos.yRot + ANGLE(180) ?
	}

	int height, ceiling;

	if (TrInput & IN_LEFT)
	{
		item->goalAnimState = LS_TURN_LEFT_SLOW;
	}
	else if (TrInput & IN_RIGHT)
	{
		item->goalAnimState = LS_TURN_RIGHT_SLOW;
	}
	else if (TrInput & IN_LSTEP)
	{
		height = LaraFloorFront(item, item->pos.yRot - ANGLE(90.0f), LARA_RAD + 48);
		ceiling = LaraCeilingFront(item, item->pos.yRot - ANGLE(90.0f), LARA_RAD + 48, LARA_HITE);

		if ((height < 128 && height > -128) && HeightType != BIG_SLOPE && ceiling <= 0)
		{
			item->goalAnimState = LS_STEP_LEFT;
		}
	}
	else if (TrInput & IN_RSTEP)
	{
		height = LaraFloorFront(item, item->pos.yRot + ANGLE(90.0f), LARA_RAD + 48);
		ceiling = LaraCeilingFront(item, item->pos.yRot + ANGLE(90.0f), LARA_RAD + 48, LARA_HITE);

		if ((height < 128 && height > -128) && HeightType != BIG_SLOPE && ceiling <= 0)
		{
			item->goalAnimState = LS_STEP_RIGHT;
		}
	}

	if (Lara.waterStatus == LW_WADE)
	{
		if (TrInput & IN_JUMP && !(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP))
		{
			item->goalAnimState = LS_JUMP_PREPARE;
		}

		if (TrInput & IN_FORWARD)
		{
			if ((g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP &&
				fHeight > -(STEPUP_HEIGHT - 1)) ||
				(fHeight < (STEPUP_HEIGHT - 1) &&
				fHeight > -(STEPUP_HEIGHT - 1)))
			{
				item->goalAnimState = LS_WADE_FORWARD;
				return;
			}

			Lara.moveAngle = 0;

			coll->badPos = NO_BAD_POS;
			coll->badNeg = -STEPUP_HEIGHT;
			coll->badCeiling = 0;
			coll->slopesAreWalls = true;
			coll->radius = LARA_RAD + 2;
			coll->facing = Lara.moveAngle;

			GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

			if (TestLaraVault(item, coll))
			{
				return;
			}

			coll->radius = LARA_RAD;

			return;
		}

		if (TrInput & IN_BACK)
		{
			if ((rHeight < (STEPUP_HEIGHT - 1)) && (rHeight > -(STEPUP_HEIGHT - 1)))
			{
				item->goalAnimState = LS_WALK_BACK;
			}

			return;
		}

		// TODO: Add failsafe which automatically crouches Lara if she unexpectedly embeds into a crawlspace.
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

		if (TrInput & IN_ROLL && Lara.waterStatus != LW_WADE)
		{
			item->goalAnimState = LS_ROLL_FORWARD;
			return;
		}
	}
	else [[likely]]
	{
		if (TrInput & IN_JUMP)
		{
			item->goalAnimState = LS_JUMP_PREPARE;
			return;
		}

		if (TrInput & IN_FORWARD)
		{
			int height = LaraFloorFront(item, item->pos.yRot, LARA_RAD + 4);
			int ceiling = LaraCeilingFront(item, item->pos.yRot, LARA_RAD + 4, LARA_HITE);

			if ((HeightType == BIG_SLOPE || HeightType == DIAGONAL) && (height < 0 || ceiling > 0))
			{
				item->goalAnimState = LS_STOP;
				return;
			}

			/*if (TrInput & IN_ACTION &&
				EnableSafetyDrop &&
				LaraFloorFront(item, -item->pos.yRot, 100) <= 1024)
			{
				item->goalAnimState = LS_SAFE_DROP;
			}
			else */if (TrInput & IN_WALK)
			{
				item->goalAnimState = LS_WALK_FORWARD;
			}
			else
			{
				item->goalAnimState = LS_RUN_FORWARD;
			}

			return;
		}

		if (TrInput & IN_BACK)
		{
			/*if (TrInput & IN_ACTION && LaraFloorFront(item, -item->pos.yRot, 100) < 1024 && EnableSafetyDrop)
			{
				item->goalAnimState = LS_SAFE_DROP;
			}
			else */if (TrInput & IN_WALK)
			{
				if ((rHeight < (STEPUP_HEIGHT - 1)) && (rHeight > -(STEPUP_HEIGHT - 1)) && HeightType != BIG_SLOPE)
				{
					item->goalAnimState = LS_WALK_BACK;
				}
			}
			else if (rHeight > -(STEPUP_HEIGHT - 1))
			{
				item->goalAnimState = LS_HOP_BACK;
			}

			return;
		}

		//if (TrInput & IN_SPRINT)
		//{
		//	item->goalAnimState = LS_SPRINT;
		//	return;
		//}

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

		if (TrInput & IN_ROLL && Lara.waterStatus != LW_WADE)
		{
			item->goalAnimState = LS_ROLL_FORWARD;
			return;
		}
	}
}

// State:	2
// State code:	lara_as_stop()
void lara_col_stop(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	Lara.moveAngle = 0;

	item->gravityStatus = false;
	item->fallspeed = 0;

	coll->badPos = STEPUP_HEIGHT;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesArePits = true;
	coll->slopesAreWalls = true;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (TestLaraVault(item, coll))
	{
		return;
	}

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraFall(coll))
	{
		item->goalAnimState = LS_FALL;
		item->fallspeed = 0;
		item->gravityStatus = true;

		return;
	}

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}

	ShiftItem(item, coll);

#if 1
	if (coll->midFloor != NO_HEIGHT)
	{
		item->pos.yPos += coll->midFloor;
	}
#else
	if (!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP) || coll->midFloor < 0)
	{
		item->pos.yPos += coll->midFloor;
	}
	else if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && coll->midFloor)
	{
		item->pos.yPos += SWAMP_GRAVITY;
	}
#endif
}

// State:		3
// Collision:	lara_col_forward_jump()
void lara_as_jump_forward(ITEM_INFO* item, COLL_INFO* coll)//18A34, 18B68 (F)
{
	//if (item->hitPoints <= 0)
	//{
	//	item->goalAnimState = LS_DEATH;
	//	return;
	//}

	if (item->fallspeed > LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
		return;
	}

	// Petition to ban banana jumps.
	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < ANGLE(-3.0f))
		{
			Lara.turnRate = ANGLE(-3.0f);
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > ANGLE(3.0f))
		{
			Lara.turnRate = ANGLE(3.0f);
		}
	}

	if (TrInput & IN_ACTION &&
		Lara.gunStatus == LG_NO_ARMS)
	{
		item->goalAnimState = LS_REACH;
		return;
	}

	if (TrInput & IN_WALK &&
		Lara.gunStatus == LG_NO_ARMS)
	{
		item->goalAnimState = LS_SWANDIVE_START;
		return;
	}

	if (TrInput & IN_ROLL ||
		(TrInput & IN_BACK && EnableBackKeyTurn))
	{
		item->goalAnimState = LS_JUMP_ROLL_180;
		return;
	}

	//item->goalAnimState = LS_JUMP_FORWARD;
}

// State:		3
// State code:	lara_as_forward_jump()
void lara_col_jump_forward(ITEM_INFO* item, COLL_INFO* coll)//18B88, 18CBC (F)
{
	if (item->speed < 0)
	{
		Lara.moveAngle = ANGLE(180.0f);
	}
	else
	{
		Lara.moveAngle = 0;
	}

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	LaraDeflectEdgeJump(item, coll);

	if (item->speed < 0)	// ??
	{
		Lara.moveAngle = 0;
	}

	if (coll->midFloor <= 0 && item->fallspeed > 0)
	{
		if (LaraLandedBad(item, coll))	// ?
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
				if (TrInput & IN_FORWARD && !(TrInput & IN_STEPSHIFT))	// ???
				{
					item->goalAnimState = LS_RUN_FORWARD;
				}
				else
				{
					item->goalAnimState = LS_STOP;
				}
			}
		}

		item->gravityStatus = false;
		item->fallspeed = 0;
		item->speed = 0;

		if (coll->midFloor != NO_HEIGHT)
		{
			item->pos.yPos += coll->midFloor;
		}

		AnimateLara(item);
	}
}

// State:		4
// Collision:	lara_void_func()
void lara_col_pose(ITEM_INFO* item, COLL_INFO* coll)//1B87C(<), 1B9B0(<) (F)
{
	lara_col_stop(item, coll);
}

// State:		5
// Collision:	lara_col_hop_back()
void lara_as_hop_back(ITEM_INFO* item, COLL_INFO* coll)//1959C(<), 196D0(<) (F)
{
	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < ANGLE(-6.0f))
		{
			Lara.turnRate = ANGLE(-6.0f);
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > ANGLE(6.0f))
		{
			Lara.turnRate = ANGLE(6.0f);
		}
	}

	item->goalAnimState = LS_STOP;
}

// State:		5
// State code:	lara_as_hop_back()
void lara_col_hop_back(ITEM_INFO* item, COLL_INFO* coll)//1B89C, 1B9D0 (F)
{
	Lara.moveAngle = ANGLE(180.0f);

	item->fallspeed = 0;
	item->gravityStatus = false;

	coll->slopesAreWalls = 0;
	coll->slopesArePits = true;
	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraWallDeflect(coll))
	{
		SetLaraWallDeflect(item, coll);
		LaraCollideStop(item, coll);
	}
	
	// TODO: Hop back step height check is unique among the land traversal states. Appropriate to make test functions have extra parameters to account for this one case?
	if (abs(coll->midFloor) > 0 && abs(coll->midFloor <= 200))
	{
		if (abs(coll->midFloor) >= 50)
		{
			item->pos.yPos += 50 * GetSign(coll->midFloor);
		}
		else
		{
			item->pos.yPos += coll->midFloor;
		}
	}

	if (coll->midFloor >= 200)
	{
		item->goalAnimState = LS_FALL_BACK;
		item->fallspeed = 0;
		item->gravityStatus = true;

		return;
	}

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}

	item->goalAnimState = LS_STOP;
}

// State:		6
// Collision:	lara_col_turn_right()
void lara_as_turn_right(ITEM_INFO* item, COLL_INFO* coll)//19628(<), 1975C(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	Lara.turnRate += LARA_TURN_RATE;
	if (Lara.waterStatus == LW_WADE ||
		Lara.gunStatus != LG_READY)
	{
		if (Lara.turnRate > ANGLE(4.0f))
		{
			item->goalAnimState = LS_TURN_RIGHT_FAST;  // TODO: Investigate why a 1 frame WALK input causes an early dispatch to FAST_TURN instead of SIDESTEP.
		}
	}
	else
	{
		item->goalAnimState = LS_TURN_RIGHT_FAST;
	}

	if (TrInput & IN_JUMP)
	{
		item->goalAnimState = LS_JUMP_PREPARE;
		return;
	}

	//if (TrInput & IN_SPRINT)
	//{
	//	item->goalAnimState = LS_SPRINT;
	//	return;
	//}

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

	if (TrInput & IN_ROLL && Lara.waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_ROLL_FORWARD;
		return;
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

		return;
	}

	if (!(TrInput & IN_RIGHT))
	{
		item->goalAnimState = LS_STOP;
		return;
	}
}

// State:		6
// State code:	lara_as_turn_r()
void lara_col_turn_right(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	Lara.moveAngle = 0;

	item->fallspeed = 0;
	item->gravityStatus = false;

	coll->badPos = STEPUP_HEIGHT;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = 1;
	coll->slopesArePits = true;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

#if 1
	if (coll->midFloor > 100)
#else
	if (coll->midFloor > 100 && !(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP))
#endif
	{
		item->goalAnimState = LS_FALL;
		item->fallspeed = 0;
		item->gravityStatus = true;

		return;
	}

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}

	//return;	??

#if 1
	if (coll->midFloor != NO_HEIGHT)
	{
		item->pos.yPos += coll->midFloor;
	}
#else
	if (!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP) || coll->midFloor < 0)
	{
		item->pos.yPos += coll->midFloor;
	}
	else if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && coll->midFloor)
	{
		item->pos.yPos += SWAMP_GRAVITY;
	}
#endif
}

// TODO: TURN_RIGHT_FAST is dispatched instead?!
// State:		7
// Collision:	lara_col_turn()
void lara_as_turn_left(ITEM_INFO* item, COLL_INFO* coll)//1972C(<), 19860(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	Lara.turnRate -= LARA_TURN_RATE;
	if (Lara.waterStatus == LW_WADE ||
		Lara.gunStatus != LG_READY)
	{
		if (Lara.turnRate < ANGLE(-4.0f))
		{
			item->goalAnimState = LS_TURN_LEFT_FAST;
		}
	}
	else
	{
		item->goalAnimState = LS_TURN_LEFT_FAST;
	}

	if (TrInput & IN_JUMP)
	{
		item->goalAnimState = LS_JUMP_PREPARE;
		return;
	}

	//if (TrInput & IN_SPRINT)
	//{
	//	item->goalAnimState = LS_SPRINT;
	//	return;
	//}

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

	if (TrInput & IN_ROLL && Lara.waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_ROLL_FORWARD;
		return;
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

		return;
	}

	if (!(TrInput & IN_LEFT))
	{
		item->goalAnimState = LS_STOP;
		return;
	}
}

// State:		6
// State code:	lara_as_turn_left()
void lara_col_turn_left(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	lara_col_turn_right(item, coll);
}

// State:		8
// Collision:	lara_col_death()
void lara_as_death(ITEM_INFO* item, COLL_INFO* coll)//19830(<), 19964(<) (F)
{
	Lara.look = false;

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (BinocularRange)
	{
		BinocularRange = 0;
		LaserSight = 0;
		LaraItem->meshBits = -1;
		Lara.busy = false;

		AlterFOV(ANGLE(80.0f));
	}
}

// State:		8
// State code:	lara_as_death()
void lara_col_death(ITEM_INFO* item, COLL_INFO* coll)//1BADC(<), 1BC10(<) (F)
{
	Lara.moveAngle = 0;

	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->radius = 400;
	coll->facing = Lara.moveAngle;

	StopSoundEffect(SFX_LARA_FALL);

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	ShiftItem(item, coll);

	item->hitPoints = -1;
	Lara.air = -1;

	if (coll->midFloor != NO_HEIGHT)
	{
		item->pos.yPos += coll->midFloor;
	}
}

// State:		9
// Collision:	lara_col_freefall()
void lara_as_freefall(ITEM_INFO* item, COLL_INFO* coll)//198BC(<), 199F0(<) (F)
{
	item->speed = (item->speed * 95) / 100;
	if (item->fallspeed == 154)
	{
		SoundEffect(SFX_LARA_FALL, &item->pos, 0);
	}
}

// State:		9
// State code:	lara_as_freefall()
void lara_col_freefall(ITEM_INFO* item, COLL_INFO* coll)//1BB88, 1BCBC (F)
{
	item->gravityStatus = true;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;
	coll->facing = Lara.moveAngle;

	StopSoundEffect(SFX_LARA_FALL);

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	PerformLaraSlideEdgeJump(item, coll);

	if (coll->midFloor <= 0)
	{
		if (LaraLandedBad(item, coll))
		{
			item->goalAnimState = LS_DEATH;
		}
		else
		{
			item->goalAnimState = LS_STOP;
		}

		item->fallspeed = 0;
		item->gravityStatus = false;

		if (coll->midFloor != NO_HEIGHT)
		{
			item->pos.yPos += coll->midFloor;
		}
	}
}

// State:		11
// Collision:	lara_col_reach()
void lara_as_reach(ITEM_INFO* item, COLL_INFO* coll)//18CE0(<), 18E14(<) (F)
{
	Camera.targetAngle = ANGLE(85.0f);

	if (item->fallspeed > LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
	}
}

// State:		11
// State code:	lara_as_reach()
void lara_col_reach(ITEM_INFO* item, COLL_INFO* coll)//18D0C, 18E40 (F)
{
	if (Lara.ropePtr == -1)
	{
		item->gravityStatus = true;
	}

	Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = 0;
	coll->badCeiling = BAD_JUMP_CEILING;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	short angle;
	bool result = false;
	int edge = 0;
	int edgeCatch = 0;

	if (TrInput & IN_ACTION &&
		Lara.gunStatus == LG_NO_ARMS &&
		!coll->hitStatic)
	{
		if (coll->collType == CT_TOP && Lara.canMonkeySwing)
		{
			Lara.headYrot = 0;
			Lara.headXrot = 0;
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			Lara.gunStatus = LG_HANDS_BUSY;

			// TODO: State dispatch. Doing it the obvious way results in a delay before the catch.
			item->animNumber = LA_REACH_TO_MONKEYSWING;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_MONKEYSWING_IDLE;
			item->currentAnimState = LS_MONKEYSWING_IDLE;
			item->gravityStatus = false;
			item->speed = 0;
			item->fallspeed = 0;

			return;
		}

		if (coll->midCeiling <= -384 &&
			coll->midFloor >= 200 &&
			coll->collType == CT_FRONT)
		{
			edgeCatch = LaraTestEdgeCatch(item, coll, &edge);

			if (!(!edgeCatch || edgeCatch < 0 && !LaraTestHangOnClimbWall(item, coll)))
			{
				angle = item->pos.yRot;
				if (coll->midSplitFloor && coll->frontSplitFloor == coll->midSplitFloor)
				{
					result = SnapToDiagonal(angle, 35);
				}
				else
				{
					result = SnapToQuadrant(angle, 35);
				}
			}
		}
	}

	if (!result)
	{
		PerformLaraSlideEdgeJump(item, coll);
		coll->facing = Lara.moveAngle;
		GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
		ShiftItem(item, coll);

		if (item->fallspeed > 0 && coll->midFloor <= 0)
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

				if (coll->midFloor != NO_HEIGHT)
				{
					item->pos.yPos += coll->midFloor;
				}
			}
		}
	}
	else
	{
		if (TestHangSwingIn(item, angle))
		{
			Lara.headYrot = 0;
			Lara.headXrot = 0;
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			item->animNumber = LA_REACH_TO_HANG_OSCILLATE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = LS_HANG;
			item->goalAnimState = LS_HANG;
			item->gravityStatus = false;
			item->speed = 0;
			item->fallspeed = 0;
		}
		else
		{
			if (TestHangFeet(item, angle))
			{
				item->animNumber = LA_REACH_TO_HANG;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_HANG;
				item->goalAnimState = LS_HANG_FEET;
			}
			else
			{
				item->animNumber = LA_REACH_TO_HANG;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_HANG;
				item->goalAnimState = LS_HANG;
			}
		}

		BOUNDING_BOX* bounds = GetBoundsAccurate(item);

		if (edgeCatch <= 0)
		{
			item->pos.yPos = edge - bounds->Y1 - 22;
		}
		else
		{
			item->pos.yPos += coll->frontFloor - bounds->Y1;

			if (coll->midSplitFloor)
			{
				Vector2 v = GetDiagonalIntersect(item->pos.xPos, item->pos.zPos, coll->midSplitFloor, LARA_RAD, angle);
				item->pos.xPos = v.x;
				item->pos.zPos = v.y;
			}
			else
			{
				Vector2 v = GetOrthogonalIntersect(item->pos.xPos, item->pos.zPos, LARA_RAD, angle);
				item->pos.xPos = v.x;
				item->pos.zPos = v.y;
			}
		}

		item->pos.yRot = angle;
		item->gravityStatus = true;
		item->speed = 2;
		item->fallspeed = 1;

		Lara.gunStatus = LG_HANDS_BUSY;
	}
}

void lara_as_splat(ITEM_INFO* item, COLL_INFO* coll)//1A340(<), 1A474(<) (F)
{
	Lara.look = false;
}

void lara_col_splat(ITEM_INFO* item, COLL_INFO* coll)//1BC74(<), 1BDA8(<) (F)
{
	Lara.moveAngle = 0;

	coll->slopesAreWalls = true;
	coll->slopesArePits = true;
	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	ShiftItem(item, coll);

	if (coll->midFloor >= -256 && coll->midFloor <= 256)
	{
		item->pos.yPos += coll->midFloor;
	}
}

// State:		15
// Collision:	lara_col_jump_prepare()
void lara_as_jump_prepare(ITEM_INFO* item, COLL_INFO* coll)
{
	if (Lara.waterStatus != LW_WADE)
	{
		// TODO: Check ceilings as well.
		if (TrInput & IN_FORWARD && LaraFloorFront(item, item->pos.yRot, 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_FORWARD;
			Lara.moveAngle = 0;
		}
		else if (TrInput & IN_BACK && LaraFloorFront(item, item->pos.yRot - ANGLE(180.0f), 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_BACK;
			Lara.moveAngle = ANGLE(180.0f);
		}
		else if (TrInput & IN_LEFT && LaraFloorFront(item, item->pos.yRot - ANGLE(90.0f), 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_LEFT;
			Lara.moveAngle = ANGLE(-90.0f);
		}
		else if (TrInput & IN_RIGHT && LaraFloorFront(item, item->pos.yRot + ANGLE(90.0f), 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_RIGHT;
			Lara.moveAngle = ANGLE(90.0f);
		}
		else if (TrInput & IN_ROLL)
		{
			item->goalAnimState = LS_JUMP_ROLL_180;
		}
		//else
		//{
		//	// Cancel jump preparation if the ceiling in a chosen direction is too low.
		//}
	}

	if (item->fallspeed > LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
	}
}

// State:		15
// State code:	lara_as_jump_prepare()
void lara_col_jump_prepare(ITEM_INFO* item, COLL_INFO* coll)//1BD30, 1BE64 (F)
{
	item->fallspeed = 0;
	item->gravityStatus = false;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = NO_HEIGHT;
	coll->badCeiling = 0;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (TestLaraFall(coll))
	{
		item->goalAnimState = LS_FALL;
		item->fallspeed = 0;
		item->gravityStatus = true;

		return;
	}

	// G A R B A G E.
	// TODO: Dispatch crouch if ceiling lowers.
	if (coll->midCeiling > -100)
	{
		item->animNumber = LA_STAND_SOLID;	// <- This anim needs to go!!
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = LS_STOP;
		item->currentAnimState = LS_STOP;

		item->speed = 0;
		item->fallspeed = 0;
		item->gravityStatus = false;

		item->pos.xPos = coll->old.x;
		item->pos.yPos = coll->old.y;
		item->pos.zPos = coll->old.z;
	}

	if (coll->midFloor > -256 && coll->midFloor < 256)
	{
		item->pos.yPos += coll->midFloor;
	}
}

// State:		16
// Collision:	lara_col_walk_back()
void lara_as_walk_back(ITEM_INFO* item, COLL_INFO* coll)//1A4F0(<), 1A624(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (Lara.isMoving)
	{
		return;
	}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < ANGLE(-4.0f))
		{
			Lara.turnRate = ANGLE(-4.0f);
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > ANGLE(4.0f))
		{
			Lara.turnRate = ANGLE(4.0f);
		}
	}

	if (TrInput & IN_JUMP)
	{
		item->goalAnimState = LS_JUMP_PREPARE;	// TODO
		return;
	}

	if (TrInput & IN_BACK &&
		(TrInput & IN_WALK || Lara.waterStatus == LW_WADE))
	{
		item->goalAnimState = LS_WALK_BACK;
		return;
	}
	
	item->goalAnimState = LS_STOP;
}

// State:		16
// State code:	lara_as_walk_back()
void lara_col_walk_back(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	item->gravityStatus = false;
	item->fallspeed = 0;
	Lara.moveAngle = ANGLE(180.0f);

	if (Lara.waterStatus == LW_WADE)
	{
		coll->badPos = NO_BAD_POS;
	}
	else
	{
		coll->badPos = STEPUP_HEIGHT;
	}

	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesArePits = true;
	coll->slopesAreWalls = 1;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (LaraHitCeiling(item, coll))
	{
		return;
	}

	if (LaraDeflectEdge(item, coll))
	{
		LaraCollideStop(item, coll);
	}

	if (TestLaraFall(coll))
	{
		item->goalAnimState = LS_FALL;
		item->fallspeed = 0;
		item->gravityStatus = true;

		return;
	}

	if (abs(coll->midFloor) > 0 && abs(coll->midFloor <= STEPUP_HEIGHT))
	{
		// Like the hop back, step back is also unique. TODO: Make step test/set functions take extra parameters.
		if (TestLaraStepDown(coll))
		{
			item->pos.yPos += coll->midFloor;

			item->goalAnimState = LS_STEP_BACK_DOWN;
			GetChange(item, &g_Level.Anims[item->animNumber]);
		}
		/*else if (TestLaraStepUp(coll))
		{
			item->pos.yPos += coll->midFloor;

			item->goalAnimState = LS_STEP_BACK_UP;
			GetChange(item, &g_Level.Anims[item->animNumber]);
		}*/
		else if (abs(coll->midFloor) >= 50)
		{
			item->pos.yPos += 50 * GetSign(coll->midFloor);
		}
		else
		{
			item->pos.yPos += coll->midFloor;
		}
	}

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}

#if 0
	if (!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP) || coll->midFloor < 0)
	{
		item->pos.yPos += coll->midFloor;
	}
	else if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && coll->midFloor)
	{
		item->pos.yPos += SWAMP_GRAVITY;
	}
#else
	//if (coll->midFloor != NO_HEIGHT)
	//{
	//	item->pos.yPos += coll->midFloor;
	//}
#endif
}

// State:		20
// Collision:	lara_col_fast_turn_right()
void lara_as_turn_right_fast(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	Lara.turnRate = LARA_FAST_TURN;

	if (TrInput & IN_JUMP)
	{
		item->goalAnimState = LS_JUMP_PREPARE;
		return;
	}

	if (TrInput & IN_ROLL && Lara.waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_ROLL_FORWARD;
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

	if (TrInput & IN_FORWARD)
	{
		if (Lara.waterStatus == LW_WADE)			// Failsafe.
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

		return;
	}

	if (!(TrInput & IN_RIGHT))
	{
		item->goalAnimState = LS_STOP;
		return;
	}
}

// State:		20
// State code:	lara_as_turn_right_fast()
void lara_col_turn_right_fast(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_stop(item, coll);	// TODO: Investigate why collision is different to regular turn.
}

// State:		157
// State code:	lara_as_turn_left_fast()
void lara_as_turn_left_fast(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	Lara.turnRate = -LARA_FAST_TURN;

	if (TrInput & IN_JUMP)
	{
		item->goalAnimState = LS_JUMP_PREPARE;
		return;
	}

	if (TrInput & IN_ROLL && Lara.waterStatus != LW_WADE)
	{
		item->goalAnimState = LS_ROLL_FORWARD;
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

	if (TrInput & IN_FORWARD)
	{
		if (Lara.waterStatus == LW_WADE)			// Failsafe.
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

		return;
	}

	if (!(TrInput & IN_LEFT))
	{
		item->goalAnimState = LS_STOP;
		return;
	}
}

// State:		157
// State code:	lara_as_turn_left_fast()
void lara_col_turn_left_fast(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_turn_right_fast(item, coll);
}

// State:		21
// Collision:	lara_col_sidestep()
void lara_as_step_right(ITEM_INFO* item, COLL_INFO* coll)//1A67C(<), 1A7B0(<) (F)
{
	Lara.look = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (Lara.isMoving)
	{
		return;
	}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < ANGLE(-4.0f))
		{
			Lara.turnRate = ANGLE(-4.0f);
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > ANGLE(4.0f))
		{
			Lara.turnRate = ANGLE(4.0f);
		}
	}

	if (TrInput & IN_RSTEP)
	{
		item->goalAnimState = LS_STEP_RIGHT;
		return;
	}

	item->goalAnimState = LS_STOP;
}

// State:		21
// State code:	lara_as_step_right()
void lara_col_step_right(ITEM_INFO* item, COLL_INFO* coll)//1BFB0, 1C0E4 (F)
{
	Lara.moveAngle = ANGLE(90.0f);

	item->gravityStatus = false;
	item->fallspeed = 0;

	if (Lara.waterStatus == LW_WADE)
	{
		coll->badPos = NO_BAD_POS;
	}
	else
	{
		coll->badPos = 128;
	}

	coll->slopesAreWalls = true;
	coll->slopesArePits = true;
	coll->badNeg = -128;
	coll->badCeiling = 0;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraWallDeflect(coll))
	{
		SetLaraWallDeflect(item, coll);
		LaraCollideStop(item, coll);
	}

	if (TestLaraFall(coll))
	{
		item->goalAnimState = LS_FALL;
		item->fallspeed = 0;
		item->gravityStatus = true;

		return;
	}

	if (abs(coll->midFloor) > 0 && coll->midFloor != NO_HEIGHT)
	{
		if (abs(coll->midFloor) >= 50)
		{
			item->pos.yPos += 50 * GetSign(coll->midFloor);
		}
		else
		{
			item->pos.yPos += coll->midFloor;
		}
	}

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}
}

// State:		22
// Collision:	lara_col_sidestep()
void lara_as_step_left(ITEM_INFO* item, COLL_INFO* coll)//1A750(<), 1A884(<) (F)
{
	Lara.look = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (Lara.isMoving)
	{
		return;
	}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_SLOW_TURN)
		{
			Lara.turnRate = -LARA_SLOW_TURN;
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_SLOW_TURN)
		{
			Lara.turnRate = LARA_SLOW_TURN;
		}
	}

	if (TrInput & IN_LSTEP)
	{
		item->goalAnimState = LS_STEP_LEFT;
		return;
	}

	item->goalAnimState = LS_STOP;
}

// State:		22
// State code:	lara_as_step_left()
void lara_col_step_left(ITEM_INFO* item, COLL_INFO* coll)//1BFB0, 1C0E4 (F)
{
	Lara.moveAngle = ANGLE(-90.0f);

	item->gravityStatus = false;
	item->fallspeed = 0;

	if (Lara.waterStatus == LW_WADE)
	{
		coll->badPos = NO_BAD_POS;
	}
	else
	{
		coll->badPos = 128;
	}

	coll->slopesAreWalls = true;
	coll->slopesArePits = true;
	coll->badNeg = -128;
	coll->badCeiling = 0;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraWallDeflect(coll))
	{
		SetLaraWallDeflect(item, coll);
		LaraCollideStop(item, coll);
	}

	if (TestLaraFall(coll))
	{
		item->goalAnimState = LS_FALL;
		item->fallspeed = 0;
		item->gravityStatus = true;

		return;
	}

	if (abs(coll->midFloor) > 0 && coll->midFloor != NO_HEIGHT)
	{
		if (abs(coll->midFloor) >= 50)
		{
			item->pos.yPos += 50 * GetSign(coll->midFloor);
		}
		else
		{
			item->pos.yPos += coll->midFloor;
		}
	}

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}
}

// State:		23
// State code:	lara_void_func()
void lara_col_roll2(ITEM_INFO* item, COLL_INFO* coll)//1C384, 1C4B8 (F)
{
	Camera.laraNode = 0;

	Lara.moveAngle = ANGLE(180.0f);

	item->gravityStatus = false;
	item->fallspeed = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = true;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (LaraHitCeiling(item, coll))
	{
		return;
	}

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}

	if (coll->midFloor > 200)
	{
		// TODO: not working?
		item->goalAnimState = LS_FALL_BACK;
		item->fallspeed = 0;
		item->gravityStatus = true;

		return;
	}

	ShiftItem(item, coll);

	if (coll->midFloor != NO_HEIGHT)
		item->pos.yPos += coll->midFloor;
}

// State:		25
// Cllision: lara_col_jump_back()
void lara_as_jump_back(ITEM_INFO* item, COLL_INFO* coll)//1A854(<), 1A988(<) (F)
{
	Camera.targetAngle = ANGLE(135.0f);
	Lara.look = false;

	//if (item->hitPoints <= 0)
	//{
	//	item->goalAnimState = LS_DEATH;
	//	return;
	//}

	if (item->fallspeed > LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
		return;
	}

	if ((TrInput & IN_ROLL ||
		TrInput & IN_FORWARD && EnableBackKeyTurn) &&
		item->goalAnimState != LS_STOP)
	{
		item->goalAnimState = LS_JUMP_ROLL_180;
		return;
	}

	//item->goalAnimState = LS_JUMP_BACK;
}

// State:		25
// State code:	lara_as_jump_back()
void lara_col_jump_back(ITEM_INFO* item, COLL_INFO* coll)//1C130(<), 1C264(<) (F)
{
	Lara.moveAngle = ANGLE(180.0f);
	lara_col_jump(item, coll);
}

// State:		26
// Collision:	lara_col_jump_right()
// 
void lara_as_jump_right(ITEM_INFO* item, COLL_INFO* coll)//1A8C4(<), 1A9F8(<) (F)
{
	Lara.look = false;

	//if (item->hitPoints <= 0)
	//{
	//	item->goalAnimState = LS_DEATH;
	//	return;
	//}

	if (item->fallspeed > LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
		return;
	}

	//if (TrInput & IN_ACTION)
	//{
	//	item->goalAnimState = LS_REACH;
	//	return;
	//}

	if ((TrInput & IN_ROLL ||
		TrInput & IN_LEFT && EnableBackKeyTurn) &&
		item->goalAnimState != LS_STOP)
	{
		item->goalAnimState = LS_JUMP_ROLL_180;
		return;
	}

	//item->goalAnimState = LS_JUMP_RIGHT;
}

// State:		26
// State code:	lara_as_jump_right()
void lara_col_jump_right(ITEM_INFO* item, COLL_INFO* coll)//1C15C(<), 1C290(<) (F)
{
	Lara.moveAngle = ANGLE(90.0f);
	lara_col_jump(item, coll);
}

// State:		27
// Collision: lara_col_jump_left()
void lara_as_jump_left(ITEM_INFO* item, COLL_INFO* coll)//1A92C(<), 1AA60(<) (F)
{
	Lara.look = false;

	//if (item->hitPoints <= 0)
	//{
	//	item->goalAnimState = LS_DEATH;
	//	return;
	//}

	if (item->fallspeed > LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
		return;
	}

	//if (TrInput & IN_ACTION)
	//{
	//	item->goalAnimState = LS_REACH;
	//	return;
	//}

	if ((TrInput & IN_ROLL ||
		TrInput & IN_RIGHT && EnableBackKeyTurn) &&
		item->goalAnimState != LS_STOP)
	{
		item->goalAnimState = LS_JUMP_ROLL_180;
		return;
	}

	//item->goalAnimState = LS_JUMP_LEFT;
}

// State:		27
// State code:	lara_as_jump_left()
void lara_col_jump_left(ITEM_INFO* item, COLL_INFO* coll)//1C188(<), 1C2BC(<) (F)
{
	Lara.moveAngle = ANGLE(-90.0f);
	lara_col_jump(item, coll);
}


// States:		25, 26, 27
// State code:	none; called in lara_col_jump_back(), lara_col_jump_right(), lara_col_jump_left()
void lara_col_jump(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	LaraDeflectEdgeJump(item, coll);

	if (item->fallspeed > 0 && coll->midFloor <= 0)
	{
		if (LaraLandedBad(item, coll))
		{
			item->goalAnimState = LS_DEATH;
		}
		else
		{
			item->goalAnimState = LS_STOP;
		}

		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->midFloor != NO_HEIGHT)
		{
			item->pos.yPos += coll->midFloor;
		}
	}
}

// State:		28
// Collision:	lara_col_jump_up()
void lara_as_jump_up(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (item->fallspeed > LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
		return;
	}

	item->goalAnimState = LS_JUMP_UP;
}

// State:		28
// State code:	lara_as_jump_up()
void lara_col_jump_up(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;
	coll->hitCeiling = false;
	coll->facing = item->speed < 0 ? Lara.moveAngle + ANGLE(180.0f) : Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 870);

	if (TrInput & IN_ACTION &&
		Lara.gunStatus == LG_NO_ARMS &&
		!coll->hitStatic)
	{
		if (Lara.canMonkeySwing && coll->collType == CT_TOP)
		{
			Lara.gunStatus = LG_HANDS_BUSY;

			item->goalAnimState = LS_MONKEYSWING_IDLE;
			item->gravityStatus = false;
			item->speed = 0;
			item->fallspeed = 0;

			MonkeySwingSnap(item, coll);

			return;
		}

		if (coll->collType == CT_FRONT &&
			coll->midCeiling <= -STEPUP_HEIGHT)
		{
			int edge;
			int edgeCatch = LaraTestEdgeCatch(item, coll, &edge);

			if (edgeCatch)
			{
				if (edgeCatch >= 0 || LaraTestHangOnClimbWall(item, coll))
				{
					short angle = item->pos.yRot;
					bool result;

					if (coll->midSplitFloor && coll->frontSplitFloor == coll->midSplitFloor)
					{
						result = SnapToDiagonal(angle, 35);
					}
					else
					{
						result = SnapToQuadrant(angle, 35);
					}

					if (result)
					{
						BOUNDING_BOX* bounds;

						if (TestHangSwingIn(item, angle))
						{
							item->animNumber = LA_JUMP_UP_TO_MONKEYSWING;
							item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
							item->goalAnimState = LS_MONKEYSWING_IDLE;
							item->currentAnimState = LS_MONKEYSWING_IDLE;
						}
						else
						{
							if (TestHangFeet(item, angle))
							{
								item->animNumber = LA_REACH_TO_HANG;
								item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 12;
								item->currentAnimState = LS_HANG;
								item->goalAnimState = LS_HANG_FEET;
							}
							else
							{
								item->goalAnimState = LS_HANG;
							}
						}

						bounds = GetBoundsAccurate(item);

						if (edgeCatch <= 0)
						{
							item->pos.yPos = edge - bounds->Y1 + 4;
						}
						else
						{
							item->pos.yPos += coll->frontFloor - bounds->Y1;
						}

						if (coll->midSplitFloor)
						{
							Vector2 v = GetDiagonalIntersect(item->pos.xPos, item->pos.zPos, coll->midSplitFloor, LARA_RAD, item->pos.yRot);
							item->pos.xPos = v.x;
							item->pos.zPos = v.y;
						}
						else
						{
							Vector2 v = GetOrthogonalIntersect(item->pos.xPos, item->pos.zPos, LARA_RAD, item->pos.yRot);
							item->pos.xPos = v.x;
							item->pos.zPos = v.y;
						}
						item->pos.yRot = angle;

						item->gravityStatus = false;
						item->speed = 0;
						item->fallspeed = 0;

						Lara.gunStatus = LG_HANDS_BUSY;
						Lara.torsoYrot = 0;
						Lara.torsoXrot = 0;

						return;
					}
				}
			}
		}
	}

	ShiftItem(item, coll);

	if (coll->collType == CT_CLAMP ||
		coll->collType == CT_TOP ||
		coll->collType == CT_TOP_FRONT ||
		coll->hitCeiling)
	{
		item->fallspeed = 1;
	}

	if (coll->collType == CT_NONE)
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

	if (item->fallspeed > 0 && coll->midFloor <= 0)
	{
		item->goalAnimState = LaraLandedBad(item, coll) ? LS_DEATH : LS_STOP;

		item->gravityStatus = false;
		item->fallspeed = 0;

		if (coll->midFloor != NO_HEIGHT)
		{
			item->pos.yPos += coll->midFloor;
		}
	}
}

// State:		29
// Collision:	lara_col_fall_back()
void lara_as_fall_back(ITEM_INFO* item, COLL_INFO* coll)//1959C(<), 196D0(<) (F)
{
	if (item->fallspeed > LARA_FREEFALL_SPEED)
	{
		item->goalAnimState = LS_FREEFALL;
		return;
	}

	if (TrInput & IN_ACTION &&
		Lara.gunStatus == LG_NO_ARMS)
	{
		item->goalAnimState = LS_REACH;
		return;
	}

	item->goalAnimState = LS_FALL_BACK;
}

// State:		29
// State code:	lara_as_fall_back()
void lara_col_fall_back(ITEM_INFO* item, COLL_INFO* coll)//1C1B4(<), 1C2E8(<) (F)
{
	Lara.moveAngle = ANGLE(180.0f);

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	LaraDeflectEdgeJump(item, coll);

	if (item->fallspeed > 0 && coll->midFloor <= 0)
	{
		if (LaraLandedBad(item, coll))
		{
			item->goalAnimState = LS_DEATH;
		}
		else
		{
			item->goalAnimState = LS_STOP;
		}

		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->midFloor != NO_HEIGHT)
		{
			item->pos.yPos += coll->midFloor;
		}
	}
}

// State:		45
// State code:	lara_void_func()
void lara_col_roll(ITEM_INFO* item, COLL_INFO* coll)//1C2B0, 1C3E4 (F)
{
	Lara.moveAngle = 0;

	item->gravityStatus = false;
	item->fallspeed = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesArePits = false;
	coll->slopesAreWalls = true;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (LaraHitCeiling(item, coll))
	{
		return;
	}

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}

	if (TestLaraFall(coll))
	{
		item->goalAnimState = LS_FALL;
		item->fallspeed = 0;
		item->gravityStatus = true;

		return;
	}

	if (TrInput & IN_FORWARD &&
		item->animNumber == LA_SWANDIVE_ROLL)
	{
		item->goalAnimState = LS_RUN_FORWARD;
	}

	ShiftItem(item, coll);

	if (coll->midFloor != NO_HEIGHT)
	{
		item->pos.yPos += coll->midFloor;
	}
}

// State:		52
// Collision:	lara_col_swandive_start()
void lara_as_swandive_start(ITEM_INFO* item, COLL_INFO* coll)//1AE08(<), 1AF3C(<) (F)
{
	coll->enableBaddiePush = true;
	coll->enableSpaz = false;

	if (item->fallspeed > LARA_FREEFALL_SPEED
		&& item->goalAnimState != LS_DIVE)
	{
		item->goalAnimState = LS_SWANDIVE_END;
		return;
	}

	item->goalAnimState = LS_SWANDIVE_START;
}

// State:		52
// State code:	lara_as_swandive_start()
void lara_col_swandive_start(ITEM_INFO* item, COLL_INFO* coll)//1C4A0(<), 1C5D4(<) (F)
{
	Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	LaraDeflectEdgeJump(item, coll);

	if (coll->midFloor <= 0 && item->fallspeed > 0)
	{
		item->goalAnimState = LS_STOP;
		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->midFloor != NO_HEIGHT)
		{
			item->pos.yPos += coll->midFloor;
		}
	}
}

// State:		53
// Collision:	lara_col_swandive_freefall()
void lara_as_swandive_freefall(ITEM_INFO* item, COLL_INFO* coll)//1AE4C(<), 1AF80(<) (F)
{
	item->speed = (item->speed * 95) / 100;

	coll->enableBaddiePush = true;
	coll->enableSpaz = false;

	if (TrInput & IN_ROLL ||
		(TrInput & IN_BACK && EnableBackKeyTurn))
	{
		item->goalAnimState = LS_JUMP_ROLL_180;
		return;
	}

	item->goalAnimState = LS_SWANDIVE_END;
}

// State:		53
// State code:	lara_as_swandive_freefall()
void lara_col_swandive_freefall(ITEM_INFO* item, COLL_INFO* coll)//1C558(<), 1C68C(<) (F)
{
	Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	LaraDeflectEdgeJump(item, coll);

	if (coll->midFloor <= 0 && item->fallspeed > 0)
	{
		if (item->fallspeed <= 133)
		{
			item->goalAnimState = LS_STOP;
		}
		else
		{
			item->goalAnimState = LS_DEATH;
		}

		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->midFloor != NO_HEIGHT)
		{
			item->pos.yPos += coll->midFloor;
		}
	}
}

// State:		54
// Collision:	lara_default_col()
void lara_as_gymnast(ITEM_INFO* item, COLL_INFO* coll)//1AEC8(<), 1AFFC(<) (F)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
}

// State:		65
// Collision:	lara_col_wade()
void lara_as_wade(ITEM_INFO* item, COLL_INFO* coll)//1AF10, 1B044 (F)
{
	Camera.targetElevation = ANGLE(-22.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP)
	{
		if (TrInput & IN_LEFT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -(LARA_FAST_TURN / 2))
			{
				Lara.turnRate = -(LARA_FAST_TURN / 2);
			}
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > (LARA_FAST_TURN / 2))
			{
				Lara.turnRate = (LARA_FAST_TURN / 2);
			}
		}

		if (TrInput & IN_FORWARD)
		{
			item->goalAnimState = LS_WADE_FORWARD;
			return;
		}
	}
	else
	{
		if (TrInput & IN_LEFT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -LARA_FAST_TURN)
			{
				Lara.turnRate = -LARA_FAST_TURN;
			}
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > LARA_FAST_TURN)
			{
				Lara.turnRate = LARA_FAST_TURN;
			}
		}

		if (TrInput & IN_FORWARD)
		{
			if (Lara.waterStatus == LW_ABOVE_WATER)
			{
				item->goalAnimState = LS_RUN_FORWARD;
			}
			else
			{
				item->goalAnimState = LS_WADE_FORWARD;
			}

			return;
		}
	}

	item->goalAnimState = LS_STOP;
}

// State:		65
// State code:	lara_as_wade()
void lara_col_wade(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = true;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP)
	{
		if (TrInput & IN_LEFT)
		{
			if (TestLaraLean(coll))
			{
				item->pos.zRot -= LARA_LEAN_RATE;
				if (item->pos.zRot < -(LARA_LEAN_MAX / 2))
				{
					item->pos.zRot = -(LARA_LEAN_MAX / 2);
				}
			}
		}
		else if (TrInput & IN_RIGHT)
		{
			if (TestLaraLean(coll))
			{
				item->pos.zRot += LARA_LEAN_RATE;
				if (item->pos.zRot > (LARA_LEAN_MAX / 2))
				{
					item->pos.zRot = (LARA_LEAN_MAX / 2);
				}
			}
		}
	}
	else
	{
		if (TrInput & IN_LEFT)
		{
			if (TestLaraLean(coll))
			{
				item->pos.zRot -= LARA_LEAN_RATE;
				if (item->pos.zRot < -LARA_LEAN_MAX)
				{
					item->pos.zRot = -LARA_LEAN_MAX;
				}
			}
		}
		else if (TrInput & IN_RIGHT)
		{
			if (TestLaraLean(coll))
			{
				item->pos.zRot += LARA_LEAN_RATE;
				if (item->pos.zRot > LARA_LEAN_MAX)
				{
					item->pos.zRot = LARA_LEAN_MAX;
				}
			}
		}
	}

	if (TestLaraVault(item, coll))
	{
		return;
	}

	if (LaraHitCeiling(item, coll))
	{
		return;
	}

	if (LaraDeflectEdge(item, coll))
	{
		item->pos.zRot = 0;


		if ((coll->frontType == WALL || coll->frontType == SPLIT_TRI) &&
			coll->frontFloor < -((STEP_SIZE * 5) / 2) &&
			!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP))
		{
			item->goalAnimState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->animNumber]))
			{
				return;
			}
		}

		LaraCollideStop(item, coll);
	}

	if (abs(coll->midFloor) > 0 && abs(coll->midFloor <= STEPUP_HEIGHT) &&
		!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP))
	{
		//DoLaraStep(item, coll);	// Uncomment this line and remove everything below when a step down anim is created.

		/*if (TestLaraStepDown(coll))
		{
			item->pos.yPos += coll->midFloor;

			item->goalAnimState = LS_STEP_DOWN;
			GetChange(item, &g_Level.Anims[item->animNumber]);
		}
		else */if (TestLaraStepUp(coll))
		{
			item->pos.yPos += coll->midFloor;

			item->goalAnimState = LS_STEP_UP;
			GetChange(item, &g_Level.Anims[item->animNumber]);
		}
		else if (abs(coll->midFloor) >= 50)
		{
			item->pos.yPos += 50 * GetSign(coll->midFloor);
		}
		else
		{
			item->pos.yPos += coll->midFloor;
		}
	}

	/*if (!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP) || coll->midFloor < 0)
	{
		item->pos.yPos += coll->midFloor;   // Enforce to floor height.. if not a swamp room.
	}
	else */if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && coll->midFloor)
	{
		item->pos.yPos += SWAMP_GRAVITY;
	}
}

// State:		73
// Collision:	lara_col_sprint()
void lara_as_sprint(ITEM_INFO* item, COLL_INFO* coll)//15A28, 15B5C (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_RUN_FORWARD;	// item->goalAnimState = LS_DEATH;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < ANGLE(-4.0f))
		{
			Lara.turnRate = ANGLE(-4.0f);
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > ANGLE(4.0f))
		{
			Lara.turnRate = ANGLE(4.0f);
		}
	}

	if (TrInput & IN_JUMP || item->gravityStatus)
	{
		if (LaraFloorFront(item, item->pos.yRot, 1600) >= 384)	// Something fun. However, the range should be probed rather than the final location.
		{
			item->goalAnimState = LS_SWANDIVE_START;
		}
		else
		{
			item->goalAnimState = LS_SPRINT_ROLL;
		}

		return;
	}

	if (TrInput & IN_DUCK &&
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

	if (!(TrInput & IN_SPRINT) || !DashTimer ||
		Lara.waterStatus == LW_WADE)
	{
		 item->goalAnimState = LS_RUN_FORWARD;
		return;
	}

	DashTimer--;

	if (TrInput & IN_FORWARD)
	{
		if (TrInput & IN_WALK)
		{
			item->goalAnimState = LS_WALK_FORWARD;
		}
		else
		{
			item->goalAnimState = LS_SPRINT;
		}
		
		return;
	}

	// Legacy doesn't require FORWARD to be held when sprinting. Keep original behaviour?
	//if (TrInput & IN_LEFT || TrInput & IN_RIGHT)
	//{
	//	item->goalAnimState = LS_SPRINT;
	//	return;
	//}

	item->goalAnimState = LS_STOP;
}

// State:		73
// State code:	lara_as_sprint()
void lara_col_sprint(ITEM_INFO* item, COLL_INFO* coll)//15C50, 15D84 (F)
{
	Lara.moveAngle = 0;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = true;
	coll->facing = Lara.moveAngle;

	if (TrInput & IN_LEFT)
	{
		if (TestLaraLean(coll))
		{
			item->pos.zRot -= ANGLE(1.5f);
			if (item->pos.zRot < ANGLE(-16.0f))
			{
				item->pos.zRot = ANGLE(-16.0f);
			}
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		item->pos.zRot += ANGLE(1.5f);
		if (item->pos.zRot > ANGLE(16.0f))
		{
			item->pos.zRot = ANGLE(16.0f);
		}
	}

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);

	if (TestLaraVault(item, coll))
	{
		return;
	}

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (LaraDeflectEdge(item, coll))
	{
		item->pos.zRot = 0;

		if (TestWall(item, 256, 0, -640))
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
		item->goalAnimState = LS_FALL;
		item->fallspeed = 0;
		item->gravityStatus = true;

		return;
	}

	// TODO: Step dispatches are the normal run step up anims. This is why Lara sprints up stairs like a dolt.
	if (abs(coll->midFloor) > 0 && abs(coll->midFloor <= STEPUP_HEIGHT))
	{
		/*if (TestLaraStepDown(coll))
		{
			item->pos.yPos += coll->midFloor;

			item->goalAnimState = LS_STEP_DOWN;
			GetChange(item, &g_Level.Anims[item->animNumber]);
		}
		else */if (TestLaraStepUp(coll))
		{
			item->pos.yPos += coll->midFloor;

			item->goalAnimState = LS_STEP_UP;
			GetChange(item, &g_Level.Anims[item->animNumber]);
		}
		else if (abs(coll->midFloor) >= 50)
		{
			item->pos.yPos += 50 * GetSign(coll->midFloor);
		}
		else
		{
			item->pos.yPos += coll->midFloor;
		}
	}

	if (TestLaraSlide(coll))
	{
		SetLaraSlide(item, coll);
		return;
	}
}

// State:		74
// Collision:	lara_col_sprint_roll()
void lara_as_sprint_roll(ITEM_INFO* item, COLL_INFO* coll)//15E1C(<), 15F50(<) (F)
{
	//if (item->hitPoints <= 0)
	//{
	//	item->goalAnimState = LS_DEATH;
	//	return;
	//}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < ANGLE(-4.0f))
		{
			Lara.turnRate = ANGLE(-4.0f);
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > ANGLE(4.0f))
		{
			Lara.turnRate = ANGLE(4.0f);
		}
	}

	if (TrInput & IN_FORWARD)					// Not necessary...
	{
		item->goalAnimState = LS_RUN_FORWARD;
		return;
	}

	item->goalAnimState = LS_STOP;

	//LEGACY.
	//if (item->goalAnimState != LS_DEATH &&
	//	item->goalAnimState != LS_STOP &&
	//	item->goalAnimState != LS_RUN_FORWARD &&
	//	item->fallspeed > LARA_FREEFALL_SPEED)
	//{
	//	item->goalAnimState = LS_FREEFALL;			// Hmm? It's not possible to go into a freefall from this state.
	//}
}

// State:		74
// State code:	lara_as_sprint_dive()
void lara_col_sprint_roll(ITEM_INFO* item, COLL_INFO* coll)//15E5C, 15F90 (F)
{
	if (item->speed < 0)					// Is this necessary??
	{
		Lara.moveAngle = ANGLE(180.0f);
	}
	else
	{
		Lara.moveAngle = 0;
	}

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -256;
	coll->badCeiling = BAD_JUMP_CEILING;
	coll->slopesAreWalls = true;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HITE);
	LaraDeflectEdgeJump(item, coll);

	if (TrInput & IN_LEFT)
	{
		if (TestLaraLean(coll))
		{
			item->pos.zRot -= ANGLE(1.5f);
			if (item->pos.zRot < ANGLE(-16.0f))
			{
				item->pos.zRot = ANGLE(-16.0f);
			}
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		item->pos.zRot += ANGLE(1.5f);
		if (item->pos.zRot > ANGLE(16.0f))
		{
			item->pos.zRot = ANGLE(16.0f);
		}
	}

	if (TestLaraFall(coll))
	{
		item->goalAnimState = LS_FALL;
		item->fallspeed = 0;
		item->gravityStatus = true;

		return;

		if (item->speed < 0)
		{
			Lara.moveAngle = 0;
		}

		if (coll->midFloor <= 0 && item->fallspeed > 0)
		{
			item->gravityStatus = false;
			item->fallspeed = 0;
			item->pos.yPos += coll->midFloor;
			item->speed = 0;

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

			AnimateLara(item);
		}

		ShiftItem(item, coll);

		if (coll->midFloor != NO_HEIGHT)
		{
			item->pos.yPos += coll->midFloor;
		}
	}
}
