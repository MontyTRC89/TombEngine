#include "framework.h"
#include "lara.h"
#include "input.h"
#include "lara_tests.h"

/*normal hanging and shimmying*/
void lara_as_hang(ITEM_INFO* item, COLL_INFO* coll)//19A28, 19B5C (F)
{
	/*state 10*/
	/*collision: lara_col_hang*/
	Lara.isClimbing = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
}

void lara_col_hang(ITEM_INFO* item, COLL_INFO* coll)//19AC8, 19BFC (F)
{
	/*state 10*/
	/*state code: lara_as_hang*/
	item->fallspeed = 0;
	item->gravityStatus = false;

	if (item->animNumber == LA_REACH_TO_HANG)
	{
		int flag;

		if (TrInput & IN_LEFT || TrInput & IN_LSTEP)
		{
			if (CanLaraHangSideways(item, coll, -ANGLE(90.0f)))
			{
				item->goalAnimState = LS_SHIMMY_LEFT;

				return;
			}

			flag = LaraHangLeftCornerTest(item, coll);
			if (flag != 0)
			{
				if (flag <= 0)
					item->goalAnimState = LS_SHIMMY_INNER_LEFT;
				else
					item->goalAnimState = LS_SHIMMY_OUTER_LEFT;

				return;
			}
		}

		if (TrInput & IN_RIGHT || TrInput & IN_RSTEP)
		{
			if (CanLaraHangSideways(item, coll, ANGLE(90.0f)))
			{
				item->goalAnimState = LS_SHIMMY_RIGHT;

				return;
			}

			flag = LaraHangRightCornerTest(item, coll);
			if (flag != 0)
			{
				if (flag <= 0)
					item->goalAnimState = LS_SHIMMY_INNER_RIGHT;
				else
					item->goalAnimState = LS_SHIMMY_OUTER_RIGHT;

				return;
			}
		}
	}

	Lara.moveAngle = 0;

	LaraHangTest(item, coll);

	if (item->animNumber == LA_REACH_TO_HANG)
	{
		TestForObjectOnLedge(item, coll);

		if (TrInput & IN_FORWARD)
		{
			if (coll->frontFloor > -850)
			{
				if (coll->frontFloor < -650 &&
					coll->frontFloor >= coll->frontCeiling &&
					coll->frontFloor >= coll->leftCeiling2 &&
					coll->frontFloor >= coll->rightCeiling2)
				{
					if (abs(coll->leftFloor2 - coll->rightFloor2) < 60 && !coll->hitStatic)
					{
						if (TrInput & IN_WALK)
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

						return;
					}
				}

				if (coll->frontFloor < -650 &&
					coll->frontFloor - coll->frontCeiling >= -256 &&
					coll->frontFloor - coll->leftCeiling2 >= -256 &&
					coll->frontFloor - coll->rightCeiling2 >= -256)
				{
					if (abs(coll->leftFloor2 - coll->rightFloor2) < 60 && !coll->hitStatic)
					{
						item->goalAnimState = LS_HANG_TO_CRAWL;
						item->requiredAnimState = LS_CROUCH_IDLE;

						return;
					}
				}
			}

			if (Lara.climbStatus != 0 &&
				coll->midCeiling <= -256 &&
				abs(coll->leftCeiling2 - coll->rightCeiling2) < 60)
			{
				if (LaraTestClimbStance(item, coll))
				{
					item->goalAnimState = LS_LADDER_IDLE;
				}
				else
				{
					item->animNumber = LA_LADDER_SHIMMY_UP;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					item->goalAnimState = LS_HANG;
					item->currentAnimState = LS_HANG;
				}
			}

			return;
		}

		if (TrInput & IN_BACK &&
			Lara.climbStatus &&
			coll->midFloor > 344 &&
			item->animNumber == LA_REACH_TO_HANG)
		{
			if (LaraTestClimbStance(item, coll))
			{
				item->goalAnimState = LS_LADDER_IDLE;
			}
			else
			{
				item->animNumber = LA_LADDER_SHIMMY_DOWN;
				item->goalAnimState = LS_HANG;
				item->currentAnimState = LS_HANG;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			}
		}
	}
}

void lara_as_hangleft(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	/*state 30*/
	/*collision: lara_col_hangleft*/
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
	if (!(TrInput & (IN_LEFT | IN_LSTEP)))
		item->goalAnimState = LS_HANG;
}

void lara_col_hangleft(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	/*state 30*/
	/*state code: lara_as_hangleft*/
	Lara.moveAngle = -ANGLE(90);
	coll->radius = 102; /* @ORIGINAL_BUG: this value (instead of LARA_RAD) can make Lara glitch if coll->frontType is DIAGONAL or SPLIT_TRI */
	LaraHangTest(item, coll);
	Lara.moveAngle = -ANGLE(90);
}

