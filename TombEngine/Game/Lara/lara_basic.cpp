#include "framework.h"
#include "Game/Lara/lara_basic.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/health.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_slide.h"
#include "Game/Lara/lara_monkey.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/pickup/pickup.h"
#include "Sound/sound.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Input;

// ------------------------------
// BASIC MOVEMENT & MISCELLANEOUS
// Control & Collision Functions
// ------------------------------

// --------------
// MISCELLANEOUS:
// --------------

void lara_void_func(ItemInfo* item, CollisionInfo* coll)
{
	return;
}

void lara_default_col(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeDown = true;
	coll->Setup.BlockFloorSlopeUp = true;
	coll->Setup.ForwardAngle = lara.Control.MoveAngle;
	GetCollisionInfo(coll, item);
	LaraResetGravityStatus(item, coll);
}

// Boulder death.
void lara_as_special(ItemInfo* item, CollisionInfo* coll)
{
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(170.0f);
	Camera.targetElevation = ANGLE(-25.0f);
}

void lara_as_null(ItemInfo* item, CollisionInfo* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
}

void lara_as_controlled(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.flags = CF_FOLLOW_CENTER;

	if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd - 1)
	{
		lara.Control.HandStatus = HandStatus::Free;

		if (UseForcedFixedCamera)
			UseForcedFixedCamera = 0;
	}
}

void lara_as_controlled_no_look(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
}

