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
#include "Scripting/GameFlowScript.h"
#include "Specific/input.h"
#include "Specific/level.h"

// -----------------------------
// MONKEY SWING
// Control & Collision Functions
// -----------------------------

// State:		LS_MONKEY_IDLE (75)
// Collision:	lara_col_monkey_idle()
void lara_as_monkey_idle(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->ExtraTorsoRot = PHD_3DPOS();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown(item);

	// TODO: overhang
	//SlopeMonkeyExtra(item, coll);

	// This check is needed to prevent stealing goal state from previously set.
	//if (item->TargetState == LS_MONKEY_IDLE)
	//	return;

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_SLOW_TURN_MAX / 2)
			lara->Control.TurnRate = -LARA_SLOW_TURN_MAX / 2;
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_SLOW_TURN_MAX / 2)
			lara->Control.TurnRate = LARA_SLOW_TURN_MAX / 2;
	}

	if (TrInput & IN_ACTION && lara->Control.CanMonkeySwing)
	{
		if (TrInput & IN_ROLL)
		{
			item->Animation.TargetState = LS_MONKEY_TURN_180;
			return;
		}

		if (TrInput & IN_FORWARD && TestLaraMonkeyForward(item, coll))
		{
			item->Animation.TargetState = LS_MONKEY_FORWARD;
			return;
		}
		else if (TrInput & IN_BACK && TestLaraMonkeyBack(item, coll))
		{
			item->Animation.TargetState = LS_MONKEY_BACK;
			return;
		}

		if (TrInput & IN_LEFT)
		{
			item->Animation.TargetState = LS_MONKEY_TURN_LEFT;
			return;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->Animation.TargetState = LS_MONKEY_TURN_RIGHT;
			return;
		}

		if (TrInput & IN_LSTEP && TestLaraMonkeyShimmyLeft(item, coll))
		{
			item->Animation.TargetState = LS_MONKEY_SHIMMY_LEFT;
			return;
		}
		else if (TrInput & IN_RSTEP && TestLaraMonkeyShimmyRight(item, coll))
		{
			item->Animation.TargetState = LS_MONKEY_SHIMMY_RIGHT;
			return;
		}

		item->Animation.TargetState = LS_MONKEY_IDLE;
		return;
	}

	item->Animation.TargetState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_IDLE (75)
// Control:		lara_as_monkey_idle()
void lara_col_monkey_idle(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot;
	item->Animation.Airborne = false;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = CLICK(1.25f);
	coll->Setup.UpperCeilingBound = -CLICK(1.25f);
	coll->Setup.CeilingSlopeIsWall = true;
	coll->Setup.NoMonkeyFlagIsWall = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);
	
	// HACK: Prevent ShiftItem() from causing an instantaneous snap, thereby interfering with DoLaraMonkeyStep(), when going down a step. @Sezz 2022.01.28
	if (coll->Shift.y >= 0 && coll->Shift.y <= CLICK(1.25f))
		coll->Shift.y = 0;
	ShiftItem(item, coll);

	if (TestLaraMonkeyFall(item, coll))
	{
		SetLaraMonkeyFallAnimation(item);
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
void lara_as_monkey_forward(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->ExtraTorsoRot = PHD_3DPOS();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = LARA_SLOW_TURN_MAX;
	}

	if (TrInput & IN_ACTION && lara->Control.CanMonkeySwing)
	{
		// TODO
		/*if (TrInput & IN_ROLL | IN_BACK)
		{
			item->TargetState = LS_MONKEY_TURN_180;
			return;
		}*/

		if (TrInput & IN_FORWARD)
		{
			item->Animation.TargetState = LS_MONKEY_FORWARD;
			return;
		}

		item->Animation.TargetState = LS_MONKEY_IDLE;
		return;
	}

	item->Animation.TargetState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_FORWARD (76)
// Control:		lara_as_monkey_forward()
void lara_col_monkey_forward(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = CLICK(1.25f);
	coll->Setup.UpperCeilingBound = -CLICK(1.25f);
	coll->Setup.CeilingSlopeIsWall = true;
	coll->Setup.NoMonkeyFlagIsWall = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeMonkey(item, coll))
		LaraCollideStopMonkey(item, coll);

	if (TestLaraMonkeyFall(item, coll))
	{
		SetLaraMonkeyFallAnimation(item);
		return;
	}

	if (TestLaraMonkeyStep(item, coll))
	{
		DoLaraMonkeyStep(item, coll);
		return;
	}
}

