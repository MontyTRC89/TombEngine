#include "framework.h"
#include "tr5_pushableblock.h"
#include "lara.h"
#include "draw.h"

#include "items.h"
#include "collide.h"
#include "effect.h"
#include "box.h"
#include "level.h"
#include "input.h"
#include "sound.h"

OBJECT_COLLISION_BOUNDS MovingBlockBounds = {
	-300, 300, 0, 0, -692, -512,
	-ANGLE(10.0f), ANGLE(10.0f),
	-ANGLE(30.0f), ANGLE(30.0f),
	-ANGLE(10.0f), ANGLE(10.0f)
};

OBJECT_COLLISION_BOUNDS PushableBlockBounds = {
	0x0000, 0x0000, 0xFF00, 0x0000,
	0x0000, 0x0000, 0xF8E4, 0x071C,
	0xEAAC, 0x1554, 0xF8E4, 0x071C
};

PHD_VECTOR PushableBlockPos = { 0, 0, 0 };
bool DoPushPull = 0;

void ClearMovableBlockSplitters(int x, int y, int z, short roomNumber)
{
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	g_Level.Boxes[floor->box].flags &= (~BLOCKED);
	short height = g_Level.Boxes[floor->box].height;
	short baseRoomNumber = roomNumber;

	floor = GetFloor(x + SECTOR(1), y, z, &roomNumber);
	if (floor->box != NO_BOX)
	{
		if (g_Level.Boxes[floor->box].height == height && (g_Level.Boxes[floor->box].flags & BLOCKABLE) && (g_Level.Boxes[floor->box].flags & BLOCKED))
			ClearMovableBlockSplitters(x + SECTOR(1), y, z, roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x - SECTOR(1), y, z, &roomNumber);
	if (floor->box != NO_BOX)
	{
		if (g_Level.Boxes[floor->box].height == height && (g_Level.Boxes[floor->box].flags & BLOCKABLE) && (g_Level.Boxes[floor->box].flags & BLOCKED))
			ClearMovableBlockSplitters(x - SECTOR(1), y, z, roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x, y, z + SECTOR(1), &roomNumber);
	if (floor->box != NO_BOX)
	{
		if (g_Level.Boxes[floor->box].height == height && (g_Level.Boxes[floor->box].flags & BLOCKABLE) && (g_Level.Boxes[floor->box].flags & BLOCKED))
			ClearMovableBlockSplitters(x, y, z + SECTOR(1), roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x, y, z - SECTOR(1), &roomNumber);
	if (floor->box != NO_BOX)
	{
		if (g_Level.Boxes[floor->box].height == height && (g_Level.Boxes[floor->box].flags & BLOCKABLE) && (g_Level.Boxes[floor->box].flags & BLOCKED))
			ClearMovableBlockSplitters(x, y, z - SECTOR(1), roomNumber);
	}
}

void InitialisePushableBlock(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	ClearMovableBlockSplitters(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);

	if (item->triggerFlags > 64 && item->status != ITEM_INVISIBLE)
		AlterFloorHeight(item, -(item->triggerFlags - 64) * CLICK(1)); 
}

void PushableBlockControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	PHD_VECTOR pos;
	pos.x = 0;
	pos.y = 0;
	pos.z = 0;

	short quadrant = GetQuadrant(LaraItem->pos.yRot);

	int x, z;
	FLOOR_INFO* floor;
	ROOM_INFO* r;
	int height;
	short roomNumber;

	switch (LaraItem->animNumber)
	{
	case LA_PUSHABLE_PUSH:
		if ((LaraItem->frameNumber < g_Level.Anims[LaraItem->animNumber].frameBase + 30
			|| LaraItem->frameNumber > g_Level.Anims[LaraItem->animNumber].frameBase + 67)
			&& (LaraItem->frameNumber < g_Level.Anims[LaraItem->animNumber].frameBase + 78
				|| LaraItem->frameNumber > g_Level.Anims[LaraItem->animNumber].frameBase + 125)
			&& (LaraItem->frameNumber < g_Level.Anims[LaraItem->animNumber].frameBase + 140
				|| LaraItem->frameNumber > g_Level.Anims[LaraItem->animNumber].frameBase + 160))
		{
			if (DoPushPull)
			{
				SoundEffect(SFX_PUSH_BLOCK_END, &item->pos, 2);
				DoPushPull = 0;
			}
		}
		else
		{
			SoundEffect(SFX_PUSHABLE_SOUND, &item->pos, 2);
			DoPushPull = 1;
		}

		GetLaraJointPosition(&pos, LM_LHAND);

		switch (quadrant)
		{
		case NORTH:
			z = pos.z + item->itemFlags[2] - LaraItem->itemFlags[2];
			if (abs(item->pos.zPos - z) < SECTOR(1) / 2 && item->pos.zPos < z)
				item->pos.zPos = z;
			break;

		case EAST:
			x = pos.x + item->itemFlags[0] - LaraItem->itemFlags[0];
			if (abs(item->pos.xPos - x) < SECTOR(1) / 2 && item->pos.xPos < x)
				item->pos.xPos = x;
			break;

		case SOUTH:
			z = pos.z + item->itemFlags[2] - LaraItem->itemFlags[2];
			if (abs(item->pos.zPos - z) < SECTOR(1) / 2 && item->pos.zPos > z)
				item->pos.zPos = z;
			break;

		case WEST:
			x = pos.x + item->itemFlags[0] - LaraItem->itemFlags[0];
			if (abs(item->pos.xPos - x) < SECTOR(1) / 2 && item->pos.xPos > x)
				item->pos.xPos = x;
			break;

		default:
			break;
		}

		if (LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd - 1)
		{
			if (TrInput & IN_ACTION)
			{
				if (!TestBlockPush(item, CLICK(4), quadrant))
					LaraItem->goalAnimState = LS_STOP;
			}
			else
			{
				LaraItem->goalAnimState = LS_STOP;
			}
		}
		break;

	case LA_PUSHABLE_PULL:
		if ((LaraItem->frameNumber <  g_Level.Anims[LaraItem->animNumber].frameBase + 40
			|| LaraItem->frameNumber >  g_Level.Anims[LaraItem->animNumber].frameBase + 122)
			&& (LaraItem->frameNumber <  g_Level.Anims[LaraItem->animNumber].frameBase + 130
				|| LaraItem->frameNumber >  g_Level.Anims[LaraItem->animNumber].frameBase + 170))
		{
			if (DoPushPull)
			{
				SoundEffect(SFX_PUSH_BLOCK_END, &item->pos, 2);
				DoPushPull = false;
			}
		}
		else
		{
			SoundEffect(SFX_PUSHABLE_SOUND, &item->pos, 2);
			DoPushPull = true;
		}

		GetLaraJointPosition(&pos, LM_LHAND);

		switch (quadrant)
		{
		case NORTH:
			z = pos.z + item->itemFlags[2] - LaraItem->itemFlags[2];
			if (abs(item->pos.zPos - z) < SECTOR(1) / 2 && item->pos.zPos > z)
				item->pos.zPos = z;
			break;

		case EAST:
			x = pos.x + item->itemFlags[0] - LaraItem->itemFlags[0];
			if (abs(item->pos.xPos - x) < SECTOR(1) / 2 && item->pos.xPos > x)
				item->pos.xPos = x;
			break;

		case SOUTH:
			z = pos.z + item->itemFlags[2] - LaraItem->itemFlags[2];
			if (abs(item->pos.zPos - z) < SECTOR(1) / 2 && item->pos.zPos < z)
				item->pos.zPos = z;
			break;

		case WEST:
			x = pos.x + item->itemFlags[0] - LaraItem->itemFlags[0];
			if (abs(item->pos.xPos - x) < SECTOR(1) / 2 && item->pos.xPos < x)
				item->pos.xPos = x;
			break;

		default:
			break;
		}

		if (LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd - 1)
		{
			if (TrInput & IN_ACTION)
			{
				if (!TestBlockPull(item, CLICK(4), quadrant))
					LaraItem->goalAnimState = LS_STOP;
			}
			else
			{
				LaraItem->goalAnimState = LS_STOP;
			}
		}
		break;
	case LA_PUSHABLE_PUSH_TO_STAND:
	case LA_PUSHABLE_PULL_TO_STAND:
		if (LaraItem->frameNumber == g_Level.Anims[LA_PUSHABLE_PUSH_TO_STAND].frameBase
			|| LaraItem->frameNumber == g_Level.Anims[LA_PUSHABLE_PULL_TO_STAND].frameBase)
		{
			item->pos.xPos = item->pos.xPos & 0xFFFFFE00 | 0x200;
			item->pos.zPos = item->pos.zPos & 0xFFFFFE00 | 0x200;
		}

		if (LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd)
		{
			if (item->triggerFlags > 64)
			{
				AlterFloorHeight(item, -(item->triggerFlags - 64) * CLICK(1));
				AdjustStopperFlag(item, item->itemFlags[7] + 0x8000, 0);
			}

			roomNumber = item->roomNumber;
			floor = GetFloor(item->pos.xPos, item->pos.yPos - CLICK(1), item->pos.zPos, &roomNumber);
			GetFloorHeight(floor, item->pos.xPos, item->pos.yPos - CLICK(1), item->pos.zPos);
			TestTriggers(TriggerIndex, 1, item->flags & 0x3E00);
			RemoveActiveItem(itemNumber);
			item->status = ITEM_NOT_ACTIVE;
		}

		/*if (item->status == ITEM_DEACTIVATED)
		{
			item->status = ITEM_NOT_ACTIVE;
			RemoveActiveItem(itemNumber);

			if (item->triggerFlags > 64)
			{
				AlterFloorHeight(item, -1024);
				AdjustStopperFlag(item, item->itemFlags[7] + 0x8000, 0);
			}

			roomNumber = item->roomNumber;
			floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
			GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
			TestTriggers(TriggerIndex, 1, 0);
		}*/

		break;
	}
}

void PushableBlockCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos - CLICK(1), item->pos.zPos, &roomNumber);
	if (item->triggerFlags < 64)
	{
		item->pos.yPos = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos - CLICK(1), item->pos.zPos);
		if (item->roomNumber != roomNumber)
			ItemNewRoom(itemNum, roomNumber);
	}

	if ((!(TrInput & IN_ACTION)
		|| l->currentAnimState != LS_STOP
		|| l->animNumber != LA_STAND_IDLE
		|| l->gravityStatus
		|| Lara.gunStatus
		|| item->status == ITEM_INVISIBLE
		|| item->triggerFlags < 0)
		&& (!Lara.isMoving || Lara.generalPtr != item))
	{
		if (item->triggerFlags < 64 &&
			(l->currentAnimState != LS_PUSHABLE_GRAB
			|| (l->frameNumber != g_Level.Anims[LA_PUSHABLE_GRAB].frameBase + 19)
			|| Lara.cornerX != (int)item))
		{
			ObjectCollision(itemNum, l, coll);
			return;
		}

		short quadrant = GetQuadrant(LaraItem->pos.yRot);

		if (TrInput & IN_FORWARD)
		{
			if (!TestBlockPush(item, CLICK(4), quadrant))
				return;
			l->goalAnimState = LS_PUSHABLE_PUSH;
		}
		else if (TrInput & IN_BACK)
		{
			if (!TestBlockPull(item, CLICK(4), quadrant))
				return;
			l->goalAnimState = LS_PUSHABLE_PULL;
		}
		else
		{
			return;
		}

		item->status = ITEM_ACTIVE;
		AddActiveItem(itemNum);
		Lara.headYrot = 0;
		Lara.headXrot = 0;
		Lara.torsoYrot = 0;
		Lara.torsoXrot = 0;

		PHD_VECTOR pos;
		pos.x = 0;
		pos.y = 0;
		pos.z = 0;

		GetLaraJointPosition(&pos, LM_LHAND);

		l->itemFlags[0] = pos.x;
		l->itemFlags[2] = pos.z;

		item->itemFlags[0] = item->pos.xPos;
		item->itemFlags[2] = item->pos.zPos;

		if (item->triggerFlags > 64 
			&& (TrInput & IN_ACTION)
			&& ((l->animNumber == LA_PUSHABLE_PULL || l->animNumber == LA_PUSHABLE_PUSH) 
				&& l->frameNumber == g_Level.Anims[l->animNumber].frameBase))
		{
			AlterFloorHeight(item, 1024);
			AdjustStopperFlag(item, item->itemFlags[7], 1);
		}
	}
	else
	{
		short roomNumber = l->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos - CLICK(1), item->pos.zPos, &roomNumber);
		if (roomNumber == item->roomNumber)
		{
			//if (item->triggerFlags < 64)
			//{
				BOUNDING_BOX* bounds = GetBoundsAccurate(item);

				PushableBlockBounds.boundingBox.X1 = (bounds->X1 / 2) - 100;
				PushableBlockBounds.boundingBox.X2 = (bounds->X2 / 2) + 100;
				PushableBlockBounds.boundingBox.Z1 = bounds->Z1 - 200;
				PushableBlockBounds.boundingBox.Z2 = 0;

				short rot = item->pos.yRot;
				item->pos.yRot = (l->pos.yRot + ANGLE(45)) & 0xC000;

				if (TestLaraPosition(&PushableBlockBounds, item, l))
				{
					if (((item->pos.yRot / 0x4000) + ((rot + 0x2000) / 0x4000)) & 1)
						PushableBlockPos.z = bounds->X1 - 35;
					else
						PushableBlockPos.z = bounds->Z1 - 35;

					if (MoveLaraPosition(&PushableBlockPos, item, l))
					{
						l->animNumber = LA_PUSHABLE_GRAB;
						l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
						l->currentAnimState = LS_PUSHABLE_GRAB;
						l->goalAnimState = LS_PUSHABLE_GRAB;
						Lara.isMoving = false;
						Lara.gunStatus = LG_HANDS_BUSY;
						Lara.cornerX = (int)item;
						item->pos.yRot = rot;
					}
					else
					{
						Lara.generalPtr = item;
						item->pos.yRot = rot;
					}
				}
				else
				{
					if (Lara.isMoving && Lara.generalPtr == item)
					{
						Lara.isMoving = false;
						Lara.gunStatus = LG_NO_ARMS;
					}
					item->pos.yRot = rot;
				}
			/*}
			else
			{
				short rot = item->pos.yRot;
				item->pos.yRot = (l->pos.yRot + ANGLE(45)) & 0xC000;

				if (TestLaraPosition(&MovingBlockBounds, item, l))
				{
					if (MoveLaraPosition(&PushableBlockPos, item, l))
					{
						l->animNumber = LA_PUSHABLE_GRAB;
						l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
						l->currentAnimState = LS_PUSHABLE_GRAB;
						l->goalAnimState = LS_PUSHABLE_GRAB;
						Lara.isMoving = false;
						Lara.gunStatus = LG_HANDS_BUSY;
						Lara.cornerX = (int)item;
						item->pos.yRot = rot;
					}
					else
					{
						Lara.generalPtr = item;
						item->pos.yRot = rot;
					}
				}
				else
				{
					if (Lara.isMoving && Lara.generalPtr == item)
					{
						Lara.isMoving = false;
						Lara.gunStatus = LG_NO_ARMS;
					}
					item->pos.yRot = rot;
				}
			}*/
		}
	}
}

