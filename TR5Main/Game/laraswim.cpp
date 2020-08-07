#include "framework.h"
#include "laraswim.h"
#include "control.h"
#include "camera.h"
#include "items.h"
#include "box.h"
#include "Lara.h"
#include "larasurf.h"
#include "effect.h"
#include "effect2.h"
#include "larafire.h"
#include "laramisc.h"
#include "draw.h"
#include "camera.h"
#include "level.h"
#include "input.h"
#include "sound.h"
#include "GameFlowScript.h"

typedef struct SUBSUIT_INFO
{
	short XRot;
	short dXRot;
	short XRotVel;
	short Vel[2];
	short YVel;
};
SUBSUIT_INFO Subsuit;
byte SubHitCount = 0;

void LaraWaterCurrent(COLL_INFO* coll) // (F) (D)
{
	if (Lara.currentActive)
	{
		OBJECT_VECTOR* sink = &FixedCameras[Lara.currentActive - 1];

		short angle = mGetAngle(sink->x, sink->z, LaraItem->pos.xPos, LaraItem->pos.zPos);
		Lara.currentXvel += ((sink->data * (phd_sin(angle - ANGLE(90)) / 4) >> 2) - Lara.currentXvel) >> 4;
		Lara.currentZvel += ((sink->data * (phd_cos(angle - ANGLE(90)) / 4) >> 2) - Lara.currentZvel) >> 4;

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

	coll->facing = phd_atan(LaraItem->pos.zPos - coll->old.z, LaraItem->pos.xPos - coll->old.x);

	GetCollisionInfo(coll, LaraItem->pos.xPos, LaraItem->pos.yPos + 200, LaraItem->pos.zPos, LaraItem->roomNumber, 400);
	
	if (coll->collType == CT_FRONT)
	{
		if (LaraItem->pos.xRot > ANGLE(35))
			LaraItem->pos.xRot += ANGLE(1);
		else if (LaraItem->pos.xRot < -ANGLE(35))
			LaraItem->pos.xRot -= ANGLE(1);
		else
			LaraItem->fallspeed = 0;
	}
	else if (coll->collType == CT_TOP)
	{
		LaraItem->pos.xRot -= ANGLE(1);
	}
	else if (coll->collType == CT_TOP_FRONT)
	{
		LaraItem->fallspeed = 0;
	}
	else if (coll->collType == CT_LEFT)
	{
		LaraItem->pos.yRot += ANGLE(5);
	}
	else if (coll->collType == CT_RIGHT)
	{
		LaraItem->pos.yRot -= ANGLE(5);
	}

	if (coll->midFloor < 0 && coll->midFloor != NO_HEIGHT)
		LaraItem->pos.yPos += coll->midFloor;

	ShiftItem(LaraItem, coll);

	coll->old.x = LaraItem->pos.xPos;
	coll->old.y = LaraItem->pos.yPos;
	coll->old.z = LaraItem->pos.zPos;
}

int GetWaterDepth(int x, int y, int z, short roomNumber)//4CA38, 4CE9C
{
	FLOOR_INFO* floor;
	ROOM_INFO* r = &g_Level.Rooms[roomNumber];
	
	short roomIndex = NO_ROOM;
	do
	{
		int zFloor = (z - r->z) >> WALL_SHIFT;
		int xFloor = (x - r->x) >> WALL_SHIFT;

		if (zFloor <= 0)
		{
			zFloor = 0;
			if (xFloor < 1)
				xFloor = 1;
			else if (xFloor > r->ySize - 2)
				xFloor = r->ySize - 2;
		}
		else if (zFloor >= r->xSize - 1)
		{
			zFloor = r->xSize - 1;
			if (xFloor < 1)
				xFloor = 1;
			else if (xFloor > r->ySize - 2)
				xFloor = r->ySize - 2;
		}
		else if (xFloor < 0)
			xFloor = 0;
		else if (xFloor >= r->ySize)
			xFloor = r->ySize - 1;

		floor = &r->floor[zFloor + xFloor * r->xSize];
		roomIndex = GetDoor(floor);
		if (roomIndex != NO_ROOM)
		{
			roomNumber = roomIndex;
			r = &g_Level.Rooms[roomIndex];
		}
	} while (roomIndex != NO_ROOM);

	if (r->flags & (ENV_FLAG_WATER|ENV_FLAG_SWAMP))
	{
		while (floor->skyRoom != NO_ROOM)
		{
			r = &g_Level.Rooms[floor->skyRoom];
			if (!(r->flags & (ENV_FLAG_WATER|ENV_FLAG_SWAMP)))
			{
				int wh = floor->ceiling << 8;
				floor = GetFloor(x, y, z, &roomNumber);
				return (GetFloorHeight(floor, x, y, z) - wh);
			}
			floor = &XZ_GET_SECTOR(r, x - r->x, z - r->z);  
		}
		return 0x7FFF;
	}
	else
	{
		while (floor->pitRoom != NO_ROOM)
		{
			r = &g_Level.Rooms[floor->pitRoom];
			if (r->flags & (ENV_FLAG_WATER|ENV_FLAG_SWAMP))
			{
				int wh = floor->floor << 8;
				floor = GetFloor(x, y, z, &roomNumber);
				return (GetFloorHeight(floor, x, y, z) - wh);
			}
			floor = &XZ_GET_SECTOR(r, x - r->x, z - r->z);
		}
		return NO_HEIGHT;
	}
}

void lara_col_waterroll(ITEM_INFO* item, COLL_INFO* coll)//4CA18(<), 4CE7C(<) (F)
{
	LaraSwimCollision(item, coll);
}

void lara_col_uwdeath(ITEM_INFO* item, COLL_INFO* coll)//4C980(<), 4CDE4(<) (F)
{
	item->hitPoints = -1;
	Lara.air = -1;
	Lara.gunStatus = LG_HANDS_BUSY;
	int wh = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
	if (wh != NO_HEIGHT)
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

	if (item->pos.xRot < -ANGLE(2) || item->pos.xRot > ANGLE(2))
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
		item->goalAnimState = LS_WATER_DEATH;
		return;
	}

	if (TrInput & IN_ROLL && LaraDrawType != LARA_DIVESUIT)
	{
		item->currentAnimState = LS_UNDERWATER_ROLL;
		item->animNumber = LA_UNDERWATER_ROLL_180_START;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
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
			item->goalAnimState = LS_UNDERWATER_FORWARD;

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
		item->goalAnimState = LS_WATER_DEATH;
		return;
	}

	if (TrInput & IN_ROLL)
	{
		if (LaraDrawType != LARA_DIVESUIT)
		{
			item->currentAnimState = LS_UNDERWATER_ROLL;
			item->animNumber = LA_UNDERWATER_ROLL_180_START;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
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
		item->goalAnimState = LS_UNDERWATER_FORWARD;

	item->fallspeed -= 6;
	if (item->fallspeed < 0)
		item->fallspeed = 0;

	if (item->fallspeed <= 133)
		item->goalAnimState = LS_UNDERWATER_STOP;
}

void lara_as_swim(ITEM_INFO* item, COLL_INFO* coll)//4C548(<), 4C9AC(<) (F)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_WATER_DEATH;
		return;
	}

	if (TrInput & IN_ROLL)
	{
		if (LaraDrawType != LARA_DIVESUIT)
		{
			item->currentAnimState = LS_UNDERWATER_ROLL;
			item->animNumber = LA_UNDERWATER_ROLL_180_START;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
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
		item->goalAnimState = LS_UNDERWATER_INERTIA;
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

		if (Lara.turnRate < -ANGLE(6))
			Lara.turnRate = -ANGLE(6);
	}
	else if (TrInput & IN_RIGHT)
	{
		Lara.turnRate += 613;

		if (Lara.turnRate > ANGLE(6))
			Lara.turnRate = ANGLE(6);
	}

	if (TrInput & IN_ACTION)
	{
		TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos, 31, 255, 255, 255);
	}

	if (TrInput & IN_OPTION)
	{
		Lara.turnRate = -ANGLE(12);
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

// CHECK all subsuit states
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

	coll->enableBaddiePush = true;
	coll->enableSpaz = false;

	coll->radius = 300;
	coll->trigger = NULL;

	if (TrInput & IN_LOOK && Lara.look)
		LookLeftRight();
	else
		ResetLook();

	Lara.look = true;

	lara_control_routines[item->currentAnimState](item, coll);

	if (LaraDrawType == LARA_DIVESUIT)
	{
		if (Lara.turnRate < -ANGLE(0.5))
		{
			Lara.turnRate += ANGLE(0.5);
		}
		else if (Lara.turnRate > ANGLE(0.5))
		{
			Lara.turnRate -= ANGLE(0.5);
		}
		else
		{
			Lara.turnRate = 0;
		}
	}
	else if (Lara.turnRate < -ANGLE(2))
	{
		Lara.turnRate += ANGLE(2);
	}
	else if (Lara.turnRate > ANGLE(2))
	{
		Lara.turnRate -= ANGLE(2);
	}
	else
	{
		Lara.turnRate = 0;
	}

	item->pos.yRot += Lara.turnRate;

	if (LaraDrawType == LARA_DIVESUIT)
		UpdateSubsuitAngles();

	if (item->pos.zRot < -ANGLE(2))
		item->pos.zRot += ANGLE(2);
	else if (item->pos.zRot > ANGLE(2))
		item->pos.zRot -= ANGLE(2);
	else
		item->pos.zRot = 0;

	if (item->pos.xRot < -ANGLE(85))
		item->pos.xRot = -ANGLE(85);
	else if (item->pos.xRot > ANGLE(85))
		item->pos.xRot = ANGLE(85);

	if (LaraDrawType == LARA_DIVESUIT)
	{
		if (item->pos.zRot > ANGLE(44))
			item->pos.zRot = ANGLE(44);
		else if (item->pos.zRot < -ANGLE(44))
			item->pos.zRot = -ANGLE(44);
	}
	else
	{
		if (item->pos.zRot > ANGLE(22))
			item->pos.zRot = ANGLE(22);
		else if (item->pos.zRot < -ANGLE(22))
			item->pos.zRot = -ANGLE(22);
	}

	if (Lara.currentActive && Lara.waterStatus != LW_FLYCHEAT)
		LaraWaterCurrent(coll);

	AnimateLara(item);

	item->pos.xPos += phd_cos(item->pos.xRot) * (item->fallspeed * phd_sin(item->pos.yRot) >> (W2V_SHIFT + 2)) >> W2V_SHIFT;
	item->pos.yPos -= item->fallspeed * phd_sin(item->pos.xRot) >> (W2V_SHIFT + 2);
	item->pos.zPos += phd_cos(item->pos.xRot) * (item->fallspeed * phd_cos(item->pos.yRot) >> (W2V_SHIFT + 2)) >> W2V_SHIFT;

	LaraBaddieCollision(item, coll);

	lara_collision_routines[item->currentAnimState](item, coll);

	UpdateLaraRoom(item, 0);

	LaraGun();

	TestTriggers(coll->trigger, 0, 0);
}

void UpdateSubsuitAngles()//4BD20, 4C184 (F)
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

			Subsuit.XRot -= ANGLE(2);

			if (Subsuit.XRot < Subsuit.dXRot)
			{
				Subsuit.XRot = Subsuit.dXRot;
			}
		}
	}
	else
	{
		if (Subsuit.XRot < 0 && Subsuit.dXRot > 0)
			Subsuit.XRot = ceil(0.75 * Subsuit.XRot);

		Subsuit.XRot += ANGLE(2);

		if (Subsuit.XRot > Subsuit.dXRot)
		{
			Subsuit.XRot = Subsuit.dXRot;
		}
	}

	if (Subsuit.dXRot != 0)
	{
		short rot = Subsuit.dXRot >> 3;
		if (rot < -ANGLE(2))
			rot = -ANGLE(2);
		else if (rot > ANGLE(2))
			rot = ANGLE(2);
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
		SoundEffect(SFX_LARA_UNDERWATER_ENGINE, &LaraItem->pos, (((Subsuit.Vel[0] + Subsuit.Vel[1]) * 4) & 0x1F00) + 10);
	}
}

