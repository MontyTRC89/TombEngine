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

int GetWaterDepth(int x, int y, int z, short roomNumber)
{
	FLOOR_INFO* floor;
	ROOM_INFO* r = &g_Level.Rooms[roomNumber];
	
	short roomIndex = NO_ROOM;
	do
	{
		int zFloor = (z - r->z) / SECTOR(1);
		int xFloor = (x - r->x) / SECTOR(1);

		if (zFloor <= 0)
		{
			zFloor = 0;
			if (xFloor < 1)
				xFloor = 1;
			else if (xFloor > r->xSize - 2)
				xFloor = r->xSize - 2;
		}
		else if (zFloor >= r->zSize - 1)
		{
			zFloor = r->zSize - 1;
			if (xFloor < 1)
				xFloor = 1;
			else if (xFloor > r->xSize - 2)
				xFloor = r->xSize - 2;
		}
		else if (xFloor < 0)
			xFloor = 0;
		else if (xFloor >= r->xSize)
			xFloor = r->xSize - 1;

		floor = &r->floor[zFloor + xFloor * r->zSize];
		roomIndex = floor->WallPortal;
		if (roomIndex != NO_ROOM)
		{
			roomNumber = roomIndex;
			r = &g_Level.Rooms[roomIndex];
		}
	} while (roomIndex != NO_ROOM);

	if (r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
	{
		while (floor->RoomAbove(x, y, z).value_or(NO_ROOM) != NO_ROOM)
		{
			r = &g_Level.Rooms[floor->RoomAbove(x, y, z).value_or(floor->Room)];
			if (!(r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP)))
			{
				int wh = floor->CeilingHeight(x, z);
				floor = GetFloor(x, y, z, &roomNumber);
				return (GetFloorHeight(floor, x, y, z) - wh);
			}
			floor = GetSector(r, x - r->x, z - r->z);
		}

		return DEEP_WATER;
	}
	else
	{
		while (floor->RoomBelow(x, y, z).value_or(NO_ROOM) != NO_ROOM)
		{
			r = &g_Level.Rooms[floor->RoomBelow(x, y, z).value_or(floor->Room)];
			if (r->flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
			{
				int wh = floor->FloorHeight(x, z);
				floor = GetFloor(x, y, z, &roomNumber);
				return (GetFloorHeight(floor, x, y, z) - wh);
			}
			floor = GetSector(r, x - r->x, z - r->z);
		}

		return NO_HEIGHT;
	}
}

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

	if (TrInput & IN_ROLL && LaraDrawType != LARA_TYPE::DIVESUIT)
	{
		item->animNumber = LA_UNDERWATER_ROLL_180_START;
		item->frameNumber = GF(LA_UNDERWATER_ROLL_180_START, 0);
		item->currentAnimState = LS_UNDERWATER_ROLL;
	}
	else
	{
		if (TrInput & IN_LOOK)
			LookUpDown();

		if (LaraDrawType == LARA_TYPE::DIVESUIT)
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

void lara_as_glide(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_WATER_DEATH;

		return;
	}

	if (TrInput & IN_ROLL)
	{
		if (LaraDrawType != LARA_TYPE::DIVESUIT)
		{
			item->animNumber = LA_UNDERWATER_ROLL_180_START;
			item->frameNumber = GF(LA_UNDERWATER_ROLL_180_START, 0);
			item->currentAnimState = LS_UNDERWATER_ROLL;

			return;
		}
	}
	else if (LaraDrawType != LARA_TYPE::DIVESUIT)
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

	if (TrInput & IN_ROLL)
	{
		if (LaraDrawType != LARA_TYPE::DIVESUIT)
		{
			item->animNumber = LA_UNDERWATER_ROLL_180_START;
			item->frameNumber = GF(LA_UNDERWATER_ROLL_180_START, 0);
			item->currentAnimState = LS_UNDERWATER_ROLL;

			return;
		}
	}
	else if (LaraDrawType != LARA_TYPE::DIVESUIT)
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

void LaraSwimCollision(ITEM_INFO* item, COLL_INFO* coll)
{
	int oldX = item->pos.xPos;
	int oldY = item->pos.yPos;
	int oldZ = item->pos.zPos;
	short oldXrot = item->pos.xRot;
	short oldYrot = item->pos.yRot;
	short oldZrot = item->pos.zRot;

	if (item->pos.xRot < -ANGLE(90.0f) ||
		item->pos.xRot > ANGLE(90.0f))
	{
		Lara.moveAngle = item->pos.yRot + ANGLE(180.0f);
		coll->Setup.ForwardAngle = item->pos.yRot - ANGLE(180.0f);
	}
	else
	{
		Lara.moveAngle = item->pos.yRot;
		coll->Setup.ForwardAngle = item->pos.yRot;
	}

	int height = LARA_HEIGHT * phd_sin(item->pos.xRot);
	height = abs(height);

	if (height < ((LaraDrawType == LARA_TYPE::DIVESUIT) << 6) + 200)
		height = ((LaraDrawType == LARA_TYPE::DIVESUIT) << 6) + 200;
	
	coll->Setup.BadHeightUp = -(STEP_SIZE / 4);
	coll->Setup.Height = height;
	
	GetCollisionInfo(coll, item, PHD_VECTOR(0, height / 2, 0));

	auto c1 = *coll;
	c1.Setup.ForwardAngle += ANGLE(45.0f);
	GetCollisionInfo(&c1, item, PHD_VECTOR(0, height / 2, 0));

	auto c2 = *coll;
	c2.Setup.ForwardAngle -= ANGLE(45.0f);
	GetCollisionInfo(&c2, item, PHD_VECTOR(0, height / 2, 0));
	
	ShiftItem(item, coll);
	
	int flag = 0;

	switch (coll->CollisionType)
	{
	case CT_FRONT:
		if (item->pos.xRot <= ANGLE(25.0f))
		{
			if (item->pos.xRot >= -ANGLE(25.0f))
			{
				if (item->pos.xRot > ANGLE(5.0f))
					item->pos.xRot += ANGLE(0.5f);
				else if (item->pos.xRot < -ANGLE(5.0f))
					item->pos.xRot -= ANGLE(0.5f);
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
				item->pos.xRot -= ANGLE(1.0f);
				flag = 1;
			}
		}
		else
		{
			item->pos.xRot += ANGLE(1.0f);
			flag = 1;
		}

		if (c1.CollisionType == CT_LEFT)
			item->pos.yRot += ANGLE(2.0f);
		else if (c1.CollisionType == CT_RIGHT)
			item->pos.yRot -= ANGLE(2.0f);
		else if (c2.CollisionType == CT_LEFT)
			item->pos.yRot += ANGLE(2.0f);
		else if (c2.CollisionType == CT_RIGHT)
			item->pos.yRot -= ANGLE(2.0f);

		break;

	case CT_TOP:
		if (item->pos.xRot >= -8190)
		{
			flag = 1;
			item->pos.xRot -= ANGLE(1.0f);
		}

		break;

	case CT_TOP_FRONT:
		item->fallspeed = 0;
		flag = 1;

		break;

	case CT_LEFT:
		item->pos.yRot += ANGLE(2.0f);
		flag = 1;

		break;

	case CT_RIGHT:
		item->pos.yRot -= ANGLE(2.0f);
		flag = 1;

		break;

	case CT_CLAMP:
		flag = 2;
		item->pos.xPos = coll->Setup.OldPosition.x;
		item->pos.yPos = coll->Setup.OldPosition.y;
		item->pos.zPos = coll->Setup.OldPosition.z;
		item->fallspeed = 0;

		break;
	}

	if (coll->Middle.Floor < 0 && coll->Middle.Floor != NO_HEIGHT)
	{
		flag = 1;
		item->pos.xRot += ANGLE(1.0f);
		item->pos.yPos += coll->Middle.Floor;
	}

	if (oldX == item->pos.xPos &&
		oldY == item->pos.yPos &&
		oldZ == item->pos.zPos &&
		oldXrot == item->pos.xRot &&
		oldYrot == item->pos.yRot ||
		flag != 1)
	{
		if (flag == 2)
			return;
	}

	if (Lara.waterStatus != LW_FLYCHEAT && Lara.ExtraAnim == NO_ITEM)
		TestLaraWaterDepth(item, coll);
}

void TestLaraWaterDepth(ITEM_INFO* item, COLL_INFO* coll)
{
	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	int waterDepth = GetWaterDepth(item->pos.xPos, item->pos.yPos, item->pos.zPos, roomNumber);

	if (waterDepth == NO_HEIGHT)
	{
		item->fallspeed = 0;
		item->pos.xPos = coll->Setup.OldPosition.x;
		item->pos.yPos = coll->Setup.OldPosition.y;
		item->pos.zPos = coll->Setup.OldPosition.z;
	}

	// Height check was at STEP_SIZE * 2 before but changed to this 
	// because now Lara surfaces on a head level, not mid-body level.

	if (waterDepth <= LARA_HEIGHT - LARA_HEADROOM / 2)
	{
		item->animNumber = LA_UNDERWATER_TO_STAND;
		item->frameNumber = GF(LA_UNDERWATER_TO_STAND, 0);
		item->currentAnimState = LS_ONWATER_EXIT;
		item->goalAnimState = LS_STOP;
		item->pos.zRot = 0;
		item->pos.xRot = 0;
		item->speed = 0;
		item->fallspeed = 0;
		item->gravityStatus = false;
		Lara.waterStatus = LW_WADE;
		item->pos.yPos = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	}
}