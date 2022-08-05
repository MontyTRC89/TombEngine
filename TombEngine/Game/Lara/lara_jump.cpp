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
#include "Flow/ScriptInterfaceFlowHandler.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Input;

// -----------------------------
// JUMP
// Control & Collision Functions
// -----------------------------

// TODO: Unused? Naming is also completely mismatched.
// State:		LS_GRAB_TO_FALL
// Collision:	lara_void_func()
void lara_col_land(ItemInfo* item, CollisionInfo* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_JUMP_FORWARD (3)
// Collision:	lara_col_jump_forward()
void lara_as_jump_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	// Update running jump counter in preparation for possible jump action soon after landing.
	lara->Control.Count.Run++;
	if (lara->Control.Count.Run > LARA_RUN_JUMP_TIME / 2)
		lara->Control.Count.Run = LARA_RUN_JUMP_TIME / 2;

	if (item->HitPoints <= 0)
	{
		if (TestLaraLand(item, coll))
		{
			item->Animation.TargetState = LS_DEATH;
			SetLaraLand(item, coll);
		}

		return;
	}

	if (TrInput & (IN_LEFT | IN_RIGHT))
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_JUMP_TURN_RATE_MAX);

	if (TestLaraLand(item, coll))
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0) USE_FEATURE_IF_CPP20([[unlikely]])
			item->Animation.TargetState = LS_DEATH;
		else if (TestLaraSlide(item, coll))
			SetLaraSlideAnimation(item, coll);
		else if (TrInput & IN_FORWARD && !(TrInput & IN_WALK) &&
			lara->Control.WaterStatus != WaterStatus::Wade)
		{
			item->Animation.TargetState = LS_RUN_FORWARD;
		}
		else
			item->Animation.TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		return;
	}

	if (item->Animation.VerticalVelocity >= LARA_FREEFALL_VELOCITY)
	{
		item->Animation.TargetState = LS_FREEFALL;
		return;
	}

	if (TrInput & IN_ACTION &&
		lara->Control.HandStatus == HandStatus::Free)
	{
		item->Animation.TargetState = LS_REACH;
		return;
	}

	if (TrInput & IN_ROLL || TrInput & IN_BACK)
	{
		item->Animation.TargetState = LS_JUMP_ROLL_180;
		return;
	}

	if (TrInput & IN_WALK &&
		lara->Control.HandStatus == HandStatus::Free)
	{
		item->Animation.TargetState = LS_SWAN_DIVE;
		return;
	}

	item->Animation.TargetState = LS_JUMP_FORWARD;
}

// State:		LS_JUMP_FORWARD (3)
// Control:		lara_as_jump_forward()
void lara_col_jump_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = (item->Animation.Velocity > 0) ? item->Pose.Orientation.y : item->Pose.Orientation.y + ANGLE(180.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	LaraDeflectEdgeJump(item, coll);

	// TODO: Why??
	lara->Control.MoveAngle = (item->Animation.Velocity < 0) ? item->Pose.Orientation.y : lara->Control.MoveAngle;
}

// State:		LS_FREEFALL (9)
// Collision:	lara_col_freefall()
void lara_as_freefall(ItemInfo* item, CollisionInfo* coll)
{
	item->Animation.Velocity = item->Animation.Velocity * 0.95f;

	ModulateLaraTurnRateY(item, 0, 0, 0);

	if (item->Animation.VerticalVelocity == LARA_DEATH_VELOCITY &&
		item->HitPoints > 0)
	{
		SoundEffect(SFX_TR4_LARA_FALL, &item->Pose);
	}

	if (TestLaraLand(item, coll))
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0)
			item->Animation.TargetState = LS_DEATH;
		else if (TestLaraSlide(item, coll))
			SetLaraSlideAnimation(item, coll);
		else USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		StopSoundEffect(SFX_TR4_LARA_FALL);
		return;
	}

	item->Animation.TargetState = LS_FREEFALL;
}

// State:		LS_FREEFALL (9)
// Control:		lara_as_freefall()
void lara_col_freefall(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	item->Animation.IsAirborne = true;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	LaraSlideEdgeJump(item, coll);
}

