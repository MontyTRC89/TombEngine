#include "framework.h"
#include "tr4_sphinx.h"
#include "Game/collision/collide_room.h"
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

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = SPHINX_SLEEPING;
	item->Animation.ActiveState = SPHINX_SLEEPING;
}

void SphinxControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CreatureInfo* creature = (CreatureInfo*)item->Data;
	OBJECT_INFO* obj = &Objects[item->ObjectNumber];

	int x = item->Pose.Position.x + 614 * phd_sin(item->Pose.Orientation.y);
	int y = item->Pose.Position.y;
	int z = item->Pose.Position.z + 614 * phd_cos(item->Pose.Orientation.y);

	short roomNumber = item->RoomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, y, z);

	if (item->Animation.ActiveState == 5 && floor->Stopper)
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
				SoundEffect(SFX_TR4_HIT_ROCK, &item->Pose, 0);

				mesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
				floor->Stopper = false;

				TestTriggers(x, y, z, item->RoomNumber, true);
			}
		}
	}

	x = item->Pose.Position.x - 614 * phd_sin(item->Pose.Orientation.y);
	y = item->Pose.Position.y;
	z = item->Pose.Position.z - 614 * phd_cos(item->Pose.Orientation.y);

	roomNumber = item->RoomNumber;

	floor = GetFloor(x, y, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, y, z);

	phd_atan(1228, height2 - height1);

	if (item->AIBits)
		GetAITarget(creature);
	else
		creature->Enemy = LaraItem;

	AI_INFO info;
	CreatureAIInfo(item, &info);

	if (creature->Enemy != LaraItem)
		phd_atan(LaraItem->Pose.Position.z - item->Pose.Position.z, LaraItem->Pose.Position.x - item->Pose.Position.x);

	GetCreatureMood(item, &info, VIOLENT);
	CreatureMood(item, &info, VIOLENT);

	short angle = CreatureTurn(item, creature->MaxTurn);

	int dx = abs(item->ItemFlags[2] - (short)item->Pose.Position.x);
	int dz = abs(item->ItemFlags[3] - (short)item->Pose.Position.z);

	switch (item->Animation.ActiveState)
	{
	case SPHINX_SLEEPING:
		creature->MaxTurn = 0;

		if (info.distance < SQUARE(1024) || item->TriggerFlags)
		{
			item->Animation.TargetState = SPHINX_WAKING_UP;
		}

		if (GetRandomControl() == 0)
		{
			item->Animation.TargetState = SPHINX_ALERTED;
		}

		break;

	case SPHINX_ALERTED:
		creature->MaxTurn = 0;

		if (info.distance < SQUARE(1024) || item->TriggerFlags)
		{
			item->Animation.TargetState = SPHINX_WAKING_UP;
		}

		if (GetRandomControl() == 0)
		{
			item->Animation.TargetState = SPHINX_SLEEPING;
		}

		break;

	case SPHINX_WALK:
		creature->MaxTurn = ANGLE(3);

		if (info.distance > SQUARE(1024) && abs(info.angle) <= 512 || item->Animation.RequiredState == SPHINX_RUN)
		{
			item->Animation.TargetState = SPHINX_RUN;
		}
		else if (info.distance < SQUARE(2048) && item->Animation.TargetState != SPHINX_RUN)
		{
			if (height2 <= item->Pose.Position.y + 256 && height2 >= item->Pose.Position.y - 256)
			{
				item->Animation.TargetState = SPHINX_STOP;
				item->Animation.RequiredState = SPHINX_WALK_BACK;
			}
		}

		break;

	case SPHINX_RUN:
		creature->MaxTurn = 60;

		if (creature->Flags == 0)
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
				creature->Flags = 1;
			}
		}

		if (dx >= 50 || dz >= 50 || item->Animation.AnimNumber != Objects[item->ObjectNumber].animIndex)
		{
			if (info.distance > SQUARE(2048) && abs(info.angle) > 512)
			{
				item->Animation.TargetState = SPHINX_STOP;
			}
		}
		else
		{
			item->Animation.TargetState = SPHINX_HIT;
			item->Animation.RequiredState = SPHINX_WALK_BACK;
			creature->MaxTurn = 0;
		}

		break;

	case SPHINX_WALK_BACK:
		creature->MaxTurn = ANGLE(3);
		if (info.distance > SQUARE(2048) || height2 > item->Pose.Position.y + 256 || height2 < item->Pose.Position.y - 256)
		{
			item->Animation.TargetState = SPHINX_STOP;
			item->Animation.RequiredState = SPHINX_RUN;
		}

		break;

	case SPHINX_HIT:
		if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
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
		creature->Flags = 0;

		if (item->Animation.RequiredState == SPHINX_WALK_BACK)
		{
			item->Animation.TargetState = SPHINX_WALK_BACK;
		}
		else
		{
			item->Animation.TargetState = SPHINX_WALK;
		}

		break;

	default:
		break;
	}

	item->ItemFlags[2] = (short)item->Pose.Position.x;
	item->ItemFlags[3] = (short)item->Pose.Position.z;

	CreatureAnimation(itemNumber, angle, 0);
}