#include "framework.h"
#include "Game/Lara/lara_monkey.h"

#include "Game/camera.h"
#include "Game/collision/floordata.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Scripting/GameFlowScript.h"

using namespace TEN::Floordata;

// -----------------------------
// MONKEY SWING
// Control & Collision Functions
// -----------------------------

// State:		LS_MONKEY_IDLE (75)
// Collision:	lara_col_monkey_idle()
void lara_as_monkey_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->torsoXrot = 0;
	info->torsoYrot = 0;
	info->torsoZrot = 0;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_IDLE; //
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_MONKEY_MOVE_TURN_MAX / 2)
			info->turnRate = -LARA_MONKEY_MOVE_TURN_MAX / 2;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_MONKEY_MOVE_TURN_MAX / 2)
			info->turnRate = LARA_MONKEY_MOVE_TURN_MAX / 2;
	}

	if (TrInput & IN_ACTION && info->canMonkeySwing)
	{
		if (TrInput & IN_ROLL)
		{
			item->goalAnimState = LS_MONKEY_TURN_180;
			return;
		}

		if (TrInput & IN_FORWARD && TestLaraMonkeyForward(item, coll))
		{
			item->goalAnimState = LS_MONKEY_FORWARD;
			return;
		}
		// Not yet.
		/*else if (TrInput & IN_BACK && TestLaraMonkeyBack(item, coll))
		{
			item->goalAnimState = LS_MONKEY_BACK;
			return;
		}*/

		if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_MONKEY_TURN_LEFT;
			return;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_MONKEY_TURN_RIGHT;
			return;
		}

		if (TrInput & IN_LSTEP && TestLaraMonkeyShimmyLeft(item, coll))
		{
			item->goalAnimState = LS_MONKEY_SHIMMY_LEFT;
			return;
		}
		else if (TrInput & IN_RSTEP && TestLaraMonkeyShimmyRight(item, coll))
		{
			item->goalAnimState = LS_MONKEY_SHIMMY_RIGHT;
			return;
		}

		item->goalAnimState = LS_MONKEY_IDLE;
		return;
	}

	SetLaraMonkeyFallState(item);
}

// State:		LS_MONKEY_IDLE (75)
// Control:		lara_as_monkey_idle()
void lara_col_monkey_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	item->fallspeed = 0;
	item->gravityStatus = false;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = NO_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.ForwardAngle = info->moveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);
	
	ShiftItem(item, coll);
	DoLaraMonkeySnap(item, coll);
}

void olara_col_monkey_idle(ITEM_INFO* item, COLL_INFO* coll)
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

		coll->Setup.ForwardAngle = Lara.moveAngle;
		coll->Setup.Radius = LARA_RAD;
		coll->Setup.Height = LARA_HEIGHT_MONKEY;

		Lara.moveAngle = item->pos.yRot;

		GetCollisionInfo(coll, item);

		if (TrInput & IN_FORWARD && coll->CollisionType != CT_FRONT && abs(coll->Middle.Ceiling - coll->Front.Ceiling) < 50)
		{
			bool monkeyFront = LaraCollisionFront(item, item->pos.yRot, coll->Setup.Radius).BottomBlock->Flags.Monkeyswing;
			if (monkeyFront)
				item->goalAnimState = LS_MONKEY_FORWARD;
		}

		if (abs(coll->Middle.Ceiling - coll->Front.Ceiling) < 50)
			DoLaraMonkeySnap(item, coll);
	}
	else
	{
		TestLaraHang(item, coll);

		if (item->goalAnimState == LS_MONKEY_IDLE)
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

// State:		LS_MONKEY_FORWARD (76)
// Collision:	lara_col_monkey_forward()
void lara_as_monkey_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->torsoXrot = 0;
	info->torsoYrot = 0;
	info->torsoZrot = 0;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_MONKEY_IDLE; //
		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_MONKEY_MOVE_TURN_MAX)
			info->turnRate = -LARA_MONKEY_MOVE_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_MONKEY_MOVE_TURN_MAX)
			info->turnRate = LARA_MONKEY_MOVE_TURN_MAX;
	}

	if (TrInput & IN_ACTION && info->canMonkeySwing)
	{
		if (TrInput & IN_FORWARD)
		{
			item->goalAnimState = LS_MONKEY_FORWARD;
			return;
		}

		item->goalAnimState = LS_MONKEY_IDLE;
		return;
	}

	SetLaraMonkeyFallState(item);
}

