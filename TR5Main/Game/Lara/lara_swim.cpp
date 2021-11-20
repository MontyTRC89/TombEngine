#include "framework.h"
#include "lara_swim.h"
#include "control/control.h"
#include "camera.h"
#include "items.h"
#include "Lara.h"
#include "animation.h"
#include "level.h"
#include "input.h"
#include "Sound/sound.h"
#include "GameFlowScript.h"
#include "lara_collide.h"

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
	item->hitPoints = -1;
	Lara.air = -1;
	Lara.gunStatus = LG_HANDS_BUSY;

	auto waterHeight = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
	if (waterHeight < (item->pos.yPos - (STEP_SIZE / 5 * 2) - 2) &&
		waterHeight != NO_HEIGHT)
	{
		item->pos.yPos -= 5;
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
	item->fallspeed = 0;
}

void lara_as_uwdeath(ITEM_INFO* item, COLL_INFO* coll)
{
	Lara.look = 0;

	item->fallspeed -= 8;
	if (item->fallspeed <= 0)
		item->fallspeed = 0;

	if (item->pos.xRot < -ANGLE(2.0f) ||
		item->pos.xRot > ANGLE(2.0f))
	{
		if (item->pos.xRot >= 0)
			item->pos.xRot -= ANGLE(2.0f);
		else
			item->pos.xRot += ANGLE(2.0f);
	}
	else
		item->pos.xRot = 0;
}

void lara_as_dive(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TrInput & IN_FORWARD)
		item->pos.xRot -= ANGLE(1.0f);
}

void lara_as_tread(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_WATER_DEATH;
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
		SwimTurn(item);

	if (TrInput & IN_JUMP)
		item->goalAnimState = LS_UNDERWATER_FORWARD;

	item->fallspeed -= 6;

	if (item->fallspeed < 0)
		item->fallspeed = 0;

	if (Lara.gunStatus == LG_HANDS_BUSY)
		Lara.gunStatus = LG_NO_ARMS;
}

void lara_as_glide(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_WATER_DEATH;
		return;
	}

	auto level = g_GameFlow->GetLevel(CurrentLevel);

	if (TrInput & IN_ROLL && level->LaraType != LaraType::Divesuit)
	{
		SetAnimation(item, LA_UNDERWATER_ROLL_180_START);
		return;
	}
	
	if (level->LaraType != LaraType::Divesuit)
		SwimTurn(item);
	else
		SwimTurnSubsuit(item);

	if (TrInput & IN_JUMP)
		item->goalAnimState = LS_UNDERWATER_FORWARD;

	item->fallspeed -= 6;
	if (item->fallspeed < 0)
		item->fallspeed = 0;

	if (item->fallspeed <= 133)
		item->goalAnimState = LS_UNDERWATER_STOP;
}

void lara_as_swim(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_WATER_DEATH;
		return;
	}

	auto level = g_GameFlow->GetLevel(CurrentLevel);

	if (TrInput & IN_ROLL && level->LaraType != LaraType::Divesuit)
	{
		SetAnimation(item, LA_UNDERWATER_ROLL_180_START);
		return;
	}
	
	if (level->LaraType != LaraType::Divesuit)
		SwimTurn(item);
	else
		SwimTurnSubsuit(item);

	item->fallspeed += 8;

	if (item->fallspeed > 200)
		item->fallspeed = 200;

	if (!(TrInput & IN_JUMP))
		item->goalAnimState = LS_UNDERWATER_INERTIA;
}

void UpdateSubsuitAngles()
{
	if (Subsuit.YVel != 0)
	{
		LaraItem->pos.yPos += Subsuit.YVel / 4;
		Subsuit.YVel = ceil(0.9375 * Subsuit.YVel - 1); // YVel * (15/16)
	}

	Subsuit.Vel[0] = Subsuit.Vel[1] = -4 * LaraItem->fallspeed;

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

		LaraItem->pos.xRot += rot;
	}

	Subsuit.Vel[0] += abs(Subsuit.XRot >> 3);
	Subsuit.Vel[1] += abs(Subsuit.XRot >> 3);

	if (Lara.turnRate > 0)
	{
		Subsuit.Vel[0] += 2 * abs(Lara.turnRate);
	}
	else if (Lara.turnRate < 0)
	{
		Subsuit.Vel[1] += 2 * abs(Lara.turnRate);
	}

	if (Subsuit.Vel[0] > 1536)
		Subsuit.Vel[0] = 1536;

	if (Subsuit.Vel[1] > 1536)
		Subsuit.Vel[1] = 1536;

	if (Subsuit.Vel[0] != 0 || Subsuit.Vel[1] != 0)
	{
		SoundEffect(SFX_TR5_LARA_UNDERWATER_ENGINE, &LaraItem->pos, (((Subsuit.Vel[0] + Subsuit.Vel[1]) * 4) & 0x1F00) + 10);
	}
}

