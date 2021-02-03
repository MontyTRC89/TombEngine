#include "framework.h"
#include "lara.h"
#include "input.h"
#include "lara_collide.h"
#include "lara_tests.h"
#include "lara_monkey.h"

/*this file has all the related functions to monkeyswinging*/

/*monkeyswing state handling functions*/
void lara_as_hang2(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 75*/
	/*collision: lara_col_hang2*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;

	if (Lara.canMonkeySwing)
	{
		if (!(TrInput & IN_ACTION) || item->hitPoints <= 0)
			MonkeySwingFall(item);

		Camera.targetAngle = 0;
		Camera.targetElevation = -ANGLE(45.0f);
	}

	if (TrInput & IN_LOOK)
		LookUpDown();
}

void lara_col_hang2(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 75*/
	/*state code: lara_as_hang2*/
	item->fallspeed = 0;
	item->gravityStatus = false;

	if (Lara.canMonkeySwing)
	{
		coll->badPos = NO_BAD_POS;
		coll->badNeg = NO_HEIGHT;
		coll->badCeiling = 0;

		coll->slopesAreWalls = 0;
		coll->facing = Lara.moveAngle;
		coll->radius = 100;

		Lara.moveAngle = item->pos.yRot;

		GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 600);

		// FOR DEBUG PURPOSES UNTIL SCRIPTING IS READY-
//		EnableMonkeyRoll = true;


		if (TrInput & IN_FORWARD && coll->collType != CT_FRONT && abs(coll->midCeiling - coll->frontCeiling) < 50)
		{
			item->goalAnimState = LS_MONKEYSWING_FORWARD;
		}
		else if (TrInput & IN_LSTEP && TestMonkeyLeft(item, coll))
		{
			item->goalAnimState = LS_MONKEYSWING_LEFT;
		}
		else if (TrInput & IN_RSTEP && TestMonkeyRight(item, coll))
		{
			item->goalAnimState = LS_MONKEYSWING_RIGHT;
		}
		else if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_MONKEYSWING_TURN_LEFT;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_MONKEYSWING_TURN_RIGHT;
		}
		else if ((TrInput & IN_ROLL))// && EnableMonkeyRoll == true)
		{
			item->goalAnimState = LS_MONKEYSWING_TURN_180;
		}

		MonkeySwingSnap(item, coll);
	}
	else
	{
		LaraHangTest(item, coll);

		if (item->goalAnimState == LS_MONKEYSWING_IDLE)
		{
			TestForObjectOnLedge(item, coll);

			if (!(TrInput & IN_FORWARD) ||
				coll->frontFloor <= -850 ||
				coll->frontFloor >= -650 ||
				coll->frontFloor < coll->frontCeiling ||
				coll->leftFloor2 < coll->leftCeiling2 ||
				coll->rightFloor2 < coll->rightCeiling2 ||
				coll->hitStatic)
			{
				if (!(TrInput & IN_FORWARD) ||
					coll->frontFloor <= -850 ||
					coll->frontFloor >= -650 ||
					coll->frontFloor - coll->frontCeiling < -256 ||
					coll->leftFloor2 - coll->leftCeiling2 < -256 ||
					coll->rightFloor2 - coll->rightCeiling2 < -256 ||
					coll->hitStatic)
				{
					if (TrInput & IN_LEFT || TrInput & IN_LSTEP)
					{
						item->goalAnimState = LS_SHIMMY_LEFT;
					}
					else if (TrInput & IN_RIGHT || TrInput & IN_RSTEP)
					{
						item->goalAnimState = LS_SHIMMY_RIGHT;
					}
				}
				else
				{
					item->goalAnimState = LS_HANG_TO_CRAWL;
					item->requiredAnimState = LS_CROUCH_IDLE;
				}
			}
			else if (TrInput & IN_WALK)
			{
				item->goalAnimState = LS_HANDSTAND;
			}
			else if (TrInput & IN_DUCK)
			{
				item->goalAnimState = LS_HANG_TO_CRAWL;
				item->requiredAnimState = LS_CROUCH_IDLE;
			}
			else
			{
				item->goalAnimState = LS_GRABBING;
			}
		}
	}
}

