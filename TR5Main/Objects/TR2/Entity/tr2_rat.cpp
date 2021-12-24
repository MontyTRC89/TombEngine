#include "framework.h"
#include "Objects/TR2/Entity/tr2_rat.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO ratBite = { 0, 0, 57, 2 };

void RatControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	short head = 0;
	short angle = 0;
	short random = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 6)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 9;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 6;
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, ANGLE(6));

		switch (item->currentAnimState)
		{
		case 4:
			if (creature->mood == BORED_MOOD || creature->mood == STALK_MOOD)
			{
				short random = (short)GetRandomControl();
				if (random < 0x500)
					item->requiredAnimState = 3;
				else if (random > 0xA00)
					item->requiredAnimState = 1;
			}
			else if (info.distance < SQUARE(340))
				item->requiredAnimState = 5;
			else
				item->requiredAnimState = 1;

			if (item->requiredAnimState)
				item->goalAnimState = 2;
			break;

		case 2:
			creature->maximumTurn = 0;

			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			break;

		case 1:
			creature->maximumTurn = ANGLE(6);

			if (creature->mood == BORED_MOOD || creature->mood == STALK_MOOD)
			{
				random = (short)GetRandomControl();
				if (random < 0x500)
				{
					item->requiredAnimState = 3;
					item->goalAnimState = 2;
				}
				else if (random < 0xA00)
					item->goalAnimState = 2;
			}
			else if (info.ahead && info.distance < SQUARE(340))
				item->goalAnimState = 2;
			break;

		case 5:
			if (!item->requiredAnimState && (item->touchBits & 0x7F))
			{
				CreatureEffect(item, &ratBite, DoBloodSplat);
				LaraItem->hitPoints -= 20;
				LaraItem->hitStatus = true;
				item->requiredAnimState = 2;
			}
			break;

		case 3:
			if (GetRandomControl() < 0x500)
				item->goalAnimState = 2;
			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, 0);
}
