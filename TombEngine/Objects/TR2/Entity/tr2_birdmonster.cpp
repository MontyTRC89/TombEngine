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
		if (item->Animation.ActiveState != 9)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 20;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 9;
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
		angle = CreatureTurn(item, creature->MaxTurn);

		switch (item->Animation.ActiveState)
		{
		case 1:
			creature->MaxTurn = 0;

			if (AI.ahead && AI.distance < pow(SECTOR(1), 2))
			{
				if (GetRandomControl() < 0x4000)
					item->Animation.TargetState = 3;
				else
					item->Animation.TargetState = 10;
			}
			else if (AI.ahead && (creature->Mood == MoodType::Bored || creature->Mood == MoodType::Stalk))
			{
				if (AI.zoneNumber != AI.enemyZone)
				{
					item->Animation.TargetState = 2;
					creature->Mood = MoodType::Escape;
				}
				else
					item->Animation.TargetState = 8;
			}
			else
				item->Animation.TargetState = 2;
			
			break;

		case 8:
			creature->MaxTurn = 0;

			if (creature->Mood != MoodType::Bored || !AI.ahead)
				item->Animation.TargetState = 1;
			
			break;

		case 2:
			creature->MaxTurn = ANGLE(4.0f);

			if (AI.ahead && AI.distance < pow(SECTOR(2), 2))
				item->Animation.TargetState = 5;
			else if ((creature->Mood == MoodType::Bored || creature->Mood == MoodType::Stalk) && AI.ahead)
				item->Animation.TargetState = 1;
			
			break;

		case 3:
			creature->Flags = 0;

			if (AI.ahead && AI.distance < pow(SECTOR(1), 2))
				item->Animation.TargetState = 4;
			else
				item->Animation.TargetState = 1;

			break;

		case 5:
			creature->Flags = 0;

			if (AI.ahead && AI.distance < pow(SECTOR(2), 2))
				item->Animation.TargetState = 6;
			else
				item->Animation.TargetState = 1;

			break;

		case 10:
			creature->Flags = 0;

			if (AI.ahead && AI.distance < pow(SECTOR(1), 2))
				item->Animation.TargetState = 11;
			else
				item->Animation.TargetState = 1;

			break;

		case 4:
		case 6:
		case 11:
		case 7:
			if (!(creature->Flags & 1) && item->TouchBits & 0x600000)
			{
				CreatureEffect(item, &BirdMonsterBiteRight, DoBloodSplat);
				DoDamage(creature->Enemy, 200);
				creature->Flags |= 1;
			}

			if (!(creature->Flags & 2) && item->TouchBits & 0x0C0000)
			{
				CreatureEffect(item, &BirdMonsterBiteLeft, DoBloodSplat);
				DoDamage(creature->Enemy, 200);
				creature->Flags |= 2;
			}

			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNumber, angle, 0);
}
