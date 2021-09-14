#include "framework.h"
#include "lara.h"
#include "lara_basic.h"
#include "lara_tests.h"
#include "lara_collide.h"
#include "lara_slide.h"
#include "lara_monkey.h"
#include "input.h"
#include "health.h"
#include "Sound\sound.h"
#include "draw.h"
#include "pickup.h"
#include "collide.h"
#include "item.h"
#include "camera.h"

bool DoJump = false;

/*generic functions*/
void lara_void_func(ITEM_INFO* item, COLL_INFO* coll)
{
	return;
}

void lara_default_col(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot;
	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesArePits = true;
	coll->slopesAreWalls = true;
	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);
}

void lara_as_special(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(170.0f);
	Camera.targetElevation = -ANGLE(25.0f);
}

void lara_as_null(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
}

void lara_as_controlled(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.look = false;
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
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
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
}

/*end generic functions*/
/*-*/
/*basic movement*/
void lara_as_walk(ITEM_INFO* item, COLL_INFO* coll)
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
			if (Lara.turnRate < -ANGLE(4.0f))
				Lara.turnRate = -ANGLE(4.0f);
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > ANGLE(4.0f))
				Lara.turnRate = ANGLE(4.0f);
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

void lara_col_walk(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 0*/
	/*state code: lara_as_walk*/
	item->gravityStatus = false;
	item->fallspeed = 0;

	Lara.moveAngle = item->pos.yRot;

	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->slopesAreWalls = true;
	coll->slopesArePits = true;
	coll->lavaIsPit = 1;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);

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
			if (coll->middle.Floor > STEP_SIZE / 2)
			{
				if (coll->front.Floor == NO_HEIGHT || coll->front.Floor <= STEP_SIZE / 2)
				{
					coll->middle.Floor = 0;
				}
				else
				{
					item->goalAnimState = LS_STEP_DOWN;
					GetChange(item, &g_Level.Anims[item->animNumber]);
				}
			}
			if (coll->middle.Floor >= -STEPUP_HEIGHT && coll->middle.Floor < -STEP_SIZE / 2)
			{
				if (coll->front.Floor == NO_HEIGHT ||
					coll->front.Floor < -STEPUP_HEIGHT ||
					coll->front.Floor >= -STEP_SIZE / 2)
				{
					coll->middle.Floor = 0;
				}
				else
				{
					item->goalAnimState = LS_STEP_UP;
					GetChange(item, &g_Level.Anims[item->animNumber]);
				}
			}

			if (!TestLaraSlide(item, coll) && coll->middle.Floor != NO_HEIGHT)
				item->pos.yPos += coll->middle.Floor;
		}
	}
}

void lara_as_run(ITEM_INFO* item, COLL_INFO* coll)
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

		if (TestLaraLean(item, coll))
		{
			item->pos.zRot -= LARA_LEAN_RATE;
			if (item->pos.zRot < -LARA_LEAN_MAX)
				item->pos.zRot = -LARA_LEAN_MAX;
		}
		else
		{
			item->pos.zRot -= LARA_LEAN_RATE;
			if (item->pos.zRot < -LARA_LEAN_MAX * 3 / 5)//gives best results
				item->pos.zRot = -LARA_LEAN_MAX * 3 / 5;
		}
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_FAST_TURN)
			Lara.turnRate = LARA_FAST_TURN;

		if (TestLaraLean(item, coll))
		{
			item->pos.zRot += LARA_LEAN_RATE;
			if (item->pos.zRot > LARA_LEAN_MAX)
				item->pos.zRot = LARA_LEAN_MAX;
		}
		else
		{
			item->pos.zRot += LARA_LEAN_RATE;
			if (item->pos.zRot > LARA_LEAN_MAX * 3 / 5)//gives best results
				item->pos.zRot = LARA_LEAN_MAX * 3 / 5;
		}
	}

	if (item->animNumber == LA_STAND_TO_RUN)
	{
		DoJump = false;
	}
	else if (item->animNumber == LA_RUN)
	{
		if (item->frameNumber == 4)
			DoJump = true;
	}
	else
	{
		DoJump = true;
	}

	if (TrInput & IN_JUMP && DoJump && !item->gravityStatus)
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

