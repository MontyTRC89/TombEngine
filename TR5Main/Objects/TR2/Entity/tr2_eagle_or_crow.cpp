#include "framework.h"
#include "Objects/TR2/Entity/tr2_eagle_or_crow.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO eagleBite = { 15, 46, 21, 6 };
BITE_INFO crowBite = { 2, 10, 60, 14 };

void InitialiseEagle(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	ClearItem(itemNum);

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

void EagleControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;

	short angle = 0;

	if (item->HitPoints <= 0)
	{
		switch (item->ActiveState)
		{
		case 4:
			if (item->Position.yPos > item->Floor)
			{
				item->Position.yPos = item->Floor;
				item->Airborne = 0;
				item->VerticalVelocity = 0;
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
			item->Airborne = 1;
			item->Velocity = 0;
			break;
		}
		item->Position.xRot = 0;
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, ANGLE(3));

		switch (item->ActiveState)
		{
		case 7:
			item->Position.yPos = item->Floor;
			if (creature->mood != BORED_MOOD)
				item->TargetState = 1;
			break;

		case 2:
			item->Position.yPos = item->Floor;
			if (creature->mood == BORED_MOOD)
				break;
			else
				item->TargetState = 1;
			break;

		case 1:
			creature->flags = 0;

			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			if (creature->mood == BORED_MOOD)
				item->TargetState = 2;
			else if (info.ahead && info.distance < SQUARE(512))
				item->TargetState = 6;
			else
				item->TargetState = 3;
			break;

		case 3:
			if (creature->mood == BORED_MOOD)
			{
				item->RequiredState = 2;
				item->TargetState = 1;
			}
			else if (info.ahead && info.distance < SQUARE(512))
				item->TargetState = 6;
			break;

		case 6:
			if (!creature->flags && item->TouchBits)
			{
				LaraItem->HitPoints -= 20;
				LaraItem->HitStatus = true;

				if (item->ObjectNumber == ID_CROW)
					CreatureEffect(item, &crowBite, DoBloodSplat);
				else
					CreatureEffect(item, &eagleBite, DoBloodSplat);

				creature->flags = 1;
			}
			break;
		}
	}

	CreatureAnimation(itemNum, angle, 0);
}