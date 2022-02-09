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

	if (item->objectNumber == ID_CROW)
	{
		item->animNumber = Objects[ID_CROW].animIndex + 14;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->activeState = item->targetState = 7;
	}
	else
	{
		item->animNumber = Objects[ID_EAGLE].animIndex + 5;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->activeState = item->targetState = 2;
	}
}

void EagleControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	short angle = 0;

	if (item->hitPoints <= 0)
	{
		switch (item->activeState)
		{
		case 4:
			if (item->pos.yPos > item->floor)
			{
				item->pos.yPos = item->floor;
				item->Airborne = 0;
				item->VerticalVelocity = 0;
				item->targetState = 5;
			}
			break;

		case 5:
			item->pos.yPos = item->floor;
			break;

		default:
			if (item->objectNumber == ID_CROW)
				item->animNumber = Objects[ID_CROW].animIndex + 1;
			else
				item->animNumber = Objects[ID_EAGLE].animIndex + 8;

			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->activeState = 4;
			item->Airborne = 1;
			item->Velocity = 0;
			break;
		}
		item->pos.xRot = 0;
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, ANGLE(3));

		switch (item->activeState)
		{
		case 7:
			item->pos.yPos = item->floor;
			if (creature->mood != BORED_MOOD)
				item->targetState = 1;
			break;

		case 2:
			item->pos.yPos = item->floor;
			if (creature->mood == BORED_MOOD)
				break;
			else
				item->targetState = 1;
			break;

		case 1:
			creature->flags = 0;

			if (item->requiredState)
				item->targetState = item->requiredState;
			if (creature->mood == BORED_MOOD)
				item->targetState = 2;
			else if (info.ahead && info.distance < SQUARE(512))
				item->targetState = 6;
			else
				item->targetState = 3;
			break;

		case 3:
			if (creature->mood == BORED_MOOD)
			{
				item->requiredState = 2;
				item->targetState = 1;
			}
			else if (info.ahead && info.distance < SQUARE(512))
				item->targetState = 6;
			break;

		case 6:
			if (!creature->flags && item->touchBits)
			{
				LaraItem->hitPoints -= 20;
				LaraItem->hitStatus = true;

				if (item->objectNumber == ID_CROW)
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