void lara_as_monkeyswing(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 76*/
	/*collision: lara_col_monkeyswing*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_MONKEYSWING_IDLE;
		return;
	}

	coll->enableSpaz = false;
	coll->enableBaddiePush = false;

	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;

	if (TrInput & IN_LOOK)
		LookUpDown();

	if (TrInput & IN_FORWARD)
		item->goalAnimState = LS_MONKEYSWING_FORWARD;
	else
		item->goalAnimState = LS_MONKEYSWING_IDLE;

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

void lara_col_monkeyswing(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 76*/
	/*state code: lara_as_monkeyswing*/
	if (TrInput & IN_ACTION && Lara.canMonkeySwing)
	{
		coll->badPos = NO_BAD_POS;
		coll->badNeg = NO_HEIGHT;
		coll->badCeiling = 0;

		Lara.moveAngle = item->pos.yRot;

		coll->enableSpaz = false;
		coll->enableBaddiePush = false;

		coll->facing = Lara.moveAngle;
		coll->radius = 100;

		GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 600);

		if (coll->collType == CT_FRONT
			|| abs(coll->midCeiling - coll->frontCeiling) > 50)
		{
			item->animNumber = LA_MONKEYSWING_IDLE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = LS_MONKEYSWING_IDLE;
			item->goalAnimState = LS_MONKEYSWING_IDLE;
		}
		else
		{
			if (abs(coll->midCeiling - coll->leftCeiling2) <= 50)
			{
				if (abs(coll->midCeiling - coll->rightCeiling2) > 50)
				{
					ShiftItem(item, coll);
					item->pos.yRot -= ANGLE(5.0f);
				}
			}
			else
			{
				ShiftItem(item, coll);
				item->pos.yRot += ANGLE(5.0f);
			}

			Camera.targetElevation = ANGLE(10.0f);
			MonkeySwingSnap(item, coll);
		}
	}
	else
	{
		MonkeySwingFall(item);
	}
}

