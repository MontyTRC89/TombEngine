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
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsPit = true;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = info->moveAngle;
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
	LaraInfo*& info = item->data;

	info->look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd - 1)
	{
		info->gunStatus = LG_HANDS_FREE;

		if (UseForcedFixedCamera)
			UseForcedFixedCamera = 0;
	}
}

void lara_as_controlled_no_look(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
}

// State:	LS_VAULT (164)
// Control:	lara_as_null()
void lara_col_vault(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	EaseOutLaraHeight(item, info->projectedFloorHeight - item->pos.yPos);
}

// State:	LS_AUTO_JUMP (62)
// Control:	lara_as_null()
void lara_col_auto_jump(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	
	info->calcJumpVelocity = -3 - sqrt(-9600 - 12 * std::max<int>(info->projectedFloorHeight - item->pos.yPos, -CLICK(7.5f)));
}

// ---------------
// BASIC MOVEMENT:
// ---------------

// State:		LS_WALK_FORWARD (0)
// Collision:	lara_col_walk_forward()
void lara_as_walk_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->runJumpCount++;
	if (info->runJumpCount > LARA_RUN_JUMP_TIME - 4)
		info->runJumpCount = LARA_RUN_JUMP_TIME - 4;

	if (item->hitPoints <= 0)
	{
		item->targetState = LS_DEATH;
		return;
	}

	// TODO: Implement item alignment properly someday. @Sezz 2021.11.01
	if (info->isMoving)
		return;

	// TODO: If stopping and holding WALK without FORWARD, Lara can't turn. @Sezz 2021.10.09
	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_MED_TURN_MAX)
			info->turnRate = -LARA_MED_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 6);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_MED_TURN_MAX)
			info->turnRate = LARA_MED_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 6);
	}

	if (TrInput & IN_FORWARD)
	{
		if (info->waterStatus == LW_WADE)
			item->targetState = LS_WADE_FORWARD;
		else if (TrInput & IN_WALK) [[likely]]
			item->targetState = LS_WALK_FORWARD;
		else
			item->targetState = LS_RUN_FORWARD;

		return;
	}

	item->targetState = LS_IDLE;
}

// State:		LA_WALK_FORWARD (0)
// Control:		lara_as_walk_forward()
void lara_col_walk_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	item->Airborne = false;
	item->VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.FloorSlopeIsPit = true;
	coll->Setup.DeathFlagIsPit = true;
	coll->Setup.ForwardAngle = info->moveAngle;
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
		item->targetState = LS_SPLAT;
		if (GetChange(item, &g_Level.Anims[item->animNumber]))
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
	LaraInfo*& info = item->data;

	info->runJumpCount++;
	if (info->runJumpCount > LARA_RUN_JUMP_TIME)
		info->runJumpCount = LARA_RUN_JUMP_TIME;

	if (item->hitPoints <= 0)
	{
		item->targetState = LS_DEATH;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_FAST_TURN_MAX)
			info->turnRate = -LARA_FAST_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_FAST_TURN_MAX)
			info->turnRate = LARA_FAST_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE);
	}

	if ((TrInput & IN_JUMP || info->runJumpQueued) &&
		info->waterStatus != LW_WADE)
	{
		if (info->runJumpCount >= LARA_RUN_JUMP_TIME &&
			TestLaraRunJumpForward(item, coll))
		{
			item->targetState = LS_JUMP_FORWARD;
			return;
		}

		SetLaraRunJumpQueue(item, coll);
	}

	if (TrInput & IN_SPRINT && info->sprintTimer &&
		info->waterStatus != LW_WADE)
	{
		item->targetState = LS_SPRINT;
		return;
	}

	// TODO: Control settings option to enable/disable FORWARD+BACK as roll input.
	if ((TrInput & (IN_ROLL | IN_FORWARD & IN_BACK)) && !info->runJumpQueued &&
		info->waterStatus != LW_WADE)
	{
		item->targetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(info->gunStatus == LG_HANDS_FREE || !IsStandingWeapon(info->gunType)) &&
		info->waterStatus != LW_WADE)
	{
		item->targetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (info->waterStatus == LW_WADE)
			item->targetState = LS_WADE_FORWARD;
		else if (TrInput & IN_WALK)
			item->targetState = LS_WALK_FORWARD;
		else [[likely]]
			item->targetState = LS_RUN_FORWARD;

		return;
	}

	item->targetState = LS_IDLE;
}

