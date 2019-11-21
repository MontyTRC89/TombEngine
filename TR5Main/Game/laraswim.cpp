#include "laraswim.h"

#include "..\Global\global.h"
#include "control.h"
#include "camera.h"
#include "collide.h"
#include "items.h"
#include "box.h"
#include "Lara.h"
#include "larasurf.h"

SUBSUIT_INFO subsuit;
char SubHitCount = 0;

void LaraWaterCurrent(COLL_INFO* coll)//4CD34, 4D198
{
	UNIMPLEMENTED();
}

long GetWaterDepth(long x, long y, long z, short room_number)//4CA38, 4CE9C
{
	UNIMPLEMENTED();
	return 0;
}

void lara_col_waterroll(ITEM_INFO* item, COLL_INFO* coll)//4CA18(<), 4CE7C(<) (F)
{
	LaraSwimCollision(item, coll);
}

void lara_col_uwdeath(ITEM_INFO* item, COLL_INFO* coll)//4C980(<), 4CDE4(<) (F)
{
	int wh;
	item->hitPoints = -1;
	Lara.air = -1;
	Lara.gunStatus = LG_HANDS_BUSY;
	wh = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->room_number);
	if (wh != -32512)
	{
		if (wh < item->pos.yPos - 100)
			item->pos.yPos -= 5;
	}
	LaraSwimCollision(item, coll);
}

void lara_col_dive(ITEM_INFO* item, COLL_INFO* coll)//4C960(<), 4CDC4(<) (F)
{
	LaraSwimCollision(item, coll);
}

void lara_col_tread(ITEM_INFO* item, COLL_INFO* coll)//4C940(<), 4CDA4(<) (F)
{
	LaraSwimCollision(item, coll);
}

void lara_col_glide(ITEM_INFO* item, COLL_INFO* coll)//4C920(<), 4CD84(<) (F)
{
	LaraSwimCollision(item, coll);
}

void lara_col_swim(ITEM_INFO* item, COLL_INFO* coll)//4C900(<), 4CD64(<) (F)
{
	LaraSwimCollision(item, coll);
}

void lara_as_waterroll(ITEM_INFO* item, COLL_INFO* coll)//4C8F8(<), 4CD5C(<) (F)
{
	item->fallspeed = 0;
}

void lara_as_uwdeath(ITEM_INFO* item, COLL_INFO* coll)//4C884(<), 4CCE8(<) (F)
{
	Lara.look = 0;

	item->fallspeed -= 8;
	if (item->fallspeed <= 0)
		item->fallspeed = 0;

	if (item->pos.xRot < ANGLE(-2) || item->pos.xRot > ANGLE(2))
	{
		if (item->pos.xRot >= 0)
			item->pos.xRot -= ANGLE(2);
		else
			item->pos.xRot += ANGLE(2);
	}
	else
	{
		item->pos.xRot = 0;
	}
}

void lara_as_dive(ITEM_INFO* item, COLL_INFO* coll)//4C854, 4CCB8 (F)
{
	if (TrInput & IN_FORWARD)
	{
		item->pos.xRot -= ANGLE(1);
	}
}

void lara_as_tread(ITEM_INFO* item, COLL_INFO* coll)//4C730, 4CB94 (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = STATE_LARA_WATER_DEATH;

		return;
	}

	if (TrInput & IN_ROLL && LaraDrawType != LARA_DIVESUIT)
	{
		item->currentAnimState = STATE_LARA_UNDERWATER_TURNAROUND;
		item->animNumber = ANIMATION_LARA_UNDERWATER_ROLL_BEGIN;
		item->frameNumber = Anims[ANIMATION_LARA_UNDERWATER_ROLL_BEGIN].frameBase;
	}
	else
	{
		if (TrInput & IN_LOOK)
			LookUpDown();

		if (LaraDrawType == LARA_DIVESUIT)
			SwimTurnSubsuit(item);
		else
			SwimTurn(item);

		if (TrInput & IN_JUMP)
			item->goalAnimState = STATE_LARA_UNDERWATER_FORWARD;

		item->fallspeed -= 6;

		if (item->fallspeed < 0)
			item->fallspeed = 0;

		if (Lara.gunStatus == LG_HANDS_BUSY)
			Lara.gunStatus = LG_NO_ARMS;
	}
}

