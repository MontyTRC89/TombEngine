#include "framework.h"
#include "Game/Lara/lara_jump.h"

#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_basic.h"
#include "Game/Lara/lara_overhang.h"
#include "Game/Lara/lara_slide.h"
#include "Scripting/GameFlowScript.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

// -----------------------------
// JUMP
// Control & Collision Functions
// -----------------------------

// TODO: Unused? Naming is also completely mismatched.
// State:		LS_GRAB_TO_FALL
// Collision:	lara_void_func()
void lara_col_land(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_JUMP_FORWARD (3)
// Collision:	lara_col_jump_forward()
void lara_as_jump_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	// Update running jump counter in preparation for possible jump action soon after landing.
	info->Control.Count.RunJump++;
	if (info->Control.Count.RunJump > LARA_RUN_JUMP_TIME / 2)
		info->Control.Count.RunJump = LARA_RUN_JUMP_TIME / 2;

	if (item->HitPoints <= 0)
	{
		if (TestLaraLand(item, coll))
		{
			item->TargetState = LS_DEATH;
			SetLaraLand(item, coll);
		}

		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_JUMP_TURN_MAX)
			info->Control.TurnRate = -LARA_JUMP_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_JUMP_TURN_MAX)
			info->Control.TurnRate = LARA_JUMP_TURN_MAX;
	}

	if (TestLaraLand(item, coll))
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0)
			item->TargetState = LS_DEATH;
		else if (TrInput & IN_FORWARD && !(TrInput & IN_WALK) &&
			info->Control.WaterStatus != WaterStatus::Wade) [[likely]]
		{
			item->TargetState = LS_RUN_FORWARD;
		}
		else
			item->TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		return;
	}

	if (TrInput & IN_ACTION &&
		info->Control.HandStatus == HandStatus::Free)
	{
		item->TargetState = LS_REACH;
		return;
	}

	if (TrInput & (IN_ROLL | IN_BACK))
	{
		item->TargetState = LS_JUMP_ROLL_180;
		return;
	}

	if (TrInput & IN_WALK &&
		info->Control.HandStatus == HandStatus::Free)
	{
		item->TargetState = LS_SWAN_DIVE;
		return;
	}

	if (item->VerticalVelocity >= LARA_FREEFALL_SPEED)
	{
		item->TargetState = LS_FREEFALL;
		return;
	}

	item->TargetState = LS_JUMP_FORWARD;
}

// State:		LS_JUMP_FORWARD (3)
// Control:		lara_as_jump_forward()
void lara_col_jump_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	info->Control.MoveAngle = (item->Velocity > 0) ? item->Position.yRot : item->Position.yRot + ANGLE(180.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraSlide(item, coll) && TestLaraLand(item, coll))
	{
		SetLaraSlideState(item, coll);
		SetLaraLand(item, coll);
		return;
	}

	LaraDeflectEdgeJump(item, coll);

	// TODO: Why??
	info->Control.MoveAngle = (item->Velocity < 0) ? item->Position.yRot : info->Control.MoveAngle;
}

// State:		LS_FREEFALL (9)
// Collision:	lara_col_freefall()
void lara_as_freefall(ITEM_INFO* item, COLL_INFO* coll)
{
	item->Velocity = item->Velocity * 0.95f;

	if (item->VerticalVelocity == LARA_FREEFALL_SCREAM_SPEED &&
		item->HitPoints > 0)
	{
		SoundEffect(SFX_TR4_LARA_FALL, &item->Position, 0);
	}

	if (TestLaraLand(item, coll))
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0)
			item->TargetState = LS_DEATH;
		else
			item->TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		StopSoundEffect(SFX_TR4_LARA_FALL);
		return;
	}

	item->TargetState = LS_FREEFALL;
}

// State:		LS_FREEFALL (9)
// Control:		lara_as_freefall()
void lara_col_freefall(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraSlide(item, coll) && TestLaraLand(item, coll))
	{
		SetLaraSlideState(item, coll);
		SetLaraLand(item, coll);
		return;
	}

	LaraSlideEdgeJump(item, coll);
}

