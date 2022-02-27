#include "framework.h"
#include "Objects/TR2/Entity/tr2_eagle_or_crow.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO EagleBite = { 15, 46, 21, 6 };
BITE_INFO CrowBite = { 2, 10, 60, 14 };

// TODO
enum EagleState
{

};

// TODO
enum EagleAnim
{

};

void InitialiseEagle(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	if (item->ObjectNumber == ID_CROW)
	{
		item->AnimNumber = Objects[ID_CROW].animIndex + 14;
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		item->ActiveState = item->TargetState = 7;
	}
	else
	{
		item->AnimNumber = Objects[ID_EAGLE].animIndex + 5;
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		item->ActiveState = item->TargetState = 2;
	}
}

void EagleControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	short angle = 0;

	if (item->HitPoints <= 0)
	{
		switch (item->ActiveState)
		{
		case 4:
			if (item->Position.yPos > item->Floor)
			{
				item->Position.yPos = item->Floor;
				item->VerticalVelocity = 0;
				item->Airborne = false;
				item->TargetState = 5;
			}

			break;

		case 5:
			item->Position.yPos = item->Floor;
			break;

		default:
			if (item->ObjectNumber == ID_CROW)
				item->AnimNumber = Objects[ID_CROW].animIndex + 1;
			else
				item->AnimNumber = Objects[ID_EAGLE].animIndex + 8;

			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 4;
			item->Velocity = 0;
			item->Airborne = true;
			break;
		}
		item->Position.xRot = 0;
	}
	else
	{
		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		GetCreatureMood(item, &aiInfo, VIOLENT);
		CreatureMood(item, &aiInfo, TIMID);

		angle = CreatureTurn(item, ANGLE(3.0f));

		switch (item->ActiveState)
		{
		case 7:
			item->Position.yPos = item->Floor;

			if (info->mood != BORED_MOOD)
				item->TargetState = 1;

			break;

		case 2:
			item->Position.yPos = item->Floor;

			if (info->mood == BORED_MOOD)
				break;
			else
				item->TargetState = 1;

			break;

		case 1:
			info->flags = 0;

			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			if (info->mood == BORED_MOOD)
				item->TargetState = 2;
			else if (aiInfo.ahead && aiInfo.distance < pow(SECTOR(0.5f), 2))
				item->TargetState = 6;
			else
				item->TargetState = 3;

			break;

		case 3:
			if (info->mood == BORED_MOOD)
			{
				item->RequiredState = 2;
				item->TargetState = 1;
			}
			else if (aiInfo.ahead && aiInfo.distance < pow(SECTOR(0.5f), 2))
				item->TargetState = 6;

			break;

		case 6:
			if (!info->flags && item->TouchBits)
			{
				LaraItem->HitPoints -= 20;
				LaraItem->HitStatus = true;

				if (item->ObjectNumber == ID_CROW)
					CreatureEffect(item, &CrowBite, DoBloodSplat);
				else
					CreatureEffect(item, &EagleBite, DoBloodSplat);

				info->flags = 1;
			}

			break;
		}
	}

	CreatureAnimation(itemNumber, angle, 0);
}