void SwimTurnSubsuit(ITEM_INFO* item)//4BBDC, 4C040 (F)
{
	if (item->pos.yPos < 14080)
		Subsuit.YVel += (14080 - item->pos.yPos) >> 4;

	if (TrInput & IN_FORWARD && item->pos.xRot > -ANGLE(85))
	{
		Subsuit.dXRot = -ANGLE(45);
	}
	else if (TrInput & IN_BACK && item->pos.xRot < ANGLE(85))
	{
		Subsuit.dXRot = ANGLE(45);
	}
	else
	{
		Subsuit.dXRot = 0;
	}

	if (TrInput & IN_LEFT)
	{
		Lara.turnRate -= 136;

		if (Lara.turnRate < -ANGLE(6))
			Lara.turnRate = -ANGLE(6);

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
		if (Lara.turnRate < -ANGLE(6))
			Lara.turnRate = -ANGLE(6);
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
	int oldX = item->pos.xPos;
	int oldY = item->pos.yPos;
	int oldZ = item->pos.zPos;
	short oldXrot = item->pos.xRot;
	short oldYrot = item->pos.yRot;
	short oldZrot = item->pos.zRot;

	if (item->pos.xRot < -ANGLE(90) || item->pos.xRot > ANGLE(90))
	{
		Lara.moveAngle = ANGLE(180);
		coll->facing = item->pos.yRot - ANGLE(180);
	}
	else
	{
		Lara.moveAngle = 0;
		coll->facing = item->pos.yRot;
	}

	short height = 762 * phd_sin(item->pos.xRot) >> W2V_SHIFT;
	height = abs(height);

	if (height < ((LaraDrawType == LARA_DIVESUIT) << 6) + 200)
		height = ((LaraDrawType == LARA_DIVESUIT) << 6) + 200;
	
	coll->badNeg = -64;
	
	COLL_INFO c1;
	COLL_INFO c2;
	memcpy(&c1, coll, sizeof(COLL_INFO));
	memcpy(&c2, coll, sizeof(COLL_INFO) - 2);

	GetCollisionInfo(coll, item->pos.xPos, height / 2 + item->pos.yPos, item->pos.zPos, item->roomNumber, height);
	
	c1.facing += ANGLE(45);
	GetCollisionInfo(&c1, item->pos.xPos, height / 2 + item->pos.yPos, item->pos.zPos, item->roomNumber, height);
	
	c2.facing -= ANGLE(45);
	GetCollisionInfo(&c2, item->pos.xPos, height / 2 + item->pos.yPos, item->pos.zPos, item->roomNumber, height);
	
	ShiftItem(item, coll);
	
	int flag = 0;

	switch (coll->collType)
	{
	case CT_FRONT:
		if (item->pos.xRot <= ANGLE(25))
		{
			if (item->pos.xRot >= -ANGLE(25))
			{
				if (item->pos.xRot > ANGLE(5))
					item->pos.xRot += ANGLE(0.5);
				else if (item->pos.xRot < -ANGLE(5))
					item->pos.xRot -= ANGLE(0.5);
				else if (item->pos.xRot > 0)
					item->pos.xRot += 45;
				else if (item->pos.xRot < 0)
					item->pos.xRot -= 45;
				else
				{
					item->fallspeed = 0;
					flag = 1;
				}
			}
			else
			{
				item->pos.xRot -= ANGLE(1);
				flag = 1;
			}
		}
		else
		{
			item->pos.xRot += ANGLE(1);
			flag = 1;
		}

		if (c1.collType == CT_LEFT)
		{
			item->pos.yRot += ANGLE(2);
		}
		else if (c1.collType == CT_RIGHT)
		{
			item->pos.yRot -= ANGLE(2);
		}
		else if (c2.collType == CT_LEFT)
		{
			item->pos.yRot += ANGLE(2);
		}
		else if (c2.collType == CT_RIGHT)
		{
			item->pos.yRot -= ANGLE(2);
		}
		break;

	case CT_TOP:
		if (item->pos.xRot >= -8190)
		{
			flag = 1;
			item->pos.xRot -= ANGLE(1);
		}
		break;

	case CT_TOP_FRONT:
		item->fallspeed = 0;
		flag = 1;
		break;

	case CT_LEFT:
		item->pos.yRot += ANGLE(2);
		flag = 1;
		break;

	case CT_RIGHT:
		item->pos.yRot -= ANGLE(2);
		flag = 1;
		break;

	case CT_CLAMP:
		flag = 2;
		item->pos.xPos = coll->old.x;
		item->pos.yPos = coll->old.y;
		item->pos.zPos = coll->old.z;
		item->fallspeed = 0;
		break;
	}

	if (coll->midFloor < 0 && coll->midFloor != NO_HEIGHT)
	{
		flag = 1;
		item->pos.xRot += ANGLE(1);
		item->pos.yPos += coll->midFloor;
	}

	if (oldX == item->pos.xPos
		&& oldY == item->pos.yPos
		&& oldZ == item->pos.zPos 
		&& oldXrot == item->pos.xRot
		&& oldYrot == item->pos.yRot
		|| flag != 1)
	{
		if (flag == 2)
			return;
	}
	else if (item->fallspeed > 100)
	{
		if (LaraDrawType == 5)
		{
			SoundEffect(SFX_SWIMSUIT_METAL_CLASH, &LaraItem->pos, ((2 * GetRandomControl() + 0x8000) << 8) | 6);
		}

		if (Lara.anxiety < 96)
		{
			Lara.anxiety += 16;
		}
	}

	if (Lara.waterStatus != LW_FLYCHEAT && Lara.ExtraAnim == NO_ITEM)
		LaraTestWaterDepth(item, coll);
}

void LaraTestWaterDepth(ITEM_INFO* item, COLL_INFO* coll)//4B4F8(<), 4B95C(<) (F)
{
	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	int wd = GetWaterDepth(item->pos.xPos, item->pos.yPos, item->pos.zPos, roomNumber);

	if (wd == NO_HEIGHT)
	{
		item->fallspeed = 0;
		item->pos.xPos = coll->old.x;
		item->pos.yPos = coll->old.y;
		item->pos.zPos = coll->old.z;
	}
	else if (wd <= 512)
	{
		item->animNumber = LA_UNDERWATER_TO_STAND;
		item->currentAnimState = LS_ONWATER_EXIT;
		item->goalAnimState = LS_STOP;
		item->pos.zRot = 0;
		item->pos.xRot = 0;
		item->speed = 0;
		item->fallspeed = 0;
		item->gravityStatus = false;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		Lara.waterStatus = LW_WADE;
		item->pos.yPos = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	}
}