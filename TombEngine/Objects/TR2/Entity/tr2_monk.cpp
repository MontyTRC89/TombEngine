#include "framework.h"
#include "Objects/TR2/Entity/tr2_monk.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

bool MonksAttackLara;

BITE_INFO MonkBite = { -23,16,265, 14 };

// TODO
enum MonkState
{

};

// TODO
enum MonkAnim
{

};

void MonkControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short torso = 0;
	short angle = 0;
	short tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 9)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 20 + (GetRandomControl() / 0x4000);
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 9;
		}
	}
	else
	{
		if (MonksAttackLara)
			creature->Enemy = LaraItem;

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		if (!MonksAttackLara && creature->Enemy == LaraItem)
			creature->Enemy = NULL;

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);
		angle = CreatureTurn(item, creature->MaxTurn);

		if (AI.ahead)
			torso = AI.angle;

		switch (item->Animation.ActiveState)
		{
		case 1:
			creature->Flags &= 0x0FFF;

			if (!MonksAttackLara && AI.ahead && Lara.TargetEntity == item)
				break;
			else if (creature->Mood == MoodType::Bored)
				item->Animation.TargetState = 2;
			else if (creature->Mood == MoodType::Escape)
				item->Animation.TargetState = 3;
			else if (AI.ahead && AI.distance < pow(SECTOR(0.5f), 2))
			{
				if (GetRandomControl() < 0x7000)
					item->Animation.TargetState = 4;
				else
					item->Animation.TargetState = 11;
			}
			else if (AI.ahead && AI.distance < pow(SECTOR(1), 2))
				item->Animation.TargetState = 7;
			else if (AI.ahead && AI.distance < pow(SECTOR(2), 2))
				item->Animation.TargetState = 2;
			else
				item->Animation.TargetState = 3;
			
			break;

		case 11:
			creature->Flags &= 0x0FFF;

			if (!MonksAttackLara && AI.ahead && Lara.TargetEntity == item)
				break;
			else if (creature->Mood == MoodType::Bored)
				item->Animation.TargetState = 2;
			else if (creature->Mood == MoodType::Escape)
				item->Animation.TargetState = 3;
			else if (AI.ahead && AI.distance < pow(SECTOR(0.5f), 2))
			{
				auto random = GetRandomControl();
				if (random < 0x3000)
					item->Animation.TargetState = 5;
				else if (random < 0x6000)
					item->Animation.TargetState = 8;
				else
					item->Animation.TargetState = 1;
			}
			else if (AI.ahead && AI.distance < pow(SECTOR(2), 2))
				item->Animation.TargetState = 2;
			else
				item->Animation.TargetState = 3;
			
			break;

		case 2:
			creature->MaxTurn = ANGLE(3.0f);

			if (creature->Mood == MoodType::Bored)
			{
				if (!MonksAttackLara && AI.ahead && Lara.TargetEntity == item)
				{
					if (GetRandomControl() < 0x4000)
						item->Animation.TargetState = 1;
					else
						item->Animation.TargetState = 11;
				}
			}
			else if (creature->Mood == MoodType::Escape)
				item->Animation.TargetState = 3;
			else if (AI.ahead && AI.distance < pow(SECTOR(0.5f), 2))
			{
				if (GetRandomControl() < 0x4000)
					item->Animation.TargetState = 1;
				else
					item->Animation.TargetState = 11;
			}
			else if (!AI.ahead || AI.distance > pow(SECTOR(2), 2))
				item->Animation.TargetState = 3;
			
			break;

		case 3:
			tilt = angle / 4;
			creature->MaxTurn = ANGLE(4.0f);
			creature->Flags &= 0x0FFF;

			if (MonksAttackLara)
				creature->MaxTurn += ANGLE(1.0f);

			if (creature->Mood == MoodType::Bored)
				item->Animation.TargetState = 1;
			else if (creature->Mood == MoodType::Escape)
				break;
			else if (AI.ahead && AI.distance < pow(SECTOR(0.5f), 2))
			{
				if (GetRandomControl() < 0x4000)
					item->Animation.TargetState = 1;
				else
					item->Animation.TargetState = 11;
			}
			else if (AI.ahead && AI.distance < pow(SECTOR(3), 2))
				item->Animation.TargetState = 10;
			else if (AI.ahead && AI.distance < pow(SECTOR(2), 2))
			{
				if (GetRandomControl() < 0x4000)
					item->Animation.TargetState = 1;
				else
					item->Animation.TargetState = 11;
			}

			break;

		case 8:
			if (!AI.ahead || AI.distance > pow(SECTOR(0.5f), 2))
				item->Animation.TargetState = 11;
			else
				item->Animation.TargetState = 6;

			break;

		case 4:
		case 5:
		case 6:
		case 7:
		case 10:
			auto* enemy = creature->Enemy;
			if (enemy == LaraItem)
			{
				if (!(creature->Flags & 0xF000) && item->TouchBits & 0x4000)
				{
					creature->Flags |= 0x1000;
					SoundEffect(SFX_TR2_CRUNCH1, &item->Pose);
					CreatureEffect(item, &MonkBite, DoBloodSplat);
					DoDamage(enemy, 150);
				}
			}
			else
			{
				if (!(creature->Flags & 0xf000) && enemy)
				{
					if (abs(enemy->Pose.Position.x - item->Pose.Position.x) < CLICK(2) &&
						abs(enemy->Pose.Position.y - item->Pose.Position.y) < CLICK(2) &&
						abs(enemy->Pose.Position.z - item->Pose.Position.z) < CLICK(2))
					{
						creature->Flags |= 0x1000;
						SoundEffect(SFX_TR2_CRUNCH1, &item->Pose);
						DoDamage(enemy, 5);
					}
				}
			}

			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso);
	CreatureAnimation(itemNumber, angle, tilt);
}