// State:		LS_RUN_FORWARD (1)
// Control:		lara_as_run_forward()
void lara_col_run_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = info->moveAngle;
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
		item->pos.zRot = 0;

		if (coll->HitTallObject || TestLaraSplat(item, OFFSET_RADIUS(coll->Setup.Radius), -CLICK(2.5f)))
		{
			item->targetState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->animNumber]))
			{
				item->activeState = LS_SPLAT;
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
	LaraInfo*& info = item->data;

	info->look = ((TestEnvironment(ENV_FLAG_SWAMP, item) && info->waterStatus == LW_WADE) || item->animNumber == LA_SWANDIVE_ROLL) ? false : true;

	if (item->hitPoints <= 0)
	{
		item->targetState = LS_DEATH;
		return;
	}

	// Handles waterskin and clockwork beetle.
	// TODO: Hardcoding.
	if (UseSpecialItem(item))
		return;

	if (TrInput & IN_LOOK && info->look)
		LookUpDown();

	if (TrInput & IN_LEFT &&
		!(TrInput & IN_JUMP))	// JUMP locks y rotation.
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN_MAX)
			info->turnRate = -LARA_SLOW_TURN_MAX;
	}
	else if (TrInput & IN_RIGHT &&
		!(TrInput & IN_JUMP))
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN_MAX)
			info->turnRate = LARA_SLOW_TURN_MAX;
	}

	if (info->waterStatus == LW_WADE)
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
		if (info->jumpDirection != JumpDirection::NoDirection)
			item->targetState = LS_JUMP_PREPARE;

		return;
	}

	if (TrInput & IN_ROLL)
	{
		item->targetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(info->gunStatus == LG_HANDS_FREE || !IsStandingWeapon(info->gunType)))
	{
		item->targetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && info->gunStatus == LG_HANDS_FREE &&
			vaultResult.Success)
		{
			item->targetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->targetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->targetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->targetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->targetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) [[likely]]
		{
			item->targetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->targetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->targetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		if (TrInput & IN_SPRINT ||
			info->turnRate <= -LARA_SLOW_TURN_MAX ||
			(info->gunStatus == LG_READY && info->gunType != WEAPON_TORCH) ||
			(info->gunStatus == LG_DRAW_GUNS && info->gunType != WEAPON_FLARE))
		{
			item->targetState = LS_TURN_LEFT_FAST;
		}
		else [[likely]]
			item->targetState = LS_TURN_LEFT_SLOW;

		return;
	}
	else if (TrInput & IN_RIGHT)
	{
		if (TrInput & IN_SPRINT ||
			info->turnRate >= LARA_SLOW_TURN_MAX ||
			(info->gunStatus == LG_READY && info->gunType != WEAPON_TORCH) ||
			(info->gunStatus == LG_DRAW_GUNS && info->gunType != WEAPON_FLARE))
		{
			item->targetState = LS_TURN_RIGHT_FAST;
		}
		else [[likely]]
			item->targetState = LS_TURN_RIGHT_SLOW;

		return;
	}

	// TODO: Without animation blending, the AFK state's
	// movement lock will be rather obnoxious.
	// Adding some idle breathing would also be nice. @Sezz 2021.10.31
	if (info->poseCount >= LARA_POSE_TIME && TestLaraPose(item, coll) &&
		g_GameFlow->Animations.Pose)
	{
		item->targetState = LS_POSE;
		return;
	}

	item->targetState = LS_IDLE;
}

// TODO: Future-proof for rising water.
// TODO: Make these into true states someday? It may take a bit of work. @Sezz 2021.10.13
// Pseudo-state for idling in wade-height water.
void PseudoLaraAsWadeIdle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (TrInput & IN_JUMP && TestLaraJumpUp(item, coll))
	{
		item->targetState = LS_JUMP_PREPARE;
		info->jumpDirection = JumpDirection::Up;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && info->gunStatus == LG_HANDS_FREE &&
			vaultResult.Success)
		{
			item->targetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->targetState = LS_WADE_FORWARD;
			return;
		}
	}

	if (TrInput & IN_BACK && TestLaraWalkBack(item, coll))
	{
		item->targetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->targetState = LS_TURN_LEFT_SLOW;
		return;
	}
	else if (TrInput & IN_RIGHT)
	{
		item->targetState = LS_TURN_RIGHT_SLOW;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->targetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->targetState = LS_STEP_RIGHT;
		return;
	}

	item->targetState = LS_IDLE;
}

