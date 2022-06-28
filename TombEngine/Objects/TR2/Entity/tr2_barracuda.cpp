#include "framework.h"
#include "Objects/TR2/Entity/tr2_barracuda.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO BarracudaBite = { 2, -60, 121, 7 };

// TODO
enum BarracudaState
{

};

// TODO
enum BarracudaAnim
{

};

void BarracudaControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short angle = 0;
	short head = 0;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 6)
		{
			item->Animation.AnimNumber = Objects[ID_BARRACUDA].animIndex + 6;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 6;
		}

		CreatureFloat(itemNumber);
		return;
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, TIMID);
		CreatureMood(item, &AI, TIMID);

		angle = CreatureTurn(item, creature->MaxTurn);

		switch (item->Animation.ActiveState)
		{
		case 1:
			creature->Flags = 0;

			if (creature->Mood == MoodType::Bored)
				item->Animation.TargetState = 2;
			else if (AI.ahead && AI.distance < pow(680, 2))
				item->Animation.TargetState = 4;
			else if (creature->Mood == MoodType::Stalk)
				item->Animation.TargetState = 2;
			else
				item->Animation.TargetState = 3;

			break;

		case 2:
			creature->MaxTurn = ANGLE(2.0f);

			if (creature->Mood == MoodType::Bored)
				break;
			else if (AI.ahead && (item->TouchBits & 0xE0))
				item->Animation.TargetState = 1;
			else if (creature->Mood != MoodType::Stalk)
				item->Animation.TargetState = 3;

			break;

		case 3:
			creature->MaxTurn = ANGLE(4.0f);
			creature->Flags = 0;

			if (creature->Mood == MoodType::Bored)
				item->Animation.TargetState = 2;
			else if (AI.ahead && AI.distance < pow(340, 2))
				item->Animation.TargetState = 5;
			else if (AI.ahead && AI.distance < pow(680, 2))
				item->Animation.TargetState = 1;
			else if (creature->Mood == MoodType::Stalk)
				item->Animation.TargetState = 2;

			break;

		case 4:
		case 5:
			if (AI.ahead)
				head = AI.angle;

			if (!creature->Flags && (item->TouchBits & 0xE0))
			{
				CreatureEffect(item, &BarracudaBite, DoBloodSplat);
				DoDamage(creature->Enemy, 100);
				creature->Flags = 1;
			}

			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNumber, angle, 0);
	CreatureUnderwater(item, CLICK(1));
}
