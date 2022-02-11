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
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Scripting/GameFlowScript.h"

// ------------------------------
// BASIC MOVEMENT & MISCELLANEOUS
// Control & Collision Functions
// ------------------------------

// --------------
// MISCELLANEOUS:
// --------------

void lara_void_func(ITEM_INFO* item, COLL_INFO* coll)
{
	return;
}

void lara_default_col(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = item->Position.yRot;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsPit = true;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	GetCollisionInfo(coll, item);
	LaraResetGravityStatus(item, coll);
}

void lara_as_special(ITEM_INFO* item, COLL_INFO* coll)
{
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(170.0f);
	Camera.targetElevation = -ANGLE(25.0f);
}

void lara_as_null(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
}

void lara_as_controlled(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd - 1)
	{
		info->Control.HandStatus = HandStatus::Free;

		if (UseForcedFixedCamera)
			UseForcedFixedCamera = 0;
	}
}

void lara_as_controlled_no_look(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
}

// State:	LS_VAULT (164)
// Control:	lara_as_null()
void lara_col_vault(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	EaseOutLaraHeight(item, info->ProjectedFloorHeight - item->Position.yPos);
}

// State:	LS_AUTO_JUMP (62)
// Control:	lara_as_null()
void lara_col_auto_jump(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	
	info->Control.CalculatedJumpVelocity = -3 - sqrt(-9600 - 12 * std::max<int>(info->ProjectedFloorHeight - item->Position.yPos, -CLICK(7.5f)));
}

// ---------------
// BASIC MOVEMENT:
// ---------------

// State:		LS_WALK_FORWARD (0)
// Collision:	lara_col_walk_forward()
void lara_as_walk_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.Count.RunJump++;
	if (info->Control.Count.RunJump > LARA_RUN_JUMP_TIME - 4)
		info->Control.Count.RunJump = LARA_RUN_JUMP_TIME - 4;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		return;
	}

	// TODO: Implement item alignment properly someday. @Sezz 2021.11.01
	if (info->Control.IsMoving)
		return;

	// TODO: If stopping and holding WALK without FORWARD, Lara can't turn. @Sezz 2021.10.09
	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_MED_TURN_MAX)
			info->Control.TurnRate = -LARA_MED_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 6);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_MED_TURN_MAX)
			info->Control.TurnRate = LARA_MED_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 6);
	}

	if (TrInput & IN_FORWARD)
	{
		if (info->Control.WaterStatus == WaterStatus::Wade)
			item->TargetState = LS_WADE_FORWARD;
		else if (TrInput & IN_WALK) [[likely]]
			item->TargetState = LS_WALK_FORWARD;
		else
			item->TargetState = LS_RUN_FORWARD;

		return;
	}

	item->TargetState = LS_IDLE;
}

// State:		LA_WALK_FORWARD (0)
// Control:		lara_as_walk_forward()
void lara_col_walk_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = item->Position.yRot;
	item->Airborne = false;
	item->VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.FloorSlopeIsPit = true;
	coll->Setup.DeathFlagIsPit = true;
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
		return;
	}

	if (TestAndDoLaraLadderClimb(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		item->TargetState = LS_SPLAT;
		if (GetChange(item, &g_Level.Anims[item->AnimNumber]))
			return;

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
void lara_as_run_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.Count.RunJump++;
	if (info->Control.Count.RunJump > LARA_RUN_JUMP_TIME)
		info->Control.Count.RunJump = LARA_RUN_JUMP_TIME;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_FAST_TURN_MAX)
			info->Control.TurnRate = -LARA_FAST_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_FAST_TURN_MAX)
			info->Control.TurnRate = LARA_FAST_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE);
	}

	if ((TrInput & IN_JUMP || info->Control.RunJumpQueued) &&
		info->Control.WaterStatus != WaterStatus::Wade)
	{
		if (info->Control.Count.RunJump >= LARA_RUN_JUMP_TIME &&
			TestLaraRunJumpForward(item, coll))
		{
			item->TargetState = LS_JUMP_FORWARD;
			return;
		}

		SetLaraRunJumpQueue(item, coll);
	}

	if (TrInput & IN_SPRINT && info->SprintEnergy &&
		info->Control.WaterStatus != WaterStatus::Wade)
	{
		item->TargetState = LS_SPRINT;
		return;
	}

	// TODO: Control settings option to enable/disable FORWARD+BACK as roll input.
	if ((TrInput & (IN_ROLL | IN_FORWARD & IN_BACK)) && !info->Control.RunJumpQueued &&
		info->Control.WaterStatus != WaterStatus::Wade)
	{
		item->TargetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(info->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(info->Control.WeaponControl.GunType)) &&
		info->Control.WaterStatus != WaterStatus::Wade)
	{
		item->TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (info->Control.WaterStatus == WaterStatus::Wade)
			item->TargetState = LS_WADE_FORWARD;
		else if (TrInput & IN_WALK)
			item->TargetState = LS_WALK_FORWARD;
		else [[likely]]
			item->TargetState = LS_RUN_FORWARD;

		return;
	}

	item->TargetState = LS_IDLE;
}

