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
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Flow/ScriptInterfaceFlowHandler.h"

using namespace TEN::Input;
using namespace TEN::Floordata;

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
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeDown = true;
	coll->Setup.BlockFloorSlopeUp = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
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
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.flags = CF_FOLLOW_CENTER;

	if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd - 1)
	{
		lara->Control.HandStatus = HandStatus::Free;

		if (UseForcedFixedCamera)
			UseForcedFixedCamera = 0;
	}
}

void lara_as_controlled_no_look(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
}

// State:		LS_VAULT (164),
//				LS_VAULT_2_STEPS (165), LS_VAULT_3_STEPS (166),
//				VAULT_1_STEP_CROUCH (167), VAULT_2_STEPS_CROUCH (168), VAULT_3_STEPS_CROUCH (169)
// Collision:	lara_void_func()
void lara_as_vault(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	EaseOutLaraHeight(item, lara->ProjectedFloorHeight - item->Pose.Position.y);
	item->Pose.Orientation.Lerp(lara->TargetOrientation, 0.4f);

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_AUTO_JUMP (62)
// Collision:	lara_as_jump_prepare()
void lara_as_auto_jump(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	item->Pose.Orientation.Lerp(lara->TargetOrientation, 0.4f);
}

// ---------------
// BASIC MOVEMENT:
// ---------------

// State:		LS_WALK_FORWARD (0)
// Collision:	lara_col_walk_forward()
void lara_as_walk_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.Count.Run++;
	if (lara->Control.Count.Run > (LARA_RUN_JUMP_TIME / 2 + 4))
		lara->Control.Count.Run = LARA_RUN_JUMP_TIME / 2 + 4;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	DoLegIK(item, coll, CLICK(1));

	// TODO: Implement item alignment properly someday. -- Sezz 2021.11.01
	if (lara->Control.IsMoving)
	{
		ModulateLaraTurnRateY(item, 0, 0, 0);
		return;
	}

	if (IsHeld(In::Left) || IsHeld(In::Right))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_MED_TURN_RATE_MAX);
		ModulateLaraLean(item, coll, LARA_LEAN_RATE / 6, LARA_LEAN_MAX / 2);
	}

	if (IsHeld(In::Forward))
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (IsHeld(In::Action) && vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (lara->Control.WaterStatus == WaterStatus::Wade)
			item->Animation.TargetState = LS_WADE_FORWARD;
		else if (IsHeld(In::Walk)) USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_WALK_FORWARD;
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
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Pose.Orientation.y;
	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeUp = true;
	coll->Setup.BlockFloorSlopeDown = true;
	coll->Setup.BlockDeathFloorDown = true;
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
	auto* lara = GetLaraInfo(item);

	lara->Control.Count.Run++;
	if (lara->Control.Count.Run > LARA_RUN_JUMP_TIME)
		lara->Control.Count.Run = LARA_RUN_JUMP_TIME;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (TrInput & (IN_LEFT | IN_RIGHT))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_FAST_TURN_RATE_MAX);
		ModulateLaraLean(item, coll, LARA_LEAN_RATE, LARA_LEAN_MAX);
	}

	if (TrInput & IN_JUMP || lara->Control.RunJumpQueued)
	{
		if (!(TrInput & IN_SPRINT) && lara->Control.Count.Run >= LARA_RUN_JUMP_TIME &&
			TestLaraRunJumpForward(item, coll))
		{
			item->Animation.TargetState = LS_JUMP_FORWARD;
			return;
		}

		SetLaraRunJumpQueue(item, coll);
	}

	if ((TrInput & IN_ROLL || (TrInput & IN_FORWARD && TrInput & IN_BACK)) &&
		!lara->Control.RunJumpQueued && // Jump queue blocks roll.
		lara->Control.WaterStatus != WaterStatus::Wade)
	{
		item->Animation.TargetState = LS_ROLL_180_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH && TestLaraCrouch(item))
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (lara->Control.WaterStatus == WaterStatus::Wade)
			item->Animation.TargetState = LS_WADE_FORWARD;
		else if (TrInput & IN_WALK)
			item->Animation.TargetState = LS_WALK_FORWARD;
		else if (TrInput & IN_SPRINT && lara->SprintEnergy)
			item->Animation.TargetState = LS_SPRINT;
		else USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_RUN_FORWARD;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_RUN_FORWARD (1)
// Control:		lara_as_run_forward()
void lara_col_run_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Pose.Orientation.y;
	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeUp = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
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

