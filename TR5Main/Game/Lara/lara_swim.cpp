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

struct SUBSUIT_INFO
{
	short XRot;
	short dXRot;
	short XRotVel;
	short Vel[2];
	short YVel;
};
SUBSUIT_INFO Subsuit;
byte SubHitCount = 0;

void lara_col_waterroll(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraSwimCollision(item, coll);
}

void lara_col_uwdeath(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	item->HitPoints = -1;
	info->Air = -1;
	info->Control.HandStatus = HandStatus::Busy;

	auto waterHeight = GetWaterHeight(item->Position.xPos, item->Position.yPos, item->Position.zPos, item->RoomNumber);
	if (waterHeight < (item->Position.yPos - (STEP_SIZE / 5 * 2) - 2) &&
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
	auto* info = GetLaraInfo(item);

	info->Control.CanLook = false;

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
	auto* info = GetLaraInfo(item);

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_WATER_DEATH;
		return;
	}

	auto level = g_GameFlow->GetLevel(CurrentLevel);

	if (TrInput & IN_ROLL && level->LaraType != LaraType::Divesuit)
	{
		SetAnimation(item, LA_UNDERWATER_ROLL_180_START);

		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	if (level->LaraType == LaraType::Divesuit)
		SwimTurnSubsuit(item);
	else
		SwimTurn(item, coll);

	if (TrInput & IN_JUMP)
		item->TargetState = LS_UNDERWATER_FORWARD;

	item->VerticalVelocity -= 6;

	if (item->VerticalVelocity < 0)
		item->VerticalVelocity = 0;

	if (info->Control.HandStatus == HandStatus::Busy)
		info->Control.HandStatus = HandStatus::Free;
}

void lara_as_glide(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_WATER_DEATH;
		return;
	}

	auto level = g_GameFlow->GetLevel(CurrentLevel);

	if (TrInput & IN_ROLL && level->LaraType != LaraType::Divesuit)
	{
		SetAnimation(item, LA_UNDERWATER_ROLL_180_START);

		return;
	}
	
	if (level->LaraType != LaraType::Divesuit)
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
	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_WATER_DEATH;
		return;
	}

	auto level = g_GameFlow->GetLevel(CurrentLevel);

	if (TrInput & IN_ROLL && level->LaraType != LaraType::Divesuit)
	{
		SetAnimation(item, LA_UNDERWATER_ROLL_180_START);

		return;
	}
	
	if (level->LaraType != LaraType::Divesuit)
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
	auto* info = GetLaraInfo(item);

	if (Subsuit.YVel != 0)
	{
		item->Position.yPos += Subsuit.YVel / 4;
		Subsuit.YVel = ceil(0.9375 * Subsuit.YVel - 1); // YVel * (15/16)
	}

	Subsuit.Vel[0] = Subsuit.Vel[1] = -4 * item->VerticalVelocity;

	if (Subsuit.XRot >= Subsuit.dXRot)
	{
		if (Subsuit.XRot > Subsuit.dXRot)
		{
			if (Subsuit.XRot > 0 && Subsuit.dXRot < 0)
				Subsuit.XRot = ceil(0.75 * Subsuit.XRot);

			Subsuit.XRot -= ANGLE(2.0f);

			if (Subsuit.XRot < Subsuit.dXRot)
				Subsuit.XRot = Subsuit.dXRot;
		}
	}
	else
	{
		if (Subsuit.XRot < 0 && Subsuit.dXRot > 0)
			Subsuit.XRot = ceil(0.75 * Subsuit.XRot);

		Subsuit.XRot += ANGLE(2.0f);

		if (Subsuit.XRot > Subsuit.dXRot)
			Subsuit.XRot = Subsuit.dXRot;
	}

	if (Subsuit.dXRot != 0)
	{
		short rot = Subsuit.dXRot >> 3;
		if (rot < -ANGLE(2.0f))
			rot = -ANGLE(2.0f);
		else if (rot > ANGLE(2.0f))
			rot = ANGLE(2.0f);

		item->Position.xRot += rot;
	}

	Subsuit.Vel[0] += abs(Subsuit.XRot >> 3);
	Subsuit.Vel[1] += abs(Subsuit.XRot >> 3);

	if (info->Control.TurnRate > 0)
	{
		Subsuit.Vel[0] += 2 * abs(info->Control.TurnRate);
	}
	else if (info->Control.TurnRate < 0)
	{
		Subsuit.Vel[1] += 2 * abs(info->Control.TurnRate);
	}

	if (Subsuit.Vel[0] > 1536)
		Subsuit.Vel[0] = 1536;

	if (Subsuit.Vel[1] > 1536)
		Subsuit.Vel[1] = 1536;

	if (Subsuit.Vel[0] != 0 || Subsuit.Vel[1] != 0)
	{
		SoundEffect(SFX_TR5_LARA_UNDERWATER_ENGINE, &item->Position, (((Subsuit.Vel[0] + Subsuit.Vel[1]) * 4) & 0x1F00) + 10);
	}
}

