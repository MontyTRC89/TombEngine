#include "framework.h"
#include "Game/Lara/lara_hang.h"

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_overhang.h"
#include "Game/Lara/lara_tests.h"
#include "Specific/input.h"
#include "Specific/level.h"

/*this file has all the lara_as/lara_col functions related to hanging*/

void SetCornerAnim(ITEM_INFO* item, COLL_INFO* coll, bool flip)
{
	auto* info = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		SetAnimation(item, LA_FALL_START);

		item->Airborne = true;
		item->Velocity = 2;
		item->Position.yPos += STEP_SIZE;
		item->VerticalVelocity = 1;

		info->Control.HandStatus = HandStatus::Free;

		item->Position.yRot += info->NextCornerPos.yRot / 2;
		return;
	}

	if (flip)
	{
		if (info->Control.IsClimbingLadder)
		{
			SetAnimation(item, LA_LADDER_IDLE);
		}
		else
		{
			SetAnimation(item, LA_REACH_TO_HANG, 21);
		}

		coll->Setup.OldPosition.x = item->Position.xPos = info->NextCornerPos.xPos;
		coll->Setup.OldPosition.y = item->Position.yPos = info->NextCornerPos.yPos;
		coll->Setup.OldPosition.z = item->Position.zPos = info->NextCornerPos.zPos;
		item->Position.yRot = info->NextCornerPos.yRot;
	}
}

/*normal hanging and shimmying*/
void lara_as_hang(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	/*state 10*/
	/*collision: lara_col_hang*/
	info->Control.IsClimbingLadder = false;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_IDLE;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	coll->Setup.Mode = CollProbeMode::FreeFlat;

	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);

	SlopeHangExtra(item, coll);
}

void lara_col_hang(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	/*state 10*/
	/*state code: lara_as_hang*/
	item->VerticalVelocity = 0;
	item->Airborne = false;

	if (item->AnimNumber == LA_REACH_TO_HANG)
	{
		if (TrInput & IN_LEFT || TrInput & IN_LSTEP)
		{
			if (TestLaraHangSideways(item, coll, -ANGLE(90.0f)))
			{
				item->TargetState = LS_SHIMMY_LEFT;
				return;
			}

			switch (TestLaraHangCorner(item, coll, -90.0f))
			{
			case CornerResult::Inner:
				item->TargetState = LS_SHIMMY_INNER_LEFT;
				return;
			
			case CornerResult::Outer:
				item->TargetState = LS_SHIMMY_OUTER_LEFT;
				return;
			
			default:
				break;
			}

			switch (TestLaraHangCorner(item, coll, -45.0f))
			{
			case CornerResult::Inner:
				item->TargetState = LS_SHIMMY_45_INNER_LEFT;
				return;

			case CornerResult::Outer:
				item->TargetState = LS_SHIMMY_45_OUTER_LEFT;
				return;

			default:
				break;
			}
		}

		if (TrInput & IN_RIGHT || TrInput & IN_RSTEP)
		{
			if (TestLaraHangSideways(item, coll, ANGLE(90.0f)))
			{
				item->TargetState = LS_SHIMMY_RIGHT;
				return;
			}

			switch (TestLaraHangCorner(item, coll, 90.0f))
			{
			case CornerResult::Inner:
				item->TargetState = LS_SHIMMY_INNER_RIGHT;
				return;

			case CornerResult::Outer:
				item->TargetState = LS_SHIMMY_OUTER_RIGHT;
				return;

			default:
				break;
			}

			switch (TestLaraHangCorner(item, coll, 45.0f))
			{
			case CornerResult::Inner:
				item->TargetState = LS_SHIMMY_45_INNER_RIGHT;
				return;

			case CornerResult::Outer:
				item->TargetState = LS_SHIMMY_45_OUTER_RIGHT;
				return;

			default:
				break;
			}
		}
	}

	info->Control.MoveAngle = item->Position.yRot;

	TestLaraHang(item, coll);

	if (item->AnimNumber == LA_REACH_TO_HANG)
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
						item->TargetState = LS_HANDSTAND;
					}
					else if (TrInput & IN_CROUCH)
					{
						item->TargetState = LS_HANG_TO_CRAWL;
						item->RequiredState = LS_CROUCH_IDLE;
					}
					else
					{
						item->TargetState = LS_GRABBING;
					}

					return;
				}

				if (coll->Front.Floor < -650 &&
					coll->Front.Floor - coll->Front.Ceiling >= -256 &&
					coll->FrontLeft.Floor - coll->FrontLeft.Ceiling >= -256 &&
					coll->FrontRight.Floor - coll->FrontRight.Ceiling >= -256)
				{
					item->TargetState = LS_HANG_TO_CRAWL;
					item->RequiredState = LS_CROUCH_IDLE;

					return;
			}
			}

			if (info->Control.CanClimbLadder &&
				coll->Middle.Ceiling <= -256 &&
				abs(coll->FrontLeft.Ceiling - coll->FrontRight.Ceiling) < SLOPE_DIFFERENCE)
			{
				if (TestLaraClimbStance(item, coll))
				{
					item->TargetState = LS_LADDER_IDLE;
				}
				else if (TestLastFrame(item))
				{
					SetAnimation(item, LA_LADDER_SHIMMY_UP);
				}
			}

			return;
		}

		if (TrInput & IN_BACK &&
			info->Control.CanClimbLadder &&
			coll->Middle.Floor > 344 &&
			item->AnimNumber == LA_REACH_TO_HANG)
		{
			if (TestLaraClimbStance(item, coll))
			{
				item->TargetState = LS_LADDER_IDLE;
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
	coll->Setup.Mode = CollProbeMode::FreeFlat;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
	if (!(TrInput & (IN_LEFT | IN_LSTEP)))
		item->TargetState = LS_HANG;
}

void lara_col_hangleft(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	/*state 30*/
	/*state code: lara_as_hangleft*/
	info->Control.MoveAngle = item->Position.yRot - ANGLE(90);
	coll->Setup.Radius = LARA_RAD;
	TestLaraHang(item, coll);
	info->Control.MoveAngle = item->Position.yRot - ANGLE(90);
}

void lara_as_hangright(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 31*/
	/*collision: lara_col_hangright*/
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	coll->Setup.Mode = CollProbeMode::FreeFlat;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);
	if (!(TrInput & (IN_RIGHT | IN_RSTEP)))
		item->TargetState = LS_HANG;
}

void lara_col_hangright(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	/*state 31*/
	/*state code: lara_as_hangright*/
	info->Control.MoveAngle = item->Position.yRot + ANGLE(90);
	coll->Setup.Radius = LARA_RAD;
	TestLaraHang(item, coll);
	info->Control.MoveAngle = item->Position.yRot + ANGLE(90);
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