// TODO: See how a height threshold looks on movement anims.
// TODO: Clamp min/max heights.
// TODO: Probe for objects.
// TODO: A nice, light bounce when Lara descends.
// TODO: Do not touch death floors.
// TODO: Tilt feet to angle of floor.
void DoLegIK(ItemInfo* item, CollisionInfo* coll, int threshold)
{
	auto* lara = GetLaraInfo(item);

	// FAILURE: Should not use absolute positons of feet. They do not occupy the same x/z space during anims, resulting in oscillation on steps.
	auto lFootPos = Vector3Int::Zero;
	auto rFootPos = Vector3Int::Zero;
	GetLaraJointPosition(&lFootPos, LM_LFOOT);
	GetLaraJointPosition(&rFootPos, LM_RFOOT);
	/*auto* lFloorInfo = GetFloor(lPos.x, lPos.y, lPos.z, &item->roomNumber);
	auto* rFloorInfo = GetFloor(rPos.x, rPos.y, rPos.z, &item->roomNumber);
	auto lFloor = GetFloorHeight(lFloorInfo, lPos.x, lPos.y, lPos.z);
	auto rFloor = GetFloorHeight(rFloorInfo, rPos.x, rPos.y, rPos.z);*/

	// TODO: In slow turn anims, it may be appropriate to probe from the player's position insted to keep feet planted on the floor.
	// TODO: Make radius equivalent to the x/y distance from Lara's centre to the absolute position of each foot.

	// Originally, using constant LARA_RADIUS. Cleaner optics but less accurate.
	// TODO: Actual distance of feet causes oscillation.
	// This is wrong. I lose one axis...
	int lRadius = sqrt(pow(lFootPos.x - item->Pose.Position.x, 2) + pow(lFootPos.y - item->Pose.Position.y, 2));
	int rRadius = sqrt(pow(rFootPos.x - item->Pose.Position.x, 2) + pow(rFootPos.y - item->Pose.Position.y, 2));

	// More accurate radii. Cause significant oscillation.
	lRadius = pow(pow(lFootPos.x - item->Pose.Position.x, 2) + pow(lFootPos.z - item->Pose.Position.z, 2) , 0.5);
	rRadius = pow(pow(rFootPos.x - item->Pose.Position.x, 2) + pow(rFootPos.z - item->Pose.Position.z, 2) , 0.5);

	lRadius = LARA_RADIUS;
	rRadius = LARA_RADIUS;

	// Originally, using item rotation -/+ ANGLE(90.0f).
	short lAngle = item->Pose.Orientation.y - phd_atan(lFootPos.z, lFootPos.x);
	short rAngle = item->Pose.Orientation.y + phd_atan(rFootPos.z, rFootPos.x);

	// Probe left.
	int lHeight = GetCollision(item, lAngle, lRadius).Position.Floor;
	lHeight -= item->Pose.Position.y;

	// Probe right.
	int rHeight = GetCollision(item, rAngle, rRadius).Position.Floor;
	rHeight -= item->Pose.Position.y;

	int yPosDist = 0;
	auto lFloor = lHeight;
	auto rFloor = rHeight;

	// Also check current distances for accuracy?

	// TODO: Fix oscillation.

	// TODO: If one leg is over threshold, but other leg is on a slanted surface, at least descend to that. Or maybe not?

	// Cancel IK.
	if (lFloor == rFloor ||		   // Both legs would ascend/descend. TODO: There are scenarios where they might not be equal, but IKing would still be inappropriate.
		abs(lFloor) > threshold || // Left height beyond threshold.
		abs(rFloor) > threshold || // Right height beyond threshold.
		lFloor > 0 && rFloor > 0 && lFloor > threshold / 2 && rFloor > threshold / 2)	// test this.
	{
		yPosDist = 0;
		lFloor = 0;
		rFloor = 0;
	}

	// Ledge on left; left leg descends.
	if (abs(lFloor) > 0 && lFloor > rFloor)
	{
		yPosDist = lFloor;
		rFloor -= lFloor;
		lFloor = 0;
	}
	// Ledge on right; right leg descends.
	else if (abs(rFloor) > 0 && rFloor > lFloor)
	{
		yPosDist = rFloor;
		lFloor -= rFloor;
		rFloor = 0;
	}
	// Otherwise, left/right leg ascends.

	//---

	auto lThighPos = Vector3Int::Zero;
	auto lShinPos = Vector3Int::Zero;
	GetLaraJointPosition(&lThighPos, LM_LTHIGH);
	GetLaraJointPosition(&lShinPos, LM_LSHIN);

	float lThighLength = pow(
		pow(lShinPos.x - lThighPos.x, 2) +
		pow(lShinPos.y - lThighPos.y, 2) +
		pow(lShinPos.z - lThighPos.z, 2),
		0.5);
	float lShinLength = pow(
		pow(lShinPos.x - lFootPos.x, 2) +
		pow(lShinPos.y - lFootPos.y, 2) +
		pow(lShinPos.z - lFootPos.z, 2),
		0.5);
	float lThighToFootLength = item->Pose.Position.y;

	//---

	// TODO: Proper sin/cos maths. This is poor!
	float dist = 25.25; // TODO: This is an estimate, and results in slightly inaccurate ascending/descending. What is the actual value and what does it represent?
	short lThighAngle = -(lFloor / dist * 6.42857); // Float value is 45(degrees) / 7.
	short lShinAngle = -lThighAngle * 2;
	short lFootAngle = lThighAngle;
	short rThighAngle = -(rFloor / dist * 6.42857);
	short rShinAngle = -rThighAngle * 2;
	short rFootAngle = rThighAngle;

	// TODO: Use mutators. They currently don't work for this application, as child bones don't inherit position and rotation of parents.
	int div = 3;
	lara->yPosOffset += (yPosDist - lara->yPosOffset) / div;
	lara->lThighXrot += (ANGLE(lThighAngle) - lara->lThighXrot) / div;
	lara->lShinXrot += (ANGLE(lShinAngle) - lara->lShinXrot) / div;
	lara->lFootXrot += (ANGLE(lFootAngle) - lara->lFootXrot) / div;
	lara->rThighXrot += (ANGLE(rThighAngle) - lara->rThighXrot) / div;
	lara->rShinXrot += (ANGLE(rShinAngle) - lara->rShinXrot) / div;
	lara->rFootXrot += (ANGLE(rFootAngle) - lara->rFootXrot) / div;
}