// Pseudo-state for idling in swamps.
void PseudoLaraAsSwampIdle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (TrInput & IN_FORWARD)
	{
		auto vaultResult = TestLaraVault(item, coll);

		if (TrInput & IN_ACTION && info->gunStatus == LG_HANDS_FREE &&
			vaultResult.Success)
		{
			item->targetState = vaultResult.TargetState;
			SetLaraVault(item, coll, vaultResult);
			return;
		}
		else if (TestLaraWadeForwardSwamp(item, coll))
		{
			item->targetState = LS_WADE_FORWARD;
			return;
		}
	}

	if (TrInput & IN_BACK && TestLaraWalkBackSwamp(item, coll))
	{
		item->targetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->targetState = LS_TURN_LEFT_SLOW;
		return;
	}
	else if (TrInput & IN_RIGHT)
	{
		item->targetState = LS_TURN_RIGHT_SLOW;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeftSwamp(item, coll))
	{
		item->targetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRightSwamp(item, coll))
	{
		item->targetState = LS_STEP_RIGHT;
		return;
	}

	item->targetState = LS_IDLE;
}

// State:		LS_IDLE (2)
// Control:		lara_as_idle()
void lara_col_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	item->Airborne = false;
	item->VerticalVelocity = 0;
	info->moveAngle = (item->Velocity >= 0) ? item->pos.yRot : item->pos.yRot + ANGLE(180.0f);
	coll->Setup.LowerFloorBound = TestEnvironment(ENV_FLAG_SWAMP, item) ? NO_LOWER_BOUND : STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsPit = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.FloorSlopeIsWall = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.ForwardAngle = info->moveAngle;
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
	if (item->hitPoints <= 0)
	{
		item->targetState = LS_DEATH;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	if (TestLaraPose(item, coll))
	{
		if (TrInput & IN_ROLL)
		{
			item->targetState = LS_ROLL_FORWARD;
			return;
		}

		if (TrInput & IN_WAKE)
		{
			item->targetState = LS_IDLE;
			return;
		}

		item->targetState = LS_POSE;
		return;
	}

	item->targetState = LS_IDLE;
}

// State:		LS_RUN_BACK (5)
// Collision:	lara_col_run_back()
void lara_as_run_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_MED_TURN_MAX)
			info->turnRate = -LARA_MED_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_MED_TURN_MAX)
			info->turnRate = LARA_MED_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);
	}

	if (TrInput & IN_ROLL)
	{
		item->targetState = LS_ROLL_FORWARD;
		return;
	}

	item->targetState = LS_IDLE;
}

