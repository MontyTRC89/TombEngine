#include "framework.h"
#include "Game/Lara/lara_swim.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara.h"
#include "Scripting/GameFlowScript.h"
#include "Specific/level.h"
#include "Specific/input.h"

// -----------------------------
// UNDERWATER SWIM
// Control & Collision Functions
// -----------------------------

// State:		LS_UNDERWATER_IDLE (13)
// Collision:	lara_col_underwater_idle()
void lara_as_underwater_idle(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	LaraType laraType = g_GameFlow->GetLevel(CurrentLevel)->LaraType;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}

	if ((TrInput & IN_ROLL || (TrInput & IN_FORWARD && TrInput & IN_BACK)) && laraType != LaraType::Divesuit)
	{
		SetAnimation(item, LA_UNDERWATER_ROLL_180_START);
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown(item);

	if (laraType == LaraType::Divesuit)
		ModulateLaraSubsuitSwimTurn(item);
	else
		ModulateLaraSwimTurn(item, coll);

	if (TrInput & IN_JUMP)
		item->Animation.TargetState = LS_UNDERWATER_SWIM_FORWARD;

	item->Animation.VerticalVelocity -= 6;
	if (item->Animation.VerticalVelocity < 0)
		item->Animation.VerticalVelocity = 0;

	if (lara->Control.HandStatus == HandStatus::Busy)
		lara->Control.HandStatus = HandStatus::Free;
}

// State:		LS_UNDERWATER_IDLE (13)
// Control:		lara_as_underwater_idle()
void lara_col_underwater_idle(ITEM_INFO* item, CollisionInfo* coll)
{
	LaraSwimCollision(item, coll);
}

// State:		LS_UNDERWATER_SWIM_FORWARD (17)
// Collision:	lara_col_underwater_swim_forward()
void lara_as_underwater_swim_forward(ITEM_INFO* item, CollisionInfo* coll)
{
	LaraType laraType = g_GameFlow->GetLevel(CurrentLevel)->LaraType;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}

	if (TrInput & IN_ROLL && laraType != LaraType::Divesuit)
	{
		SetAnimation(item, LA_UNDERWATER_ROLL_180_START);
		return;
	}

	if (laraType != LaraType::Divesuit)
		ModulateLaraSwimTurn(item, coll);
	else
		ModulateLaraSubsuitSwimTurn(item);

	item->Animation.VerticalVelocity += LARA_SWIM_ACCELERATION;
	if (item->Animation.VerticalVelocity > LARA_SWIM_VELOCITY_MAX)
		item->Animation.VerticalVelocity = LARA_SWIM_VELOCITY_MAX;

	if (!(TrInput & IN_JUMP))
		item->Animation.TargetState = LS_UNDERWATER_INERTIA;
}

// State:		LS_UNDERWATER_SWIM_FORWARD (17)
// Control:		lara_as_underwater_swim_forward()
void lara_col_underwater_swim_forward(ITEM_INFO* item, CollisionInfo* coll)
{
	LaraSwimCollision(item, coll);
}

// State:		LS_UNDERWATER_INERTIA (18)
// Collision:	lara_col_underwater_inertia()
void lara_as_underwater_inertia(ITEM_INFO* item, CollisionInfo* coll)
{
	LaraType laraType = g_GameFlow->GetLevel(CurrentLevel)->LaraType;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}

	if (TrInput & IN_ROLL && laraType != LaraType::Divesuit)
	{
		SetAnimation(item, LA_UNDERWATER_ROLL_180_START);
		return;
	}

	if (laraType != LaraType::Divesuit)
		ModulateLaraSwimTurn(item, coll);
	else
		ModulateLaraSubsuitSwimTurn(item);

	if (TrInput & IN_JUMP)
		item->Animation.TargetState = LS_UNDERWATER_SWIM_FORWARD;

	item->Animation.VerticalVelocity -= LARA_SWIM_DECELERATION;
	if (item->Animation.VerticalVelocity < 0)
		item->Animation.VerticalVelocity = 0;

	if (item->Animation.VerticalVelocity < LARA_SWIM_INTERTIA_VELOCITY_MIN)
		item->Animation.TargetState = LS_UNDERWATER_IDLE;
}

// State:		LS_UNDERWATER_INERTIA (18)
// Collision:	lara_as_underwater_inertia()
void lara_col_underwater_inertia(ITEM_INFO* item, CollisionInfo* coll)
{
	LaraSwimCollision(item, coll);
}

// State:		LS_WATER_DEATH (44)
// Collision:	lara_col_underwater_death()
void lara_as_underwater_death(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;

	item->Animation.VerticalVelocity -= LARA_SWIM_DECELERATION;
	if (item->Animation.VerticalVelocity < 0)
		item->Animation.VerticalVelocity = 0;

	if (item->Position.xRot < -ANGLE(2.0f) ||
		item->Position.xRot > ANGLE(2.0f))
	{
		if (item->Position.xRot >= 0)
			item->Position.xRot -= ANGLE(2.0f);
		else
			item->Position.xRot += ANGLE(2.0f);
	}
	else
		item->Position.xRot = 0;
}

// State:		LS_WATER_DEATH (44)
// Control:	lara_as_underwater_death()
void lara_col_underwater_death(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	item->HitPoints = -1;
	lara->Air = -1;
	lara->Control.HandStatus = HandStatus::Busy;

	int waterHeight = GetWaterHeight(item);
	if (waterHeight < (item->Position.yPos - (CLICK(0.4f) - 2)) &&
		waterHeight != NO_HEIGHT)
	{
		item->Position.yPos -= 5;
	}

	LaraSwimCollision(item, coll);
}

// State:		LS_UNDERWATER_ROLL (66)
// Collision:	lara_col_underwater_roll_180()
void lara_as_underwater_roll_180(ITEM_INFO* item, CollisionInfo* coll)
{
	item->Animation.VerticalVelocity = 0;
}

// State:		LS_UNDERWATER_ROLL (66)
// Control:		lara_as_underwater_roll_180()
void lara_col_underwater_roll_180(ITEM_INFO* item, CollisionInfo* coll)
{
	LaraSwimCollision(item, coll);
}