bool TestBlockMovable(ITEM_INFO* item, int blockHeight)
{
	short roomNum = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNum);
	if (floor->floor == NO_HEIGHT / CLICK(1))
		return true;
	else if (floor->floor * CLICK(1) != item->pos.yPos - blockHeight)
		return false;

	return true;
}

bool TestBlockPush(ITEM_INFO* item, int blockHeight, unsigned short quadrant)
{
	//if (!TestBlockMovable(item, blokhite))
	//	return 0;

	int x = item->pos.xPos;
	int y = item->pos.yPos;
	int z = item->pos.zPos;
	
	short roomNum = item->roomNumber;
	switch (quadrant)
	{
	case NORTH:
		z += SECTOR(1);
		break;

	case EAST:
		x += SECTOR(1);
		break;

	case SOUTH:
		z -= SECTOR(1);
		break;

	case WEST:
		x -= SECTOR(1);
		break;
	}

	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNum);  
	ROOM_INFO* r = &g_Level.Rooms[roomNum];
	if (XZ_GET_SECTOR(r, x - r->x, z - r->z).stopper)
		return false;

	if (GetFloorHeight(floor, x, y - CLICK(1), z) != y)
		return false;

	GetFloorHeight(floor, x, y, z);
	if (HeightType)
		return false;

	int ceiling = y - blockHeight + 100;
	floor = GetFloor(x, ceiling, z, &roomNum);
	if (GetCeiling(floor, x, ceiling, z) > ceiling)
		return false;

	int oldX = item->pos.xPos;
	int oldZ = item->pos.zPos;
	item->pos.xPos = x;
	item->pos.zPos = z;
	GetCollidedObjects(item, 256, 1, &CollidedItems[0], 0, 0);
	item->pos.xPos = oldX;
	item->pos.zPos = oldZ;

	item->itemFlags[7] = LaraItem->pos.yRot;

	return CollidedItems[0] == NULL;
}

