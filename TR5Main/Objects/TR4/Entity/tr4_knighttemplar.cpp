#include "framework.h"
#include "tr4_knighttemplar.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/debris.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

BITE_INFO knightTemplarBite = { 0, 0, 0, 11 };

void InitialiseKnightTemplar(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->animNumber = Objects[ID_KNIGHT_TEMPLAR].animIndex + 2;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->meshBits &= 0xF7FF;
}

void KnightTemplarControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	if (item->animNumber == obj->animIndex ||
		item->animNumber - obj->animIndex == 1 ||
		item->animNumber - obj->animIndex == 11 ||
		item->animNumber - obj->animIndex == 12)
	{
		if (GetRandomControl() & 1)
		{
			PHD_VECTOR pos;

			pos.x = 0;
			pos.y = 48;
			pos.z = 448;

			GetJointAbsPosition(item, &pos, 10);
			TriggerMetalSparks(pos.x, pos.y, pos.z, (GetRandomControl() & 0x1FF) - 256, -128 - (GetRandomControl() & 0x7F), (GetRandomControl() & 0x1FF) - 256, 0);
		}
	}

	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	short tilt = 0;
	short angle = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;

	// Knight is immortal
	if (item->hitPoints < obj->hitPoints)
		item->hitPoints = obj->hitPoints;

	if (item->aiBits)
		GetAITarget(creature);
	else if (creature->hurtByLara)
		creature->enemy = LaraItem;

	AI_INFO info;
	AI_INFO laraInfo;

	CreatureAIInfo(item, &info);

	int a = 0;
	if (creature->enemy != LaraItem)
		a = phd_atan(item->pos.zPos - LaraItem->pos.zPos, item->pos.xPos - LaraItem->pos.xPos);

	GetCreatureMood(item, &info, VIOLENT);
	CreatureMood(item, &info, VIOLENT);

	angle = CreatureTurn(item, creature->maximumTurn);

	if (info.ahead)
	{
		joint0 = info.angle / 2;
		joint2 = info.angle / 2;
		joint1 = info.xAngle;
	}

	int frameBase = 0;
	int frameNumber = 0;

	switch (item->currentAnimState)
	{
	case 1:
		creature->flags = 0;
		creature->maximumTurn = ANGLE(2);

		item->goalAnimState = 2;

		if (info.distance > SQUARE(682))
		{
			if (Lara.target == item)
				item->goalAnimState = 6;
		}
		else if (GetRandomControl() & 1)
		{
			item->goalAnimState = 4;
		}
		else if (GetRandomControl() & 1)
		{
			item->goalAnimState = 3;
		}
		else
		{
			item->goalAnimState = 5;
		}

		break;

	case 2:
		creature->maximumTurn = ANGLE(7);

		if (Lara.target == item || info.distance <= SQUARE(682))
		{
			item->goalAnimState = 1;
		}

		break;

	case 3:
	case 4:
	case 5:
		creature->maximumTurn = 0;

		if (abs(info.angle) >= ANGLE(1))
		{
			if (info.angle >= 0)
			{
				item->pos.yRot += ANGLE(1);
			}
			else
			{
				item->pos.yRot -= ANGLE(1);
			}
		}
		else
		{
			item->pos.yRot += info.angle;
		}

		frameNumber = item->frameNumber;
		frameBase = g_Level.Anims[item->animNumber].frameBase;

		if (frameNumber > frameBase + 42 && frameNumber < frameBase + 51)
		{
			PHD_VECTOR pos;
			pos.x = 0;
			pos.y = 0;
			pos.z = 0;

			GetJointAbsPosition(item, &pos, 11);

			ROOM_INFO* room = &g_Level.Rooms[item->roomNumber];

			FLOOR_INFO* currentFloor = &room->floor[(pos.z - room->z) / SECTOR(1) + (pos.z - room->x) / SECTOR(1) * room->zSize];

			if (currentFloor->Stopper)
			{
				for (int i = 0; i < room->mesh.size(); i++)
				{
					MESH_INFO* mesh = &room->mesh[i];

					if (floor(pos.x) == floor(mesh->pos.xPos) &&
						floor(pos.z) == floor(mesh->pos.zPos) &&
						StaticObjects[mesh->staticNumber].shatterType != SHT_NONE)
					{
						ShatterObject(NULL, mesh, -64, LaraItem->roomNumber, 0);
						SoundEffect(SFX_TR4_HIT_ROCK, &item->pos, 0);

						mesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
						currentFloor->Stopper = false;

						TestTriggers(pos.x, pos.y, pos.z, item->roomNumber, true);
					}

					mesh++;
				}
			}

			if (!creature->flags)
			{
				if (item->touchBits & 0xC00)
				{
					LaraItem->hitPoints -= 120;
					LaraItem->hitStatus = true;

					CreatureEffect2(
						item,
						&knightTemplarBite,
						20,
						-1,
						DoBloodSplat);

					creature->flags = 1;
				}
			}
		}

	case 6:
		creature->maximumTurn = 0;

		if (abs(info.angle) >= ANGLE(1))
		{
			if (info.angle >= 0)
			{
				item->pos.yRot += ANGLE(1);
			}
			else
			{
				item->pos.yRot -= ANGLE(1);
			}
		}
		else
		{
			item->pos.yRot += info.angle;
		}

		if (item->hitStatus)
		{
			if (GetRandomControl() & 1)
			{
				item->goalAnimState = 7;
			}
			else
			{
				item->goalAnimState = 8;
			}
		}
		else if (info.distance <= SQUARE(682) || Lara.target != item)
		{
			item->goalAnimState = 1;
		}
		else
		{
			item->goalAnimState = 6;
		}
		break;

	default:
		break;

	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureAnimation(itemNumber, angle, tilt);
}