void lara_col_run(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 1*/
	/*state code: lara_col_run*/
	Lara.moveAngle = item->pos.yRot;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->slopesAreWalls = true;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);

	if (!LaraHitCeiling(item, coll) && !TestLaraVault(item, coll))
	{
		if (LaraDeflectEdge(item, coll))
		{
			item->pos.zRot = 0;

			if (coll->hitTallBounds || TestWall(item, 256, 0, -640))
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
			if (coll->middle.Floor >= -STEPUP_HEIGHT && coll->middle.Floor < -STEP_SIZE / 2)
			{
				if (coll->front.Floor == NO_HEIGHT || coll->front.Floor < -STEPUP_HEIGHT || coll->front.Floor >= -STEP_SIZE / 2)
				{
					coll->middle.Floor = 0;
				}
				else
				{
					item->goalAnimState = LS_STEP_UP;
					GetChange(item, &g_Level.Anims[item->animNumber]);
				}
			}

			if (!TestLaraSlide(item, coll))
			{
				if (coll->middle.Floor < 50)
				{
					if (coll->middle.Floor != NO_HEIGHT)
						item->pos.yPos += coll->middle.Floor;
				}
				else
				{
					item->goalAnimState = LS_STEP_DOWN; // for theoretical running stepdown anims, not in default anims
					if (GetChange(item, &g_Level.Anims[item->animNumber]))
						item->pos.yPos += coll->middle.Floor; // move Lara to middle.Floor
					else
						item->pos.yPos += 50; // do the default aligment
				}
			}
		}
	}
}

void lara_as_stop(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 2*/
	/*collision: lara_col_stop*/
	COLL_RESULT fheight = {}; fheight.FloorHeight = NO_HEIGHT;
	COLL_RESULT rheight = {}; rheight.FloorHeight = NO_HEIGHT;

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
		&&  Lara.waterStatus != LW_WADE
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

	if (TrInput & IN_FORWARD)
		fheight = LaraCollisionFront(item, item->pos.yRot, LARA_RAD + 4);
	else if (TrInput & IN_BACK)
		rheight = LaraCollisionFront(item, item->pos.yRot - ANGLE(180.0f), LARA_RAD + 4); // TR3: item->pos.yRot + ANGLE(180) ?

	if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP)
	{
		if (TrInput & IN_LEFT)
			item->goalAnimState = LS_TURN_LEFT_SLOW;
		else if (TrInput & IN_RIGHT)
			item->goalAnimState = LS_TURN_RIGHT_SLOW;
	}
	else
	{
		if (TrInput & IN_LSTEP)
		{
			auto collFloorResult = LaraCollisionFront(item, item->pos.yRot - ANGLE(90.0f), LARA_RAD + 48);
			auto collCeilingResult = LaraCeilingCollisionFront(item, item->pos.yRot - ANGLE(90.0f), LARA_RAD + 48, LARA_HEIGHT);

			if ((collFloorResult.FloorHeight < 128 && collFloorResult.FloorHeight > -128) && collFloorResult.HeightType != BIG_SLOPE && collCeilingResult.CeilingHeight <= 0)
				item->goalAnimState = LS_STEP_LEFT;
		}
		else if (TrInput & IN_RSTEP)
		{
			auto collFloorResult = LaraCollisionFront(item, item->pos.yRot + ANGLE(90.0f), LARA_RAD + 48);
			auto collCeilingResult = LaraCeilingCollisionFront(item, item->pos.yRot + ANGLE(90.0f), LARA_RAD + 48, LARA_HEIGHT);

			if ((collFloorResult.FloorHeight < 128 && collFloorResult.FloorHeight > -128) && collFloorResult.HeightType != BIG_SLOPE && collCeilingResult.CeilingHeight <= 0)
				item->goalAnimState = LS_STEP_RIGHT;
		}
		else if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_TURN_LEFT_SLOW;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_TURN_RIGHT_SLOW;
		}
	}

	if (Lara.waterStatus == LW_WADE)
	{
		if (TrInput & IN_JUMP && !(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP))
			item->goalAnimState = LS_JUMP_PREPARE;

		if (TrInput & IN_FORWARD)
		{
			bool wade = false;

			if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP)
			{
				if (fheight.FloorHeight > -(STEPUP_HEIGHT - 1))
				{
					lara_as_wade(item, coll);
					wade = true;
				}
			}
			else
			{
				if ((fheight.FloorHeight < (STEPUP_HEIGHT - 1)) && (fheight.FloorHeight > -(STEPUP_HEIGHT - 1)))
				{
					lara_as_wade(item, coll);
					wade = true;
				}
			}

			if (!wade)
			{
				Lara.moveAngle = item->pos.yRot;
				coll->badPos = NO_BAD_POS;
				coll->badNeg = -STEPUP_HEIGHT;
				coll->badCeiling = 0;
				coll->slopesAreWalls = true;
				coll->radius = LARA_RAD + 2;
				coll->facing = Lara.moveAngle;

				GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);
				if (TestLaraVault(item, coll))
					return;

				coll->radius = LARA_RAD;
			}
		}
		else if (TrInput & IN_BACK)
		{
			if ((rheight.FloorHeight < (STEPUP_HEIGHT - 1)) && (rheight.FloorHeight > -(STEPUP_HEIGHT - 1)))
				lara_as_back(item, coll);
		}
	}
	else
	{
		if (TrInput & IN_JUMP)
		{
			item->goalAnimState = LS_JUMP_PREPARE;
		}
		else if (TrInput & IN_FORWARD)
		{
			auto collFloorResult = LaraCollisionFront(item, item->pos.yRot, LARA_RAD + 4);
			auto collCeilingResult = LaraCeilingCollisionFront(item, item->pos.yRot, LARA_RAD + 4, LARA_HEIGHT);

			if ((collFloorResult.HeightType == BIG_SLOPE || collFloorResult.HeightType == DIAGONAL) && (collFloorResult.FloorHeight < 0 || collCeilingResult.CeilingHeight > 0))
			{
				item->goalAnimState = LS_STOP;
				return;
			}
			if (TrInput & IN_WALK)
				lara_as_walk(item, coll);
			else
				lara_as_run(item, coll);
		}
		else if (TrInput & IN_BACK)
		{
			if (TrInput & IN_WALK)
			{
				if ((rheight.FloorHeight < (STEPUP_HEIGHT - 1)) && (rheight.FloorHeight > -(STEPUP_HEIGHT - 1)) && rheight.HeightType != BIG_SLOPE)
					lara_as_back(item, coll);
			}
			else if (rheight.FloorHeight > -(STEPUP_HEIGHT - 1))
			{
				item->goalAnimState = LS_HOP_BACK;
			}
		}
	}
}