// State:		LS_REACH (11)
// Collision:	lara_col_reach()
void lara_as_reach(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	Camera.targetAngle = ANGLE(85.0f);

	if (item->HitPoints <= 0)
	{
		if (TestLaraLand(item, coll))
		{
			item->TargetState = LS_DEATH;
			SetLaraLand(item, coll);
		}

		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_JUMP_TURN_MAX / 2)
			info->Control.TurnRate = -LARA_JUMP_TURN_MAX / 2;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_JUMP_TURN_MAX / 2)
			info->Control.TurnRate = LARA_JUMP_TURN_MAX / 2;
	}

	if (TestLaraLand(item, coll))
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0)
			item->TargetState = LS_DEATH;
		else
			item->TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		return;
	}

	if (item->VerticalVelocity >= LARA_FREEFALL_SPEED)
	{
		item->TargetState = LS_FREEFALL;
		return;
	}

	item->TargetState = LS_REACH;
	// TODO: overhang
	//SlopeReachExtra(item, coll);
}

// State:		LS_REACH (11)
// Control:		lara_as_reach()
void lara_col_reach(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	if (info->Control.RopeControl.Ptr == -1)
		item->Airborne = true;

	info->Control.MoveAngle = item->Position.yRot;

	// HACK: height is altered according to fallspeed to fix "issues" with physically impossible
	// 6-click high ceiling running jumps. While TEN model is physically correct, original engines
	// allowed certain margin of deflection due to bug caused by hacky inclusion of headroom in coll checks.

	coll->Setup.Height = item->VerticalVelocity > 0 ? LARA_HEIGHT_REACH : LARA_HEIGHT;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	coll->Setup.Radius = coll->Setup.Radius * 1.2f;
	coll->Setup.Mode = CollProbeMode::FreeForward;
	GetCollisionInfo(coll, item);

	if (TestLaraHangJump(item, coll))
		return;

	if (TestLaraSlide(item, coll) && TestLaraLand(item, coll))
	{
		SetLaraSlideState(item, coll);
		SetLaraLand(item, coll);
		return;
	}

	LaraSlideEdgeJump(item, coll);

	GetCollisionInfo(coll, item);
	ShiftItem(item, coll);
}

// State:		LS_JUMP_PREPARE (15)
// Collision:	lara_col_jump_prepare()
void lara_as_jump_prepare(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_IDLE;
		return;
	}

	if (TrInput & (IN_FORWARD | IN_BACK))
	{
		if (TrInput & IN_LEFT)
		{
			info->Control.TurnRate -= LARA_TURN_RATE;
			if (info->Control.TurnRate < -LARA_SLOW_TURN_MAX)
				info->Control.TurnRate = -LARA_SLOW_TURN_MAX;
		}
		else if (TrInput & IN_RIGHT)
		{
			info->Control.TurnRate += LARA_TURN_RATE;
			if (info->Control.TurnRate > LARA_SLOW_TURN_MAX)
				info->Control.TurnRate = LARA_SLOW_TURN_MAX;
		}
	}

	// JUMP key repressed without directional key; cancel directional jump lock.
	if (DbInput & IN_JUMP && !(TrInput & IN_DIRECTION))
		info->Control.JumpDirection = JumpDirection::None;

	if ((TrInput & IN_FORWARD ||
		!(TrInput & IN_DIRECTION) && info->Control.JumpDirection == JumpDirection::Forward) &&
		TestLaraJumpForward(item, coll))
	{
		item->TargetState = LS_JUMP_FORWARD;
		info->Control.JumpDirection = JumpDirection::Forward;
		return;
	}
	else if ((TrInput & IN_BACK ||
		!(TrInput & IN_DIRECTION) && info->Control.JumpDirection == JumpDirection::Back) &&
		TestLaraJumpBack(item, coll))
	{
		item->TargetState = LS_JUMP_BACK;
		info->Control.JumpDirection = JumpDirection::Back;
		return;
	}

	if ((TrInput & IN_LEFT ||
		!(TrInput & IN_DIRECTION) && info->Control.JumpDirection == JumpDirection::Left) &&
		TestLaraJumpLeft(item, coll))
	{
		item->TargetState = LS_JUMP_LEFT;
		info->Control.JumpDirection = JumpDirection::Left;
		return;
	}
	else if ((TrInput & IN_RIGHT ||
		!(TrInput & IN_DIRECTION) && info->Control.JumpDirection == JumpDirection::Right) &&
		TestLaraJumpRight(item, coll))
	{
		item->TargetState = LS_JUMP_RIGHT;
		info->Control.JumpDirection = JumpDirection::Right;
		return;
	}

	// No directional key pressed AND no directional lock; commit to jump up.
	if (TestLaraJumpUp(item, coll))
	{
		item->TargetState = LS_JUMP_UP;
		info->Control.JumpDirection = JumpDirection::Up;
		return;
	}

	item->TargetState = LS_IDLE;
	info->Control.JumpDirection = JumpDirection::None;
}

