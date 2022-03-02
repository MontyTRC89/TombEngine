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
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/input.h"

void lara_col_waterroll(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraSwimCollision(item, coll);
}

void lara_col_uwdeath(ITEM_INFO* item, COLL_INFO* coll)
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

void lara_col_dive(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraSwimCollision(item, coll);
}

void lara_col_tread(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraSwimCollision(item, coll);
}

void lara_col_glide(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraSwimCollision(item, coll);
}

void lara_col_swim(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraSwimCollision(item, coll);
}

void lara_as_waterroll(ITEM_INFO* item, COLL_INFO* coll)
{
	item->VerticalVelocity = 0;
}

void lara_as_uwdeath(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.CanLook = false;

	item->VerticalVelocity -= 8;
	if (item->VerticalVelocity <= 0)
		item->VerticalVelocity = 0;

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

void lara_as_dive(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_FORWARD)
		item->Position.xRot -= ANGLE(1.0f);
}

void lara_as_tread(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	LaraType laraType = g_GameFlow->GetLevel(CurrentLevel)->LaraType;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_WATER_DEATH;
		return;
	}

	if (TrInput & IN_ROLL && laraType != LaraType::Divesuit)
	{
		SetAnimation(item, LA_UNDERWATER_ROLL_180_START);
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown(item);

	if (laraType == LaraType::Divesuit)
		SwimTurnSubsuit(item);
	else
		SwimTurn(item, coll);

	if (TrInput & IN_JUMP)
		item->TargetState = LS_UNDERWATER_FORWARD;

	item->VerticalVelocity -= 6;
	if (item->VerticalVelocity < 0)
		item->VerticalVelocity = 0;

	if (lara->Control.HandStatus == HandStatus::Busy)
		lara->Control.HandStatus = HandStatus::Free;
}

void lara_as_glide(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraType laraType = g_GameFlow->GetLevel(CurrentLevel)->LaraType;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_WATER_DEATH;
		return;
	}

	if (TrInput & IN_ROLL && laraType != LaraType::Divesuit)
	{
		SetAnimation(item, LA_UNDERWATER_ROLL_180_START);
		return;
	}
	
	if (laraType != LaraType::Divesuit)
		SwimTurn(item, coll);
	else
		SwimTurnSubsuit(item);

	if (TrInput & IN_JUMP)
		item->TargetState = LS_UNDERWATER_FORWARD;

	item->VerticalVelocity -= 6;
	if (item->VerticalVelocity < 0)
		item->VerticalVelocity = 0;

	if (item->VerticalVelocity <= 133)
		item->TargetState = LS_UNDERWATER_STOP;
}

void lara_as_swim(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraType laraType = g_GameFlow->GetLevel(CurrentLevel)->LaraType;

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_WATER_DEATH;
		return;
	}

	if (TrInput & IN_ROLL && laraType != LaraType::Divesuit)
	{
		SetAnimation(item, LA_UNDERWATER_ROLL_180_START);
		return;
	}
	
	if (laraType != LaraType::Divesuit)
		SwimTurn(item, coll);
	else
		SwimTurnSubsuit(item);

	item->VerticalVelocity += 8;
	if (item->VerticalVelocity > 200)
		item->VerticalVelocity = 200;

	if (!(TrInput & IN_JUMP))
		item->TargetState = LS_UNDERWATER_INERTIA;
}

