#include "framework.h"
#include "tr5_doberman.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"
#include "Game/items.h"

BITE_INFO DobermanBite = { 0, 0x1E, 0x8D, 0x14 };

void InitialiseDoberman(short itemNum)
{
    ITEM_INFO* item;

    item = &g_Level.Items[itemNum];
    if (item->triggerFlags)
    {
        item->currentAnimState = 5;
        item->animNumber = Objects[item->objectNumber].animIndex + 6;
		// TODO: item->flags2 ^= (item->flags2 ^ ((item->flags2 & 0xFE) + 2)) & 6;
    }
    else
    {
        item->currentAnimState = 6;
        item->animNumber = Objects[item->objectNumber].animIndex + 10;
    }
    item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
}

void DobermanControl(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		short angle = 0;
		short tilt = 0;
		short joint = 0;
		
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
		
		if (item->hitPoints > 0)
		{
			AI_INFO info;
			CreatureAIInfo(item,&info);

			if (info.ahead)
				joint = info.angle;

			GetCreatureMood(item,&info, TIMID);
			CreatureMood(item,&info, TIMID);

			angle = CreatureTurn(item, creature->maximumTurn);
		
			int random;

			switch (item->currentAnimState)
			{
			case 1:
				creature->maximumTurn = ANGLE(3);
				if (creature->mood)
				{
					item->goalAnimState = 2;
				}
				else
				{
					random = GetRandomControl();
					if (random < 768)
					{
						item->requiredAnimState = 4;
						item->goalAnimState = 3;
						break;
					}
					if (random < 1536)
					{
						item->requiredAnimState = 5;
						item->goalAnimState = 3;
						break;
					}
					if (random < 2816)
					{
						item->goalAnimState = 3;
						break;
					}
				}
				break;

			case 2:
				tilt = angle;
				creature->maximumTurn = ANGLE(6);
				if (!creature->mood)
				{
					item->goalAnimState = 3;
					break;
				}
				if (info.distance < SQUARE(768))
					item->goalAnimState = 8;
				break;

			case 3:
				creature->maximumTurn = 0;
				creature->flags = 0;
				if (creature->mood)
				{
					if (creature->mood != ESCAPE_MOOD 
						&& info.distance < SQUARE(341)
						&& info.ahead)
						item->goalAnimState = 7;
					else
						item->goalAnimState = 2;
				}
				else
				{
					if (item->requiredAnimState)
					{
						item->goalAnimState = item->requiredAnimState;
					}
					else
					{
						random = GetRandomControl();
						if (random >= 768)
						{
							if (random >= 1536)
							{
								if (random < 9728)
									item->goalAnimState = 1;
							}
							else
							{
								item->goalAnimState = 5;
							}
						}
						else
						{
							item->goalAnimState = 4;
						}
					}
				}
				break;

			case 4:
				if (creature->mood || GetRandomControl() < 1280)
				{
					item->goalAnimState = 3;
				}
				break;

			case 5:
				if (creature->mood || GetRandomControl() < 256)
				{
					item->goalAnimState = 3;
				}
				break;

			case 6:
				if (creature->mood || GetRandomControl() < 512)
				{
					item->goalAnimState = 3;
				}
				break;

			case 7:
				creature->maximumTurn = ANGLE(1) / 2;
				if (creature->flags != 1 
					&& info.ahead 
					&& item->touchBits & 0x122000)
				{
					CreatureEffect(item,&DobermanBite, DoBloodSplat);
					LaraItem->hitPoints -= 30;
					LaraItem->hitStatus = true;
					creature->flags = 1;
				}

				if (info.distance <= SQUARE(341) || info.distance >= SQUARE(682))
					item->goalAnimState = 3;
				else
					item->goalAnimState = 9;
				break;

			case 8:
				if (creature->flags != 2 && item->touchBits & 0x122000)
				{
					CreatureEffect(item,&DobermanBite, DoBloodSplat);
					LaraItem->hitPoints -= 80;
					LaraItem->hitStatus = true;
					creature->flags = 2;
				}
				if (info.distance >= SQUARE(341))
				{
					if (info.distance < SQUARE(682))
						item->goalAnimState = 9;
				}
				else
				{
					item->goalAnimState = 7;
				}
				break;
			case 9:
				creature->maximumTurn = ANGLE(6);
				if (creature->flags != 3 && item->touchBits & 0x122000)
				{
					CreatureEffect(item,&DobermanBite, DoBloodSplat);
					LaraItem->hitPoints -= 50;
					LaraItem->hitStatus = true;
					creature->flags = 3;
				}
				if (info.distance < SQUARE(341))
					item->goalAnimState = 7;
				break;
			default:
				break;
			}
		}
		else if (item->currentAnimState != 10)
		{
			item->animNumber = Objects[ID_DOG].animIndex + 13;
			item->currentAnimState = 10;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, 0);
		CreatureJoint(item, 1, joint);
		CreatureJoint(item, 2, 0);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
