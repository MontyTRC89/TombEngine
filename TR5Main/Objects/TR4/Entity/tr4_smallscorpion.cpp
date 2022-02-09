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

	item->AnimNumber = Objects[ID_SMALL_SCORPION].animIndex + 2;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->TargetState = SMALL_SCORPION_STOP;
	item->ActiveState = SMALL_SCORPION_STOP;
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
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;

	if (item->HitPoints <= 0)
	{
		item->HitPoints = 0;
		if (item->ActiveState != SMALL_SCORPION_DEATH1 && item->ActiveState != SMALL_SCORPION_DEATH2)
		{
			item->AnimNumber = Objects[ID_SMALL_SCORPION].animIndex + 5;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = SMALL_SCORPION_DEATH1;
		}
	}
	else
	{
		int dx = LaraItem->Position.xPos - item->Position.xPos;
		int dz = LaraItem->Position.zPos - item->Position.zPos;
		int laraDistance = dx * dx + dz * dz;

		if (item->AIBits & GUARD)
			GetAITarget(creature);
		else
			creature->enemy = LaraItem;

		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		switch (item->ActiveState)
		{
		case SMALL_SCORPION_STOP:
			creature->maximumTurn = 0;
			creature->flags = 0;
			if (info.distance > SQUARE(341))
			{
				item->TargetState = SMALL_SCORPION_WALK;
			}
			else if (info.bite)
			{
				creature->maximumTurn = ANGLE(6);
				if (GetRandomControl() & 1)
				{
					item->TargetState = SMALL_SCORPION_ATTACK1;
				}
				else
				{
					item->TargetState = SMALL_SCORPION_ATTACK2;
				}
			}
			else if (!info.ahead)
			{
				item->TargetState = SMALL_SCORPION_RUN;
			}
			break;

		case SMALL_SCORPION_WALK:
			creature->maximumTurn = ANGLE(6);
			if (info.distance >= SQUARE(341))
			{
				if (info.distance > SQUARE(213))
				{
					item->TargetState = SMALL_SCORPION_RUN;
				}
			}
			else
			{
				item->TargetState = SMALL_SCORPION_STOP;
			}
			break;

		case SMALL_SCORPION_RUN:
			creature->maximumTurn = ANGLE(8);
			if (info.distance < SQUARE(341))
			{
				item->TargetState = SMALL_SCORPION_STOP;
			}
			break;

		case SMALL_SCORPION_ATTACK1:
		case SMALL_SCORPION_ATTACK2:
			creature->maximumTurn = 0;
			if (abs(info.angle) >= ANGLE(6))
			{
				if (info.angle >= 0)
				{
					item->Position.yRot += ANGLE(6);
				}
				else
				{
					item->Position.yRot -= ANGLE(6);
				}
			}
			else
			{
				item->Position.yRot += info.angle;
			}
			if (!creature->flags)
			{
				if (item->TouchBits & 0x1B00100)
				{
					if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 20 &&
						item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 32)
					{
						Lara.poisoned += 512;
						LaraItem->HitPoints -= 20;
						LaraItem->HitStatus = true;

						BITE_INFO* biteInfo;
						short rot;

						if (item->ActiveState == SMALL_SCORPION_ATTACK1)
						{
							rot = item->Position.yRot + -ANGLE(180);
							biteInfo = &smallScorpionBiteInfo1;
						}
						else
						{
							rot = item->Position.yRot + -ANGLE(180);
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
