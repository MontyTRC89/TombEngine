#include "framework.h"
#include "tr4_big_beetle.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Specific/trmath.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Game/itemdata/creature_info.h"

namespace TEN::Entities::TR4
{
	BITE_INFO BitBeetleBite = { 0,0,0,12 };

	void InitialiseBigBeetle(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->animNumber = Objects[item->objectNumber].animIndex + 3;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = 1;
		item->currentAnimState = 1;
	}

	void BigBeetleControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

		short angle = 0;

		if (item->hitPoints <= 0)
		{
			if (item->currentAnimState != 6)
			{
				if (item->currentAnimState != 7)
				{
					if (item->currentAnimState == 8)
					{
						item->pos.xRot = 0;
						item->pos.yPos = item->floor;
					}
					else
					{
						item->animNumber = Objects[item->objectNumber].animIndex + 5;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->gravityStatus = true;
						item->currentAnimState = 6;
						item->speed = 0;
						item->pos.xRot = 0;
					}
				}
				else if (item->pos.yPos >= item->floor)
				{
					item->pos.yPos = item->floor;
					item->gravityStatus = false;
					item->fallspeed = 0;
					item->goalAnimState = 8;
				}
			}
			item->pos.xRot = 0;
		}
		else
		{
			AI_INFO info;
			CreatureAIInfo(item, &info);

			GetCreatureMood(item, &info, VIOLENT);
			if (creature->flags)
				creature->mood = ESCAPE_MOOD;
			CreatureMood(item, &info, VIOLENT);

			angle = CreatureTurn(item, creature->maximumTurn);

			if (info.distance > SQUARE(3072)
				|| !(GetRandomControl() & 0x7F)
				|| item->hitStatus)
			{
				creature->flags = 0;
			}

			switch (item->currentAnimState)
			{
			case 1:
				item->pos.yPos = item->floor;
				creature->maximumTurn = ANGLE(1);

				if (item->hitStatus
					|| info.distance < SQUARE(3072)
					|| creature->hurtByLara
					|| item->aiBits == MODIFY)
				{
					item->goalAnimState = 2;
				}

				break;

			case 3:
				creature->maximumTurn = ANGLE(7);

				if (item->requiredAnimState)
				{
					item->goalAnimState = item->requiredAnimState;
				}
				else if (info.ahead)
				{
					if (info.distance < SQUARE(256))
					{
						item->goalAnimState = 9;
					}
				}

				break;

			case 4u:
				creature->maximumTurn = ANGLE(7);

				if (info.ahead)
				{
					if (info.distance < SQUARE(256))
					{
						item->goalAnimState = 4;
					}
				}
				else if (info.distance < SQUARE(256))
				{
					item->goalAnimState = 9;
				}
				else
				{
					item->requiredAnimState = 3;
					item->goalAnimState = 9;
				}

				if (!creature->flags)
				{
					if (item->touchBits & 0x60)
					{
						LaraItem->hitPoints -= 50;
						LaraItem->hitStatus = true;
						CreatureEffect2(
							item,
							&BitBeetleBite,
							5,
							-1,
							DoBloodSplat);
						creature->flags = 1;
					}
				}

				break;

			case 5:
				creature->flags = 0;

				item->pos.yPos += 51;
				if (item->pos.yPos > item->floor)
					item->pos.yPos = item->floor;

				break;

			case 9u:
				creature->maximumTurn = ANGLE(7);

				if (item->requiredAnimState)
				{
					item->goalAnimState = item->requiredAnimState;
				}
				else if (!item->hitStatus
					&& GetRandomControl() >= 384
					&& item->aiBits != MODIFY
					&& (creature->mood && GetRandomControl() >= 128
						|| creature->hurtByLara
						|| item->aiBits == MODIFY))
				{
					if (info.ahead)
					{
						if (info.distance < SQUARE(256) && !creature->flags)
						{
							item->goalAnimState = 4;
						}
					}
				}
				else
				{
					item->goalAnimState = 3;
				}

				break;

			default:
				break;

			}
		}

		CreatureTilt(item, 2 * angle);
		CreatureAnimation(itemNumber, angle, angle);
	}
}
