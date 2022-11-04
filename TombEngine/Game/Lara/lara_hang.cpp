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
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Input;

// -----------------------------
// LEDGE HANG
// Control & Collision Functions
// -----------------------------

// State:		LS_HANG_IDLE (10)
// Collision:	lara_col_hang_idle()
void lara_as_hang_idle(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.IsClimbingLadder = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);

	if (item->HitPoints <= 0)
	{
		SetLaraHangReleaseAnimation(item);
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown(item);

	if (TrInput & IN_ACTION)
	{
		// TODO: Allow direction locking just like with standing jumps. Needs new ledge jump prepare state? -- Sezz 24.10.2022
		if (TrInput & IN_JUMP && TestLaraLedgeJump(item, coll))
		{
			if (TrInput & IN_BACK)
				item->Animation.TargetState = LS_JUMP_FORWARD;
			else
				item->Animation.TargetState = LS_JUMP_UP;
		}

		if (TrInput & IN_FORWARD)
		{
			if (TestLaraHangToCrouch(item, coll))
			{
				item->Animation.TargetState = LS_HANG_TO_CROUCH;
				return;
			}
			else if (TestLaraHangToStand(item, coll)) [[likely]]
			{
				if (TrInput & IN_CROUCH)
					item->Animation.TargetState = LS_HANG_TO_CROUCH;
				else if (TrInput & IN_WALK)
					item->Animation.TargetState = LS_HANDSTAND;
				else [[likely]]
					item->Animation.TargetState = LS_GRABBING;

				return;
			}
			else if (TestLaraLadderShimmyUp(item, coll))
			{
				if (!(TrInput & IN_CROUCH) && TestLaraClimbIdle(item, coll))
					item->Animation.TargetState = LS_LADDER_IDLE;
				else if (TestLastFrame(item))
					SetAnimation(item, LA_LADDER_SHIMMY_UP);

				return;
			}
		}
		else if (TrInput & IN_BACK)
		{
			if (TestLaraLadderShimmyDown(item, coll))
			{
				if (!(TrInput & IN_CROUCH) && TestLaraClimbIdle(item, coll))
					item->Animation.TargetState = LS_LADDER_IDLE;
				else if (TestLastFrame(item))
					SetAnimation(item, LA_LADDER_SHIMMY_DOWN);

				return;
			}
		}

		if (TrInput & (IN_LEFT | IN_LSTEP))
		{
			if (TestLaraShimmyLeft(item, coll) && HasStateDispatch(item, LS_SHIMMY_LEFT))
			{
				item->Animation.TargetState = LS_SHIMMY_LEFT;
				return;
			}

			auto cornerShimmyState = GetLaraCornerShimmyState(item, coll);
			if (cornerShimmyState != (LaraState)-1)
			{
				item->Animation.TargetState = cornerShimmyState;
				return;
			}
		}
		else if (TrInput & (IN_RIGHT | IN_RSTEP))
		{
			if (TestLaraShimmyRight(item, coll) && HasStateDispatch(item, LS_SHIMMY_RIGHT))
			{
				item->Animation.TargetState = LS_SHIMMY_RIGHT;
				return;
			}

			auto cornerShimmyState = GetLaraCornerShimmyState(item, coll);
			if (cornerShimmyState != (LaraState)-1)
			{
				item->Animation.TargetState = cornerShimmyState;
				return;
			}
		}

		item->Animation.TargetState = LS_HANG_IDLE;
		return;
	}

	SetLaraHangReleaseAnimation(item);
}

// State:		LS_HANG_IDLE (10)
// Control:		lara_as_hang_idle()
void lara_col_hang_idle(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	lara->Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.Mode = CollisionProbeMode::FreeFlat;

	TestLaraHang(item, coll);
}

// State:		LS_SHIMMY_LEFT (30)
// Collision:	lara_col_shimmy_left()
void lara_as_shimmy_left(ItemInfo* item, CollisionInfo* coll)
{
	coll->Setup.Mode = CollisionProbeMode::FreeFlat;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);

	if (item->HitPoints <= 0)
	{
		SetLaraHangReleaseAnimation(item);
		return;
	}

	if (TrInput & IN_ACTION)
	{
		if (TrInput & (IN_LEFT | IN_LSTEP))
		{
			item->Animation.TargetState = LS_SHIMMY_LEFT;
			return;
		}

		item->Animation.TargetState = LS_HANG_IDLE;
		return;
	}

	SetLaraHangReleaseAnimation(item);
}

// State:		LS_SHIMMY_LEFT (30)
// Control:		lara_as_shimmy_left()
void lara_col_shimmy_left(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Pose.Orientation.y - ANGLE(90.0f);
	coll->Setup.Radius = LARA_RADIUS;

	TestLaraHang(item, coll);
	lara->Control.MoveAngle = item->Pose.Orientation.y - ANGLE(90.0f);
}

// State:		LS_SHIMMY_RIGHT (31)
// Collision:	lara_col_shimmy_right()
void lara_as_shimmy_right(ItemInfo* item, CollisionInfo* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	coll->Setup.Mode = CollisionProbeMode::FreeFlat;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);

	if (item->HitPoints <= 0)
	{
		SetLaraHangReleaseAnimation(item);
		return;
	}

	if (TrInput & IN_ACTION)
	{
		if (TrInput & (IN_RIGHT | IN_RSTEP))
		{
			item->Animation.TargetState = LS_SHIMMY_RIGHT;
			return;
		}

		item->Animation.TargetState = LS_HANG_IDLE;
		return;
	}

	SetLaraHangReleaseAnimation(item);
}

// State:		LS_SHIMMY_RIGHT (31)
// Control:		lara_as_shimmy_right()
void lara_col_shimmy_right(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Pose.Orientation.y + ANGLE(90.0f);
	coll->Setup.Radius = LARA_RADIUS;

	TestLaraHang(item, coll);
	lara->Control.MoveAngle = item->Pose.Orientation.y + ANGLE(90.0f);
}

// State:		LS_SHIMMY_OUTER_LEFT (107), LS_SHIMMY_OUTER_RIGHT (108), LS_SHIMMY_INNER_LEFT (109), LS_SHIMMY_INNER_RIGHT (110)
// Collision:	lara_default_col()
void lara_as_shimmy_corner(ItemInfo* item, CollisionInfo* coll)
{
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(33.0f);
	Camera.laraNode = LM_TORSO;

	SetLaraCornerAnimation(item, coll, TestLastFrame(item));
}

// State:		LS_HANDSTAND (54)
// Collision:	lara_default_col()
void lara_as_handstand(ItemInfo* item, CollisionInfo* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
}