// State:		LS_MONKEY_BACK (163)
// Collision:	lara_col_monkey_back()
void lara_as_monkey_back(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->ExtraTorsoRot = PHD_3DPOS();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = LARA_SLOW_TURN_MAX;
	}

	if (TrInput & IN_ACTION && lara->Control.CanMonkeySwing)
	{
		if (TrInput & IN_BACK)
		{
			item->Animation.TargetState = LS_MONKEY_BACK;
			return;
		}

		item->Animation.TargetState = LS_MONKEY_IDLE;
		return;
	}

	item->Animation.TargetState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_BACK (163)
// Control:		lara_as_monkey_back()
void lara_col_monkey_back(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot + ANGLE(180.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = CLICK(1.25f);
	coll->Setup.UpperCeilingBound = -CLICK(1.25f);
	coll->Setup.CeilingSlopeIsWall = true;
	coll->Setup.NoMonkeyFlagIsWall = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeMonkey(item, coll))
		LaraCollideStopMonkey(item, coll);

	if (TestLaraMonkeyFall(item, coll))
	{
		SetLaraMonkeyFallAnimation(item);
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
void lara_as_monkey_shimmy_left(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->ExtraTorsoRot = PHD_3DPOS();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = LARA_SLOW_TURN_MAX;
	}

	if (TrInput & IN_ACTION && lara->Control.CanMonkeySwing)
	{
		if (TrInput & IN_LSTEP)
		{
			item->Animation.TargetState = LS_MONKEY_SHIMMY_LEFT;
			return;
		}

		item->Animation.TargetState = LS_MONKEY_IDLE;
		return;
	}

	item->Animation.TargetState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_SHIMMY_LEFT (7)
// Control:		lara_as_monkey_shimmy_left()
void lara_col_monkey_shimmy_left(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot - ANGLE(90.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = CLICK(0.5f);
	coll->Setup.UpperCeilingBound = -CLICK(0.5f);
	coll->Setup.CeilingSlopeIsWall = true;
	coll->Setup.NoMonkeyFlagIsWall = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeMonkey(item, coll))
		LaraCollideStopMonkey(item, coll);

	if (TestLaraMonkeyFall(item, coll))
	{
		SetLaraMonkeyFallAnimation(item);
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
void lara_as_monkey_shimmy_right(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->ExtraTorsoRot = PHD_3DPOS();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_SLOW_TURN_MAX)
			lara->Control.TurnRate = LARA_SLOW_TURN_MAX;
	}

	if (TrInput & IN_ACTION && lara->Control.CanMonkeySwing)
	{
		if (TrInput & IN_RSTEP)
		{
			item->Animation.TargetState = LS_MONKEY_SHIMMY_RIGHT;
			return;
		}

		item->Animation.TargetState = LS_MONKEY_IDLE;
		return;
	}

	item->Animation.TargetState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_SHIMMY_RIGHT (78)
// Control:		lara_as_monkey_shimmy_right()
void lara_col_monkey_shimmy_right(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot + ANGLE(90.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = CLICK(0.5f);
	coll->Setup.UpperCeilingBound = -CLICK(0.5f);
	coll->Setup.CeilingSlopeIsWall = true;
	coll->Setup.NoMonkeyFlagIsWall = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	coll->Setup.Radius = LARA_RAD;
	coll->Setup.Height = LARA_HEIGHT_MONKEY;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeMonkey(item, coll))
		LaraCollideStopMonkey(item, coll);

	if (TestLaraMonkeyFall(item, coll))
	{
		SetLaraMonkeyFallAnimation(item);
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
void lara_as_monkey_turn_180(ITEM_INFO* item, CollisionInfo* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	item->Animation.TargetState = LS_MONKEY_IDLE;
}

// State:		LS_MONKEY_TURN_180 (79)
// Control:		lara_as_monkey_turn_180()
void lara_col_monkey_turn_180(ITEM_INFO* item, CollisionInfo* coll)
{
	lara_col_monkey_idle(item, coll);
}

// State:		LS_MONKEY_TURN_LEFT (82)
// Collision:	lara_col_monkey_turn_left()
void lara_as_monkey_turn_left(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->ExtraTorsoRot = PHD_3DPOS();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_ACTION && lara->Control.CanMonkeySwing)
	{
		if (TrInput & IN_FORWARD && TestLaraMonkeyForward(item, coll))
		{
			item->Animation.TargetState = LS_MONKEY_FORWARD;
			return;
		}
		else if (TrInput & IN_BACK && TestLaraMonkeyBack(item, coll))
		{
			item->Animation.TargetState = LS_MONKEY_BACK;
			return;
		}

		if (TrInput & IN_LSTEP && TestLaraMonkeyShimmyLeft(item, coll))
		{
			item->Animation.TargetState = LS_MONKEY_SHIMMY_LEFT;
			return;
		}
		else if (TrInput & IN_RSTEP && TestLaraMonkeyShimmyRight(item, coll))
		{
			item->Animation.TargetState = LS_MONKEY_SHIMMY_RIGHT;
			return;
		}

		if (TrInput & IN_LEFT)
		{
			item->Animation.TargetState = LS_MONKEY_TURN_LEFT;

			lara->Control.TurnRate -= LARA_TURN_RATE;
			if (lara->Control.TurnRate < -LARA_SLOW_TURN_MAX)
				lara->Control.TurnRate = -LARA_SLOW_TURN_MAX;

			return;
		}

		item->Animation.TargetState = LS_MONKEY_IDLE;
		return;
	}

	item->Animation.TargetState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_TURN_LEFT (82)
// Control:		lara_as_monkey_turn_left()
void lara_col_monkey_turn_left(ITEM_INFO* item, CollisionInfo* coll)
{
	lara_col_monkey_idle(item, coll);
}

// State:		LS_MONKEY_TURN_RIGHT (83)
// Collision:	lara_col_monkey_turn_right()
void lara_as_monkey_turn_right(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->ExtraTorsoRot = PHD_3DPOS();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & IN_ACTION && lara->Control.CanMonkeySwing)
	{
		if (TrInput & IN_FORWARD && TestLaraMonkeyForward(item, coll))
		{
			item->Animation.TargetState = LS_MONKEY_FORWARD;
			return;
		}
		else if (TrInput & IN_BACK && TestLaraMonkeyBack(item, coll))
		{
			item->Animation.TargetState = LS_MONKEY_BACK;
			return;
		}

		if (TrInput & IN_LSTEP && TestLaraMonkeyShimmyLeft(item, coll))
		{
			item->Animation.TargetState = LS_MONKEY_SHIMMY_LEFT;
			return;
		}
		else if (TrInput & IN_RSTEP && TestLaraMonkeyShimmyRight(item, coll))
		{
			item->Animation.TargetState = LS_MONKEY_SHIMMY_RIGHT;
			return;
		}

		if (TrInput & IN_RIGHT)
		{
			item->Animation.TargetState = LS_MONKEY_TURN_RIGHT;

			lara->Control.TurnRate += LARA_TURN_RATE;
			if (lara->Control.TurnRate > LARA_SLOW_TURN_MAX)
				lara->Control.TurnRate = LARA_SLOW_TURN_MAX;

			return;
		}

		item->Animation.TargetState = LS_MONKEY_IDLE;
		return;
	}

	item->Animation.TargetState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_TURN_RIGHT (83)
// Control:		lara_as_monkey_turn_right()
void lara_col_monkey_turn_right(ITEM_INFO* item, CollisionInfo* coll)
{
	lara_col_monkey_idle(item, coll);
}
