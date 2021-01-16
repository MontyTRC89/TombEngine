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

OBJECT_COLLISION_BOUNDS PushableBlockBounds = {
	0x0000, 0x0000, 0xFF00, 0x0000,
	0x0000, 0x0000, 0xF8E4, 0x071C,
	0xEAAC, 0x1554, 0xF8E4, 0x071C
};

PHD_VECTOR PushableBlockPos = { 0, 0, 0 };
int DoPushPull = 0;

void ClearMovableBlockSplitters(int x, int y, int z, short roomNumber)
{
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	g_Level.Boxes[floor->box].flags &= (~BLOCKED);
	short height = g_Level.Boxes[floor->box].height;
	short baseRoomNumber = roomNumber;

	floor = GetFloor(x + 1024, y, z, &roomNumber);
	if (floor->box != NO_BOX)
	{
		if (g_Level.Boxes[floor->box].height == height && (g_Level.Boxes[floor->box].flags & BLOCKABLE) && (g_Level.Boxes[floor->box].flags & BLOCKED))
			ClearMovableBlockSplitters(x + 1024, y, z, roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x - 1024, y, z, &roomNumber);
	if (floor->box != NO_BOX)
	{
		if (g_Level.Boxes[floor->box].height == height && (g_Level.Boxes[floor->box].flags & BLOCKABLE) && (g_Level.Boxes[floor->box].flags & BLOCKED))
			ClearMovableBlockSplitters(x - 1024, y, z, roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x, y, z + 1024, &roomNumber);
	if (floor->box != NO_BOX)
	{
		if (g_Level.Boxes[floor->box].height == height && (g_Level.Boxes[floor->box].flags & BLOCKABLE) && (g_Level.Boxes[floor->box].flags & BLOCKED))
			ClearMovableBlockSplitters(x, y, z + 1024, roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x, y, z - 1024, &roomNumber);
	if (floor->box != NO_BOX)
	{
		if (g_Level.Boxes[floor->box].height == height && (g_Level.Boxes[floor->box].flags & BLOCKABLE) && (g_Level.Boxes[floor->box].flags & BLOCKED))
			ClearMovableBlockSplitters(x, y, z - 1024, roomNumber);
	}
}

void InitialisePushableBlock(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	ClearMovableBlockSplitters(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
	item->itemFlags[1] = NO_ITEM; // used for linking pushables together in stack
	//if (item->status != ITEM_INVISIBLE && item->triggerFlags >= 64)
	//	AlterFloorHeight(item, -((item->triggerFlags - 64) * 256));
}

void PushableBlockControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	
	PHD_VECTOR pos;
	pos.x = 0;
	pos.y = 0;
	pos.z = 0;

	short quadrant = (unsigned short)(LaraItem->pos.yRot + ANGLE(45)) / ANGLE(90);

	int x, z;
	FLOOR_INFO* floor;
	ROOM_INFO* r;
	int height;
	short roomNumber;

	int stackIndex; // used for stacked pushables
	int blockHeight;

	if (item->triggerFlags > 64)
		blockHeight = CLICK(item->triggerFlags - 64);
	else
		blockHeight = -(GetBoundsAccurate(item)->Y1);

	// get total height of stack
	stackIndex = item->itemFlags[1];
	while (stackIndex != NO_ITEM)
	{
		auto stackItem = &g_Level.Items[stackIndex];
		if (stackItem->triggerFlags > 64)
			blockHeight += CLICK(stackItem->triggerFlags - 64);
		else
			blockHeight += -(GetBoundsAccurate(stackItem)->Y1);

		stackIndex = stackItem->itemFlags[1];
	}


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
		case 0:
			z = pos.z + item->itemFlags[2] - LaraItem->itemFlags[2];
			if (abs(item->pos.zPos - z) < 512 && item->pos.zPos < z)
				item->pos.zPos = z;
			break;

		case 1:
			x = pos.x + item->itemFlags[0] - LaraItem->itemFlags[0];
			if (abs(item->pos.xPos - x) < 512 && item->pos.xPos < x)
				item->pos.xPos = x;
			
			break;

		case 2:
			z = pos.z + item->itemFlags[2] - LaraItem->itemFlags[2];
			if (abs(item->pos.zPos - z) < 512 && item->pos.zPos > z)
				item->pos.zPos = z;
			break;

		case 3:
			x = pos.x + item->itemFlags[0] - LaraItem->itemFlags[0];
			if (abs(item->pos.xPos - x) < 512 && item->pos.xPos > x)
				item->pos.xPos = x;
			break;

		default:
			break;
		}

		stackIndex = item->itemFlags[1];
		while (stackIndex != NO_ITEM) // move pushblock stack together with bottom pushblock
		{
			auto stackItem = &g_Level.Items[stackIndex];
			stackItem->pos.xPos = item->pos.xPos;
			stackItem->pos.zPos = item->pos.zPos;

			stackIndex = stackItem->itemFlags[1];
		}

		if (LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameBase)
		{
			// unlink pushable from stack if linked
			for (int i = 0; i < g_Level.NumItems; i++)
			{
				if (i == itemNumber)
					continue;

				auto belowItem = &g_Level.Items[i];

				int objectNum = belowItem->objectNumber;
				if (objectNum >= ID_PUSHABLE_OBJECT1 && objectNum <= ID_PUSHABLE_OBJECT10)
				{
					if (belowItem->itemFlags[1] == itemNumber)
						belowItem->itemFlags[1] = NO_ITEM;
				}
			}
		}

		if (LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd - 1)
		{
			if (TrInput & IN_ACTION)
			{
				if (!TestBlockPush(item, blockHeight, quadrant))
					LaraItem->goalAnimState = LS_STOP;
				else
				{
					int newRoomNumber = T5M::Floordata::GetRoom(item->roomNumber, item->pos.xPos, item->pos.yPos, item->pos.zPos);
					if (newRoomNumber != item->roomNumber)
						ItemNewRoom(itemNumber, newRoomNumber);

					TestTriggersAtXYZ(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 1, item->flags & 0x3E00);
				}
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
			if (abs(item->pos.zPos - z) < 512 && item->pos.zPos > z)
				item->pos.zPos = z;
			break;

		case EAST:
			x = pos.x + item->itemFlags[0] - LaraItem->itemFlags[0];
			if (abs(item->pos.xPos - x) < 512 && item->pos.xPos > x)
				item->pos.xPos = x;
			break;

		case SOUTH:
			z = pos.z + item->itemFlags[2] - LaraItem->itemFlags[2];
			if (abs(item->pos.zPos - z) < 512 && item->pos.zPos < z)
				item->pos.zPos = z;
			break;

		case WEST:
			x = pos.x + item->itemFlags[0] - LaraItem->itemFlags[0];
			if (abs(item->pos.xPos - x) < 512 && item->pos.xPos < x)
				item->pos.xPos = x;
			break;

		default:
			break;
		}

		stackIndex = item->itemFlags[1];
		while (stackIndex != NO_ITEM) // move pushblock stack together with bottom pushblock
		{
			auto stackItem = &g_Level.Items[stackIndex];
			stackItem->pos.xPos = item->pos.xPos;
			stackItem->pos.zPos = item->pos.zPos;

			stackIndex = stackItem->itemFlags[1];
		}

		if (LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameBase)
		{
			// unlink pushable from stack if linked
			for (int i = 0; i < g_Level.NumItems; i++)
			{
				if (i == itemNumber)
					continue;

				auto belowItem = &g_Level.Items[i];

				int objectNum = belowItem->objectNumber;
				if (objectNum >= ID_PUSHABLE_OBJECT1 && objectNum <= ID_PUSHABLE_OBJECT10)
				{
					if (belowItem->itemFlags[1] == itemNumber)
						belowItem->itemFlags[1] = NO_ITEM;

				}
			}
		}

		if (LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd - 1)
		{
			if (TrInput & IN_ACTION)
			{
				if (!TestBlockPull(item, blockHeight, quadrant))
					LaraItem->goalAnimState = LS_STOP;
				else
				{
					int newRoomNumber = T5M::Floordata::GetRoom(item->roomNumber, item->pos.xPos, item->pos.yPos, item->pos.zPos);
					if (newRoomNumber != item->roomNumber)
						ItemNewRoom(itemNumber, newRoomNumber);

					TestTriggersAtXYZ(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 1, item->flags & 0x3E00);
				}
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

			stackIndex = item->itemFlags[1];
			while (stackIndex != NO_ITEM) // move pushblock stack together with bottom pushblock
			{
				auto stackItem = &g_Level.Items[stackIndex];
				stackItem->pos.xPos = item->pos.xPos;
				stackItem->pos.zPos = item->pos.zPos;

				stackIndex = stackItem->itemFlags[1];
			}
		}

		if (LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd)
		{
			int newRoomNumber = T5M::Floordata::GetRoom(item->roomNumber, item->pos.xPos, item->pos.yPos, item->pos.zPos);
			if (newRoomNumber != item->roomNumber)
				ItemNewRoom(itemNumber, newRoomNumber);

			roomNumber = item->roomNumber;
			floor = GetFloor(item->pos.xPos, item->pos.yPos - 256, item->pos.zPos, &roomNumber);
			GetFloorHeight(floor, item->pos.xPos, item->pos.yPos - 256, item->pos.zPos);
			TestTriggers(TriggerIndex, 1, item->flags & 0x3E00);
			RemoveActiveItem(itemNumber);
			item->status = ITEM_NOT_ACTIVE;

			if (item->triggerFlags >= 64)
			{
				//AlterFloorHeight(item, -((item->triggerFlags - 64) * 256));
				AdjustStopperFlag(item, item->itemFlags[0] + 0x8000, 0);
			}


			int stackTop = NO_ITEM; // index of heighest (yPos) pushable in stack
			int stackYmin = CLICK(256); // set starting height

			//Check for pushable directly below current one in same sector
			for (int i = 0; i < g_Level.NumItems; i++)
			{
				if (i == itemNumber)
					continue;
				
				auto belowItem = &g_Level.Items[i];

				int objectNum = belowItem->objectNumber;
				if (objectNum >= ID_PUSHABLE_OBJECT1 && objectNum <= ID_PUSHABLE_OBJECT10)
				{
					int x = item->pos.xPos;
					int y = item->pos.yPos;
					int z = item->pos.zPos;

					if (belowItem->pos.xPos == x && belowItem->pos.zPos == z)
					{
						int belowY = belowItem->pos.yPos;
						if (belowY > y && belowY < stackYmin)
						{
							// set heighest pushable so far as top of stack
							stackTop = i;
							stackYmin = belowItem->pos.yPos;
						}
					}
				}
			}

			// if top pushable in stack was located, link index of current pushable to below pushable via itemFlags[1]
			if (stackTop != NO_ITEM)
				g_Level.Items[stackTop].itemFlags[1] = itemNumber;
		}
		break;
	}
}

void PushableBlockCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos - 256, item->pos.zPos, &roomNumber);

	int stackIndex; // used for stacked pushables
	int blockHeight;

	if (item->triggerFlags > 64)
		blockHeight = CLICK(item->triggerFlags - 64);
	else
		blockHeight = -(GetBoundsAccurate(item)->Y1);

	// get total height of stack
	stackIndex = item->itemFlags[1];
	while (stackIndex != NO_ITEM)
	{
		auto stackItem = &g_Level.Items[stackIndex];
		if (stackItem->triggerFlags > 64)
			blockHeight += CLICK(stackItem->triggerFlags - 64);
		else
			blockHeight += -(GetBoundsAccurate(stackItem)->Y1);

		stackIndex = stackItem->itemFlags[1];
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
		if ((l->currentAnimState != LS_PUSHABLE_GRAB
			|| (l->frameNumber != g_Level.Anims[LA_PUSHABLE_GRAB].frameBase + 19)
			|| Lara.cornerX != (int)item))
		{
			if (item->triggerFlags < 64)
				ObjectCollision(itemNum, l, coll);
			return;
		}

		short quadrant = (unsigned short)(LaraItem->pos.yRot + ANGLE(45)) / ANGLE(90);

		if (TrInput & IN_FORWARD)
		{
			if (!TestBlockPush(item, blockHeight, quadrant))
				return;
			l->goalAnimState = LS_PUSHABLE_PUSH;
		}
		else if (TrInput & IN_BACK)
		{
			if (!TestBlockPull(item, blockHeight, quadrant))
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

		if (item->triggerFlags >= 64)
		{
			//AlterFloorHeight(item, ((item->triggerFlags - 64) * 256));
			AdjustStopperFlag(item, item->itemFlags[0], 0);
		}
	}
	else
	{
		short roomNumber = l->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos - 256, item->pos.zPos, &roomNumber);
		if (roomNumber == item->roomNumber)
		{
			BOUNDING_BOX* bounds = GetBoundsAccurate(item);

			PushableBlockBounds.boundingBox.X1 = (bounds->X1 / 2) - 100;
			PushableBlockBounds.boundingBox.X2 = (bounds->X2 / 2) + 100;
			PushableBlockBounds.boundingBox.Z1 = bounds->Z1 - 200;
			PushableBlockBounds.boundingBox.Z2 = 0;

			short rot = item->pos.yRot;
			item->pos.yRot = (l->pos.yRot + ANGLE(45)) & 0xC000;

			if (TestLaraPosition(&PushableBlockBounds, item, l))
			{
				unsigned short quadrant = (unsigned short)((item->pos.yRot / 0x4000) + ((rot + 0x2000) / 0x4000));
				if (quadrant & 1)
					PushableBlockPos.z = bounds->X1 - 35;
				else
					PushableBlockPos.z = bounds->Z1 - 35;

				if (item->triggerFlags > 64)
				{					
					// For now don't use auto-align function because it can collide with climb up moves of Lara

					LaraItem->pos.xRot = item->pos.xRot;
					LaraItem->pos.yRot = item->pos.yRot;
					LaraItem->pos.zRot = item->pos.zRot;

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
		}
	}
}

int TestBlockMovable(ITEM_INFO* item, int blokhite)
{
	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	if (floor->floor == NO_HEIGHT / 256)
		return 1;

	if (floor->floor * 256 != item->pos.yPos - blokhite)
		return 0;

	return 1;
}

int TestBlockPush(ITEM_INFO* item, int blockhite, unsigned short quadrant)
{
	int x = item->pos.xPos;
	int y = item->pos.yPos;
	int z = item->pos.zPos;

	short roomNum = item->roomNumber;
	switch (quadrant)
	{
	case NORTH:
		z += 1024;
		break;

	case EAST:
		x += 1024;
		break;

	case SOUTH:
		z -= 1024;
		break;

	case WEST:
		x -= 1024;
		break;
	}

	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNum);
	ROOM_INFO* r = &g_Level.Rooms[roomNum];
	if (XZ_GET_SECTOR(r, x - r->x, z - r->z).stopper)
		return 0;

	if (GetFloorHeight(floor, x, y - 256, z) != y)
		return 0;

	GetFloorHeight(floor, x, y, z);
	if (HeightType)
		return 0;

	int ceiling = y - blockhite + 100;
	floor = GetFloor(x, ceiling, z, &roomNum);
	if (GetCeiling(floor, x, ceiling, z) > ceiling)
		return 0;

	int oldX = item->pos.xPos;
	int oldZ = item->pos.zPos;
	item->pos.xPos = x;
	item->pos.zPos = z;
	GetCollidedObjects(item, 256, 1, &CollidedItems[0], 0, 1);
	item->pos.xPos = oldX;
	item->pos.zPos = oldZ;

	int i = 0;
	while (CollidedItems[i] != NULL)
	{
		if (Objects[CollidedItems[i]->objectNumber].floor == NULL)
			return 0;
		i++;
	}
	return 1;
}

int TestBlockPull(ITEM_INFO* item, int blockhite, short quadrant)
{
	int xadd = 0;
	int zadd = 0;

	switch (quadrant)
	{
	case NORTH:
		zadd = -1024;
		break;

	case EAST:
		xadd = -1024;
		break;

	case SOUTH:
		zadd = 1024;
		break;

	case WEST:
		xadd = 1024;
		break;
	}

	int x = item->pos.xPos + xadd;
	int y = item->pos.yPos;
	int z = item->pos.zPos + zadd;
	short roomNum = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y - 256, z, &roomNum);

	ROOM_INFO* r = &g_Level.Rooms[roomNum];
	if (XZ_GET_SECTOR(r, x - r->x, z - r->z).stopper)
		return 0;

	if (GetFloorHeight(floor, x, y - 256, z) != y)
		return 0;

	if (GetFloor(x, y - blockhite, z, &roomNum)->ceiling * 256 > y - blockhite)
		return 0;

	int oldX = item->pos.xPos;
	int oldZ = item->pos.zPos;
	item->pos.xPos = x;
	item->pos.zPos = z;
	GetCollidedObjects(item, 256, 1, &CollidedItems[0], 0, 1);
	item->pos.xPos = oldX;
	item->pos.zPos = oldZ;

	int i = 0;
	while (CollidedItems[i] != NULL)
	{
		if (Objects[CollidedItems[i]->objectNumber].floor == NULL)
			return 0;
		i++;
	}

	x += xadd;
	z += zadd;
	roomNum = item->roomNumber;
	floor = GetFloor(x, y - 256, z, &roomNum);

	if (GetFloorHeight(floor, x, y - 256, z) != y)
		return 0;

	if (GetFloor(x, y - 762, z, &roomNum)->ceiling * 256 > y - 762)
		return 0;

	x = LaraItem->pos.xPos + xadd;
	z = LaraItem->pos.zPos + zadd;

	roomNum = LaraItem->roomNumber;
	GetFloor(x, y, z, &roomNum);

	r = &g_Level.Rooms[roomNum];
	if (XZ_GET_SECTOR(r, x - r->x, z - r->z).stopper)
		return 0;

	oldX = LaraItem->pos.xPos;
	oldZ = LaraItem->pos.zPos;
	LaraItem->pos.xPos = x;
	LaraItem->pos.zPos = z;
	GetCollidedObjects(LaraItem, 256, 1, &CollidedItems[0], 0, 1);
	LaraItem->pos.xPos = oldX;
	LaraItem->pos.zPos = oldZ;

	i = 0;
	while (CollidedItems[i] != NULL)
	{
		if (Objects[CollidedItems[i]->objectNumber].floor == NULL)
			return 0;
		i++;
	}
	return 1;
}

std::optional<int> PushableBlockFloor(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	if (item.status != ITEM_INVISIBLE && item.triggerFlags >= 64)
	{
		const auto height = item.pos.yPos - (item.triggerFlags - 64) * CLICK(1);
		return std::optional{height};
	}
	return std::nullopt;
}

std::optional<int> PushableBlockCeiling(short itemNumber, int x, int y, int z)
{
	const auto& item = g_Level.Items[itemNumber];
	if (item.status != ITEM_INVISIBLE && item.triggerFlags >= 64)
		return std::optional{item.pos.yPos};
	return std::nullopt;
}
