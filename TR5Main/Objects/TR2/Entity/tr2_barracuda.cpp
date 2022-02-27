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
	auto* info = GetCreatureInfo(item);

	short angle = 0;
	short head = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 6)
		{
			item->AnimNumber = Objects[ID_BARRACUDA].animIndex + 6;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 6;
		}

		CreatureFloat(itemNumber);
		return;
	}
	else
	{
		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		GetCreatureMood(item, &aiInfo, TIMID);
		CreatureMood(item, &aiInfo, TIMID);

		angle = CreatureTurn(item, info->maximumTurn);

		switch (item->ActiveState)
		{
		case 1:
			info->flags = 0;

			if (info->mood == BORED_MOOD)
				item->TargetState = 2;
			else if (aiInfo.ahead && aiInfo.distance < 680)
				item->TargetState = 4;
			else if (info->mood == STALK_MOOD)
				item->TargetState = 2;
			else
				item->TargetState = 3;

			break;

		case 2:
			info->maximumTurn = ANGLE(2.0f);

			if (info->mood == BORED_MOOD)
				break;
			else if (aiInfo.ahead && (item->TouchBits & 0xE0))
				item->TargetState = 1;
			else if (info->mood != STALK_MOOD)
				item->TargetState = 3;

			break;

		case 3:
			info->maximumTurn = ANGLE(4.0f);
			info->flags = 0;

			if (info->mood == BORED_MOOD)
				item->TargetState = 2;
			else if (aiInfo.ahead && aiInfo.distance < 340)
				item->TargetState = 5;
			else if (aiInfo.ahead && aiInfo.distance < 680)
				item->TargetState = 1;
			else if (info->mood == STALK_MOOD)
				item->TargetState = 2;

			break;

		case 4:
		case 5:
			if (aiInfo.ahead)
				head = aiInfo.angle;

			if (!info->flags && (item->TouchBits & 0xE0))
			{
				CreatureEffect(item, &BarracudaBite, DoBloodSplat);
				info->flags = 1;

				LaraItem->HitPoints -= 100;
				LaraItem->HitStatus = true;
			}

			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNumber, angle, 0);
	CreatureUnderwater(item, CLICK(1));
}
