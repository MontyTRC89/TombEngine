#include "framework.h"
#include "lara.h"
#include "input.h"
#include "control/control.h"
#include "lara_collide.h"
#include "lara_tests.h"
#include "lara_monkey.h"
#include "floordata.h"
#include "collide.h"
#include "items.h"
#include "camera.h"
#include "level.h"
#include "Scripting/GameFlowScript.h"

using namespace TEN::Floordata;

/*this file has all the related functions to monkeyswinging*/

/*monkeyswing state handling functions*/
void lara_as_monkey_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 75*/
	/*collision: lara_col_hang2*/
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

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

void lara_col_monkey_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 75*/
	/*state code: lara_as_hang2*/
	item->fallspeed = 0;
	item->gravityStatus = false;

	if (Lara.canMonkeySwing)
	{
		coll->Setup.BadHeightDown = NO_BAD_POS;
		coll->Setup.BadHeightUp = NO_HEIGHT;
		coll->Setup.BadCeilingHeight = 0;

		coll->Setup.SlopesAreWalls = false;
		coll->Setup.ForwardAngle = Lara.moveAngle;
		coll->Setup.Radius = LARA_RAD;
		coll->Setup.Height = LARA_HEIGHT_MONKEY;

		Lara.moveAngle = item->pos.yRot;

		GetCollisionInfo(coll, item);

		if (TrInput & IN_FORWARD && coll->CollisionType != CT_FRONT && abs(coll->Middle.Ceiling - coll->Front.Ceiling) < 50)
		{
			bool monkeyFront = LaraCollisionFront(item, item->pos.yRot, coll->Setup.Radius).BottomBlock->Flags.Monkeyswing;
			if (monkeyFront)
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
		else if (TrInput & IN_ROLL && g_GameFlow->Animations.MonkeyRoll)
		{
			item->goalAnimState = LS_MONKEYSWING_TURN_180;
		}

		if (abs(coll->Middle.Ceiling - coll->Front.Ceiling) < 50)
			MonkeySwingSnap(item, coll);
	}
	else
	{
		TestLaraHang(item, coll);

		if (item->goalAnimState == LS_MONKEYSWING_IDLE)
		{
			TestForObjectOnLedge(item, coll);

			if (!(TrInput & IN_FORWARD) ||
				coll->Front.Floor <= -850 ||
				coll->Front.Floor >= -650 ||
				coll->Front.Floor < coll->Front.Ceiling ||
				coll->FrontLeft.Floor < coll->FrontLeft.Ceiling ||
				coll->FrontRight.Floor < coll->FrontRight.Ceiling ||
				coll->HitStatic)
			{
				if (!(TrInput & IN_FORWARD) ||
					coll->Front.Floor <= -850 ||
					coll->Front.Floor >= -650 ||
					coll->Front.Floor - coll->Front.Ceiling < -256 ||
					coll->FrontLeft.Floor - coll->FrontLeft.Ceiling < -256 ||
					coll->FrontRight.Floor - coll->FrontRight.Ceiling < -256 ||
					coll->HitStatic)
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

	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = false;

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
		coll->Setup.BadHeightDown = NO_BAD_POS;
		coll->Setup.BadHeightUp = NO_HEIGHT;
		coll->Setup.BadCeilingHeight = 0;

		Lara.moveAngle = item->pos.yRot;

		coll->Setup.EnableSpaz = false;
		coll->Setup.EnableObjectPush = false;

		coll->Setup.ForwardAngle = Lara.moveAngle;
		coll->Setup.Radius = LARA_RAD;
		coll->Setup.Height = LARA_HEIGHT_MONKEY;

		GetCollisionInfo(coll, item);

		bool monkeyFront = LaraCollisionFront(item, item->pos.yRot, coll->Setup.Radius).BottomBlock->Flags.Monkeyswing;

		if (!monkeyFront || coll->CollisionType == CT_FRONT	|| abs(coll->Middle.Ceiling - coll->Front.Ceiling) > 50)
		{
			SetAnimation(item, LA_MONKEYSWING_IDLE);
		}
		else
		{
			if (abs(coll->Middle.Ceiling - coll->FrontLeft.Ceiling) <= 50)
			{
				if (abs(coll->Middle.Ceiling - coll->FrontRight.Ceiling) > 50)
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

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

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
			SetAnimation(item, LA_MONKEYSWING_IDLE);
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

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;

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
			SetAnimation(item, LA_MONKEYSWING_IDLE);
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
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
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
		coll->Setup.BadHeightDown = NO_BAD_POS;
		coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
		coll->Setup.BadCeilingHeight = 0;

		Lara.moveAngle = item->pos.yRot;

		coll->Setup.ForwardAngle = item->pos.yRot;
		coll->Setup.Radius = LARA_RAD;
		coll->Setup.Height = LARA_HEIGHT_MONKEY;
		coll->Setup.SlopesAreWalls = true;

		GetCollisionInfo(coll, item);
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

	Lara.moveAngle = item->pos.yRot + ANGLE(90);

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = false;
	coll->Setup.ForwardAngle = Lara.moveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;

	GetCollisionInfo(coll, item);

	if (abs(coll->Middle.Ceiling - coll->Front.Ceiling) > 50)
		return 0;

	if (!LaraCollisionFront(item, Lara.moveAngle, coll->Setup.Radius).BottomBlock->Flags.Monkeyswing)
		return 0;

	if (coll->CollisionType == CT_NONE)
		return 1;

	oct = GetDirOctant(item->pos.yRot);
	if (oct)
	{
		if (oct != 1)
			return 1;
		if (coll->CollisionType != CT_FRONT && coll->CollisionType != CT_RIGHT && coll->CollisionType != CT_LEFT)
			return 1;
	}
	else if (coll->CollisionType != CT_FRONT)
	{
		return 1;
	}

	return 0;
}

short TestMonkeyLeft(ITEM_INFO* item, COLL_INFO* coll)
{
	short oct;

	Lara.moveAngle = item->pos.yRot - ANGLE(90);

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = NO_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = false;
	coll->Setup.ForwardAngle = Lara.moveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;

	GetCollisionInfo(coll, item);

	if (abs(coll->Middle.Ceiling - coll->Front.Ceiling) > 50)
		return 0;

	if (!LaraCollisionFront(item, Lara.moveAngle, coll->Setup.Radius).BottomBlock->Flags.Monkeyswing)
		return 0;

	if (coll->CollisionType == CT_NONE)
		return 1;

	oct = GetDirOctant(item->pos.yRot);
	if (oct)
	{
		if (oct != 1)
			return 1;
		if (coll->CollisionType != CT_RIGHT && coll->CollisionType != CT_LEFT)
			return 1;
	}
	else
	{
		if (coll->CollisionType != CT_FRONT && coll->CollisionType != CT_LEFT)
			return 1;
	}

	return 0;
}

void MonkeySwingSnap(ITEM_INFO* item, COLL_INFO* coll)
{
	ROOM_VECTOR location = GetRoom(item->location, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	int height = GetCeilingHeight(location, item->pos.xPos, item->pos.zPos).value_or(NO_HEIGHT);
	if (height != NO_HEIGHT)
		item->pos.yPos = height + 704;
}

void MonkeySwingFall(ITEM_INFO* item)
{
	if (item->currentAnimState != LS_MONKEYSWING_TURN_180)
	{
		SetAnimation(item, LA_JUMP_UP, 9);

		item->speed = 2;
		item->gravityStatus = true;
		item->fallspeed = 1;
		item->pos.yPos += 256;

		Lara.gunStatus = LG_NO_ARMS;
	}
}
