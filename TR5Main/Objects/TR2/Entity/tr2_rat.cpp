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
		if (item->ActiveState != 6)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 9;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 6;
		}
	}
	else
	{
		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		if (aiInfo.ahead)
			head = aiInfo.angle;

		GetCreatureMood(item, &aiInfo, TIMID);
		CreatureMood(item, &aiInfo, TIMID);

		angle = CreatureTurn(item, ANGLE(6.0f));

		switch (item->ActiveState)
		{
		case 4:
			if (info->mood == MoodType::Bored || info->mood == MoodType::Stalk)
			{
				short random = (short)GetRandomControl();
				if (random < 0x500)
					item->RequiredState = 3;
				else if (random > 0xA00)
					item->RequiredState = 1;
			}
			else if (aiInfo.distance < pow(340, 2))
				item->RequiredState = 5;
			else
				item->RequiredState = 1;

			if (item->RequiredState)
				item->TargetState = 2;

			break;

		case 2:
			info->maximumTurn = 0;

			if (item->RequiredState)
				item->TargetState = item->RequiredState;

			break;

		case 1:
			info->maximumTurn = ANGLE(6.0f);

			if (info->mood == MoodType::Bored || info->mood == MoodType::Stalk)
			{
				random = (short)GetRandomControl();
				if (random < 0x500)
				{
					item->RequiredState = 3;
					item->TargetState = 2;
				}
				else if (random < 0xA00)
					item->TargetState = 2;
			}
			else if (aiInfo.ahead && aiInfo.distance < pow(340, 2))
				item->TargetState = 2;

			break;

		case 5:
			if (!item->RequiredState && item->TouchBits & 0x7F)
			{
				CreatureEffect(item, &RatBite, DoBloodSplat);
				item->RequiredState = 2;

				LaraItem->HitPoints -= 20;
				LaraItem->HitStatus = true;
			}

			break;

		case 3:
			if (GetRandomControl() < 0x500)
				item->TargetState = 2;

			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNumber, angle, 0);
}