// State:		LS_MONKEY_FORWARD (76)
// Control:		lara_as_monkey_forward()
void lara_col_monkey_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = NO_HEIGHT;
	coll->Setup.BadCeilingHeight = CLICK(1);
	coll->Setup.ForwardAngle = info->moveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeMonkey(item, coll))
		LaraCollideStopMonkey(item, coll);

	/*bool monkeyFront = LaraCollisionFront(item, item->pos.yRot, coll->Setup.Radius).BottomBlock->Flags.Monkeyswing;

	if (!monkeyFront || coll->CollisionType == CT_FRONT || abs(coll->Middle.Ceiling - coll->Front.Ceiling) > SLOPE_DIFFERENCE)
		SetAnimation(item, LA_MONKEYSWING_IDLE);
	else
	{
		if (abs(coll->Middle.Ceiling - coll->FrontLeft.Ceiling) <= SLOPE_DIFFERENCE)
		{
			if (abs(coll->Middle.Ceiling - coll->FrontRight.Ceiling) > SLOPE_DIFFERENCE)
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
	}*/

	DoLaraMonkeySnap(item, coll);
}

// State:		LS_MONKEY_BACK (143)
// Collision:	lara_col_monkey_back()
void lara_as_monkey_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->torsoXrot = 0;
	info->torsoYrot = 0;
	info->torsoZrot = 0;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_MONKEY_IDLE; //
		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN_MAX)
			info->turnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN_MAX)
			info->turnRate = LARA_SLOW_TURN_MAX;
	}

	if (TrInput & IN_ACTION && info->canMonkeySwing)
	{
		if (TrInput & IN_BACK)
		{
			item->goalAnimState = LS_MONKEY_BACK;
			return;
		}

		item->goalAnimState = LS_MONKEY_IDLE;
		return;
	}

	SetLaraMonkeyFallState(item);
}

// State:		LS_MONKEY_BACK (143)
// State:		lara_as_monkey_back()
void lara_col_monkey_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot + ANGLE(180.0f);
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = NO_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.ForwardAngle = info->moveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeMonkey(item, coll))
		LaraCollideStopMonkey(item, coll);

	ShiftItem(item, coll);
	DoLaraMonkeySnap(item, coll);
}

// State:		LS_MONKEY_SHIMMY_LEFT (77)
// Collision:	lara_col_monkey_shimmy_left()
void lara_as_monkey_shimmy_left(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->torsoXrot = 0;
	info->torsoYrot = 0;
	info->torsoZrot = 0;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_MONKEY_IDLE; //
		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN_MAX)
			info->turnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN_MAX)
			info->turnRate = LARA_SLOW_TURN_MAX;
	}

	if (TrInput & IN_ACTION && info->canMonkeySwing)
	{
		if (TrInput & IN_LSTEP)
		{
			item->goalAnimState = LS_MONKEY_SHIMMY_LEFT;
			return;
		}

		item->goalAnimState = LS_MONKEY_IDLE;
		return;
	}

	SetLaraMonkeyFallState(item);
}

// State:		LS_MONKEY_SHIMMY_LEFT (7)
// Control:		lara_as_monkey_shimmy_left()
void lara_col_monkey_shimmy_left(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot - ANGLE(90.0f);
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = NO_HEIGHT;
	coll->Setup.BadCeilingHeight = CLICK(0.8f);
	coll->Setup.ForwardAngle = info->moveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeMonkey(item, coll))
		LaraCollideStopMonkey(item, coll);

	ShiftItem(item, coll);
	DoLaraMonkeySnap(item, coll);
}

