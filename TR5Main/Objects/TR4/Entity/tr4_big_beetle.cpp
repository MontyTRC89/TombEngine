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
		item->targetState = 1;
		item->activeState = 1;
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
			if (item->activeState != 6)
			{
				if (item->activeState != 7)
				{
					if (item->activeState == 8)
					{
						item->pos.xRot = 0;
						item->pos.yPos = item->floor;
					}
					else
					{
						item->animNumber = Objects[item->objectNumber].animIndex + 5;
						item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
						item->Airborne = true;
						item->activeState = 6;
						item->Velocity = 0;
						item->pos.xRot = 0;
					}
				}
				else if (item->pos.yPos >= item->floor)
				{
					item->pos.yPos = item->floor;
					item->Airborne = false;
					item->VerticalVelocity = 0;
					item->targetState = 8;
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

			switch (item->activeState)
			{
			case 1:
				item->pos.yPos = item->floor;
				creature->maximumTurn = ANGLE(1);

				if (item->hitStatus
					|| info.distance < SQUARE(3072)
					|| creature->hurtByLara
					|| item->aiBits == MODIFY)
				{
					item->targetState = 2;
				}

				break;

			case 3:
				creature->maximumTurn = ANGLE(7);

				if (item->requiredState)
				{
					item->targetState = item->requiredState;
				}
				else if (info.ahead)
				{
					if (info.distance < SQUARE(256))
					{
						item->targetState = 9;
					}
				}

				break;

			case 4u:
				creature->maximumTurn = ANGLE(7);

				if (info.ahead)
				{
					if (info.distance < SQUARE(256))
					{
						item->targetState = 4;
					}
				}
				else if (info.distance < SQUARE(256))
				{
					item->targetState = 9;
				}
				else
				{
					item->requiredState = 3;
					item->targetState = 9;
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

				if (item->requiredState)
				{
					item->targetState = item->requiredState;
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
							item->targetState = 4;
						}
					}
				}
				else
				{
					item->targetState = 3;
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