void lara_as_monkeyr(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 78*/
	/*collision: lara_col_monkeyr*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_MONKEYSWING_IDLE;
		return;
	}

	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (TrInput & IN_RSTEP)
	{
		item->goalAnimState = LS_MONKEYSWING_RIGHT;
		Camera.targetElevation = ANGLE(10.0f);
	}
	else
	{
		item->goalAnimState = LS_MONKEYSWING_IDLE;
		Camera.targetElevation = ANGLE(10.0f);
	}
}

void lara_col_monkeyr(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 78*/
	/*state code: lara_as_monkeyr*/
	if ((TrInput & IN_ACTION) && Lara.canMonkeySwing)
	{
		if (TestMonkeyRight(item, coll))
		{
			MonkeySwingSnap(item, coll);
		}
		else
		{
			item->animNumber = LA_MONKEYSWING_IDLE;
			item->currentAnimState = LS_MONKEYSWING_IDLE;
			item->goalAnimState = LS_MONKEYSWING_IDLE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
	else
	{
		MonkeySwingFall(item);
	}
}

void lara_as_monkeyl(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 77*/
	/*collision: lara_col_monkeyl*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_MONKEYSWING_IDLE;

		return;
	}

	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (TrInput & IN_LSTEP)
	{
		item->goalAnimState = LS_MONKEYSWING_LEFT;
		Camera.targetElevation = ANGLE(10.0f);
	}
	else
	{
		item->goalAnimState = LS_MONKEYSWING_IDLE;
		Camera.targetElevation = ANGLE(10.0f);
	}
}

void lara_col_monkeyl(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 77*/
	/*state code: lara_as_monkeyl*/
	if ((TrInput & IN_ACTION) && Lara.canMonkeySwing)
	{
		if (TestMonkeyLeft(item, coll))
		{
			MonkeySwingSnap(item, coll);
		}
		else
		{
			item->animNumber = LA_MONKEYSWING_IDLE;
			item->currentAnimState = LS_MONKEYSWING_IDLE;
			item->goalAnimState = LS_MONKEYSWING_IDLE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
	else
	{
		MonkeySwingFall(item);
	}
}

void lara_as_monkey180(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 79*/
	/*collision: lara_col_monkey180*/
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	item->goalAnimState = LS_MONKEYSWING_IDLE;
}

void lara_col_monkey180(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 79*/
	/*state code: lara_as_monkey180*/
	lara_col_monkeyswing(item, coll);
}

void lara_as_hangturnr(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 83*/
	/*collision: lara_col_hangturnlr*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_MONKEYSWING_IDLE;
		return;
	}

	Camera.targetElevation = 1820;
	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;
	item->pos.yRot += ANGLE(1.5f);

	if (!(TrInput & IN_RIGHT))
		item->goalAnimState = LS_MONKEYSWING_IDLE;
}
//both lara_as_hangturnl and lara_as_hangturnr states use lara_col_hangturnlr for collision//
void lara_as_hangturnl(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 82*/
	/*collision: lara_col_hangturnlr*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_MONKEYSWING_IDLE;
		return;
	}

	Camera.targetElevation = 1820;
	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;
	item->pos.yRot -= ANGLE(1.5f);

	if (!(TrInput & IN_LEFT))
		item->goalAnimState = LS_MONKEYSWING_IDLE;
}

void lara_col_hangturnlr(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 82 and 83*/
	/*state code: lara_as_hangturnr(83), lara_as_hangturnl(82)*/
	if ((TrInput & IN_ACTION) && Lara.canMonkeySwing)
	{
		coll->badPos = NO_BAD_POS;
		coll->badNeg = -STEPUP_HEIGHT;
		coll->badCeiling = 0;

		Lara.moveAngle = item->pos.yRot;

		coll->facing = item->pos.yRot;
		coll->radius = 100;
		coll->slopesAreWalls = true;

		GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 600);
		MonkeySwingSnap(item, coll);
	}
	else
	{
		MonkeySwingFall(item);
	}
}

/*tests and other functions*/

short TestMonkeyRight(ITEM_INFO* item, COLL_INFO* coll)
{
	short oct;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -STEPUP_HEIGHT;
	coll->badCeiling = 0;
	Lara.moveAngle = item->pos.yRot + ANGLE(90);
	coll->slopesAreWalls = 0;
	coll->facing = Lara.moveAngle;
	coll->radius = 100;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 600);

	if (abs(coll->midCeiling - coll->frontCeiling) > 50)
		return 0;

	if (!coll->collType)
		return 1;

	oct = GetDirOctant(item->pos.yRot);
	if (oct)
	{
		if (oct != 1)
			return 1;
		if (coll->collType != CT_FRONT && coll->collType != CT_RIGHT && coll->collType != CT_LEFT)
			return 1;
	}
	else if (coll->collType != CT_FRONT)
	{
		return 1;
	}

	return 0;
}

short TestMonkeyLeft(ITEM_INFO* item, COLL_INFO* coll)
{
	short oct;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = NO_HEIGHT;
	coll->badCeiling = 0;
	Lara.moveAngle = item->pos.yRot - ANGLE(90);
	coll->slopesAreWalls = 0;
	coll->facing = Lara.moveAngle;
	coll->radius = 100;

	GetCollisionInfo(coll, item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 600);

	if (abs(coll->midCeiling - coll->frontCeiling) > 50)
		return 0;

	if (!coll->collType)
		return 1;

	oct = GetDirOctant(item->pos.yRot);
	if (oct)
	{
		if (oct != 1)
			return 1;
		if (coll->collType != CT_RIGHT && coll->collType != CT_LEFT)
			return 1;
	}
	else
	{
		if (coll->collType != CT_FRONT && coll->collType != CT_LEFT)
			return 1;
	}

	return 0;
}

void MonkeySwingSnap(ITEM_INFO* item, COLL_INFO* coll)
{
	short roomNum = item->roomNumber;
	item->pos.yPos = GetCeiling(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNum),
		item->pos.xPos, item->pos.yPos, item->pos.zPos) + 704;
}

void MonkeySwingFall(ITEM_INFO* item)
{
	item->goalAnimState = LS_JUMP_UP;
	item->currentAnimState = LS_JUMP_UP;
	item->animNumber = LA_JUMP_UP;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 9;

	item->speed = 2;
	item->gravityStatus = true;
	item->fallspeed = 1;
	item->pos.yPos += 256;

	Lara.gunStatus = LG_NO_ARMS;
}
