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
	item->HitPoints = -1;
	Lara.air = -1;
	Lara.Control.HandStatus = HandStatus::Busy;

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
	Lara.Control.CanLook = false;

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

	if (Lara.Control.HandStatus == HandStatus::Busy)
		Lara.Control.HandStatus = HandStatus::Free;
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

void UpdateSubsuitAngles()
{
	if (Subsuit.YVel != 0)
	{
		LaraItem->Position.yPos += Subsuit.YVel / 4;
		Subsuit.YVel = ceil(0.9375 * Subsuit.YVel - 1); // YVel * (15/16)
	}

	Subsuit.Vel[0] = Subsuit.Vel[1] = -4 * LaraItem->VerticalVelocity;

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

		LaraItem->Position.xRot += rot;
	}

	Subsuit.Vel[0] += abs(Subsuit.XRot >> 3);
	Subsuit.Vel[1] += abs(Subsuit.XRot >> 3);

	if (Lara.Control.TurnRate > 0)
	{
		Subsuit.Vel[0] += 2 * abs(Lara.Control.TurnRate);
	}
	else if (Lara.Control.TurnRate < 0)
	{
		Subsuit.Vel[1] += 2 * abs(Lara.Control.TurnRate);
	}

	if (Subsuit.Vel[0] > 1536)
		Subsuit.Vel[0] = 1536;

	if (Subsuit.Vel[1] > 1536)
		Subsuit.Vel[1] = 1536;

	if (Subsuit.Vel[0] != 0 || Subsuit.Vel[1] != 0)
	{
		SoundEffect(SFX_TR5_LARA_UNDERWATER_ENGINE, &LaraItem->Position, (((Subsuit.Vel[0] + Subsuit.Vel[1]) * 4) & 0x1F00) + 10);
	}
}

void SwimTurnSubsuit(ITEM_INFO* item)
{
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
		Lara.Control.TurnRate -= LARA_SUBSUIT_TURN_RATE;
		if (Lara.Control.TurnRate < -LARA_MED_TURN_MAX)
			Lara.Control.TurnRate = -LARA_MED_TURN_MAX;

		item->Position.zRot -= LARA_LEAN_RATE;
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.Control.TurnRate += LARA_SUBSUIT_TURN_RATE;
		if (Lara.Control.TurnRate > LARA_MED_TURN_MAX)
			Lara.Control.TurnRate = LARA_MED_TURN_MAX;

		item->Position.zRot += LARA_LEAN_RATE;
	}
}

void SwimTurn(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_FORWARD)
		item->Position.xRot -= ANGLE(2.0f);
	else if (TrInput & IN_BACK)
		item->Position.xRot += ANGLE(2.0f);

	if (TrInput & IN_LEFT)
	{
		Lara.Control.TurnRate -= LARA_TURN_RATE;
		if (Lara.Control.TurnRate < -LARA_MED_TURN_MAX)
			Lara.Control.TurnRate = -LARA_MED_TURN_MAX;

		item->Position.zRot -= LARA_LEAN_RATE;
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.Control.TurnRate += LARA_TURN_RATE;
		if (Lara.Control.TurnRate > LARA_MED_TURN_MAX)
			Lara.Control.TurnRate = LARA_MED_TURN_MAX;

		item->Position.zRot += LARA_LEAN_RATE;
	}
}

void SwimDive(ITEM_INFO* item)
{

	SetAnimation(item, LA_ONWATER_DIVE);
	item->TargetState = LS_UNDERWATER_FORWARD;
	item->Position.xRot = ANGLE(-45.0f);
	item->VerticalVelocity = 80;
	Lara.Control.WaterStatus = WaterStatus::Underwater;
}

void LaraWaterCurrent(COLL_INFO* coll)
{
	if (Lara.currentActive)
	{
		SINK_INFO* sink = &g_Level.Sinks[Lara.currentActive - 1];

		short angle = mGetAngle(sink->x, sink->z, LaraItem->Position.xPos, LaraItem->Position.zPos);
		Lara.Control.ExtraVelocity.x += (sink->strength * 1024 * phd_sin(angle - ANGLE(90.0f)) - Lara.Control.ExtraVelocity.x) / 16;
		Lara.Control.ExtraVelocity.z += (sink->strength * 1024 * phd_cos(angle - ANGLE(90.0f)) - Lara.Control.ExtraVelocity.z) / 16;

		LaraItem->Position.yPos += (sink->y - LaraItem->Position.yPos) >> 4;
	}
	else
	{
		int shift = 0;

		if (abs(Lara.Control.ExtraVelocity.x) <= 16)
			shift = (abs(Lara.Control.ExtraVelocity.x) > 8) + 2;
		else
			shift = 4;
		Lara.Control.ExtraVelocity.x -= Lara.Control.ExtraVelocity.x >> shift;

		if (abs(Lara.Control.ExtraVelocity.x) < 4)
			Lara.Control.ExtraVelocity.x = 0;

		if (abs(Lara.Control.ExtraVelocity.z) <= 16)
			shift = (abs(Lara.Control.ExtraVelocity.z) > 8) + 2;
		else
			shift = 4;
		Lara.Control.ExtraVelocity.z -= Lara.Control.ExtraVelocity.z >> shift;

		if (abs(Lara.Control.ExtraVelocity.z) < 4)
			Lara.Control.ExtraVelocity.z = 0;

		if (!Lara.Control.ExtraVelocity.x && !Lara.Control.ExtraVelocity.z)
			return;
	}

	LaraItem->Position.xPos += Lara.Control.ExtraVelocity.x >> 8;
	LaraItem->Position.zPos += Lara.Control.ExtraVelocity.z >> 8;
	Lara.currentActive = 0;

	coll->Setup.ForwardAngle = phd_atan(LaraItem->Position.zPos - coll->Setup.OldPosition.z, LaraItem->Position.xPos - coll->Setup.OldPosition.x);
	coll->Setup.Height = LARA_HEIGHT_CRAWL;

	GetCollisionInfo(coll, LaraItem, PHD_VECTOR(0, 200, 0));

	if (coll->CollisionType == CT_FRONT)
	{
		if (LaraItem->Position.xRot > ANGLE(35.0f))
			LaraItem->Position.xRot += ANGLE(1.0f);
		else if (LaraItem->Position.xRot < -ANGLE(35.0f))
			LaraItem->Position.xRot -= ANGLE(1.0f);
		else
			LaraItem->VerticalVelocity = 0;
	}
	else if (coll->CollisionType == CT_TOP)
		LaraItem->Position.xRot -= ANGLE(1.0f);
	else if (coll->CollisionType == CT_TOP_FRONT)
		LaraItem->VerticalVelocity = 0;
	else if (coll->CollisionType == CT_LEFT)
		LaraItem->Position.yRot += ANGLE(5.0f);
	else if (coll->CollisionType == CT_RIGHT)
		LaraItem->Position.yRot -= ANGLE(5.0f);

	if (coll->Middle.Floor < 0 && coll->Middle.Floor != NO_HEIGHT)
		LaraItem->Position.yPos += coll->Middle.Floor;

	ShiftItem(LaraItem, coll);

	coll->Setup.OldPosition.x = LaraItem->Position.xPos;
	coll->Setup.OldPosition.y = LaraItem->Position.yPos;
	coll->Setup.OldPosition.z = LaraItem->Position.zPos;
}
