#include "../newobjects.h"
#include "../../Game/Box.h"
#include "../../Game/effects.h"
#include "../../Game/items.h"
#include "../../Specific/setup.h"
#include "../../Game/lot.h"

BITE_INFO batBite = { 0, 16, 45, 4 };

void InitialiseBat(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[ID_BAT].animIndex + 5;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 6;
	item->currentAnimState = 6;
}

void BatControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	short angle = 0;
	short head = 0;
	short neck = 0;
	short tilt = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short joint3 = 0;

	if (item->hitPoints > 0)
	{
		int dx = LaraItem->pos.xPos - item->pos.xPos;
		int dz = LaraItem->pos.zPos - item->pos.zPos;
		int laraDistance = dx * dx + dz * dz;

		if (item->aiBits)
		{
			GetAITarget(creature);
		}
		else
		{
			creature->enemy = LaraItem;

			CREATURE_INFO* baddie = &BaddieSlots[0];
			CREATURE_INFO* found = &BaddieSlots[0];
			int minDistance = 0x7FFFFFFF;

			for (int i = 0; i < NUM_SLOTS; i++, baddie++)
			{
				if (baddie->itemNum == NO_ITEM || baddie->itemNum == itemNum)
					continue;

				ITEM_INFO* target = &Items[baddie->itemNum];
				if (target->objectNumber != ID_WILD_BOAR)
				{
					int dx2 = target->pos.xPos - item->pos.xPos;
					int dz2 = target->pos.zPos - item->pos.zPos;
					int distance = dx2 * dx2 + dz2 * dz2;

					if (distance < minDistance)
					{
						creature->enemy = target;
						minDistance = distance;
					}
				}
			}
		}

		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, TIMID);
		//if (item->flags)
		//	creature->mood = ESCAPE_MOOD;
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, ANGLE(20));

		switch (item->currentAnimState)
		{
		case 2:
			if (info.distance < SQUARE(256) || !(GetRandomControl() & 0x3F))
			{
				creature->flags = 0;
			}

			if (!creature->flags)
			{
				if (item->touchBits || creature->enemy != LaraItem && info.distance < SQUARE(256) && info.ahead && abs(item->pos.yPos - creature->enemy->pos.yPos) < 896)
				{
					item->goalAnimState = 3;
				}
			}
			break;

		case 3:
			if (!creature->flags && (item->touchBits || creature->enemy != LaraItem && info.distance < SQUARE(256) && info.ahead && abs(item->pos.yPos - creature->enemy->pos.yPos) < 896))
			{
				CreatureEffect(item, &batBite, DoBloodSplat);
				if (creature->enemy == LaraItem)
				{
					LaraItem->hitPoints -= 2;
					LaraItem->hitStatus = true;
				}
				creature->flags = 1;
			}
			else
			{
				item->goalAnimState = 2;
				creature->mood = BORED_MOOD;
			}
			break;

		case 6:
			if (info.distance < SQUARE(5120) ||
				item->hitStatus ||
				creature->hurtByLara)
			{
				item->goalAnimState = 1;
			}
			break;

		}
	}
	else if (item->currentAnimState == 3)
	{
		item->animNumber = Objects[ID_BAT].animIndex + 1;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->goalAnimState = 2;
		item->currentAnimState = 2;
	}
	else
	{
		if (item->pos.yPos >= item->floor)
		{
			item->goalAnimState = 5;
			item->pos.yPos = item->floor;
			item->gravityStatus = false;
		}
		else
		{
			item->gravityStatus = true;
			item->animNumber = Objects[ID_BAT].animIndex + 3;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->goalAnimState = 4;
			item->currentAnimState = 4;
			item->speed = 0;
		}
	}

	CreatureAnimation(itemNum, angle, 0);
}