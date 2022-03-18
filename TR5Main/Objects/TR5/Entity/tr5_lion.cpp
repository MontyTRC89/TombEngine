#include "framework.h"
#include "tr5_lion.h"

#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Game/control/box.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"

BITE_INFO LionBite1 = { -2, -10, 250, 21 };
BITE_INFO LionBite2 = { -2, -10, 132, 21 };

void InitialiseLion(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[item->objectNumber].animIndex;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
}

void LionControl(short itemNum)
{
	short joint0 = 0;
	short angle = 0;
	short tilt = 0;
	short joint1 = 0;

	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (CreatureActive(itemNum))
	{
		CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

		if (item->hitPoints <= 0)
		{
			item->hitPoints = 0;
			if (item->currentAnimState != 5)
			{
				item->animNumber = Objects[item->objectNumber].animIndex + (GetRandomControl() & 1) + 7;
				item->currentAnimState = 5;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			}
		}
		else
		{
			AI_INFO info;
			CreatureAIInfo(item, &info);

			if (info.ahead)
				joint1 = info.angle;

			GetCreatureMood(item, &info, VIOLENT);
			CreatureMood(item, &info, VIOLENT);

			angle = CreatureTurn(item, creature->maximumTurn);
			joint0 = -16 * angle;

			switch (item->currentAnimState)
			{
			case 1:
				creature->maximumTurn = 0;
				if (item->requiredAnimState)
				{
					item->goalAnimState = item->requiredAnimState;
					break;
				}
				if (!creature->mood)
				{
					if (!(GetRandomControl() & 0x3F))
						item->goalAnimState = 2;
					break;
				}
				if (info.ahead)
				{
					if (item->touchBits & 0x200048)
					{
						item->goalAnimState = 7;
						break;
					}
					if (info.distance < SQUARE(1024))
					{
						item->goalAnimState = 4;
						break;
					}
				}
				item->goalAnimState = 3;
				break;

			case 2:
				creature->maximumTurn = ANGLE(2);
				if (!creature->mood)
				{
					if (GetRandomControl() < 128)
					{
						item->requiredAnimState = 6;
						item->goalAnimState = 1;
					}
				}
				else
					item->goalAnimState = 1;
				break;

			case 3:
				creature->maximumTurn = ANGLE(5);
				tilt = angle;
				if (creature->mood)
				{
					if (info.ahead && info.distance < SQUARE(1024))
					{
						item->goalAnimState = 1;
					}
					else if (item->touchBits & 0x200048 && info.ahead)
					{
						item->goalAnimState = 1;
					}
					else if (creature->mood != ESCAPE_MOOD)
					{
						if (GetRandomControl() < 128)
						{
							item->requiredAnimState = 6;
							item->goalAnimState = 1;
						}
					}
				}
				else
				{
					item->goalAnimState = 1;
				}
				break;

			case 4:
				if (!item->requiredAnimState && item->touchBits & 0x200048)
				{
					LaraItem->hitPoints -= 200;
					LaraItem->hitStatus = true;
					CreatureEffect2(item, &LionBite1, 10, item->pos.yRot, DoBloodSplat);
					item->requiredAnimState = 1;
				}
				break;
			case 7:
				creature->maximumTurn = ANGLE(1);
				if (!item->requiredAnimState && item->touchBits & 0x200048)
				{
					CreatureEffect2(item, &LionBite2, 10, item->pos.yRot, DoBloodSplat);
					LaraItem->hitPoints -= 60;
					LaraItem->hitStatus = true;
					item->requiredAnimState = 1;
				}
				break;
			}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureAnimation(itemNum, angle, 0);
}