// State:		LS_REACH (11)
// Collision:	lara_col_reach()
void lara_as_reach(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	Camera.targetAngle = ANGLE(85.0f);

	if (item->HitPoints <= 0)
	{
		if (TestLaraLand(item, coll))
		{
			item->Animation.TargetState = LS_DEATH;
			SetLaraLand(item, coll);
		}

		return;
	}

	if (TrInput & (IN_LEFT | IN_RIGHT))
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_JUMP_TURN_RATE_MAX / 2);

	if (TestLaraLand(item, coll))
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0)
			item->Animation.TargetState = LS_DEATH;
		else if (TestLaraSlide(item, coll))
			SetLaraSlideAnimation(item, coll);
		else USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		return;
	}

	if (item->Animation.VerticalVelocity >= LARA_FREEFALL_VELOCITY)
	{
		item->Animation.TargetState = LS_FREEFALL;
		return;
	}

	item->Animation.TargetState = LS_REACH;
}

// State:		LS_REACH (11)
// Control:		lara_as_reach()
void lara_col_reach(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.Rope.Ptr == -1)
		item->Animation.IsAirborne = true;

	lara->Control.MoveAngle = item->Pose.Orientation.y;

	// HACK: height is altered according to VerticalVelocity to fix "issues" with physically impossible
	// 6-click high ceiling running jumps. While TEN model is physically correct, original engines
	// allowed certain margin of deflection due to bug caused by hacky inclusion of headroom in coll checks.

	coll->Setup.Height = item->Animation.VerticalVelocity > 0 ? LARA_HEIGHT_REACH : LARA_HEIGHT;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = 0;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	coll->Setup.Radius = coll->Setup.Radius * 1.2f;
	coll->Setup.Mode = CollisionProbeMode::FreeForward;
	GetCollisionInfo(coll, item);

	// Overhang hook.
	SlopeReachExtra(item, coll);

	if (TestLaraHangJump(item, coll))
		return;

	LaraSlideEdgeJump(item, coll);

	GetCollisionInfo(coll, item);
	ShiftItem(item, coll);
}

// State:		LS_JUMP_PREPARE (15)
// Collision:	lara_col_jump_prepare()
void lara_as_jump_prepare(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	// TODO: I need to revise the directional jump system to work with changes done for OIS. @Sezz 2022.07.05
	ModulateLaraTurnRateY(item, 0, 0, 0);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_IDLE;
		return;
	}

	// JUMP key repressed without directional key; cancel directional jump lock.
	if (DbInput & IN_JUMP && !(TrInput & IN_DIRECTION))
		lara->Control.JumpDirection = JumpDirection::None;

	if (((TrInput & IN_FORWARD &&
			!(TrInput & IN_BACK && lara->Control.JumpDirection == JumpDirection::Back)) ||	// Back jump takes priority in this exception.
		!(TrInput & IN_DIRECTION) && lara->Control.JumpDirection == JumpDirection::Forward) &&
		TestLaraJumpForward(item, coll))
	{
		item->Animation.TargetState = LS_JUMP_FORWARD;
		lara->Control.JumpDirection = JumpDirection::Forward;
		return;
	}
	else if ((TrInput & IN_BACK ||
		!(TrInput & IN_DIRECTION) && lara->Control.JumpDirection == JumpDirection::Back) &&
		TestLaraJumpBack(item, coll))
	{
		item->Animation.TargetState = LS_JUMP_BACK;
		lara->Control.JumpDirection = JumpDirection::Back;
		return;
	}

	if ((TrInput & IN_LEFT ||
		!(TrInput & IN_DIRECTION) && lara->Control.JumpDirection == JumpDirection::Left) &&
		TestLaraJumpLeft(item, coll))
	{
		item->Animation.TargetState = LS_JUMP_LEFT;
		lara->Control.JumpDirection = JumpDirection::Left;
		return;
	}
	else if ((TrInput & IN_RIGHT ||
		!(TrInput & IN_DIRECTION) && lara->Control.JumpDirection == JumpDirection::Right) &&
		TestLaraJumpRight(item, coll))
	{
		item->Animation.TargetState = LS_JUMP_RIGHT;
		lara->Control.JumpDirection = JumpDirection::Right;
		return;
	}

	// No directional key pressed AND no directional lock; commit to jump up.
	if (TestLaraJumpUp(item, coll))
	{
		item->Animation.TargetState = LS_JUMP_UP;
		lara->Control.JumpDirection = JumpDirection::Up;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
	lara->Control.JumpDirection = JumpDirection::None;
}

