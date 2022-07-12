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
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO SpiderBite = { 0, 0, 41, 1 };

static void S_SpiderBite(ItemInfo* item)
{
	Vector3Int pos = { SpiderBite.x, SpiderBite.y, SpiderBite.z };
	GetJointAbsPosition(item, &pos, SpiderBite.meshNum);

	DoBloodSplat(pos.x, pos.y, pos.z, 10, item->Pose.Position.y, item->RoomNumber);
}

static void SpiderLeap(short itemNumber, ItemInfo* item, short angle)
{
	GameVector vec;
	vec.x = item->Pose.Position.x;
	vec.y = item->Pose.Position.y;
	vec.z = item->Pose.Position.z;
	vec.roomNumber = item->RoomNumber;

	CreatureAnimation(itemNumber, angle, 0);

	if (item->Pose.Position.y > (vec.y - CLICK(1.5f)))
		return;

	item->Pose.Position.x = vec.x;
	item->Pose.Position.y = vec.y;
	item->Pose.Position.z = vec.z;
	if (item->RoomNumber != vec.roomNumber)
		ItemNewRoom(item->RoomNumber, vec.roomNumber);

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 2;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.ActiveState = 5;

	CreatureAnimation(itemNumber, angle, 0);
}

void SmallSpiderControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short angle = 0;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 7)
		{
			ExplodingDeath(itemNumber, 0);
			DisableEntityAI(itemNumber);
			item->Animation.ActiveState = 7;
			KillItem(itemNumber);
		}
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);
		angle = CreatureTurn(item, ANGLE(8.0f));

		switch (item->Animation.ActiveState)
		{
		case 1:
			creature->Flags = 0;

			if (creature->Mood == MoodType::Bored)
			{
				if (GetRandomControl() < 0x100)
					item->Animation.TargetState = 2;
				else
					break;
			}
			else if (AI.ahead && item->TouchBits)
				item->Animation.TargetState = 4;
			else if (creature->Mood == MoodType::Stalk)
				item->Animation.TargetState = 2;
			else if (creature->Mood == MoodType::Escape || creature->Mood == MoodType::Attack)
				item->Animation.TargetState = 3;
			
			break;

		case 2:
			if (creature->Mood == MoodType::Bored)
			{
				if (GetRandomControl() < 0x100)
					item->Animation.TargetState = 1;
				else
					break;
			}
			else if (creature->Mood == MoodType::Escape || creature->Mood == MoodType::Attack)
				item->Animation.TargetState = 3;
			
			break;

		case 3:
			creature->Flags = 0;

			if (creature->Mood == MoodType::Bored || creature->Mood == MoodType::Stalk)
				item->Animation.TargetState = 2;
			else if (AI.ahead && item->TouchBits)
				item->Animation.TargetState = 1;
			else if (AI.ahead && AI.distance < pow(SECTOR(0.2f), 2))
				item->Animation.TargetState = 6;
			else if (AI.ahead && AI.distance < pow(SECTOR(0.5f), 2))
				item->Animation.TargetState = 5;

			break;

		case 4:
		case 5:
		case 6:
			if (!creature->Flags && item->TouchBits)
			{
				S_SpiderBite(item);
				DoDamage(creature->Enemy, 25);
				creature->Flags = 1;
			}

			break;
		}
	}


	if (item->Animation.ActiveState == 5 || item->Animation.ActiveState == 4)
		CreatureAnimation(itemNumber, angle, 0);
	else
		SpiderLeap(itemNumber, item, angle);
}

void BigSpiderControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short angle = 0;

	AI_INFO AI;
	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 7)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 2;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 7;
		}
	}
	else
	{
		CreatureAIInfo(item, &AI);
		GetCreatureMood(item, &AI, TRUE);
		CreatureMood(item, &AI, TRUE);
		angle = CreatureTurn(item, ANGLE(4.0f));

		switch (item->Animation.ActiveState)
		{
		case 1:
			creature->Flags = 0;

			if (creature->Mood == MoodType::Bored)
			{
				if (GetRandomControl() < 0x200)
					item->Animation.TargetState = 2;
				else
					break;
			}
			else if (AI.ahead && AI.distance < (pow(CLICK(3), 2) + 15))
				item->Animation.TargetState = 4;
			else if (creature->Mood == MoodType::Stalk)
				item->Animation.TargetState = 2;
			else if (creature->Mood == MoodType::Escape || creature->Mood == MoodType::Attack)
				item->Animation.TargetState = 3;
			
			break;

		case 2:
			if (creature->Mood == MoodType::Bored)
			{
				if (GetRandomControl() < 0x200)
					item->Animation.TargetState = 1;
				else
					break;
			}
			else if (creature->Mood == MoodType::Escape || creature->Mood == MoodType::Attack)
				item->Animation.TargetState = 3;
			
			break;

		case 3:
			creature->Flags = 0;

			if (creature->Mood == MoodType::Bored || creature->Mood == MoodType::Stalk)
				item->Animation.TargetState = 2;
			else if (AI.ahead && item->TouchBits)
				item->Animation.TargetState = 1;

			break;

		case 4:
		case 5:
		case 6:
			if (!creature->Flags && item->TouchBits)
			{
				S_SpiderBite(item);
				DoDamage(creature->Enemy, 100);
				creature->Flags = 1;
			}

			break;
		}
	}

	CreatureAnimation(itemNumber, angle, 0);
}