bool TestBlockPull(ITEM_INFO * item, int blockHeight, short quadrant)
{
	//if (!TestBlockMovable(item, blokhite))
	//	return (0);

	int xadd = 0;
	int zadd = 0;

	switch (quadrant)
	{
	case NORTH:
		zadd = -SECTOR(1);
		break;

	case EAST:
		xadd = -SECTOR(1);
		break;

	case SOUTH:
		zadd = SECTOR(1);
		break;

	case WEST:
		xadd = SECTOR(1);
		break;
	}

	int x = item->pos.xPos + xadd;
	int y = item->pos.yPos;
	int z = item->pos.zPos + zadd;
	short roomNum = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y - CLICK(1), z, &roomNum);  

	ROOM_INFO* r = &g_Level.Rooms[roomNum];
	if (XZ_GET_SECTOR(r, x - r->x, z - r->z).stopper)
		return false;

	if (GetFloorHeight(floor, x, y - CLICK(1), z) != y)
		return false;

	if (GetFloor(x, y - blockHeight, z, &quadrant)->ceiling * CLICK(1) > y - blockHeight)
		return false;

	int oldX = item->pos.xPos;
	int oldZ = item->pos.zPos;
	item->pos.xPos = x;
	item->pos.zPos = z;
	GetCollidedObjects(item, 256, 1, &CollidedItems[0], 0, 0);
	item->pos.xPos = oldX;
	item->pos.zPos = oldZ;

	if (CollidedItems[0] != NULL)
		return false;

	x += xadd;
	z += zadd;
	roomNum = item->roomNumber;
	floor = GetFloor(x, y - CLICK(1), z, &roomNum);

	if (GetFloorHeight(floor, x, y - CLICK(1), z) != y)
		return false;

	if (GetFloor(x, y - 762, z, &roomNum)->ceiling * CLICK(1) > y - 762)
		return false;

	x = LaraItem->pos.xPos + xadd;
	z = LaraItem->pos.zPos + zadd;

	roomNum = LaraItem->roomNumber;
	GetFloor(x, y, z, &roomNum);

	r = &g_Level.Rooms[roomNum];
	if (XZ_GET_SECTOR(r, x - r->x, z - r->z).stopper)
		return false;

	oldX = LaraItem->pos.xPos;
	oldZ = LaraItem->pos.zPos;
	LaraItem->pos.xPos = x;
	LaraItem->pos.zPos = z;
	GetCollidedObjects(LaraItem, 256, 1, &CollidedItems[0], 0, 0);
	LaraItem->pos.xPos = oldX;
	LaraItem->pos.zPos = oldZ;

	return (CollidedItems[0] == NULL);
}