void lara_as_glide(ITEM_INFO* item, COLL_INFO* coll)//4C634(<), 4CA98(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = STATE_LARA_WATER_DEATH;

		return;
	}

	if (TrInput & IN_ROLL)
	{
		if (LaraDrawType != LARA_DIVESUIT)
		{
			item->currentAnimState = STATE_LARA_UNDERWATER_TURNAROUND;
			item->animNumber = ANIMATION_LARA_UNDERWATER_ROLL_BEGIN;
			item->frameNumber = Anims[ANIMATION_LARA_UNDERWATER_ROLL_BEGIN].frameBase;
			return;
		}
	}
	else if (LaraDrawType != LARA_DIVESUIT)
	{
		SwimTurn(item);
	}
	else
	{
		SwimTurnSubsuit(item);
	}

	if (TrInput & IN_JUMP)
		item->goalAnimState = STATE_LARA_UNDERWATER_FORWARD;

	item->fallspeed -= 6;
	if (item->fallspeed < 0)
		item->fallspeed = 0;

	if (item->fallspeed <= 133)
		item->goalAnimState = STATE_LARA_UNDERWATER_STOP;
}

void lara_as_swim(ITEM_INFO* item, COLL_INFO* coll)//4C548(<), 4C9AC(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = STATE_LARA_WATER_DEATH;

		return;
	}

	if (TrInput & IN_ROLL)
	{
		if (LaraDrawType != LARA_DIVESUIT)
		{
			item->currentAnimState = STATE_LARA_UNDERWATER_TURNAROUND;
			item->animNumber = ANIMATION_LARA_UNDERWATER_ROLL_BEGIN;
			item->frameNumber = Anims[ANIMATION_LARA_UNDERWATER_ROLL_BEGIN].frameBase;
			return;
		}
	}
	else if (LaraDrawType != LARA_DIVESUIT)
	{
		SwimTurn(item);
	}
	else
	{
		SwimTurnSubsuit(item);
	}

	item->fallspeed += 8;

	if (item->fallspeed > 200)
		item->fallspeed = 200;

	if (!(TrInput & IN_JUMP))
		item->goalAnimState = STATE_LARA_UNDERWATER_INERTIA;
}

void lara_as_swimcheat(ITEM_INFO* item, COLL_INFO* coll)//4C3A8, 4C80C (F)
{
	if (TrInput & IN_FORWARD)
	{
		item->pos.xRot -= ANGLE(3);
	}
	else if (TrInput & IN_BACK)
	{
		item->pos.xRot += ANGLE(3);
	}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= 613;

		if (Lara.turnRate < ANGLE(-6))
			Lara.turnRate = ANGLE(-6);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += 613;

		if (Lara.turnRate > ANGLE(6))
			Lara.turnRate = ANGLE(6);
	}

	if (TrInput & IN_ACTION)
	{
		TriggerDynamics(item->pos.xPos, item->pos.yPos, item->pos.zPos, 31, 255, 255, 255);
	}

	if (TrInput & IN_OPTION)
	{
		Lara.turnRate = ANGLE(-12);
	}

	if (TrInput & IN_JUMP)
	{
		item->fallspeed += 16;

		if (item->fallspeed > 400)
			item->fallspeed = 400;
	}
	else
	{
		if (item->fallspeed >= 8)
			item->fallspeed -= item->fallspeed >> 3;
		else
			item->fallspeed = 0;
	}
}