void SwimTurnSubsuit(ITEM_INFO* item)
{
	if (item->pos.yPos < 14080)
		Subsuit.YVel += (14080 - item->pos.yPos) >> 4;

	if (TrInput & IN_FORWARD && item->pos.xRot > -ANGLE(85.0f))
		Subsuit.dXRot = -ANGLE(45.0f);
	else if (TrInput & IN_BACK && item->pos.xRot < ANGLE(85.0f))
		Subsuit.dXRot = ANGLE(45.0f);
	else
		Subsuit.dXRot = 0;

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= SUB_SUIT_TURN_RATE;
		if (Lara.turnRate < -LARA_MED_TURN)
			Lara.turnRate = -LARA_MED_TURN;

		item->pos.zRot -= LARA_LEAN_RATE * 2;
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += SUB_SUIT_TURN_RATE;
		if (Lara.turnRate > LARA_MED_TURN)
			Lara.turnRate = LARA_MED_TURN;

		item->pos.zRot += LARA_LEAN_RATE * 2;
	}
}

void SwimTurn(ITEM_INFO* item)
{
	if (TrInput & IN_FORWARD)
		item->pos.xRot -= ANGLE(2.0f);
	else if (TrInput & IN_BACK)
		item->pos.xRot += ANGLE(2.0f);

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= LARA_TURN_RATE;
		if (Lara.turnRate < -LARA_MED_TURN)
			Lara.turnRate = -LARA_MED_TURN;

		item->pos.zRot -= LARA_LEAN_RATE * 2;
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += LARA_TURN_RATE;
		if (Lara.turnRate > LARA_MED_TURN)
			Lara.turnRate = LARA_MED_TURN;

		item->pos.zRot += LARA_LEAN_RATE * 2;
	}
}

void SwimDive(ITEM_INFO* item)
{

	SetAnimation(item, LA_ONWATER_DIVE);
	item->goalAnimState = LS_UNDERWATER_FORWARD;
	item->pos.xRot = ANGLE(-45.0f);
	item->fallspeed = 80;
	Lara.waterStatus = LW_UNDERWATER;
}

void LaraWaterCurrent(COLL_INFO* coll)
{
	if (Lara.currentActive)
	{
		SINK_INFO* sink = &g_Level.Sinks[Lara.currentActive - 1];

		short angle = mGetAngle(sink->x, sink->z, LaraItem->pos.xPos, LaraItem->pos.zPos);
		Lara.currentXvel += (sink->strength * 1024 * phd_sin(angle - ANGLE(90.0f)) - Lara.currentXvel) / 16;
		Lara.currentZvel += (sink->strength * 1024 * phd_cos(angle - ANGLE(90.0f)) - Lara.currentZvel) / 16;

		LaraItem->pos.yPos += (sink->y - LaraItem->pos.yPos) >> 4;
	}
	else
	{
		int shift = 0;

		if (abs(Lara.currentXvel) <= 16)
			shift = (abs(Lara.currentXvel) > 8) + 2;
		else
			shift = 4;
		Lara.currentXvel -= Lara.currentXvel >> shift;

		if (abs(Lara.currentXvel) < 4)
			Lara.currentXvel = 0;

		if (abs(Lara.currentZvel) <= 16)
			shift = (abs(Lara.currentZvel) > 8) + 2;
		else
			shift = 4;
		Lara.currentZvel -= Lara.currentZvel >> shift;

		if (abs(Lara.currentZvel) < 4)
			Lara.currentZvel = 0;

		if (!Lara.currentXvel && !Lara.currentZvel)
			return;
	}

	LaraItem->pos.xPos += Lara.currentXvel >> 8;
	LaraItem->pos.zPos += Lara.currentZvel >> 8;
	Lara.currentActive = 0;

	coll->Setup.ForwardAngle = phd_atan(LaraItem->pos.zPos - coll->Setup.OldPosition.z, LaraItem->pos.xPos - coll->Setup.OldPosition.x);
	coll->Setup.Height = LARA_HEIGHT_CRAWL;

	GetCollisionInfo(coll, LaraItem, PHD_VECTOR(0, 200, 0));

	if (coll->CollisionType == CT_FRONT)
	{
		if (LaraItem->pos.xRot > ANGLE(35.0f))
			LaraItem->pos.xRot += ANGLE(1.0f);
		else if (LaraItem->pos.xRot < -ANGLE(35.0f))
			LaraItem->pos.xRot -= ANGLE(1.0f);
		else
			LaraItem->fallspeed = 0;
	}
	else if (coll->CollisionType == CT_TOP)
		LaraItem->pos.xRot -= ANGLE(1.0f);
	else if (coll->CollisionType == CT_TOP_FRONT)
		LaraItem->fallspeed = 0;
	else if (coll->CollisionType == CT_LEFT)
		LaraItem->pos.yRot += ANGLE(5.0f);
	else if (coll->CollisionType == CT_RIGHT)
		LaraItem->pos.yRot -= ANGLE(5.0f);

	if (coll->Middle.Floor < 0 && coll->Middle.Floor != NO_HEIGHT)
		LaraItem->pos.yPos += coll->Middle.Floor;

	ShiftItem(LaraItem, coll);

	coll->Setup.OldPosition.x = LaraItem->pos.xPos;
	coll->Setup.OldPosition.y = LaraItem->pos.yPos;
	coll->Setup.OldPosition.z = LaraItem->pos.zPos;
}
