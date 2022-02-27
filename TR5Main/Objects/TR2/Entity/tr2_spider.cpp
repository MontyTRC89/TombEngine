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
	DoBloodSplat(pos.x, pos.y, pos.z, 10, item->Position.yPos, item->RoomNumber);
}

static void SpiderLeap(short itemNum, ITEM_INFO* item, short angle)
{
	GAME_VECTOR vec;

	vec.x = item->Position.xPos;
	vec.y = item->Position.yPos;
	vec.z = item->Position.zPos;
	vec.roomNumber = item->RoomNumber;

	CreatureAnimation(itemNum, angle, 0);

	if (item->Position.yPos > (vec.y - (STEP_SIZE * 3) / 2))
		return;

	item->Position.xPos = vec.x;
	item->Position.yPos = vec.y;
	item->Position.zPos = vec.z;
	if (item->RoomNumber != vec.roomNumber)
		ItemNewRoom(item->RoomNumber, vec.roomNumber);

	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 2;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->ActiveState = 5;

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
	spider = (CREATURE_INFO*)item->Data;
	angle = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 7)
		{
			ExplodingDeath(itemNum, -1, 256);
			DisableEntityAI(itemNum);
			item->ActiveState = 7;
			KillItem(itemNum);
		}
	}
	else
	{
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, ANGLE(8));

		switch (item->ActiveState)
		{
		case 1:
			spider->flags = 0;

			if (spider->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x100)
					item->TargetState = 2;
				else
					break;
			}
			else if (info.ahead && item->TouchBits)
			{
				item->TargetState = 4;
			}
			else if (spider->mood == STALK_MOOD)
			{
				item->TargetState = 2;
			}
			else if (spider->mood == ESCAPE_MOOD || spider->mood == ATTACK_MOOD)
			{
				item->TargetState = 3;
			}
			break;

		case 2:
			if (spider->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x100)
					item->TargetState = 1;
				else
					break;
			}
			else if (spider->mood == ESCAPE_MOOD || spider->mood == ATTACK_MOOD)
			{
				item->TargetState = 3;
			}
			break;

		case 3:
			spider->flags = 0;

			if (spider->mood == BORED_MOOD || spider->mood == STALK_MOOD)
				item->TargetState = 2;
			else if (info.ahead && item->TouchBits)
				item->TargetState = 1;
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 5))
				item->TargetState = 6;
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 2))
				item->TargetState = 5;
			break;

		case 4:
		case 5:
		case 6:
			if (!spider->flags && item->TouchBits)
			{
				S_SpiderBite(item);
				LaraItem->HitPoints -= 25;
				LaraItem->HitStatus = 1;
				spider->flags = 1;
			}
			break;
		}
	}


	if (item->ActiveState == 5 || item->ActiveState == 4)
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
	spider = (CREATURE_INFO*)item->Data;
	angle = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 7)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 2;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 7;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, TRUE);
		CreatureMood(item, &info, TRUE);
		angle = CreatureTurn(item, ANGLE(4));

		switch (item->ActiveState)
		{
		case 1:
			spider->flags = 0;

			if (spider->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x200)
					item->TargetState = 2;
				else
					break;
			}
			else if (info.ahead && info.distance < (SQUARE(STEP_SIZE * 3) + 15))
			{
				item->TargetState = 4;
			}
			else if (spider->mood == STALK_MOOD)
			{
				item->TargetState = 2;
			}
			else if (spider->mood == ESCAPE_MOOD || spider->mood == ATTACK_MOOD)
			{
				item->TargetState = 3;
			}
			break;

		case 2:
			if (spider->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x200)
					item->TargetState = 1;
				else
					break;
			}
			else if (spider->mood == ESCAPE_MOOD || spider->mood == ATTACK_MOOD)
			{
				item->TargetState = 3;
			}
			break;

		case 3:
			spider->flags = 0;

			if (spider->mood == BORED_MOOD || spider->mood == STALK_MOOD)
				item->TargetState = 2;
			else if (info.ahead && item->TouchBits)
				item->TargetState = 1;
			break;

		case 4:
		case 5:
		case 6:
			if (!spider->flags && item->TouchBits)
			{
				S_SpiderBite(item);
				LaraItem->HitPoints -= 100;
				LaraItem->HitStatus = 1;
				spider->flags = 1;
			}
			break;
		}
	}

	CreatureAnimation(itemNum, angle, 0);
}