// State:		LS_RUN_BACK (5)
// Control:		lara_as_run_back()
void lara_col_run_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot + ANGLE(180.0f);
	item->VerticalVelocity = 0;
	item->Airborne = false;
	coll->Setup.FloorSlopeIsPit = true;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.ForwardAngle = info->moveAngle;
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
	LaraInfo*& info = item->data;

	info->look = (TestEnvironment(ENV_FLAG_SWAMP, item) && info->waterStatus == LW_WADE) ? false : true;

	if (item->hitPoints <= 0)
	{
		item->targetState = LS_DEATH;
		return;
	}

	info->turnRate += LARA_TURN_RATE;
	if (info->turnRate < 0)
		info->turnRate = 0;
	else if (info->turnRate > LARA_MED_FAST_TURN_MAX)
		info->turnRate = LARA_MED_FAST_TURN_MAX;

	if (info->waterStatus == LW_WADE)
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
		if (info->jumpDirection != JumpDirection::NoDirection)
		{
			item->targetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if (TrInput & IN_ROLL)
	{
		item->targetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(info->gunStatus == LG_HANDS_FREE || !IsStandingWeapon(info->gunType)))
	{
		item->targetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->targetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->targetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->targetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->targetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) [[likely]]
		{
			item->targetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->targetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->targetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_RIGHT)
	{
		if (TrInput & IN_WALK) // TODO: This hasn't worked since TR1.
		{
			item->targetState = LS_TURN_RIGHT_SLOW;

			if (info->turnRate > LARA_SLOW_TURN_MAX)
				info->turnRate = LARA_SLOW_TURN_MAX;
		}
		else if (info->turnRate > LARA_SLOW_MED_TURN_MAX)
			item->targetState = LS_TURN_RIGHT_FAST;
		else [[likely]]
			item->targetState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->targetState = LS_IDLE;
}

// Pseudo-state for turning right slowly in wade-height water.
void PsuedoLaraAsWadeTurnRightSlow(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (info->turnRate > LARA_WADE_TURN_MAX)
		info->turnRate = LARA_WADE_TURN_MAX;

	if (TrInput & IN_JUMP && TestLaraJumpUp(item, coll))
	{
		item->targetState = LS_JUMP_PREPARE;
		info->jumpDirection = JumpDirection::Up;
		return;
	}

	if (TrInput & IN_FORWARD && TestLaraRunForward(item, coll))
	{
		item->targetState = LS_WADE_FORWARD;
		return;
	}
	else if (TrInput & IN_BACK && TestLaraWalkBack(item, coll))
	{
		item->targetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->targetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->targetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_RIGHT)
	{
		item->targetState = LS_TURN_RIGHT_SLOW;
		return;
	}

	item->targetState = LS_IDLE;
}

// Pseudo-state for turning right slowly in swamps.
void PsuedoLaraAsSwampTurnRightSlow(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (info->turnRate > LARA_SWAMP_TURN_MAX)
		info->turnRate = LARA_SWAMP_TURN_MAX;

	if (TrInput & IN_FORWARD && TestLaraWadeForwardSwamp(item, coll))
	{
		item->targetState = LS_WADE_FORWARD;
		return;
	}
	else if (TrInput & IN_BACK && TestLaraWalkBackSwamp(item, coll))
	{
		item->targetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeftSwamp(item, coll))
	{
		item->targetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRightSwamp(item, coll))
	{
		item->targetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_RIGHT)
	{
		item->targetState = LS_TURN_RIGHT_SLOW;
		return;
	}

	item->targetState = LS_IDLE;
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
	LaraInfo*& info = item->data;

	info->look = (TestEnvironment(ENV_FLAG_SWAMP, item) && info->waterStatus == LW_WADE) ? false : true;

	if (item->hitPoints <= 0)
	{
		item->targetState = LS_DEATH;
		return;
	}

	info->turnRate -= LARA_TURN_RATE;
	if (info->turnRate > 0)
		info->turnRate = 0;
	else if (info->turnRate < -LARA_MED_FAST_TURN_MAX)
		info->turnRate = -LARA_MED_FAST_TURN_MAX;

	if (info->waterStatus == LW_WADE)
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
		if (info->jumpDirection != JumpDirection::NoDirection)
		{
			item->targetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if (TrInput & IN_ROLL)
	{
		item->targetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(info->gunStatus == LG_HANDS_FREE || !IsStandingWeapon(info->gunType)))
	{
		item->targetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->targetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->targetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->targetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->targetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) [[likely]]
		{
			item->targetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->targetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->targetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		if (TrInput & IN_WALK)
		{
			item->targetState = LS_TURN_LEFT_SLOW;

			if (info->turnRate < -LARA_SLOW_TURN_MAX)
				info->turnRate = -LARA_SLOW_TURN_MAX;
		}
		else if (info->turnRate < -LARA_SLOW_MED_TURN_MAX)
			item->targetState = LS_TURN_LEFT_FAST;
		else [[likely]]
			item->targetState = LS_TURN_RIGHT_SLOW;

		return;
	}

	item->targetState = LS_IDLE;
}

// Pseudo-state for turning left slowly in wade-height water.
void PsuedoLaraAsWadeTurnLeftSlow(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (info->turnRate < -LARA_WADE_TURN_MAX)
		info->turnRate = -LARA_WADE_TURN_MAX;

	if (TrInput & IN_JUMP && TestLaraJumpUp(item, coll))
	{
		item->targetState = LS_JUMP_PREPARE;
		info->jumpDirection = JumpDirection::Up;
		return;
	}

	if (TrInput & IN_FORWARD && TestLaraRunForward(item, coll))
	{
		item->targetState = LS_WADE_FORWARD;
		return;
	}
	else if (TrInput & IN_BACK && TestLaraWalkBack(item, coll))
	{
		item->targetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->targetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->targetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->targetState = LS_TURN_LEFT_SLOW;
		return;
	}

	item->targetState = LS_IDLE;
}

// Pseudo-state for turning left slowly in swamps.
void PsuedoLaraAsSwampTurnLeftSlow(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (info->turnRate < -LARA_SWAMP_TURN_MAX)
		info->turnRate = -LARA_SWAMP_TURN_MAX;

	if (TrInput & IN_FORWARD && TestLaraWadeForwardSwamp(item, coll))
	{
		item->targetState = LS_WADE_FORWARD;
		return;
	}
	else if (TrInput & IN_BACK && TestLaraWalkBack(item, coll))
	{
		item->targetState = LS_WALK_BACK;
		return;
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->targetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->targetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->targetState = LS_TURN_LEFT_SLOW;
		return;
	}

	item->targetState = LS_IDLE;
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
	LaraInfo*& info = item->data;

	info->look = false;
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;

	if (BinocularRange)
	{
		BinocularRange = 0;
		LaserSight = false;
		AlterFOV(ANGLE(80.0f));
		item->meshBits = -1;
		info->busy = false;
	}
}

// State:		LS_DEATH (8)
// Control:	lara_as_death()
void lara_col_death(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.Radius = LARA_RAD_DEATH;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);

	StopSoundEffect(SFX_TR4_LARA_FALL);
	item->hitPoints = -1;
	info->air = -1;

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
	LaraInfo*& info = item->data;

	info->look = false;
}

// State:		LS_SPLAT (12)
// Control:		lara_as_splat()
void lara_col_splat(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.FloorSlopeIsPit = true;
	coll->Setup.LowerFloorBound = STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);

	ShiftItem(item, coll);

	if (abs(coll->Middle.Floor) <= CLICK(1))
		LaraSnapToHeight(item, coll);
}