void lara_col_stop(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 2*/
	/*state code: lara_as_stop*/
	Lara.moveAngle = item->pos.yRot;
	coll->badPos = STEPUP_HEIGHT;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->slopesArePits = true;
	coll->slopesAreWalls = true;
	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);

	if (LaraHitCeiling(item, coll))
		return;

	if (LaraFallen(item, coll))
		return;

	if (TestLaraSlide(item, coll))
		return;

	ShiftItem(item, coll);

#if 1
	if (coll->middle.Floor != NO_HEIGHT)
		item->pos.yPos += coll->middle.Floor;
#else
	if (!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP) || coll->middle.Floor < 0)
		item->pos.yPos += coll->middle.Floor;
	else if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && coll->middle.Floor)
		item->pos.yPos += SWAMP_GRAVITY;
#endif
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

		if (Lara.turnRate < -ANGLE(3.0f))
			Lara.turnRate = -ANGLE(3.0f);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;

		if (Lara.turnRate > ANGLE(3.0f))
			Lara.turnRate = ANGLE(3.0f);
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

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);
	LaraDeflectEdgeJump(item, coll);

	if (item->speed < 0)
		Lara.moveAngle = item->pos.yRot;

	if (coll->middle.Floor <= 0 && item->fallspeed > 0)
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

		if (coll->middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->middle.Floor;

		AnimateLara(item);
	}
}

void lara_col_pose(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 4*/
	/*state code: lara_void_func*/
	lara_col_stop(item, coll);
}

void lara_as_fastback(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state: 5*/
	/*collision: lara_col_fastback*/
	item->goalAnimState = LS_STOP;
	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -ANGLE(6.0f))
			Lara.turnRate = -ANGLE(6.0f);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > ANGLE(6.0f))
			Lara.turnRate = ANGLE(6.0f);
	}
}

void lara_col_fastback(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state: 5*/
	/*state code: lara_as_fastback*/
	item->fallspeed = 0;
	item->gravityStatus = false;

	Lara.moveAngle = item->pos.yRot + ANGLE(180);

	coll->slopesAreWalls = false;
	coll->slopesArePits = true;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);

	if (!LaraHitCeiling(item, coll))
	{
		if (coll->middle.Floor <= 200)
		{
			if (LaraDeflectEdge(item, coll))
				LaraCollideStop(item, coll);

			if (!TestLaraSlide(item, coll) && coll->middle.Floor != NO_HEIGHT)
				item->pos.yPos += coll->middle.Floor;
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

void lara_as_turn_r(ITEM_INFO* item, COLL_INFO* coll)
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
		if (Lara.turnRate > ANGLE(4.0f))
		{
			if (TrInput & IN_WALK)
				Lara.turnRate = ANGLE(4.0f);
			else
				item->goalAnimState = LS_TURN_FAST;
		}
	}
	else
	{
		item->goalAnimState = LS_TURN_FAST;
	}

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

void lara_col_turn_r(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state: 6*/
	/*state code: lara_as_turn_r*/
	item->fallspeed = 0;
	item->gravityStatus = false;
	Lara.moveAngle = item->pos.yRot;
	coll->badPos = STEPUP_HEIGHT;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = true;
	coll->slopesArePits = true;
	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);

#if 1
	if (coll->middle.Floor > 100)
#else
	if (coll->middle.Floor > 100 && !(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP))
#endif
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

#if 1
	if (coll->middle.Floor != NO_HEIGHT)
		item->pos.yPos += coll->middle.Floor;
#else
	if (!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP) || coll->middle.Floor < 0)
		item->pos.yPos += coll->middle.Floor;
	else if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && coll->middle.Floor)
		item->pos.yPos += SWAMP_GRAVITY;
