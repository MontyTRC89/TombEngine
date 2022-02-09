#include "framework.h"
#include "Objects/TR3/Entity/tr3_raptor.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

static BITE_INFO raptorBite = { 0, 66, 318, 22 };

void RaptorControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	ITEM_INFO* nearestItem = NULL;
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
	short head = 0;
	short neck = 0;
	short angle = 0;
	short tilt = 0;
	INT minDistance = MAXINT;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 5)
		{
			if (GetRandomControl() > 0x4000)
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + 9;
			else
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + 10;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 5;
		}
	}
	else
	{
		if (creature->enemy == NULL || !(GetRandomControl() & 0x7F))
		{
			ITEM_INFO* target = NULL;
			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				CREATURE_INFO* currentCreature = ActiveCreatures[i];
				if (currentCreature->itemNum == NO_ITEM || currentCreature->itemNum == itemNum)
				{
					currentCreature++;
					continue;
				}

				target = &g_Level.Items[currentCreature->itemNum];

				int x = (target->Position.xPos - item->Position.xPos) / 64;
				int y = (target->Position.yPos - item->Position.yPos) / 64;
				int z = (target->Position.zPos - item->Position.zPos) / 64;
				int distance = x * x + y * y + z * z;
				if (distance < minDistance && item->HitPoints > 0)
				{
					nearestItem = target;
					minDistance = distance;
				}

				currentCreature++;
			}

			if (nearestItem != NULL && (nearestItem->ObjectNumber != ID_RAPTOR || (GetRandomControl() < 0x400 && minDistance < SQUARE(2048))))
				creature->enemy = nearestItem;

			int x = (LaraItem->Position.xPos - item->Position.xPos) / 64;
			int y = (LaraItem->Position.yPos - item->Position.yPos) / 64;
			int z = (LaraItem->Position.zPos - item->Position.zPos) / 64;
			int distance = x * x + y * y + z * z;
			if (distance <= minDistance)
				creature->enemy = LaraItem;
		}

		if (item->AIBits)
			GetAITarget(creature);

		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		if (creature->mood == BORED_MOOD)
			creature->maximumTurn /= 2;

		angle = CreatureTurn(item, creature->maximumTurn);
		neck = -(angle * 6);

		switch (item->ActiveState)
		{
		case 1:
			creature->maximumTurn = 0;
			creature->flags &= ~1;

			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (creature->flags & 2)
			{
				creature->flags &= ~2;
				item->TargetState = 6;
			}
			else if ((item->TouchBits & 0xFF7C00) || (info.distance < SQUARE(585) && info.bite))
				item->TargetState = 8;
			else if (info.bite && info.distance < SQUARE(1536))
				item->TargetState = 4;
			else if (creature->mood == ESCAPE_MOOD && Lara.target != item && info.ahead && !item->HitStatus)
				item->TargetState = 1;
			else if (creature->mood == BORED_MOOD)
				item->TargetState = 2;
			else
				item->TargetState = 3;
			break;

		case 2:
			creature->maximumTurn = ANGLE(2);
			creature->flags &= ~1;

			if (creature->mood != BORED_MOOD)
				item->TargetState = 1;
			else if (info.ahead && GetRandomControl() < 0x80)
			{
				item->RequiredState = 6;
				item->TargetState = 1;
				creature->flags &= ~2;
			}
			break;

		case 3:
			tilt = angle;
			creature->maximumTurn = ANGLE(4);
			creature->flags &= ~1;

			if (item->TouchBits & 0xFF7C00)
				item->TargetState = 1;
			else if (creature->flags & 2)
			{
				item->RequiredState = 6;
				item->TargetState = 1;
				creature->flags &= ~2;
			}
			else if (info.bite && info.distance < SQUARE(1536))
			{
				if (item->TargetState == 3)
				{
					if (GetRandomControl() < 0x2000)
						item->TargetState = 1;
					else
						item->TargetState = 7;
				}
			}
			else if (info.ahead && creature->mood != ESCAPE_MOOD && GetRandomControl() < 0x80)
			{
				item->RequiredState = 6;
				item->TargetState = 1;
			}
			else if (creature->mood == BORED_MOOD || (creature->mood == ESCAPE_MOOD && Lara.target != item && info.ahead))
				item->TargetState = 1;
			break;

		case 4:
			tilt = angle;
			creature->maximumTurn = ANGLE(2);
			if (creature->enemy == LaraItem)
			{
				if (!(creature->flags & 1) && (item->TouchBits & 0xFF7C00))
				{
					creature->flags |= 1;
					CreatureEffect(item, &raptorBite, DoBloodSplat);
					if (LaraItem->HitPoints <= 0)
						creature->flags |= 2;
					LaraItem->HitPoints -= 100;
					LaraItem->HitStatus = 1;
					item->RequiredState = 1;
				}
			}
			else
			{
				if (!(creature->flags & 1) && creature->enemy)
				{
					if (abs(creature->enemy->Position.xPos - item->Position.xPos) < 512 &&
						abs(creature->enemy->Position.yPos - item->Position.yPos) < 512 &&
						abs(creature->enemy->Position.zPos - item->Position.zPos) < 512)
					{
						creature->enemy->HitPoints -= 25;
						creature->enemy->HitStatus = 1;
						if (creature->enemy->HitPoints <= 0)
							creature->flags |= 2;
						creature->flags |= 1;
						CreatureEffect(item, &raptorBite, DoBloodSplat);

					}
				}
			}

			break;

		case 8:
			tilt = angle;
			creature->maximumTurn = ANGLE(2);
			if (creature->enemy == LaraItem)
			{
				if (!(creature->flags & 1) && (item->TouchBits & 0xFF7C00))
				{
					creature->flags |= 1;
					CreatureEffect(item, &raptorBite, DoBloodSplat);
					if (LaraItem->HitPoints <= 0)
						creature->flags |= 2;
					LaraItem->HitPoints -= 100;
					LaraItem->HitStatus = 1;

					item->RequiredState = 1;
				}
			}
			else
			{
				if (!(creature->flags & 1) && creature->enemy)
				{
					if (abs(creature->enemy->Position.xPos - item->Position.xPos) < 512 &&
						abs(creature->enemy->Position.yPos - item->Position.yPos) < 512 &&
						abs(creature->enemy->Position.zPos - item->Position.zPos) < 512)
					{
						creature->enemy->HitPoints -= 25;
						creature->enemy->HitStatus = 1;
						if (creature->enemy->HitPoints <= 0)
							creature->flags |= 2;
						creature->flags |= 1;
						CreatureEffect(item, &raptorBite, DoBloodSplat);

					}
				}
			}

			break;

		case 7:
			tilt = angle;
			creature->maximumTurn = ANGLE(2);
			if (creature->enemy == LaraItem)
			{
				if (!(creature->flags & 1) && (item->TouchBits & 0xFF7C00))
				{
					creature->flags |= 1;
					CreatureEffect(item, &raptorBite, DoBloodSplat);

					LaraItem->HitPoints -= 100;
					LaraItem->HitStatus = 1;
					if (LaraItem->HitPoints <= 0)
						creature->flags |= 2;
					item->RequiredState = 3;
				}
			}
			else
			{
				if (!(creature->flags & 1) && creature->enemy)
				{
					if (abs(creature->enemy->Position.xPos - item->Position.xPos) < 512 &&
						abs(creature->enemy->Position.yPos - item->Position.yPos) < 512 &&
						abs(creature->enemy->Position.zPos - item->Position.zPos) < 512)
					{
						creature->enemy->HitPoints -= 25;
						creature->enemy->HitStatus = 1;
						if (creature->enemy->HitPoints <= 0)
							creature->flags |= 2;
						creature->flags |= 1;
						CreatureEffect(item, &raptorBite, DoBloodSplat);

					}
				}
			}

			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head >> 1);
	CreatureJoint(item, 1, head >> 1);
	CreatureJoint(item, 2, neck);
	CreatureJoint(item, 3, neck);
	CreatureAnimation(itemNum, angle, tilt);
}