void LaraUnderWater(ITEM_INFO* item, COLL_INFO* coll)//4BFB4, 4C418 (F)
{
	coll->badPos = 32512;
	coll->badNeg = -400;
	coll->badCeiling = 400;

	coll->old.x = item->pos.xPos;
	coll->old.y = item->pos.yPos;
	coll->old.z = item->pos.zPos;

	coll->slopesAreWalls = 0;
	coll->slopesArePits = 0;
	coll->lavaIsPit = 0;

	coll->enableBaddiePush = TRUE;
	coll->enableSpaz = false;

	coll->radius = 300;
	coll->trigger = 0;

	if (TrInput & IN_LOOK && Lara.look)
		LookLeftRight();
	else
		ResetLook();

	Lara.look = TRUE;

	lara_control_routines[item->currentAnimState](item, coll);

	if (LaraDrawType == LARA_DIVESUIT)
	{
		Lara.turnRate = CLAMPADD(Lara.turnRate, ANGLE(-0.5), ANGLE(0.5));
	}
	else
	{
		Lara.turnRate = CLAMPADD(Lara.turnRate, ANGLE(-2), ANGLE(2));
	}

	item->pos.yRot += Lara.turnRate;

	if (LaraDrawType == LARA_DIVESUIT)
		UpdateSubsuitAngles();

	item->pos.zRot = CLAMPADD(item->pos.zRot, ANGLE(-2), ANGLE(2));
	item->pos.xRot = CLAMPADD(item->pos.xRot, ANGLE(-85), ANGLE(85));

	if (LaraDrawType == LARA_DIVESUIT)
	{
		item->pos.zRot = CLAMP(item->pos.zRot, ANGLE(-44), ANGLE(44));
	}
	else
	{
		item->pos.zRot = CLAMP(item->pos.zRot, ANGLE(-22), ANGLE(22));
	}

	if (Lara.currentActive && Lara.waterStatus != LW_FLYCHEAT)
		LaraWaterCurrent(coll);

	AnimateLara(item);

	item->pos.xPos += 4 * COS(item->pos.xRot) * (item->fallspeed * SIN(item->pos.yRot) >> W2V_SHIFT) >> W2V_SHIFT;
	item->pos.yPos -= item->fallspeed * 4 * SIN(item->pos.xRot) >> W2V_SHIFT >> 2;
	item->pos.zPos += 4 * COS(item->pos.xRot) * (item->fallspeed * COS(item->pos.yRot) >> W2V_SHIFT) >> W2V_SHIFT;

	LaraBaddieCollision(item, coll);

	lara_collision_routines[item->currentAnimState](item, coll);

	UpdateLaraRoom(item, 0);

	LaraGun();

	TestTriggers(coll->trigger, 0, 0);
}

void UpdateSubsuitAngles()//4BD20, 4C184 (F)
{
#ifdef PC_VERSION
	if (subsuit.YVel != 0)
	{
		lara_item->pos.yPos += subsuit.YVel / 4;
		subsuit.YVel = ceil(0.9375 * subsuit.YVel - 1); // YVel * (15/16)
	}

	subsuit.Vel[0] = subsuit.Vel[1] = -4 * lara_item->fallspeed;

	if (subsuit.XRot >= subsuit.dXRot)
	{
		if (subsuit.XRot > subsuit.dXRot)
		{
			if (subsuit.XRot > 0 && subsuit.dXRot < 0)
				subsuit.XRot = ceil(0.75 * subsuit.XRot);

			subsuit.XRot -= ANGLE(2);

			if (subsuit.XRot < subsuit.dXRot)
			{
				subsuit.XRot = subsuit.dXRot;
			}
		}
	}
	else
	{
		if (subsuit.XRot < 0 && subsuit.dXRot > 0)
			subsuit.XRot = ceil(0.75 * subsuit.XRot);

		subsuit.XRot += ANGLE(2);

		if (subsuit.XRot > subsuit.dXRot)
		{
			subsuit.XRot = subsuit.dXRot;
		}
	}

	if (subsuit.dXRot != 0)
	{
		lara_item->pos.xRot += CLAMP(subsuit.dXRot >> 3, ANGLE(-2), ANGLE(2));
	}

	subsuit.Vel[0] += abs(subsuit.XRot >> 3);
	subsuit.Vel[1] += abs(subsuit.XRot >> 3);

	if (Lara.turnRate > 0)
	{
		subsuit.Vel[0] += 2 * abs(Lara.turnRate);
	}
	else if (Lara.turnRate < 0)
	{
		subsuit.Vel[1] += 2 * abs(Lara.turnRate);
	}

	if (subsuit.Vel[0] > 1536)
		subsuit.Vel[0] = 1536;

	if (subsuit.Vel[1] > 1536)
		subsuit.Vel[1] = 1536;

	if (subsuit.Vel[0] != 0 || subsuit.Vel[1] != 0)
	{
		// todo make the formula clearer
		SoundEffect(SFX_LARA_UNDERWATER_ENGINE, &lara_item->pos, (((subsuit.Vel[0] + subsuit.Vel[1]) * 4) & 0x1F00) + 10);
	}
#endif
}

