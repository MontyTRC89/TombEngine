#include "framework.h"
#include "Objects/TR2/Entity/tr2_eagle_or_crow.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO EagleBite = { 15, 46, 21, 6 };
BITE_INFO CrowBite = { 2, 10, 60, 14 };

// TODO
enum EagleState
{

};

// TODO
enum EagleAnim
{

};

void InitialiseEagle(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	if (item->ObjectNumber == ID_CROW)
	{
		item->Animation.AnimNumber = Objects[ID_CROW].animIndex + 14;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = item->Animation.TargetState = 7;
	}
	else
	{
		item->Animation.AnimNumber = Objects[ID_EAGLE].animIndex + 5;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = item->Animation.TargetState = 2;
	}
}

void EagleControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short angle = 0;

	if (item->HitPoints <= 0)
	{
		switch (item->Animation.ActiveState)
		{
		case 4:
			if (item->Pose.Position.y > item->Floor)
			{
				item->Pose.Position.y = item->Floor;
				item->Animation.VerticalVelocity = 0;
				item->Animation.IsAirborne = false;
				item->Animation.TargetState = 5;
			}

			break;

		case 5:
			item->Pose.Position.y = item->Floor;
			break;

		default:
			if (item->ObjectNumber == ID_CROW)
				item->Animation.AnimNumber = Objects[ID_CROW].animIndex + 1;
			else
				item->Animation.AnimNumber = Objects[ID_EAGLE].animIndex + 8;

			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 4;
			item->Animation.Velocity = 0;
			item->Animation.IsAirborne = true;
			break;
		}
		item->Pose.Orientation.x = 0;
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, TIMID);

		angle = CreatureTurn(item, ANGLE(3.0f));

		switch (item->Animation.ActiveState)
		{
		case 7:
			item->Pose.Position.y = item->Floor;

			if (creature->Mood != MoodType::Bored)
				item->Animation.TargetState = 1;

			break;

		case 2:
			item->Pose.Position.y = item->Floor;

			if (creature->Mood == MoodType::Bored)
				break;
			else
				item->Animation.TargetState = 1;

			break;

		case 1:
			creature->Flags = 0;

			if (item->Animation.RequiredState)
				item->Animation.TargetState = item->Animation.RequiredState;
			if (creature->Mood == MoodType::Bored)
				item->Animation.TargetState = 2;
			else if (AI.ahead && AI.distance < pow(SECTOR(0.5f), 2))
				item->Animation.TargetState = 6;
			else
				item->Animation.TargetState = 3;

			break;

		case 3:
			if (creature->Mood == MoodType::Bored)
			{
				item->Animation.RequiredState = 2;
				item->Animation.TargetState = 1;
			}
			else if (AI.ahead && AI.distance < pow(SECTOR(0.5f), 2))
				item->Animation.TargetState = 6;

			break;

		case 6:
			if (!creature->Flags && item->TouchBits)
			{
				DoDamage(creature->Enemy, 20);

				if (item->ObjectNumber == ID_CROW)
					CreatureEffect(item, &CrowBite, DoBloodSplat);
				else
					CreatureEffect(item, &EagleBite, DoBloodSplat);

				creature->Flags = 1;
			}

			break;
		}
	}

	CreatureAnimation(itemNumber, angle, 0);
}
