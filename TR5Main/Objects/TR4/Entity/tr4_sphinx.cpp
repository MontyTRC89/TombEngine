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

	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->TargetState = SPHINX_SLEEPING;
	item->ActiveState = SPHINX_SLEEPING;
}

void SphinxControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
	OBJECT_INFO* obj = &Objects[item->ObjectNumber];

	int x = item->Position.xPos + 614 * phd_sin(item->Position.yRot);
	int y = item->Position.yPos;
	int z = item->Position.zPos + 614 * phd_cos(item->Position.yRot);

	short roomNumber = item->RoomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, y, z);

	if (item->ActiveState == 5 && floor->Stopper)
	{
		ROOM_INFO* room = &g_Level.Rooms[item->RoomNumber];

		for (int i = 0; i < room->mesh.size(); i++)
		{
			MESH_INFO* mesh = &room->mesh[i];

			if (((mesh->pos.zPos / 1024) == (z / 1024)) && 
				((mesh->pos.xPos / 1024) == (x / 1024)) && 
				StaticObjects[mesh->staticNumber].shatterType != SHT_NONE)
			{
				ShatterObject(NULL, mesh, -64, item->RoomNumber, 0);
				SoundEffect(SFX_TR4_HIT_ROCK, &item->Position, 0);

				mesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
				floor->Stopper = false;

				TestTriggers(x, y, z, item->RoomNumber, true);
			}
		}
	}

	x = item->Position.xPos - 614 * phd_sin(item->Position.yRot);
	y = item->Position.yPos;
	z = item->Position.zPos - 614 * phd_cos(item->Position.yRot);

	roomNumber = item->RoomNumber;

	floor = GetFloor(x, y, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, y, z);

	phd_atan(1228, height2 - height1);

	if (item->AIBits)
		GetAITarget(creature);
	else
		creature->enemy = LaraItem;

	AI_INFO info;
	CreatureAIInfo(item, &info);

	if (creature->enemy != LaraItem)
		phd_atan(LaraItem->Position.zPos - item->Position.zPos, LaraItem->Position.xPos - item->Position.xPos);

	GetCreatureMood(item, &info, VIOLENT);
	CreatureMood(item, &info, VIOLENT);

	short angle = CreatureTurn(item, creature->maximumTurn);

	int dx = abs(item->ItemFlags[2] - (short)item->Position.xPos);
	int dz = abs(item->ItemFlags[3] - (short)item->Position.zPos);

	switch (item->ActiveState)
	{
	case SPHINX_SLEEPING:
		creature->maximumTurn = 0;

		if (info.distance < SQUARE(1024) || item->TriggerFlags)
		{
			item->TargetState = SPHINX_WAKING_UP;
		}

		if (GetRandomControl() == 0)
		{
			item->TargetState = SPHINX_ALERTED;
		}

		break;

	case SPHINX_ALERTED:
		creature->maximumTurn = 0;

		if (info.distance < SQUARE(1024) || item->TriggerFlags)
		{
			item->TargetState = SPHINX_WAKING_UP;
		}

		if (GetRandomControl() == 0)
		{
			item->TargetState = SPHINX_SLEEPING;
		}

		break;

	case SPHINX_WALK:
		creature->maximumTurn = ANGLE(3);

		if (info.distance > SQUARE(1024) && abs(info.angle) <= 512 || item->RequiredState == SPHINX_RUN)
		{
			item->TargetState = SPHINX_RUN;
		}
		else if (info.distance < SQUARE(2048) && item->TargetState != SPHINX_RUN)
		{
			if (height2 <= item->Position.yPos + 256 && height2 >= item->Position.yPos - 256)
			{
				item->TargetState = SPHINX_STOP;
				item->RequiredState = SPHINX_WALK_BACK;
			}
		}

		break;

	case SPHINX_RUN:
		creature->maximumTurn = 60;

		if (creature->flags == 0)
		{
			if (item->TouchBits & 0x40)
			{
				CreatureEffect2(
					item,
					&sphinxBiteInfo,
					20,
					-1,
					DoBloodSplat);

				LaraItem->HitPoints -= 200;
				creature->flags = 1;
			}
		}

		if (dx >= 50 || dz >= 50 || item->AnimNumber != Objects[item->ObjectNumber].animIndex)
		{
			if (info.distance > SQUARE(2048) && abs(info.angle) > 512)
			{
				item->TargetState = SPHINX_STOP;
			}
		}
		else
		{
			item->TargetState = SPHINX_HIT;
			item->RequiredState = SPHINX_WALK_BACK;
			creature->maximumTurn = 0;
		}

		break;

	case SPHINX_WALK_BACK:
		creature->maximumTurn = ANGLE(3);
		if (info.distance > SQUARE(2048) || height2 > item->Position.yPos + 256 || height2 < item->Position.yPos - 256)
		{
			item->TargetState = SPHINX_STOP;
			item->RequiredState = SPHINX_RUN;
		}

		break;

	case SPHINX_HIT:
		if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
		{
			TestTriggers(item, true);

			if (item->TouchBits & 0x40)
			{
				CreatureEffect2(
					item,
					&sphinxBiteInfo,
					50,
					-1,
					DoBloodSplat);

				LaraItem->HitPoints = 0;
			}
		}

		break;

	case SPHINX_STOP:
		creature->flags = 0;

		if (item->RequiredState == SPHINX_WALK_BACK)
		{
			item->TargetState = SPHINX_WALK_BACK;
		}
		else
		{
			item->TargetState = SPHINX_WALK;
		}

		break;

	default:
		break;
	}

	item->ItemFlags[2] = (short)item->Position.xPos;
	item->ItemFlags[3] = (short)item->Position.zPos;

	CreatureAnimation(itemNumber, angle, 0);
}