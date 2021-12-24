#include "framework.h"
#include "Objects/TR2/Entity/tr2_birdmonster.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO birdyBiteL = { 0, 224, 0, 19 };
BITE_INFO birdyBiteR = { 0, 224, 0, 22 };

void BirdMonsterControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* monster;
	AI_INFO info;
	short angle, head;

	item = &g_Level.Items[itemNum];
	monster = (CREATURE_INFO*)item->data;
	angle = head = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 9)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 20;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 9;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, monster->maximumTurn);

		switch (item->currentAnimState)
		{
		case 1:
			monster->maximumTurn = 0;

			if (info.ahead && info.distance < SQUARE(WALL_SIZE))
			{
				if (GetRandomControl() < 0x4000)
					item->goalAnimState = 3;
				else
					item->goalAnimState = 10;
			}
			else if (info.ahead && (monster->mood == BORED_MOOD || monster->mood == STALK_MOOD))
			{
				if (info.zoneNumber != info.enemyZone)
				{
					item->goalAnimState = 2;
					monster->mood = ESCAPE_MOOD;
				}
				else
					item->goalAnimState = 8;
			}
			else
			{
				item->goalAnimState = 2;
			}
			break;
		case 8:
			monster->maximumTurn = 0;

			if (monster->mood != BORED_MOOD || !info.ahead)
				item->goalAnimState = 1;
			break;
		case 2:
			monster->maximumTurn = ANGLE(4);

			if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
				item->goalAnimState = 5;
			else if ((monster->mood == BORED_MOOD || monster->mood == STALK_MOOD) && info.ahead)
				item->goalAnimState = 1;
			break;
		case 3:
			monster->flags = 0;

			if (info.ahead && info.distance < SQUARE(WALL_SIZE))
				item->goalAnimState = 4;
			else
				item->goalAnimState = 1;
			break;
		case 5:
			monster->flags = 0;

			if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
				item->goalAnimState = 6;
			else
				item->goalAnimState = 1;
			break;
		case 10:
			monster->flags = 0;

			if (info.ahead && info.distance < SQUARE(WALL_SIZE))
				item->goalAnimState = 11;
			else
				item->goalAnimState = 1;
			break;
		case 4:
		case 6:
		case 11:
		case 7:
			if (!(monster->flags & 1) && (item->touchBits & 0x600000))
			{
				CreatureEffect(item, &birdyBiteR, DoBloodSplat);
				LaraItem->hitPoints -= 200;
				LaraItem->hitStatus = true;
				monster->flags |= 1;
			}

			if (!(monster->flags & 2) && (item->touchBits & 0x0C0000))
			{
				CreatureEffect(item, &birdyBiteL, DoBloodSplat);
				LaraItem->hitPoints -= 200;
				LaraItem->hitStatus = true;
				monster->flags |= 2;
			}
			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, 0);
}