// State:		LS_MONKEY_SHIMMY_RIGHT (78)
// Collision:	lara_col_monkey_shimmy_right()
void lara_as_monkey_shimmy_right(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->torsoXrot = 0;
	info->torsoYrot = 0;
	info->torsoZrot = 0;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_MONKEY_IDLE; //
		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;

		if (info->turnRate < -LARA_SLOW_TURN_MAX)
			info->turnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;

		if (info->turnRate > LARA_SLOW_TURN_MAX)
			info->turnRate = LARA_SLOW_TURN_MAX;
	}

	if (TrInput & IN_ACTION && info->canMonkeySwing)
	{
		if (TrInput & IN_RSTEP)
		{
			item->goalAnimState = LS_MONKEY_SHIMMY_RIGHT;
			return;
		}

		item->goalAnimState = LS_MONKEY_IDLE;
		return;
	}

	SetLaraMonkeyFallState(item);
}

// State:		LS_MONKEY_SHIMMY_RIGHT (78)
// Control:		lara_as_monkey_shimmy_right()
void lara_col_monkey_shimmy_right(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot + ANGLE(90.0f);
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = NO_HEIGHT;
	coll->Setup.BadCeilingHeight = CLICK(0.8f);
	coll->Setup.ForwardAngle = info->moveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeMonkey(item, coll))
		LaraCollideStopMonkey(item, coll);

	ShiftItem(item, coll);
	DoLaraMonkeySnap(item, coll);
}

// State:		LS_MONKEY_TURN_180 (79)
// Control:		lara_col_monkey_turn_180()
void lara_as_monkey_turn_180(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetElevation = -ANGLE(5.0f);

	item->goalAnimState = LS_MONKEY_IDLE;
}

void lara_col_monkey_turn_180(ITEM_INFO* item, COLL_INFO* coll)
{
	/*state 79*/
	/*state code: lara_as_monkey180*/
	lara_col_monkey_forward(item, coll);
}

// State:		LS_MONKEY_TURN_LEFT (82)
// Collision:	lara_as_monkey_turn_left()
void lara_as_monkey_turn_left(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->torsoXrot = 0;
	info->torsoYrot = 0;
	info->torsoZrot = 0;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_MONKEY_IDLE; //
		return;
	}

	item->pos.yRot -= LARA_MONKEY_TURN_MAX;

	if (TrInput & IN_ACTION && info->canMonkeySwing)
	{
		if (TrInput & IN_LSTEP && TestLaraMonkeyShimmyLeft(item, coll))
		{
			item->goalAnimState = LS_MONKEY_SHIMMY_LEFT;
			return;
		}
		else if (TrInput & IN_RSTEP && TestLaraMonkeyShimmyRight(item, coll))
		{
			item->goalAnimState = LS_MONKEY_SHIMMY_RIGHT;
			return;
		}

		if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_MONKEY_TURN_LEFT;
			return;
		}

		item->goalAnimState = LS_MONKEY_IDLE;
		return;
	}

	SetLaraMonkeyFallState(item);
}

// State:		LS_MONKEY_TURN_LEFT (82)
// Control:		lara_as_monkey_turn_left()
void lara_col_monkey_turn_left(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_monkey_idle(item, coll);
}

// State:		LS_MONKEY_TURN_RIGHT (83)
// Collision:	lara_as_monkey_turn_right()
void lara_as_monkey_turn_right(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->torsoXrot = 0;
	info->torsoYrot = 0;
	info->torsoZrot = 0;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_MONKEY_IDLE; //
		return;
	}

	item->pos.yRot += LARA_MONKEY_TURN_MAX;

	if (TrInput & IN_ACTION && info->canMonkeySwing)
	{
		if (TrInput & IN_LSTEP && TestLaraMonkeyShimmyLeft(item, coll))
		{
			item->goalAnimState = LS_MONKEY_SHIMMY_LEFT;
			return;
		}
		else if (TrInput & IN_RSTEP && TestLaraMonkeyShimmyRight(item, coll))
		{
			item->goalAnimState = LS_MONKEY_SHIMMY_RIGHT;
			return;
		}

		if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_MONKEY_TURN_RIGHT;
			return;
		}

		item->goalAnimState = LS_MONKEY_IDLE;
		return;
	}

	SetLaraMonkeyFallState(item);
}

// State:		LS_MONKEY_TURN_RIGHT (83)
// Control:		lara_as_monkey_turn_right()
void lara_col_monkey_turn_right(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_monkey_idle(item, coll);
}

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