#endif
}

void lara_as_turn_l(ITEM_INFO* item, COLL_INFO* coll)
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
		if (Lara.turnRate < -ANGLE(4.0f))
		{
			if (TrInput & IN_WALK)
				Lara.turnRate = -ANGLE(4.0f);
			else
				item->goalAnimState = LS_TURN_FAST;
		}
	}
	else
	{
		item->goalAnimState = LS_TURN_FAST;
	}

	if (!(TrInput & IN_FORWARD))
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

void lara_col_turn_l(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 7*/
	/*state code: lara_as_turn_l*/
	lara_col_turn_r(item, coll);
}

void lara_as_death(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 8*/
	/*collision: lara_col_death*/
	Lara.look = false;
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	if (BinocularRange)
	{
		BinocularRange = 0;
		LaserSight = 0;
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
	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->radius = 400;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);
	ShiftItem(item, coll);

	item->hitPoints = -1;
	Lara.air = -1;

	if (coll->middle.Floor != NO_HEIGHT)
		item->pos.yPos += coll->middle.Floor;
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

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);
	LaraSlideEdgeJump(item, coll);

	if (coll->middle.Floor <= 0)
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

		if (coll->middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->middle.Floor;
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

	coll->badPos = NO_BAD_POS;
	coll->badNeg = 0;
	coll->badCeiling = BAD_JUMP_CEILING;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);

	short angle;
	bool result = false;
	int edge = 0;
	int edgeCatch = 0;

	if (TrInput & IN_ACTION && Lara.gunStatus == LG_NO_ARMS && !coll->hitStatic)
	{
		if (Lara.canMonkeySwing && coll->collType == CT_TOP)
		{
			Lara.headYrot = 0;
			Lara.headXrot = 0;
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			Lara.gunStatus = LG_HANDS_BUSY;

			item->animNumber = LA_REACH_TO_MONKEYSWING;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_MONKEYSWING_IDLE;
			item->currentAnimState = LS_MONKEYSWING_IDLE;
			item->gravityStatus = false;
			item->speed = 0;
			item->fallspeed = 0;

			return;
		}

		if (coll->middle.Ceiling <= -384 &&
			coll->middle.Floor >= 200 &&
			coll->collType == CT_FRONT)
		{
			edgeCatch = LaraTestEdgeCatch(item, coll, &edge);

			if (!(!edgeCatch || edgeCatch < 0 && !LaraTestHangOnClimbWall(item, coll)))
			{
				angle = item->pos.yRot;
				/*if (coll->middle.SplitFloor && coll->front.SplitFloor == coll->middle.SplitFloor)
				{
					result = SnapToDiagonal(angle, 35);
				}
				else*/
				{
					result = SnapToQuadrant(angle, 35);
				}
			}
		}
	}

	if (!result)
	{
		LaraSlideEdgeJump(item, coll);
		coll->facing = Lara.moveAngle;
		GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);
		ShiftItem(item, coll);

		if (item->fallspeed > 0 && coll->middle.Floor <= 0)
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
				if (coll->middle.Floor != NO_HEIGHT)
					item->pos.yPos += coll->middle.Floor;
			}
		}
	}
	else
	{
		if (TestHangSwingIn(item, angle))
		{
			if (Lara.NewAnims.OscillateHanging)
			{
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				item->animNumber = LA_REACH_TO_HANG_OSCILLATE;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_HANG;
				item->goalAnimState = LS_HANG;
			}
			else
			{
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				item->animNumber = LA_REACH_TO_MONKEYSWING;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_MONKEYSWING_IDLE;
				item->goalAnimState = LS_MONKEYSWING_IDLE;
			}
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
			item->pos.yPos += coll->front.Floor - bounds->Y1;

			/*if (coll->middle.SplitFloor)
			{
				Vector2 v = GetDiagonalIntersect(item->pos.xPos, item->pos.zPos, coll->middle.SplitFloor, LARA_RAD, angle);
				item->pos.xPos = v.x;
				item->pos.zPos = v.y;
			}
			else*/
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

void lara_as_splat(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.look = false;
}

void lara_col_splat(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.moveAngle = item->pos.yRot;

	coll->slopesAreWalls = true;
	coll->slopesArePits = true;

	coll->badPos = 384;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);
	ShiftItem(item, coll);

	if (coll->middle.Floor >= -256 && coll->middle.Floor <= 256)
		item->pos.yPos += coll->middle.Floor;
}

void lara_col_land(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 14*/
	/*state code: lara_void_func*/
	lara_col_stop(item, coll);
}

void lara_as_compress(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 15*/
	/*collision: lara_col_compress*/
	if (Lara.waterStatus != LW_WADE)
	{
		if (TrInput & IN_FORWARD && !LaraFacingCorner(item, item->pos.yRot, 256) && LaraFloorFront(item, item->pos.yRot, 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_FORWARD;
			Lara.moveAngle = item->pos.yRot;
		}
		else if (TrInput & IN_LEFT && !LaraFacingCorner(item, item->pos.yRot - ANGLE(90.0f), 256) && LaraFloorFront(item, item->pos.yRot - ANGLE(90.0f), 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_LEFT;
			Lara.moveAngle = item->pos.yRot - ANGLE(90);
		}
		else if (TrInput & IN_RIGHT && !LaraFacingCorner(item, item->pos.yRot + ANGLE(90.0f), 256) && LaraFloorFront(item, item->pos.yRot + ANGLE(90.0f), 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_RIGHT;
			Lara.moveAngle = item->pos.yRot + ANGLE(90);
		}
		else if (TrInput & IN_BACK && !LaraFacingCorner(item, item->pos.yRot - ANGLE(180.0f), 256) && LaraFloorFront(item, item->pos.yRot - ANGLE(180.0f), 256) >= -384)
		{
			item->goalAnimState = LS_JUMP_BACK;
			Lara.moveAngle = item->pos.yRot + ANGLE(180);
		}
	}

	if (item->fallspeed > LARA_FREEFALL_SPEED)
		item->goalAnimState = LS_FREEFALL;
}

void lara_col_compress(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 15*/
	/*state code: lara_as_compress*/
	item->fallspeed = 0;
	item->gravityStatus = false;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = NO_HEIGHT;
	coll->badCeiling = 0;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);

	if (!LaraFallen(item, coll))
	{
		if (coll->middle.Ceiling > -100)
		{
			item->animNumber = LA_STAND_SOLID;
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

		if (coll->middle.Floor > -256 && coll->middle.Floor < 256)
			item->pos.yPos += coll->middle.Floor;
	}
}

void lara_as_back(ITEM_INFO* item, COLL_INFO* coll)
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
			if (Lara.turnRate < -ANGLE(4.0f))
				Lara.turnRate = -ANGLE(4.0f);
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > ANGLE(4.0f))
				Lara.turnRate = ANGLE(4.0f);
		}
	}
}