void SwimTurnSubsuit(ITEM_INFO* item)
{
	auto* info = GetLaraInfo(item);

	if (item->Position.yPos < 14080)
		Subsuit.YVel += (14080 - item->Position.yPos) >> 4;

	if (TrInput & IN_FORWARD && item->Position.xRot > -ANGLE(85.0f))
		Subsuit.dXRot = -ANGLE(45.0f);
	else if (TrInput & IN_BACK && item->Position.xRot < ANGLE(85.0f))
		Subsuit.dXRot = ANGLE(45.0f);
	else
		Subsuit.dXRot = 0;

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_SUBSUIT_TURN_RATE;
		if (info->Control.TurnRate < -LARA_MED_TURN_MAX)
			info->Control.TurnRate = -LARA_MED_TURN_MAX;

		item->Position.zRot -= LARA_LEAN_RATE;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_SUBSUIT_TURN_RATE;
		if (info->Control.TurnRate > LARA_MED_TURN_MAX)
			info->Control.TurnRate = LARA_MED_TURN_MAX;

		item->Position.zRot += LARA_LEAN_RATE;
	}
}

void SwimTurn(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	if (TrInput & IN_FORWARD)
		item->Position.xRot -= ANGLE(2.0f);
	else if (TrInput & IN_BACK)
		item->Position.xRot += ANGLE(2.0f);

	if (TrInput & IN_LEFT)
	{
		info->Control.TurnRate -= LARA_TURN_RATE;
		if (info->Control.TurnRate < -LARA_MED_TURN_MAX)
			info->Control.TurnRate = -LARA_MED_TURN_MAX;

		item->Position.zRot -= LARA_LEAN_RATE;
	}
	else if (TrInput & IN_RIGHT)
	{
		info->Control.TurnRate += LARA_TURN_RATE;
		if (info->Control.TurnRate > LARA_MED_TURN_MAX)
			info->Control.TurnRate = LARA_MED_TURN_MAX;

		item->Position.zRot += LARA_LEAN_RATE;
	}
}

void SwimDive(ITEM_INFO* item)
{
	auto* info = GetLaraInfo(item);

	SetAnimation(item, LA_ONWATER_DIVE);
	item->TargetState = LS_UNDERWATER_FORWARD;
	item->Position.xRot = ANGLE(-45.0f);
	item->VerticalVelocity = 80;
	info->Control.WaterStatus = WaterStatus::Underwater;
}

void LaraWaterCurrent(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	if (info->Control.WaterCurrentActive)
	{
		auto* sink = &g_Level.Sinks[info->Control.WaterCurrentActive - 1];

		short angle = mGetAngle(sink->x, sink->z, item->Position.xPos, item->Position.zPos);
		info->ExtraVelocity.x += (sink->strength * 1024 * phd_sin(angle - ANGLE(90.0f)) - info->ExtraVelocity.x) / 16;
		info->ExtraVelocity.z += (sink->strength * 1024 * phd_cos(angle - ANGLE(90.0f)) - info->ExtraVelocity.z) / 16;

		item->Position.yPos += (sink->y - item->Position.yPos) >> 4;
	}
	else
	{
		int shift = 0;

		if (abs(info->ExtraVelocity.x) <= 16)
			shift = (abs(info->ExtraVelocity.x) > 8) + 2;
		else
			shift = 4;
		info->ExtraVelocity.x -= info->ExtraVelocity.x >> shift;

		if (abs(info->ExtraVelocity.x) < 4)
			info->ExtraVelocity.x = 0;

		if (abs(info->ExtraVelocity.z) <= 16)
			shift = (abs(info->ExtraVelocity.z) > 8) + 2;
		else
			shift = 4;
		info->ExtraVelocity.z -= info->ExtraVelocity.z >> shift;

		if (abs(info->ExtraVelocity.z) < 4)
			info->ExtraVelocity.z = 0;

		if (!info->ExtraVelocity.x && !info->ExtraVelocity.z)
			return;
	}

	item->Position.xPos += info->ExtraVelocity.x >> 8;
	item->Position.zPos += info->ExtraVelocity.z >> 8;
	info->Control.WaterCurrentActive = 0;

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
