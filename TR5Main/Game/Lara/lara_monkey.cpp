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
#include "Game/Lara/lara_overhang.h"
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
		item->goalAnimState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	// TODO: overhang
	//SlopeMonkeyExtra(item, coll);

	// This check is needed to prevent stealing goal state from previously set.
	//if (item->goalAnimState == LS_MONKEYSWING_IDLE)
	//	return;
	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN_MAX / 2)
			info->turnRate = -LARA_SLOW_TURN_MAX / 2;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN_MAX / 2)
			info->turnRate = LARA_SLOW_TURN_MAX / 2;
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

	item->goalAnimState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_IDLE (75)
// Control:		lara_as_monkey_idle()
void lara_col_monkey_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	item->fallspeed = 0;
	item->airborne = false;
	coll->Setup.BadFloorHeightDown = NO_BAD_POS;
	coll->Setup.BadFloorHeightUp = 0;
	coll->Setup.BadCeilingHeightDown = CLICK(1.25f);
	coll->Setup.BadCeilingHeightUp = -CLICK(1.25f);
	coll->Setup.CeilingSlopesAreWalls = true;
	coll->Setup.NoMonkeyFlagIsWall = true;
	coll->Setup.ForwardAngle = info->moveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);
	
	ShiftItem(item, coll);

	if (TestLaraMonkeyFall(item, coll))
	{
		SetLaraMonkeyFallState(item);
		return;
	}

	if (TestLaraMonkeyStep(item, coll))
	{
		DoLaraMonkeyStep(item, coll);
		return;
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
		item->goalAnimState = LS_DEATH;
		SetLaraMonkeyRelease(item);
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
		// TODO
		/*if (TrInput & IN_ROLL | IN_BACK)
		{
			item->goalAnimState = LS_MONKEY_TURN_180;
			return;
		}*/

		if (TrInput & IN_FORWARD)
		{
			item->goalAnimState = LS_MONKEY_FORWARD;
			return;
		}

		item->goalAnimState = LS_MONKEY_IDLE;
		return;
	}

	item->goalAnimState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_FORWARD (76)
// Control:		lara_as_monkey_forward()
void lara_col_monkey_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.BadFloorHeightDown = NO_BAD_POS;
	coll->Setup.BadFloorHeightUp = 0;
	coll->Setup.BadCeilingHeightDown = CLICK(1.25f);
	coll->Setup.BadCeilingHeightUp = -CLICK(1.25f);
	coll->Setup.CeilingSlopesAreWalls = true;
	coll->Setup.NoMonkeyFlagIsWall = true;
	coll->Setup.ForwardAngle = info->moveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);

	// TODO: Front collision handling when approaching non-monkey sectors displays strange behaviour.
	if (LaraDeflectEdgeMonkey(item, coll))
		LaraCollideStopMonkey(item, coll);

	if (TestLaraMonkeyFall(item, coll))
	{
		SetLaraMonkeyFallState(item);
		return;
	}

	if (TestLaraMonkeyStep(item, coll))
	{
		DoLaraMonkeyStep(item, coll);
		return;
	}
}

// State:		LS_MONKEY_BACK (TODO)
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
		item->goalAnimState = LS_DEATH;
		SetLaraMonkeyRelease(item);
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

	item->goalAnimState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_BACK (TODO)
// Control:		lara_as_monkey_back()
void lara_col_monkey_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot + ANGLE(180.0f);
	coll->Setup.BadFloorHeightDown = NO_BAD_POS;
	coll->Setup.BadFloorHeightUp = 0;
	coll->Setup.BadCeilingHeightDown = CLICK(1.25f);
	coll->Setup.BadCeilingHeightUp = -CLICK(1.25f);
	coll->Setup.CeilingSlopesAreWalls = true;
	coll->Setup.NoMonkeyFlagIsWall = true;
	coll->Setup.ForwardAngle = info->moveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeMonkey(item, coll))
		LaraCollideStopMonkey(item, coll);

	if (TestLaraMonkeyFall(item, coll))
	{
		SetLaraMonkeyFallState(item);
		return;
	}

	if (TestLaraMonkeyStep(item, coll))
	{
		DoLaraMonkeyStep(item, coll);
		return;
	}
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
		item->goalAnimState = LS_DEATH;
		SetLaraMonkeyRelease(item);
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

	item->goalAnimState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_SHIMMY_LEFT (7)
