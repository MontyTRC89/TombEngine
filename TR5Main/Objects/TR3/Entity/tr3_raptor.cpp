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
	auto* info = GetCreatureInfo(item);

	short head = 0;
	short neck = 0;
	short angle = 0;
	short tilt = 0;

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
		if (info->enemy == NULL || !(GetRandomControl() & 0x7F))
		{
			ITEM_INFO* nearestItem = NULL;
			int minDistance = MAXINT;

			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				auto* currentCreatureInfo = ActiveCreatures[i];
				if (currentCreatureInfo->itemNum == NO_ITEM || currentCreatureInfo->itemNum == itemNumber)
				{
					currentCreatureInfo++;
					continue;
				}

				auto* targetItem = &g_Level.Items[currentCreatureInfo->itemNum];

				int x = (targetItem->Position.xPos - item->Position.xPos) / 64;
				int y = (targetItem->Position.yPos - item->Position.yPos) / 64;
				int z = (targetItem->Position.zPos - item->Position.zPos) / 64;

				int distance = x * x + y * y + z * z;
				if (distance < minDistance && item->HitPoints > 0)
				{
					nearestItem = targetItem;
					minDistance = distance;
				}

				currentCreatureInfo++;
			}

			if (nearestItem != NULL && (nearestItem->ObjectNumber != ID_RAPTOR || (GetRandomControl() < 0x400 && minDistance < pow(SECTOR(2), 2))))
				info->enemy = nearestItem;

			int x = (LaraItem->Position.xPos - item->Position.xPos) / 64;
			int y = (LaraItem->Position.yPos - item->Position.yPos) / 64;
			int z = (LaraItem->Position.zPos - item->Position.zPos) / 64;

			int distance = pow(x, 2) + pow(y, 2) + pow(z, 2);
			if (distance <= minDistance)
				info->enemy = LaraItem;
		}

		if (item->AIBits)
			GetAITarget(info);

		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		if (aiInfo.ahead)
			head = aiInfo.angle;

		GetCreatureMood(item, &aiInfo, VIOLENT);
		CreatureMood(item, &aiInfo, VIOLENT);

		if (info->mood == BORED_MOOD)
			info->maximumTurn /= 2;

		angle = CreatureTurn(item, info->maximumTurn);
		neck = -angle * 6;

		switch (item->ActiveState)
		{
		case 1:
			info->maximumTurn = 0;
			info->flags &= ~1;

			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (info->flags & 2)
			{
				info->flags &= ~2;
				item->TargetState = 6;
			}
			else if (item->TouchBits & 0xFF7C00 || (aiInfo.distance < pow(585, 2) && aiInfo.bite))
				item->TargetState = 8;
			else if (aiInfo.bite && aiInfo.distance < pow(SECTOR(1.5f), 2))
				item->TargetState = 4;
			else if (info->mood == ESCAPE_MOOD && Lara.target != item && aiInfo.ahead && !item->HitStatus)
				item->TargetState = 1;
			else if (info->mood == BORED_MOOD)
				item->TargetState = 2;
			else
				item->TargetState = 3;

			break;

		case 2:
			info->maximumTurn = ANGLE(2.0f);
			info->flags &= ~1;

			if (info->mood != BORED_MOOD)
				item->TargetState = 1;
			else if (aiInfo.ahead && GetRandomControl() < 0x80)
			{
				item->RequiredState = 6;
				item->TargetState = 1;
				info->flags &= ~2;
			}

			break;

		case 3:
			tilt = angle;
			info->maximumTurn = ANGLE(4.0f);
			info->flags &= ~1;

			if (item->TouchBits & 0xFF7C00)
				item->TargetState = 1;
			else if (info->flags & 2)
			{
				item->RequiredState = 6;
				item->TargetState = 1;
				info->flags &= ~2;
			}
			else if (aiInfo.bite && aiInfo.distance < SQUARE(1536))
			{
				if (item->TargetState == 3)
				{
					if (GetRandomControl() < 0x2000)
						item->TargetState = 1;
					else
						item->TargetState = 7;
				}
			}
			else if (aiInfo.ahead && info->mood != ESCAPE_MOOD && GetRandomControl() < 0x80)
			{
				item->RequiredState = 6;
				item->TargetState = 1;
			}
			else if (info->mood == BORED_MOOD || (info->mood == ESCAPE_MOOD && Lara.target != item && aiInfo.ahead))
				item->TargetState = 1;

			break;

		case 4:
			tilt = angle;
			info->maximumTurn = ANGLE(2.0f);

			if (info->enemy == LaraItem)
			{
				if (!(info->flags & 1) && (item->TouchBits & 0xFF7C00))
				{
					info->flags |= 1;
					CreatureEffect(item, &RaptorBite, DoBloodSplat);

					if (LaraItem->HitPoints <= 0)
						info->flags |= 2;

					LaraItem->HitPoints -= 100;
					LaraItem->HitStatus = 1;

					item->RequiredState = 1;
				}
			}
			else
			{
				if (!(info->flags & 1) && info->enemy)
				{
					if (abs(info->enemy->Position.xPos - item->Position.xPos) < CLICK(2) &&
						abs(info->enemy->Position.yPos - item->Position.yPos) < CLICK(2) &&
						abs(info->enemy->Position.zPos - item->Position.zPos) < CLICK(2))
					{
						info->enemy->HitPoints -= 25;
						info->enemy->HitStatus = 1;

						if (info->enemy->HitPoints <= 0)
							info->flags |= 2;

						info->flags |= 1;
						CreatureEffect(item, &RaptorBite, DoBloodSplat);

					}
				}
			}

			break;

		case 8:
			tilt = angle;
			info->maximumTurn = ANGLE(2.0f);

			if (info->enemy == LaraItem)
			{
				if (!(info->flags & 1) && (item->TouchBits & 0xFF7C00))
				{
					info->flags |= 1;
					CreatureEffect(item, &RaptorBite, DoBloodSplat);

					if (LaraItem->HitPoints <= 0)
						info->flags |= 2;

					LaraItem->HitPoints -= 100;
					LaraItem->HitStatus = 1;

					item->RequiredState = 1;
				}
			}
			else
			{
				if (!(info->flags & 1) && info->enemy)
				{
					if (abs(info->enemy->Position.xPos - item->Position.xPos) < 512 &&
						abs(info->enemy->Position.yPos - item->Position.yPos) < 512 &&
						abs(info->enemy->Position.zPos - item->Position.zPos) < 512)
					{
						info->enemy->HitPoints -= 25;
						info->enemy->HitStatus = 1;

						if (info->enemy->HitPoints <= 0)
							info->flags |= 2;

						info->flags |= 1;
						CreatureEffect(item, &RaptorBite, DoBloodSplat);

					}
				}
			}

			break;

		case 7:
			tilt = angle;
			info->maximumTurn = ANGLE(2.0f);
			if (info->enemy == LaraItem)
			{
				if (!(info->flags & 1) && item->TouchBits & 0xFF7C00)
				{
					info->flags |= 1;
					CreatureEffect(item, &RaptorBite, DoBloodSplat);

					LaraItem->HitPoints -= 100;
					LaraItem->HitStatus = true;

					if (LaraItem->HitPoints <= 0)
						info->flags |= 2;

					item->RequiredState = 3;
				}
			}
			else
			{
				if (!(info->flags & 1) && info->enemy)
				{
					if (abs(info->enemy->Position.xPos - item->Position.xPos) < 512 &&
						abs(info->enemy->Position.yPos - item->Position.yPos) < 512 &&
						abs(info->enemy->Position.zPos - item->Position.zPos) < 512)
					{
						info->enemy->HitPoints -= 25;
						info->enemy->HitStatus = 1;

						if (info->enemy->HitPoints <= 0)
							info->flags |= 2;

						info->flags |= 1;
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
