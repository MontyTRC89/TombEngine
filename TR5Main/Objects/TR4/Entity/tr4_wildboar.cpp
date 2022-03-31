#include "framework.h"
#include "tr4_wildboar.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Game/control/lot.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"

BITE_INFO WildBoatBiteInfo = { 0, 0, 0, 14 };

void InitialiseWildBoar(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->Animation.AnimNumber = Objects[ID_WILD_BOAR].animIndex + 6;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = 1;
	item->Animation.ActiveState = 1;
}

void WildBoarControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

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
			GetAITarget(creature);
		else
		{
			creature->Enemy = LaraItem;

			int minDistance = 0x7FFFFFFF;

			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				auto* currentItem = ActiveCreatures[i];

				if (currentItem->ItemNumber == NO_ITEM || currentItem->ItemNumber == itemNumber)
					continue;

				auto* target = &g_Level.Items[currentItem->ItemNumber];
				if (target->ObjectNumber != ID_WILD_BOAR)
				{
					int dx2 = target->Position.xPos - item->Position.xPos;
					int dz2 = target->Position.zPos - item->Position.zPos;
					int distance = dx2 * dx2 + dz2 * dz2;

					if (distance < minDistance &&
						distance < laraDistance)
					{
						creature->Enemy = target;
						minDistance = distance;
					}
				}
			}
		}

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);

		if (item->Flags)
			creature->Mood = MoodType::Escape;

		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);

		if (AI.ahead)
		{
			joint1 = AI.angle / 2;
			joint3 = AI.angle / 2;
		}

		switch (item->Animation.ActiveState)
		{
		case 1:
			creature->MaxTurn = 0;

			if (AI.ahead && AI.distance || item->Flags)
				item->Animation.TargetState = 2;
			else if (GetRandomControl() & 0x7F)
			{
				joint1 = AIGuard(creature) / 2;
				joint3 = joint1;
			}
			else
				item->Animation.TargetState = 3;
			
			break;

		case 3:
			creature->MaxTurn = 0;

			if (AI.ahead && AI.distance)
				item->Animation.TargetState = 1;
			else if (!(GetRandomControl() & 0x7F))
				item->Animation.TargetState = 1;
			
			break;

		case 2:
			if (AI.distance >= 0x400000)
			{
				creature->MaxTurn = 1092;
				item->Flags = 0;
			}
			else
			{
				creature->MaxTurn = 546;
				joint0 = -AI.distance;
				joint2 = -AI.distance;
			}

			if (!item->Flags && (AI.distance < 0x10000 && AI.bite))
			{
				item->Animation.TargetState = 4;

				if (creature->Enemy == LaraItem)
				{
					creature->Enemy->HitPoints -= 30;
					creature->Enemy->HitStatus = true;
				}

				CreatureEffect2(item, &WildBoatBiteInfo, 3, item->Position.yRot, DoBloodSplat);
				item->Flags = 1;
			}

			break;

		case 4:
			creature->MaxTurn = 0;
			break;
		}
	}
	else
	{
		item->HitPoints = 0;

		if (item->Animation.ActiveState != 5)
		{
			item->Animation.AnimNumber = Objects[ID_WILD_BOAR].animIndex + 5;
			item->Animation.ActiveState = 5;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		}
	}

	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureJoint(item, 3, joint3);
	CreatureAnimation(itemNumber, angle, 0);
}
