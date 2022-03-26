#include "framework.h"
#include "Objects/TR3/Entity/tr3_raptor.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

static BITE_INFO RaptorBite = { 0, 66, 318, 22 };

// TODO
enum RaptorState
{

};

// TODO
enum RaptorAnim
{

};

void RaptorControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short head = 0;
	short neck = 0;
	short angle = 0;
	short tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 5)
		{
			if (GetRandomControl() > 0x4000)
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 9;
			else
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 10;

			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
			item->Animation.ActiveState = 5;
		}
	}
	else
	{
		if (creature->Enemy == NULL || !(GetRandomControl() & 0x7F))
		{
			ITEM_INFO* nearestItem = NULL;
			int minDistance = MAXINT;

			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				auto* currentCreatureInfo = ActiveCreatures[i];
				if (currentCreatureInfo->ItemNumber == NO_ITEM || currentCreatureInfo->ItemNumber == itemNumber)
				{
					currentCreatureInfo++;
					continue;
				}

				auto* targetItem = &g_Level.Items[currentCreatureInfo->ItemNumber];

				int x = (targetItem->Position.xPos - item->Position.xPos) / 64;
				int y = (targetItem->Position.yPos - item->Position.yPos) / 64;
				int z = (targetItem->Position.zPos - item->Position.zPos) / 64;

				int distance = pow(x, 2) + pow(y, 2) + pow(z, 2);
				if (distance < minDistance && item->HitPoints > 0)
				{
					nearestItem = targetItem;
					minDistance = distance;
				}

				currentCreatureInfo++;
			}

			if (nearestItem != NULL && (nearestItem->ObjectNumber != ID_RAPTOR || (GetRandomControl() < 0x400 && minDistance < pow(SECTOR(2), 2))))
				creature->Enemy = nearestItem;

			int x = (LaraItem->Position.xPos - item->Position.xPos) / 64;
			int y = (LaraItem->Position.yPos - item->Position.yPos) / 64;
			int z = (LaraItem->Position.zPos - item->Position.zPos) / 64;

			int distance = pow(x, 2) + pow(y, 2) + pow(z, 2);
			if (distance <= minDistance)
				creature->Enemy = LaraItem;
		}

		if (item->AIBits)
			GetAITarget(creature);

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		if (AI.ahead)
			head = AI.angle;

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		if (creature->Mood == MoodType::Bored)
			creature->MaxTurn /= 2;

		angle = CreatureTurn(item, creature->MaxTurn);
		neck = -angle * 6;

		switch (item->Animation.ActiveState)
		{
		case 1:
			creature->MaxTurn = 0;
			creature->Flags &= ~1;

			if (item->Animation.RequiredState)
				item->Animation.TargetState = item->Animation.RequiredState;
			else if (creature->Flags & 2)
			{
				creature->Flags &= ~2;
				item->Animation.TargetState = 6;
			}
			else if (item->TouchBits & 0xFF7C00 || (AI.distance < pow(585, 2) && AI.bite))
				item->Animation.TargetState = 8;
			else if (AI.bite && AI.distance < pow(SECTOR(1.5f), 2))
				item->Animation.TargetState = 4;
			else if (creature->Mood == MoodType::Escape && Lara.TargetEntity != item && AI.ahead && !item->HitStatus)
				item->Animation.TargetState = 1;
			else if (creature->Mood == MoodType::Bored)
				item->Animation.TargetState = 2;
			else
				item->Animation.TargetState = 3;

			break;

		case 2:
			creature->MaxTurn = ANGLE(2.0f);
			creature->Flags &= ~1;

			if (creature->Mood != MoodType::Bored)
				item->Animation.TargetState = 1;
			else if (AI.ahead && GetRandomControl() < 0x80)
			{
				item->Animation.RequiredState = 6;
				item->Animation.TargetState = 1;
				creature->Flags &= ~2;
			}

			break;

		case 3:
			creature->MaxTurn = ANGLE(4.0f);
			creature->Flags &= ~1;
			tilt = angle;

			if (item->TouchBits & 0xFF7C00)
				item->Animation.TargetState = 1;
			else if (creature->Flags & 2)
			{
				item->Animation.RequiredState = 6;
				item->Animation.TargetState = 1;
				creature->Flags &= ~2;
			}
			else if (AI.bite && AI.distance < pow(SECTOR(1.5f), 2))
			{
				if (item->Animation.TargetState == 3)
				{
					if (GetRandomControl() < 0x2000)
						item->Animation.TargetState = 1;
					else
						item->Animation.TargetState = 7;
				}
			}
			else if (AI.ahead && creature->Mood != MoodType::Escape && GetRandomControl() < 0x80)
			{
				item->Animation.RequiredState = 6;
				item->Animation.TargetState = 1;
			}
			else if (creature->Mood == MoodType::Bored || (creature->Mood == MoodType::Escape && Lara.TargetEntity != item && AI.ahead))
				item->Animation.TargetState = 1;

			break;

		case 4:
			creature->MaxTurn = ANGLE(2.0f);
			tilt = angle;

			if (creature->Enemy == LaraItem)
			{
				if (!(creature->Flags & 1) && (item->TouchBits & 0xFF7C00))
				{
					creature->Flags |= 1;
					CreatureEffect(item, &RaptorBite, DoBloodSplat);

					if (LaraItem->HitPoints <= 0)
						creature->Flags |= 2;

					LaraItem->HitPoints -= 100;
					LaraItem->HitStatus = 1;

					item->Animation.RequiredState = 1;
				}
			}
			else
			{
				if (!(creature->Flags & 1) && creature->Enemy)
				{
					if (abs(creature->Enemy->Position.xPos - item->Position.xPos) < CLICK(2) &&
						abs(creature->Enemy->Position.yPos - item->Position.yPos) < CLICK(2) &&
						abs(creature->Enemy->Position.zPos - item->Position.zPos) < CLICK(2))
					{
						creature->Enemy->HitPoints -= 25;
						creature->Enemy->HitStatus = 1;

						if (creature->Enemy->HitPoints <= 0)
							creature->Flags |= 2;

						creature->Flags |= 1;
						CreatureEffect(item, &RaptorBite, DoBloodSplat);

					}
				}
			}

			break;

		case 8:
			creature->MaxTurn = ANGLE(2.0f);
			tilt = angle;

			if (creature->Enemy == LaraItem)
			{
				if (!(creature->Flags & 1) && (item->TouchBits & 0xFF7C00))
				{
					creature->Flags |= 1;
					CreatureEffect(item, &RaptorBite, DoBloodSplat);

					if (LaraItem->HitPoints <= 0)
						creature->Flags |= 2;

					LaraItem->HitPoints -= 100;
					LaraItem->HitStatus = 1;

					item->Animation.RequiredState = 1;
				}
			}
			else
			{
				if (!(creature->Flags & 1) && creature->Enemy)
				{
					if (abs(creature->Enemy->Position.xPos - item->Position.xPos) < 512 &&
						abs(creature->Enemy->Position.yPos - item->Position.yPos) < 512 &&
						abs(creature->Enemy->Position.zPos - item->Position.zPos) < 512)
					{
						creature->Enemy->HitPoints -= 25;
						creature->Enemy->HitStatus = 1;

						if (creature->Enemy->HitPoints <= 0)
							creature->Flags |= 2;

						creature->Flags |= 1;
						CreatureEffect(item, &RaptorBite, DoBloodSplat);

					}
				}
			}

			break;

		case 7:
			creature->MaxTurn = ANGLE(2.0f);
			tilt = angle;

			if (creature->Enemy == LaraItem)
			{
				if (!(creature->Flags & 1) && item->TouchBits & 0xFF7C00)
				{
					creature->Flags |= 1;
					CreatureEffect(item, &RaptorBite, DoBloodSplat);

					LaraItem->HitPoints -= 100;
					LaraItem->HitStatus = true;

					if (LaraItem->HitPoints <= 0)
						creature->Flags |= 2;

					item->Animation.RequiredState = 3;
				}
			}
			else
			{
				if (!(creature->Flags & 1) && creature->Enemy)
				{
					if (abs(creature->Enemy->Position.xPos - item->Position.xPos) < 512 &&
						abs(creature->Enemy->Position.yPos - item->Position.yPos) < 512 &&
						abs(creature->Enemy->Position.zPos - item->Position.zPos) < 512)
					{
						creature->Enemy->HitPoints -= 25;
						creature->Enemy->HitStatus = 1;

						if (creature->Enemy->HitPoints <= 0)
							creature->Flags |= 2;

						creature->Flags |= 1;
						CreatureEffect(item, &RaptorBite, DoBloodSplat);

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
	CreatureAnimation(itemNumber, angle, tilt);
}
