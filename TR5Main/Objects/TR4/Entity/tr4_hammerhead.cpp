#include "framework.h"
#include "tr4_hammerhead.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"
#include "Game/itemdata/creature_info.h"
#include "Game/misc.h"

BITE_INFO HammerheadBite = { 0, 0, 0, 12 };

enum HammerheadState
{
	HAMMERHEAD_STATE_IDLE = 0,
	HAMMERHEAD_STATE_SWIM_SLOW = 1,
	HAMMERHEAD_STATE_SWIM_FAST = 2,
	HAMMERHEAD_STATE_ATTACK = 3,
	HAMMERHEAD_STATE_DEATH = 5,
	HAMMERHEAD_STATE_KILL = 6
};

// TODO
enum HammerheadAnim
{

};

void InitialiseHammerhead(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 8;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = HAMMERHEAD_STATE_IDLE;
	item->Animation.ActiveState = HAMMERHEAD_STATE_IDLE;
}

void HammerheadControl(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		if (item->HitPoints > 0)
		{
			if (item->AIBits)
				GetAITarget(creature);
			else if (creature->HurtByLara)
				creature->Enemy = LaraItem;

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (creature->Enemy != LaraItem)
				phd_atan(LaraItem->Position.zPos - item->Position.zPos, LaraItem->Position.xPos - item->Position.xPos);

			GetCreatureMood(item, &AI, VIOLENT);
			CreatureMood(item, &AI, VIOLENT);

			short angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case HAMMERHEAD_STATE_IDLE:
				item->Animation.TargetState = HAMMERHEAD_STATE_SWIM_SLOW;
				creature->Flags = 0;
				break;

			case HAMMERHEAD_STATE_SWIM_SLOW:
				creature->MaxTurn = ANGLE(7.0f);

				if (AI.distance <= pow(SECTOR(1), 2))
				{
					if (AI.distance < pow(682, 2))
						item->Animation.TargetState = HAMMERHEAD_STATE_ATTACK;
				}
				else
					item->Animation.TargetState = HAMMERHEAD_STATE_SWIM_FAST;
				
				break;

			case HAMMERHEAD_STATE_SWIM_FAST:
				if (AI.distance < pow(SECTOR(1), 2))
					item->Animation.TargetState = HAMMERHEAD_STATE_SWIM_SLOW;
				
				break;

			case HAMMERHEAD_STATE_ATTACK:
				if (!creature->Flags)
				{
					if (item->TouchBits & 0x3400)
					{
						CreatureEffect(item, &HammerheadBite, DoBloodSplat);
						creature->Flags = 1;

						LaraItem->HitPoints -= 120;
						LaraItem->HitStatus = true;
					}
				}

				break;

			default:
				break;
			}

			CreatureTilt(item, 0);
			CreatureJoint(item, 0, -2 * angle);
			CreatureJoint(item, 1, -2 * angle);
			CreatureJoint(item, 2, -2 * angle);
			CreatureJoint(item, 3, 2 * angle);

			// NOTE: in TR2 shark there was a call to CreatureKill with special kill anim
			// Hammerhead seems to not have it in original code but this check is still there as a leftover
			if (item->Animation.ActiveState == HAMMERHEAD_STATE_KILL)
				AnimateItem(item);
			else
			{
				CreatureAnimation(itemNumber, angle, 0);
				CreatureUnderwater(item, 341);
			}
		}
		else
		{
			item->HitPoints = 0;

			if (item->Animation.ActiveState != HAMMERHEAD_STATE_DEATH)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 4;
				item->Animation.ActiveState = HAMMERHEAD_STATE_DEATH;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.FrameNumber].frameBase;
			}

			CreatureFloat(itemNumber);
		}
	}
}
