#include "framework.h"
#include "tr4_smallscorpion.h"
#include "box.h"
#include "items.h"
#include "effect.h"
#include "setup.h"
#include "level.h"
#include "lara.h"

BITE_INFO smallScorpionBiteInfo1 = { 0, 0, 0, 0 };
BITE_INFO smallScorpionBiteInfo2 = { 0, 0, 0, 23 };

void InitialiseSmallScorpion(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->animNumber = Objects[ID_SMALL_SCORPION].animIndex + 2;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
}

void SmallScorpionControl(short itemNumber)
{
	short angle = 0;
	short head = 0;
	short neck = 0;
	short tilt = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short joint3 = 0;

	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	if (item->hitPoints > 0)
	{
		int dx = LaraItem->pos.xPos - item->pos.xPos;
		int dz = LaraItem->pos.zPos - item->pos.zPos;
		int laraDistance = dx * dx + dz * dz;

		if (item->aiBits & GUARD)
			GetAITarget(creature);
		else
			creature->enemy = LaraItem;

		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		switch (item->currentAnimState)
		{
		case 1:
			creature->maximumTurn = 0;
			creature->flags = 0;
			if (info.distance > 116281)
			{
				item->goalAnimState = 2;
			}
			else if (info.bite)
			{
				creature->maximumTurn = 1092;
				if (GetRandomControl() & 1 /*|| creature->enemy->objectNumber == 59 && creature->enemy->hitPoints <= 2*/)
				{
					item->goalAnimState = 4;
				}
				else
				{
					item->goalAnimState = 5;
				}
			}
			else if (!info.ahead)
			{
				item->goalAnimState = 2;
			}
			break;

		case 3:
			creature->maximumTurn = 1456;
			if (info.distance < 116281)
			{
				item->goalAnimState = 1;
			}
			break;

		case 2:
			creature->maximumTurn = 1092;
			if (info.distance >= 116281)
			{
				if (info.distance > 45369)
				{
					item->goalAnimState = 3;
				}
			}
			else
			{
				item->goalAnimState = 1;
			}
			break;

		case 4:
		case 5:
			creature->maximumTurn = 0;
			if (abs(info.angle) >= 1092)
			{
				if (info.angle >= 0)
				{
					item->pos.yRot += 1092;
				}
				else
				{
					item->pos.yRot -= 1092;
				}
			}
			else
			{
				item->pos.yRot += info.angle;
			}
			if (!creature->flags)
			{
				if (item->touchBits & 0x1B00100)
				{
					if (item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 20 &&
						item->frameNumber < g_Level.Anims[item->animNumber].frameBase + 32)
					{
						LaraItem->hitPoints -= 20;
						LaraItem->hitStatus = true;

						BITE_INFO* biteInfo;
						short rot;

						if (item->currentAnimState == 5)
						{
							rot = item->pos.yRot + -ANGLE(180);
							biteInfo = &smallScorpionBiteInfo1;
						}
						else
						{
							rot = item->pos.yRot + -ANGLE(180);
							biteInfo = &smallScorpionBiteInfo2;
						}
						CreatureEffect2(item, biteInfo, 3, rot, DoBloodSplat);
						creature->flags = 1;
					}
				}
			}
			break;
		}
	}
	else
	{
		item->hitPoints = 0;
		if (item->currentAnimState != 6 && item->currentAnimState != 7)
		{
			item->animNumber = Objects[ID_SMALL_SCORPION].animIndex + 5;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 6;
		}
	}

	CreatureAnimation(itemNumber, angle, 0);
}