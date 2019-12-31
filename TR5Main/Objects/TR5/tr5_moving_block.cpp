#include "../newobjects.h"
#include "../oldobjects.h"
#include "../../Game/lara.h"
#include "../../Game/draw.h"
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

short PushableBlockBounds[12] = {
	0x0000, 0x0000, 0xFF00, 0x0000,
	0x0000, 0x0000, 0xF8E4, 0x071C,
	0xEAAC, 0x1554, 0xF8E4, 0x071C
};

PHD_VECTOR PushableBlockPos = { 0, 0, 0 };

int DoPushPull = 0;

void InitialisePushableBlock(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearMovableBlockSplitters(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);

	//if (item->status != ITEM_INVISIBLE)
	//	AlterFloorHeight(item, -1024); 
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

	switch (LaraItem->animNumber)
	{
	case ANIMATION_LARA_OBJECT_PUSH:
		if ((LaraItem->frameNumber < Anims[LaraItem->animNumber].frameBase + 30
		||   LaraItem->frameNumber > Anims[LaraItem->animNumber].frameBase + 67)
		&&  (LaraItem->frameNumber < Anims[LaraItem->animNumber].frameBase + 78
		||   LaraItem->frameNumber > Anims[LaraItem->animNumber].frameBase + 125)
		&&  (LaraItem->frameNumber < Anims[LaraItem->animNumber].frameBase + 140
		||   LaraItem->frameNumber > Anims[LaraItem->animNumber].frameBase + 160))
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

		GetLaraJointPosition(&pos, LJ_LHAND);

		switch (quadrant)
		{
		case 0:
			z = pos.z + item->itemFlags[2] - LaraItem->itemFlags[2];
			if (abs(item->pos.zPos-z) < 512 && item->pos.zPos < z)
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

		if (LaraItem->frameNumber == Anims[LaraItem->animNumber].frameEnd - 1)
		{
			if (TrInput & IN_ACTION)
			{
				if (!TestBlockPush(item,1024,quadrant))
					LaraItem->goalAnimState = STATE_LARA_STOP;
			}
			else
			{
				LaraItem->goalAnimState = STATE_LARA_STOP;
			}
		}
		break;

	case ANIMATION_LARA_OBJECT_PULL:
		if ((LaraItem->frameNumber <  Anims[LaraItem->animNumber].frameBase + 40
			|| LaraItem->frameNumber >  Anims[LaraItem->animNumber].frameBase + 122)
			&& (LaraItem->frameNumber <  Anims[LaraItem->animNumber].frameBase + 130
				|| LaraItem->frameNumber >  Anims[LaraItem->animNumber].frameBase + 170))
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

		GetLaraJointPosition(&pos, LJ_LHAND);
		
		switch (quadrant)
		{
		case 0:
			z = pos.z + item->itemFlags[2] - LaraItem->itemFlags[2];
			if (abs(item->pos.zPos - z) < 512 && item->pos.zPos > z)
				item->pos.zPos = z;
			break;

		case 1:
			x = pos.x + item->itemFlags[0] - LaraItem->itemFlags[0];
			if (abs(item->pos.xPos - x) < 512 && item->pos.xPos > x)
				item->pos.xPos = x;
			break;

		case 2:
			z = pos.z + item->itemFlags[2] - LaraItem->itemFlags[2];
			if (abs(item->pos.zPos - z) < 512 && item->pos.zPos < z)
				item->pos.zPos = z;
			break;

		case 3:
			x = pos.x + item->itemFlags[0] - LaraItem->itemFlags[0];
			if (abs(item->pos.xPos - x) < 512 && item->pos.xPos < x)
				item->pos.xPos = x;
			break;

		default:
			break;
		}

		if (LaraItem->frameNumber == Anims[LaraItem->animNumber].frameEnd - 1)
		{
			if (TrInput & IN_ACTION)
			{
				if (!TestBlockPull(item, 1024, quadrant))
					LaraItem->goalAnimState = STATE_LARA_STOP;
			}
			else
			{
				LaraItem->goalAnimState = STATE_LARA_STOP;
			}
		}
		break;
	case ANIMATION_LARA_PUSHABLE_PUSH_TO_STAND:
	case ANIMATION_LARA_PUSHABLE_PULL_TO_STAND:
		if (LaraItem->frameNumber == Anims[ANIMATION_LARA_PUSHABLE_PUSH_TO_STAND].frameBase
			|| LaraItem->frameNumber == Anims[ANIMATION_LARA_PUSHABLE_PULL_TO_STAND].frameBase)
		{
			item->pos.xPos = item->pos.xPos & 0xFFFFFE00 | 0x200;
			item->pos.zPos = item->pos.zPos & 0xFFFFFE00 | 0x200;
		}

		if (LaraItem->frameNumber == Anims[LaraItem->animNumber].frameEnd)
		{
			roomNumber = item->roomNumber;
			floor = GetFloor(item->pos.xPos, item->pos.yPos - 256, item->pos.zPos, &roomNumber);
			GetFloorHeight(floor, item->pos.xPos, item->pos.yPos - 256, item->pos.zPos);
			TestTriggers(TriggerIndex, 1, item->flags & 0x3E00);
			RemoveActiveItem(itemNumber);
			item->status = ITEM_INACTIVE;
		}
		break;
	}

	return;

	// TR3 code below, to add in the future
	if (item->flags & ONESHOT)
	{
		AlterFloorHeight(item, 1024);
		KillItem(itemNumber);
		return;
	}

	AnimateItem(item);

	roomNumber = item->roomNumber;
	floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

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

void PushableBlockCollision(short itemNum, ITEM_INFO * l, COLL_INFO * coll)
{
	ITEM_INFO* item = &Items[itemNum];

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos - 256, item->pos.zPos, &roomNumber);
	item->pos.yPos = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos - 256, item->pos.zPos);
	if (item->roomNumber != roomNumber)
		ItemNewRoom(itemNum, roomNumber);

	if ((!(TrInput & IN_ACTION)
		|| l->currentAnimState != STATE_LARA_STOP
		|| l->animNumber != ANIMATION_LARA_STAY_IDLE
		|| l->gravityStatus
		|| Lara.gunStatus
		|| item->status == ITEM_INVISIBLE
		|| item->triggerFlags < 0)
		&& (!Lara.isMoving || Lara.generalPtr != item))
	{
		if (l->currentAnimState != STATE_LARA_PUSHABLE_GRAB 
			|| (l->frameNumber != Anims[ANIMATION_LARA_START_OBJECT_MOVING].frameBase + 19)
			|| Lara.cornerX != (int)item)
		{
			ObjectCollision(itemNum, l, coll);
			return;
		}

		short quadrant = (unsigned short)(LaraItem->pos.yRot + ANGLE(45)) / ANGLE(90);

		if (TrInput & IN_FORWARD)
		{
			if (!TestBlockPush(item, 1024, quadrant))
				return;
			l->goalAnimState = STATE_LARA_PUSHABLE_PUSH;
		}
		else if (TrInput & IN_BACK)
		{
			if (!TestBlockPull(item, 1024, quadrant))
				return;
			l->goalAnimState = STATE_LARA_PUSHABLE_PULL;
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

		GetLaraJointPosition(&pos, LJ_LHAND);

		l->itemFlags[0] = pos.x;
		l->itemFlags[2] = pos.z;

		item->itemFlags[0] = item->pos.xPos;
		item->itemFlags[2] = item->pos.zPos;
	}
	else
	{
		short roomNumber = l->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos - 256, item->pos.zPos, &roomNumber);
		if (roomNumber == item->roomNumber)
		{
			short* bounds = GetBoundsAccurate(item);

			PushableBlockBounds[0] = (bounds[0] >> 1) - 100;
			PushableBlockBounds[1] = (bounds[1] >> 1) + 100;
			PushableBlockBounds[4] = bounds[4] - 200;
			PushableBlockBounds[5] = 0;
			
			short rot = item->pos.yRot;
			item->pos.yRot = (l->pos.yRot + ANGLE(45)) & 0xC000;
			
			if (TestLaraPosition(PushableBlockBounds, item, l))
			{
				if (((item->pos.yRot / 0x4000) + ((rot + 0x2000) / 0x4000)) & 1)
					PushableBlockPos.z = bounds[0] - 35;
				else
					PushableBlockPos.z = bounds[4] - 35;

				if (MoveLaraPosition(&PushableBlockPos, item, l))
				{
					l->animNumber = ANIMATION_LARA_START_OBJECT_MOVING;
					l->frameNumber = Anims[l->animNumber].frameBase;
					l->currentAnimState = STATE_LARA_PUSHABLE_GRAB;
					l->goalAnimState = STATE_LARA_PUSHABLE_GRAB;
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
		}
	}

	return;

	// TR3 code to add in the future
	if (!(TrInput & IN_ACTION) ||                  			 
		item->status == ITEM_ACTIVE ||    
		l->gravityStatus ||      
		l->pos.yPos != item->pos.yPos)
		return;

	unsigned short quadrant = (unsigned short)(l->pos.yRot + 0x2000) / 0x4000;
	if (l->currentAnimState == STATE_LARA_STOP)
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
		if (!TestLaraPosition(MovingBlockBounds, item, l))
			return;

		short roomNumber = l->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		if (roomNumber != item->roomNumber)
			return;
		
		switch (quadrant)
		{
		case NORTH:
			l->pos.zPos &= -1024;  
			l->pos.zPos += 1024 - LARA_RAD; 
			break;

		case SOUTH:
			l->pos.zPos &= -1024;
			l->pos.zPos += LARA_RAD;
			break;

		case EAST:
			l->pos.xPos &= -1024;
			l->pos.xPos += 1024 - LARA_RAD;
			break;

		case WEST:
			l->pos.xPos &= -1024;
			l->pos.xPos += LARA_RAD;
			break;
		}
		l->pos.yRot = item->pos.yRot;
		l->goalAnimState = STATE_LARA_PUSHABLE_GRAB;
		AnimateLara(l);
		if (l->currentAnimState == STATE_LARA_PUSHABLE_GRAB)  
			Lara.gunStatus = LG_HANDS_BUSY;
	}
	else if (l->currentAnimState == STATE_LARA_PUSHABLE_GRAB)
	{
		if (l->frameNumber != Anims[ANIMATION_LARA_START_OBJECT_MOVING].frameBase + 19)
			return;
		if (!TestLaraPosition(MovingBlockBounds, item, l))
			return;
		if (TrInput & IN_FORWARD)
		{
			if (!TestBlockPush(item, 1024, quadrant))  
				return;
			item->goalAnimState = 2;
			l->goalAnimState = STATE_LARA_PUSHABLE_PUSH;
		}
		else if (TrInput & IN_BACK)
		{
			if (!TestBlockPull(item, 1024, quadrant)) 
				return;    
			item->goalAnimState = 3;
			l->goalAnimState = STATE_LARA_PUSHABLE_PULL;
		}
		else
			return;

		AddActiveItem(itemNum);
		AlterFloorHeight(item, 1024);	
		AdjustStopperFlag(item, item->itemFlags[0], 1);
		item->status = ITEM_ACTIVE;

		AnimateItem(item); 
		AnimateLara(l); 

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

int TestBlockPush(ITEM_INFO * item, int blockhite, unsigned short quadrant)
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
	GetCollidedObjects(item, 256, 1, &CollidedItems[0], 0, 0);
	item->pos.xPos = oldX;
	item->pos.zPos = oldZ;

	return CollidedItems[0] == NULL;
	
	// TR3 code
	/*COLL_INFO scoll;
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
	 return 1;*/
}

int TestBlockPull(ITEM_INFO * item, int blockhite, short quadrant)
{
	//if (!TestBlockMovable(item, blokhite))
	//	return (0);

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

	ROOM_INFO* r = &Rooms[roomNum];
	if (XZ_GET_SECTOR(r, x - r->x, z - r->z).stopper)
		return 0;

	if (GetFloorHeight(floor, x, y - 256, z) != y)
		return 0;

	if (GetFloor(x, y - blockhite, z, &quadrant)->ceiling * 256 > y - blockhite)
		return 0;

	int oldX = item->pos.xPos;
	int oldZ = item->pos.zPos;
	item->pos.xPos = x;
	item->pos.zPos = z;
	GetCollidedObjects(item, 256, 1, &CollidedItems[0], 0, 0);
	item->pos.xPos = oldX;
	item->pos.zPos = oldZ;

	if (CollidedItems[0] != NULL)
		return 0;

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

	r = &Rooms[roomNum];
	if (XZ_GET_SECTOR(r, x - r->x, z - r->z).stopper)
		return 0;

	oldX = LaraItem->pos.xPos;
	oldZ = LaraItem->pos.zPos;
	LaraItem->pos.xPos = x;
	LaraItem->pos.zPos = z;
	GetCollidedObjects(LaraItem, 256, 1, &CollidedItems[0], 0, 0);
	LaraItem->pos.xPos = oldX;
	LaraItem->pos.zPos = oldZ;

	return (CollidedItems[0] == NULL);

	/*
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

	return 1;*/
}

