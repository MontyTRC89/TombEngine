#include "../newobjects.h"
#include "../../Game/box.h"
#include "../../Game/effects.h"
#include "../../specific/setup.h"
#include "..\..\Specific\level.h"

BITE_INFO bearBite = { 0, 96, 335, 14 };

void BearControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	short head = 0;
	short angle = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 9)
		{
			angle = CreatureTurn(item, ANGLE(1));

			switch (item->currentAnimState)
			{
			case 2:
				item->goalAnimState = 4;
				break;
			case 3:
			case 0:
				item->goalAnimState = 1;
				break;

			case 4:
				creature->flags = 1;
				item->goalAnimState = 9;
				break;

			case 1:
				creature->flags = 0;
				item->goalAnimState = 9;
				break;

			case 9:
				if (creature->flags && (item->touchBits & 0x2406C))
				{
					LaraItem->hitPoints -= 200;
					LaraItem->hitStatus = 1;
					creature->flags = 0;
				}

				item->animNumber = Objects[item->objectNumber].animIndex + 20;
				item->frameNumber = Anims[item->animNumber].frameBase;
				item->currentAnimState = 9;
				break;
			}
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (item->hitStatus)
			creature->flags = 1;

		switch (item->currentAnimState)
		{
		case 1:
			if (LaraItem->hitPoints <= 0)
			{
				if (info.bite && info.distance < SQUARE(768))
				{
					item->goalAnimState = 8;
				}
				else
				{
					item->goalAnimState = 0;
				}
			}
			else if (item->requiredAnimState)
			{
				item->goalAnimState = item->requiredAnimState;
			}
			else if (creature->mood == BORED_MOOD)
			{
				item->goalAnimState = 0;
			}
			else
			{
				item->goalAnimState = 3;
			}
			break;

		case 0:
			creature->maximumTurn = ANGLE(2);

			if (LaraItem->hitPoints <= 0 && (item->touchBits & 0x2406C) && info.ahead)
			{
				item->goalAnimState = 1;
			}
			else if (creature->mood != BORED_MOOD)
			{
				item->goalAnimState = 1;
				if (creature->mood == ESCAPE_MOOD)
				{
					item->requiredAnimState = 0;
				}
			}
			else if (GetRandomControl() < 0x50)
			{
				item->requiredAnimState = 5;
				item->goalAnimState = 1;
			}
			break;

		case 3:
			creature->maximumTurn = ANGLE(5);

			// if the bear slams you it hurts
			if (item->touchBits & 0x2406C)
			{
				LaraItem->hitPoints -= 3;
				LaraItem->hitStatus = true;
			}

			if (creature->mood == BORED_MOOD || LaraItem->hitPoints <= 0)
			{
				item->goalAnimState = 1;
			}
			else if (info.ahead && !item->requiredAnimState)
			{
				// bear may rear up, but not after he's taken some bullets!
				if (!creature->flags && info.distance < SQUARE(2048) && GetRandomControl() < 0x300)
				{
					item->requiredAnimState = 4;
					item->goalAnimState = 1;
				}
				else if (info.distance < SQUARE(1024))
				{
					item->goalAnimState = 6;
				}
			}
			break;

		case 4:
			if (creature->flags)
			{
				item->requiredAnimState = 0;
				item->goalAnimState = 1;
			}
			else if (item->requiredAnimState)
			{
				item->goalAnimState = item->requiredAnimState;
			}
			else if (creature->mood == BORED_MOOD || creature->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = 1;
			}
			else if (info.bite && info.distance < SQUARE(600))
			{
				item->goalAnimState = 7;
			}
			else
			{
				item->goalAnimState = 2;
			}
			break;

		case 2:
			if (creature->flags)
			{
				item->requiredAnimState = 0;
				item->goalAnimState = 4;
			}
			else if (info.ahead && (item->touchBits & 0x2406C))
			{
				item->goalAnimState = 4;
			}
			else if (creature->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = 4;
				item->requiredAnimState = 0;
			}
			else if (creature->mood == BORED_MOOD || GetRandomControl() < 0x50)
			{
				item->requiredAnimState = 5;
				item->goalAnimState = 4;
			}
			else if (info.distance > SQUARE(2048) || GetRandomControl() < 0x600)
			{
				item->requiredAnimState = 1;
				item->goalAnimState = 4;
			}
			break;

		case 7:
			if (!item->requiredAnimState && (item->touchBits & 0x2406C))
			{
				LaraItem->hitPoints -= 400;
				LaraItem->hitStatus = true;
				item->requiredAnimState = 4;
			}

			break;

		case 6:
			if (!item->requiredAnimState && (item->touchBits & 0x2406C))
			{
				CreatureEffect(item, &bearBite, DoBloodSplat);
				LaraItem->hitPoints -= 200;
				LaraItem->hitStatus = true;
				item->requiredAnimState = 1;
			}
			break;

		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, 0);
}