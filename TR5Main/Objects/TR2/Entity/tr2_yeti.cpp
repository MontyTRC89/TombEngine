#include "framework.h"
#include "Objects/TR2/Entity/tr2_yeti.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO YetiBiteRight = { 12, 101, 19, 10 };
BITE_INFO YetiBiteLeft = { 12, 101, 19, 13 };

// TODO
enum YetiState
{

};

// TODO
enum YetiAnim
{

};

void InitialiseYeti(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);
	
	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 19;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->ActiveState = g_Level.Anims[item->AnimNumber].ActiveState;
}

void YetiControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	bool laraAlive = LaraItem->HitPoints > 0;

	short angle = 0;
	short torso = 0;
	short head = 0;
	short tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 8)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 31;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 8;
		}
	}
	else
	{
		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		GetCreatureMood(item, &aiInfo, VIOLENT);
		CreatureMood(item, &aiInfo, VIOLENT);

		angle = CreatureTurn(item, info->maximumTurn);

		switch (item->ActiveState)
		{
		case 2:
			info->maximumTurn = 0;
			info->flags = 0;

			if (aiInfo.ahead)
				head = aiInfo.angle;

			if (info->mood == ESCAPE_MOOD)
				item->TargetState = 1;
			else if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (info->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x100 || !laraAlive)
					item->TargetState = 7;
				else if (GetRandomControl() < 0x200)
					item->TargetState = 9;
				else if (GetRandomControl() < 0x300)
					item->TargetState = 3;
			}
			else if (aiInfo.ahead && aiInfo.distance < pow(SECTOR(0.5f), 2) && GetRandomControl() < 0x4000)
				item->TargetState = 4;
			else if (aiInfo.ahead && aiInfo.distance < pow(CLICK(1), 2))
				item->TargetState = 5;
			else if (info->mood == STALK_MOOD)
				item->TargetState = 3;
			else
				item->TargetState = 1;
			
			break;

		case 7:
			if (aiInfo.ahead)
				head = aiInfo.angle;

			if (info->mood == ESCAPE_MOOD || item->HitStatus)
				item->TargetState = 2;
			else if (info->mood == BORED_MOOD)
			{
				if (laraAlive)
				{
					if (GetRandomControl() < 0x100)
						item->TargetState = 2;
					else if (GetRandomControl() < 0x200)
						item->TargetState = 9;
					else if (GetRandomControl() < 0x300)
					{
						item->TargetState = 2;
						item->RequiredState = 3;
					}
				}
			}
			else if (GetRandomControl() < 0x200)
				item->TargetState = 2;
			
			break;

		case 9:
			if (aiInfo.ahead)
				head = aiInfo.angle;

			if (info->mood == ESCAPE_MOOD || item->HitStatus)
				item->TargetState = 2;
			else if (info->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x100 || !laraAlive)
					item->TargetState = 7;
				else if (GetRandomControl() < 0x200)
					item->TargetState = 2;
				else if (GetRandomControl() < 0x300)
				{
					item->TargetState = 2;
					item->RequiredState = 3;
				}
			}
			else if (GetRandomControl() < 0x200)
				item->TargetState = 2;
			
			break;

		case 3:
			info->maximumTurn = ANGLE(4.0f);

			if (aiInfo.ahead)
				head = aiInfo.angle;

			if (info->mood == ESCAPE_MOOD)
				item->TargetState = 1;
			else if (info->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x100 || !laraAlive)
				{
					item->TargetState = 2;
					item->RequiredState = 7;
				}
				else if (GetRandomControl() < 0x200)
				{
					item->TargetState = 2;
					item->RequiredState = 9;
				}
				else if (GetRandomControl() < 0x300)
					item->TargetState = 2;
			}
			else if (info->mood == ATTACK_MOOD)
			{
				if (aiInfo.ahead && aiInfo.distance < pow(CLICK(1), 2))
					item->TargetState = 2;
				else if (aiInfo.distance < pow(SECTOR(2), 2))
					item->TargetState = 1;
			}

			break;

		case 1:
			info->maximumTurn = ANGLE(6.0f);
			tilt = angle / 4;
			info->flags = 0;

			if (aiInfo.ahead)
				head = aiInfo.angle;

			if (info->mood == ESCAPE_MOOD)
				break;
			else if (info->mood == BORED_MOOD)
				item->TargetState = 3;
			else if (aiInfo.ahead && aiInfo.distance < pow(CLICK(1), 2))
				item->TargetState = 2;
			else if (aiInfo.ahead && aiInfo.distance < pow(SECTOR(2), 2))
				item->TargetState = 6;
			else if (info->mood == STALK_MOOD)
				item->TargetState = 3;
			
			break;

		case 4:
			if (aiInfo.ahead)
				torso = aiInfo.angle;

			if (!info->flags && item->TouchBits & 0x1400)
			{
				CreatureEffect(item, &YetiBiteRight, DoBloodSplat);
				info->flags = 1;

				LaraItem->HitPoints -= 100;
				LaraItem->HitStatus = true;
			}

			break;

		case 5:
			info->maximumTurn = ANGLE(4.0f);

			if (aiInfo.ahead)
				torso = aiInfo.angle;

			if (!info->flags && item->TouchBits & (0x0700 | 0x1400))
			{
				if (item->TouchBits & 0x0700)
					CreatureEffect(item, &YetiBiteLeft, DoBloodSplat);
				if (item->TouchBits & 0x1400)
					CreatureEffect(item, &YetiBiteRight, DoBloodSplat);

				info->flags = 1;

				LaraItem->HitPoints -= 150;
				LaraItem->HitStatus = true;
			}

			break;

		case 6:
			if (aiInfo.ahead)
				torso = aiInfo.angle;

			if (!info->flags && item->TouchBits & (0x0700 | 0x1400))
			{
				if (item->TouchBits & 0x0700)
					CreatureEffect(item, &YetiBiteLeft, DoBloodSplat);
				if (item->TouchBits & 0x1400)
					CreatureEffect(item, &YetiBiteRight, DoBloodSplat);

				info->flags = 1;

				LaraItem->HitPoints -= 200;
				LaraItem->HitStatus = true;
			}

			break;

		case 10:
		case 11:
		case 12:
		case 13:
			info->maximumTurn = 0;
			break;
		}
	}

	if (!laraAlive)
	{
		info->maximumTurn = 0;
		CreatureKill(item, 31, 14, 103);
		return;
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso);
	CreatureJoint(item, 1, head);

	if (item->ActiveState < 10)
	{
		switch (CreatureVault(itemNumber, angle, 2, 300))
		{
		case 2:
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 34;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 10;
			break;

		case 3:
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 33;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 11;
			break;

		case 4:
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 32;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 12;
			break;

		case -4:
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 35;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 13;
			break;
		}
	}
	else
	{
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
