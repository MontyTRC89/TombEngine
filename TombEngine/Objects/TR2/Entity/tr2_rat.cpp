#include "framework.h"
#include "Objects/TR2/Entity/tr2_rat.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO RatBite = { 0, 0, 57, 2 };

// TODO
enum RatState
{

};

// TODO
enum RatAnim
{

};

void RatControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	short head = 0;
	short angle = 0;
	short random = 0;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 6)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 9;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 6;
		}
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		if (AI.ahead)
			head = AI.angle;

		GetCreatureMood(item, &AI, TIMID);
		CreatureMood(item, &AI, TIMID);

		angle = CreatureTurn(item, ANGLE(6.0f));

		switch (item->Animation.ActiveState)
		{
		case 4:
			if (info->Mood == MoodType::Bored || info->Mood == MoodType::Stalk)
			{
				short random = (short)GetRandomControl();
				if (random < 0x500)
					item->Animation.RequiredState = 3;
				else if (random > 0xA00)
					item->Animation.RequiredState = 1;
			}
			else if (AI.distance < pow(340, 2))
				item->Animation.RequiredState = 5;
			else
				item->Animation.RequiredState = 1;

			if (item->Animation.RequiredState)
				item->Animation.TargetState = 2;

			break;

		case 2:
			info->MaxTurn = 0;

			if (item->Animation.RequiredState)
				item->Animation.TargetState = item->Animation.RequiredState;

			break;

		case 1:
			info->MaxTurn = ANGLE(6.0f);

			if (info->Mood == MoodType::Bored || info->Mood == MoodType::Stalk)
			{
				random = (short)GetRandomControl();
				if (random < 0x500)
				{
					item->Animation.RequiredState = 3;
					item->Animation.TargetState = 2;
				}
				else if (random < 0xA00)
					item->Animation.TargetState = 2;
			}
			else if (AI.ahead && AI.distance < pow(340, 2))
				item->Animation.TargetState = 2;

			break;

		case 5:
			if (!item->Animation.RequiredState && item->TouchBits & 0x7F)
			{
				CreatureEffect(item, &RatBite, DoBloodSplat);
				DoDamage(info->Enemy, 20);
				item->Animation.RequiredState = 2;
			}

			break;

		case 3:
			if (GetRandomControl() < 0x500)
				item->Animation.TargetState = 2;

			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNumber, angle, 0);
}
