#include "framework.h"
#include "lara.h"
#include "input.h"
#include "lara_tests.h"

// HANGING

// ------------------------------
// HANGING AND SHIMMYING
// Control & Collision Functions
// ------------------------------

// State:		10
// Collision:	lara_col_hang()
void lara_as_hang(ITEM_INFO* item, COLL_INFO* coll)//19A28, 19B5C (F)
{
	Camera.targetAngle = 0;
	Camera.targetElevation = ANGLE(-45.0f);
	Lara.isClimbing = false;

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (TrInput & IN_LOOK)
	{
		LookUpDown();
	}
}

// State:		10
// State code:	lara_as_hang()
void lara_col_hang(ITEM_INFO* item, COLL_INFO* coll)//19AC8, 19BFC (F)
{
	item->fallspeed = 0;
	item->gravityStatus = false;

	if (item->animNumber == LA_REACH_TO_HANG)
	{
		int flag;

		if (TrInput & IN_LEFT || TrInput & IN_LSTEP)
		{
			if (CanLaraHangSideways(item, coll, ANGLE(-90.0f)))
			{
				item->goalAnimState = LS_SHIMMY_LEFT;
				return;
			}

			flag = LaraHangLeftCornerTest(item, coll);
			if (flag != 0)
			{
				if (flag <= 0)
				{
					item->goalAnimState = LS_SHIMMY_INNER_LEFT;
				}
				else
				{
					item->goalAnimState = LS_SHIMMY_OUTER_LEFT;
				}

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
				{
					item->goalAnimState = LS_SHIMMY_INNER_RIGHT;
				}
				else
				{
					item->goalAnimState = LS_SHIMMY_OUTER_RIGHT;
				}

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

// State:		30
// Collision:	lara_col_hang_left()
void lara_as_shimmy_left(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	Camera.targetAngle = 0;
	Camera.targetElevation = ANGLE(-45.0f);

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (TrInput & IN_LEFT || TrInput & IN_LSTEP)
	{
		item->goalAnimState = LS_SHIMMY_LEFT;
		return;
	}

	item->goalAnimState = LS_HANG;
}
// State:		30
// State code: lara_as_hang_left()
void lara_col_shimmy_left(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	Lara.moveAngle = ANGLE(-90.0f);
	coll->radius = LARA_RAD; /* @ORIGINAL_BUG: original value, 102, can make Lara glitch if coll->frontType is DIAGONAL or SPLIT_TRI */
	LaraHangTest(item, coll);
	Lara.moveAngle = ANGLE(-90.0f);
}

// State:		31
// Collision:	lara_col_shimmy_right()
void lara_as_shimmy_right(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	Camera.targetAngle = 0;
	Camera.targetElevation = ANGLE(-45.0f);

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (TrInput & IN_RIGHT || TrInput & IN_RSTEP)
	{
		item->goalAnimState = LS_SHIMMY_RIGHT;
		return;
	}

	item->goalAnimState = LS_HANG;
}

// State:		31
// State code:	lara_as_shimmy_right()
void lara_col_shimmy_right(ITEM_INFO* item, COLL_INFO* coll) // (F) (D)
{
	Lara.moveAngle = ANGLE(90.0f);
	coll->radius = LARA_RAD; /* @ORIGINAL_BUG: original value, 102, can make Lara glitch if coll->frontType is DIAGONAL or SPLIT_TRI */
	LaraHangTest(item, coll);
	Lara.moveAngle = ANGLE(90.0f);
}

// ------------------------------
// SHIMMYING AROUND CORNERS
// Control & Collision Functions
// ------------------------------

// State:		107
// Collision:	lara_default_col()
void lara_as_extcornerl(ITEM_INFO* item, COLL_INFO* coll)//1A1F0(<), 1A324(<) (F)
{
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);

	SetCornerAnim(item, coll, ANGLE(90.0f),
		item->animNumber == LA_SHIMMY_LEFT_CORNER_OUTER_END ||
		item->animNumber == LA_LADDER_LEFT_CORNER_OUTER_END);
}

// State:		108
// Collision:	lara_default_col()
void lara_as_extcornerr(ITEM_INFO* item, COLL_INFO* coll)//1A244(<), 1A378(<) (F)
{
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);

	SetCornerAnim(item, coll, -ANGLE(90.0f),
		item->animNumber == LA_SHIMMY_RIGHT_CORNER_OUTER_END ||
		item->animNumber == LA_LADDER_RIGHT_CORNER_OUTER_END);
}

// State:		109
// Collision:	lara_default_col()
void lara_as_intcornerl(ITEM_INFO* item, COLL_INFO* coll)//1A298(<), 1A3CC(<) (F)
{
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);

	SetCornerAnim(item, coll, -ANGLE(90.0f),
		item->animNumber == LA_SHIMMY_LEFT_CORNER_INNER_END ||
		item->animNumber == LA_LADDER_LEFT_CORNER_INNER_END);
}

// State:		110
// Collision:	lara_default_col()
void lara_as_intcornerr(ITEM_INFO* item, COLL_INFO* coll)//1A2EC(<), 1A420(<) (F)
{
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);

	SetCornerAnim(item, coll, ANGLE(90.0f),
		item->animNumber == LA_SHIMMY_RIGHT_CORNER_INNER_END ||
		item->animNumber == LA_LADDER_RIGHT_CORNER_INNER_END);
}

