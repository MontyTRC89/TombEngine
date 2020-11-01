#include "framework.h"
#include "tr3_moveblock.h"
#include "Lara.h"
#include "draw.h"

#include "items.h"
#include "collide.h"
#include "effect.h"
#include "box.h"
#include "level.h"
#include "input.h"
#include "sound.h"

#define MAXOFF_MB	300
#define MAXOFF_Z	(LARA_RAD + 80)

#define STATE_MOVEBLOCK_STILL		1
#define STATE_MOVEBLOCK_PUSH		2
#define STATE_MOVEBLOCK_PULL		3

OBJECT_COLLISION_BOUNDS MovingBlockBounds = {
	-300, 300, 0, 0, -692, -512,
	-ANGLE(10.0f), ANGLE(10.0f),
	-ANGLE(30.0f), ANGLE(30.0f),
	-ANGLE(10.0f), ANGLE(10.0f)
};

void InitialiseMovingBlock(short itemNum)
{
	ITEM_INFO *item = &g_Level.Items[itemNum];

	ClearMovableBlockSplitters3(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);

	if (item->status != ITEM_INVISIBLE)
		AlterFloorHeight(item, -((item->triggerFlags - 64) * 256));
}

void ClearMovableBlockSplitters3(int x, int y, int z, short roomNumber)
{
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	g_Level.Boxes[floor->box].flags &= (~BLOCKED);
	short height = g_Level.Boxes[floor->box].height;
	short baseRoomNumber = roomNumber;

	floor = GetFloor(x + 1024, y, z, &roomNumber);
	if (floor->box != NO_BOX)
	{
		if (g_Level.Boxes[floor->box].height == height && (g_Level.Boxes[floor->box].flags & BLOCKABLE) && (g_Level.Boxes[floor->box].flags & BLOCKED))
			ClearMovableBlockSplitters3(x + 1024, y, z, roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x - 1024, y, z, &roomNumber);
	if (floor->box != NO_BOX)
	{
		if (g_Level.Boxes[floor->box].height == height && (g_Level.Boxes[floor->box].flags & BLOCKABLE) && (g_Level.Boxes[floor->box].flags & BLOCKED))
			ClearMovableBlockSplitters3(x - 1024, y, z, roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x, y, z + 1024, &roomNumber);
	if (floor->box != NO_BOX)
	{
		if (g_Level.Boxes[floor->box].height == height && (g_Level.Boxes[floor->box].flags & BLOCKABLE) && (g_Level.Boxes[floor->box].flags & BLOCKED))
			ClearMovableBlockSplitters3(x, y, z + 1024, roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x, y, z - 1024, &roomNumber);
	if (floor->box != NO_BOX)
	{
		if (g_Level.Boxes[floor->box].height == height && (g_Level.Boxes[floor->box].flags & BLOCKABLE) && (g_Level.Boxes[floor->box].flags & BLOCKED))
			ClearMovableBlockSplitters3(x, y, z - 1024, roomNumber);
	}
}

void MovableBlockControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->flags & ONESHOT)
	{
		AlterFloorHeight(item, ((item->triggerFlags - 64) * 256));
		KillItem(itemNumber);
		return;
	}

	AnimateItem(item);

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	int height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	if (item->pos.yPos < height)
		item->gravityStatus = true;
	else if (item->gravityStatus)
	{
		item->gravityStatus = false;
		item->pos.yPos = height;
		item->status = ITEM_DEACTIVATED;
		floor_shake_effect(item);
		SoundEffect(70, &item->pos, 0);
	}

	if (item->roomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	if (item->status == ITEM_DEACTIVATED)
	{
		item->status = ITEM_NOT_ACTIVE;
		RemoveActiveItem(itemNumber);
		AlterFloorHeight(item, -((item->triggerFlags - 64) * 256));
		AdjustStopperFlag(item, item->itemFlags[0] + 0x8000, 0);

		roomNumber = item->roomNumber;
		floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		TestTriggers(TriggerIndex, 1, 0);
	}
}

void MovableBlockCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	if (!(TrInput & IN_ACTION) ||
		item->status == ITEM_ACTIVE ||
		laraitem->gravityStatus ||
		laraitem->pos.yPos != item->pos.yPos)
		return;

	unsigned short quadrant = (unsigned short)(laraitem->pos.yRot + ANGLE(45)) / ANGLE(90);
	if (laraitem->currentAnimState == LS_STOP)
	{
		if (Lara.gunStatus != LG_NO_ARMS)
			return;

		switch (quadrant)
		{
		case NORTH:
			item->pos.yRot = 0;
			break;

		case EAST:
			item->pos.yRot = 16384;
			break;

		case SOUTH:
			item->pos.yRot = -32768;
			break;

		case WEST:
			item->pos.yRot = -16384;
			break;
		}
		if (!TestLaraPosition(&MovingBlockBounds, item, laraitem))
			return;

		short		roomNumber = laraitem->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		if (roomNumber != item->roomNumber)
			return;

		switch (quadrant)
		{
		case NORTH:
			laraitem->pos.zPos &= -WALL_SIZE;
			laraitem->pos.zPos += WALL_SIZE - LARA_RAD;
			break;

		case SOUTH:
			laraitem->pos.zPos &= -WALL_SIZE;
			laraitem->pos.zPos += LARA_RAD;
			break;

		case EAST:
			laraitem->pos.xPos &= -WALL_SIZE;
			laraitem->pos.xPos += WALL_SIZE - LARA_RAD;
			break;

		case WEST:
			laraitem->pos.xPos &= -WALL_SIZE;
			laraitem->pos.xPos += LARA_RAD;
			break;
		}
		laraitem->pos.yRot = item->pos.yRot;
		laraitem->goalAnimState = LS_PUSHABLE_GRAB;
		AnimateLara(laraitem);
		if (laraitem->currentAnimState == LS_PUSHABLE_GRAB)
			Lara.gunStatus = LG_HANDS_BUSY;
	}
	else if (laraitem->currentAnimState == LS_PUSHABLE_GRAB)
	{
		if (laraitem->frameNumber != g_Level.Anims[LA_PUSHABLE_GRAB].frameBase + 19)
			return;

		if (!TestLaraPosition(&MovingBlockBounds, item, laraitem))
			return;

		if (TrInput & IN_FORWARD)
		{
			if (!TestBlockPush3(item, 1024, quadrant))
				return;

			item->goalAnimState = STATE_MOVEBLOCK_PUSH;
			laraitem->goalAnimState = LS_PUSHABLE_PUSH;
		}
		else if (TrInput & IN_BACK)
		{
			if (!TestBlockPull3(item, 1024, quadrant))
				return;

			item->goalAnimState = STATE_MOVEBLOCK_PULL;
			laraitem->goalAnimState = LS_PUSHABLE_PULL;
		}
		else
			return;

		AddActiveItem(itemNum);
		AlterFloorHeight(item, ((item->triggerFlags - 64) * 256));
		AdjustStopperFlag(item, item->itemFlags[0], 1);
		item->status = ITEM_ACTIVE;
		AnimateItem(item);
		AnimateLara(laraitem);
		Lara.headXrot = Lara.headYrot = 0;
		Lara.torsoXrot = Lara.torsoYrot = 0;
	}
}

bool TestBlockMovable3(ITEM_INFO* item, int blokhite)
{
	return true;

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	if (floor->floor == NO_HEIGHT / 256)
		return true;

	if (floor->floor * 256 != item->pos.yPos - blokhite)
		return false;

	return true;
}

bool TestBlockPush3(ITEM_INFO* item, int blokhite, unsigned short quadrant)
{
	if (!TestBlockMovable3(item, blokhite))
		return false;

	int x = item->pos.xPos;
	int y = item->pos.yPos;
	int z = item->pos.zPos;

	short roomNum = item->roomNumber;

	switch (quadrant)
	{
	case NORTH:
		z += WALL_SIZE;
		break;

	case EAST:
		x += WALL_SIZE;
		break;

	case SOUTH:
		z -= WALL_SIZE;
		break;

	case WEST:
		x -= WALL_SIZE;
		break;
	}

	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNum);

	ROOM_INFO* r = &g_Level.Rooms[roomNum];
	if (r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->xSize].stopper)
		return false;

	COLL_INFO scoll;

	scoll.quadrant = quadrant;
	scoll.radius = 500;

	if (CollideStaticObjects(&scoll, x, y, z, roomNum, 1000))
		return false;

	if (((int)floor->floor << 8) != y)
		return false;

	GetFloorHeight(floor, x, y, z);
	if (HeightType != WALL)
		return false;

	int cmax = y - (blokhite - 100);
	floor = GetFloor(x, cmax, z, &roomNum);
	if (GetCeiling(floor, x, cmax, z) > cmax)
		return false;

	item->itemFlags[0] = LaraItem->pos.yRot;

	return true;
}

