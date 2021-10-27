#include "framework.h"
#include "lara.h"
#include "input.h"
#include "lara_tests.h"
#include "items.h"
#include "collide.h"
#include "camera.h"
#include "level.h"

/*this file has all the lara_as/lara_col functions related to hanging*/

/*normal hanging and shimmying*/
void lara_as_hang(ITEM_INFO* item, COLL_INFO* coll)
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

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	coll->Setup.Mode = COLL_PROBE_MODE::FREE_FLAT;

	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
}

void lara_col_hang(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 10*/
	/*state code: lara_as_hang*/
	item->fallspeed = 0;
	item->gravityStatus = false;

	if (item->animNumber == LA_REACH_TO_HANG)
	{
		if (TrInput & IN_LEFT || TrInput & IN_LSTEP)
		{
			if (TestLaraHangSideways(item, coll, -ANGLE(90.0f)))
			{
				item->goalAnimState = LS_SHIMMY_LEFT;
				return;
			}

			switch (TestLaraHangCorner(item, coll, -90.0f))
			{
			case CORNER_RESULT::INNER:
				item->goalAnimState = LS_SHIMMY_INNER_LEFT;
				return;
			
			case CORNER_RESULT::OUTER:
				item->goalAnimState = LS_SHIMMY_OUTER_LEFT;
				return;
			
			default:
				break;
			}
		}

		if (TrInput & IN_RIGHT || TrInput & IN_RSTEP)
		{
			if (TestLaraHangSideways(item, coll, ANGLE(90.0f)))
			{
				item->goalAnimState = LS_SHIMMY_RIGHT;
				return;
			}

			switch (TestLaraHangCorner(item, coll, 90.0f))
			{
			case CORNER_RESULT::INNER:
				item->goalAnimState = LS_SHIMMY_INNER_RIGHT;
				return;

			case CORNER_RESULT::OUTER:
				item->goalAnimState = LS_SHIMMY_OUTER_RIGHT;
				return;

			default:
				break;
			}
		}
	}

	Lara.moveAngle = item->pos.yRot;

	TestLaraHang(item, coll);

	if (item->animNumber == LA_REACH_TO_HANG)
	{
		TestForObjectOnLedge(item, coll);

		if (TrInput & IN_FORWARD)
		{
			if (coll->Front.Floor > -850)
			{
				if (coll->Front.Floor < -650 &&
					coll->Front.Floor >= coll->Front.Ceiling &&
					coll->Front.Floor >= coll->FrontLeft.Ceiling &&
					coll->Front.Floor >= coll->FrontRight.Ceiling)
				{
					if (TestValidLedge(item, coll) && !coll->HitStatic)
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

				if (coll->Front.Floor < -650 &&
					coll->Front.Floor - coll->Front.Ceiling >= -256 &&
					coll->Front.Floor - coll->FrontLeft.Ceiling >= -256 &&
					coll->Front.Floor - coll->FrontRight.Ceiling >= -256)
				{
					if (abs(coll->FrontLeft.Floor - coll->FrontRight.Floor) < 60 && !coll->HitStatic)
					{
						item->goalAnimState = LS_HANG_TO_CRAWL;
						item->requiredAnimState = LS_CROUCH_IDLE;

						return;
					}
				}
			}

			if (Lara.climbStatus != 0 &&
				coll->Middle.Ceiling <= -256 &&
				abs(coll->FrontLeft.Ceiling - coll->FrontRight.Ceiling) < 60)
			{
				if (TestLaraClimbStance(item, coll))
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
			coll->Middle.Floor > 344 &&
			item->animNumber == LA_REACH_TO_HANG)
		{
			if (TestLaraClimbStance(item, coll))
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

void lara_as_hangleft(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 30*/
	/*collision: lara_col_hangleft*/
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	coll->Setup.Mode = COLL_PROBE_MODE::FREE_FLAT;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
	if (!(TrInput & (IN_LEFT | IN_LSTEP)))
		item->goalAnimState = LS_HANG;
}

void lara_col_hangleft(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 30*/
	/*state code: lara_as_hangleft*/
	Lara.moveAngle = item->pos.yRot - ANGLE(90);
	coll->Setup.Radius = LARA_RAD;
	TestLaraHang(item, coll);
	Lara.moveAngle = item->pos.yRot - ANGLE(90);
}

void lara_as_hangright(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 31*/
	/*collision: lara_col_hangright*/
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	coll->Setup.Mode = COLL_PROBE_MODE::FREE_FLAT;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
	if (!(TrInput & (IN_RIGHT | IN_RSTEP)))
		item->goalAnimState = LS_HANG;
}

void lara_col_hangright(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 31*/
	/*state code: lara_as_hangright*/
	Lara.moveAngle = item->pos.yRot + ANGLE(90);
	coll->Setup.Radius = LARA_RAD;
	TestLaraHang(item, coll);
	Lara.moveAngle = item->pos.yRot + ANGLE(90);
}

/*go around corners*/

void lara_as_extcornerl(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 107*/
	/*collision: lara_default_col*/
	Camera.laraNode = LM_TORSO;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(33.0f);
	SetCornerAnim(item, coll, ANGLE(90.0f),
		item->animNumber == LA_SHIMMY_LEFT_CORNER_OUTER_END ||
		item->animNumber == LA_LADDER_LEFT_CORNER_OUTER_END);
}

void lara_as_extcornerr(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 108*/
	/*collision: lara_default_col*/
	Camera.laraNode = LM_TORSO;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(33.0f);
	SetCornerAnim(item, coll, -ANGLE(90.0f),
		item->animNumber == LA_SHIMMY_RIGHT_CORNER_OUTER_END ||
		item->animNumber == LA_LADDER_RIGHT_CORNER_OUTER_END);
}

void lara_as_intcornerl(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 109*/
	/*collision: lara_default_col*/
	Camera.targetAngle = 0;
	Camera.laraNode = LM_TORSO;
	Camera.targetElevation = -ANGLE(33.0f);
	SetCornerAnim(item, coll, -ANGLE(90.0f),
		item->animNumber == LA_SHIMMY_LEFT_CORNER_INNER_END ||
		item->animNumber == LA_LADDER_LEFT_CORNER_INNER_END);
}

void lara_as_intcornerr(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 110*/
	/*collision: lara_default_col*/
	Camera.laraNode = LM_TORSO;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(33.0f);
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
	//collision: lara_col_hang_feet
	Lara.isClimbing = false;

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_STOP;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
}

void lara_col_hang_feet(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 139*/
	//state code: lara_as_hang_feet
	item->fallspeed = 0;
	item->gravityStatus = false;

	Lara.moveAngle = item->pos.yRot;

	TestLaraHang(item, coll);

	if (!(TrInput & IN_ACTION))
		item->goalAnimState = LS_JUMP_UP;

	if (item->animNumber == LA_HANG_FEET_IDLE)
	{
		if (TrInput & IN_LEFT || TrInput & IN_LSTEP)
		{
			if (TestLaraHangSideways(item, coll, -ANGLE(90.0f)))
			{
				item->goalAnimState = LS_SHIMMY_FEET_LEFT;
				return;
			}

			switch (TestLaraHangCorner(item, coll, -90.0f))
			{
			case CORNER_RESULT::INNER:
				item->goalAnimState = LS_SHIMMY_FEET_INNER_LEFT;
				return;

			case CORNER_RESULT::OUTER:
				item->goalAnimState = LS_SHIMMY_FEET_OUTER_LEFT;
				return;

			default:
				break;
			}
		}

		if (TrInput & IN_RIGHT || TrInput & IN_RSTEP)
		{
			if (TestLaraHangSideways(item, coll, ANGLE(90.0f)))
			{
				item->goalAnimState = LS_SHIMMY_FEET_RIGHT;
				return;
			}

			switch (TestLaraHangCorner(item, coll, 90.0f))
			{
			case CORNER_RESULT::INNER:
				item->goalAnimState = LS_SHIMMY_FEET_INNER_RIGHT;
				return;

			case CORNER_RESULT::OUTER:
				item->goalAnimState = LS_SHIMMY_FEET_OUTER_RIGHT;
				return;

			default:
				break;
			}
		}


		TestForObjectOnLedge(item, coll);
		if (TrInput & IN_FORWARD)
		{
			if (coll->Front.Floor > -850)
			{
				if (coll->Front.Floor < -650 &&
					coll->Front.Floor >= coll->Front.Ceiling &&
					coll->Front.Floor >= coll->FrontLeft.Ceiling &&
					coll->Front.Floor >= coll->FrontRight.Ceiling)
				{
					if (abs(coll->FrontLeft.Floor - coll->FrontRight.Floor) < 60 && !coll->HitStatic)
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
			if (coll->Front.Floor < -650 &&
				coll->Front.Floor - coll->Front.Ceiling >= -256 &&
				coll->Front.Floor - coll->FrontLeft.Ceiling >= -256 &&
				coll->Front.Floor - coll->FrontRight.Ceiling >= -256)
			{
				if (abs(coll->FrontLeft.Floor - coll->FrontRight.Floor) < 60 && !coll->HitStatic)
				{
					item->goalAnimState = LS_HANG_TO_CRAWL;
					item->requiredAnimState = LS_CROUCH_IDLE;
					return;
				}
			}
		}

		if (Lara.climbStatus != 0 &&
			coll->Middle.Ceiling <= -256 &&
			abs(coll->FrontLeft.Ceiling - coll->FrontRight.Ceiling) < 60)
		{
			if (TestLaraClimbStance(item, coll))
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

		Lara.moveAngle = item->pos.yRot;
		TestLaraHang(item, coll);
}

void lara_as_hang_feet_shimmyr(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 140*/
	//collision: lara_col_hang_feet_shimmyr
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);

	if (!(TrInput & (IN_RIGHT | IN_RSTEP)))
		item->goalAnimState = LS_HANG_FEET;
}

void lara_col_hang_feet_shimmyr(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 140*/
	//state code: lara_as_hang_feet_shimmyr
	Lara.moveAngle = item->pos.yRot + ANGLE(90);
	coll->Setup.Radius = LARA_RAD;
	TestLaraHang(item, coll);
	Lara.moveAngle = item->pos.yRot + ANGLE(90);
}

void lara_as_hang_feet_shimmyl(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 141*/
	//collision: lara_col_hang_feet_shimmyl
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
	if (!(TrInput & (IN_LEFT | IN_LSTEP)))
		item->goalAnimState = LS_HANG_FEET;
}

void lara_col_hang_feet_shimmyl(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 141*/
	//state code: lara_as_hang_feet_shimmyl
	Lara.moveAngle = item->pos.yRot - ANGLE(90);
	coll->Setup.Radius = LARA_RAD;
	TestLaraHang(item, coll);
	Lara.moveAngle = item->pos.yRot - ANGLE(90);
}

//go around corners feet

void lara_as_hang_feet_inRcorner(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 142*/
	//collision: lara_default_col
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	if (item->frameNumber == g_Level.Anims[LA_SHIMMY_FEET_RIGHT_CORNER_INNER].frameEnd) // I don't like this either but it's better than adding 4 new 1 frame anims?
		SetCornerAnimFeet(item, coll, ANGLE(90.0f), 1);
}

void lara_as_hang_feet_inLcorner(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 143*/
	//collision: lara_default_col
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	if (item->frameNumber == g_Level.Anims[LA_SHIMMY_FEET_LEFT_CORNER_INNER].frameEnd)
		SetCornerAnimFeet(item, coll, -ANGLE(90.0f), 1);
}

void lara_as_hang_feet_outRcorner(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 144*/
	//collision: lara_default_col
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	if (item->frameNumber == g_Level.Anims[LA_SHIMMY_FEET_RIGHT_CORNER_OUTER].frameEnd)
		SetCornerAnimFeet(item, coll, -ANGLE(90.0f), 1);
}

void lara_as_hang_feet_outLcorner(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 145*/
	//collision: lara_default_col
	Camera.laraNode = 8;
	Camera.targetElevation = ANGLE(33.0f);
	if (item->frameNumber == g_Level.Anims[LA_SHIMMY_FEET_LEFT_CORNER_OUTER].frameEnd)
		SetCornerAnimFeet(item, coll, ANGLE(90.0f), 1);
}