// ------------------------------
// HANGING AND SHIMMYING BY FEET
// Control & Collision Functions
// ------------------------------

/*obviously, not all animations were made yet, we still need: 
-crouch pull up(works well, tested with placeholder anim)
-corner anims(works well, tested with placeholder anims)
-handstand(not tested)*/

// State:		139
// Collision:	lara_col_hang_feet()
void lara_as_hang_feet(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_as_hang(item, coll);
}

// State:		139
// Collision:	lara_as_hang_feet()
void lara_col_hang_feet(ITEM_INFO* item, COLL_INFO* coll)
{
	item->fallspeed = 0;
	item->gravityStatus = false;

	Lara.moveAngle = 0;

	LaraHangTest(item, coll);

	if (!(TrInput & IN_ACTION))
	{
		item->goalAnimState = LS_JUMP_UP;
	}

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

// State:		140
// Collision:	lara_col_hang_feet_shimmy_right()
void lara_as_shimmy_feet_right(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.targetAngle = 0;
	Camera.targetElevation = ANGLE(-45.0f);

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (TrInput & IN_RIGHT || TrInput & IN_RSTEP)
	{
		item->goalAnimState = LS_SHIMMY_FEET_RIGHT;
		return;
	}

	item->goalAnimState = LS_HANG_FEET;
}

// State:		140
// State code:	lara_as_hang_feet_shimmy_right()
void lara_col_shimmy_feet_right(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_shimmy_right(item, coll);
}

// State:		141
// Collision:	lara_as_hang_feet_shimmy_left()
void lara_as_shimmy_feet_left(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.targetAngle = 0;
	Camera.targetElevation = ANGLE(-45.0f);

	coll->enableBaddiePush = false;
	coll->enableSpaz = false;

	if (TrInput & IN_LEFT || TrInput & IN_LSTEP)
	{
		item->goalAnimState = LS_SHIMMY_FEET_LEFT;
		return;
	}

	item->goalAnimState = LS_HANG_FEET;
}

// State:		141
// State code:	lara_as_hang_feet_shimmy_left()
void lara_col_shimmy_feet_left(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_shimmy_left(item, coll);
}

// ------------------------------
// SHIMMYING BY FEET AROUND CORNERS
// Control & Collision Functions
// ------------------------------

// State		142
// Collision:	lara_default_col()
void lara_as_hang_feet_right_corner_inner(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);

	if (item->frameNumber == g_Level.Anims[LA_SHIMMY_FEET_RIGHT_CORNER_INNER].frameEnd) // I don't like this either but it's better than adding 4 new 1 frame anims?
	{
		SetCornerAnimFeet(item, coll, ANGLE(90.0f),
			item->animNumber = LA_SHIMMY_FEET_RIGHT_CORNER_INNER);
	}
}

// State		143
// Collision:	lara_default_col()
void lara_as_hang_feet_left_corner_inner(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);

	if (item->frameNumber == g_Level.Anims[LA_SHIMMY_FEET_LEFT_CORNER_INNER].frameEnd)
	{
		SetCornerAnimFeet(item, coll, -ANGLE(90.0f),
			item->animNumber = LA_SHIMMY_FEET_LEFT_CORNER_INNER);
	}
}

// State		144
// Collision:	lara_default_col()
void lara_as_hang_feet_right_corner_outer(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);

	if (item->frameNumber == g_Level.Anims[LA_SHIMMY_FEET_RIGHT_CORNER_OUTER].frameEnd)
	{
		SetCornerAnimFeet(item, coll, -ANGLE(90.0f),
			item->animNumber = LA_SHIMMY_FEET_RIGHT_CORNER_OUTER);
	}
}

// State		145
// Collision:	lara_default_col()
void lara_as_hang_feet_left_corner_outer(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);

	if (item->frameNumber == g_Level.Anims[LA_SHIMMY_FEET_LEFT_CORNER_OUTER].frameEnd)
	{
		SetCornerAnimFeet(item, coll, ANGLE(90.0f),
			item->animNumber = LA_SHIMMY_FEET_LEFT_CORNER_OUTER);
	}
}