void lara_col_back(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 16*/
	/*state code: lara_as_back*/
	item->gravityStatus = false;
	item->fallspeed = 0;
	Lara.moveAngle = item->pos.yRot + ANGLE(180);

	if (Lara.waterStatus == LW_WADE)
		coll->badPos = NO_BAD_POS;
	else
		coll->badPos = STEPUP_HEIGHT;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesArePits = true;
	coll->slopesAreWalls = true;
	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);

	if (LaraHitCeiling(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (LaraFallen(item, coll))
		return;

	if (coll->middle.Floor > STEP_SIZE / 2 && coll->middle.Floor < STEPUP_HEIGHT)
	{
		item->goalAnimState = LS_STEP_BACK_DOWN;
		GetChange(item, &g_Level.Anims[item->animNumber]);
	}

	if (TestLaraSlide(item, coll))
		return;

#if 0
	if (!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP) || coll->middle.Floor < 0)
		item->pos.yPos += coll->middle.Floor;
	else if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && coll->middle.Floor)
		item->pos.yPos += SWAMP_GRAVITY;
#else
	if (coll->middle.Floor != NO_HEIGHT)
		item->pos.yPos += coll->middle.Floor;
#endif
}

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

void lara_col_fastturn(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 20*/
	/*state code: lara_as_fastturn*/
	lara_col_stop(item, coll);
}