// State:		LS_WALK_BACK (16)
// Collision:	lara_col_walk_back()
void lara_as_walk_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->look = (TestEnvironment(ENV_FLAG_SWAMP, item) && info->waterStatus == LW_WADE) ? false : true;

	if (item->hitPoints <= 0)
	{
		item->targetState = LS_DEATH;
		return;
	}

	if (info->isMoving)
		return;

	if (TestEnvironment(ENV_FLAG_SWAMP, item) &&
		info->waterStatus == LW_WADE)
	{
		PseudoLaraAsSwampWalkBack(item, coll);
		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN_MAX)
			info->turnRate = -LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);

	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN_MAX)
			info->turnRate = LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 4);
	}

	if (TrInput & IN_BACK &&
		(TrInput & IN_WALK || info->waterStatus == LW_WADE))
	{
		item->targetState = LS_WALK_BACK;
		return;
	}

	item->targetState = LS_IDLE;
}

// Pseudo-state for walking back in swamps.
void PseudoLaraAsSwampWalkBack(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN_MAX / 3)
			info->turnRate = -LARA_SLOW_TURN_MAX / 3;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 3);

	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN_MAX / 3)
			info->turnRate = LARA_SLOW_TURN_MAX / 3;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 3, LARA_LEAN_RATE / 3);
	}

	if (TrInput & IN_BACK)
	{
		item->targetState = LS_WALK_BACK;
		return;
	}

	item->targetState = LS_IDLE;
}

