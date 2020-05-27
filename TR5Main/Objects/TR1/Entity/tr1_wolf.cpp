#include "framework.h"
#include "newobjects.h"
#include "items.h"
#include "box.h"
#include "effect.h"
#include "lara.h"
#include "setup.h"
#include "level.h"

BITE_INFO wolfBite = { 0, -14, 174, 6 };

void InitialiseWolf(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	ClearItem(itemNum);
	item->frameNumber = 96;
}

void WolfControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	short head = 0;
	short angle = 0;
	short tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 11)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 20 + (short)(GetRandomControl() / 11000);
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 11;
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

		angle = CreatureTurn(item, creature->maximumTurn);

		switch (item->currentAnimState)
		{
		case 8:
			head = 0;

			if (creature->mood == ESCAPE_MOOD || info.zoneNumber == info.enemyZone)
			{
				item->requiredAnimState = 9;
				item->goalAnimState = 1;
			}
			else if (GetRandomControl() < 0x20)
			{
				item->requiredAnimState = 2;
				item->goalAnimState = 1;
			}
			break;

		case 1:
			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else
				item->goalAnimState = 2;
			break;

		case 2:
			creature->maximumTurn = ANGLE(2);

			if (creature->mood != BORED_MOOD)
			{
				item->goalAnimState = 5;
				item->requiredAnimState = 0;
			}
			else if (GetRandomControl() < 0x20)
			{
				item->requiredAnimState = 8;
				item->goalAnimState = 1;
			}
			break;

		case 9:
			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else if (creature->mood == ESCAPE_MOOD)
				item->goalAnimState = 3;
			else if (info.distance < SQUARE(345) && info.bite)
				item->goalAnimState = 12;
			else if (creature->mood == STALK_MOOD)
				item->goalAnimState = 5;
			else if (creature->mood == BORED_MOOD)
				item->goalAnimState = 1;
			else
				item->goalAnimState = 3;
			break;

		case 5:
			creature->maximumTurn = ANGLE(2);

			if (creature->mood == ESCAPE_MOOD)
				item->goalAnimState = 3;
			else if (info.distance < SQUARE(345) && info.bite)
				item->goalAnimState = 12;
			else if (info.distance > SQUARE(3072))
				item->goalAnimState = 3;
			else if (creature->mood == ATTACK_MOOD)
			{
				if (!info.ahead || info.distance > SQUARE(1536) ||
					(info.enemyFacing < FRONT_ARC && info.enemyFacing > -FRONT_ARC))
					item->goalAnimState = 3;
			}
			else if (GetRandomControl() < 0x180)
			{
				item->requiredAnimState = 7;
				item->goalAnimState = 9;
			}
			else if (creature->mood == BORED_MOOD)
				item->goalAnimState = 9;
			break;

		case 3:
			creature->maximumTurn = ANGLE(5);
			tilt = angle;

			if (info.ahead && info.distance < SQUARE(1536))
			{
				if (info.distance > (SQUARE(1536) / 2) &&
					(info.enemyFacing > FRONT_ARC || info.enemyFacing < -FRONT_ARC))
				{
					item->requiredAnimState = 5;
					item->goalAnimState = 9;
				}
				else
				{
					item->goalAnimState = 6;
					item->requiredAnimState = 0;
				}
			}
			else if (creature->mood == STALK_MOOD && info.distance < SQUARE(3072))
			{
				item->requiredAnimState = 5;
				item->goalAnimState = 9;
			}
			else if (creature->mood == BORED_MOOD)
				item->goalAnimState = 9;
			break;

		case 6:
			tilt = angle;
			if (!item->requiredAnimState && (item->touchBits & 0x774F))
			{
				CreatureEffect(item, &wolfBite, DoBloodSplat);
				LaraItem->hitPoints -= 50;
				LaraItem->hitStatus = true;
				item->requiredAnimState = 3;
			}
			item->goalAnimState = 3;
			break;

		case 12:
			if (!item->requiredAnimState && (item->touchBits & 0x774F) && info.ahead)
			{
				CreatureEffect(item, &wolfBite, DoBloodSplat);
				LaraItem->hitPoints -= 100;
				LaraItem->hitStatus = true;
				item->requiredAnimState = 9;
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, tilt);
}