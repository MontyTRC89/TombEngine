#include "framework.h"
#include "Objects/TR2/Entity/tr2_birdmonster.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO BirdMonsterBiteLeft = { 0, 224, 0, 19 };
BITE_INFO BirdMonsterBiteRight = { 0, 224, 0, 22 };

// TODO
enum BirdMonsterState
{

};

// TODO
enum BirdMonsterAnim
{

};

void BirdMonsterControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short angle = 0;
	short head = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 9)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 20;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 9;
		}
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		if (AI.ahead)
			head = AI.angle;

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);
		angle = CreatureTurn(item, creature->maximumTurn);

		switch (item->ActiveState)
		{
		case 1:
			creature->maximumTurn = 0;

			if (AI.ahead && AI.distance < pow(SECTOR(1), 2))
			{
				if (GetRandomControl() < 0x4000)
					item->TargetState = 3;
				else
					item->TargetState = 10;
			}
			else if (AI.ahead && (creature->mood == BORED_MOOD || creature->mood == STALK_MOOD))
			{
				if (AI.zoneNumber != AI.enemyZone)
				{
					item->TargetState = 2;
					creature->mood = ESCAPE_MOOD;
				}
				else
					item->TargetState = 8;
			}
			else
				item->TargetState = 2;
			
			break;

		case 8:
			creature->maximumTurn = 0;

			if (creature->mood != BORED_MOOD || !AI.ahead)
				item->TargetState = 1;
			
			break;

		case 2:
			creature->maximumTurn = ANGLE(4.0f);

			if (AI.ahead && AI.distance < pow(SECTOR(2), 2))
				item->TargetState = 5;
			else if ((creature->mood == BORED_MOOD || creature->mood == STALK_MOOD) && AI.ahead)
				item->TargetState = 1;
			
			break;

		case 3:
			creature->flags = 0;

			if (AI.ahead && AI.distance < pow(SECTOR(1), 2))
				item->TargetState = 4;
			else
				item->TargetState = 1;

			break;

		case 5:
			creature->flags = 0;

			if (AI.ahead && AI.distance < pow(SECTOR(2), 2))
				item->TargetState = 6;
			else
				item->TargetState = 1;

			break;

		case 10:
			creature->flags = 0;

			if (AI.ahead && AI.distance < pow(SECTOR(1), 2))
				item->TargetState = 11;
			else
				item->TargetState = 1;

			break;

		case 4:
		case 6:
		case 11:
		case 7:
			if (!(creature->flags & 1) && item->TouchBits & 0x600000)
			{
				CreatureEffect(item, &BirdMonsterBiteRight, DoBloodSplat);
				creature->flags |= 1;

				LaraItem->HitPoints -= 200;
				LaraItem->HitStatus = true;
			}

			if (!(creature->flags & 2) && item->TouchBits & 0x0C0000)
			{
				CreatureEffect(item, &BirdMonsterBiteLeft, DoBloodSplat);
				creature->flags |= 2;

				LaraItem->HitPoints -= 200;
				LaraItem->HitStatus = true;
			}

			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNumber, angle, 0);
}
