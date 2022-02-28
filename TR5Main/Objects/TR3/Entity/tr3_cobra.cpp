#include "framework.h"
#include "Objects/TR3/Entity/tr3_cobra.h"

#include "Game/control/box.h"
#include "Game/itemdata/creature_info.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO CobraBite = { 0, 0, 0, 13 };

// TODO
enum CobraState
{

};

// TODO
enum CobraAnim
{

};

void InitialiseCobra(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 2;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase + 45;
	item->ActiveState = item->TargetState = 3;
	item->ItemFlags[2] = item->HitStatus;
}

void CobraControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	short head = 0;
	short angle = 0;
	short tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 4)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 4;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 4;
		}
	}
	else
	{
		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		aiInfo.angle += 0xC00;

		GetCreatureMood(item, &aiInfo, 1);
		CreatureMood(item, &aiInfo, 1);

		info->target.x = LaraItem->Position.xPos;
		info->target.z = LaraItem->Position.zPos;
		angle = CreatureTurn(item, info->maximumTurn);

		if (aiInfo.ahead)
			head = aiInfo.angle;

		if (abs(aiInfo.angle) < ANGLE(10.0f))
			item->Position.yRot += aiInfo.angle;
		else if (aiInfo.angle < 0)
			item->Position.yRot -= ANGLE(10.0f);
		else
			item->Position.yRot += ANGLE(10.0f);

		switch (item->ActiveState)
		{
		case 1:
			info->flags = 0;
			if (aiInfo.distance > pow(SECTOR(2.5f), 2))
				item->TargetState = 3;
			else if (LaraItem->HitPoints > 0 &&
				((aiInfo.ahead && aiInfo.distance < pow(SECTOR(1), 2)) || item->HitStatus || LaraItem->Velocity > 15))
			{
				item->TargetState = 2;
			}

			break;

		case 3:
			info->flags = 0;
			if (item->HitPoints != -16384)
			{
				item->ItemFlags[2] = item->HitPoints;
				item->HitPoints = -16384;
			}
			if (aiInfo.distance < pow(SECTOR(1.5f), 2) && LaraItem->HitPoints > 0)
			{
				item->TargetState = 0;
				item->HitPoints = item->ItemFlags[2];
			}

			break;

		case 2:
			if (info->flags != 1 && item->TouchBits & 0x2000)
			{
				info->flags = 1;

				LaraItem->HitPoints -= 80;
				LaraItem->HitStatus = true;
				Lara.Poisoned = 0x100;

				CreatureEffect(item, &CobraBite, DoBloodSplat);
			}

			break;

		case 0:
			item->HitPoints = item->ItemFlags[2];
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head >> 1);
	CreatureJoint(item, 1, head >> 1);
	CreatureAnimation(itemNumber, angle, tilt);
}