// State:		LS_RUN_FORWARD (1)
// Control:		lara_as_run_forward()
void lara_col_run_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = item->Position.yRot;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	GetCollisionInfo(coll, item);
	LaraResetGravityStatus(item, coll);

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
		return;
	}

	if (TestAndDoLaraLadderClimb(item, coll))
		return;

	if (LaraDeflectEdge(item, coll))
	{
		item->Position.zRot = 0;

		if (coll->HitTallObject || TestLaraSplat(item, OFFSET_RADIUS(coll->Setup.Radius), -CLICK(2.5f)))
		{
			item->TargetState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->AnimNumber]))
			{
				item->ActiveState = LS_SPLAT;
				return;
			}
		}

		LaraCollideStop(item, coll);
	}

	if (TestLaraStep(item, coll) && coll->CollisionType != CT_FRONT)
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_IDLE (2), LS_POSE (4)
// Collision:	lara_col_idle()
void lara_as_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.CanLook = ((TestEnvironment(ENV_FLAG_SWAMP, item) && info->Control.WaterStatus == WaterStatus::Wade) || item->AnimNumber == LA_SWANDIVE_ROLL) ? false : true;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		return;
	}

	// Handles waterskin and clockwork beetle.
	// TODO: Hardcoding.
	if (UseSpecialItem(item))
		return;

	if (TrInput & IN_LOOK && info->Control.CanLook)
		LookUpDown();

	if (TrInput & IN_LEFT &&
		!(TrInput & IN_JUMP))	// JUMP locks y rotation.
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			info->Control.TurnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT &&
		!(TrInput & IN_JUMP))
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_SLOW_TURN_MAX)
			info->Control.TurnRate = LARA_SLOW_TURN_MAX;
	}

	if (info->Control.WaterStatus == WaterStatus::Wade)
	{
		if (TestEnvironment(ENV_FLAG_SWAMP, item))
			PseudoLaraAsSwampIdle(item, coll);
		else [[likely]]
			PseudoLaraAsWadeIdle(item, coll);

		return;
	}

	if (TrInput & IN_JUMP)
	{
		SetLaraJumpDirection(item, coll);
		if (info->Control.JumpDirection != JumpDirection::None)
			item->TargetState = LS_JUMP_PREPARE;

		return;
	}

	if (TrInput & IN_ROLL)
	{
		item->TargetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(info->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(info->Control.WeaponControl.GunType)))
	{
		item->TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && info->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->TargetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->TargetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) [[likely]]
		{
			item->TargetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		if (TrInput & IN_SPRINT ||
			info->Control.TurnRate <= -LARA_SLOW_TURN_MAX ||
			(info->Control.HandStatus == HandStatus::WeaponReady && info->Control.WeaponControl.GunType != WEAPON_TORCH) ||
			(info->Control.HandStatus == HandStatus::DrawWeapon && info->Control.WeaponControl.GunType != WEAPON_FLARE))
		{
			item->TargetState = LS_TURN_LEFT_FAST;
		}
		else [[likely]]
			item->TargetState = LS_TURN_LEFT_SLOW;

		return;
	}
	else if (TrInput & IN_RIGHT)
	{
		if (TrInput & IN_SPRINT ||
			info->Control.TurnRate >= LARA_SLOW_TURN_MAX ||
			(info->Control.HandStatus == HandStatus::WeaponReady && info->Control.WeaponControl.GunType != WEAPON_TORCH) ||
			(info->Control.HandStatus == HandStatus::DrawWeapon && info->Control.WeaponControl.GunType != WEAPON_FLARE))
		{
			item->TargetState = LS_TURN_RIGHT_FAST;
		}
		else [[likely]]
			item->TargetState = LS_TURN_RIGHT_SLOW;

		return;
	}

	// TODO: Without animation blending, the AFK state's
	// movement lock will be rather obnoxious.
	// Adding some idle breathing would also be nice. @Sezz 2021.10.31
	if (info->Control.Count.Pose >= LARA_POSE_TIME && TestLaraPose(item, coll) &&
		g_GameFlow->Animations.Pose)
	{
		item->TargetState = LS_POSE;
		return;
	}

	item->TargetState = LS_IDLE;
}

