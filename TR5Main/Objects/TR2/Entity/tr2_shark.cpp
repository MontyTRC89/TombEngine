#include "framework.h"
#include "Objects/TR2/Entity/tr2_shark.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO sharkBite = { 17, -22, 344, 12 };

void SharkControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	short angle = 0;
	short head = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 5)
		{
			item->animNumber = Objects[ID_SHARK].animIndex + 4;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 5;
		}
		CreatureFloat(itemNum);
		return;
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		switch (item->currentAnimState)
		{
		case 0:
			creature->flags = 0;
			creature->maximumTurn = 0;

			if (info.ahead && info.distance < SQUARE(768) && info.zoneNumber == info.enemyZone)
				item->goalAnimState = 3;
			else
				item->goalAnimState = 1;
			break;

		case 1:
			creature->maximumTurn = ANGLE(1) / 2;
			if (creature->mood == BORED_MOOD)
				break;
			else if (info.ahead && info.distance < SQUARE(768))
				item->goalAnimState = 0;
			else if (creature->mood == ESCAPE_MOOD || info.distance > SQUARE(3072) || !info.ahead)
				item->goalAnimState = 2;
			break;

		case 2:
			creature->flags = 0;
			creature->maximumTurn = ANGLE(2);

			if (creature->mood == BORED_MOOD)
				item->goalAnimState = 1;
			else if (creature->mood == ESCAPE_MOOD)
				break;
			else if (info.ahead && info.distance < SQUARE(1365) && info.zoneNumber == info.enemyZone)
			{
				if (GetRandomControl() < 0x800)
					item->goalAnimState = 0;
				else if (info.distance < SQUARE(768))
					item->goalAnimState = 4;
			}
			break;

		case 3:
		case 4:
			if (info.ahead)
				head = info.angle;

			if (!creature->flags && (item->touchBits & 0x3400))
			{
				LaraItem->hitPoints -= 400;
				LaraItem->hitStatus = true;
				CreatureEffect(item, &sharkBite, DoBloodSplat);

				creature->flags = 1;
			}
			break;
		}
	}

	if (item->currentAnimState != 6)
	{
		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNum, angle, 0);
		CreatureUnderwater(item, 340);
	}
	else
	{
		AnimateItem(item);
	}
}