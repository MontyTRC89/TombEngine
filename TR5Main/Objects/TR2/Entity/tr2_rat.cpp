#include "framework.h"
#include "Objects/TR2/Entity/tr2_rat.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO ratBite = { 0, 0, 57, 2 };

void RatControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
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
		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, ANGLE(6));

		switch (item->ActiveState)
		{
		case 4:
			if (creature->mood == BORED_MOOD || creature->mood == STALK_MOOD)
			{
				short random = (short)GetRandomControl();
				if (random < 0x500)
					item->RequiredState = 3;
				else if (random > 0xA00)
					item->RequiredState = 1;
			}
			else if (info.distance < SQUARE(340))
				item->RequiredState = 5;
			else
				item->RequiredState = 1;

			if (item->RequiredState)
				item->TargetState = 2;
			break;

		case 2:
			creature->maximumTurn = 0;

			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			break;

		case 1:
			creature->maximumTurn = ANGLE(6);

			if (creature->mood == BORED_MOOD || creature->mood == STALK_MOOD)
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
			else if (info.ahead && info.distance < SQUARE(340))
				item->TargetState = 2;
			break;

		case 5:
			if (!item->RequiredState && (item->TouchBits & 0x7F))
			{
				CreatureEffect(item, &ratBite, DoBloodSplat);
				LaraItem->HitPoints -= 20;
				LaraItem->HitStatus = true;
				item->RequiredState = 2;
			}
			break;

		case 3:
			if (GetRandomControl() < 0x500)
				item->TargetState = 2;
			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, 0);
}