// State:	  LS_VAULT (164)
// Collision: lara_void_func()
void lara_as_vault(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	EaseOutLaraHeight(item, lara.ProjectedFloorHeight - item->Pose.Position.y);
	item->Pose.Orientation.Lerp(lara.TargetOrientation, 0.4f);

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_AUTO_JUMP (62)
// Collision:	lara_as_jump_prepare()
void lara_as_auto_jump(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	item->Pose.Orientation.Lerp(lara.TargetOrientation, 0.4f);
}

// ---------------
// BASIC MOVEMENT:
// ---------------

// State:		LS_WALK_FORWARD (0)
// Collision:	lara_col_walk_forward()
void lara_as_walk_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.Count.Run++;
	if (lara.Control.Count.Run > (LARA_RUN_JUMP_TIME / 2 + 4))
		lara.Control.Count.Run = LARA_RUN_JUMP_TIME / 2 + 4;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	// HACK: Interaction alignment.
	if (lara.Control.IsMoving)
	{
		ResetLaraTurnRateY(item);
		return;
	}

	if (IsHeld(In::Left) || IsHeld(In::Right))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_MED_TURN_RATE_MAX);
		ModulateLaraLean(item, coll, LARA_LEAN_RATE / 6, LARA_LEAN_MAX / 2);
	}

	if (IsHeld(In::Forward))
	{
		auto vaultContext = TestLaraVault(item, coll);
		if (IsHeld(In::Action) && vaultContext.Success)
		{
			item->Animation.TargetState = vaultContext.TargetState;
			SetLaraVault(item, coll, vaultContext);
			return;
		}
		
		if (lara.Control.WaterStatus == WaterStatus::Wade)
		{
			item->Animation.TargetState = LS_WADE_FORWARD;
		}
		else if (IsHeld(In::Walk))
		{
			item->Animation.TargetState = LS_WALK_FORWARD;
		}
		else
			item->Animation.TargetState = LS_RUN_FORWARD;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LA_WALK_FORWARD (0)
// Control:		lara_as_walk_forward()
void lara_col_walk_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.MoveAngle = item->Pose.Orientation.y;
	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeUp = true;
	coll->Setup.BlockFloorSlopeDown = true;
	coll->Setup.BlockDeathFloorDown = true;
	coll->Setup.ForwardAngle = lara.Control.MoveAngle;
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
		return;
	}

	if (TestAndDoLaraLadderClimb(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		item->Animation.TargetState = LS_SOFT_SPLAT;
		if (GetStateDispatch(item, g_Level.Anims[item->Animation.AnimNumber]))
		{
			item->Animation.ActiveState = LS_SOFT_SPLAT;
			return;
		}

		LaraCollideStop(item, coll);
	}

	if (TestLaraStep(item, coll) && coll->CollisionType != CT_FRONT)
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_RUN_FORWARD (1)
// Collision:	lara_col_run_forward()
void lara_as_run_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.Count.Run++;
	if (lara.Control.Count.Run > LARA_RUN_JUMP_TIME)
		lara.Control.Count.Run = LARA_RUN_JUMP_TIME;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (IsHeld(In::Left) || IsHeld(In::Right))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_FAST_TURN_RATE_MAX);
		ModulateLaraLean(item, coll, LARA_LEAN_RATE, LARA_LEAN_MAX);
	}

	if (IsHeld(In::Jump) || lara.Control.RunJumpQueued)
	{
		if (!IsHeld(In::Sprint) && Context::CanRunJumpForward(item, coll))
		{
			item->Animation.TargetState = LS_JUMP_FORWARD;
			return;
		}

		SetLaraRunJumpQueue(item, coll);
	}

	if ((IsHeld(In::Roll) || (IsHeld(In::Forward) && IsHeld(In::Back))) &&
		lara.Control.WaterStatus != WaterStatus::Wade &&
		!lara.Control.RunJumpQueued) // Jump queue blocks 180 roll.
	{
		item->Animation.TargetState = LS_ROLL_180_FORWARD;
		return;
	}

	if (IsHeld(In::Crouch) && Context::CanCrouch(item, coll))
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (IsHeld(In::Forward))
	{
		auto vaultContext = TestLaraVault(item, coll);
		if (IsHeld(In::Action) && vaultContext.Success)
		{
			item->Animation.TargetState = vaultContext.TargetState;
			SetLaraVault(item, coll, vaultContext);
			return;
		}
		
		if (lara.Control.WaterStatus == WaterStatus::Wade)
		{
			item->Animation.TargetState = LS_WADE_FORWARD;
		}
		else if (IsHeld(In::Walk))
		{
			item->Animation.TargetState = LS_WALK_FORWARD;
		}
		else if (IsHeld(In::Sprint) && lara.SprintEnergy)
		{
			item->Animation.TargetState = LS_SPRINT;
		}
		else
			item->Animation.TargetState = LS_RUN_FORWARD;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_RUN_FORWARD (1)
// Control:		lara_as_run_forward()
void lara_col_run_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.MoveAngle = item->Pose.Orientation.y;
	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeUp = true;
	coll->Setup.ForwardAngle = lara.Control.MoveAngle;
	GetCollisionInfo(coll, item);
	LaraResetGravityStatus(item, coll);

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
		return;
	}

	if (TestAndDoLaraLadderClimb(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		ResetLaraLean(item);

		if (TestLaraWall(item, OFFSET_RADIUS(coll->Setup.Radius), -CLICK(2.5f)) ||
			coll->HitTallObject)
		{
			item->Animation.TargetState = LS_SPLAT;
			if (GetStateDispatch(item, g_Level.Anims[item->Animation.AnimNumber]))
			{
				Rumble(0.4f, 0.15f);

				item->Animation.ActiveState = LS_SPLAT;
				return;
			}
		}

		item->Animation.TargetState = LS_SOFT_SPLAT;
		if (GetStateDispatch(item, g_Level.Anims[item->Animation.AnimNumber]))
		{
			item->Animation.ActiveState = LS_SOFT_SPLAT;
			return;
		}

		LaraCollideStop(item, coll);
	}

	if (TestLaraStep(item, coll) && coll->CollisionType != CT_FRONT)
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_IDLE (2), LS_SPLAT_SOFT (170)
// Collision:	lara_col_idle()
void lara_as_idle(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara.Control.CanLook = ((isSwamp && lara.Control.WaterStatus == WaterStatus::Wade) || item->Animation.AnimNumber == LA_SWANDIVE_ROLL) ? false : true;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	// HACK: Interaction alignment.
	if (lara.Control.IsMoving)
	{
		ResetLaraTurnRateY(item);
		return;
	}

	// TODO: Handle waterskin and mechanical scarab.
	if (UseSpecialItem(item))
		return;

	if (IsHeld(In::Look) && lara.Control.CanLook)
		LookUpDown(item);

	// HACK: Handle binoculars.
	if (BinocularOn)
		return;

	// Jump locks orientation. TODO: Check over this.
	if (!IsHeld(In::Jump))
	{
		// Sidestep locks orientation.
		if ((IsHeld(In::LeftStep) || (IsHeld(In::Walk) && IsHeld(In::Left))) ||
			(IsHeld(In::RightStep) || (IsHeld(In::Walk) && IsHeld(In::Right))))
		{
			ResetLaraTurnRateY(item);
		}
		else if (IsHeld(In::Left) || IsHeld(In::Right))
			ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_MED_TURN_RATE_MAX);
	}

	if (IsHeld(In::Jump) && Context::CanPerformJump(item, coll))
	{
		SetLaraJumpDirection(item, coll);
		if (lara.Control.JumpDirection != JumpDirection::None)
			item->Animation.TargetState = LS_JUMP_PREPARE;

		return;
	}

	if (IsHeld(In::Roll) || (IsHeld(In::Forward) && IsHeld(In::Back)))
	{
		if (IsHeld(In::Walk) || Context::CanTurn180(item, coll))
			item->Animation.TargetState = LS_TURN_180;
		else
			item->Animation.TargetState = LS_ROLL_180_FORWARD;
		
		return;
	}

	if (IsHeld(In::Crouch) && Context::CanCrouch(item, coll))
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (IsHeld(In::Forward))
	{
		auto vaultContext = TestLaraVault(item, coll);
		if (IsHeld(In::Action) && vaultContext.Success)
		{
			item->Animation.TargetState = vaultContext.TargetState;
			SetLaraVault(item, coll, vaultContext);
			return;
		}

		if (Context::CanWadeForward(item, coll))
		{
			item->Animation.TargetState = LS_WADE_FORWARD;
			return;
		}
		else if (IsHeld(In::Walk))
		{
			if (Context::CanWalkForward(item, coll))
			{
				item->Animation.TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (Context::CanRunForward(item, coll))
		{
			if (IsHeld(In::Sprint))
				item->Animation.TargetState = LS_SPRINT;
			else
				item->Animation.TargetState = LS_RUN_FORWARD;

			return;
		}
	}
	else if (IsHeld(In::Back))
	{
		if (Context::CanWadeBackward(item, coll))
		{
			item->Animation.TargetState = LS_WALK_BACK;
			return;
		}
		else if (IsHeld(In::Walk))
		{
			if (Context::CanWalkBackward(item, coll))
			{
				item->Animation.TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (Context::CanRunBackward(item, coll))
		{
			item->Animation.TargetState = LS_RUN_BACK;
			return;
		}
	}

	if (IsHeld(In::LeftStep) || (IsHeld(In::Walk) && IsHeld(In::Left)))
	{
		if (Context::CanSidestepLeft(item, coll))
			item->Animation.TargetState = LS_STEP_LEFT;
		else
			item->Animation.TargetState = LS_IDLE;

		return;
	}
	else if (IsHeld(In::RightStep) || (IsHeld(In::Walk) && IsHeld(In::Right)))
	{
		if (Context::CanSidestepRight(item, coll))
			item->Animation.TargetState = LS_STEP_RIGHT;
		else
			item->Animation.TargetState = LS_IDLE;

		return;
	}

	if (IsHeld(In::Left))
	{
		if ((IsHeld(In::Sprint) || lara.Control.TurnRate.y <= -LARA_SLOW_TURN_RATE_MAX || Context::CanTurnFast(item, coll)) &&
			lara.Control.WaterStatus != WaterStatus::Wade)
		{
			item->Animation.TargetState = LS_TURN_LEFT_FAST;
		}
		else
			item->Animation.TargetState = LS_TURN_LEFT_SLOW;

		return;
	}
	else if (IsHeld(In::Right))
	{
		if ((IsHeld(In::Sprint) || lara.Control.TurnRate.y >= LARA_SLOW_TURN_RATE_MAX || Context::CanTurnFast(item, coll)) &&
			lara.Control.WaterStatus != WaterStatus::Wade)
		{
			item->Animation.TargetState = LS_TURN_RIGHT_FAST;
		}
		else
			item->Animation.TargetState = LS_TURN_RIGHT_SLOW;

		return;
	}

	// TODO: Without animation blending, the AFK state's movement lock will be rather obnoxious.
	// TODO: Add idle breathing motion. -- Sezz 2021.10.31
	if (Context::CanAFKPose(item, coll))
	{
		item->Animation.TargetState = LS_POSE;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_IDLE (2), LS_POSE (4), LS_SPLAT_SOFT (170)
// Control:		lara_as_idle(), lara_as_pose()
void lara_col_idle(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	lara.Control.MoveAngle = (item->Animation.Velocity.z >= 0) ? item->Pose.Orientation.y : (item->Pose.Orientation.y + ANGLE(180.0f));
	coll->Setup.LowerFloorBound = isSwamp ? NO_LOWER_BOUND : STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeDown = !isSwamp;
	coll->Setup.BlockFloorSlopeUp = !isSwamp;
	coll->Setup.ForwardAngle = lara.Control.MoveAngle;
	GetCollisionInfo(coll, item);

	// TODO: Better clamp handling. This can result in Lara standing above or below the floor. -- Sezz 2022.04.01
	/*if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}*/

	if (TestLaraFall(item, coll))
	{
		SetLaraFallAnimation(item);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideAnimation(item, coll);
		return;
	}

	if (TestAndDoLaraLadderClimb(item, coll))
		return;

	ShiftItem(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_POSE (4)
// Collision:	lara_col_idle()
void lara_as_pose(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (IsHeld(In::Look))
		LookUpDown(item);

	if (Context::CanAFKPose(item, coll))
	{
		if (IsHeld(In::Roll))
		{
			item->Animation.TargetState = LS_ROLL_180_FORWARD;
			return;
		}

		if (IsWakeActionHeld())
		{
			item->Animation.TargetState = LS_IDLE;
			return;
		}

		item->Animation.TargetState = LS_POSE;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_RUN_BACK (5)
// Collision:	lara_col_run_back()
void lara_as_run_back(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	if (IsHeld(In::Left) || IsHeld(In::Right))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_MED_TURN_RATE_MAX);
		ModulateLaraLean(item, coll, LARA_LEAN_RATE / 4, LARA_LEAN_MAX / 3);
	}

	if (IsHeld(In::Roll))
	{
		item->Animation.TargetState = LS_ROLL_180_FORWARD;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_RUN_BACK (5)
// Control:		lara_as_run_back()
void lara_col_run_back(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.MoveAngle = item->Pose.Orientation.y + ANGLE(180.0f);
	item->Animation.Velocity.y = 0;
	item->Animation.IsAirborne = false;
	coll->Setup.BlockFloorSlopeDown = true;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.ForwardAngle = lara.Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (coll->Middle.Floor > (STEPUP_HEIGHT / 2))
	{
		SetLaraFallBackAnimation(item);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideAnimation(item, coll);
		return;
	}

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_TURN_RIGHT_SLOW (6), LS_TURN_LEFT_SLOW (7)
// Collision:	lara_col_turn_slow()
void lara_as_turn_slow(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara.Control.CanLook = (isSwamp && lara.Control.WaterStatus == WaterStatus::Wade) ? false : true;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (lara.Control.WaterStatus == WaterStatus::Wade)
	{
		if (isSwamp)
			ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SWAMP_TURN_RATE_MAX);
		else
			ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_WADE_TURN_RATE_MAX);
	}
	else
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_MED_FAST_TURN_RATE_MAX);

	if (IsHeld(In::Jump) && Context::CanPerformJump(item, coll))
	{
		SetLaraJumpDirection(item, coll);
		if (lara.Control.JumpDirection != JumpDirection::None)
		{
			item->Animation.TargetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if ((IsHeld(In::Roll) || (IsHeld(In::Forward) && IsHeld(In::Back))) &&
		lara.Control.WaterStatus != WaterStatus::Wade)
	{
		item->Animation.TargetState = LS_ROLL_180_FORWARD;
		return;
	}

	if (IsHeld(In::Crouch) && Context::CanCrouch(item, coll))
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (IsHeld(In::Forward))
	{
		auto vaultContext = TestLaraVault(item, coll);
		if (IsHeld(In::Action) && vaultContext.Success)
		{
			item->Animation.TargetState = vaultContext.TargetState;
			SetLaraVault(item, coll, vaultContext);
			return;
		}

		if (Context::CanWadeForward(item, coll))
		{
			item->Animation.TargetState = LS_WADE_FORWARD;
			return;
		}
		else if (IsHeld(In::Walk))
		{
			if (Context::CanWalkForward(item, coll))
			{
				item->Animation.TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (Context::CanRunForward(item, coll))
		{
			if (IsHeld(In::Sprint))
				item->Animation.TargetState = LS_SPRINT;
			else
				item->Animation.TargetState = LS_RUN_FORWARD;

			return;
		}
	}
	else if (IsHeld(In::Back))
	{
		if (Context::CanWadeBackward(item, coll))
		{
			item->Animation.TargetState = LS_WALK_BACK;
			return;
		}
		else if (IsHeld(In::Walk))
		{
			if (Context::CanWalkBackward(item, coll))
			{
				item->Animation.TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (Context::CanRunBackward(item, coll))
		{
			item->Animation.TargetState = LS_RUN_BACK;
			return;
		}
	}

	// TODO: Swamps.
	if (IsHeld(In::LeftStep) || (IsHeld(In::Walk) && IsHeld(In::Left)) &&
		Context::CanSidestepLeft(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (IsHeld(In::RightStep) || (IsHeld(In::Walk) && IsHeld(In::Right)) &&
		Context::CanSidestepRight(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	if (IsHeld(In::Left))
	{
		// TODO: This hasn't worked since TR1.
		/*if (IsHeld(In::Walk))
		{
			item->Animation.TargetState = LS_TURN_LEFT_SLOW;

			if (lara.Control.TurnRate.y < -LARA_SLOW_TURN_RATE_MAX)
				lara.Control.TurnRate.y = -LARA_SLOW_TURN_RATE_MAX;
		}
		else */if (lara.Control.TurnRate.y < -LARA_SLOW_MED_TURN_RATE_MAX && lara.Control.WaterStatus != WaterStatus::Wade)
		{
			item->Animation.TargetState = LS_TURN_LEFT_FAST;
		}
		else
			item->Animation.TargetState = LS_TURN_LEFT_SLOW;

		return;
	}
	else if (IsHeld(In::Right))
	{
		// TODO: This hasn't worked since TR1.
		/*if (IsHeld(In::Walk))
		{
			item->Animation.TargetState = LS_TURN_RIGHT_SLOW;

			if (lara.Control.TurnRate > LARA_SLOW_TURN_RATE_MAX)
				lara.Control.TurnRate = LARA_SLOW_TURN_RATE_MAX;
		}
		else */if (lara.Control.TurnRate.y > LARA_SLOW_MED_TURN_RATE_MAX && lara.Control.WaterStatus != WaterStatus::Wade)
		{
			item->Animation.TargetState = LS_TURN_RIGHT_FAST;
		}
		else
			item->Animation.TargetState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_TURN_RIGHT_SLOW (6), LS_TURN_LEFT_SLOW (7)
// Control:		lara_as_turn_slow()
void lara_col_turn_slow(ItemInfo* item, CollisionInfo* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_DEATH (8)
// Collision:	lara_col_death()
void lara_as_death(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	item->Animation.Velocity.z = 0.0f;
	lara.Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	ResetLaraTurnRateY(item);

	if (BinocularRange)
	{
		BinocularRange = 0;
		LaserSight = false;
		AlterFOV(ANGLE(80.0f));
		item->MeshBits.SetAll();
		lara.Inventory.IsBusy = false;
	}

	auto bounds = GameBoundingBox(item);
	if (bounds.GetHeight() <= (LARA_HEIGHT * 0.75f))
		AlignLaraToSurface(item);
}

// State:		LS_DEATH (8)
// Control:		lara_as_death()
void lara_col_death(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	item->Animation.IsAirborne = false;
	lara.Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.Radius = LARA_RADIUS_DEATH;
	coll->Setup.ForwardAngle = lara.Control.MoveAngle;
	GetCollisionInfo(coll, item);

	StopSoundEffect(SFX_TR4_LARA_FALL);
	item->HitPoints = -1;
	lara.Air = -1;

	ShiftItem(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_SPLAT (12)
// Collision:	lara_col_splat()
void lara_as_splat(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.CanLook = false;
	ResetLaraTurnRateY(item);
}

// State:		LS_SPLAT (12)
// Control:		lara_as_splat()
void lara_col_splat(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.BlockFloorSlopeUp = true;
	coll->Setup.BlockFloorSlopeDown = true;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.ForwardAngle = lara.Control.MoveAngle;
	GetCollisionInfo(coll, item);

	ShiftItem(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_WALK_BACK (16)
// Collision:	lara_col_walk_back()
void lara_as_walk_back(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara.Control.CanLook = (isSwamp && lara.Control.WaterStatus == WaterStatus::Wade) ? false : true;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (lara.Control.IsMoving)
	{
		ResetLaraTurnRateY(item);
		return;
	}

	if (IsHeld(In::Left) || IsHeld(In::Right))
	{
		if (lara.Control.WaterStatus == WaterStatus::Wade && isSwamp)
		{
			ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX / 3);
			ModulateLaraLean(item, coll, LARA_LEAN_RATE / 3, LARA_LEAN_MAX / 3);
		}
		else
		{
			ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX);
			ModulateLaraLean(item, coll, LARA_LEAN_RATE / 4, LARA_LEAN_MAX / 3);
		}
	}

	if (IsHeld(In::Back) &&
		(IsHeld(In::Walk) || lara.Control.WaterStatus == WaterStatus::Wade))
	{
		item->Animation.TargetState = LS_WALK_BACK;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_WALK_BACK (16)
// Control:		lara_as_walk_back()
void lara_col_walk_back(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara.Control.MoveAngle = item->Pose.Orientation.y + ANGLE(180.0f);
	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	coll->Setup.LowerFloorBound = (lara.Control.WaterStatus == WaterStatus::Wade) ? NO_LOWER_BOUND : STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeDown = !isSwamp;
	coll->Setup.BlockFloorSlopeUp = !isSwamp;
	coll->Setup.BlockDeathFloorDown = true;
	coll->Setup.ForwardAngle = lara.Control.MoveAngle;
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
		return;
	}

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_TURN_RIGHT_FAST (20), LS_TURN_LEFT_FAST (152)
// Collision:	lara_col_turn_fast()
void lara_as_turn_fast(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, LARA_MED_TURN_RATE_MAX, LARA_FAST_TURN_RATE_MAX);

	if (IsHeld(In::Jump) && Context::CanPerformJump(item, coll))
	{
		SetLaraJumpDirection(item, coll);
		if (lara.Control.JumpDirection != JumpDirection::None)
		{
			item->Animation.TargetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if ((IsHeld(In::Roll) || (IsHeld(In::Forward) && IsHeld(In::Back))) &&
		lara.Control.WaterStatus != WaterStatus::Wade)
	{
		item->Animation.TargetState = LS_ROLL_180_FORWARD;
		return;
	}

	if (IsHeld(In::Crouch) && Context::CanCrouch(item, coll))
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (IsHeld(In::Forward))
	{
		auto vaultContext = TestLaraVault(item, coll);
		if (IsHeld(In::Action) && vaultContext.Success)
		{
			item->Animation.TargetState = vaultContext.TargetState;
			SetLaraVault(item, coll, vaultContext);
			return;
		}

		if (Context::CanWadeForward(item, coll))
		{
			item->Animation.TargetState = LS_WADE_FORWARD;
			return;
		}
		else if (IsHeld(In::Walk))
		{
			if (Context::CanWalkForward(item, coll))
			{
				item->Animation.TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (Context::CanRunForward(item, coll))
		{
			if (IsHeld(In::Sprint))
				item->Animation.TargetState = LS_SPRINT;
			else
				item->Animation.TargetState = LS_RUN_FORWARD;

			return;
		}
	}
	else if (IsHeld(In::Back))
	{
		if (Context::CanWadeBackward(item, coll))
		{
			item->Animation.TargetState = LS_WALK_BACK;
			return;
		}
		else if (IsHeld(In::Walk))
		{
			if (Context::CanWalkBackward(item, coll))
			{
				item->Animation.TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (Context::CanRunBackward(item, coll))
		{
			item->Animation.TargetState = LS_RUN_BACK;
			return;
		}
	}

	// TODO: Swamps.
	if (IsHeld(In::LeftStep) || (IsHeld(In::Walk) && IsHeld(In::Left)) &&
		Context::CanSidestepLeft(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (IsHeld(In::RightStep) || (IsHeld(In::Walk) && IsHeld(In::Right)) &&
		Context::CanSidestepRight(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	if (IsHeld(In::Left))
	{
		item->Animation.TargetState = LS_TURN_LEFT_FAST;
		return;
	}
	else if (IsHeld(In::Right))
	{
		item->Animation.TargetState = LS_TURN_RIGHT_FAST;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_TURN_RIGHT_FAST (20), LS_TURN_LEFT_FAST (152)
// Control:		lara_as_turn_fast()
void lara_col_turn_fast(ItemInfo* item, CollisionInfo* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_STEP_RIGHT (21)
// Collision:	lara_col_step_right()
void lara_as_step_right(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.CanLook = false;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (lara.Control.IsMoving)
	{
		ResetLaraTurnRateY(item);
		return;
	}

	// Walk action locks orientation.
	if (IsHeld(In::Walk))
	{
		ResetLaraTurnRateY(item);
	}
	else if (IsHeld(In::Left) || IsHeld(In::Right))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX);
	}

	if (IsHeld(In::RightStep) || (IsHeld(In::Walk) && IsHeld(In::Right)))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}
	
	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_STEP_RIGHT (21)
// Control:		lara_as_step_right()
void lara_col_step_right(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara.Control.MoveAngle = item->Pose.Orientation.y + ANGLE(90.0f);
	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	coll->Setup.LowerFloorBound = (lara.Control.WaterStatus == WaterStatus::Wade) ? NO_LOWER_BOUND : CLICK(0.8f);
	coll->Setup.UpperFloorBound = -CLICK(0.8f);
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeDown = !isSwamp;
	coll->Setup.BlockFloorSlopeUp = !isSwamp;
	coll->Setup.BlockDeathFloorDown = true;
	coll->Setup.ForwardAngle = lara.Control.MoveAngle;
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
		return;
	}

	if (LaraDeflectEdge(item, coll))
	{
		item->Animation.TargetState = LS_SOFT_SPLAT;
		if (GetStateDispatch(item, g_Level.Anims[item->Animation.AnimNumber]))
		{
			item->Animation.ActiveState = LS_SOFT_SPLAT;
			return;
		}

		LaraCollideStop(item, coll);
	}

	if (TestLaraStep(item, coll) || isSwamp)
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_STEP_LEFT (22)
// Collision:	lara_col_step_left()
void lara_as_step_left(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.CanLook = false;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (lara.Control.IsMoving)
	{
		ResetLaraTurnRateY(item);
		return;
	}

	// Walk action locks orientation.
	if (IsHeld(In::Walk))
	{
		ResetLaraTurnRateY(item);
	}
	else if (IsHeld(In::Left) || IsHeld(In::Right))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX);
	}

	if (IsHeld(In::LeftStep) || (IsHeld(In::Walk) && IsHeld(In::Left)))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_STEP_LEFT (22)
// Control:		lara_as_step_left()
void lara_col_step_left(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara.Control.MoveAngle = item->Pose.Orientation.y - ANGLE(90.0f);
	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	coll->Setup.LowerFloorBound = (lara.Control.WaterStatus == WaterStatus::Wade) ? NO_LOWER_BOUND : CLICK(0.8f);
	coll->Setup.UpperFloorBound = -CLICK(0.8f);
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeDown = !isSwamp;
	coll->Setup.BlockFloorSlopeUp = !isSwamp;
	coll->Setup.BlockDeathFloorDown = true;
	coll->Setup.ForwardAngle = lara.Control.MoveAngle;
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
		return;
	}

	if (LaraDeflectEdge(item, coll))
	{
		item->Animation.TargetState = LS_SOFT_SPLAT;
		if (GetStateDispatch(item, g_Level.Anims[item->Animation.AnimNumber]))
		{
			item->Animation.ActiveState = LS_SOFT_SPLAT;
			return;
		}

		LaraCollideStop(item, coll);
	}

	if (TestLaraStep(item, coll) || isSwamp)
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:	  LS_TURN_180 (173)
// Collision: lara_col_turn_180()
void lara_as_turn_180(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.CanLook = false;
	ResetLaraTurnRateY(item);

	item->Animation.TargetState = LS_IDLE;
}

// State:	LS_TURN_180 (173)
// Control: lara_as_turn_180()
void lara_col_turn_180(ItemInfo* item, CollisionInfo* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_ROLL_180_BACK (23)
// Collision:	lara_col_roll_back()
void lara_as_roll_180_back(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.CanLook = false;
	ResetLaraTurnRateY(item);

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_ROLL_180_BACK (23)
// Control:		lara_as_roll_back()
void lara_col_roll_180_back(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.MoveAngle = item->Pose.Orientation.y + ANGLE(180.0f);
	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeUp = true;
	coll->Setup.ForwardAngle = lara.Control.MoveAngle;
	Camera.laraNode = LM_HIPS;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideAnimation(item, coll);
		return;
	}

	if (coll->Middle.Floor > (STEPUP_HEIGHT / 2))
	{
		SetLaraFallBackAnimation(item);
		return;
	}

	ShiftItem(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_ROLL_180_FORWARD (45)
// Collision:	lara_col_roll_180_forward()
void lara_as_roll_180_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.CanLook = false;
	ResetLaraTurnRateY(item);

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_ROLL_180_FORWARD (45)
// Control:		lara_as_roll_180_forward()
void lara_col_roll_180_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.MoveAngle = item->Pose.Orientation.y;
	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeUp = true;
	coll->Setup.ForwardAngle = lara.Control.MoveAngle;
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
		return;
	}

	ShiftItem(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_WADE_FORWARD (65)
// Collision:	lara_col_wade_forward()
void lara_as_wade_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara.Control.CanLook = (isSwamp && lara.Control.WaterStatus == WaterStatus::Wade) ? false : true;
	Camera.targetElevation = -ANGLE(22.0f);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_IDLE;
		return;
	}

	if (IsHeld(In::Left) || IsHeld(In::Right))
	{
		if (isSwamp)
		{
			ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SWAMP_TURN_RATE_MAX);
			ModulateLaraLean(item, coll, LARA_LEAN_RATE / 3, LARA_LEAN_MAX * 0.6f);
		}
		else
		{
			ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_MED_TURN_RATE_MAX);
			ModulateLaraLean(item, coll, LARA_LEAN_RATE / 2, LARA_LEAN_MAX);
		}
	}

	if (IsHeld(In::Forward))
	{
		auto vaultContext = TestLaraVault(item, coll);
		if (IsHeld(In::Action) && vaultContext.Success)
		{
			item->Animation.TargetState = vaultContext.TargetState;
			SetLaraVault(item, coll, vaultContext);
			return;
		}
		
		if (lara.Control.WaterStatus == WaterStatus::Dry)
		{
			item->Animation.TargetState = LS_RUN_FORWARD;
		}
		else
			item->Animation.TargetState = LS_WADE_FORWARD;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_WADE_FORWARD (65)
// Control:		lara_as_wade_forward()
void lara_col_wade_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara.Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeUp = !isSwamp;
	coll->Setup.ForwardAngle = lara.Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestAndDoLaraLadderClimb(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		ResetLaraLean(item);

		item->Animation.TargetState = LS_SOFT_SPLAT;
		if (GetStateDispatch(item, g_Level.Anims[item->Animation.AnimNumber]))
		{
			item->Animation.ActiveState = LS_SOFT_SPLAT;
			return;
		}

		LaraCollideStop(item, coll);
	}

	if (TestLaraStep(item, coll) || isSwamp)
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_SPRINT (73)
// Collision:	lara_col_sprint()
void lara_as_sprint(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.SprintEnergy--;
	if (lara.SprintEnergy < 0)
		lara.SprintEnergy = 0;

	lara.Control.Count.Run++;
	if (lara.Control.Count.Run > LARA_SPRINT_JUMP_TIME)
		lara.Control.Count.Run = LARA_SPRINT_JUMP_TIME;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (IsHeld(In::Left) || IsHeld(In::Right))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX);
		ModulateLaraLean(item, coll, LARA_LEAN_RATE, LARA_LEAN_MAX);
	}

	if (IsHeld(In::Jump) || lara.Control.RunJumpQueued)
	{
		if (IsHeld(In::Walk) || !g_GameFlow->HasSprintJump())
		{
			item->Animation.TargetState = LS_SPRINT_DIVE;
			return;
		}
		else if (IsHeld(In::Sprint) && Context::CanSprintJumpForward(item, coll))
		{
			item->Animation.TargetState = LS_JUMP_FORWARD;
			return;
		}

		SetLaraRunJumpQueue(item, coll);
	}

	if (IsHeld(In::Crouch) && Context::CanCrouch(item, coll))
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (IsHeld(In::Forward))
	{
		auto vaultContext = TestLaraVault(item, coll);
		if (IsHeld(In::Action) && vaultContext.Success &&
			!TestLaraWall(item, OFFSET_RADIUS(coll->Setup.Radius), -BLOCK(5.0f / 8))) // HACK: Allow immediate vault only in the case of a soft splat.
		{
			item->Animation.TargetState = vaultContext.TargetState;
			SetLaraVault(item, coll, vaultContext);
			return;
		}
		
		if (lara.Control.WaterStatus == WaterStatus::Wade)
		{
			item->Animation.TargetState = LS_RUN_FORWARD; // TODO: Dispatch to wade forward state directly. @Sezz 2021.09.29
		}
		else if (IsHeld(In::Walk))
		{
			item->Animation.TargetState = LS_RUN_FORWARD;
		}
		else if (IsHeld(In::Sprint) && lara.SprintEnergy > 0)
		{
			item->Animation.TargetState = LS_SPRINT;
		}
		else
			item->Animation.TargetState = LS_RUN_FORWARD;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_SPRINT (73)
// Control:		lara_as_sprint()
void lara_col_sprint(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeUp = true;
	coll->Setup.ForwardAngle = lara.Control.MoveAngle;
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
		return;
	}

	if (LaraDeflectEdge(item, coll))
	{
		ResetLaraLean(item);

		if (TestLaraWall(item, OFFSET_RADIUS(coll->Setup.Radius), -BLOCK(5.0f / 8)) ||
			coll->HitTallObject)
		{
			item->Animation.TargetState = LS_SPLAT;
			if (GetStateDispatch(item, g_Level.Anims[item->Animation.AnimNumber]))
			{
				Rumble(0.5f, 0.15f);

				item->Animation.ActiveState = LS_SPLAT;
				return;
			}
		}

		item->Animation.TargetState = LS_SOFT_SPLAT;
		if (GetStateDispatch(item, g_Level.Anims[item->Animation.AnimNumber]))
		{
			item->Animation.ActiveState = LS_SOFT_SPLAT;
			return;
		}

		LaraCollideStop(item, coll);
	}

	if (TestAndDoLaraLadderClimb(item, coll))
		return;

	if (TestLaraStep(item, coll) && coll->CollisionType != CT_FRONT)
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_SPRINT_DIVE (74)
// Collision:	lara_col_sprint_dive()
void lara_as_sprint_dive(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.Count.Run++;
	if (lara.Control.Count.Run > LARA_RUN_JUMP_TIME)
		lara.Control.Count.Run = LARA_RUN_JUMP_TIME;

	if (IsHeld(In::Left) || IsHeld(In::Right))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX);
		ModulateLaraLean(item, coll, LARA_LEAN_RATE, LARA_LEAN_MAX * 0.6f);
	}

	item->Animation.TargetState = LS_RUN_FORWARD;
}

// State:		LS_SPRINT_DIVE (74)
// Control:		lara_col_sprint_dive()
void lara_col_sprint_dive(ItemInfo* item, CollisionInfo* coll)
{
	auto& lara = *GetLaraInfo(item);

	lara.Control.MoveAngle = (item->Animation.Velocity.z >= 0.0f) ? item->Pose.Orientation.y : item->Pose.Orientation.y + ANGLE(180.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.BlockFloorSlopeUp = true;
	coll->Setup.ForwardAngle = lara.Control.MoveAngle;
	GetCollisionInfo(coll, item);

	LaraDeflectEdgeJump(item, coll);

	if (TestLaraFall(item, coll))
	{
		SetLaraFallAnimation(item);
		return;
	}

	if (item->Animation.Velocity.z < 0.0f)
		lara.Control.MoveAngle = item->Pose.Orientation.y; // ???

	ShiftItem(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}