void lara_as_hangright(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	/*state 31*/
	/*collision: lara_col_hangright*/
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
	if (!(TrInput & (IN_RIGHT | IN_RSTEP)))
		item->goalAnimState = LS_HANG;
}

void lara_col_hangright(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	/*state 31*/
	/*state code: lara_as_hangright*/
	Lara.moveAngle = ANGLE(90);
	coll->radius = 102; /* @ORIGINAL_BUG: this value (instead of LARA_RAD) can make Lara glitch if coll->frontType is DIAGONAL or SPLIT_TRI */
	LaraHangTest(item, coll);
	Lara.moveAngle = ANGLE(90);
}

/*go around corners*/

void lara_as_extcornerl(ITEM_INFO* item, COLL_INFO* coll)//1A1F0(<), 1A324(<) (F)
{
	/*state 107*/
	/*collision: lara_default_col*/
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	SetCornerAnim(item, coll, ANGLE(90.0f),
		item->animNumber == LA_SHIMMY_LEFT_CORNER_OUTER_END ||
		item->animNumber == LA_LADDER_LEFT_CORNER_OUTER_END);
}

void lara_as_extcornerr(ITEM_INFO* item, COLL_INFO* coll)//1A244(<), 1A378(<) (F)
{
	/*state 108*/
	/*collision: lara_default_col*/
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	SetCornerAnim(item, coll, -ANGLE(90.0f),
		item->animNumber == LA_SHIMMY_RIGHT_CORNER_OUTER_END ||
		item->animNumber == LA_LADDER_RIGHT_CORNER_OUTER_END);
}

void lara_as_intcornerl(ITEM_INFO* item, COLL_INFO* coll)//1A298(<), 1A3CC(<) (F)
{
	/*state 109*/
	/*collision: lara_default_col*/
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	SetCornerAnim(item, coll, -ANGLE(90.0f),
		item->animNumber == LA_SHIMMY_LEFT_CORNER_INNER_END ||
		item->animNumber == LA_LADDER_LEFT_CORNER_INNER_END);
}

void lara_as_intcornerr(ITEM_INFO* item, COLL_INFO* coll)//1A2EC(<), 1A420(<) (F)
{
	/*state 110*/
	/*collision: lara_default_col*/
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	SetCornerAnim(item, coll, ANGLE(90.0f),
		item->animNumber == LA_SHIMMY_RIGHT_CORNER_INNER_END ||
		item->animNumber == LA_LADDER_RIGHT_CORNER_INNER_END);
}

/*feet hanging and shimmying
////obviously, not all animations were made yet, we still need: 
-crouch pull up(works well, tested with placeholder anim)
-corner anims(works well, tested with placeholder anims)
-handstand(not tested)*/

void lara_as_hang_feet(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 139*/
	/*collision: lara_col_hang_feet*/
	Lara.isClimbing = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
}