// TODO: Future-proof for rising water.
// TODO: Make these into true states someday? It may take a bit of work. @Sezz 2021.10.13
// Pseudo-state for idling in wade-height water.
void PseudoLaraAsWadeIdle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	if (TrInput & IN_JUMP && TestLaraJumpUp(item, coll))
	{
		item->TargetState = LS_JUMP_PREPARE;
		info->Control.JumpDirection = JumpDirection::Up;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && info->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->TargetState = LS_WADE_FORWARD;
			return;
		}
	}

	if (TrInput & IN_BACK && TestLaraWalkBack(item, coll))
	{
		item->TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->TargetState = LS_TURN_LEFT_SLOW;
		return;
	}
	else if (TrInput & IN_RIGHT)
	{
		item->TargetState = LS_TURN_RIGHT_SLOW;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->TargetState = LS_STEP_RIGHT;
		return;
	}

	item->TargetState = LS_IDLE;
}

// Pseudo-state for idling in swamps.
void PseudoLaraAsSwampIdle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && info->Control.HandStatus == HandStatus::Free &&
			vaultResult.Success)
		{
			item->TargetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TestLaraWadeForwardSwamp(item, coll))
		{
			item->TargetState = LS_WADE_FORWARD;
			return;
		}
	}

	if (TrInput & IN_BACK && TestLaraWalkBackSwamp(item, coll))
	{
		item->TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->TargetState = LS_TURN_LEFT_SLOW;
		return;
	}
	else if (TrInput & IN_RIGHT)
	{
		item->TargetState = LS_TURN_RIGHT_SLOW;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeftSwamp(item, coll))
	{
		item->TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRightSwamp(item, coll))
	{
		item->TargetState = LS_STEP_RIGHT;
		return;
	}

	item->TargetState = LS_IDLE;
}

// State:		LS_IDLE (2)
// Control:		lara_as_idle()
void lara_col_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	item->Airborne = false;
	item->VerticalVelocity = 0;
	info->Control.MoveAngle = (item->Velocity >= 0) ? item->Position.yRot : item->Position.yRot + ANGLE(180.0f);
	coll->Setup.LowerFloorBound = TestEnvironment(ENV_FLAG_SWAMP, item) ? NO_LOWER_BOUND : STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsPit = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.FloorSlopeIsWall = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
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
void lara_as_pose(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	if (TestLaraPose(item, coll))
	{
		if (TrInput & IN_ROLL)
		{
			item->TargetState = LS_ROLL_FORWARD;
			return;
		}

		if (TrInput & IN_WAKE)
		{
			item->TargetState = LS_IDLE;
			return;
		}

		item->TargetState = LS_POSE;
		return;
	}

	item->TargetState = LS_IDLE;
}

// State:		LS_RUN_BACK (5)
// Collision:	lara_col_run_back()
void lara_as_run_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_MED_TURN_MAX)
			info->Control.TurnRate = -LARA_MED_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_MED_TURN_MAX)
			info->Control.TurnRate = LARA_MED_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);
	}

	if (TrInput & IN_ROLL)
	{
		item->TargetState = LS_ROLL_FORWARD;
		return;
	}

	item->TargetState = LS_IDLE;
}

