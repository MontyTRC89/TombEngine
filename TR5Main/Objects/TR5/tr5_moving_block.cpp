#include "../newobjects.h"
#include "../oldobjects.h"
#include "../../Global/global.h"
#include "../../Game/items.h"
#include "../../Game/collide.h"
#include "../../Game/effects.h"
#include "../../Game/laramisc.h"
#include "../../Game/Box.h"

short MovingBlockBounds[12] = { 
	-300, 300, 0, 0, -692, -512,
	-10 * ONE_DEGREE, 10 * ONE_DEGREE,
	-30 * ONE_DEGREE, 30 * ONE_DEGREE,
	-10 * ONE_DEGREE, 10 * ONE_DEGREE
};

void InitialisePushableBlock(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearMovableBlockSplitters(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);

	if (item->status != ITEM_INVISIBLE)
		AlterFloorHeight(item, -1024); 
}

void ClearMovableBlockSplitters(int x, int y, int z, short roomNumber)
{
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	Boxes[floor->box].overlapIndex &= (~BLOCKED);	
	short height = Boxes[floor->box].height;
	short baseRoomNumber = roomNumber;

	floor = GetFloor(x + 1024, y, z, &roomNumber);
	if (floor->box != NO_BOX)
	{
		if (Boxes[floor->box].height == height && (Boxes[floor->box].overlapIndex & BLOCKABLE) && (Boxes[floor->box].overlapIndex & BLOCKED))
			ClearMovableBlockSplitters(x + 1024, y, z, roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x - 1024, y, z, &roomNumber);
	if (floor->box != NO_BOX)
	{
		if (Boxes[floor->box].height == height && (Boxes[floor->box].overlapIndex & BLOCKABLE) && (Boxes[floor->box].overlapIndex & BLOCKED))
			ClearMovableBlockSplitters(x - 1024, y, z, roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x, y, z + 1024, &roomNumber);
	if (floor->box != NO_BOX)
	{
		if (Boxes[floor->box].height == height && (Boxes[floor->box].overlapIndex & BLOCKABLE) && (Boxes[floor->box].overlapIndex & BLOCKED))
			ClearMovableBlockSplitters(x, y, z + 1024, roomNumber);
	}

	roomNumber = baseRoomNumber;
	floor = GetFloor(x, y, z - 1024, &roomNumber);
	if (floor->box != NO_BOX)
	{
		if (Boxes[floor->box].height == height && (Boxes[floor->box].overlapIndex & BLOCKABLE) && (Boxes[floor->box].overlapIndex & BLOCKED))
			ClearMovableBlockSplitters(x, y, z - 1024, roomNumber);
	}
}

void PushableBlockControl(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (item->flags & ONESHOT)
	{
		AlterFloorHeight(item, 1024);
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
		SoundEffect(SFX_LARA_THUD, &item->pos, 0);
	}

	if (item->roomNumber != roomNumber)  
		ItemNewRoom(itemNumber, roomNumber);
	if (item->status == ITEM_DEACTIVATED)
	{   
		item->status = ITEM_INACTIVE;
		RemoveActiveItem(itemNumber);   

		AlterFloorHeight(item, -1024);	 
		AdjustStopperFlag(item, item->itemFlags[0] + 0x8000, 0);	

		roomNumber = item->roomNumber;
		floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

		TestTriggers(TriggerIndex, 1, 0);
	}
}

void PushableBlockCollision(short itemNum, ITEM_INFO * laraitem, COLL_INFO * coll)
{
	ITEM_INFO* item = &Items[itemNum];

	if (!(TrInput & IN_ACTION) ||                  			 
		item->status == ITEM_ACTIVE ||    
		laraitem->gravityStatus ||      
		laraitem->pos.yPos != item->pos.yPos)
		return;

	unsigned short quadrant = (unsigned short)(laraitem->pos.yRot + 0x2000) / 0x4000;	 
	if (laraitem->currentAnimState == STATE_LARA_STOP)
	{
		if (Lara.gunStatus != LG_NO_ARMS)  
			return;

		switch (quadrant)
		{
		case NORTH:
			item->pos.yRot = 0;
			break;

		case EAST:
			item->pos.yRot = ANGLE(90);
			break;

		case SOUTH:
			item->pos.yRot = -ANGLE(180);
			break;

		case WEST:
			item->pos.yRot = -ANGLE(90);
			break;
		}
		if (!TestLaraPosition(MovingBlockBounds, item, laraitem))
			return;

		short roomNumber = laraitem->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		if (roomNumber != item->roomNumber)
			return;
		
		switch (quadrant)
		{
		case NORTH:
			laraitem->pos.zPos &= -1024;  
			laraitem->pos.zPos += 1024 - LARA_RAD; 
			break;

		case SOUTH:
			laraitem->pos.zPos &= -1024;
			laraitem->pos.zPos += LARA_RAD;
			break;

		case EAST:
			laraitem->pos.xPos &= -1024;
			laraitem->pos.xPos += 1024 - LARA_RAD;
			break;

		case WEST:
			laraitem->pos.xPos &= -1024;
			laraitem->pos.xPos += LARA_RAD;
			break;
		}
		laraitem->pos.yRot = item->pos.yRot;
		laraitem->goalAnimState = STATE_LARA_PUSHABLE_GRAB;
		AnimateLara(laraitem);
		if (laraitem->currentAnimState == STATE_LARA_PUSHABLE_GRAB)  
			Lara.gunStatus = LG_HANDS_BUSY;
	}
	else if (laraitem->currentAnimState == STATE_LARA_PUSHABLE_GRAB)
	{
		if (laraitem->frameNumber != Anims[ANIMATION_LARA_START_OBJECT_MOVING].frameBase + 19)
			return;
		if (!TestLaraPosition(MovingBlockBounds, item, laraitem))
			return;
		if (TrInput & IN_FORWARD)
		{
			if (!TestBlockPush(item, 1024, quadrant))  
				return;
			item->goalAnimState = 2;
			laraitem->goalAnimState = STATE_LARA_PUSHABLE_PUSH;
		}
		else if (TrInput & IN_BACK)
		{
			if (!TestBlockPull(item, 1024, quadrant)) 
				return;    
			item->goalAnimState = 3;
			laraitem->goalAnimState = STATE_LARA_PUSHABLE_PULL;
		}
		else
			return;

		AddActiveItem(itemNum);
		AlterFloorHeight(item, 1024);	
		AdjustStopperFlag(item, item->itemFlags[0], 1);
		item->status = ITEM_ACTIVE;

		AnimateItem(item); 
		AnimateLara(laraitem); 

		Lara.headXrot = Lara.headYrot = 0;
		Lara.torsoXrot = Lara.torsoYrot = 0;
	}
}

int TestBlockMovable(ITEM_INFO * item, int blokhite)
{
	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	if (floor->floor == NO_HEIGHT / 256)
		return 1;

	if (floor->floor * 256 != item->pos.yPos - blokhite)
		return 0;

	return 1;
}

int TestBlockPush(ITEM_INFO * item, int blokhite, unsigned short quadrant)
{
	if (!TestBlockMovable(item, blokhite))
		return 0;

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
	ROOM_INFO* r = &Rooms[roomNum];
	if (XZ_GET_SECTOR(r,x-r->x,z-r->z).stopper )
		return 0;

	COLL_INFO scoll;
	scoll.quadrant = quadrant;
	scoll.radius = 500;
	if (CollideStaticObjects(&scoll, x, y, z, roomNum, 1000))
		return 0;

	if (((int)floor->floor << 8) != y)
		return 0;

	GetFloorHeight(floor, x, y, z);
	if (HeightType != WALL) 
		return 0;

	int cmax = y - (blokhite - 100);
	floor = GetFloor(x, cmax, z, &roomNum);
	if (GetCeiling(floor, x, cmax, z) > cmax)
		return 0;

	item->itemFlags[0] = LaraItem->pos.yRot;
	 return 1;
}

int TestBlockPull(ITEM_INFO * item, int blokhite, short quadrant)
{
	if (!TestBlockMovable(item, blokhite))
		return (0);

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
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNum);  

	ROOM_INFO* r = &Rooms[roomNum];
	if (XZ_GET_SECTOR(r,x-r->x,z-r->z).stopper)
		return 0;

	COLL_INFO scoll;
	scoll.quadrant = quadrant;
	scoll.radius = 500;
	if (CollideStaticObjects(&scoll, x, y, z, roomNum, 1000))	 
		return 0; 

	if (((int)floor->floor << 8) != y)
		return 0;

	int cmax = y - blokhite;
	floor = GetFloor(x, cmax, z, &roomNum);
	if (((int)floor->ceiling << 8) > cmax)
		return 0;

	x += xadd; 
	z += zadd;
	roomNum = item->roomNumber;
	floor = GetFloor(x, y, z, &roomNum);  

	if (((int)floor->floor << 8) != y)
		return 0;

	cmax = y - LARA_HITE;
	floor = GetFloor(x, cmax, z, &roomNum);
	if (((int)floor->ceiling << 8) > cmax)
		return 0;

	x = LaraItem->pos.xPos + xadd;  
	y = LaraItem->pos.yPos;
	z = LaraItem->pos.zPos + zadd;
	roomNum = LaraItem->roomNumber;
	floor = GetFloor(x, y, z, &roomNum);  
	scoll.quadrant = (quadrant + 2) & 3;
	scoll.radius = LARA_RAD;
	if (CollideStaticObjects(&scoll, x, y, z, roomNum, LARA_HITE))
		return 0;

	item->itemFlags[0] = LaraItem->pos.yRot + ANGLE(180);

	return 1;
}