// State:		LS_WALK_BACK (16)
// Control:		lara_as_walk_back()
void lara_col_walk_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot + ANGLE(180.0f);
	item->Airborne = false;
	item->VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = (info->waterStatus == LW_WADE) ? NO_LOWER_BOUND : STEPUP_HEIGHT;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsPit = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.FloorSlopeIsWall = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.DeathFlagIsPit = true;
	coll->Setup.ForwardAngle = info->moveAngle;
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
	LaraInfo*& info = item->data;

	if (item->hitPoints <= 0)
	{
		item->targetState = LS_DEATH;
		return;
	}

	info->turnRate += LARA_TURN_RATE;
	if (info->turnRate < LARA_MED_TURN_MAX)
		info->turnRate = LARA_MED_TURN_MAX;
	else if (info->turnRate > LARA_FAST_TURN_MAX)
		info->turnRate = LARA_FAST_TURN_MAX;

	if (TrInput & IN_JUMP)
	{
		SetLaraJumpDirection(item, coll);
		if (info->jumpDirection != JumpDirection::NoDirection)
		{
			item->targetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if (TrInput & IN_ROLL &&
		info->waterStatus != LW_WADE)
	{
		item->targetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(info->gunStatus == LG_HANDS_FREE || !IsStandingWeapon(info->gunType)) &&
		info->waterStatus != LW_WADE)
	{
		item->targetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (info->waterStatus == LW_WADE)		// Should not be possible, but here for security.
		{
			if (TestLaraRunForward(item, coll))
			{
				item->targetState = LS_WADE_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->targetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->targetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->targetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->targetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) [[likely]]
		{
			item->targetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP && TestLaraStepLeft(item, coll))
	{
		item->targetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->targetState = LS_STEP_RIGHT;
		return;
	}

	// TODO: Hold WALK to slow down again.
	if (TrInput & IN_RIGHT)
	{
		item->targetState = LS_TURN_RIGHT_FAST;
		return;
	}

	item->targetState = LS_IDLE;
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
	LaraInfo*& info = item->data;

	if (item->hitPoints <= 0)
	{
		item->targetState = LS_DEATH;
		return;
	}

	info->turnRate -= LARA_TURN_RATE;
	if (info->turnRate > -LARA_MED_TURN_MAX)
		info->turnRate = -LARA_MED_TURN_MAX;
	else if (info->turnRate < -LARA_FAST_TURN_MAX)
		info->turnRate = -LARA_FAST_TURN_MAX;

	if (TrInput & IN_JUMP)
	{
		SetLaraJumpDirection(item, coll);
		if (info->jumpDirection != JumpDirection::NoDirection)
		{
			item->targetState = LS_JUMP_PREPARE;
			return;
		}
	}

	if (TrInput & IN_ROLL &&
		info->waterStatus != LW_WADE)
	{
		item->targetState = LS_ROLL_FORWARD;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(info->gunStatus == LG_HANDS_FREE || !IsStandingWeapon(info->gunType)) &&
		info->waterStatus != LW_WADE)
	{
		item->targetState = LS_CROUCH_IDLE;
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		if (info->waterStatus == LW_WADE)		// Should not be possible, but here for security.
		{
			if (TestLaraRunForward(item, coll))
			{
				item->targetState = LS_WADE_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_WALK)
		{
			if (TestLaraWalkForward(item, coll))
			{
				item->targetState = LS_WALK_FORWARD;
				return;
			}
		}
		else if (TrInput & IN_SPRINT && TestLaraRunForward(item, coll))
		{
			item->targetState = LS_SPRINT;
			return;
		}
		else if (TestLaraRunForward(item, coll)) [[likely]]
		{
			item->targetState = LS_RUN_FORWARD;
			return;
		}
	}
	else if (TrInput & IN_BACK)
	{
		if (TrInput & IN_WALK)
		{
			if (TestLaraWalkBack(item, coll))
			{
				item->targetState = LS_WALK_BACK;
				return;
			}
		}
		else if (TestLaraRunBack(item, coll)) [[likely]]
		{
			item->targetState = LS_RUN_BACK;
			return;
		}
	}

	if (TrInput & IN_LSTEP && 	TestLaraStepLeft(item, coll))
	{
		item->targetState = LS_STEP_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP && TestLaraStepRight(item, coll))
	{
		item->targetState = LS_STEP_RIGHT;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		item->targetState = LS_TURN_LEFT_FAST;
		return;
	}

	item->targetState = LS_IDLE;
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
	LaraInfo*& info = item->data;

	info->look = false;

	if (item->hitPoints <= 0)
	{
		item->targetState = LS_DEATH;
		return;
	}

	if (info->isMoving)
		return;

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

	if (TrInput & IN_RSTEP)
	{
		item->targetState = LS_STEP_RIGHT;
		return;
	}
	
	item->targetState = LS_IDLE;
}

// State:		LS_STEP_RIGHT (21)
// Control:		lara_as_step_right()
void lara_col_step_right(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot + ANGLE(90.0f);
	item->Airborne = false;
	item->VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = (info->waterStatus == LW_WADE) ? NO_LOWER_BOUND : CLICK(0.8f);
	coll->Setup.UpperFloorBound = -CLICK(0.8f);
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsPit = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.FloorSlopeIsWall = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.DeathFlagIsPit = true;
	coll->Setup.ForwardAngle = info->moveAngle;
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
	LaraInfo*& info = item->data;

	info->look = false;

	if (item->hitPoints <= 0)
	{
		item->targetState = LS_DEATH;
		return;
	}

	if (info->isMoving)
		return;

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

	if (TrInput & IN_LSTEP)
	{
		item->targetState = LS_STEP_LEFT;
		return;
	}

	item->targetState = LS_IDLE;
}

// State:		LS_STEP_LEFT (22)
// Control:		lara_as_step_left()
void lara_col_step_left(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot - ANGLE(90.0f);
	item->Airborne = false;
	item->VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = (info->waterStatus == LW_WADE) ? NO_LOWER_BOUND : CLICK(0.8f);
	coll->Setup.UpperFloorBound = -CLICK(0.8f);
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsPit = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.FloorSlopeIsWall = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.DeathFlagIsPit = true;
	coll->Setup.ForwardAngle = info->moveAngle;
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
	LaraInfo*& info = item->data;

	info->look = false;

	if (TrInput & IN_ROLL)
	{
		item->targetState = LS_ROLL_BACK;
		return;
	}

	item->targetState = LS_IDLE;
}

// State:		LS_ROLL_BACK (23)
// Control:		lara_as_roll_back()
void lara_col_roll_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot + ANGLE(180.0f);
	item->Airborne = false;
	item->VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = info->moveAngle;
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
	LaraInfo*& info = item->data;

	info->look = false;

	// TODO: Change swandive roll anim state to something sensible.
	if (TrInput & IN_FORWARD &&
		item->animNumber == LA_SWANDIVE_ROLL) // Hack.
	{
		item->targetState = LS_RUN_FORWARD;
		return;
	}

	if (TrInput & IN_ROLL)
	{
		item->targetState = LS_ROLL_FORWARD;
		return;
	}

	item->targetState = LS_IDLE;
}

// State:		LS_ROLL_FORWARD (45)
// Control:		lara_as_roll_forward()
void lara_col_roll_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	item->Airborne = false;
	item->VerticalVelocity = 0;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = info->moveAngle;
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
	LaraInfo*& info = item->data;

	info->look = (TestEnvironment(ENV_FLAG_SWAMP, item) && info->waterStatus == LW_WADE) ? false : true;
	Camera.targetElevation = -ANGLE(22.0f);

	if (item->hitPoints <= 0)
	{
		item->targetState = LS_IDLE;
		return;
	}

	if (TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		PseudoLaraAsSwampWadeForward(item, coll);
		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_FAST_TURN_MAX)
			info->turnRate = -LARA_FAST_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE / 2);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_FAST_TURN_MAX)
			info->turnRate = LARA_FAST_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE / 2);
	}

	if (TrInput & IN_FORWARD)
	{
		if (info->waterStatus == LW_ABOVE_WATER)
			item->targetState = LS_RUN_FORWARD;
		else [[likely]]
			item->targetState = LS_WADE_FORWARD;

		return;
	}

	item->targetState = LS_IDLE;
}

