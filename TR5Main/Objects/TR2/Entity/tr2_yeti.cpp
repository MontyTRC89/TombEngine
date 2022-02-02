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
	item->animNumber = Objects[item->objectNumber].animIndex + 19;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->activeState = g_Level.Anims[item->animNumber].activeState;
}

void YetiControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* yeti = (CREATURE_INFO*)item->data;
	AI_INFO info;
	short angle = 0, torso = 0, head = 0, tilt = 0;
	bool lara_alive;

	lara_alive = (LaraItem->hitPoints > 0);

	if (item->hitPoints <= 0)
	{
		if (item->activeState != 8)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 31;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->activeState = 8;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, yeti->maximumTurn);

		switch (item->activeState)
		{
		case 2:
			if (info.ahead)
				head = info.angle;

			yeti->flags = 0;
			yeti->maximumTurn = 0;

			if (yeti->mood == ESCAPE_MOOD)
			{
				item->targetState = 1;
			}
			else if (item->requiredState)
			{
				item->targetState = item->requiredState;
			}
			else if (yeti->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x100 || !lara_alive)
					item->targetState = 7;
				else if (GetRandomControl() < 0x200)
					item->targetState = 9;
				else if (GetRandomControl() < 0x300)
					item->targetState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 2) && GetRandomControl() < 0x4000)
			{
				item->targetState = 4;
			}
			else if (info.ahead && info.distance < SQUARE(STEP_SIZE))
			{
				item->targetState = 5;
			}
			else if (yeti->mood == STALK_MOOD)
			{
				item->targetState = 3;
			}
			else
			{
				item->targetState = 1;
			}
			break;
		case 7:
			if (info.ahead)
				head = info.angle;

			if (yeti->mood == ESCAPE_MOOD || item->hitStatus)
			{
				item->targetState = 2;
			}
			else if (yeti->mood == BORED_MOOD)
			{
				if (lara_alive)
				{
					if (GetRandomControl() < 0x100)
					{
						item->targetState = 2;
					}
					else if (GetRandomControl() < 0x200)
					{
						item->targetState = 9;
					}
					else if (GetRandomControl() < 0x300)
					{
						item->targetState = 2;
						item->requiredState = 3;
					}
				}
			}
			else if (GetRandomControl() < 0x200)
			{
				item->targetState = 2;
			}
			break;
		case 9:
			if (info.ahead)
				head = info.angle;

			if (yeti->mood == ESCAPE_MOOD || item->hitStatus)
			{
				item->targetState = 2;
			}
			else if (yeti->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x100 || !lara_alive)
				{
					item->targetState = 7;
				}
				else if (GetRandomControl() < 0x200)
				{
					item->targetState = 2;
				}
				else if (GetRandomControl() < 0x300)
				{
					item->targetState = 2;
					item->requiredState = 3;
				}
			}
			else if (GetRandomControl() < 0x200)
			{
				item->targetState = 2;
			}
			break;
		case 3:
			if (info.ahead)
				head = info.angle;

			yeti->maximumTurn = ANGLE(4);

			if (yeti->mood == ESCAPE_MOOD)
			{
				item->targetState = 1;
			}
			else if (yeti->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x100 || !lara_alive)
				{
					item->targetState = 2;
					item->requiredState = 7;
				}
				else if (GetRandomControl() < 0x200)
				{
					item->targetState = 2;
					item->requiredState = 9;
				}
				else if (GetRandomControl() < 0x300)
				{
					item->targetState = 2;
				}
			}
			else if (yeti->mood == ATTACK_MOOD)
			{
				if (info.ahead && info.distance < SQUARE(STEP_SIZE))
					item->targetState = 2;
				else if (info.distance < SQUARE(WALL_SIZE * 2))
					item->targetState = 1;
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
				item->targetState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(STEP_SIZE))
			{
				item->targetState = 2;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
			{
				item->targetState = 6;
			}
			else if (yeti->mood == STALK_MOOD)
			{
				item->targetState = 3;
			}
			break;
		case 4:
			if (info.ahead)
				torso = info.angle;

			if (!yeti->flags && (item->touchBits & 0x1400))
			{
				CreatureEffect(item, &yetiBiteR, DoBloodSplat);
				LaraItem->hitPoints -= 100;
				LaraItem->hitStatus = true;

				yeti->flags = 1;
			}
			break;
		case 5:
			if (info.ahead)
				torso = info.angle;
			yeti->maximumTurn = ANGLE(4);

			if (!yeti->flags && (item->touchBits & (0x0700 | 0x1400)))
			{
				if (item->touchBits & 0x0700)
					CreatureEffect(item, &yetiBiteL, DoBloodSplat);
				if (item->touchBits & 0x1400)
					CreatureEffect(item, &yetiBiteR, DoBloodSplat);

				LaraItem->hitPoints -= 150;
				LaraItem->hitStatus = true;

				yeti->flags = 1;
			}
			break;
		case 6:
			if (info.ahead)
				torso = info.angle;

			if (!yeti->flags && (item->touchBits & (0x0700 | 0x1400)))
			{
				if (item->touchBits & 0x0700)
					CreatureEffect(item, &yetiBiteL, DoBloodSplat);
				if (item->touchBits & 0x1400)
					CreatureEffect(item, &yetiBiteR, DoBloodSplat);

				LaraItem->hitPoints -= 200;
				LaraItem->hitStatus = true;

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

	if (item->activeState < 10)
	{
		switch (CreatureVault(itemNum, angle, 2, 300))
		{
		case 2:
			item->animNumber = Objects[item->objectNumber].animIndex + 34;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->activeState = 10;
			break;
		case 3:
			item->animNumber = Objects[item->objectNumber].animIndex + 33;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->activeState = 11;
			break;
		case 4:
			item->animNumber = Objects[item->objectNumber].animIndex + 32;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->activeState = 12;
			break;
		case -4:
			item->animNumber = Objects[item->objectNumber].animIndex + 35;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->activeState = 13;
			break;
		}
	}
	else
	{
		CreatureAnimation(itemNum, angle, tilt);
	}
}