// State:		LS_JUMP_PREPARE (15)
// Collision:	lara_as_jump_prepare()
void lara_col_jump_prepare(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	info->Control.MoveAngle = item->Position.yRot;
	switch (info->Control.JumpDirection)
	{
	case JumpDirection::Back:
		info->Control.MoveAngle += ANGLE(180.0f);
		break;

	case JumpDirection::Left:
		info->Control.MoveAngle -= ANGLE(90.0f);
		break;

	case JumpDirection::Right:
		info->Control.MoveAngle += ANGLE(90.0f);
		break;

	default:
		break;
	}

	item->VerticalVelocity = 0; // TODO: Check this.
	coll->Setup.LowerFloorBound = TestEnvironment(ENV_FLAG_SWAMP, item) ? NO_LOWER_BOUND : STEPUP_HEIGHT;	// Security.
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsPit = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;		// Security.
	coll->Setup.FloorSlopeIsWall = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;	// Security.
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraFall(item, coll))
	{
		SetLaraFallState(item);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideState(item, coll);
		SetLaraLand(item, coll);
		return;
	}

	ShiftItem(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_JUMP_BACK (25)
// Collision:	lara_col_jump_back()
void lara_as_jump_back(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	info->Control.CanLook = false;
	Camera.targetAngle = ANGLE(135.0f);

	if (item->HitPoints <= 0)
	{
		if (TestLaraLand(item, coll))
		{
			item->TargetState = LS_DEATH;
			SetLaraLand(item, coll);
		}

		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_JUMP_TURN_MAX / 2)
			info->Control.TurnRate = -LARA_JUMP_TURN_MAX / 2;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_JUMP_TURN_MAX / 2)
			info->Control.TurnRate = LARA_JUMP_TURN_MAX / 2;
	}

	if (TestLaraLand(item, coll))
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0)
			item->TargetState = LS_DEATH;
		else
			item->TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		return;
	}

	if (TrInput & (IN_ROLL | IN_FORWARD))
	{
		item->TargetState = LS_JUMP_ROLL_180;
		return;
	}

	if (item->VerticalVelocity >= LARA_FREEFALL_SPEED)
	{
		item->TargetState = LS_FREEFALL;
		return;
	}

	item->TargetState = LS_JUMP_BACK;
}

// State:		LS_JUMP_BACK (25)
// Control:		lara_as_jump_back()
void lara_col_jump_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraJumpCollision(item, coll, item->Position.yRot + ANGLE(180.0f));
}

// State:		LS_JUMP_RIGHT (26)
// Collision:	lara_col_jump_right()
void lara_as_jump_right(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	info->Control.CanLook = false;

	if (item->HitPoints <= 0)
	{
		if (TestLaraLand(item, coll))
		{
			item->TargetState = LS_DEATH;
			SetLaraLand(item, coll);
		}

		return;
	}

	if (TestLaraLand(item, coll))
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0)
			item->TargetState = LS_DEATH;
		else
			item->TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		return;
	}

	// TODO: Core appears to have planned this feature. Add an animation to make it possible.
	/*if (TrInput & (IN_ROLL | IN_LEFT))
	{
		item->TargetState = LS_JUMP_ROLL_180;
		return;
	}*/

	if (item->VerticalVelocity >= LARA_FREEFALL_SPEED)
	{
		item->TargetState = LS_FREEFALL;
		return;
	}

	item->TargetState = LS_JUMP_RIGHT;
}

// State:		LS_JUMP_RIGHT (26)
// Control:		lara_as_jump_right()
void lara_col_jump_right(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraJumpCollision(item, coll, item->Position.yRot + ANGLE(90.0f));
}

