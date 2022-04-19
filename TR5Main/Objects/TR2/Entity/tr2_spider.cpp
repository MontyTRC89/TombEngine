#include "framework.h"
#include "Objects/TR2/Entity/tr2_spider.h"

#include "Game/animation.h"
#include "Game/control/control.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO spiderBite = { 0, 0, 41, 1 };

static void S_SpiderBite(ITEM_INFO* item)
{
	PHD_VECTOR pos;
	pos.x = spiderBite.x;
	pos.y = spiderBite.y;
	pos.z = spiderBite.z;
	GetJointAbsPosition(item, &pos, spiderBite.meshNum);
	DoBloodSplat(pos.x, pos.y, pos.z, 10, item->pos.yPos, item->roomNumber);
}

static void SpiderLeap(short itemNum, ITEM_INFO* item, short angle)
{
	GAME_VECTOR vec;

	vec.x = item->pos.xPos;
	vec.y = item->pos.yPos;
	vec.z = item->pos.zPos;
	vec.roomNumber = item->roomNumber;

	CreatureAnimation(itemNum, angle, 0);

	if (item->pos.yPos > (vec.y - (STEP_SIZE * 3) / 2))
		return;

	item->pos.xPos = vec.x;
	item->pos.yPos = vec.y;
	item->pos.zPos = vec.z;
	if (item->roomNumber != vec.roomNumber)
		ItemNewRoom(item->roomNumber, vec.roomNumber);

	item->animNumber = Objects[item->objectNumber].animIndex + 2;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->currentAnimState = 5;

	CreatureAnimation(itemNum, angle, 0);
}

void SmallSpiderControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* spider;
	AI_INFO info;
	short angle;

	item = &g_Level.Items[itemNum];
	spider = (CREATURE_INFO*)item->data;
	angle = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 7)
		{
			ExplodingDeath(itemNum, -1, 256);
			DisableBaddieAI(itemNum);
			item->currentAnimState = 7;
			KillItem(itemNum);
		}
	}
	else
	{
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, ANGLE(8));

		switch (item->currentAnimState)
		{
		case 1:
			spider->flags = 0;

			if (spider->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x100)
					item->goalAnimState = 2;
				else
					break;
			}
			else if (info.ahead && item->touchBits)
			{
				item->goalAnimState = 4;
			}
			else if (spider->mood == STALK_MOOD)
			{
				item->goalAnimState = 2;
			}
			else if (spider->mood == ESCAPE_MOOD || spider->mood == ATTACK_MOOD)
			{
				item->goalAnimState = 3;
			}
			break;

		case 2:
			if (spider->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x100)
					item->goalAnimState = 1;
				else
					break;
			}
			else if (spider->mood == ESCAPE_MOOD || spider->mood == ATTACK_MOOD)
			{
				item->goalAnimState = 3;
			}
			break;

		case 3:
			spider->flags = 0;

			if (spider->mood == BORED_MOOD || spider->mood == STALK_MOOD)
				item->goalAnimState = 2;
			else if (info.ahead && item->touchBits)
				item->goalAnimState = 1;
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 5))
				item->goalAnimState = 6;
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 2))
				item->goalAnimState = 5;
			break;

		case 4:
		case 5:
		case 6:
			if (!spider->flags && item->touchBits)
			{
				S_SpiderBite(item);
				LaraItem->hitPoints -= 25;
				LaraItem->hitStatus = 1;
				spider->flags = 1;
			}
			break;
		}
	}


	if (item->currentAnimState == 5 || item->currentAnimState == 4)
		CreatureAnimation(itemNum, angle, 0);
	else
		SpiderLeap(itemNum, item, angle);
}

void BigSpiderControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* spider;
	AI_INFO info;
	short angle;

	item = &g_Level.Items[itemNum];
	spider = (CREATURE_INFO*)item->data;
	angle = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 7)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 2;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 7;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, TRUE);
		CreatureMood(item, &info, TRUE);
		angle = CreatureTurn(item, ANGLE(4));

		switch (item->currentAnimState)
		{
		case 1:
			spider->flags = 0;

			if (spider->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x200)
					item->goalAnimState = 2;
				else
					break;
			}
			else if (info.ahead && info.distance < (SQUARE(STEP_SIZE * 3) + 15))
			{
				item->goalAnimState = 4;
			}
			else if (spider->mood == STALK_MOOD)
			{
				item->goalAnimState = 2;
			}
			else if (spider->mood == ESCAPE_MOOD || spider->mood == ATTACK_MOOD)
			{
				item->goalAnimState = 3;
			}
			break;

		case 2:
			if (spider->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x200)
					item->goalAnimState = 1;
				else
					break;
			}
			else if (spider->mood == ESCAPE_MOOD || spider->mood == ATTACK_MOOD)
			{
				item->goalAnimState = 3;
			}
			break;

		case 3:
			spider->flags = 0;

			if (spider->mood == BORED_MOOD || spider->mood == STALK_MOOD)
				item->goalAnimState = 2;
			else if (info.ahead && item->touchBits)
				item->goalAnimState = 1;
			break;

		case 4:
		case 5:
		case 6:
			if (!spider->flags && item->touchBits)
			{
				S_SpiderBite(item);
				LaraItem->hitPoints -= 100;
				LaraItem->hitStatus = 1;
				spider->flags = 1;
			}
			break;
		}
	}

	CreatureAnimation(itemNum, angle, 0);
}
