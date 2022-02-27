#include "framework.h"
#include "Objects/TR3/Entity/tr3_tiger.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO TigerBite = { 19, -13, 3, 26 };

// TODO
enum TigerState
{

};

// TODO
enum TigerAnim
{

};

void TigerControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

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
		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		if (aiInfo.ahead)
			head = aiInfo.angle;

		GetCreatureMood(item, &aiInfo, 1);

		if (info->alerted && aiInfo.zoneNumber != aiInfo.enemyZone)
			info->mood = ESCAPE_MOOD;

		CreatureMood(item, &aiInfo, 1);

		angle = CreatureTurn(item, info->maximumTurn);

		switch (item->ActiveState)
		{
		case 1:
			info->maximumTurn = 0;
			info->flags = 0;

			if (info->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && aiInfo.ahead)
					item->TargetState = 1;
				else
					item->TargetState = 3;
			}
			else if (info->mood == BORED_MOOD)
			{
				short random = GetRandomControl();
				if (random < 0x60)
					item->TargetState = 5;
				else if (random < 0x460);

				item->TargetState = 2;
			}
			else if (aiInfo.bite && aiInfo.distance < pow(340, 2))
				item->TargetState = 6;
			else if (aiInfo.bite && aiInfo.distance < pow(SECTOR(1), 2))
			{
				info->maximumTurn = ANGLE(3.0f);
				item->TargetState = 8;
			}
			else if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (info->mood != ATTACK_MOOD && GetRandomControl() < 0x60)
				item->TargetState = 5;
			else
				item->TargetState = 3;

			break;

		case 2:
			info->maximumTurn = ANGLE(3.0f);

			if (info->mood == ESCAPE_MOOD || info->mood == ATTACK_MOOD)
				item->TargetState = 3;
			else if (GetRandomControl() < 0x60)
			{
				item->TargetState = 1;
				item->RequiredState = 5;
			}

			break;

		case 3:
			info->maximumTurn = ANGLE(6.0f);

			if (info->mood == BORED_MOOD)
				item->TargetState = 1;
			else if (info->flags && aiInfo.ahead)
				item->TargetState = 1;
			else if (aiInfo.bite && aiInfo.distance < pow(SECTOR(1.5f), 2))
			{
				if (LaraItem->Velocity == 0)
					item->TargetState = 1;
				else
					item->TargetState = 7;
			}
			else if (info->mood != ATTACK_MOOD && GetRandomControl() < 0x60)
			{
				item->RequiredState = 5;
				item->TargetState = 1;
			}
			else if (info->mood == ESCAPE_MOOD && Lara.target != item && aiInfo.ahead)
				item->TargetState = 1;

			info->flags = 0;
			break;

		case 6:
		case 7:
		case 8:
			if (!info->flags && item->TouchBits & 0x7FDC000)
			{
				CreatureEffect(item, &TigerBite, DoBloodSplat);
				info->flags = 1;

				LaraItem->HitStatus = true;
				LaraItem->HitPoints -= 90;
			}

			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNumber, angle, tilt);
}
