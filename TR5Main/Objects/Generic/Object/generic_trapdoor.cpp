#include "framework.h"
#include "generic_trapdoor.h"
#include "lara.h"
#include "input.h"

OBJECT_COLLISION_BOUNDS CeilingTrapDoorBounds = {-256, 256, 0, 900, -768, -256, -1820, 1820, -5460, 5460, -1820, 1820};
static PHD_VECTOR CeilingTrapDoorPos = {0, 1056, -480};
OBJECT_COLLISION_BOUNDS FloorTrapDoorBounds = {-256, 256, 0, 0, -1024, -256, -1820, 1820, -5460, 5460, -1820, 1820};
static PHD_VECTOR FloorTrapDoorPos = {0, 0, -655};

void InitialiseTrapDoor(short itemNumber)
{
	ITEM_INFO* item;

	item = &g_Level.Items[itemNumber];
	CloseTrapDoor(item);
}

void TrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item;

	item = &g_Level.Items[itemNumber];
	if (item->currentAnimState == 1 && item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
		ObjectCollision(itemNumber, l, coll);
}

void CeilingTrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item;
	int result, result2;

	item = &g_Level.Items[itemNumber];
	result = TestLaraPosition(&CeilingTrapDoorBounds, item, l);
	l->pos.yRot += ANGLE(180);
	result2 = TestLaraPosition(&CeilingTrapDoorBounds, item, l);
	l->pos.yRot += ANGLE(180);
	if (TrInput & IN_ACTION && item->status != ITEM_DEACTIVATED && l->currentAnimState == LS_JUMP_UP && l->gravityStatus && Lara.gunStatus == LG_NO_ARMS && (result || result2))
	{
		AlignLaraPosition(&CeilingTrapDoorPos, item, l);
		if (result2)
			l->pos.yRot += ANGLE(180);
		Lara.headYrot = 0;
		Lara.headXrot = 0;
		Lara.torsoYrot = 0;
		Lara.torsoXrot = 0;
		Lara.gunStatus = LG_HANDS_BUSY;
		l->gravityStatus = false;
		l->fallspeed = 0;
		l->animNumber = LA_TRAPDOOR_CEILING_OPEN;
		l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
		l->currentAnimState = LS_FREEFALL_BIS;
		AddActiveItem(itemNumber);
		item->status = ITEM_ACTIVE;
		item->goalAnimState = 1;

		UseForcedFixedCamera = 1;
		ForcedFixedCamera.x = item->pos.xPos - phd_sin(item->pos.yRot) * 1024;
		ForcedFixedCamera.y = item->pos.yPos + 1024;
		ForcedFixedCamera.z = item->pos.zPos - phd_cos(item->pos.yRot) * 1024;
		ForcedFixedCamera.roomNumber = item->roomNumber;
	}
	else
	{
		if (item->currentAnimState == 1)
			UseForcedFixedCamera = 0;
	}

	if (item->currentAnimState == 1 && item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
		ObjectCollision(itemNumber, l, coll);
}

void FloorTrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item;

	item = &g_Level.Items[itemNumber];
	if (TrInput & IN_ACTION && item->status != ITEM_DEACTIVATED && l->currentAnimState == LS_STOP && l->animNumber == LA_STAND_IDLE && Lara.gunStatus == LG_NO_ARMS
		|| Lara.isMoving && Lara.generalPtr == (void *)itemNumber)
	{
		if (TestLaraPosition(&FloorTrapDoorBounds, item, l))
		{
			if (MoveLaraPosition(&FloorTrapDoorPos, item, l))
			{
				l->animNumber = LA_TRAPDOOR_FLOOR_OPEN;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				l->currentAnimState = LS_TRAPDOOR_FLOOR_OPEN;
				Lara.isMoving = false;
				Lara.headYrot = 0;
				Lara.headXrot = 0;
				Lara.torsoYrot = 0;
				Lara.torsoXrot = 0;
				Lara.gunStatus = LG_HANDS_BUSY;
				AddActiveItem(itemNumber);
				item->status = ITEM_ACTIVE;
				item->goalAnimState = 1;

				UseForcedFixedCamera = 1;
				ForcedFixedCamera.x = item->pos.xPos - phd_sin(item->pos.yRot) * 2048;
				ForcedFixedCamera.y = item->pos.yPos - 2048;
				if (ForcedFixedCamera.y < g_Level.Rooms[item->roomNumber].maxceiling)
					ForcedFixedCamera.y = g_Level.Rooms[item->roomNumber].maxceiling;
				ForcedFixedCamera.z = item->pos.zPos - phd_cos(item->pos.yRot) * 2048;
				ForcedFixedCamera.roomNumber = item->roomNumber;
			}
			else
			{
				Lara.generalPtr = (void *)itemNumber;
			}
		}
	}
	else
	{
		if (item->currentAnimState == 1)
			UseForcedFixedCamera = 0;
	}

	if (item->currentAnimState == 1 && item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
		ObjectCollision(itemNumber, l, coll);
}

void TrapDoorControl(short itemNumber)
{
	ITEM_INFO* item;

	item = &g_Level.Items[itemNumber];
	if (TriggerActive(item))
	{
		if (!item->currentAnimState && item->triggerFlags >= 0)
		{
			item->goalAnimState = 1;
		}
		else if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd && CurrentLevel == 14 && item->objectNumber == ID_TRAPDOOR1)
		{
			item->status = ITEM_INVISIBLE;
		}
	}
	else
	{
		item->status = ITEM_ACTIVE;

		if (item->currentAnimState == 1)
		{
			item->goalAnimState = 0;
		}
	}

	AnimateItem(item);

	if (item->currentAnimState == 1 && (item->itemFlags[2] || JustLoaded))
	{
		OpenTrapDoor(item);
	}
	else if (!item->currentAnimState && !item->itemFlags[2])
	{
		CloseTrapDoor(item);
	}
}

void CloseTrapDoor(ITEM_INFO* item)
{
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	unsigned short pitsky;

	r = &g_Level.Rooms[item->roomNumber];
	floor = &XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z);
	pitsky = 0;

	if (item->pos.yPos == r->minfloor)
	{
		pitsky = floor->pitRoom;
		floor->pitRoom = NO_ROOM;
		r = &g_Level.Rooms[pitsky];
		floor = &XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z);
		pitsky |= floor->skyRoom * 256;
		floor->skyRoom = NO_ROOM;
	}
	else if (item->pos.yPos == r->maxceiling)
	{
		pitsky = floor->skyRoom;
		floor->skyRoom = NO_ROOM;
		r = &g_Level.Rooms[pitsky];
		floor = &XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z);
		pitsky = ((pitsky * 256) | floor->pitRoom);
		floor->pitRoom = NO_ROOM;
	}

	item->itemFlags[2] = 1;
	item->itemFlags[3] = pitsky;
}

void OpenTrapDoor(ITEM_INFO* item)
{
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	unsigned short pitsky;

	r = &g_Level.Rooms[item->roomNumber];
	floor = &XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z);
	pitsky = item->itemFlags[3];

	if (item->pos.yPos == r->minfloor)
	{
		floor->pitRoom = (unsigned char)pitsky;
		r = &g_Level.Rooms[floor->pitRoom];
		floor = &XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z);
		floor->skyRoom = pitsky / 256;
	}
	else
	{
		floor->skyRoom = pitsky / 256;
		r = &g_Level.Rooms[floor->skyRoom];
		floor = &XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z);
		floor->pitRoom = (unsigned char)pitsky;
	}

	item->itemFlags[2] = 0;
}
