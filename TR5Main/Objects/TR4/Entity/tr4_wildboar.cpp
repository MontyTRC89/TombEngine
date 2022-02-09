#include "framework.h"
#include "tr4_wildboar.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Game/control/lot.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"

BITE_INFO wildboardBiteInfo = { 0, 0, 0, 14 };

void InitialiseWildBoar(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->AnimNumber = Objects[ID_WILD_BOAR].animIndex + 6;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->TargetState = 1;
	item->ActiveState = 1;
}

void WildBoarControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
	short angle = 0;
	short head = 0;
	short neck = 0;
	short tilt = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short joint3 = 0;

	if (item->HitPoints > 0)
	{
		int dx = LaraItem->Position.xPos - item->Position.xPos;
		int dz = LaraItem->Position.zPos - item->Position.zPos;
		int laraDistance = dx * dx + dz * dz;

		if (item->AIBits & GUARD)
		{
			GetAITarget(creature);
		}
		else
		{
			creature->enemy = LaraItem;

			int minDistance = 0x7FFFFFFF;

			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				CREATURE_INFO* baddie = ActiveCreatures[i];

				if (baddie->itemNum == NO_ITEM || baddie->itemNum == itemNumber)
					continue;

				ITEM_INFO* target = &g_Level.Items[baddie->itemNum];
				if (target->ObjectNumber != ID_WILD_BOAR)
				{
					int dx2 = target->Position.xPos - item->Position.xPos;
					int dz2 = target->Position.zPos - item->Position.zPos;
					int distance = dx2 * dx2 + dz2 * dz2;

					if (distance < minDistance && distance < laraDistance)
					{
						creature->enemy = target;
						minDistance = distance;
					}
				}
			}
		}

		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		if (item->Flags)
			creature->mood = ESCAPE_MOOD;
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (info.ahead)
		{
			joint1 = info.angle / 2;
			joint3 = info.angle / 2;
		}

		switch (item->ActiveState)
		{
		case 1:
			creature->maximumTurn = 0;
			if (info.ahead && info.distance || item->Flags)
			{
				item->TargetState = 2;
			}
			else if (GetRandomControl() & 0x7F)
			{
				joint1 = AIGuard(creature) / 2;
				joint3 = joint1;
			}
			else
			{
				item->TargetState = 3;
			}
			break;

		case 3:
			creature->maximumTurn = 0;
			if (info.ahead && info.distance)
			{
				item->TargetState = 1;
			}
			else if (!(GetRandomControl() & 0x7F))
			{
				item->TargetState = 1;
			}
			break;

		case 2:
			if (info.distance >= 0x400000)
			{
				creature->maximumTurn = 1092;
				item->Flags = 0;
			}
			else
			{
				creature->maximumTurn = 546;
				joint0 = -info.distance;
				joint2 = -info.distance;
			}
			if (!item->Flags && (info.distance < 0x10000 && info.bite))
			{
				item->TargetState = 4;
				if (creature->enemy == LaraItem)
				{
					LaraItem->HitPoints -= 30;
					LaraItem->HitStatus = true;
				}

				CreatureEffect2(item, &wildboardBiteInfo, 3, item->Position.yRot, DoBloodSplat);
				item->Flags = 1;
			}
			break;

		case 4:
			creature->maximumTurn = 0;
			break;

		}
	}
	else
	{
		item->HitPoints = 0;
		if (item->ActiveState != 5)
		{
			item->AnimNumber = Objects[ID_WILD_BOAR].animIndex + 5;
			item->ActiveState = 5;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		}
	}

	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureJoint(item, 3, joint3);
	CreatureAnimation(itemNumber, angle, 0);
}