void UpdateSubsuitAngles(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.SubsuitControl.VerticalVelocity != 0)
	{
		item->Position.yPos += lara->Control.SubsuitControl.VerticalVelocity / 4;
		lara->Control.SubsuitControl.VerticalVelocity = ceil((15 / 16) * lara->Control.SubsuitControl.VerticalVelocity - 1);
	}

	lara->Control.SubsuitControl.Velocity[0] = -4 * item->VerticalVelocity;
	lara->Control.SubsuitControl.Velocity[1] = -4 * item->VerticalVelocity;

	if (lara->Control.SubsuitControl.XRot >= lara->Control.SubsuitControl.DXRot)
	{
		if (lara->Control.SubsuitControl.XRot > lara->Control.SubsuitControl.DXRot)
		{
			if (lara->Control.SubsuitControl.XRot > 0 && lara->Control.SubsuitControl.DXRot < 0)
				lara->Control.SubsuitControl.XRot = ceil(0.75 * lara->Control.SubsuitControl.XRot);

			lara->Control.SubsuitControl.XRot -= ANGLE(2.0f);
			if (lara->Control.SubsuitControl.XRot < lara->Control.SubsuitControl.DXRot)
				lara->Control.SubsuitControl.XRot = lara->Control.SubsuitControl.DXRot;
		}
	}
	else
	{
		if (lara->Control.SubsuitControl.XRot < 0 && lara->Control.SubsuitControl.DXRot > 0)
			lara->Control.SubsuitControl.XRot = ceil(0.75 * lara->Control.SubsuitControl.XRot);

		lara->Control.SubsuitControl.XRot += ANGLE(2.0f);
		if (lara->Control.SubsuitControl.XRot > lara->Control.SubsuitControl.DXRot)
			lara->Control.SubsuitControl.XRot = lara->Control.SubsuitControl.DXRot;
	}

	if (lara->Control.SubsuitControl.DXRot != 0)
	{
		short rotation = lara->Control.SubsuitControl.DXRot >> 3;
		if (rotation < -ANGLE(2.0f))
			rotation = -ANGLE(2.0f);
		else if (rotation > ANGLE(2.0f))
			rotation = ANGLE(2.0f);

		item->Position.xRot += rotation;
	}

	lara->Control.SubsuitControl.Velocity[0] += abs(lara->Control.SubsuitControl.XRot >> 3);
	lara->Control.SubsuitControl.Velocity[1] += abs(lara->Control.SubsuitControl.XRot >> 3);

	if (lara->Control.TurnRate > 0)
		lara->Control.SubsuitControl.Velocity[0] += 2 * abs(lara->Control.TurnRate);
	else if (lara->Control.TurnRate < 0)
		lara->Control.SubsuitControl.Velocity[1] += 2 * abs(lara->Control.TurnRate);

	if (lara->Control.SubsuitControl.Velocity[0] > SECTOR(1.5f))
		lara->Control.SubsuitControl.Velocity[0] = SECTOR(1.5f);

	if (lara->Control.SubsuitControl.Velocity[1] > SECTOR(1.5f))
		lara->Control.SubsuitControl.Velocity[1] = SECTOR(1.5f);

	if (lara->Control.SubsuitControl.Velocity[0] != 0 || lara->Control.SubsuitControl.Velocity[1] != 0)
		SoundEffect(SFX_TR5_LARA_UNDERWATER_ENGINE, &item->Position, (((lara->Control.SubsuitControl.Velocity[0] + lara->Control.SubsuitControl.Velocity[1]) * 4) & 0x1F00) + 10);
}

void SwimTurnSubsuit(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	if (item->Position.yPos < 14080)
		lara->Control.SubsuitControl.VerticalVelocity += (14080 - item->Position.yPos) >> 4;

	if (TrInput & IN_FORWARD && item->Position.xRot > -ANGLE(85.0f))
		lara->Control.SubsuitControl.DXRot = -ANGLE(45.0f);
	else if (TrInput & IN_BACK && item->Position.xRot < ANGLE(85.0f))
		lara->Control.SubsuitControl.DXRot = ANGLE(45.0f);
	else
		lara->Control.SubsuitControl.DXRot = 0;

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_SUBSUIT_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_MED_TURN_MAX)
			lara->Control.TurnRate = -LARA_MED_TURN_MAX;

		item->Position.zRot -= LARA_LEAN_RATE;
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_SUBSUIT_TURN_RATE;
		if (lara->Control.TurnRate > LARA_MED_TURN_MAX)
			lara->Control.TurnRate = LARA_MED_TURN_MAX;

		item->Position.zRot += LARA_LEAN_RATE;
	}
}

