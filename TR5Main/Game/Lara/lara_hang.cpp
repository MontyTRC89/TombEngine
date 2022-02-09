#include "framework.h"
#include "Game/Lara/lara_hang.h"

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_overhang.h"
#include "Game/Lara/lara_tests.h"
#include "Specific/input.h"
#include "Specific/level.h"

/*this file has all the lara_as/lara_col functions related to hanging*/

void SetCornerAnim(ITEM_INFO* item, COLL_INFO* coll, bool flip)
{
	if (item->hitPoints <= 0)
	{
		SetAnimation(item, LA_FALL_START);

		item->Airborne = true;
		item->Velocity = 2;
		item->pos.yPos += STEP_SIZE;
		item->VerticalVelocity = 1;

		Lara.gunStatus = LG_HANDS_FREE;

		item->pos.yRot += Lara.nextCornerPos.yRot / 2;
		return;
	}

	if (flip)
	{
		if (Lara.isClimbing)
		{
			SetAnimation(item, LA_LADDER_IDLE);
		}
		else
		{
			SetAnimation(item, LA_REACH_TO_HANG, 21);
		}

		coll->Setup.OldPosition.x = item->pos.xPos = Lara.nextCornerPos.xPos;
		coll->Setup.OldPosition.y = item->pos.yPos = Lara.nextCornerPos.yPos;
		coll->Setup.OldPosition.z = item->pos.zPos = Lara.nextCornerPos.zPos;
		item->pos.yRot = Lara.nextCornerPos.yRot;
	}
}

/*normal hanging and shimmying*/
void lara_as_hang(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 10*/
	/*collision: lara_col_hang*/
	Lara.isClimbing = false;

	if (item->hitPoints <= 0)
	{
		item->targetState = LS_IDLE;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	coll->Setup.Mode = COLL_PROBE_MODE::FREE_FLAT;

	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);

	SlopeHangExtra(item, coll);
}

void lara_col_hang(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 10*/
	/*state code: lara_as_hang*/
	item->VerticalVelocity = 0;
	item->Airborne = false;

	if (item->animNumber == LA_REACH_TO_HANG)
	{
		if (TrInput & IN_LEFT || TrInput & IN_LSTEP)
		{
			if (TestLaraHangSideways(item, coll, -ANGLE(90.0f)))
			{
				item->targetState = LS_SHIMMY_LEFT;
				return;
			}

			switch (TestLaraHangCorner(item, coll, -90.0f))
			{
			case CORNER_RESULT::INNER:
				item->targetState = LS_SHIMMY_INNER_LEFT;
				return;
			
			case CORNER_RESULT::OUTER:
				item->targetState = LS_SHIMMY_OUTER_LEFT;
				return;
			
			default:
				break;
			}

			switch (TestLaraHangCorner(item, coll, -45.0f))
			{
			case CORNER_RESULT::INNER:
				item->targetState = LS_SHIMMY_45_INNER_LEFT;
				return;

			case CORNER_RESULT::OUTER:
				item->targetState = LS_SHIMMY_45_OUTER_LEFT;
				return;

			default:
				break;
			}
		}

		if (TrInput & IN_RIGHT || TrInput & IN_RSTEP)
		{
			if (TestLaraHangSideways(item, coll, ANGLE(90.0f)))
			{
				item->targetState = LS_SHIMMY_RIGHT;
				return;
			}

			switch (TestLaraHangCorner(item, coll, 90.0f))
			{
			case CORNER_RESULT::INNER:
				item->targetState = LS_SHIMMY_INNER_RIGHT;
				return;

			case CORNER_RESULT::OUTER:
				item->targetState = LS_SHIMMY_OUTER_RIGHT;
				return;

			default:
				break;
			}

			switch (TestLaraHangCorner(item, coll, 45.0f))
			{
			case CORNER_RESULT::INNER:
				item->targetState = LS_SHIMMY_45_INNER_RIGHT;
				return;

			case CORNER_RESULT::OUTER:
				item->targetState = LS_SHIMMY_45_OUTER_RIGHT;
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
			if (coll->Front.Floor > -850 && TestValidLedge(item, coll) && !coll->HitStatic)
			{
				if (coll->Front.Floor < -650 &&
					coll->Front.Floor >= coll->Front.Ceiling &&
					coll->FrontLeft.Floor >= coll->FrontLeft.Ceiling &&
					coll->FrontRight.Floor >= coll->FrontRight.Ceiling)
				{
					if (TrInput & IN_WALK)
					{
						item->targetState = LS_HANDSTAND;
					}
					else if (TrInput & IN_CROUCH)
					{
						item->targetState = LS_HANG_TO_CRAWL;
						item->requiredState = LS_CROUCH_IDLE;
					}
					else
					{
						item->targetState = LS_GRABBING;
					}

					return;
				}

				if (coll->Front.Floor < -650 &&
					coll->Front.Floor - coll->Front.Ceiling >= -256 &&
					coll->FrontLeft.Floor - coll->FrontLeft.Ceiling >= -256 &&
					coll->FrontRight.Floor - coll->FrontRight.Ceiling >= -256)
				{
					item->targetState = LS_HANG_TO_CRAWL;
					item->requiredState = LS_CROUCH_IDLE;

					return;
			}
			}

			if (Lara.climbStatus &&
				coll->Middle.Ceiling <= -256 &&
				abs(coll->FrontLeft.Ceiling - coll->FrontRight.Ceiling) < SLOPE_DIFFERENCE)
			{
				if (TestLaraClimbStance(item, coll))
				{
					item->targetState = LS_LADDER_IDLE;
				}
				else if (TestLastFrame(item))
				{
					SetAnimation(item, LA_LADDER_SHIMMY_UP);
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
				item->targetState = LS_LADDER_IDLE;
			}
			else if (TestLastFrame(item))
			{
				SetAnimation(item, LA_LADDER_SHIMMY_DOWN);
			}
		}
	}
}

void lara_as_hangleft(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 30*/
	/*collision: lara_col_hangleft*/
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	coll->Setup.Mode = COLL_PROBE_MODE::FREE_FLAT;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
	if (!(TrInput & (IN_LEFT | IN_LSTEP)))
		item->targetState = LS_HANG;
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
	coll->Setup.EnableSpasm = false;
	coll->Setup.Mode = COLL_PROBE_MODE::FREE_FLAT;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
	if (!(TrInput & (IN_RIGHT | IN_RSTEP)))
		item->targetState = LS_HANG;
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

void lara_as_gymnast(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 54*/
	/*collision: lara_default_col*/
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
}

/*go around corners*/

void lara_as_corner(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 107*/
	/*collision: lara_default_col*/
	Camera.laraNode = LM_TORSO;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(33.0f);
	SetCornerAnim(item, coll, TestLastFrame(item));
}