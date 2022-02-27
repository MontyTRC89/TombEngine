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

	item->AnimNumber = Objects[ID_KNIGHT_TEMPLAR].animIndex + 2;
	item->TargetState = 1;
	item->ActiveState = 1;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->MeshBits &= 0xF7FF;
}

void KnightTemplarControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* objectInfo = &Objects[item->ObjectNumber];

	if (item->AnimNumber == objectInfo->animIndex ||
		item->AnimNumber - objectInfo->animIndex == 1 ||
		item->AnimNumber - objectInfo->animIndex == 11 ||
		item->AnimNumber - objectInfo->animIndex == 12)
	{
		if (GetRandomControl() & 1)
		{
			PHD_VECTOR pos = { 0, 48, 448 };
			GetJointAbsPosition(item, &pos, 10);

			TriggerMetalSparks(pos.x, pos.y, pos.z, (GetRandomControl() & 0x1FF) - 256, -128 - (GetRandomControl() & 0x7F), (GetRandomControl() & 0x1FF) - 256, 0);
		}
	}

	auto* info = GetCreatureInfo(item);

	short tilt = 0;
	short angle = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;

	// Knight is immortal
	if (item->HitPoints < objectInfo->HitPoints)
		item->HitPoints = objectInfo->HitPoints;

	if (item->AIBits)
		GetAITarget(info);
	else if (info->hurtByLara)
		info->enemy = LaraItem;

	AI_INFO aiInfo;
	CreatureAIInfo(item, &aiInfo);

	int a = 0;
	if (info->enemy != LaraItem)
		a = phd_atan(item->Position.zPos - LaraItem->Position.zPos, item->Position.xPos - LaraItem->Position.xPos);

	GetCreatureMood(item, &aiInfo, VIOLENT);
	CreatureMood(item, &aiInfo, VIOLENT);

	angle = CreatureTurn(item, info->maximumTurn);

	if (aiInfo.ahead)
	{
		joint0 = aiInfo.angle / 2;
		joint1 = aiInfo.xAngle;
		joint2 = aiInfo.angle / 2;
	}

	int frameBase = 0;
	int frameNumber = 0;

	switch (item->ActiveState)
	{
	case 1:
		info->maximumTurn = ANGLE(2.0f);
		item->TargetState = 2;
		info->flags = 0;

		if (aiInfo.distance > SQUARE(682))
		{
			if (Lara.target == item)
				item->TargetState = 6;
		}
		else if (GetRandomControl() & 1)
			item->TargetState = 4;
		else if (GetRandomControl() & 1)
			item->TargetState = 3;
		else
			item->TargetState = 5;

		break;

	case 2:
		info->maximumTurn = ANGLE(7);

		if (Lara.target == item || aiInfo.distance <= SQUARE(682))
			item->TargetState = 1;

		break;

	case 3:
	case 4:
	case 5:
		info->maximumTurn = 0;

		if (abs(aiInfo.angle) >= ANGLE(1))
		{
			if (aiInfo.angle >= 0)
				item->Position.yRot += ANGLE(1);
			else
				item->Position.yRot -= ANGLE(1);
		}
		else
			item->Position.yRot += aiInfo.angle;

		frameNumber = item->FrameNumber;
		frameBase = g_Level.Anims[item->AnimNumber].frameBase;

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

			if (!info->flags)
			{
				if (item->TouchBits & 0xC00)
				{
					LaraItem->HitPoints -= 120;
					LaraItem->HitStatus = true;

					CreatureEffect2(
						item,
						&KnightTemplarBite,
						20,
						-1,
						DoBloodSplat);

					info->flags = 1;
				}
			}
		}

	case 6:
		info->maximumTurn = 0;

		if (abs(aiInfo.angle) >= ANGLE(1))
		{
			if (aiInfo.angle >= 0)
				item->Position.yRot += ANGLE(1);
			else
				item->Position.yRot -= ANGLE(1);
		}
		else
			item->Position.yRot += aiInfo.angle;

		if (item->HitStatus)
		{
			if (GetRandomControl() & 1)
				item->TargetState = 7;
			else
				item->TargetState = 8;
		}
		else if (aiInfo.distance <= SQUARE(682) || Lara.target != item)
			item->TargetState = 1;
		else
			item->TargetState = 6;
		
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