// State:		LS_RUN_BACK (5)
// Control:		lara_as_run_back()
void lara_col_run_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = item->Position.yRot + ANGLE(180.0f);
	item->VerticalVelocity = 0;
	item->Airborne = false;
	coll->Setup.FloorSlopeIsPit = true;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (coll->Middle.Floor > (STEPUP_HEIGHT / 2))
	{
		SetLaraFallBackState(item);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideState(item, coll);
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
void lara_as_turn_right_slow(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.CanLook = (TestEnvironment(ENV_FLAG_SWAMP, item) && info->Control.WaterStatus == WaterStatus::Wade) ? false : true;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		return;
	}

	info->Control.TurnRate += LARA_TURN_RATE;
	if (info->Control.TurnRate < 0)
		info->Control.TurnRate = 0;
	else if (info->Control.TurnRate > LARA_MED_FAST_TURN_MAX)
		info->Control.TurnRate = LARA_MED_FAST_TURN_MAX;

	if (info->Control.WaterStatus == WaterStatus::Wade)
	{
		if (TestEnvironment(ENV_FLAG_SWAMP, item))
			PsuedoLaraAsSwampTurnRightSlow(item, coll);
		else [[likely]]
			PsuedoLaraAsWadeTurnRightSlow(item, coll);

		return;
	}

	if (TrInput & IN_JUMP)
	{
		SetLaraJumpDirection(item, coll);
		if (info->Control.JumpDirection != JumpDirection::None)
		{
			item->TargetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if (TrInput & IN_ROLL)
	{
		item->TargetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(info->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(info->Control.WeaponControl.GunType)))
	{
		item->TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->TargetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->TargetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) [[likely]]
		{
			item->TargetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_RIGHT)
	{
		if (TrInput & IN_WALK) // TODO: This hasn't worked since TR1.
		{
			item->TargetState = LS_TURN_RIGHT_SLOW;

			if (info->Control.TurnRate > LARA_SLOW_TURN_MAX)
				info->Control.TurnRate = LARA_SLOW_TURN_MAX;
		}
		else if (info->Control.TurnRate > LARA_SLOW_MED_TURN_MAX)
			item->TargetState = LS_TURN_RIGHT_FAST;
		else [[likely]]
			item->TargetState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->TargetState = LS_IDLE;
}

// Pseudo-state for turning right slowly in wade-height water.
void PsuedoLaraAsWadeTurnRightSlow(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	if (info->Control.TurnRate > LARA_WADE_TURN_MAX)
		info->Control.TurnRate = LARA_WADE_TURN_MAX;

	if (TrInput & IN_JUMP && TestLaraJumpUp(item, coll))
	{
		item->TargetState = LS_JUMP_PREPARE;
		info->Control.JumpDirection = JumpDirection::Up;
		return;
	}

	if (TrInput & IN_FORWARD && TestLaraRunForward(item, coll))
	{
		item->TargetState = LS_WADE_FORWARD;
		return;
	}
	else if (TrInput & IN_BACK && TestLaraWalkBack(item, coll))
	{
		item->TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_RIGHT)
	{
		item->TargetState = LS_TURN_RIGHT_SLOW;
		return;
	}

	item->TargetState = LS_IDLE;
}

// Pseudo-state for turning right slowly in swamps.
void PsuedoLaraAsSwampTurnRightSlow(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	if (info->Control.TurnRate > LARA_SWAMP_TURN_MAX)
		info->Control.TurnRate = LARA_SWAMP_TURN_MAX;

	if (TrInput & IN_FORWARD && TestLaraWadeForwardSwamp(item, coll))
	{
		item->TargetState = LS_WADE_FORWARD;
		return;
	}
	else if (TrInput & IN_BACK && TestLaraWalkBackSwamp(item, coll))
	{
		item->TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeftSwamp(item, coll))
	{
		item->TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRightSwamp(item, coll))
	{
		item->TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_RIGHT)
	{
		item->TargetState = LS_TURN_RIGHT_SLOW;
		return;
	}

	item->TargetState = LS_IDLE;
}

// State:		LS_TURN_RIGHT_SLOW (6)
// Control:		lara_as_turn_right_slow()
void lara_col_turn_right_slow(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_TURN_LEFT_SLOW (7)
// Collision:	lara_col_turn_left_slow()
void lara_as_turn_left_slow(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.CanLook = (TestEnvironment(ENV_FLAG_SWAMP, item) && info->Control.WaterStatus == WaterStatus::Wade) ? false : true;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		return;
	}

	info->Control.TurnRate -= LARA_TURN_RATE;
	if (info->Control.TurnRate > 0)
		info->Control.TurnRate = 0;
	else if (info->Control.TurnRate < -LARA_MED_FAST_TURN_MAX)
		info->Control.TurnRate = -LARA_MED_FAST_TURN_MAX;

	if (info->Control.WaterStatus == WaterStatus::Wade)
	{
		if (TestEnvironment(ENV_FLAG_SWAMP, item))
			PsuedoLaraAsSwampTurnLeftSlow(item, coll);
		else [[likely]]
			PsuedoLaraAsWadeTurnLeftSlow(item, coll);

		return;
	}

	if (TrInput & IN_JUMP)
	{
		SetLaraJumpDirection(item, coll);
		if (info->Control.JumpDirection != JumpDirection::None)
		{
			item->TargetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if (TrInput & IN_ROLL)
	{
		item->TargetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(info->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(info->Control.WeaponControl.GunType)))
	{
		item->TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->TargetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->TargetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) [[likely]]
		{
			item->TargetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		if (TrInput & IN_WALK)
		{
			item->TargetState = LS_TURN_LEFT_SLOW;

			if (info->Control.TurnRate < -LARA_SLOW_TURN_MAX)
				info->Control.TurnRate = -LARA_SLOW_TURN_MAX;
		}
		else if (info->Control.TurnRate < -LARA_SLOW_MED_TURN_MAX)
			item->TargetState = LS_TURN_LEFT_FAST;
		else [[likely]]
			item->TargetState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->TargetState = LS_IDLE;
}

// Pseudo-state for turning left slowly in wade-height water.
void PsuedoLaraAsWadeTurnLeftSlow(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	if (info->Control.TurnRate < -LARA_WADE_TURN_MAX)
		info->Control.TurnRate = -LARA_WADE_TURN_MAX;

	if (TrInput & IN_JUMP && TestLaraJumpUp(item, coll))
	{
		item->TargetState = LS_JUMP_PREPARE;
		info->Control.JumpDirection = JumpDirection::Up;
		return;
	}

	if (TrInput & IN_FORWARD && TestLaraRunForward(item, coll))
	{
		item->TargetState = LS_WADE_FORWARD;
		return;
	}
	else if (TrInput & IN_BACK && TestLaraWalkBack(item, coll))
	{
		item->TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->TargetState = LS_TURN_LEFT_SLOW;
		return;
	}

	item->TargetState = LS_IDLE;
}

// Pseudo-state for turning left slowly in swamps.
void PsuedoLaraAsSwampTurnLeftSlow(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	if (info->Control.TurnRate < -LARA_SWAMP_TURN_MAX)
		info->Control.TurnRate = -LARA_SWAMP_TURN_MAX;

	if (TrInput & IN_FORWARD && TestLaraWadeForwardSwamp(item, coll))
	{
		item->TargetState = LS_WADE_FORWARD;
		return;
	}
	else if (TrInput & IN_BACK && TestLaraWalkBack(item, coll))
	{
		item->TargetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->TargetState = LS_TURN_LEFT_SLOW;
		return;
	}

	item->TargetState = LS_IDLE;
}

// State:		LS_TURN_LEFT_SLOW (7)
// Control:		lara_as_turn_left_slow()
void lara_col_turn_left_slow(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_turn_right_slow(item, coll);
}

// State:		LS_DEATH (8)
// Collision:	lara_col_death()
void lara_as_death(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.CanLook = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	if (BinocularRange)
	{
		BinocularRange = 0;
		LaserSight = false;
		AlterFOV(ANGLE(80.0f));
		item->MeshBits = -1;
		info->Control.IsBusy = false;
	}
}

// State:		LS_DEATH (8)
// Control:	lara_as_death()
void lara_col_death(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = item->Position.yRot;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.Radius = LARA_RAD_DEATH;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	StopSoundEffect(SFX_TR4_LARA_FALL);
	item->HitPoints = -1;
	info->Air = -1;

	ShiftItem(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_SPLAT (12)
// Collision:	lara_col_splat()
void lara_as_splat(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.CanLook = false;
}

// State:		LS_SPLAT (12)
// Control:		lara_as_splat()
void lara_col_splat(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = item->Position.yRot;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.FloorSlopeIsPit = true;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	ShiftItem(item, coll);

	if (abs(coll->Middle.Floor) <= CLICK(1))
		LaraSnapToHeight(item, coll);
}

// State:		LS_WALK_BACK (16)
// Collision:	lara_col_walk_back()
void lara_as_walk_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.CanLook = (TestEnvironment(ENV_FLAG_SWAMP, item) && info->Control.WaterStatus == WaterStatus::Wade) ? false : true;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		return;
	}

	if (info->Control.IsMoving)
		return;

	if (TestEnvironment(ENV_FLAG_SWAMP, item) &&
		info->Control.WaterStatus == WaterStatus::Wade)
	{
		PseudoLaraAsSwampWalkBack(item, coll);
		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			info->Control.TurnRate = -LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);

	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_SLOW_TURN_MAX)
			info->Control.TurnRate = LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);
	}

	if (TrInput & IN_BACK &&
		(TrInput & IN_WALK || info->Control.WaterStatus == WaterStatus::Wade))
	{
		item->TargetState = LS_WALK_BACK;
		return;
	}

	item->TargetState = LS_IDLE;
}

// Pseudo-state for walking back in swamps.
void PseudoLaraAsSwampWalkBack(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_SLOW_TURN_MAX / 3)
			info->Control.TurnRate = -LARA_SLOW_TURN_MAX / 3;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 3);

	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_SLOW_TURN_MAX / 3)
			info->Control.TurnRate = LARA_SLOW_TURN_MAX / 3;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 3);
	}

	if (TrInput & IN_BACK)
	{
		item->TargetState = LS_WALK_BACK;
		return;
	}

	item->TargetState = LS_IDLE;
}

