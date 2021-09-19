#include "framework.h"
#include "generic_trapdoor.h"
#include "lara.h"
#include "input.h"
#include "camera.h"
#include "control/control.h"
#include "level.h"
#include "animation.h"

OBJECT_COLLISION_BOUNDS CeilingTrapDoorBounds = {-256, 256, 0, 900, -768, -256, -1820, 1820, -5460, 5460, -1820, 1820};
static PHD_VECTOR CeilingTrapDoorPos = {0, 1056, -480};
OBJECT_COLLISION_BOUNDS FloorTrapDoorBounds = {-256, 256, 0, 0, -1024, -256, -1820, 1820, -5460, 5460, -1820, 1820};
static PHD_VECTOR FloorTrapDoorPos = {0, 0, -655};

void InitialiseTrapDoor(short itemNumber)
{
	ITEM_INFO* item;

	item = &g_Level.Items[itemNumber];
	TEN::Floordata::AddBridge(itemNumber);
	CloseTrapDoor(itemNumber);
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
	if (TrInput & IN_ACTION && item->status != ITEM_ACTIVE && l->currentAnimState == LS_JUMP_UP && l->gravityStatus && Lara.gunStatus == LG_NO_ARMS && (result || result2))
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
	if (TrInput & IN_ACTION && item->status != ITEM_ACTIVE && l->currentAnimState == LS_STOP && l->animNumber == LA_STAND_IDLE && Lara.gunStatus == LG_NO_ARMS
		|| Lara.isMoving && Lara.interactedItem == itemNumber)
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
				Lara.interactedItem =itemNumber;
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
		OpenTrapDoor(itemNumber);
	}
	else if (!item->currentAnimState && !item->itemFlags[2])
	{
		CloseTrapDoor(itemNumber);
	}
}

void CloseTrapDoor(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	item->itemFlags[2] = 1;
}

void OpenTrapDoor(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	item->itemFlags[2] = 0;
}

int TrapDoorFloorBorder(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	return item->pos.yPos;
}

int TrapDoorCeilingBorder(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	return (item->pos.yPos + 128);
}

std::optional<int> TrapDoorFloor(short itemNumber, int x, int y, int z)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	if (!item->meshBits || item->itemFlags[2] == 0)
		return std::nullopt;

	int height = item->pos.yPos;
	return std::optional{ height };
}

std::optional<int> TrapDoorCeiling(short itemNumber, int x, int y, int z)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!item->meshBits || item->itemFlags[2] == 0)
		return std::nullopt;

	int height = item->pos.yPos + 32;
	return std::optional{ height };
}