void lara_col_hang_feet(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 139*/
	/*state code: lara_as_hang_feet*/
	item->fallspeed = 0;
	item->gravityStatus = false;

	Lara.moveAngle = 0;

	LaraHangTest(item, coll);

	if (!(TrInput & IN_ACTION))
		item->goalAnimState = LS_JUMP_UP;

	if (item->animNumber == LA_HANG_FEET_IDLE)
	{
		int flag;

		if (TrInput & IN_LEFT || TrInput & IN_LSTEP)
		{
			if (CanLaraHangSideways(item, coll, -ANGLE(90.0f)))
			{
				item->goalAnimState = LS_SHIMMY_FEET_LEFT;
				return;
			}
			flag = LaraHangLeftCornerTest(item, coll);
			if (flag != 0)
			{
				if (flag <= 0)
					item->goalAnimState = LS_SHIMMY_FEET_INNER_LEFT;
				else
					item->goalAnimState = LS_SHIMMY_FEET_OUTER_LEFT;
				return;
			}

		}


		if (TrInput & IN_RIGHT || TrInput & IN_RSTEP)
		{
			if (CanLaraHangSideways(item, coll, ANGLE(90.0f)))
			{
				item->goalAnimState = LS_SHIMMY_FEET_RIGHT;

				return;
			}
			flag = LaraHangRightCornerTest(item, coll);
			if (flag != 0)
			{
				if (flag <= 0)
					item->goalAnimState = LS_SHIMMY_FEET_INNER_RIGHT;
				else
					item->goalAnimState = LS_SHIMMY_FEET_OUTER_RIGHT;
				return;
			}
		}


		TestForObjectOnLedge(item, coll);
		if (TrInput & IN_FORWARD)
		{
			if (coll->frontFloor > -850)
			{
				if (coll->frontFloor < -650 &&
					coll->frontFloor >= coll->frontCeiling &&
					coll->frontFloor >= coll->leftCeiling2 &&
					coll->frontFloor >= coll->rightCeiling2)
				{
					if (abs(coll->leftFloor2 - coll->rightFloor2) < 60 && !coll->hitStatic)
					{
						if (TrInput & IN_WALK)
						{
							//till anim							item->goalAnimState = LS_HANDSTAND;
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
						return;
					}
				}
			}
			if (coll->frontFloor < -650 &&
				coll->frontFloor - coll->frontCeiling >= -256 &&
				coll->frontFloor - coll->leftCeiling2 >= -256 &&
				coll->frontFloor - coll->rightCeiling2 >= -256)
			{
				if (abs(coll->leftFloor2 - coll->rightFloor2) < 60 && !coll->hitStatic)
				{
					item->goalAnimState = LS_HANG_TO_CRAWL;
					item->requiredAnimState = LS_CROUCH_IDLE;
					return;
				}
			}
		}
		/*
			if (Lara.climbStatus != 0 &&
				coll->midCeiling <= -256 &&
				abs(coll->leftCeiling2 - coll->rightCeiling2) < 60)
			{
				if (LaraTestClimbStance(item, coll))
				{
					item->goalAnimState = LS_LADDER_IDLE;
				}
				else
				{
					item->animNumber = LA_LADDER_UP_HANDS;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					item->goalAnimState = LS_HANG;
					item->currentAnimState = LS_HANG;
				}
			}
			return;
					}*///commenting till daniel makes anims

		Lara.moveAngle = 0;
		LaraHangTest(item, coll);
	}
}

void lara_as_hang_feet_shimmyr(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 140*/
	/*collision: lara_col_hang_feet_shimmyr*/
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);

	if (!(TrInput & (IN_RIGHT | IN_RSTEP)))
		item->goalAnimState = LS_HANG_FEET;
}

void lara_col_hang_feet_shimmyr(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 140*/
	/*state code: lara_as_hang_feet_shimmyr*/
	Lara.moveAngle = ANGLE(90);
	coll->radius = LARA_RAD;
	LaraHangTest(item, coll);
	Lara.moveAngle = ANGLE(90);
}

void lara_as_hang_feet_shimmyl(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 141*/
	/*collision: lara_col_hang_feet_shimmyl*/
	coll->enableBaddiePush = false;
	coll->enableSpaz = false;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
	if (!(TrInput & (IN_LEFT | IN_LSTEP)))
		item->goalAnimState = LS_HANG_FEET;
}

void lara_col_hang_feet_shimmyl(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 141*/
	/*state code: lara_as_hang_feet_shimmyl*/
	Lara.moveAngle = -ANGLE(90);
	coll->radius = LARA_RAD;
	LaraHangTest(item, coll);
	Lara.moveAngle = -ANGLE(90);
}

/*go around corners feet*/

void lara_as_hang_feet_inRcorner(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 142*/
	/*collision: lara_default_col*/
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	if (item->frameNumber == g_Level.Anims[LA_SHIMMY_FEET_RIGHT_CORNER_INNER].frameEnd) // I don't like this either but it's better than adding 4 new 1 frame anims?
		SetCornerAnimFeet(item, coll, ANGLE(90.0f),
			item->animNumber = LA_SHIMMY_FEET_RIGHT_CORNER_INNER);
}

void lara_as_hang_feet_inLcorner(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 143*/
	/*collision: lara_default_col*/
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	if (item->frameNumber == g_Level.Anims[LA_SHIMMY_FEET_LEFT_CORNER_INNER].frameEnd)
		SetCornerAnimFeet(item, coll, -ANGLE(90.0f),
			item->animNumber = LA_SHIMMY_FEET_LEFT_CORNER_INNER);
}

void lara_as_hang_feet_outRcorner(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 144*/
	/*collision: lara_default_col*/
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	if (item->frameNumber == g_Level.Anims[LA_SHIMMY_FEET_RIGHT_CORNER_OUTER].frameEnd)
		SetCornerAnimFeet(item, coll, -ANGLE(90.0f),
			item->animNumber = LA_SHIMMY_FEET_RIGHT_CORNER_OUTER);
}

void lara_as_hang_feet_outLcorner(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 145*/
	/*collision: lara_default_col*/
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	if (item->frameNumber == g_Level.Anims[LA_SHIMMY_FEET_LEFT_CORNER_OUTER].frameEnd)
		SetCornerAnimFeet(item, coll, ANGLE(90.0f),
			item->animNumber = LA_SHIMMY_FEET_LEFT_CORNER_OUTER);
}