// Pseudo-state for wading in swamps.
void PseudoLaraAsSwampWadeForward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SWAMP_TURN_MAX)
			info->turnRate = -LARA_SWAMP_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX / 5 * 3, LARA_LEAN_RATE / 3);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SWAMP_TURN_MAX)
			info->turnRate = LARA_SWAMP_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX / 5 * 3, LARA_LEAN_RATE / 3);
	}

	if (TrInput & IN_FORWARD)
	{
		item->targetState = LS_WADE_FORWARD;
		return;
	}

	item->targetState = LS_IDLE;
}

// State:		LS_WADE_FORWARD (65)
// Control:		lara_as_wade_forward()
void lara_col_wade_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = TestEnvironment(ENV_FLAG_SWAMP, item) ? false : true;
	coll->Setup.ForwardAngle = info->moveAngle;
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
		item->pos.zRot = 0;

		if (coll->Front.Floor < -STEPUP_HEIGHT &&
			!coll->Front.FloorSlope &&
			!TestEnvironment(ENV_FLAG_SWAMP, item)) // TODO: Check this.
		{
			item->targetState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->animNumber]))
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
	LaraInfo*& info = item->data;

	info->sprintTimer--;

	info->runJumpCount++;
	if (info->runJumpCount > LARA_RUN_JUMP_TIME)
		info->runJumpCount = LARA_RUN_JUMP_TIME;

	if (item->hitPoints <= 0)
	{
		item->targetState = LS_DEATH;
		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN_MAX)
			info->turnRate = -LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN_MAX)
			info->turnRate = LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE);
	}

	if (TrInput & IN_JUMP)
	{
		item->targetState = LS_SPRINT_DIVE;
		return;
	}

	if (TrInput & IN_CROUCH &&
		(info->gunStatus == LG_HANDS_FREE || !IsStandingWeapon(info->gunType)))
	{
		item->targetState = LS_CROUCH_IDLE;
		return;
	}

	// TODO: Supposedly there is a bug wherein sprinting into the boundary between shallow and deep water
	// while meeting some condition allows Lara to run around in the water room. Investigate. @Sezz 2021.09.29
	if (TrInput & IN_FORWARD)
	{
		if (info->waterStatus == LW_WADE)
			item->targetState = LS_RUN_FORWARD;	// TODO: Dispatch to wade forward state directly. @Sezz 2021.09.29
		else if (TrInput & IN_WALK)
			item->targetState = LS_WALK_FORWARD;
		else if (TrInput & IN_SPRINT && info->sprintTimer > 0) [[likely]]
			item->targetState = LS_SPRINT;
		else
			item->targetState = LS_RUN_FORWARD;

		return;
	}

	item->targetState = LS_IDLE;
}