// State:		LS_WALK_BACK (16)
// Control:		lara_as_walk_back()
void lara_col_walk_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = item->Position.yRot + ANGLE(180.0f);
	item->Airborne = false;
	item->VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = (info->Control.WaterStatus == WaterStatus::Wade) ? NO_LOWER_BOUND : STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsPit = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.FloorSlopeIsWall = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.DeathFlagIsPit = true;
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
void lara_as_turn_right_fast(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		return;
	}

	info->Control.TurnRate += LARA_TURN_RATE;
	if (info->Control.TurnRate < LARA_MED_TURN_MAX)
		info->Control.TurnRate = LARA_MED_TURN_MAX;
	else if (info->Control.TurnRate > LARA_FAST_TURN_MAX)
		info->Control.TurnRate = LARA_FAST_TURN_MAX;

	if (TrInput & IN_JUMP)
	{
		SetLaraJumpDirection(item, coll);
		if (info->Control.JumpDirection != JumpDirection::None)
		{
			item->TargetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if (TrInput & IN_ROLL &&
		info->Control.WaterStatus != WaterStatus::Wade)
	{
		item->TargetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(info->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(info->Control.WeaponControl.GunType)) &&
		info->Control.WaterStatus != WaterStatus::Wade)
	{
		item->TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (info->Control.WaterStatus == WaterStatus::Wade)		// Should not be possible, but here for security.
		{
			if (TestLaraRunForward(item, coll))
			{
				item->TargetState = LS_WADE_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->TargetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->TargetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) [[likely]]
		{
			item->TargetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->TargetState = LS_STEP_RIGHT;
		return;
	}

	// TODO: Hold WALK to slow down again.
	if (TrInput & IN_RIGHT)
	{
		item->TargetState = LS_TURN_RIGHT_FAST;
		return;
	}

	item->TargetState = LS_IDLE;
}

// State:		LS_TURN_RIGHT_FAST (20)
// Control:		lara_as_turn_right_fast()
void lara_col_turn_right_fast(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_TURN_LEFT_FAST (152)
// Collision:	lara_col_turn_left_fast()
void lara_as_turn_left_fast(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		return;
	}

	info->Control.TurnRate -= LARA_TURN_RATE;
	if (info->Control.TurnRate > -LARA_MED_TURN_MAX)
		info->Control.TurnRate = -LARA_MED_TURN_MAX;
	else if (info->Control.TurnRate < -LARA_FAST_TURN_MAX)
		info->Control.TurnRate = -LARA_FAST_TURN_MAX;

	if (TrInput & IN_JUMP)
	{
		SetLaraJumpDirection(item, coll);
		if (info->Control.JumpDirection != JumpDirection::None)
		{
			item->TargetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if (TrInput & IN_ROLL &&
		info->Control.WaterStatus != WaterStatus::Wade)
	{
		item->TargetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(info->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(info->Control.WeaponControl.GunType)) &&
		info->Control.WaterStatus != WaterStatus::Wade)
	{
		item->TargetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (info->Control.WaterStatus == WaterStatus::Wade)		// Should not be possible, but here for security.
		{
			if (TestLaraRunForward(item, coll))
			{
				item->TargetState = LS_WADE_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->TargetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->TargetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->TargetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->TargetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) [[likely]]
		{
			item->TargetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP && 	TestLaraStepLeft(item, coll))
	{
		item->TargetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->TargetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->TargetState = LS_TURN_LEFT_FAST;
		return;
	}

	item->TargetState = LS_IDLE;
}

// State:		LS_TURN_LEFT_FAST (152)
// Control:		lara_as_turn_left_fast()
void lara_col_turn_left_fast(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_idle(item, coll);
}

// State:		LS_STEP_RIGHT (21)
// Collision:	lara_col_step_right()
void lara_as_step_right(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.CanLook = false;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		return;
	}

	if (info->Control.IsMoving)
		return;

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

	if (TrInput & IN_RSTEP)
	{
		item->TargetState = LS_STEP_RIGHT;
		return;
	}
	
	item->TargetState = LS_IDLE;
}

// State:		LS_STEP_RIGHT (21)
// Control:		lara_as_step_right()
void lara_col_step_right(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = item->Position.yRot + ANGLE(90.0f);
	item->Airborne = false;
	item->VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = (info->Control.WaterStatus == WaterStatus::Wade) ? NO_LOWER_BOUND : CLICK(0.8f);
	coll->Setup.UpperFloorBound = -CLICK(0.8f);
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsPit = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.FloorSlopeIsWall = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.DeathFlagIsPit = true;
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
		return;
	}

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (TestLaraStep(item, coll) || TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_STEP_LEFT (22)
// Collision:	lara_col_step_left()
void lara_as_step_left(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.CanLook = false;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		return;
	}

	if (info->Control.IsMoving)
		return;

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

	if (TrInput & IN_LSTEP)
	{
		item->TargetState = LS_STEP_LEFT;
		return;
	}

	item->TargetState = LS_IDLE;
}

// State:		LS_STEP_LEFT (22)
// Control:		lara_as_step_left()
void lara_col_step_left(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = item->Position.yRot - ANGLE(90.0f);
	item->Airborne = false;
	item->VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = (info->Control.WaterStatus == WaterStatus::Wade) ? NO_LOWER_BOUND : CLICK(0.8f);
	coll->Setup.UpperFloorBound = -CLICK(0.8f);
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsPit = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.FloorSlopeIsWall = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.DeathFlagIsPit = true;
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
		return;
	}

	if (LaraDeflectEdge(item, coll))
		LaraCollideStop(item, coll);

	if (TestLaraStep(item, coll) || TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_ROLL_BACK (23)
// Collision:	lara_col_roll_back()
void lara_as_roll_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.CanLook = false;

	if (TrInput & IN_ROLL)
	{
		item->TargetState = LS_ROLL_BACK;
		return;
	}

	item->TargetState = LS_IDLE;
}

// State:		LS_ROLL_BACK (23)
// Control:		lara_as_roll_back()
void lara_col_roll_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = item->Position.yRot + ANGLE(180.0f);
	item->Airborne = false;
	item->VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	Camera.laraNode = 0;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		SetLaraSlideState(item, coll);
		return;
	}

	if (coll->Middle.Floor > (STEPUP_HEIGHT / 2)) // Was 200.
	{
		SetLaraFallBackState(item);
		return;
	}

	ShiftItem(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_ROLL_FORWARD (45)
// Collision:	lara_col_roll_forward()
void lara_as_roll_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.CanLook = false;

	// TODO: Change swandive roll anim state to something sensible.
	if (TrInput & IN_FORWARD &&
		item->AnimNumber == LA_SWANDIVE_ROLL) // Hack.
	{
		item->TargetState = LS_RUN_FORWARD;
		return;
	}

	if (TrInput & IN_ROLL)
	{
		item->TargetState = LS_ROLL_FORWARD;
		return;
	}

	item->TargetState = LS_IDLE;
}

// State:		LS_ROLL_FORWARD (45)
// Control:		lara_as_roll_forward()
void lara_col_roll_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = item->Position.yRot;
	item->Airborne = false;
	item->VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
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
void lara_as_wade_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.CanLook = (TestEnvironment(ENV_FLAG_SWAMP, item) && info->Control.WaterStatus == WaterStatus::Wade) ? false : true;
	Camera.targetElevation = -ANGLE(22.0f);

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_IDLE;
		return;
	}

	if (TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		PseudoLaraAsSwampWadeForward(item, coll);
		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_FAST_TURN_MAX)
			info->Control.TurnRate = -LARA_FAST_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE / 2);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_FAST_TURN_MAX)
			info->Control.TurnRate = LARA_FAST_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE / 2);
	}

	if (TrInput & IN_FORWARD)
	{
		if (info->Control.WaterStatus == WaterStatus::Dry)
			item->TargetState = LS_RUN_FORWARD;
		else [[likely]]
			item->TargetState = LS_WADE_FORWARD;

		return;
	}

	item->TargetState = LS_IDLE;
}