void lara_as_stepright(ITEM_INFO* item, COLL_INFO* coll)
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
			if (Lara.turnRate < -ANGLE(4.0f))
				Lara.turnRate = -ANGLE(4.0f);
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > ANGLE(4.0f))
				Lara.turnRate = ANGLE(4.0f);
		}
	}
}

void lara_col_stepright(ITEM_INFO* item, COLL_INFO* coll)
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
		coll->badPos = NO_BAD_POS;
	else
		coll->badPos = 128;

	coll->slopesAreWalls = true;
	coll->slopesArePits = true;

	coll->badNeg = -128;
	coll->badCeiling = 0;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);

	if (!LaraHitCeiling(item, coll))
	{
		if (LaraDeflectEdge(item, coll))
			LaraCollideStop(item, coll);

		if (!LaraFallen(item, coll) && !TestLaraSlide(item, coll) && coll->middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->middle.Floor;
	}
}

void lara_as_stepleft(ITEM_INFO* item, COLL_INFO* coll)
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

void lara_col_stepleft(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 22*/
	/*state code: lara_as_stepleft*/
	lara_col_stepright(item, coll);
}

void lara_col_roll2(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 23*/
	/*state code: lara_void_func*/
	Camera.laraNode = 0;
	Lara.moveAngle = item->pos.yRot + ANGLE(180);

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesAreWalls = true;

	item->gravityStatus = false;
	item->fallspeed = 0;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);

	if (LaraHitCeiling(item, coll))
		return;
	if (TestLaraSlide(item, coll))
		return;

	if (coll->middle.Floor > 200)
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

	if (coll->middle.Floor != NO_HEIGHT)
		item->pos.yPos += coll->middle.Floor;
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
	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;
	coll->facing = Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);
	LaraDeflectEdgeJump(item, coll);

	if (item->fallspeed > 0 && coll->middle.Floor <= 0)
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			item->goalAnimState = LS_STOP;

		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->middle.Floor;
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

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;
	coll->facing = item->speed < 0 ? Lara.moveAngle + ANGLE(180.0f) : Lara.moveAngle;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 870);

	if (TrInput & IN_ACTION && Lara.gunStatus == LG_NO_ARMS && !coll->hitStatic)
	{
		if (Lara.canMonkeySwing && coll->collType == CT_TOP)
		{
			item->goalAnimState = LS_MONKEYSWING_IDLE;
			item->currentAnimState = LS_MONKEYSWING_IDLE;
			item->animNumber = LA_JUMP_UP_TO_MONKEYSWING;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->gravityStatus = false;
			item->speed = 0;
			item->fallspeed = 0;

			Lara.gunStatus = LG_HANDS_BUSY;

			MonkeySwingSnap(item, coll);

			return;
		}

		if (coll->collType == CT_FRONT && coll->middle.Ceiling <= -STEPUP_HEIGHT)
		{
			int edge;
			int edgeCatch = LaraTestEdgeCatch(item, coll, &edge);

			if (edgeCatch)
			{
				if (edgeCatch >= 0 || LaraTestHangOnClimbWall(item, coll))
				{
					short angle = item->pos.yRot;
					bool result;

					/*if (coll->middle.SplitFloor && coll->front.SplitFloor == coll->middle.SplitFloor)
					{
						result = SnapToDiagonal(angle, 35);
					}
					else*/
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
								item->animNumber = LA_REACH_TO_HANG;
								item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 12;
								item->currentAnimState = LS_HANG;
								item->goalAnimState = LS_HANG;
							}
						}

						bounds = GetBoundsAccurate(item);

						if (edgeCatch <= 0)
							item->pos.yPos = edge - bounds->Y1 + 4;
						else
							item->pos.yPos += coll->front.Floor - bounds->Y1;

						/*if (coll->middle.SplitFloor)
						{
							Vector2 v = GetDiagonalIntersect(item->pos.xPos, item->pos.zPos, coll->middle.SplitFloor, LARA_RAD, item->pos.yRot);
							item->pos.xPos = v.x;
							item->pos.zPos = v.y;
						}
						else*/
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
		coll->collType == CT_TOP_FRONT)
		item->fallspeed = 1;

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

	if (item->fallspeed > 0 && coll->middle.Floor <= 0)
	{
		item->goalAnimState = LaraLandedBad(item, coll) ? LS_DEATH : LS_STOP;

		item->gravityStatus = false;
		item->fallspeed = 0;

		if (coll->middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->middle.Floor;
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

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);
	LaraDeflectEdgeJump(item, coll);

	if (item->fallspeed > 0 && coll->middle.Floor <= 0)
	{
		if (LaraLandedBad(item, coll))
			item->goalAnimState = LS_DEATH;
		else
			item->goalAnimState = LS_STOP;

		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->middle.Floor;
	}
}

void lara_col_roll(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 45*/
	/*state code: lara_void_func*/
	Lara.moveAngle = item->pos.yRot;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	coll->slopesArePits = false;
	coll->slopesAreWalls = true;

	item->gravityStatus = false;
	item->fallspeed = 0;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);

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

	if (coll->middle.Floor != NO_HEIGHT)
		item->pos.yPos += coll->middle.Floor;
}

