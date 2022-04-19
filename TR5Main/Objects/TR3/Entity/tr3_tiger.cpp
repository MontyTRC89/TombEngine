#include "framework.h"
#include "Objects/TR3/Entity/tr3_tiger.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO tigerBite = { 19, -13, 3, 26 };

void TigerControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	short head = 0;
	short angle = 0;
	short tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 9)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 11;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 9;
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, 1);

		if (creature->alerted && info.zoneNumber != info.enemyZone)
			creature->mood = ESCAPE_MOOD;

		CreatureMood(item, &info, 1);

		angle = CreatureTurn(item, creature->maximumTurn);

		switch (item->currentAnimState)
		{
		case 1:
			creature->maximumTurn = 0;
			creature->flags = 0;

			if (creature->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 3;
			}
			else if (creature->mood == BORED_MOOD)
			{
				short random = GetRandomControl();
				if (random < 0x60)
					item->goalAnimState = 5;
				else if (random < 0x460);
				item->goalAnimState = 2;
			}
			else if (info.bite && info.distance < SQUARE(340))
				item->goalAnimState = 6;
			else if (info.bite && info.distance < SQUARE(1024))
			{
				creature->maximumTurn = ANGLE(3);
				item->goalAnimState = 8;
			}
			else if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else if (creature->mood != ATTACK_MOOD && GetRandomControl() < 0x60)
				item->goalAnimState = 5;
			else
				item->goalAnimState = 3;
			break;

		case 2:
			creature->maximumTurn = ANGLE(3);

			if (creature->mood == ESCAPE_MOOD || creature->mood == ATTACK_MOOD)
				item->goalAnimState = 3;
			else if (GetRandomControl() < 0x60)
			{
				item->goalAnimState = 1;
				item->requiredAnimState = 5;
			}
			break;

		case 3:
			creature->maximumTurn = ANGLE(6);

			if (creature->mood == BORED_MOOD)
				item->goalAnimState = 1;
			else if (creature->flags && info.ahead)
				item->goalAnimState = 1;
			else if (info.bite && info.distance < SQUARE(1536))
			{
				if (LaraItem->speed == 0)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 7;
			}
			else if (creature->mood != ATTACK_MOOD && GetRandomControl() < 0x60)
			{
				item->requiredAnimState = 5;
				item->goalAnimState = 1;
			}
			else if (creature->mood == ESCAPE_MOOD && Lara.target != item && info.ahead)
				item->goalAnimState = 1;

			creature->flags = 0;
			break;

		case 6:
		case 7:
		case 8:
			if (!creature->flags && (item->touchBits & 0x7FDC000))
			{
				LaraItem->hitStatus = true;
				LaraItem->hitPoints -= 90;
				CreatureEffect(item, &tigerBite, DoBloodSplat);
				creature->flags = 1;
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, tilt);
}