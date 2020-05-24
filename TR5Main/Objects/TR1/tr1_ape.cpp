#include "framework.h"
#include "newobjects.h"
#include "box.h"
#include "effect.h"
#include "setup.h"
#include "level.h"
#include "lara.h"

BITE_INFO apeBite = { 0, -19, 75, 15 };

void ApeControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	short head = 0;
	short angle = 0;
	short random = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 5)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 7 + (short)(GetRandomControl() / 0x4000);
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 5;
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

		if (item->hitStatus || info.distance < SQUARE(2048))
			creature->flags |= 1;

		switch (item->currentAnimState)
		{
		case 1:
			if (creature->flags & 2)
			{
				item->pos.yRot -= ANGLE(90);
				creature->flags -= 2;
			}
			else if (item->flags & 4)
			{
				item->pos.yRot += ANGLE(90);
				creature->flags -= 4;
			}

			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else if (info.bite && info.distance < SQUARE(430))
				item->goalAnimState = 4;
			else if (!(creature->flags & 1) &&
				info.zoneNumber == info.enemyZone && info.ahead)
			{
				random = (short)(GetRandomControl() >> 5);
				if (random < 0xA0)
					item->goalAnimState = 10;
				else if (random < 0x140)
					item->goalAnimState = 6;
				else if (random < 0x1E0)
					item->goalAnimState = 7;
				else if (random < 0x2F0)
				{
					item->goalAnimState = 8;
					creature->maximumTurn = 0;
				}
				else
				{
					item->goalAnimState = 9;
					creature->maximumTurn = 0;
				}
			}
			else
				item->goalAnimState = 3;
			break;

		case 3:
			creature->maximumTurn = ANGLE(5);

			if (creature->flags == 0 && info.angle > -ANGLE(45) && info.angle < ANGLE(45))
				item->goalAnimState = 1;
			else if (info.ahead && (item->touchBits & 0xFF00))
			{
				item->requiredAnimState = 4;
				item->goalAnimState = 1;
			}
			else if (creature->mood != ESCAPE_MOOD)
			{
				random = (short)GetRandomControl();
				if (random < 0xA0)
				{
					item->requiredAnimState = 10;
					item->goalAnimState = 1;
				}
				else if (random < 0x140)
				{
					item->requiredAnimState = 6;
					item->goalAnimState = 1;
				}
				else if (random < 0x1E0)
				{
					item->requiredAnimState = 7;
					item->goalAnimState = 1;
				}
			}
			break;

		case 8:
			if (!(creature->flags & 4))
			{
				item->pos.yRot -= ANGLE(90);
				creature->flags |= 4;
			}

			item->goalAnimState = 1;
			break;

		case 9:
			if (!(creature->flags & 2))
			{
				item->pos.yRot += ANGLE(90);
				creature->flags |= 2;
			}

			item->goalAnimState = 1;
			break;

		case 4:
			if (!item->requiredAnimState && (item->touchBits & 0xFF00))
			{
				CreatureEffect(item, &apeBite, DoBloodSplat);

				LaraItem->hitPoints -= 200;
				LaraItem->hitStatus = true;

				item->requiredAnimState = 1;
			}
			break;
		}
	}

	CreatureJoint(item, 0, head);

	if (item->currentAnimState != 11)
	{
		if (creature->flags & 2)
		{
			item->pos.yRot -= ANGLE(90);
			creature->flags -= 2;
		}
		else if (item->flags & 4)
		{
			item->pos.yRot += ANGLE(90);
			creature->flags -= 4;
		}

		switch (CreatureVault(itemNum, angle, 2, 75))
		{
			case 2:
				creature->maximumTurn = 0;
				item->animNumber = Objects[item->objectNumber].animIndex + 19;
				item->currentAnimState = 11;
				item->frameNumber = Anims[item->animNumber].frameBase;
				break;

			default:
				return;
		}
	}
	else
		CreatureAnimation(itemNum, angle, 0);
}