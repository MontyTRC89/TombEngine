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

extern bool MonksAttackLara;

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
	auto* info = GetCreatureInfo(item);

	short torso = 0;
	short angle = 0;
	short tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 9)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 20 + (GetRandomControl() / 0x4000);
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 9;
		}
	}
	else
	{
		if (MonksAttackLara)
			info->enemy = LaraItem;

		AI_INFO AIInfo;
		CreatureAIInfo(item, &AIInfo);

		if (!MonksAttackLara && info->enemy == LaraItem)
			info->enemy = NULL;

		GetCreatureMood(item, &AIInfo, VIOLENT);
		CreatureMood(item, &AIInfo, VIOLENT);
		angle = CreatureTurn(item, info->maximumTurn);

		if (AIInfo.ahead)
			torso = AIInfo.angle;

		switch (item->ActiveState)
		{
		case 1:
			info->flags &= 0x0FFF;

			if (!MonksAttackLara && AIInfo.ahead && Lara.target == item)
				break;
			else if (info->mood == BORED_MOOD)
				item->TargetState = 2;
			else if (info->mood == ESCAPE_MOOD)
				item->TargetState = 3;
			else if (AIInfo.ahead && AIInfo.distance < pow(SECTOR(0.5f), 2))
			{
				if (GetRandomControl() < 0x7000)
					item->TargetState = 4;
				else
					item->TargetState = 11;
			}
			else if (AIInfo.ahead && AIInfo.distance < pow(SECTOR(1), 2))
				item->TargetState = 7;
			else if (AIInfo.ahead && AIInfo.distance < pow(SECTOR(2), 2))
				item->TargetState = 2;
			else
				item->TargetState = 3;
			
			break;

		case 11:
			info->flags &= 0x0FFF;

			if (!MonksAttackLara && AIInfo.ahead && Lara.target == item)
				break;
			else if (info->mood == BORED_MOOD)
				item->TargetState = 2;
			else if (info->mood == ESCAPE_MOOD)
				item->TargetState = 3;
			else if (AIInfo.ahead && AIInfo.distance < pow(SECTOR(0.5f), 2))
			{
				auto random = GetRandomControl();
				if (random < 0x3000)
					item->TargetState = 5;
				else if (random < 0x6000)
					item->TargetState = 8;
				else
					item->TargetState = 1;
			}
			else if (AIInfo.ahead && AIInfo.distance < pow(SECTOR(2), 2))
				item->TargetState = 2;
			else
				item->TargetState = 3;
			
			break;

		case 2:
			info->maximumTurn = ANGLE(3.0f);

			if (info->mood == BORED_MOOD)
			{
				if (!MonksAttackLara && AIInfo.ahead && Lara.target == item)
				{
					if (GetRandomControl() < 0x4000)
						item->TargetState = 1;
					else
						item->TargetState = 11;
				}
			}
			else if (info->mood == ESCAPE_MOOD)
				item->TargetState = 3;
			else if (AIInfo.ahead && AIInfo.distance < pow(SECTOR(0.5f), 2))
			{
				if (GetRandomControl() < 0x4000)
					item->TargetState = 1;
				else
					item->TargetState = 11;
			}
			else if (!AIInfo.ahead || AIInfo.distance > pow(SECTOR(2), 2))
				item->TargetState = 3;
			
			break;

		case 3:
			info->flags &= 0x0FFF;
			info->maximumTurn = ANGLE(4.0f);

			if (MonksAttackLara)
				info->maximumTurn += ANGLE(1.0f);

			tilt = angle / 4;

			if (info->mood == BORED_MOOD)
				item->TargetState = 1;
			else if (info->mood == ESCAPE_MOOD)
				break;
			else if (AIInfo.ahead && AIInfo.distance < pow(SECTOR(0.5f), 2))
			{
				if (GetRandomControl() < 0x4000)
					item->TargetState = 1;
				else
					item->TargetState = 11;
			}
			else if (AIInfo.ahead && AIInfo.distance < pow(SECTOR(3), 2))
				item->TargetState = 10;
			else if (AIInfo.ahead && AIInfo.distance < pow(SECTOR(2), 2))
			{
				if (GetRandomControl() < 0x4000)
					item->TargetState = 1;
				else
					item->TargetState = 11;
			}

			break;

		case 8:
			if (!AIInfo.ahead || AIInfo.distance > pow(SECTOR(0.5f), 2))
				item->TargetState = 11;
			else
				item->TargetState = 6;

			break;

		case 4:
		case 5:
		case 6:
		case 7:
		case 10:
			auto* enemy = info->enemy;
			if (enemy == LaraItem)
			{
				if (!(info->flags & 0xF000) && item->TouchBits & 0x4000)
				{
					info->flags |= 0x1000;
					SoundEffect(SFX_TR2_CRUNCH1, &item->Position, 0);
					CreatureEffect(item, &MonkBite, DoBloodSplat);

					enemy->HitPoints -= 150;
					enemy->HitStatus = true;
				}
			}
			else
			{
				if (!(info->flags & 0xf000) && enemy)
				{
					if (abs(enemy->Position.xPos - item->Position.xPos) < CLICK(2) &&
						abs(enemy->Position.yPos - item->Position.yPos) < CLICK(2) &&
						abs(enemy->Position.zPos - item->Position.zPos) < CLICK(2))
					{
						info->flags |= 0x1000;
						SoundEffect(SFX_TR2_CRUNCH1, &item->Position, 0);

						enemy->HitPoints -= 5;
						enemy->HitStatus = true;
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