void lara_as_swandive(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 52*/
	/*collision: lara_col_swandive*/
	coll->enableBaddiePush = true;
	coll->enableSpaz = false;
	if (item->fallspeed > LARA_FREEFALL_SPEED && item->goalAnimState != LS_DIVE)
		item->goalAnimState = LS_SWANDIVE_END;
}

void lara_col_swandive(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 52*/
	/*state code: lara_as_swandive*/
	Lara.moveAngle = item->pos.yRot;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);
	LaraDeflectEdgeJump(item, coll);

	if (coll->middle.Floor <= 0 && item->fallspeed > 0)
	{
		item->goalAnimState = LS_STOP;
		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->middle.Floor;
	}
}

void lara_as_fastdive(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 53*/
	/*collision: lara_col_fastdive*/
	if (TrInput & IN_ROLL && item->goalAnimState == LS_SWANDIVE_END)
		item->goalAnimState = LS_JUMP_ROLL_180;
	coll->enableBaddiePush = true;
	coll->enableSpaz = false;
	item->speed = (item->speed * 95) / 100;
}

void lara_col_fastdive(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 53*/
	/*state code: lara_as_fastdive*/
	Lara.moveAngle = item->pos.yRot;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);
	LaraDeflectEdgeJump(item, coll);

	if (coll->middle.Floor <= 0 && item->fallspeed > 0)
	{
		if (item->fallspeed <= 133)
			item->goalAnimState = LS_STOP;
		else
			item->goalAnimState = LS_DEATH;

		item->fallspeed = 0;
		item->gravityStatus = 0;

		if (coll->middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->middle.Floor;
	}
}

void lara_as_gymnast(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 54*/
	/*collision: lara_default_col*/
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
}

void lara_as_wade(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 65*/
	/*collision: lara_col_wade*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	Camera.targetElevation = -ANGLE(22.0f);

	if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP)
	{
		if (TrInput & IN_LEFT)
		{
			Lara.turnRate -= LARA_TURN_RATE;
			if (Lara.turnRate < -(LARA_FAST_TURN >> 1))
				Lara.turnRate = -(LARA_FAST_TURN >> 1);

			if (TestLaraLean(item, coll))
			{
				item->pos.zRot -= LARA_LEAN_RATE;
				if (item->pos.zRot < -(LARA_LEAN_MAX >> 1))
					item->pos.zRot = -(LARA_LEAN_MAX >> 1);
			}
			else
			{
				item->pos.zRot -= LARA_LEAN_RATE;
				if (item->pos.zRot < -LARA_LEAN_MAX * 3 / 5)
					item->pos.zRot = -LARA_LEAN_MAX * 3 / 5;
			}
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > (LARA_FAST_TURN / 2))
				Lara.turnRate = (LARA_FAST_TURN / 2);

			if (TestLaraLean(item, coll))
			{
				item->pos.zRot += LARA_LEAN_RATE;
				if (item->pos.zRot > (LARA_LEAN_MAX >> 1))
					item->pos.zRot = (LARA_LEAN_MAX >> 1);
			}
			else
			{
				item->pos.zRot += LARA_LEAN_RATE;
				if (item->pos.zRot > LARA_LEAN_MAX * 3 / 5)
					item->pos.zRot = LARA_LEAN_MAX * 3 / 5;
			}
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

			if (TestLaraLean(item, coll))
			{
				item->pos.zRot -= LARA_LEAN_RATE;
				if (item->pos.zRot < -LARA_LEAN_MAX)
					item->pos.zRot = -LARA_LEAN_MAX;
			}
			else
			{
				item->pos.zRot -= LARA_LEAN_RATE;
				if (item->pos.zRot < -LARA_LEAN_MAX * 3 / 5)
					item->pos.zRot = -LARA_LEAN_MAX * 3 / 5;
			}
		}
		else if (TrInput & IN_RIGHT)
		{
			Lara.turnRate += LARA_TURN_RATE;
			if (Lara.turnRate > LARA_FAST_TURN)
				Lara.turnRate = LARA_FAST_TURN;

			if (TestLaraLean(item, coll))
			{
				item->pos.zRot += LARA_LEAN_RATE;
				if (item->pos.zRot > LARA_LEAN_MAX)
					item->pos.zRot = LARA_LEAN_MAX;
			}
			else
			{
				item->pos.zRot += LARA_LEAN_RATE;
				if (item->pos.zRot > LARA_LEAN_MAX * 3 / 5)
					item->pos.zRot = LARA_LEAN_MAX * 3 / 5;
			}
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

void lara_col_wade(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 65*/
	/*state code: lara_as_wade*/
	Lara.moveAngle = item->pos.yRot;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->slopesAreWalls = true;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);

	if (LaraHitCeiling(item, coll))
		return;

	if (TestLaraVault(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		item->pos.zRot = 0;


		if ((coll->front.Type == WALL || coll->front.Type == SPLIT_TRI) && coll->front.Floor < -((STEP_SIZE * 5) / 2) && !(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP))
		{
			item->goalAnimState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->animNumber]))
				return;
		}

		LaraCollideStop(item, coll);
	}


	if (coll->middle.Floor >= -STEPUP_HEIGHT && coll->middle.Floor < -STEP_SIZE / 2 && !(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP))
	{
		item->goalAnimState = LS_STEP_UP;
		GetChange(item, &g_Level.Anims[item->animNumber]);
	}

	if (coll->middle.Floor >= 50 && !(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP))
	{
		item->pos.yPos += 50;
		return;
	}

	if (!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP) || coll->middle.Floor < 0)
		item->pos.yPos += coll->middle.Floor;   // Enforce to floor height.. if not a swamp room.
	else if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && coll->middle.Floor)
		item->pos.yPos += SWAMP_GRAVITY;
}

