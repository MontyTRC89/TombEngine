#include "framework.h"
#include "Game/Lara/lara_surface.h"

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_swim.h"
#include "Game/Lara/lara_tests.h"
#include "Specific/level.h"
#include "Specific/input.h"

void lara_col_surftread(ITEM_INFO* item, CollisionInfo* coll) 
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot;
	LaraSurfaceCollision(item, coll);
}

void lara_col_surfright(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot + ANGLE(90.0f);
	LaraSurfaceCollision(item, coll);
}

void lara_col_surfleft(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot - ANGLE(90.0f);
	LaraSurfaceCollision(item, coll);
}

void lara_col_surfback(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = item->Position.yRot + ANGLE(180.0f);
	LaraSurfaceCollision(item, coll);
}

void lara_col_surfswim(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	lara->Control.MoveAngle = item->Position.yRot;
	LaraSurfaceCollision(item, coll);
	TestLaraWaterClimbOut(item, coll);
	TestLaraLadderClimbOut(item, coll);
}

void lara_as_surftread(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	item->Animation.VerticalVelocity -= 4;
	if (item->Animation.VerticalVelocity < 0)
		item->Animation.VerticalVelocity = 0;

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}

	if (TrInput & IN_LOOK)
	{
		LookUpDown(item);
		return;
	}

	if (TrInput & IN_LEFT)
		item->Position.yRot -= ANGLE(4.0f);
	else if (TrInput & IN_RIGHT)
		item->Position.yRot += ANGLE(4.0f);

	if (TrInput & IN_JUMP)
	{
		lara->Control.Count.Dive++;
		if (lara->Control.Count.Dive == 10)
			SwimDive(item);

		return;
	}
	else if (TrInput & IN_FORWARD)
	{
		item->Animation.TargetState = LS_ONWATER_FORWARD;
		return;
	}
	else if (TrInput & IN_BACK)
	{
		item->Animation.TargetState = LS_ONWATER_BACK;
		return;
	}
	else if (TrInput & IN_ROLL)
	{
		item->Animation.TargetState = LS_ROLL_FORWARD;
		return;
	}
	else if (TrInput & IN_LSTEP)
	{
		item->Animation.TargetState = LS_ONWATER_LEFT;
		return;
	}
	else if (TrInput & IN_RSTEP)
	{
		item->Animation.TargetState = LS_ONWATER_RIGHT;
		return;
	}

	item->Animation.TargetState = LS_ONWATER_STOP;
	lara->Control.Count.Dive = 0;
}

void lara_as_surfright(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}

	lara->Control.Count.Dive = 0;

	if (TrInput & IN_LEFT)
		item->Position.yRot -= ANGLE(2.0f);
	else if (TrInput & IN_RIGHT)
		item->Position.yRot += ANGLE(2.0f);

	if (!(TrInput & IN_RSTEP))
		item->Animation.TargetState = LS_ONWATER_STOP;

	item->Animation.VerticalVelocity += 8;
	if (item->Animation.VerticalVelocity > 60)
		item->Animation.VerticalVelocity = 60;
}

void lara_as_surfleft(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}

	lara->Control.Count.Dive = 0;

	if (TrInput & IN_LEFT)
		item->Position.yRot -= ANGLE(2.0f);
	else if (TrInput & IN_RIGHT)
		item->Position.yRot += ANGLE(2.0f);

	if (!(TrInput & IN_LSTEP))
		item->Animation.TargetState = LS_ONWATER_STOP;

	item->Animation.VerticalVelocity += 8;
	if (item->Animation.VerticalVelocity > 60)
		item->Animation.VerticalVelocity = 60;
}

void lara_as_surfback(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}

	lara->Control.Count.Dive = 0;

	if (TrInput & IN_LEFT)
		item->Position.yRot -= ANGLE(2.0f);
	else if (TrInput & IN_RIGHT)
		item->Position.yRot += ANGLE(2.0f);

	if (!(TrInput & IN_BACK))
		item->Animation.TargetState = LS_ONWATER_STOP;

	item->Animation.VerticalVelocity += 8;
	if (item->Animation.VerticalVelocity > 60)
		item->Animation.VerticalVelocity = 60;
}

void lara_as_surfswim(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_WATER_DEATH;
		return;
	}

	lara->Control.Count.Dive = 0;

	if (TrInput & IN_LEFT)
		item->Position.yRot -= ANGLE(4.0f);
	else if (TrInput & IN_RIGHT)
		item->Position.yRot += ANGLE(4.0f);

	if (!(TrInput & IN_FORWARD))
		item->Animation.TargetState = LS_ONWATER_STOP;
	if (TrInput & IN_JUMP)
		item->Animation.TargetState = LS_ONWATER_STOP;

	item->Animation.VerticalVelocity += 8;
	if (item->Animation.VerticalVelocity > 60)
		item->Animation.VerticalVelocity = 60;
}

void lara_as_waterout(ITEM_INFO* item, CollisionInfo* coll)
{
	coll->Setup.EnableObjectPush = false;
	coll->Setup.EnableSpasm = false;
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.laraNode = LM_HIPS;	//forces the camera to follow Lara instead of snapping
}