// State:		LS_JUMP_LEFT (27)
// Collision:	lara_as_jump_left()
void lara_as_jump_left(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	info->Control.CanLook = false;

	if (item->HitPoints <= 0)
	{
		if (TestLaraLand(item, coll))
		{
			item->TargetState = LS_DEATH;
			SetLaraLand(item, coll);
		}

		return;
	}

	if (TestLaraLand(item, coll))
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0)
			item->TargetState = LS_DEATH;
		else
			item->TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		return;
	}

	// TODO: Core appears to have planned this feature. Add an animation to make it possible.
	/*if (TrInput & (IN_ROLL | IN_RIGHT))
	{
		item->TargetState = LS_JUMP_ROLL_180;
		return;
	}*/

	if (item->VerticalVelocity >= LARA_FREEFALL_SPEED)
	{
		item->TargetState = LS_FREEFALL;
		return;
	}

	item->TargetState = LS_JUMP_LEFT;
}

// State:		LS_JUMP_LEFT (27)
// Control:		lara_as_jump_left()
void lara_col_jump_left(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraJumpCollision(item, coll, item->Position.yRot - ANGLE(90.0f));
}

// State:		LS_JUMP_UP (28)
// Collision:	lara_col_jump_up()
void lara_as_jump_up(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	info->Control.CanLook = false;

	if (item->HitPoints <= 0)
	{
		if (TestLaraLand(item, coll))
		{
			item->TargetState = LS_DEATH;
			SetLaraLand(item, coll);
		}

		return;
	}

	if (TestLaraLand(item, coll))
	{
		item->TargetState = LS_IDLE;
		SetLaraLand(item, coll);
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		item->Velocity += 2;
		if (item->Velocity > 5)
			item->Velocity = 5;
	}
	else if (TrInput & IN_BACK)
	{
		item->Velocity -= 2;
		if (item->Velocity < -5)
			item->Velocity = -5;

		// TODO: Holding BACK + LEFT/RIGHT results in Lara flexing more.
		item->Position.xRot += std::min<short>(LARA_LEAN_RATE / 3, abs(ANGLE(item->Velocity) - item->Position.xRot) / 3);
		info->Control.ExtraHeadRot.yRot += (ANGLE(10.0f) - item->Position.zRot) / 3;
	}
	else
		item->Velocity = item->Velocity <= 0 ? -2 : 2;

	if (item->VerticalVelocity >= LARA_FREEFALL_SPEED)
	{
		item->TargetState = LS_FREEFALL;
		return;
	}

	item->TargetState = LS_JUMP_UP;
}

// State:		LS_JUMP_UP (28)
// Control:		lara_as_jump_up()
void lara_col_jump_up(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	info->Control.MoveAngle = item->Position.yRot;
	coll->Setup.Height = LARA_HEIGHT_STRETCH;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = (item->Velocity >= 0) ? info->Control.MoveAngle : info->Control.MoveAngle + ANGLE(180.0f);
	coll->Setup.Mode = CollProbeMode::FreeForward;
	GetCollisionInfo(coll, item);

	if (TestLaraHangJumpUp(item, coll))
		return;

	if (TestLaraSlide(item, coll) && TestLaraLand(item, coll))
	{
		SetLaraSlideState(item, coll);
		SetLaraLand(item, coll);
		return;
	}

	if (coll->Middle.Ceiling >= 0 ||
		coll->CollisionType == CT_TOP ||
		coll->CollisionType == CT_TOP_FRONT ||
		coll->CollisionType == CT_CLAMP)
	{
		item->VerticalVelocity = 1;
	}

	ShiftItem(item, coll);
}