void lara_as_dash(ITEM_INFO* item, COLL_INFO* coll)
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
		if (Lara.turnRate < -ANGLE(4.0f))
			Lara.turnRate = -ANGLE(4.0f);

		item->pos.zRot -= ANGLE(1.5f);
		if (item->pos.zRot < -ANGLE(16.0f))
			item->pos.zRot = -ANGLE(16.0f);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > ANGLE(4.0f))
			Lara.turnRate = ANGLE(4.0f);

		item->pos.zRot += ANGLE(1.5f);
		if (item->pos.zRot > ANGLE(16.0f))
			item->pos.zRot = ANGLE(16.0f);
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

void lara_col_dash(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 73*/
	/*state code: lara_as_dash*/
	Lara.moveAngle = item->pos.yRot;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;

	coll->slopesAreWalls = true;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);

	if (!LaraHitCeiling(item, coll) && !TestLaraVault(item, coll))
	{
		if (LaraDeflectEdge(item, coll))
		{
			item->pos.zRot = 0;

			if (coll->hitTallBounds || TestWall(item, 256, 0, -640))
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
			if (coll->middle.Floor >= -STEPUP_HEIGHT && coll->middle.Floor < -STEP_SIZE / 2)
			{
				item->goalAnimState = LS_STEP_UP;
				GetChange(item, &g_Level.Anims[item->animNumber]);
			}

			if (!TestLaraSlide(item, coll))
			{
				if (coll->middle.Floor < 50)
				{
					if (coll->middle.Floor != NO_HEIGHT)
						item->pos.yPos += coll->middle.Floor;
				}
				else
				{
					item->goalAnimState = LS_STEP_DOWN; // for theoretical sprint stepdown anims, not in default anims
					if (GetChange(item, &g_Level.Anims[item->animNumber]))
						item->pos.yPos += coll->middle.Floor; // move Lara to middle.Floor
					else
						item->pos.yPos += 50; // do the default aligment
				}
			}
		}
	}
}

void lara_as_dashdive(ITEM_INFO* item, COLL_INFO* coll)
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

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = BAD_JUMP_CEILING;

	coll->slopesAreWalls = true;

	coll->facing = Lara.moveAngle;
	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, LARA_HEIGHT);
	LaraDeflectEdgeJump(item, coll);

	if (!LaraFallen(item, coll))
	{
		if (item->speed < 0)
			Lara.moveAngle = item->pos.yRot;

		if (coll->middle.Floor <= 0 && item->fallspeed > 0)
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
			item->pos.yPos += coll->middle.Floor;
			item->speed = 0;

			AnimateLara(item);
		}

		ShiftItem(item, coll);

		if (coll->middle.Floor != NO_HEIGHT)
			item->pos.yPos += coll->middle.Floor;
	}
}