void SwimTurn(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	if (TrInput & IN_FORWARD)
		item->Position.xRot -= ANGLE(2.0f);
	else if (TrInput & IN_BACK)
		item->Position.xRot += ANGLE(2.0f);

	if (TrInput & IN_LEFT)
	{
		lara->Control.TurnRate -= LARA_TURN_RATE;
		if (lara->Control.TurnRate < -LARA_MED_TURN_MAX)
			lara->Control.TurnRate = -LARA_MED_TURN_MAX;

		item->Position.zRot -= LARA_LEAN_RATE;
	}
	else if (TrInput & IN_RIGHT)
	{
		lara->Control.TurnRate += LARA_TURN_RATE;
		if (lara->Control.TurnRate > LARA_MED_TURN_MAX)
			lara->Control.TurnRate = LARA_MED_TURN_MAX;

		item->Position.zRot += LARA_LEAN_RATE;
	}
}

void SwimDive(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	SetAnimation(item, LA_ONWATER_DIVE);
	item->TargetState = LS_UNDERWATER_FORWARD;
	item->Position.xRot = ANGLE(-45.0f);
	item->VerticalVelocity = 80;
	lara->Control.WaterStatus = WaterStatus::Underwater;
}

void LaraWaterCurrent(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	if (lara->Control.WaterCurrentActive)
	{
		auto* sink = &g_Level.Sinks[lara->Control.WaterCurrentActive - 1];

		short angle = mGetAngle(sink->x, sink->z, item->Position.xPos, item->Position.zPos);
		lara->ExtraVelocity.x += (sink->strength * SECTOR(1) * phd_sin(angle - ANGLE(90.0f)) - lara->ExtraVelocity.x) / 16;
		lara->ExtraVelocity.z += (sink->strength * SECTOR(1) * phd_cos(angle - ANGLE(90.0f)) - lara->ExtraVelocity.z) / 16;

		item->Position.yPos += (sink->y - item->Position.yPos) >> 4;
	}
	else
	{
		int shift = 0;

		if (abs(lara->ExtraVelocity.x) <= 16)
			shift = (abs(lara->ExtraVelocity.x) > 8) + 2;
		else
			shift = 4;
		lara->ExtraVelocity.x -= lara->ExtraVelocity.x >> shift;

		if (abs(lara->ExtraVelocity.x) < 4)
			lara->ExtraVelocity.x = 0;

		if (abs(lara->ExtraVelocity.z) <= 16)
			shift = (abs(lara->ExtraVelocity.z) > 8) + 2;
		else
			shift = 4;
		lara->ExtraVelocity.z -= lara->ExtraVelocity.z >> shift;

		if (abs(lara->ExtraVelocity.z) < 4)
			lara->ExtraVelocity.z = 0;

		if (!lara->ExtraVelocity.x && !lara->ExtraVelocity.z)
			return;
	}

	item->Position.xPos += lara->ExtraVelocity.x >> 8;
	item->Position.zPos += lara->ExtraVelocity.z >> 8;
	lara->Control.WaterCurrentActive = 0;

	coll->Setup.ForwardAngle = phd_atan(item->Position.zPos - coll->Setup.OldPosition.z, item->Position.xPos - coll->Setup.OldPosition.x);
	coll->Setup.Height = LARA_HEIGHT_CRAWL;
	GetCollisionInfo(coll, item, PHD_VECTOR(0, 200, 0));

	if (coll->CollisionType == CT_FRONT)
	{
		if (item->Position.xRot > ANGLE(35.0f))
			item->Position.xRot += ANGLE(1.0f);
		else if (item->Position.xRot < -ANGLE(35.0f))
			item->Position.xRot -= ANGLE(1.0f);
		else
			item->VerticalVelocity = 0;
	}
	else if (coll->CollisionType == CT_TOP)
		item->Position.xRot -= ANGLE(1.0f);
	else if (coll->CollisionType == CT_TOP_FRONT)
		item->VerticalVelocity = 0;
	else if (coll->CollisionType == CT_LEFT)
		item->Position.yRot += ANGLE(5.0f);
	else if (coll->CollisionType == CT_RIGHT)
		item->Position.yRot -= ANGLE(5.0f);

	if (coll->Middle.Floor < 0 && coll->Middle.Floor != NO_HEIGHT)
		item->Position.yPos += coll->Middle.Floor;

	ShiftItem(item, coll);

	coll->Setup.OldPosition.x = item->Position.xPos;
	coll->Setup.OldPosition.y = item->Position.yPos;
	coll->Setup.OldPosition.z = item->Position.zPos;
}
