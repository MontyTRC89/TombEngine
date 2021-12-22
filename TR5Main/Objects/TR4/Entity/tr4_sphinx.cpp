#include "framework.h"
#include "tr4_sphinx.h"
#include "Game/effects/debris.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

enum SPHIX_STATES {
	SPHINX_EMPTY,
	SPHINX_SLEEPING,
	SPHINX_ALERTED,
	SPHINX_WAKING_UP,
	SPHINX_WALK,
	SPHINX_RUN,
	SPHINX_WALK_BACK,
	SPHINX_HIT,
	SPHINX_SHAKING,
	SPHINX_STOP
};

BITE_INFO sphinxBiteInfo = { 0, 0, 0, 6 };

void InitialiseSphinx(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->animNumber = Objects[item->objectNumber].animIndex + 1;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->goalAnimState = SPHINX_SLEEPING;
	item->currentAnimState = SPHINX_SLEEPING;
}

void SphinxControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	int x = item->pos.xPos + 614 * phd_sin(item->pos.yRot);
	int y = item->pos.yPos;
	int z = item->pos.zPos + 614 * phd_cos(item->pos.yRot);

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, y, z);

	if (item->currentAnimState == 5 && floor->Stopper)
	{
		ROOM_INFO* room = &g_Level.Rooms[item->roomNumber];

		for (int i = 0; i < room->mesh.size(); i++)
		{
			MESH_INFO* mesh = &room->mesh[i];

			if (((mesh->pos.zPos / 1024) == (z / 1024)) && 
				((mesh->pos.xPos / 1024) == (x / 1024)) && 
				StaticObjects[mesh->staticNumber].shatterType != SHT_NONE)
			{
				ShatterObject(NULL, mesh, -64, item->roomNumber, 0);
				SoundEffect(SFX_TR4_HIT_ROCK, &item->pos, 0);

				mesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
				floor->Stopper = false;

				TestTriggers(x, y, z, item->roomNumber, true);
			}
		}
	}

	x = item->pos.xPos - 614 * phd_sin(item->pos.yRot);
	y = item->pos.yPos;
	z = item->pos.zPos - 614 * phd_cos(item->pos.yRot);

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

	int dx = abs(item->itemFlags[2] - (short)item->pos.xPos);
	int dz = abs(item->itemFlags[3] - (short)item->pos.zPos);

	switch (item->currentAnimState)
	{
	case SPHINX_SLEEPING:
		creature->maximumTurn = 0;

		if (info.distance < SQUARE(1024) || item->triggerFlags)
		{
			item->goalAnimState = SPHINX_WAKING_UP;
		}

		if (GetRandomControl() == 0)
		{
			item->goalAnimState = SPHINX_ALERTED;
		}

		break;

	case SPHINX_ALERTED:
		creature->maximumTurn = 0;

		if (info.distance < SQUARE(1024) || item->triggerFlags)
		{
			item->goalAnimState = SPHINX_WAKING_UP;
		}

		if (GetRandomControl() == 0)
		{
			item->goalAnimState = SPHINX_SLEEPING;
		}

		break;

	case SPHINX_WALK:
		creature->maximumTurn = ANGLE(3);

		if (info.distance > SQUARE(1024) && abs(info.angle) <= 512 || item->requiredAnimState == SPHINX_RUN)
		{
			item->goalAnimState = SPHINX_RUN;
		}
		else if (info.distance < SQUARE(2048) && item->goalAnimState != SPHINX_RUN)
		{
			if (height2 <= item->pos.yPos + 256 && height2 >= item->pos.yPos - 256)
			{
				item->goalAnimState = SPHINX_STOP;
				item->requiredAnimState = SPHINX_WALK_BACK;
			}
		}

		break;

	case SPHINX_RUN:
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

		if (dx >= 50 || dz >= 50 || item->animNumber != Objects[item->objectNumber].animIndex)
		{
			if (info.distance > SQUARE(2048) && abs(info.angle) > 512)
			{
				item->goalAnimState = SPHINX_STOP;
			}
		}
		else
		{
			item->goalAnimState = SPHINX_HIT;
			item->requiredAnimState = SPHINX_WALK_BACK;
			creature->maximumTurn = 0;
		}

		break;

	case SPHINX_WALK_BACK:
		creature->maximumTurn = ANGLE(3);
		if (info.distance > SQUARE(2048) || height2 > item->pos.yPos + 256 || height2 < item->pos.yPos - 256)
		{
			item->goalAnimState = SPHINX_STOP;
			item->requiredAnimState = SPHINX_RUN;
		}

		break;

	case SPHINX_HIT:
		if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
		{
			TestTriggers(item, true);

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

	case SPHINX_STOP:
		creature->flags = 0;

		if (item->requiredAnimState == SPHINX_WALK_BACK)
		{
			item->goalAnimState = SPHINX_WALK_BACK;
		}
		else
		{
			item->goalAnimState = SPHINX_WALK;
		}

		break;

	default:
		break;
	}

	item->itemFlags[2] = (short)item->pos.xPos;
	item->itemFlags[3] = (short)item->pos.zPos;

	CreatureAnimation(itemNumber, angle, 0);
}