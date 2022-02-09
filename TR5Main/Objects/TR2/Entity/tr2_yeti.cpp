#include "framework.h"
#include "Objects/TR2/Entity/tr2_yeti.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO yetiBiteR = { 12, 101, 19, 10 };
BITE_INFO yetiBiteL = { 12, 101, 19, 13 };

void InitialiseYeti(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	ClearItem(itemNum);
	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 19;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->ActiveState = g_Level.Anims[item->AnimNumber].ActiveState;
}

void YetiControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* yeti = (CREATURE_INFO*)item->Data;
	AI_INFO info;
	short angle = 0, torso = 0, head = 0, tilt = 0;
	bool lara_alive;

	lara_alive = (LaraItem->HitPoints > 0);

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
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, yeti->maximumTurn);

		switch (item->ActiveState)
		{
		case 2:
			if (info.ahead)
				head = info.angle;

			yeti->flags = 0;
			yeti->maximumTurn = 0;

			if (yeti->mood == ESCAPE_MOOD)
			{
				item->TargetState = 1;
			}
			else if (item->RequiredState)
			{
				item->TargetState = item->RequiredState;
			}
			else if (yeti->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x100 || !lara_alive)
					item->TargetState = 7;
				else if (GetRandomControl() < 0x200)
					item->TargetState = 9;
				else if (GetRandomControl() < 0x300)
					item->TargetState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 2) && GetRandomControl() < 0x4000)
			{
				item->TargetState = 4;
			}
			else if (info.ahead && info.distance < SQUARE(STEP_SIZE))
			{
				item->TargetState = 5;
			}
			else if (yeti->mood == STALK_MOOD)
			{
				item->TargetState = 3;
			}
			else
			{
				item->TargetState = 1;
			}
			break;
		case 7:
			if (info.ahead)
				head = info.angle;

			if (yeti->mood == ESCAPE_MOOD || item->HitStatus)
			{
				item->TargetState = 2;
			}
			else if (yeti->mood == BORED_MOOD)
			{
				if (lara_alive)
				{
					if (GetRandomControl() < 0x100)
					{
						item->TargetState = 2;
					}
					else if (GetRandomControl() < 0x200)
					{
						item->TargetState = 9;
					}
					else if (GetRandomControl() < 0x300)
					{
						item->TargetState = 2;
						item->RequiredState = 3;
					}
				}
			}
			else if (GetRandomControl() < 0x200)
			{
				item->TargetState = 2;
			}
			break;
		case 9:
			if (info.ahead)
				head = info.angle;

			if (yeti->mood == ESCAPE_MOOD || item->HitStatus)
			{
				item->TargetState = 2;
			}
			else if (yeti->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x100 || !lara_alive)
				{
					item->TargetState = 7;
				}
				else if (GetRandomControl() < 0x200)
				{
					item->TargetState = 2;
				}
				else if (GetRandomControl() < 0x300)
				{
					item->TargetState = 2;
					item->RequiredState = 3;
				}
			}
			else if (GetRandomControl() < 0x200)
			{
				item->TargetState = 2;
			}
			break;
		case 3:
			if (info.ahead)
				head = info.angle;

			yeti->maximumTurn = ANGLE(4);

			if (yeti->mood == ESCAPE_MOOD)
			{
				item->TargetState = 1;
			}
			else if (yeti->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x100 || !lara_alive)
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
				{
					item->TargetState = 2;
				}
			}
			else if (yeti->mood == ATTACK_MOOD)
			{
				if (info.ahead && info.distance < SQUARE(STEP_SIZE))
					item->TargetState = 2;
				else if (info.distance < SQUARE(WALL_SIZE * 2))
					item->TargetState = 1;
			}
			break;
		case 1:
			if (info.ahead)
				head = info.angle;

			yeti->flags = 0;
			yeti->maximumTurn = ANGLE(6);
			tilt = (angle / 4);

			if (yeti->mood == ESCAPE_MOOD)
			{
				break;
			}
			else if (yeti->mood == BORED_MOOD)
			{
				item->TargetState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(STEP_SIZE))
			{
				item->TargetState = 2;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
			{
				item->TargetState = 6;
			}
			else if (yeti->mood == STALK_MOOD)
			{
				item->TargetState = 3;
			}
			break;
		case 4:
			if (info.ahead)
				torso = info.angle;

			if (!yeti->flags && (item->TouchBits & 0x1400))
			{
				CreatureEffect(item, &yetiBiteR, DoBloodSplat);
				LaraItem->HitPoints -= 100;
				LaraItem->HitStatus = true;

				yeti->flags = 1;
			}
			break;
		case 5:
			if (info.ahead)
				torso = info.angle;
			yeti->maximumTurn = ANGLE(4);

			if (!yeti->flags && (item->TouchBits & (0x0700 | 0x1400)))
			{
				if (item->TouchBits & 0x0700)
					CreatureEffect(item, &yetiBiteL, DoBloodSplat);
				if (item->TouchBits & 0x1400)
					CreatureEffect(item, &yetiBiteR, DoBloodSplat);

				LaraItem->HitPoints -= 150;
				LaraItem->HitStatus = true;

				yeti->flags = 1;
			}
			break;
		case 6:
			if (info.ahead)
				torso = info.angle;

			if (!yeti->flags && (item->TouchBits & (0x0700 | 0x1400)))
			{
				if (item->TouchBits & 0x0700)
					CreatureEffect(item, &yetiBiteL, DoBloodSplat);
				if (item->TouchBits & 0x1400)
					CreatureEffect(item, &yetiBiteR, DoBloodSplat);

				LaraItem->HitPoints -= 200;
				LaraItem->HitStatus = true;

				yeti->flags = 1;
			}
			break;
		case 10:
		case 11:
		case 12:
		case 13:
			yeti->maximumTurn = 0;
			break;
		}
	}

	if (!lara_alive)
	{
		yeti->maximumTurn = 0;
		CreatureKill(item, 31, 14, 103);
		return;
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso);
	CreatureJoint(item, 1, head);

	if (item->ActiveState < 10)
	{
		switch (CreatureVault(itemNum, angle, 2, 300))
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
		CreatureAnimation(itemNum, angle, tilt);
	}
}
