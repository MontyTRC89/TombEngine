#include "framework.h"
#include "tr5_dog.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/control/control.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"
#include "Game/items.h"

static BYTE DogAnims[] = { 20, 21, 22, 20 };
static BITE_INFO DogBite = { 0, 0, 100, 3 };

void InitialiseTr5Dog(short itemNum)
{
    ITEM_INFO* item;

    item = &g_Level.Items[itemNum];
    item->currentAnimState = 1;
    item->animNumber = Objects[item->objectNumber].animIndex + 8;
    if (!item->triggerFlags)
    {
        item->animNumber = Objects[item->objectNumber].animIndex + 1;
        // TODO: item->flags2 ^= (item->flags2 ^ ((item->flags2 & 0xFE) + 2)) & 6;
    }
    item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
}

void Tr5DogControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	short angle = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	if (item->hitPoints <= 0)
	{
		if (item->animNumber == obj->animIndex + 1)
		{
			item->hitPoints = obj->hitPoints;
		}
		else if (item->currentAnimState != 11)
		{
			item->animNumber = obj->animIndex + DogAnims[GetRandomControl() & 3];
			item->currentAnimState = 11;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
	else
	{
		if (item->aiBits)
			GetAITarget(creature);
		else
			creature->enemy = LaraItem;

		AI_INFO info;
		CreatureAIInfo(item,&info);

		int distance;
		if (creature->enemy == LaraItem)
		{
			distance = info.distance;
		}
		else
		{
			int dx = LaraItem->pos.xPos - item->pos.xPos;
			int dz = LaraItem->pos.zPos - item->pos.zPos;
			phd_atan(dz, dx);
			distance = SQUARE(dx) + SQUARE(dz);
		}

		if (info.ahead)
		{
			joint2 = info.xAngle; // Maybe swapped
			joint1 = info.angle;
		}

		GetCreatureMood(item,&info, VIOLENT);
		CreatureMood(item,&info, VIOLENT);

		if (!creature->mood)
			creature->maximumTurn /= 2;

		angle = CreatureTurn(item, creature->maximumTurn);
		joint0 = 4 * angle;


		if (creature->hurtByLara || distance < SQUARE(3072) && !(item->aiBits & MODIFY))
		{
			AlertAllGuards(itemNumber);
			item->aiBits &= ~MODIFY;
		}

		short random = GetRandomControl();
		int frame = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;

		switch (item->currentAnimState)
		{
		case 0:
		case 8:
			joint1 = 0;
			joint2 = 0;
			if (creature->mood && (item->aiBits) != MODIFY)
			{
				item->goalAnimState = 1;
			}
			else
			{
				creature->flags++;
				creature->maximumTurn = 0;
				if (creature->flags > 300 && random < 128)
					item->goalAnimState = 1;
			}
			break;

		case 1:
		case 9:
			if (item->currentAnimState == 9 && item->requiredAnimState)
			{
				item->goalAnimState = item->requiredAnimState;
				break;
			}

			creature->maximumTurn = 0;
			if (item->aiBits & GUARD)
			{
				joint1 = AIGuard(creature);
				if (GetRandomControl())
					break;
				if (item->currentAnimState == 1)
				{
					item->goalAnimState = 9;
					break;
				}
			}
			else
			{
				if (item->currentAnimState == 9 && random < 128)
				{
					item->goalAnimState = 1;
					break;
				}

				if (item->aiBits & PATROL1)
				{
					if (item->currentAnimState == 1)
						item->goalAnimState = 2;
					else
						item->goalAnimState = 1;
					break;
				}

				if (creature->mood == ESCAPE_MOOD)
				{
					if (Lara.target == item || !info.ahead || item->hitStatus)
					{
						item->requiredAnimState = 3;
						item->goalAnimState = 9;
					}
					else
					{
						item->goalAnimState = 1;
					}
					break;
				}

				if (creature->mood)
				{
					item->requiredAnimState = 3;
					if (item->currentAnimState == 1)
						item->goalAnimState = 9;
					break;
				}

				creature->flags = 0;
				creature->maximumTurn = ANGLE(1);

				if (random < 256)
				{
					if (item->aiBits & MODIFY)
					{
						if (item->currentAnimState == 1)
						{
							item->goalAnimState = 8;
							creature->flags = 0;
							break;
						}
					}
				}

				if (random >= 4096)
				{
					if (!(random & 0x1F))
						item->goalAnimState = 7;
					break;
				}

				if (item->currentAnimState == 1)
				{
					item->goalAnimState = 2;
					break;
				}
			}
			item->goalAnimState = 1;
			break;

		case 2:
			creature->maximumTurn = ANGLE(3);
			if (item->aiBits & PATROL1)
			{
				item->goalAnimState = 2;
				break;
			}

			if (!creature->mood && random < 256)
			{
				item->goalAnimState = 1;
				break;
			}
			item->goalAnimState = 5;
			break;

		case 3:
			creature->maximumTurn = ANGLE(6);
			if (creature->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead)
					item->goalAnimState = 9;
			}
			else if (creature->mood)
			{
				if (info.bite && info.distance < SQUARE(1024))
				{
					item->goalAnimState = 6;
				}
				else if (info.distance < SQUARE(1536))
				{
					item->requiredAnimState = 5;
					item->goalAnimState = 9;
				}
			}
			else
			{
				item->goalAnimState = 9;
			}
			break;

		case 5:
			creature->maximumTurn = ANGLE(3);
			if (creature->mood)
			{
				if (creature->mood == ESCAPE_MOOD)
				{
					item->goalAnimState = 3;
				}
				else if (info.bite && info.distance < SQUARE(341))
				{
					item->goalAnimState = 12;
					item->requiredAnimState = 5;
				}
				else if (info.distance > SQUARE(1536) || item->hitStatus)
				{
					item->goalAnimState = 3;
				}
			}
			else
			{
				item->goalAnimState = 9;
			}
			break;
		case 6:
			if (info.bite
				&& item->touchBits & 0x6648
				&& frame >= 4
				&& frame <= 14)
			{
				CreatureEffect2(item,&DogBite, 2, -1, DoBloodSplat);
				LaraItem->hitPoints -= 20;
				LaraItem->hitStatus = true;
			}
			item->goalAnimState = 3;
			break;

		case 7:
			joint1 = 0;
			joint2 = 0;
			break;

		case 12:
			if (info.bite
				&& item->touchBits & 0x48
				&& (frame >= 9
					&& frame <= 12
					|| frame >= 22
					&& frame <= 25))
			{
				CreatureEffect2(item,&DogBite, 2, -1, DoBloodSplat);
				LaraItem->hitPoints -= 10;
				LaraItem->hitStatus = true;
			}
			break;
		default:
			break;
		}
	}

	CreatureTilt(item, 0);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureAnimation(itemNumber, angle, 0);
}