// State:		LS_IDLE (2), LS_SPLAT_SOFT (170)
// Collision:	lara_col_idle()
void lara_as_idle(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.CanLook = ((isSwamp && lara->Control.WaterStatus == WaterStatus::Wade) || item->Animation.AnimNumber == LA_SWANDIVE_ROLL) ? false : true;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (lara->Control.IsMoving)
	{
		ModulateLaraTurnRateY(item, 0, 0, 0);
		return;
	}

	DoLegIK(item, coll, CLICK(1));

	// Handles waterskin and clockwork beetle.
	// TODO: Hardcoding.
	if (UseSpecialItem(item))
		return;

	if (IsHeld(In::Look) && lara->Control.CanLook)
		LookUpDown(item);

	// HACK.
	if (BinocularOn)
		return;

	if (!IsHeld(In::Jump) || isSwamp) // JUMP locks orientation outside swamps.
	{
		// Sidestep locks orientation.
		if ((IsHeld(In::LeftStep) || (IsHeld(In::Walk) && IsHeld(In::Left))) ||
			(IsHeld(In::RightStep) || (IsHeld(In::Walk) && IsHeld(In::Right))))
		{
			ModulateLaraTurnRateY(item, 0, 0, 0);
		}
		else if (IsHeld(In::Left) || IsHeld(In::Right))
			ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_MED_TURN_RATE_MAX);
	}

	if (lara->Control.WaterStatus == WaterStatus::Wade)
	{
		if (isSwamp)
			PseudoLaraAsSwampIdle(item, coll);
		else USE_FEATURE_IF_CPP20([[likely]])
			PseudoLaraAsWadeIdle(item, coll);

		return;
	}

	if (IsHeld(In::Jump))
	{
		SetLaraJumpDirection(item, coll);
		if (lara->Control.JumpDirection != JumpDirection::None)
			item->Animation.TargetState = LS_JUMP_PREPARE;

		return;
	}

	if (IsHeld(In::Roll) || (IsHeld(In::Forward) && IsHeld(In::Back)))
	{
		if (TrInput & IN_WALK || TestLaraTurn180(item, coll))
			item->Animation.TargetState = LS_TURN_180;
		else
			item->Animation.TargetState = LS_ROLL_180_FORWARD;
		
		return;
	}

	if (IsHeld(In::Crouch) && TestLaraCrouch(item))
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (IsHeld(In::Forward))
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (IsHeld(In::Action) && vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (IsHeld(In::Walk))
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->Animation.TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (IsHeld(In::Sprint) && TestLaraRunForward(item, coll))
		{
			item->Animation.TargetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		{
			item->Animation.TargetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (IsHeld(In::Back))
	{
		if (IsHeld(In::Walk))
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->Animation.TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		{
			item->Animation.TargetState = LS_RUN_BACK;
			return;
		}
	}

	if (IsHeld(In::LeftStep) || (IsHeld(In::Walk) && IsHeld(In::Left)))
	{
		if (TestLaraStepLeft(item, coll))
			item->Animation.TargetState = LS_STEP_LEFT;
		else
			item->Animation.TargetState = LS_IDLE;

		return;
	}
	else if (IsHeld(In::RightStep) || (IsHeld(In::Walk) && IsHeld(In::Right)))
	{
		if (TestLaraStepRight(item, coll))
			item->Animation.TargetState = LS_STEP_RIGHT;
		else
			item->Animation.TargetState = LS_IDLE;

		return;
	}

	if (IsHeld(In::Left))
	{
		if (IsHeld(In::Sprint) ||
			lara->Control.TurnRate <= -LARA_SLOW_TURN_RATE_MAX || TestLaraFastTurn(item))
		{
			item->Animation.TargetState = LS_TURN_LEFT_FAST;
		}
		else USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_TURN_LEFT_SLOW;

		return;
	}
	else if (IsHeld(In::Right))
	{
		if (IsHeld(In::Sprint) ||
			lara->Control.TurnRate >= LARA_SLOW_TURN_RATE_MAX || TestLaraFastTurn(item))
		{
			item->Animation.TargetState = LS_TURN_RIGHT_FAST;
		}
		else USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_TURN_RIGHT_SLOW;

		return;
	}

	// TODO: Without animation blending, the AFK state's
	// movement lock will be rather obnoxious.
	// Adding some idle breathing would also be nice. @Sezz 2021.10.31
	if (lara->Control.Count.Pose >= LARA_POSE_TIME && TestLaraPose(item, coll) &&
		g_GameFlow->HasAFKPose())
	{
		item->Animation.TargetState = LS_POSE;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// NOTE: Pseudo-states already removed on states_tier_3.
// Pseudo-state for idling in wade-height water.
void PseudoLaraAsWadeIdle(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);
	
	if (TrInput & IN_JUMP && TestLaraJumpUp(item, coll))
	{
		item->Animation.TargetState = LS_JUMP_PREPARE;
		lara->Control.JumpDirection = JumpDirection::Up;
		return;
	}

	if (TrInput & IN_ROLL || (TrInput & IN_FORWARD && TrInput & IN_BACK))
	{
		item->Animation.TargetState = LS_TURN_180;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TestLaraRunForward(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		{
			item->Animation.TargetState = LS_WADE_FORWARD;
			return;
		}
	}

	if (TrInput & IN_BACK && TestLaraWalkBack(item, coll))
	{
		item->Animation.TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP || (TrInput & IN_WALK && TrInput & IN_LEFT))
	{
		if (TestLaraStepLeft(item, coll))
			item->Animation.TargetState = LS_STEP_LEFT;
		else
			item->Animation.TargetState = LS_IDLE;

		return;
	}
	else if (TrInput & IN_RSTEP || (TrInput & IN_WALK && TrInput & IN_RIGHT))
	{
		if (TestLaraStepRight(item, coll))
			item->Animation.TargetState = LS_STEP_RIGHT;
		else
			item->Animation.TargetState = LS_IDLE;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->Animation.TargetState = LS_TURN_LEFT_SLOW;
		return;
	}
	else if (TrInput & IN_RIGHT)
	{
		item->Animation.TargetState = LS_TURN_RIGHT_SLOW;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// NOTE: Pseudo-states already removed on states_tier_3.
// Pseudo-state for idling in swamps.
void PseudoLaraAsSwampIdle(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_ROLL || (TrInput & IN_FORWARD && TrInput & IN_BACK))
	{
		item->Animation.TargetState = LS_TURN_180;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TestLaraWadeForwardSwamp(item, coll))
		{
			item->Animation.TargetState = LS_WADE_FORWARD;
			return;
		}
	}

	if (TrInput & IN_BACK && TestLaraWalkBackSwamp(item, coll))
	{
		item->Animation.TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP || (TrInput & IN_WALK && TrInput & IN_LEFT))
	{
		if (TestLaraStepLeftSwamp(item, coll))
			item->Animation.TargetState = LS_STEP_LEFT;
		else
			item->Animation.TargetState = LS_IDLE;

		return;
	}
	else if (TrInput & IN_RSTEP || (TrInput & IN_WALK && TrInput & IN_RIGHT))
	{
		if (TestLaraStepRightSwamp(item, coll))
			item->Animation.TargetState = LS_STEP_RIGHT;
		else
			item->Animation.TargetState = LS_IDLE;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->Animation.TargetState = LS_TURN_LEFT_SLOW;
		return;
	}
	else if (TrInput & IN_RIGHT)
	{
		item->Animation.TargetState = LS_TURN_RIGHT_SLOW;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_IDLE (2), LS_POSE (4), LS_SPLAT_SOFT (170)
// Control:		lara_as_idle(), lara_as_pose()
void lara_col_idle(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	lara->Control.MoveAngle = (item->Animation.Velocity.z >= 0) ? item->Pose.Orientation.y : (item->Pose.Orientation.y + ANGLE(180.0f));
	coll->Setup.LowerFloorBound = isSwamp ? NO_LOWER_BOUND : STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeDown = !isSwamp;
	coll->Setup.BlockFloorSlopeUp = !isSwamp;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	// TODO: Better clamp handling. This can result in Lara standing above or below the floor. @Sezz 2022.04.01
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
	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown(item);

	if (TestLaraPose(item, coll))
	{
		if (TrInput & IN_ROLL)
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
	auto* lara = GetLaraInfo(item);

	if (TrInput & (IN_LEFT | IN_RIGHT))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_MED_TURN_RATE_MAX);
		ModulateLaraLean(item, coll, LARA_LEAN_RATE / 4, LARA_LEAN_MAX / 3);
	}

	if (TrInput & IN_ROLL)
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
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Pose.Orientation.y + ANGLE(180.0f);
	item->Animation.Velocity.y = 0;
	item->Animation.IsAirborne = false;
	coll->Setup.BlockFloorSlopeDown = true;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
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

// State:		LS_TURN_RIGHT_SLOW (6)
// Collision:	lara_col_turn_right_slow()
void lara_as_turn_right_slow(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.CanLook = (isSwamp && lara->Control.WaterStatus == WaterStatus::Wade) ? false : true;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	DoLegIK(item, coll, CLICK(1));

	if (lara->Control.WaterStatus == WaterStatus::Wade)
	{
		if (isSwamp)
			PsuedoLaraAsSwampTurnRightSlow(item, coll);
		else USE_FEATURE_IF_CPP20([[likely]])
			PsuedoLaraAsWadeTurnRightSlow(item, coll);

		return;
	}

	ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_MED_FAST_TURN_RATE_MAX);

	if (TrInput & IN_JUMP)
	{
		SetLaraJumpDirection(item, coll);
		if (lara->Control.JumpDirection != JumpDirection::None)
		{
			item->Animation.TargetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if (TrInput & IN_ROLL)
	{
		item->Animation.TargetState = LS_ROLL_180_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH && TestLaraCrouch(item))
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->Animation.TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->Animation.TargetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		{
			item->Animation.TargetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->Animation.TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		{
			item->Animation.TargetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP || (TrInput & IN_WALK && TrInput & IN_LEFT) &&
		TestLaraStepLeft(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP || (TrInput & IN_WALK && TrInput & IN_RIGHT) &&
		TestLaraStepRight(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_RIGHT)
	{
		// TODO: This hasn't worked since TR1.
		/*if (TrInput & IN_WALK)
		{
			item->Animation.TargetState = LS_TURN_RIGHT_SLOW;

			if (lara->Control.TurnRate > LARA_SLOW_TURN_RATE_MAX)
				lara->Control.TurnRate = LARA_SLOW_TURN_RATE_MAX;
		}
		else */if (lara->Control.TurnRate > LARA_SLOW_MED_TURN_RATE_MAX)
			item->Animation.TargetState = LS_TURN_RIGHT_FAST;
		else USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// Pseudo-state for turning right slowly in wade-height water.
void PsuedoLaraAsWadeTurnRightSlow(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_WADE_TURN_RATE_MAX);

	if (TrInput & IN_JUMP && TestLaraJumpUp(item, coll))
	{
		item->Animation.TargetState = LS_JUMP_PREPARE;
		lara->Control.JumpDirection = JumpDirection::Up;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TestLaraRunForward(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		{
			item->Animation.TargetState = LS_WADE_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK && TestLaraWalkBack(item, coll))
	{
		item->Animation.TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP || (TrInput & IN_WALK && TrInput & IN_LEFT) &&
		TestLaraStepLeft(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP || (TrInput & IN_WALK && TrInput & IN_RIGHT) &&
		TestLaraStepRight(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_RIGHT)
	{
		item->Animation.TargetState = LS_TURN_RIGHT_SLOW;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// Pseudo-state for turning right slowly in swamps.
void PsuedoLaraAsSwampTurnRightSlow(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SWAMP_TURN_RATE_MAX);

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TestLaraWadeForwardSwamp(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		{
			item->Animation.TargetState = LS_WADE_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK && TestLaraWalkBackSwamp(item, coll))
	{
		item->Animation.TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP || (TrInput & IN_WALK && TrInput & IN_LEFT) &&
		TestLaraStepLeftSwamp(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP || (TrInput & IN_WALK && TrInput & IN_RIGHT) &&
		TestLaraStepRightSwamp(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_RIGHT)
	{
		item->Animation.TargetState = LS_TURN_RIGHT_SLOW;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_TURN_RIGHT_SLOW (6)
// Control:		lara_as_turn_right_slow()
void lara_col_turn_right_slow(ItemInfo* item, CollisionInfo* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_TURN_LEFT_SLOW (7)
// Collision:	lara_col_turn_left_slow()
void lara_as_turn_left_slow(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.CanLook = (isSwamp && lara->Control.WaterStatus == WaterStatus::Wade) ? false : true;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	DoLegIK(item, coll, CLICK(1));

	if (lara->Control.WaterStatus == WaterStatus::Wade)
	{
		if (isSwamp)
			PsuedoLaraAsSwampTurnLeftSlow(item, coll);
		else USE_FEATURE_IF_CPP20([[likely]])
			PsuedoLaraAsWadeTurnLeftSlow(item, coll);

		return;
	}

	ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_MED_FAST_TURN_RATE_MAX);

	if (TrInput & IN_JUMP)
	{
		SetLaraJumpDirection(item, coll);
		if (lara->Control.JumpDirection != JumpDirection::None)
		{
			item->Animation.TargetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if (TrInput & IN_ROLL)
	{
		item->Animation.TargetState = LS_ROLL_180_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH && TestLaraCrouch(item))
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if(TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->Animation.TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->Animation.TargetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		{
			item->Animation.TargetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->Animation.TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		{
			item->Animation.TargetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP || (TrInput & IN_WALK && TrInput & IN_LEFT) &&
		TestLaraStepLeft(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP || (TrInput & IN_WALK && TrInput & IN_RIGHT) &&
		TestLaraStepRight(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		// TODO: This hasn't worked since TR1.
		/*if (TrInput & IN_WALK)
		{
			item->Animation.TargetState = LS_TURN_LEFT_SLOW;

			if (lara->Control.TurnRate < -LARA_SLOW_TURN_RATE_MAX)
				lara->Control.TurnRate = -LARA_SLOW_TURN_RATE_MAX;
		}
		else */if (lara->Control.TurnRate < -LARA_SLOW_MED_TURN_RATE_MAX)
			item->Animation.TargetState = LS_TURN_LEFT_FAST;
		else USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_TURN_LEFT_SLOW;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// Pseudo-state for turning left slowly in wade-height water.
void PsuedoLaraAsWadeTurnLeftSlow(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_WADE_TURN_RATE_MAX);

	if (TrInput & IN_JUMP && TestLaraJumpUp(item, coll))
	{
		item->Animation.TargetState = LS_JUMP_PREPARE;
		lara->Control.JumpDirection = JumpDirection::Up;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TestLaraRunForward(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		{
			item->Animation.TargetState = LS_WADE_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK && TestLaraWalkBack(item, coll))
	{
		item->Animation.TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP || (TrInput & IN_WALK && TrInput & IN_LEFT) &&
		TestLaraStepLeft(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP || (TrInput & IN_WALK && TrInput & IN_RIGHT) &&
		TestLaraStepRight(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->Animation.TargetState = LS_TURN_LEFT_SLOW;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// Pseudo-state for turning left slowly in swamps.
void PsuedoLaraAsSwampTurnLeftSlow(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SWAMP_TURN_RATE_MAX);

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TestLaraWadeForwardSwamp(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		{
			item->Animation.TargetState = LS_WADE_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK && TestLaraWalkBackSwamp(item, coll))
	{
		item->Animation.TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP || (TrInput & IN_WALK && TrInput & IN_LEFT) &&
		TestLaraStepLeftSwamp(item, coll))
	{
		item->Animation.TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP || (TrInput & IN_WALK && TrInput & IN_RIGHT) &&
		TestLaraStepRightSwamp(item, coll))
	{
		item->Animation.TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->Animation.TargetState = LS_TURN_LEFT_SLOW;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_TURN_LEFT_SLOW (7)
// Control:		lara_as_turn_left_slow()
void lara_col_turn_left_slow(ItemInfo* item, CollisionInfo* coll)
{
	lara_col_turn_right_slow(item, coll);
}

// State:		LS_DEATH (8)
// Collision:	lara_col_death()
void lara_as_death(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	item->Animation.Velocity.z = 0.0f;
	lara->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	if (BinocularRange)
	{
		BinocularRange = 0;
		LaserSight = false;
		AlterFOV(LastFOV);
		item->MeshBits = ALL_JOINT_BITS;
		lara->Inventory.IsBusy = false;
	}

	auto bounds = GameBoundingBox(item);
	if (bounds.GetHeight() <= (LARA_HEIGHT * 0.75f))
		AlignLaraToSurface(item);

	ModulateLaraTurnRateY(item, 0, 0, 0);
}

// State:		LS_DEATH (8)
// Control:		lara_as_death()
void lara_col_death(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	item->Animation.IsAirborne = false;
	lara->Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.Radius = LARA_RADIUS_DEATH;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	StopSoundEffect(SFX_TR4_LARA_FALL);
	item->HitPoints = -1;
	lara->Air = -1;

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
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	ModulateLaraTurnRateY(item, 0, 0, 0);
}

// State:		LS_SPLAT (12)
// Control:		lara_as_splat()
void lara_col_splat(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.BlockFloorSlopeUp = true;
	coll->Setup.BlockFloorSlopeDown = true;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
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
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.CanLook = (isSwamp && lara->Control.WaterStatus == WaterStatus::Wade) ? false : true;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	DoLegIK(item, coll, CLICK(1));

	if (lara->Control.IsMoving)
	{
		ModulateLaraTurnRateY(item, 0, 0, 0);
		return;
	}

	if (isSwamp && lara->Control.WaterStatus == WaterStatus::Wade)
	{
		PseudoLaraAsSwampWalkBack(item, coll);
		return;
	}

	if (TrInput & (IN_LEFT | IN_RIGHT))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX);
		ModulateLaraLean(item, coll, LARA_LEAN_RATE / 4, LARA_LEAN_MAX / 3);
	}

	if (TrInput & IN_BACK &&
		(TrInput & IN_WALK || lara->Control.WaterStatus == WaterStatus::Wade))
	{
		item->Animation.TargetState = LS_WALK_BACK;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// Pseudo-state for walking back in swamps.
void PseudoLaraAsSwampWalkBack(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & (IN_LEFT | IN_RIGHT))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX / 3);
		ModulateLaraLean(item, coll, LARA_LEAN_RATE / 3, LARA_LEAN_MAX / 3);
	}

	if (TrInput & IN_BACK)
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
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.MoveAngle = item->Pose.Orientation.y + ANGLE(180.0f);
	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	coll->Setup.LowerFloorBound = (lara->Control.WaterStatus == WaterStatus::Wade) ? NO_LOWER_BOUND : STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeDown = !isSwamp;
	coll->Setup.BlockFloorSlopeUp = !isSwamp;
	coll->Setup.BlockDeathFloorDown = true;
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

// State:		LS_TURN_RIGHT_FAST (20)
// Collision:	lara_col_turn_right_fast()
void lara_as_turn_right_fast(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, LARA_MED_TURN_RATE_MAX, LARA_FAST_TURN_RATE_MAX);
	DoLegIK(item, coll, CLICK(1));

	if (TrInput & IN_JUMP)
	{
		SetLaraJumpDirection(item, coll);
		if (lara->Control.JumpDirection != JumpDirection::None)
		{
			item->Animation.TargetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if (TrInput & IN_ROLL &&
		lara->Control.WaterStatus != WaterStatus::Wade)
	{
		item->Animation.TargetState = LS_ROLL_180_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH && TestLaraCrouch(item))
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (lara->Control.WaterStatus == WaterStatus::Wade)
		{
			if (TestLaraRunForward(item, coll))
			{
				item->Animation.TargetState = LS_WADE_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->Animation.TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->Animation.TargetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		{
			item->Animation.TargetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->Animation.TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		{
			item->Animation.TargetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP || (TrInput & IN_WALK && TrInput & IN_LEFT))
	{
		if (TestLaraStepLeft(item, coll))
			item->Animation.TargetState = LS_STEP_LEFT;
		else
			item->Animation.TargetState = LS_IDLE;

		return;
	}
	else if (TrInput & IN_RSTEP || (TrInput & IN_WALK && TrInput & IN_RIGHT))
	{
		if (TestLaraStepRight(item, coll))
			item->Animation.TargetState = LS_STEP_RIGHT;
		else
			item->Animation.TargetState = LS_IDLE;

		return;
	}

	// TODO: Hold WALK to slow down again? Will require introduction more player options.
	if (TrInput & IN_RIGHT)
	{
		item->Animation.TargetState = LS_TURN_RIGHT_FAST;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_TURN_RIGHT_FAST (20)
// Control:		lara_as_turn_right_fast()
void lara_col_turn_right_fast(ItemInfo* item, CollisionInfo* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_TURN_LEFT_FAST (152)
// Collision:	lara_col_turn_left_fast()
void lara_as_turn_left_fast(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, LARA_MED_TURN_RATE_MAX, LARA_FAST_TURN_RATE_MAX);
	DoLegIK(item, coll, CLICK(1));

	if (TrInput & IN_JUMP)
	{
		SetLaraJumpDirection(item, coll);
		if (lara->Control.JumpDirection != JumpDirection::None)
		{
			item->Animation.TargetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if (TrInput & IN_ROLL &&
		lara->Control.WaterStatus != WaterStatus::Wade)
	{
		item->Animation.TargetState = LS_ROLL_180_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH && TestLaraCrouch(item))
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if(lara->Control.WaterStatus == WaterStatus::Wade)
		{
			if (TestLaraRunForward(item, coll))
			{
				item->Animation.TargetState = LS_WADE_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->Animation.TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->Animation.TargetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		{
			item->Animation.TargetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->Animation.TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) USE_FEATURE_IF_CPP20([[likely]])
		{
			item->Animation.TargetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP || (TrInput & IN_WALK && TrInput & IN_LEFT))
	{
		if (TestLaraStepLeft(item, coll))
			item->Animation.TargetState = LS_STEP_LEFT;
		else
			item->Animation.TargetState = LS_IDLE;

		return;
	}
	else if (TrInput & IN_RSTEP || (TrInput & IN_WALK && TrInput & IN_RIGHT))
	{
		if (TestLaraStepRight(item, coll))
			item->Animation.TargetState = LS_STEP_RIGHT;
		else
			item->Animation.TargetState = LS_IDLE;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->Animation.TargetState = LS_TURN_LEFT_FAST;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_TURN_LEFT_FAST (152)
// Control:		lara_as_turn_left_fast()
void lara_col_turn_left_fast(ItemInfo* item, CollisionInfo* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_STEP_RIGHT (21)
// Collision:	lara_col_step_right()
void lara_as_step_right(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	DoLegIK(item, coll, CLICK(1));

	if (lara->Control.IsMoving)
	{
		ModulateLaraTurnRateY(item, 0, 0, 0);
		return;
	}

	if (TrInput & IN_WALK) // WALK locks orientation.
		ModulateLaraTurnRateY(item, 0, 0, 0);
	else
	{
		if (TrInput & (IN_LEFT | IN_RIGHT))
			ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX);
	}

	if (TrInput & IN_RSTEP || (TrInput & IN_WALK && TrInput & IN_RIGHT))
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
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.MoveAngle = item->Pose.Orientation.y + ANGLE(90.0f);
	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	coll->Setup.LowerFloorBound = (lara->Control.WaterStatus == WaterStatus::Wade) ? NO_LOWER_BOUND : CLICK(0.8f);
	coll->Setup.UpperFloorBound = -CLICK(0.8f);
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeDown = !isSwamp;
	coll->Setup.BlockFloorSlopeUp = !isSwamp;
	coll->Setup.BlockDeathFloorDown = true;
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
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	DoLegIK(item, coll, CLICK(1));

	if (lara->Control.IsMoving)
	{
		ModulateLaraTurnRateY(item, 0, 0, 0);
		return;
	}

	if (TrInput & IN_WALK) // WALK locks orientation.
		ModulateLaraTurnRateY(item, 0, 0, 0);
	else
	{
		if (TrInput & (IN_LEFT | IN_RIGHT))
			ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX);
	}

	if (TrInput & IN_LSTEP || (TrInput & IN_WALK && TrInput & IN_LEFT))
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
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.MoveAngle = item->Pose.Orientation.y - ANGLE(90.0f);
	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	coll->Setup.LowerFloorBound = (lara->Control.WaterStatus == WaterStatus::Wade) ? NO_LOWER_BOUND : CLICK(0.8f);
	coll->Setup.UpperFloorBound = -CLICK(0.8f);
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeDown = !isSwamp;
	coll->Setup.BlockFloorSlopeUp = !isSwamp;
	coll->Setup.BlockDeathFloorDown = true;
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
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	ModulateLaraTurnRateY(item, 0, 0, 0);

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
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	ModulateLaraTurnRateY(item, 0, 0, 0);

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_ROLL_180_BACK (23)
// Control:		lara_as_roll_back()
void lara_col_roll_180_back(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Pose.Orientation.y + ANGLE(180.0f);
	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeUp = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
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
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;
	ModulateLaraTurnRateY(item, 0, 0, 0);

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_ROLL_180_FORWARD (45)
// Control:		lara_as_roll_180_forward()
void lara_col_roll_180_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Pose.Orientation.y;
	item->Animation.IsAirborne = false;
	item->Animation.Velocity.y = 0;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeUp = true;
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
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.CanLook = (isSwamp && lara->Control.WaterStatus == WaterStatus::Wade) ? false : true;
	Camera.targetElevation = -ANGLE(22.0f);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_IDLE;
		return;
	}

	if (isSwamp)
	{
		PseudoLaraAsSwampWadeForward(item, coll);
		return;
	}

	if (TrInput & (IN_LEFT | IN_RIGHT))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_MED_TURN_RATE_MAX);
		ModulateLaraLean(item, coll, LARA_LEAN_RATE / 2, LARA_LEAN_MAX);
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (lara->Control.WaterStatus == WaterStatus::Dry)
			item->Animation.TargetState = LS_RUN_FORWARD;
		else USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_WADE_FORWARD;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// Pseudo-state for wading in swamps.
void PseudoLaraAsSwampWadeForward(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & (IN_LEFT | IN_RIGHT))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SWAMP_TURN_RATE_MAX);
		ModulateLaraLean(item, coll, LARA_LEAN_RATE / 3, LARA_LEAN_MAX * 0.6f);
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && vaultResult.Success)
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_WADE_FORWARD;

		return;
	}

	item->Animation.TargetState = LS_IDLE;
}

// State:		LS_WADE_FORWARD (65)
// Control:		lara_as_wade_forward()
void lara_col_wade_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	bool isSwamp = TestEnvironment(ENV_FLAG_SWAMP, item);

	lara->Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeUp = !isSwamp;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
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
	auto* lara = GetLaraInfo(item);

	lara->SprintEnergy--;

	lara->Control.Count.Run++;
	if (lara->Control.Count.Run > LARA_SPRINT_JUMP_TIME)
		lara->Control.Count.Run = LARA_SPRINT_JUMP_TIME;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (TrInput & (IN_LEFT | IN_RIGHT))
	{
		ModulateLaraTurnRateY(item, LARA_TURN_RATE_ACCEL, 0, LARA_SLOW_TURN_RATE_MAX);
		ModulateLaraLean(item, coll, LARA_LEAN_RATE, LARA_LEAN_MAX);
	}

	if (TrInput & IN_JUMP || lara->Control.RunJumpQueued)
	{
		if (TrInput & IN_WALK || !g_GameFlow->HasSprintJump())
		{
			item->Animation.TargetState = LS_SPRINT_DIVE;
			return;
		}
		else if (TrInput & IN_SPRINT && lara->Control.Count.Run >= LARA_SPRINT_JUMP_TIME &&
			TestLaraRunJumpForward(item, coll) && HasStateDispatch(item, LS_JUMP_FORWARD))
		{
			item->Animation.TargetState = LS_JUMP_FORWARD;
			return;
		}

		SetLaraRunJumpQueue(item, coll);
	}

	if (TrInput & IN_CROUCH && TestLaraCrouch(item))
	{
		item->Animation.TargetState = LS_CROUCH_IDLE;
		return;
	}

	// TODO: Supposedly there is a bug wherein sprinting into the boundary between shallow and deep water
	// while meeting some condition allows Lara to run around in the water room. Investigate. @Sezz 2021.09.29
	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && vaultResult.Success &&
			!TestLaraWall(item, OFFSET_RADIUS(coll->Setup.Radius), -CLICK(2.5f))) // HACK: Allow immediate vault only in the case of a soft splat.
		{
			item->Animation.TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (lara->Control.WaterStatus == WaterStatus::Wade)
			item->Animation.TargetState = LS_RUN_FORWARD; // TODO: Dispatch to wade forward state directly. @Sezz 2021.09.29
		else if (TrInput & IN_WALK)
			item->Animation.TargetState = LS_RUN_FORWARD;
		else if (TrInput & IN_SPRINT && lara->SprintEnergy > 0) USE_FEATURE_IF_CPP20([[likely]])
			item->Animation.TargetState = LS_SPRINT;
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
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.BlockFloorSlopeUp = true;
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
		return;
	}

	if (LaraDeflectEdge(item, coll))
	{
		ResetLaraLean(item);

		if (TestLaraWall(item, OFFSET_RADIUS(coll->Setup.Radius), -CLICK(2.5f)) ||
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
	auto* lara = GetLaraInfo(item);

	lara->Control.Count.Run++;
	if (lara->Control.Count.Run > LARA_RUN_JUMP_TIME)
		lara->Control.Count.Run = LARA_RUN_JUMP_TIME;

	if (TrInput & (IN_LEFT | IN_RIGHT))
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
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = (item->Animation.Velocity.z >= 0) ? item->Pose.Orientation.y : item->Pose.Orientation.y + ANGLE(180.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.BlockFloorSlopeUp = true;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	LaraDeflectEdgeJump(item, coll);

	if (TestLaraFall(item, coll))
	{
		SetLaraFallAnimation(item);
		return;
	}

	if (item->Animation.Velocity.z < 0)
		lara->Control.MoveAngle = item->Pose.Orientation.y; // ???

	ShiftItem(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}
