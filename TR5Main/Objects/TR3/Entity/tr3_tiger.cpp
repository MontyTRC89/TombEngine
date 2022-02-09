#include "framework.h"
#include "Objects/TR3/Entity/tr3_tiger.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO tigerBite = { 19, -13, 3, 26 };

void TigerControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
	short head = 0;
	short angle = 0;
	short tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 9)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 11;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 9;
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, 1);

		if (creature->alerted && info.zoneNumber != info.enemyZone)
			creature->mood = ESCAPE_MOOD;

		CreatureMood(item, &info, 1);

		angle = CreatureTurn(item, creature->maximumTurn);

		switch (item->ActiveState)
		{
		case 1:
			creature->maximumTurn = 0;
			creature->flags = 0;

			if (creature->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead)
					item->TargetState = 1;
				else
					item->TargetState = 3;
			}
			else if (creature->mood == BORED_MOOD)
			{
				short random = GetRandomControl();
				if (random < 0x60)
					item->TargetState = 5;
				else if (random < 0x460);
				item->TargetState = 2;
			}
			else if (info.bite && info.distance < SQUARE(340))
				item->TargetState = 6;
			else if (info.bite && info.distance < SQUARE(1024))
			{
				creature->maximumTurn = ANGLE(3);
				item->TargetState = 8;
			}
			else if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (creature->mood != ATTACK_MOOD && GetRandomControl() < 0x60)
				item->TargetState = 5;
			else
				item->TargetState = 3;
			break;

		case 2:
			creature->maximumTurn = ANGLE(3);

			if (creature->mood == ESCAPE_MOOD || creature->mood == ATTACK_MOOD)
				item->TargetState = 3;
			else if (GetRandomControl() < 0x60)
			{
				item->TargetState = 1;
				item->RequiredState = 5;
			}
			break;

		case 3:
			creature->maximumTurn = ANGLE(6);

			if (creature->mood == BORED_MOOD)
				item->TargetState = 1;
			else if (creature->flags && info.ahead)
				item->TargetState = 1;
			else if (info.bite && info.distance < SQUARE(1536))
			{
				if (LaraItem->Velocity == 0)
					item->TargetState = 1;
				else
					item->TargetState = 7;
			}
			else if (creature->mood != ATTACK_MOOD && GetRandomControl() < 0x60)
			{
				item->RequiredState = 5;
				item->TargetState = 1;
			}
			else if (creature->mood == ESCAPE_MOOD && Lara.target != item && info.ahead)
				item->TargetState = 1;

			creature->flags = 0;
			break;

		case 6:
		case 7:
		case 8:
			if (!creature->flags && (item->TouchBits & 0x7FDC000))
			{
				LaraItem->HitStatus = true;
				LaraItem->HitPoints -= 90;
				CreatureEffect(item, &tigerBite, DoBloodSplat);
				creature->flags = 1;
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, tilt);
}