// Pseudo-state for wading in swamps.
void PseudoLaraAsSwampWadeForward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_SWAMP_TURN_MAX)
			info->Control.TurnRate = -LARA_SWAMP_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 5 * 3, LARA_LEAN_RATE / 3);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_SWAMP_TURN_MAX)
			info->Control.TurnRate = LARA_SWAMP_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 5 * 3, LARA_LEAN_RATE / 3);
	}

	if (TrInput & IN_FORWARD)
	{
		item->TargetState = LS_WADE_FORWARD;
		return;
	}

	item->TargetState = LS_IDLE;
}

// State:		LS_WADE_FORWARD (65)
// Control:		lara_as_wade_forward()
void lara_col_wade_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = item->Position.yRot;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
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
		item->Position.zRot = 0;

		if (coll->Front.Floor < -STEPUP_HEIGHT &&
			!coll->Front.FloorSlope &&
			!TestEnvironment(ENV_FLAG_SWAMP, item)) // TODO: Check this.
		{
			item->TargetState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->AnimNumber]))
				return;
		}

		LaraCollideStop(item, coll);
	}

	if (TestLaraStep(item, coll) || TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_SPRINT (73)
// Collision:	lara_col_sprint()
void lara_as_sprint(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->SprintEnergy--;

	info->Control.Count.RunJump++;
	if (info->Control.Count.RunJump > LARA_RUN_JUMP_TIME)
		info->Control.Count.RunJump = LARA_RUN_JUMP_TIME;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			info->Control.TurnRate = -LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_SLOW_TURN_MAX)
			info->Control.TurnRate = LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE);
	}

	if (TrInput & IN_JUMP)
	{
		item->TargetState = LS_SPRINT_DIVE;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(info->Control.HandStatus == HandStatus::Free || !IsStandingWeapon(info->Control.WeaponControl.GunType)))
	{
		item->TargetState = LS_CROUCH_IDLE;
		return;
	}

	// TODO: Supposedly there is a bug wherein sprinting into the boundary between shallow and deep water
	// while meeting some condition allows Lara to run around in the water room. Investigate. @Sezz 2021.09.29
	if (TrInput & IN_FORWARD)
	{
		if (info->Control.WaterStatus == WaterStatus::Wade)
			item->TargetState = LS_RUN_FORWARD;	// TODO: Dispatch to wade forward state directly. @Sezz 2021.09.29
		else if (TrInput & IN_WALK)
			item->TargetState = LS_WALK_FORWARD;
		else if (TrInput & IN_SPRINT && info->SprintEnergy > 0) [[likely]]
			item->TargetState = LS_SPRINT;
		else
			item->TargetState = LS_RUN_FORWARD;

		return;
	}

	item->TargetState = LS_IDLE;
}

