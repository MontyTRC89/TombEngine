#include "framework.h"
#include "tr4_sphinx.h"
#include "debris.h"
#include "items.h"
#include "box.h"
#include "effect.h"
#include "setup.h"
#include "level.h"
#include "lara.h"
#include "sound.h"

BITE_INFO sphinxBiteInfo = { 0, 0, 0, 6 };

void InitialiseSphinx(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	ClearItem(itemNumber);

	item->animNumber = Objects[item->animNumber].animIndex + 1;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
}

void SphinxControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	ObjectInfo* obj = &Objects[item->objectNumber];

	int x = item->pos.xPos + 614 * phd_sin(item->pos.yRot) >> W2V_SHIFT;
	int y = item->pos.yPos;
	int z = item->pos.zPos + 614 * phd_cos(item->pos.yRot) >> W2V_SHIFT;

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, y, z);

	if (item->currentAnimState == 5 && floor->stopper)
	{
		ROOM_INFO* room = &Rooms[item->roomNumber];

		for (int i = 0; i < room->numMeshes; i++)
		{
			MESH_INFO* mesh = &room->mesh[i];

			if (mesh->z >> 10 == z >> 10 && mesh->x >> 10 == x >> 10 && mesh->staticNumber >= 50)
			{
				ShatterObject(NULL, mesh, -64, item->roomNumber, 0);
				SoundEffect(SFX_TR4_HIT_ROCK, &item->pos, 0);

				mesh->Flags &= ~0x100;
				floor->stopper = false;

				TestTriggers(TriggerIndex, 1, 0);
			}
		}
	}

	x = item->pos.xPos - 614 * phd_sin(item->pos.yRot) >> W2V_SHIFT;
	y = item->pos.yPos;
	z = item->pos.zPos - 614 * phd_cos(item->pos.yRot) >> W2V_SHIFT;

	roomNumber = item->roomNumber;

	floor = GetFloor(x, y, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, y, z);

	phd_atan(1228, height2 - height1);

	if (item->aiBits)
		GetAITarget(creature);
	else
		creature->enemy = LaraItem;

	AI_INFO info;
	CreatureAIInfo(item, &info);

	if (creature->enemy != LaraItem)
		phd_atan(LaraItem->pos.zPos - item->pos.zPos, LaraItem->pos.xPos - item->pos.xPos);

	GetCreatureMood(item, &info, VIOLENT);
	CreatureMood(item, &info, VIOLENT);

	short angle = CreatureTurn(item, creature->maximumTurn);

	int dx = abs(item->itemFlags[2] - item->pos.xPos);
	int dz = abs(item->itemFlags[3] - item->pos.zPos);

	switch (item->currentAnimState)
	{
	case 1:
		creature->maximumTurn = 0;

		if (info.distance < SQUARE(1024) || item->triggerFlags)
		{
			item->goalAnimState = 3;
		}

		if (GetRandomControl() == 0)
		{
			item->goalAnimState = 2;
		}

		break;

	case 2:
		creature->maximumTurn = 0;

		if (info.distance < SQUARE(1024) || item->triggerFlags)
		{
			item->goalAnimState = 3;
		}

		if (GetRandomControl() == 0)
		{
			item->goalAnimState = 1;
		}

		break;

	case 4:
		creature->maximumTurn = ANGLE(3);
		if (info.distance > SQUARE(1024) && abs(info.angle) <= 512 || item->requiredAnimState == 5)
		{
			item->goalAnimState = 5;
		}
		else if (info.distance < SQUARE(2048) && item->goalAnimState != 5)
		{
			if (height2 <= item->pos.yPos + 256 && height2 >= item->pos.yPos - 256)
			{
				item->goalAnimState = 9;
				item->requiredAnimState = 6;
			}
		}

		break;

	case 5:
		creature->maximumTurn = 60;

		if (creature->flags == 0)
		{
			if (item->touchBits & 0x40)
			{
				CreatureEffect2(
					item,
					&sphinxBiteInfo,
					20,
					-1,
					DoBloodSplat);

				LaraItem->hitPoints -= 200;
				creature->flags = 1;
			}
		}
		if (dx >= 50 || dz >= 50 || item->animNumber != Objects[ID_SPHINX].animIndex)
		{
			if (info.distance > SQUARE(2048) && abs(info.angle) > 512)
			{
				item->goalAnimState = 9;
			}
		}
		else
		{
			item->goalAnimState = 7;
			item->requiredAnimState = 6;
			creature->maximumTurn = 0;
		}

		break;

	case 6:
		creature->maximumTurn = ANGLE(3);
		if (info.distance > SQUARE(2048) || height2 > item->pos.yPos + 256 || height2 < item->pos.yPos - 256)
		{
			item->goalAnimState = 9;
			item->requiredAnimState = 5;
		}

		break;

	case 7:
		//v32 = item->roomNumber;
		//v36 = (signed short)item->currentAnimState - 1;

		roomNumber = item->roomNumber;

		floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

		if (item->frameNumber == Anims[item->animNumber].frameBase)
		{
			TestTriggers(TriggerIndex, 1, 0);

			if (item->touchBits & 0x40)
			{
				CreatureEffect2(
					item,
					&sphinxBiteInfo,
					50,
					-1,
					DoBloodSplat);

				LaraItem->hitPoints = 0;
			}
		}

		break;

	case 9:
		creature->flags = 0;

		if (item->requiredAnimState == 6)
		{
			item->goalAnimState = 6;
		}
		else
		{
			item->goalAnimState = 4;
		}

		break;

	default:
		break;
	}

	item->itemFlags[2] = item->pos.xPos;
	item->itemFlags[3] = item->pos.zPos;

	CreatureAnimation(itemNumber, angle, 0);
}