// State:		LS_FALL_BACK (29)
// Collision:	lara_col_fall_back()
void lara_as_fall_back(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		if (TestLaraLand(item, coll))
		{
			item->TargetState = LS_DEATH;
			SetLaraLand(item, coll);
		}

		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_JUMP_TURN_MAX / 2)
			info->Control.TurnRate = -LARA_JUMP_TURN_MAX / 2;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_JUMP_TURN_MAX / 2)
			info->Control.TurnRate = LARA_JUMP_TURN_MAX / 2;
	}

	if (TestLaraLand(item, coll))
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0)
			item->TargetState = LS_DEATH;
		else
			item->TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		return;
	}

	if (TrInput & IN_ACTION &&
		info->Control.HandStatus == HandStatus::Free)
	{
		item->TargetState = LS_REACH;
		return;
	}

	if (item->VerticalVelocity >= LARA_FREEFALL_SPEED)
	{
		item->TargetState = LS_FREEFALL;
		return;
	}

	item->TargetState = LS_FALL_BACK;
}

// State:		LS_FALL_BACK (29)
// Collision:	lara_col_fall_back()
void lara_col_fall_back(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_jump_back(item, coll);
}

// State:		LS_SWAN_DIVE (52)
// Collision:	lara_col_swan_dive()
void lara_as_swan_dive(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);

	info->Control.HandStatus = HandStatus::Busy;
	info->Control.CanLook = false;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpasm = false;

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_JUMP_TURN_MAX)
			info->Control.TurnRate = -LARA_JUMP_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE / 2);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_JUMP_TURN_MAX)
			info->Control.TurnRate = LARA_JUMP_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE / 2);
	}

	if (item->VerticalVelocity >= LARA_FREEFALL_SPEED)
	{
		item->TargetState = LS_FREEFALL_DIVE;
		return;
	}

	item->TargetState = LS_SWAN_DIVE;
}

// State:		LS_SWAN_DIVE (52)
// Control:		lara_as_swan_dive()
void lara_col_swan_dive(ITEM_INFO* item, COLL_INFO* coll)
{
	auto info = GetLaraInfo(item);
	auto bounds = GetBoundsAccurate(item);
	int realHeight = bounds->Y2 - bounds->Y1;

	info->Control.MoveAngle = item->Position.yRot;
	info->Control.KeepLow = TestLaraKeepLow(item, coll);
	coll->Setup.Height = std::max<int>(LARA_HEIGHT_CRAWL, realHeight * 0.7f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeJump(item, coll))
		info->Control.HandStatus = HandStatus::Free;

	if (coll->Middle.Floor <= 0 && item->VerticalVelocity > 0)
	{
		auto probe = GetCollisionResult(item, coll->Setup.ForwardAngle, coll->Setup.Radius, -coll->Setup.Height);

		if (TestLaraSlide(item, coll))
			SetLaraSlideState(item, coll);
		else if (info->Control.KeepLow ||
			abs(probe.Position.Ceiling - probe.Position.Floor) < LARA_HEIGHT &&
			g_GameFlow->Animations.CrawlspaceSwandive)
		{
			SetAnimation(item, LA_SPRINT_TO_CROUCH_LEFT, 10);

			if (!info->Control.KeepLow) // HACK: If Lara landed on the edge, shift forward to avoid standing up or falling out.
				MoveItem(item, coll->Setup.ForwardAngle, CLICK(0.5f));
		}
		else [[likely]]
			SetAnimation(item, LA_SWANDIVE_ROLL);

		SetLaraLand(item, coll);
		info->Control.HandStatus = HandStatus::Free;

		LaraSnapToHeight(item, coll);
	}
}

// State:		LS_FREEFALL_DIVE (53)
// Collision:	lara_col_freefall_dive()
void lara_as_freefall_dive(ITEM_INFO* item, COLL_INFO* coll)
{
	item->Velocity = item->Velocity * 0.95f;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpasm = false;

	if (TestLaraLand(item, coll))
	{
		DoLaraFallDamage(item);	// Should never occur before fall speed reaches death speed, but here for extendability.

		if (item->HitPoints <= 0 || item->VerticalVelocity >= LARA_FREEFALL_DIVE_DEATH_SPEED)
			item->TargetState = LS_DEATH;
		else
			item->TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		return;
	}

	if (TrInput & IN_ROLL)
	{
		item->TargetState = LS_JUMP_ROLL_180; // TODO: New state?
		return;
	}

	item->TargetState = LS_FREEFALL_DIVE;
}

// State:		LS_FREEFALL_DIVE (53)
// Control:		lara_as_freefall_dive()
void lara_col_freefall_dive(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_jump_forward(item, coll);
}