// State:		LS_JUMP_PREPARE (15)
// Collision:	lara_as_jump_prepare()
void lara_col_jump_prepare(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.MoveAngle = item->Pose.Orientation.y;
	switch (lara->Control.JumpDirection)
	{
	case JumpDirection::Back:
		lara->Control.MoveAngle += ANGLE(180.0f);
		break;

	case JumpDirection::Left:
		lara->Control.MoveAngle -= ANGLE(90.0f);
		break;

	case JumpDirection::Right:
		lara->Control.MoveAngle += ANGLE(90.0f);
		break;

	default:
		break;
	}
	
	coll->Setup.LowerFloorBound = isSwamp ? NO_LOWER_BOUND : STEPUP_HEIGHT;	// Security.
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeDown = !isSwamp;	// Security.
	coll->Setup.BlockFloorSlopeUp = !isSwamp;	// Security.
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraFall(item, coll))
	{
		SetLaraFallAnimation(item);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideAnimation(item, coll);
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
void lara_as_jump_back(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	Camera.targetAngle = ANGLE(135.0f);

	if (item->HitPoints <= 0)
	{
		if (TestLaraLand(item, coll))
		{
			item->Animation.TargetState = LS_DEATH;
			SetLaraLand(item, coll);
		}

		return;
	}

	if (TrInput & (IN_LEFT | IN_RIGHT))
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_JUMP_TURN_RATE_MAX);

	if (TestLaraLand(item, coll))
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0)
			item->Animation.TargetState = LS_DEATH;
		else if (TestLaraSlide(item, coll))
			SetLaraSlideAnimation(item, coll);
		else USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		return;
	}

	if (item->Animation.VerticalVelocity >= LARA_FREEFALL_VELOCITY)
	{
		item->Animation.TargetState = LS_FREEFALL;
		return;
	}

	if (TrInput & IN_ROLL || TrInput & IN_FORWARD)
	{
		item->Animation.TargetState = LS_JUMP_ROLL_180;
		return;
	}

	item->Animation.TargetState = LS_JUMP_BACK;
}

// State:		LS_JUMP_BACK (25)
// Control:		lara_as_jump_back()
void lara_col_jump_back(ItemInfo* item, CollisionInfo* coll)
{
	LaraJumpCollision(item, coll, item->Pose.Orientation.y + ANGLE(180.0f));
}

// State:		LS_JUMP_RIGHT (26)
// Collision:	lara_col_jump_right()
void lara_as_jump_right(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;

	if (item->HitPoints <= 0)
	{
		if (TestLaraLand(item, coll))
		{
			item->Animation.TargetState = LS_DEATH;
			SetLaraLand(item, coll);
		}

		return;
	}

	if (TestLaraLand(item, coll))
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0)
			item->Animation.TargetState = LS_DEATH;
		else if (TestLaraSlide(item, coll))
			SetLaraSlideAnimation(item, coll);
		else USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		return;
	}

	if (item->Animation.VerticalVelocity >= LARA_FREEFALL_VELOCITY)
	{
		item->Animation.TargetState = LS_FREEFALL;
		return;
	}

	// TODO: It appears Core planned this feature. Add animations to make it possible.
	/*if (TrInput & (IN_ROLL | IN_LEFT))
	{
		item->TargetState = LS_JUMP_ROLL_180;
		return;
	}*/

	item->Animation.TargetState = LS_JUMP_RIGHT;
}

// State:		LS_JUMP_RIGHT (26)
// Control:		lara_as_jump_right()
void lara_col_jump_right(ItemInfo* item, CollisionInfo* coll)
{
	LaraJumpCollision(item, coll, item->Pose.Orientation.y + ANGLE(90.0f));
}

// State:		LS_JUMP_LEFT (27)
// Collision:	lara_as_jump_left()
void lara_as_jump_left(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;

	if (item->HitPoints <= 0)
	{
		if (TestLaraLand(item, coll))
		{
			item->Animation.TargetState = LS_DEATH;
			SetLaraLand(item, coll);
		}

		return;
	}

	if (TestLaraLand(item, coll))
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0)
			item->Animation.TargetState = LS_DEATH;
		else if (TestLaraSlide(item, coll))
			SetLaraSlideAnimation(item, coll);
		else USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		return;
	}

	if (item->Animation.VerticalVelocity >= LARA_FREEFALL_VELOCITY)
	{
		item->Animation.TargetState = LS_FREEFALL;
		return;
	}

	// TODO: It appears Core planned this feature. Add animations to make it possible.
	/*if (TrInput & (IN_ROLL | IN_RIGHT))
	{
		item->TargetState = LS_JUMP_ROLL_180;
		return;
	}*/

	item->Animation.TargetState = LS_JUMP_LEFT;
}

