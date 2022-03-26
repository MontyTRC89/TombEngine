#include "framework.h"
#include "tr4_knighttemplar.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/debris.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

BITE_INFO KnightTemplarBite = { 0, 0, 0, 11 };

void InitialiseKnightTemplar(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->Animation.AnimNumber = Objects[ID_KNIGHT_TEMPLAR].animIndex + 2;
	item->Animation.TargetState = 1;
	item->Animation.ActiveState = 1;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
	item->MeshBits &= 0xF7FF;
}

void KnightTemplarControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);
	auto* object = &Objects[item->ObjectNumber];

	if (item->Animation.AnimNumber == object->animIndex ||
		item->Animation.AnimNumber - object->animIndex == 1 ||
		item->Animation.AnimNumber - object->animIndex == 11 ||
		item->Animation.AnimNumber - object->animIndex == 12)
	{
		if (GetRandomControl() & 1)
		{
			PHD_VECTOR pos = { 0, 48, 448 };
			GetJointAbsPosition(item, &pos, 10);

			TriggerMetalSparks(pos.x, pos.y, pos.z, (GetRandomControl() & 0x1FF) - 256, -128 - (GetRandomControl() & 0x7F), (GetRandomControl() & 0x1FF) - 256, 0);
		}
	}

	short tilt = 0;
	short angle = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;

	// Knight is immortal.
	if (item->HitPoints < object->HitPoints)
		item->HitPoints = object->HitPoints;

	if (item->AIBits)
		GetAITarget(creature);
	else if (creature->HurtByLara)
		creature->Enemy = LaraItem;

	AI_INFO AI;
	CreatureAIInfo(item, &AI);

	int a = 0;
	if (creature->Enemy != LaraItem)
		a = phd_atan(item->Position.zPos - LaraItem->Position.zPos, item->Position.xPos - LaraItem->Position.xPos);

	GetCreatureMood(item, &AI, VIOLENT);
	CreatureMood(item, &AI, VIOLENT);

	angle = CreatureTurn(item, creature->MaxTurn);

	if (AI.ahead)
	{
		joint0 = AI.angle / 2;
		joint1 = AI.xAngle;
		joint2 = AI.angle / 2;
	}

	int frameBase = 0;
	int frameNumber = 0;

	switch (item->Animation.ActiveState)
	{
	case 1:
		item->Animation.TargetState = 2;
		creature->MaxTurn = ANGLE(2.0f);
		creature->Flags = 0;

		if (AI.distance > pow(682, 2))
		{
			if (Lara.TargetEntity == item)
				item->Animation.TargetState = 6;
		}
		else if (GetRandomControl() & 1)
			item->Animation.TargetState = 4;
		else if (GetRandomControl() & 1)
			item->Animation.TargetState = 3;
		else
			item->Animation.TargetState = 5;

		break;

	case 2:
		creature->MaxTurn = ANGLE(7.0f);

		if (Lara.TargetEntity == item || AI.distance <= pow(682, 2))
			item->Animation.TargetState = 1;

		break;

	case 3:
	case 4:
	case 5:
		creature->MaxTurn = 0;

		if (abs(AI.angle) >= ANGLE(1.0f))
		{
			if (AI.angle >= 0)
				item->Position.yRot += ANGLE(1.0f);
			else
				item->Position.yRot -= ANGLE(1.0f);
		}
		else
			item->Position.yRot += AI.angle;

		frameNumber = item->Animation.FrameNumber;
		frameBase = g_Level.Anims[item->Animation.AnimNumber].FrameBase;

		if (frameNumber > frameBase + 42 && frameNumber < frameBase + 51)
		{
			PHD_VECTOR pos = { 0, 0, 0 };
			GetJointAbsPosition(item, &pos, 11);

			auto* room = &g_Level.Rooms[item->RoomNumber];
			FLOOR_INFO* currentFloor = &room->floor[(pos.z - room->z) / SECTOR(1) + (pos.z - room->x) / SECTOR(1) * room->zSize];

			if (currentFloor->Stopper)
			{
				for (int i = 0; i < room->mesh.size(); i++)
				{
					auto* mesh = &room->mesh[i];

					if (floor(pos.x) == floor(mesh->pos.xPos) &&
						floor(pos.z) == floor(mesh->pos.zPos) &&
						StaticObjects[mesh->staticNumber].shatterType != SHT_NONE)
					{
						ShatterObject(NULL, mesh, -64, LaraItem->RoomNumber, 0);
						SoundEffect(SFX_TR4_HIT_ROCK, &item->Position, 0);

						mesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
						currentFloor->Stopper = false;

						TestTriggers(pos.x, pos.y, pos.z, item->RoomNumber, true);
					}

					mesh++;
				}
			}

			if (!creature->Flags)
			{
				if (item->TouchBits & 0xC00)
				{
					CreatureEffect2(
						item,
						&KnightTemplarBite,
						20,
						-1,
						DoBloodSplat);

					creature->Flags = 1;

					LaraItem->HitPoints -= 120;
					LaraItem->HitStatus = true;
				}
			}
		}

	case 6:
		creature->MaxTurn = 0;

		if (abs(AI.angle) >= ANGLE(1.0f))
		{
			if (AI.angle >= 0)
				item->Position.yRot += ANGLE(1.0f);
			else
				item->Position.yRot -= ANGLE(1.0f);
		}
		else
			item->Position.yRot += AI.angle;

		if (item->HitStatus)
		{
			if (GetRandomControl() & 1)
				item->Animation.TargetState = 7;
			else
				item->Animation.TargetState = 8;
		}
		else if (AI.distance <= pow(682, 2) || Lara.TargetEntity != item)
			item->Animation.TargetState = 1;
		else
			item->Animation.TargetState = 6;
		
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