// State:		LS_SPRINT (73)
// Control:		lara_as_sprint()
void lara_col_sprint(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = item->Position.yRot;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
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
		return;
	}

	if (LaraDeflectEdge(item, coll))
	{
		item->Position.zRot = 0;

		if (coll->HitTallObject || TestLaraSplat(item, OFFSET_RADIUS(coll->Setup.Radius), -CLICK(2.5f)))
		{
			item->TargetState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->AnimNumber]))
			{
				item->ActiveState = LS_SPLAT;
				return;
			}
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
void lara_as_sprint_dive(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.Count.RunJump++;
	if (info->Control.Count.RunJump > LARA_RUN_JUMP_TIME)
		info->Control.Count.RunJump = LARA_RUN_JUMP_TIME;

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_SLOW_TURN_MAX)
			info->Control.TurnRate = -LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, -(LARA_LEAN_MAX * 3) / 5, LARA_LEAN_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_SLOW_TURN_MAX)
			info->Control.TurnRate = LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, (LARA_LEAN_MAX * 3) / 5, LARA_LEAN_RATE);
	}

	// TODO: Make this state a true jump?
	if (item->TargetState != LS_DEATH &&
		item->TargetState != LS_IDLE &&
		item->TargetState != LS_RUN_FORWARD &&
		item->VerticalVelocity >= LARA_FREEFALL_SPEED)
	{
		item->TargetState = LS_FREEFALL;
	}
}