// State:		LS_JUMP_LEFT (27)
// Control:		lara_as_jump_left()
void lara_col_jump_left(ItemInfo* item, CollisionInfo* coll)
{
	LaraJumpCollision(item, coll, item->Pose.Orientation.y - ANGLE(90.0f));
}

// State:		LS_JUMP_UP (28)
// Collision:	lara_col_jump_up()
void lara_as_jump_up(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;

	if (item->HitPoints <= 0)
	{
		if (TestLaraLand(item, coll))
		{
			item->Animation.TargetState = LS_DEATH;
			SetLaraLand(item, coll);
		}

		return;
	}

	if (TestLaraLand(item, coll))
	{
		if (item->HitPoints <= 0)
			item->Animation.TargetState = LS_DEATH;
		else if (TestLaraSlide(item, coll))
			SetLaraSlideAnimation(item, coll);
		else USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		return;
	}

	if (item->Animation.VerticalVelocity >= LARA_FREEFALL_VELOCITY)
	{
		item->Animation.TargetState = LS_FREEFALL;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		item->Animation.Velocity += 2;
		if (item->Animation.Velocity > 5)
			item->Animation.Velocity = 5;
	}
	else if (TrInput & IN_BACK)
	{
		item->Animation.Velocity -= 2;
		if (item->Animation.Velocity < -5)
			item->Animation.Velocity = -5;
	}
	else
		item->Animation.Velocity = (item->Animation.Velocity < 0) ? -2 : 2;

	if (item->Animation.Velocity < 0)
	{
		// TODO: Holding BACK + LEFT/RIGHT results in Lara flexing more.
		item->Pose.Orientation.x += std::min<short>(LARA_LEAN_RATE / 3, abs(ANGLE(item->Animation.Velocity) - item->Pose.Orientation.x) / 3);
		lara->ExtraHeadRot.y += (ANGLE(10.0f) - item->Pose.Orientation.z) / 3;
	}

	item->Animation.TargetState = LS_JUMP_UP;
}

// State:		LS_JUMP_UP (28)
// Control:		lara_as_jump_up()
void lara_col_jump_up(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.Height = LARA_HEIGHT_STRETCH;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = (item->Animation.Velocity >= 0) ? lara->Control.MoveAngle : lara->Control.MoveAngle + ANGLE(180.0f);
	coll->Setup.Mode = CollisionProbeMode::FreeForward;
	GetCollisionInfo(coll, item);

	if (TestLaraHangJumpUp(item, coll))
		return;

	if (coll->Middle.Ceiling >= 0 ||
		coll->CollisionType == CT_TOP ||
		coll->CollisionType == CT_TOP_FRONT ||
		coll->CollisionType == CT_CLAMP)
	{
		item->Animation.VerticalVelocity = 1;
	}

	ShiftItem(item, coll);
}

// State:		LS_FALL_BACK (29)
// Collision:	lara_col_fall_back()
void lara_as_fall_back(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		if (TestLaraLand(item, coll))
		{
			item->Animation.TargetState = LS_DEATH;
			SetLaraLand(item, coll);
		}

		return;
	}

	if (TrInput & (IN_LEFT | IN_RIGHT))
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_JUMP_TURN_RATE_MAX / 2);

	if (TestLaraLand(item, coll))
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0)
			item->Animation.TargetState = LS_DEATH;
		else if (TestLaraSlide(item, coll))
			SetLaraSlideAnimation(item, coll);
		else USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		return;
	}

	if (item->Animation.VerticalVelocity >= LARA_FREEFALL_VELOCITY)
	{
		item->Animation.TargetState = LS_FREEFALL;
		return;
	}

	if (TrInput & IN_ACTION &&
		lara->Control.HandStatus == HandStatus::Free)
	{
		item->Animation.TargetState = LS_REACH;
		return;
	}

	item->Animation.TargetState = LS_FALL_BACK;
}