// Control:		lara_as_monkey_shimmy_left()
void lara_col_monkey_shimmy_left(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot - ANGLE(90.0f);
	coll->Setup.BadFloorHeightDown = NO_BAD_POS;
	coll->Setup.BadFloorHeightUp = 0;
	coll->Setup.BadCeilingHeightDown = CLICK(0.5f);
	coll->Setup.BadCeilingHeightUp = -CLICK(0.5f);
	coll->Setup.CeilingSlopesAreWalls = true;
	coll->Setup.NoMonkeyFlagIsWall = true;
	coll->Setup.ForwardAngle = info->moveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeMonkey(item, coll))
		LaraCollideStopMonkey(item, coll);

	if (TestLaraMonkeyFall(item, coll))
	{
		SetLaraMonkeyFallState(item);
		return;
	}

	if (TestLaraMonkeyStep(item, coll))
	{
		DoLaraMonkeyStep(item, coll);
		return;
	}
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
		item->goalAnimState = LS_DEATH;
		SetLaraMonkeyRelease(item);
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

	item->goalAnimState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_SHIMMY_RIGHT (78)
// Control:		lara_as_monkey_shimmy_right()
void lara_col_monkey_shimmy_right(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot + ANGLE(90.0f);
	coll->Setup.BadFloorHeightDown = NO_BAD_POS;
	coll->Setup.BadFloorHeightUp = 0;
	coll->Setup.BadCeilingHeightDown = CLICK(0.5f);
	coll->Setup.BadCeilingHeightUp = -CLICK(0.5f);
	coll->Setup.CeilingSlopesAreWalls = true;
	coll->Setup.NoMonkeyFlagIsWall = true;
	coll->Setup.ForwardAngle = info->moveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeMonkey(item, coll))
		LaraCollideStopMonkey(item, coll);

	if (TestLaraMonkeyFall(item, coll))
	{
		SetLaraMonkeyFallState(item);
		return;
	}

	if (TestLaraMonkeyStep(item, coll))
	{
		DoLaraMonkeyStep(item, coll);
		return;
	}
}

// State:		LS_MONKEY_TURN_180 (79)
// Collision:	lara_as_monkey_turn_180()
void lara_as_monkey_turn_180(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpaz = false;
	Camera.targetElevation = -ANGLE(5.0f);

	item->goalAnimState = LS_MONKEY_IDLE;
}

// State:		LS_MONKEY_TURN_180 (79)
// Control:		lara_as_monkey_turn_180()
void lara_col_monkey_turn_180(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_monkey_idle(item, coll);
}

// State:		LS_MONKEY_TURN_LEFT (82)
// Collision:	lara_col_monkey_turn_left()
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
		item->goalAnimState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_ACTION && info->canMonkeySwing)
	{
		if (TrInput & IN_FORWARD && TestLaraMonkeyForward(item, coll))
		{
			item->goalAnimState = LS_MONKEY_FORWARD;
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

		if (TrInput & IN_LEFT)
		{
			// TODO: When an immediate dispatch out of this state becomes possible from any frame,
			// move the following block right beneath the death check. @Sezz 2022.01.16
			info->turnRate -= LARA_TURN_RATE;
			if (info->turnRate < -LARA_SLOW_TURN_MAX)
				info->turnRate = -LARA_SLOW_TURN_MAX;

			item->goalAnimState = LS_MONKEY_TURN_LEFT;
			return;
		}

		item->goalAnimState = LS_MONKEY_IDLE;
		return;
	}

	item->goalAnimState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_TURN_LEFT (82)
// Control:		lara_as_monkey_turn_left()
void lara_col_monkey_turn_left(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_monkey_idle(item, coll);
}

// State:		LS_MONKEY_TURN_RIGHT (83)
// Collision:	lara_col_monkey_turn_right()
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
		item->goalAnimState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_ACTION && info->canMonkeySwing)
	{
		if (TrInput & IN_FORWARD && TestLaraMonkeyForward(item, coll))
		{
			item->goalAnimState = LS_MONKEY_FORWARD;
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

		if (TrInput & IN_RIGHT)
		{
			// TODO: When an immediate dispatch out of this state becomes possible from any frame,
			// move the following block right beneath the death check. @Sezz 2022.01.16
			info->turnRate += LARA_TURN_RATE;
			if (info->turnRate > LARA_SLOW_TURN_MAX)
				info->turnRate = LARA_SLOW_TURN_MAX;

			item->goalAnimState = LS_MONKEY_TURN_RIGHT;
			return;
		}

		item->goalAnimState = LS_MONKEY_IDLE;
		return;
	}

	item->goalAnimState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_TURN_RIGHT (83)
// Control:		lara_as_monkey_turn_right()
void lara_col_monkey_turn_right(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_monkey_idle(item, coll);
}