// State:		LS_SPRINT_DIVE (74)
// Control:		lara_col_sprint_dive()
void lara_col_sprint_dive(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = (item->Velocity >= 0) ? item->Position.yRot : item->Position.yRot + ANGLE(180.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	LaraDeflectEdgeJump(item, coll);

	if (TestLaraFall(item, coll))
	{
		SetLaraFallState(item);
		return;
	}

	if (item->Velocity < 0)
		info->Control.MoveAngle = item->Position.yRot; // ???

	if (coll->Middle.Floor <= 0 && item->VerticalVelocity > 0)
	{
		DoLaraFallDamage(item);

		if (item->HitPoints <= 0) // TODO: It seems Core wanted to make the sprint dive a true jump.
			item->TargetState = LS_DEATH;
		else if (!(TrInput & IN_FORWARD) || TrInput & IN_WALK || info->Control.WaterStatus == WaterStatus::Wade)
			item->TargetState = LS_IDLE;
		else
			item->TargetState = LS_RUN_FORWARD;

		item->Airborne = false;
		item->VerticalVelocity = 0;
		item->Position.yPos += coll->Middle.Floor;
		item->Velocity = 0;

		AnimateLara(item);
	}

	ShiftItem(item, coll);

	if (TestLaraStep(item, coll))
	{
		DoLaraStep(item, coll);
		return;
	}
}