// State:		LS_FALL_BACK (29)
// Collision:	lara_col_fall_back()
void lara_col_fall_back(ItemInfo* item, CollisionInfo* coll)
{
	lara_col_jump_back(item, coll);
}

// State:		LS_SWAN_DIVE (52)
// Collision:	lara_col_swan_dive()
void lara_as_swan_dive(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.HandStatus = HandStatus::Busy;
	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpasm = false;

	if (item->HitPoints <= 0)
	{
		if (TestLaraLand(item, coll))
		{
			item->Animation.TargetState = LS_DEATH;
			SetLaraLand(item, coll);
		}

		if (item->Animation.VerticalVelocity >= LARA_FREEFALL_VELOCITY)
		{
			item->Animation.TargetState = LS_FREEFALL_DIVE;
			return;
		}

		return;
	}

	if (TrInput & (IN_LEFT | IN_RIGHT))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_JUMP_TURN_RATE_MAX);
		ModulateLaraLean(item, coll, LARA_LEAN_RATE / 2, LARA_LEAN_MAX);
	}

	if (TestLaraLand(item, coll))
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0)
			item->Animation.TargetState = LS_DEATH;
		else if (TestLaraSlide(item, coll))
			SetLaraSlideAnimation(item, coll);
		else if ((TrInput & IN_CROUCH || TestLaraCrawlspaceDive(item, coll)) &&
			g_GameFlow->HasCrawlspaceSwandive())
		{
			item->Animation.TargetState = LS_CROUCH_IDLE;
			TranslateItem(item, coll->Setup.ForwardAngle, CLICK(0.5f)); // HACK: Move forward to avoid standing up or falling out on an edge.
		}
		else USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		lara->Control.HandStatus = HandStatus::Free;
		return;
	}

	if (item->Animation.VerticalVelocity >= LARA_FREEFALL_VELOCITY)
	{
		item->Animation.TargetState = LS_FREEFALL_DIVE;
		return;
	}

	item->Animation.TargetState = LS_SWAN_DIVE;
}

// State:		LS_SWAN_DIVE (52)
// Control:		lara_as_swan_dive()
void lara_col_swan_dive(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	auto* bounds = GetBoundsAccurate(item);
	int realHeight = g_GameFlow->HasCrawlspaceSwandive() ? ((bounds->Y2 - bounds->Y1) * 0.7f) : LARA_HEIGHT;

	lara->Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.Height = std::max(LARA_HEIGHT_CRAWL, realHeight);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (LaraDeflectEdgeJump(item, coll))
	{
		// Reset position to avoid embedding inside sloped ceilings meeting the floor.
		item->Pose.Position = coll->Setup.OldPosition;
		lara->Control.HandStatus = HandStatus::Free;
	}
}

// State:		LS_FREEFALL_DIVE (53)
// Collision:	lara_col_freefall_dive()
void lara_as_freefall_dive(ItemInfo* item, CollisionInfo* coll)
{
	item->Animation.Velocity = item->Animation.Velocity * 0.95f;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.EnableSpasm = false;

	ModulateLaraTurnRateY(item, 0, 0, 0);

	if (item->HitPoints <= 0)
	{
		if (TestLaraLand(item, coll))
		{
			item->Animation.TargetState = LS_DEATH;
			SetLaraLand(item, coll);
		}

		return;
	}

	if (TestLaraLand(item, coll))
	{
		if (item->Animation.VerticalVelocity >= LARA_DIVE_DEATH_VELOCITY ||
			item->HitPoints <= 0)
		{
			item->Animation.TargetState = LS_DEATH;
			Rumble(0.5f, 0.2f);
		}
		else if (TestLaraSlide(item, coll))
			SetLaraSlideAnimation(item, coll);
		else USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_IDLE;

		SetLaraLand(item, coll);
		return;
	}

	if (TrInput & IN_ROLL)
	{
		item->Animation.TargetState = LS_JUMP_ROLL_180;
		return;
	}

	item->Animation.TargetState = LS_FREEFALL_DIVE;
}

// State:		LS_FREEFALL_DIVE (53)
// Control:		lara_as_freefall_dive()
void lara_col_freefall_dive(ItemInfo* item, CollisionInfo* coll)
{
	lara_col_jump_forward(item, coll);
}
