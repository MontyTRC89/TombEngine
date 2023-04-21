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
#include "Specific/Input/Input.h"

using namespace TEN::Entities::Player;
using namespace TEN::Input;

// -----------------------------
// UNDERWATER SWIM
// Control & Collision Functions
// -----------------------------

// State:	  LS_UNDERWATER_IDLE (13)
// Collision: lara_col_underwater_idle()
void lara_as_underwater_idle(ItemInfo* item, CollisionInfo* coll)
{
	auto& player = GetLaraInfo(*item);

	auto playerType = g_GameFlow->GetLevel(CurrentLevel)->GetLaraType();
	bool isInDivesuit = (playerType == LaraType::Divesuit);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}

	if ((IsHeld(In::Roll) || (IsHeld(In::Forward) && IsHeld(In::Back))) &&
		!isInDivesuit)
	{
		SetAnimation(item, LA_UNDERWATER_ROLL_180_START);
		return;
	}

	if (IsHeld(In::Look))
		LookUpDown(item);

	isInDivesuit ? ModulateLaraSubsuitSwimTurnRates(item) : ModulateLaraSwimTurnRates(item, coll);

	if (IsHeld(In::Jump))
		item->Animation.TargetState = LS_UNDERWATER_SWIM_FORWARD;

	item->Animation.Velocity.y -= LARA_SWIM_VELOCITY_DECEL;
	if (item->Animation.Velocity.y < 0.0f)
		item->Animation.Velocity.y = 0.0f;

	if (player.Control.HandStatus == HandStatus::Busy)
		player.Control.HandStatus = HandStatus::Free;
}

// State:	LS_UNDERWATER_IDLE (13)
// Control: lara_as_underwater_idle()
void lara_col_underwater_idle(ItemInfo* item, CollisionInfo* coll)
{
	LaraSwimCollision(item, coll);
}

// State:	  LS_UNDERWATER_SWIM_FORWARD (17)
// Collision: lara_col_underwater_swim_forward()
void lara_as_underwater_swim_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto playerType = g_GameFlow->GetLevel(CurrentLevel)->GetLaraType();
	bool isInDivesuit = (playerType == LaraType::Divesuit);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}

	if (IsHeld(In::Roll) && !isInDivesuit)
	{
		SetAnimation(item, LA_UNDERWATER_ROLL_180_START);
		return;
	}

	isInDivesuit ? ModulateLaraSubsuitSwimTurnRates(item) : ModulateLaraSwimTurnRates(item, coll);

	item->Animation.Velocity.y += LARA_SWIM_VELOCITY_ACCEL;
	if (item->Animation.Velocity.y > LARA_SWIM_VELOCITY_MAX)
		item->Animation.Velocity.y = LARA_SWIM_VELOCITY_MAX;

	if (!IsHeld(In::Jump))
		item->Animation.TargetState = LS_UNDERWATER_INERTIA;
}

// State:	LS_UNDERWATER_SWIM_FORWARD (17)
// Control: lara_as_underwater_swim_forward()
void lara_col_underwater_swim_forward(ItemInfo* item, CollisionInfo* coll)
{
	LaraSwimCollision(item, coll);
}

// State:	  LS_UNDERWATER_INERTIA (18)
// Collision: lara_col_underwater_inertia()
void lara_as_underwater_inertia(ItemInfo* item, CollisionInfo* coll)
{
	auto playerType = g_GameFlow->GetLevel(CurrentLevel)->GetLaraType();
	bool isInDivesuit = (playerType == LaraType::Divesuit);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}

	if (IsHeld(In::Roll) && !isInDivesuit)
	{
		SetAnimation(item, LA_UNDERWATER_ROLL_180_START);
		return;
	}

	isInDivesuit ? ModulateLaraSubsuitSwimTurnRates(item) : ModulateLaraSwimTurnRates(item, coll);

	if (IsHeld(In::Jump))
		item->Animation.TargetState = LS_UNDERWATER_SWIM_FORWARD;

	item->Animation.Velocity.y -= LARA_SWIM_VELOCITY_DECEL;
	if (item->Animation.Velocity.y < 0.0f)
		item->Animation.Velocity.y = 0.0f;

	if (item->Animation.Velocity.y < LARA_SWIM_INTERTIA_VELOCITY_MIN)
		item->Animation.TargetState = LS_UNDERWATER_IDLE;
}

// State:	  LS_UNDERWATER_INERTIA (18)
// Collision: lara_as_underwater_inertia()
void lara_col_underwater_inertia(ItemInfo* item, CollisionInfo* coll)
{
	LaraSwimCollision(item, coll);
}

// State:	  LS_WATER_DEATH (44)
// Collision: lara_col_underwater_death()
void lara_as_underwater_death(ItemInfo* item, CollisionInfo* coll)
{
	auto& player = GetLaraInfo(*item);

	player.Control.CanLook = false;

	item->Animation.Velocity.y -= LARA_SWIM_VELOCITY_DECEL;
	if (item->Animation.Velocity.y < 0.0f)
		item->Animation.Velocity.y = 0.0f;

	if (item->Pose.Orientation.x < -ANGLE(2.0f) ||
		item->Pose.Orientation.x > ANGLE(2.0f))
	{
		if (item->Pose.Orientation.x >= 0)
		{
			item->Pose.Orientation.x -= ANGLE(2.0f);
		}
		else
		{
			item->Pose.Orientation.x += ANGLE(2.0f);
		}
	}
	else
	{
		item->Pose.Orientation.x = 0;
	}
}

// State:	LS_WATER_DEATH (44)
// Control:	lara_as_underwater_death()
void lara_col_underwater_death(ItemInfo* item, CollisionInfo* coll)
{
	constexpr auto UPPER_WATER_BOUND = CLICK(0.4f);
	constexpr auto VERTICAL_VEL		 = -5.0f;

	auto& player = GetLaraInfo(*item);

	item->HitPoints = -1;
	player.Control.HandStatus = HandStatus::Busy;

	int waterHeight = GetWaterHeight(item);
	if (waterHeight < (item->Pose.Position.y - UPPER_WATER_BOUND) &&
		waterHeight != NO_HEIGHT)
	{
		item->Pose.Position.y += VERTICAL_VEL;
	}

	LaraSwimCollision(item, coll);
}

// State:	  LS_UNDERWATER_ROLL (66)
// Collision: lara_col_underwater_roll_180()
void lara_as_underwater_roll_180(ItemInfo* item, CollisionInfo* coll)
{
	item->Animation.Velocity.y = 0.0f;
}

// State:	LS_UNDERWATER_ROLL (66)
// Control: lara_as_underwater_roll_180()
void lara_col_underwater_roll_180(ItemInfo* item, CollisionInfo* coll)
{
	LaraSwimCollision(item, coll);
}
