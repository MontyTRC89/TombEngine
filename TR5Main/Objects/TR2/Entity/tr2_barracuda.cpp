#include "framework.h"
#include "Objects/TR2/Entity/tr2_barracuda.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO barracudaBite = { 2, -60, 121, 7 };

void BarracudaControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	short angle = 0;
	short head = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 6)
		{
			item->animNumber = Objects[ID_BARRACUDA].animIndex + 6;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 6;
		}

		CreatureFloat(itemNum);
		return;
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, creature->maximumTurn);

		switch (item->currentAnimState)
		{
		case 1:
			creature->flags = 0;

			if (creature->mood == BORED_MOOD)
				item->goalAnimState = 2;
			else if (info.ahead && info.distance < 680)
				item->goalAnimState = 4;
			else if (creature->mood == STALK_MOOD)
				item->goalAnimState = 2;
			else
				item->goalAnimState = 3;
			break;

		case 2:
			creature->maximumTurn = ANGLE(2);

			if (creature->mood == BORED_MOOD)
				break;
			else if (info.ahead && (item->touchBits & 0xE0))
				item->goalAnimState = 1;
			else if (creature->mood != STALK_MOOD)
				item->goalAnimState = 3;
			break;

		case 3:
			creature->maximumTurn = ANGLE(4);
			creature->flags = 0;

			if (creature->mood == BORED_MOOD)
				item->goalAnimState = 2;
			else if (info.ahead && info.distance < 340)
				item->goalAnimState = 5;
			else if (info.ahead && info.distance < 680)
				item->goalAnimState = 1;
			else if (creature->mood == STALK_MOOD)
				item->goalAnimState = 2;
			break;

		case 4:
		case 5:
			if (info.ahead)
				head = info.angle;

			if (!creature->flags && (item->touchBits & 0xE0))
			{
				LaraItem->hitPoints -= 100;
				LaraItem->hitStatus = true;
				CreatureEffect(item, &barracudaBite, DoBloodSplat);

				creature->flags = 1;
			}
			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, 0);
	CreatureUnderwater(item, STEP_SIZE);
}