// State:		LS_SPRINT (73)
// Control:		lara_as_sprint()
void lara_col_sprint(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = item->pos.yRot;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = info->moveAngle;
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
		item->pos.zRot = 0;

		if (coll->HitTallObject || TestLaraSplat(item, OFFSET_RADIUS(coll->Setup.Radius), -CLICK(2.5f)))
		{
			item->targetState = LS_SPLAT;
			if (GetChange(item, &g_Level.Anims[item->animNumber]))
			{
				item->activeState = LS_SPLAT;
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
	LaraInfo*& info = item->data;

	info->runJumpCount++;
	if (info->runJumpCount > LARA_RUN_JUMP_TIME)
		info->runJumpCount = LARA_RUN_JUMP_TIME;

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_SLOW_TURN_MAX)
			info->turnRate = -LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, -(LARA_LEAN_MAX * 3) / 5, LARA_LEAN_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_SLOW_TURN_MAX)
			info->turnRate = LARA_SLOW_TURN_MAX;

		DoLaraLean(item, coll, (LARA_LEAN_MAX * 3) / 5, LARA_LEAN_RATE);
	}

	// TODO: Make this state a true jump?
	if (item->targetState != LS_DEATH &&
		item->targetState != LS_IDLE &&
		item->targetState != LS_RUN_FORWARD &&
		item->VerticalVelocity >= LARA_FREEFALL_SPEED)
	{
		item->targetState = LS_FREEFALL;
	}
}

// State:		LS_SPRINT_DIVE (74)
// Control:		lara_col_sprint_dive()
void lara_col_sprint_dive(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->moveAngle = (item->Velocity >= 0) ? item->pos.yRot : item->pos.yRot + ANGLE(180.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.FloorSlopeIsWall = true;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item);

	LaraDeflectEdgeJump(item, coll);

	if (TestLaraFall(item, coll))
	{
		SetLaraFallState(item);
		return;
	}

	if (item->Velocity < 0)
		info->moveAngle = item->pos.yRot; // ???

	if (coll->Middle.Floor <= 0 && item->VerticalVelocity > 0)
	{
		DoLaraFallDamage(item);

		if (item->hitPoints <= 0) // TODO: It seems Core wanted to make the sprint dive a true jump.
			item->targetState = LS_DEATH;
		else if (!(TrInput & IN_FORWARD) || TrInput & IN_WALK || info->waterStatus == LW_WADE)
			item->targetState = LS_IDLE;
		else
			item->targetState = LS_RUN_FORWARD;

		item->Airborne = false;
		item->VerticalVelocity = 0;
		item->pos.yPos += coll->Middle.Floor;
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