void SwimTurnSubsuit(ITEM_INFO* item)//4BBDC, 4C040 (F)
{
	if (item->pos.yPos < 14080)
		subsuit.YVel += (14080 - item->pos.yPos) >> 4;

	if (TrInput & IN_FORWARD && item->pos.xRot > ANGLE(-85))
	{
		subsuit.dXRot = ANGLE(-45);
	}
	else if (TrInput & IN_BACK && item->pos.xRot < ANGLE(85))
	{
		subsuit.dXRot = ANGLE(45);
	}
	else
	{
		subsuit.dXRot = 0;
	}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= 136;

		if (Lara.turnRate < ANGLE(-6))
			Lara.turnRate = ANGLE(-6);

		item->pos.zRot -= ANGLE(3);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += 136;

		if (Lara.turnRate > ANGLE(6))
			Lara.turnRate = ANGLE(6);

		item->pos.zRot += ANGLE(3);
	}
}

void SwimTurn(ITEM_INFO* item)//4BAF4(<), 4BF58(<) (F)
{
	if (TrInput & IN_FORWARD)
	{
		item->pos.xRot -= ANGLE(2);
	}
	else if (TrInput & IN_BACK)
	{
		item->pos.xRot += ANGLE(2);
	}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= 409;
		if (Lara.turnRate < ANGLE(-6))
			Lara.turnRate = ANGLE(-6);
		item->pos.zRot -= ANGLE(3);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += 409;
		if (Lara.turnRate > ANGLE(6))
			Lara.turnRate = ANGLE(6);
		item->pos.zRot += ANGLE(3);
	}
}

void LaraSwimCollision(ITEM_INFO* item, COLL_INFO* coll)//4B608, 4BA6C
{
	UNIMPLEMENTED();
}

void LaraTestWaterDepth(ITEM_INFO* item, COLL_INFO* coll)//4B4F8(<), 4B95C(<) (F)
{
	int wd;
	FLOOR_INFO* floor;
	short room_number;

	room_number = item->room_number;
	floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &room_number);
	wd = GetWaterDepth(item->pos.xPos, item->pos.yPos, item->pos.zPos, room_number);

	if (wd == -32512)
	{
		item->pos.xPos = coll->old.x;
		item->pos.yPos = coll->old.y;
		item->fallspeed = 0;
		item->pos.zPos = coll->old.z;
	}
	else if (wd < 513)
	{
		//loc_4B580
		item->animNumber = 192;
		item->currentAnimState = 55;
		item->goalAnimState = 2;
		item->pos.zRot = 0;
		item->pos.xRot = 0;
		item->speed = 0;
		item->fallspeed = 0;
		item->gravity_status = 0;
		item->frameNumber = Anims[192].frameBase;
		Lara.waterStatus = 4;
		item->pos.yPos = GetHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	}//loc_4B5F0
}