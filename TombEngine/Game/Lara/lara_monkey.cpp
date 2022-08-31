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
#include "Flow/ScriptInterfaceFlowHandler.h"

using namespace TEN::Input;

// -----------------------------
// MONKEY SWING
// Control & Collision Functions
// -----------------------------

// State:		LS_MONKEY_IDLE (75)
// Collision:	lara_col_monkey_idle()
void lara_as_monkey_idle(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->ExtraTorsoRot = Vector3Shrt();
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

	// Overhang hook.
	SlopeMonkeyExtra(item, coll);

	if ((TrInput & IN_LEFT &&
			!(TrInput & IN_LSTEP || (TrInput & IN_WALK && TrInput & IN_LEFT))) ||	// Shimmy locks orientation.
		(TrInput & IN_RIGHT &&
			!(TrInput & IN_RSTEP || (TrInput & IN_WALK && TrInput & IN_RIGHT))))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX / 2);
	}

	if (TrInput & IN_ACTION && lara->Control.CanMonkeySwing)
	{
		if (TrInput & IN_JUMP)
		{
			item->Animation.TargetState = LS_JUMP_FORWARD;
			lara->Control.HandStatus = HandStatus::Free;
			return;
		}

		if (TrInput & IN_ROLL || (TrInput & IN_FORWARD && TrInput & IN_BACK))
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

		if (TrInput & IN_LSTEP || (TrInput & IN_WALK && TrInput & IN_LEFT))
		{
			if (TestLaraMonkeyShimmyLeft(item, coll))
				item->Animation.TargetState = LS_MONKEY_SHIMMY_LEFT;
			else
				item->Animation.TargetState = LS_MONKEY_IDLE;

			return;
		}
		else if (TrInput & IN_RSTEP || (TrInput & IN_WALK && TrInput & IN_RIGHT))
		{
			if (TestLaraMonkeyShimmyRight(item, coll))
				item->Animation.TargetState = LS_MONKEY_SHIMMY_RIGHT;
			else
				item->Animation.TargetState = LS_MONKEY_IDLE;

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

		item->Animation.TargetState = LS_MONKEY_IDLE;
		return;
	}

	item->Animation.TargetState = LS_JUMP_UP;
	SetLaraMonkeyRelease(item);
}