bool TestBlockPull3(ITEM_INFO* item, int blokhite, unsigned short quadrant)
{
	if (!TestBlockMovable3(item, blokhite))
		return false;

	int xadd = 0;
	int zadd = 0;

	switch (quadrant)
	{
	case NORTH:
		zadd = -WALL_SIZE;
		break;

	case EAST:
		xadd = -WALL_SIZE;
		break;

	case SOUTH:
		zadd = WALL_SIZE;
		break;

	case WEST:
		xadd = WALL_SIZE;
		break;
	}

	int x = item->pos.xPos + xadd;
	int y = item->pos.yPos;
	int z = item->pos.zPos + zadd;

	short roomNum = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNum);

	ROOM_INFO* r = &g_Level.Rooms[roomNum];
	if (r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->xSize].stopper)
		return false;

	COLL_INFO scoll;

	scoll.quadrant = quadrant;
	scoll.radius = 500;
	if (CollideStaticObjects(&scoll, x, y, z, roomNum, 1000))
		return false;

	if (((int)floor->floor << 8) != y)
		return false;

	int cmax = y - blokhite;
	floor = GetFloor(x, cmax, z, &roomNum);
	if (((int)floor->ceiling << 8) > cmax)
		return false;

	x += xadd;
	z += zadd;
	roomNum = item->roomNumber;
	floor = GetFloor(x, y, z, &roomNum);

	if (((int)floor->floor << 8) != y)
		return false;

	cmax = y - LARA_HITE;
	floor = GetFloor(x, cmax, z, &roomNum);
	if (((int)floor->ceiling << 8) > cmax)
		return false;

	x = LaraItem->pos.xPos + xadd;
	y = LaraItem->pos.yPos;
	z = LaraItem->pos.zPos + zadd;

	roomNum = LaraItem->roomNumber;
	floor = GetFloor(x, y, z, &roomNum);

	scoll.quadrant = (quadrant + 2) & 3;
	scoll.radius = LARA_RAD;

	if (CollideStaticObjects(&scoll, x, y, z, roomNum, LARA_HITE))
		return false;

	item->itemFlags[0] = LaraItem->pos.yRot + 0x8000;

	return true;
}