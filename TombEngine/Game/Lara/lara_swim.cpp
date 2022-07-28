#include "framework.h"
#include "Game/Lara/lara_swim.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara.h"
#include "Flow/ScriptInterfaceFlowHandler.h"
#include "ScriptInterfaceLevel.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/input.h"

using namespace TEN::Input;

// -----------------------------
// UNDERWATER SWIM
// Control & Collision Functions
// -----------------------------

// State:		LS_UNDERWATER_IDLE (13)
// Collision:	lara_col_underwater_idle()
void lara_as_underwater_idle(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	LaraType laraType = g_GameFlow->GetLevel(CurrentLevel)->GetLaraType();

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}

	auto level = g_GameFlow->GetLevel(CurrentLevel);

	if ((TrInput & IN_ROLL || (TrInput & IN_FORWARD && TrInput & IN_BACK)) && laraType != LaraType::Divesuit)
	{
		SetAnimation(item, LA_UNDERWATER_ROLL_180_START);
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown(item);

	if (laraType == LaraType::Divesuit)
		ModulateLaraSubsuitSwimTurnRates(item);
	else
		ModulateLaraSwimTurnRates(item, coll);

	if (TrInput & IN_JUMP)
		item->Animation.TargetState = LS_UNDERWATER_SWIM_FORWARD;

	item->Animation.VerticalVelocity -= LARA_SWIM_VELOCITY_DECEL;
	if (item->Animation.VerticalVelocity < 0)
		item->Animation.VerticalVelocity = 0;

	if (lara->Control.HandStatus == HandStatus::Busy)
		lara->Control.HandStatus = HandStatus::Free;
}

// State:		LS_UNDERWATER_IDLE (13)
// Control:		lara_as_underwater_idle()
void lara_col_underwater_idle(ItemInfo* item, CollisionInfo* coll)
{
	LaraSwimCollision(item, coll);
}

// State:		LS_UNDERWATER_SWIM_FORWARD (17)
// Collision:	lara_col_underwater_swim_forward()
void lara_as_underwater_swim_forward(ItemInfo* item, CollisionInfo* coll)
{
	LaraType laraType = g_GameFlow->GetLevel(CurrentLevel)->GetLaraType();

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
		ModulateLaraSwimTurnRates(item, coll);
	else
		ModulateLaraSubsuitSwimTurnRates(item);

	item->Animation.VerticalVelocity += LARA_SWIM_VELOCITY_ACCEL;
	if (item->Animation.VerticalVelocity > LARA_SWIM_VELOCITY_MAX)
		item->Animation.VerticalVelocity = LARA_SWIM_VELOCITY_MAX;

	if (!(TrInput & IN_JUMP))
		item->Animation.TargetState = LS_UNDERWATER_INERTIA;
}

// State:		LS_UNDERWATER_SWIM_FORWARD (17)
// Control:		lara_as_underwater_swim_forward()
void lara_col_underwater_swim_forward(ItemInfo* item, CollisionInfo* coll)
{
	LaraSwimCollision(item, coll);
}

// State:		LS_UNDERWATER_INERTIA (18)
// Collision:	lara_col_underwater_inertia()
void lara_as_underwater_inertia(ItemInfo* item, CollisionInfo* coll)
{
	LaraType laraType = g_GameFlow->GetLevel(CurrentLevel)->GetLaraType();

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
		ModulateLaraSwimTurnRates(item, coll);
	else
		ModulateLaraSubsuitSwimTurnRates(item);

	if (TrInput & IN_JUMP)
		item->Animation.TargetState = LS_UNDERWATER_SWIM_FORWARD;

	item->Animation.VerticalVelocity -= LARA_SWIM_VELOCITY_DECEL;
	if (item->Animation.VerticalVelocity < 0)
		item->Animation.VerticalVelocity = 0;

	if (item->Animation.VerticalVelocity < LARA_SWIM_INTERTIA_VELOCITY_MIN)
		item->Animation.TargetState = LS_UNDERWATER_IDLE;
}

// State:		LS_UNDERWATER_INERTIA (18)
// Collision:	lara_as_underwater_inertia()
void lara_col_underwater_inertia(ItemInfo* item, CollisionInfo* coll)
{
	LaraSwimCollision(item, coll);
}

// State:		LS_WATER_DEATH (44)
// Collision:	lara_col_underwater_death()
void lara_as_underwater_death(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;

	item->Animation.VerticalVelocity -= LARA_SWIM_VELOCITY_DECEL;
	if (item->Animation.VerticalVelocity < 0)
		item->Animation.VerticalVelocity = 0;

	if (item->Pose.Orientation.x < -ANGLE(2.0f) ||
		item->Pose.Orientation.x > ANGLE(2.0f))
	{
		if (item->Pose.Orientation.x >= 0)
			item->Pose.Orientation.x -= ANGLE(2.0f);
		else
			item->Pose.Orientation.x += ANGLE(2.0f);
	}
	else
		item->Pose.Orientation.x = 0;
}

// State:		LS_WATER_DEATH (44)
// Control:	lara_as_underwater_death()
void lara_col_underwater_death(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	item->HitPoints = -1;
	lara->Air = -1;
	lara->Control.HandStatus = HandStatus::Busy;

	int waterHeight = GetWaterHeight(item);
	if (waterHeight < (item->Pose.Position.y - (CLICK(0.4f) - 2)) &&
		waterHeight != NO_HEIGHT)
	{
		item->Pose.Position.y -= 5;
	}

	LaraSwimCollision(item, coll);
}

// State:		LS_UNDERWATER_ROLL (66)
// Collision:	lara_col_underwater_roll_180()
void lara_as_underwater_roll_180(ItemInfo* item, CollisionInfo* coll)
{
	item->Animation.VerticalVelocity = 0;
}

// State:		LS_UNDERWATER_ROLL (66)
// Control:		lara_as_underwater_roll_180()
void lara_col_underwater_roll_180(ItemInfo* item, CollisionInfo* coll)
{
	LaraSwimCollision(item, coll);
}