// State:		LS_MONKEY_IDLE (75)
// Control:		lara_as_monkey_idle()
void lara_col_monkey_idle(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Pose.Orientation.y;
	item->Animation.IsAirborne = false;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = CLICK(1.25f);
	coll->Setup.UpperCeilingBound = -CLICK(1.25f);
	coll->Setup.BlockCeilingSlope = true;
	coll->Setup.BlockMonkeySwingEdge = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	coll->Setup.Radius = LARA_RADIUS;
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
void lara_as_monkey_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->ExtraTorsoRot = Vector3Shrt();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & (IN_LEFT | IN_RIGHT))
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX);

	if (TrInput & IN_ACTION && lara->Control.CanMonkeySwing)
	{
		if (TrInput & IN_ROLL || (TrInput & IN_FORWARD && TrInput & IN_BACK))
		{
			item->Animation.TargetState = LS_MONKEY_TURN_180;
			return;
		}

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
void lara_col_monkey_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = CLICK(1.25f);
	coll->Setup.UpperCeilingBound = -CLICK(1.25f);
	coll->Setup.BlockCeilingSlope = true;
	coll->Setup.BlockMonkeySwingEdge = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	coll->Setup.Radius = LARA_RADIUS;
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
void lara_as_monkey_back(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->ExtraTorsoRot = Vector3Shrt();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (TrInput & (IN_LEFT | IN_RIGHT))
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX);

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
void lara_col_monkey_back(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Pose.Orientation.y + ANGLE(180.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = CLICK(1.25f);
	coll->Setup.UpperCeilingBound = -CLICK(1.25f);
	coll->Setup.BlockCeilingSlope = true;
	coll->Setup.BlockMonkeySwingEdge = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	coll->Setup.Radius = LARA_RADIUS;
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
void lara_as_monkey_shimmy_left(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->ExtraTorsoRot = Vector3Shrt();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (!(TrInput & IN_WALK))	// WALK locks orientation.
	{
		if (TrInput & (IN_LEFT | IN_RIGHT))
			ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX);
	}

	if (TrInput & IN_ACTION && lara->Control.CanMonkeySwing)
	{
		if (TrInput & IN_LSTEP || (TrInput & IN_WALK && TrInput & IN_LEFT))
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
void lara_col_monkey_shimmy_left(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Pose.Orientation.y - ANGLE(90.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = CLICK(0.5f);
	coll->Setup.UpperCeilingBound = -CLICK(0.5f);
	coll->Setup.BlockCeilingSlope = true;
	coll->Setup.BlockMonkeySwingEdge = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	coll->Setup.Radius = LARA_RADIUS;
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
void lara_as_monkey_shimmy_right(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->ExtraTorsoRot = Vector3Shrt();
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		SetLaraMonkeyRelease(item);
		return;
	}

	if (!(TrInput & IN_WALK))	// WALK locks orientation.
	{
		if (TrInput & (IN_LEFT | IN_RIGHT))
			ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX);
	}

	if (TrInput & IN_ACTION && lara->Control.CanMonkeySwing)
	{
		if (TrInput & IN_RSTEP || (TrInput & IN_WALK && TrInput & IN_RIGHT))
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
void lara_col_monkey_shimmy_right(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Pose.Orientation.y + ANGLE(90.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = CLICK(0.5f);
	coll->Setup.UpperCeilingBound = -CLICK(0.5f);
	coll->Setup.BlockCeilingSlope = true;
	coll->Setup.BlockMonkeySwingEdge = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	coll->Setup.Radius = LARA_RADIUS;
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
void lara_as_monkey_turn_180(ItemInfo* item, CollisionInfo* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetElevation = -ANGLE(5.0f);

	ModulateLaraTurnRateY(item, 0, 0, 0);

	item->Animation.TargetState = LS_MONKEY_IDLE;
}

// State:		LS_MONKEY_TURN_180 (79)
// Control:		lara_as_monkey_turn_180()
void lara_col_monkey_turn_180(ItemInfo* item, CollisionInfo* coll)
{
	lara_col_monkey_idle(item, coll);
}

// State:		LS_MONKEY_TURN_LEFT (82)
// Collision:	lara_col_monkey_turn_left()
void lara_as_monkey_turn_left(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->ExtraTorsoRot = Vector3Shrt();
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
		if (TrInput & IN_JUMP)
		{
			item->Animation.TargetState = LS_JUMP_FORWARD;
			lara->Control.HandStatus = HandStatus::Free;
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

		if (TrInput & IN_LSTEP || (TrInput & IN_WALK && TrInput & IN_LEFT))
		{
			if (TestLaraMonkeyShimmyLeft(item, coll))
				item->Animation.TargetState = LS_MONKEY_SHIMMY_LEFT;
			else
				item->Animation.TargetState = LS_MONKEY_IDLE;

			return;
		}
		else if (TrInput & IN_RSTEP || (TrInput & IN_WALK && TrInput & IN_RIGHT))
		{
			if (TestLaraMonkeyShimmyRight(item, coll))
				item->Animation.TargetState = LS_MONKEY_SHIMMY_RIGHT;
			else
				item->Animation.TargetState = LS_MONKEY_IDLE;

			return;
		}

		if (TrInput & IN_LEFT)
		{
			item->Animation.TargetState = LS_MONKEY_TURN_LEFT;
			ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX);
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
void lara_col_monkey_turn_left(ItemInfo* item, CollisionInfo* coll)
{
	lara_col_monkey_idle(item, coll);
}

// State:		LS_MONKEY_TURN_RIGHT (83)
// Collision:	lara_col_monkey_turn_right()
void lara_as_monkey_turn_right(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->ExtraTorsoRot = Vector3Shrt();
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
		if (TrInput & IN_JUMP)
		{
			item->Animation.TargetState = LS_JUMP_FORWARD;
			lara->Control.HandStatus = HandStatus::Free;
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

		if (TrInput & IN_LSTEP || (TrInput & IN_WALK && TrInput & IN_LEFT))
		{
			if (TestLaraMonkeyShimmyLeft(item, coll))
				item->Animation.TargetState = LS_MONKEY_SHIMMY_LEFT;
			else
				item->Animation.TargetState = LS_MONKEY_IDLE;

			return;
		}
		else if (TrInput & IN_RSTEP || (TrInput & IN_WALK && TrInput & IN_RIGHT))
		{
			if (TestLaraMonkeyShimmyRight(item, coll))
				item->Animation.TargetState = LS_MONKEY_SHIMMY_RIGHT;
			else
				item->Animation.TargetState = LS_MONKEY_IDLE;

			return;
		}

		if (TrInput & IN_RIGHT)
		{
			item->Animation.TargetState = LS_MONKEY_TURN_RIGHT;
			ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX);
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
void lara_col_monkey_turn_right(ItemInfo* item, CollisionInfo* coll)
{
	lara_col_monkey_idle(item, coll);
}
