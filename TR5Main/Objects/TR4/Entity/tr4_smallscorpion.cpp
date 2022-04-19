#include "framework.h"
#include "tr4_smallscorpion.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"

BITE_INFO smallScorpionBiteInfo1 = { 0, 0, 0, 0 };
BITE_INFO smallScorpionBiteInfo2 = { 0, 0, 0, 23 };

enum SMALL_SCORPION_STATES {
	SMALL_SCORPION_STOP = 1,
	SMALL_SCORPION_WALK = 2,
	SMALL_SCORPION_RUN = 3,
	SMALL_SCORPION_ATTACK1 = 4,
	SMALL_SCORPION_ATTACK2 = 5,
	SMALL_SCORPION_DEATH1 = 6,
	SMALL_SCORPION_DEATH2 = 7
};

void InitialiseSmallScorpion(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->animNumber = Objects[ID_SMALL_SCORPION].animIndex + 2;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->goalAnimState = SMALL_SCORPION_STOP;
	item->currentAnimState = SMALL_SCORPION_STOP;
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

	if (item->hitPoints <= 0)
	{
		item->hitPoints = 0;
		if (item->currentAnimState != SMALL_SCORPION_DEATH1 && item->currentAnimState != SMALL_SCORPION_DEATH2)
		{
			item->animNumber = Objects[ID_SMALL_SCORPION].animIndex + 5;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = SMALL_SCORPION_DEATH1;
		}
	}
	else
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
		case SMALL_SCORPION_STOP:
			creature->maximumTurn = 0;
			creature->flags = 0;
			if (info.distance > SQUARE(341))
			{
				item->goalAnimState = SMALL_SCORPION_WALK;
			}
			else if (info.bite)
			{
				creature->maximumTurn = ANGLE(6);
				if (GetRandomControl() & 1)
				{
					item->goalAnimState = SMALL_SCORPION_ATTACK1;
				}
				else
				{
					item->goalAnimState = SMALL_SCORPION_ATTACK2;
				}
			}
			else if (!info.ahead)
			{
				item->goalAnimState = SMALL_SCORPION_RUN;
			}
			break;

		case SMALL_SCORPION_WALK:
			creature->maximumTurn = ANGLE(6);
			if (info.distance >= SQUARE(341))
			{
				if (info.distance > SQUARE(213))
				{
					item->goalAnimState = SMALL_SCORPION_RUN;
				}
			}
			else
			{
				item->goalAnimState = SMALL_SCORPION_STOP;
			}
			break;

		case SMALL_SCORPION_RUN:
			creature->maximumTurn = ANGLE(8);
			if (info.distance < SQUARE(341))
			{
				item->goalAnimState = SMALL_SCORPION_STOP;
			}
			break;

		case SMALL_SCORPION_ATTACK1:
		case SMALL_SCORPION_ATTACK2:
			creature->maximumTurn = 0;
			if (abs(info.angle) >= ANGLE(6))
			{
				if (info.angle >= 0)
				{
					item->pos.yRot += ANGLE(6);
				}
				else
				{
					item->pos.yRot -= ANGLE(6);
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
						Lara.poisoned += 512;
						LaraItem->hitPoints -= 20;
						LaraItem->hitStatus = true;

						BITE_INFO* biteInfo;
						short rot;

						if (item->currentAnimState == SMALL_SCORPION_ATTACK1)
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

	CreatureAnimation(itemNumber, angle, 0);
}
