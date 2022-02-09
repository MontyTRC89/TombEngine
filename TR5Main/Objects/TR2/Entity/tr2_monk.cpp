#include "framework.h"
#include "Objects/TR2/Entity/tr2_monk.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO monkBite = { -23,16,265, 14 };

extern bool MonksAttackLara;

void MonkControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item, *enemy;
	CREATURE_INFO* monk;
	short angle, torso, tilt;
	AI_INFO info;
	int random;

	item = &g_Level.Items[itemNum];
	monk = (CREATURE_INFO*)item->Data;
	torso = angle = tilt = 0;

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
			monk->enemy = LaraItem;

		CreatureAIInfo(item, &info);

		if (!MonksAttackLara && monk->enemy == LaraItem)
			monk->enemy = NULL;

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, monk->maximumTurn);

		if (info.ahead)
			torso = info.angle;

		switch (item->ActiveState)
		{
		case 1:
			monk->flags &= 0x0FFF;

			if (!MonksAttackLara && info.ahead && Lara.target == item)
			{
				break;
			}
			else if (monk->mood == BORED_MOOD)
			{
				item->TargetState = 2;
			}
			else if (monk->mood == ESCAPE_MOOD)
			{
				item->TargetState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 2))
			{
				if (GetRandomControl() < 0x7000)
					item->TargetState = 4;
				else
					item->TargetState = 11;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE))
			{
				item->TargetState = 7;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
			{
				item->TargetState = 2;
			}
			else
			{
				item->TargetState = 3;
			}
			break;

		case 11:
			monk->flags &= 0x0FFF;

			if (!MonksAttackLara && info.ahead && Lara.target == item)
			{
				break;
			}
			else if (monk->mood == BORED_MOOD)
			{
				item->TargetState = 2;
			}
			else if (monk->mood == ESCAPE_MOOD)
			{
				item->TargetState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 2))
			{
				random = GetRandomControl();
				if (random < 0x3000)
					item->TargetState = 5;
				else if (random < 0x6000)
					item->TargetState = 8;
				else
					item->TargetState = 1;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
			{
				item->TargetState = 2;
			}
			else
			{
				item->TargetState = 3;
			}
			break;

		case 2:
			monk->maximumTurn = ANGLE(3);

			if (monk->mood == BORED_MOOD)
			{
				if (!MonksAttackLara && info.ahead && Lara.target == item)
				{
					if (GetRandomControl() < 0x4000)
						item->TargetState = 1;
					else
						item->TargetState = 11;
				}
			}
			else if (monk->mood == ESCAPE_MOOD)
			{
				item->TargetState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 2))
			{
				if (GetRandomControl() < 0x4000)
					item->TargetState = 1;
				else
					item->TargetState = 11;
			}
			else if (!info.ahead || info.distance > SQUARE(WALL_SIZE * 2))
			{
				item->TargetState = 3;
			}
			break;

		case 3:
			monk->flags &= 0x0FFF;
			monk->maximumTurn = ANGLE(4);

			if (MonksAttackLara)
				monk->maximumTurn += ANGLE(1);

			tilt = angle / 4;

			if (monk->mood == BORED_MOOD)
			{
				item->TargetState = 1;
			}
			else if (monk->mood == ESCAPE_MOOD)
			{
				break;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 2))
			{
				if (GetRandomControl() < 0x4000)
					item->TargetState = 1;
				else
					item->TargetState = 11;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 3))
			{
				item->TargetState = 10;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
			{
				if (GetRandomControl() < 0x4000)
					item->TargetState = 1;
				else
					item->TargetState = 11;
			}
			break;

		case 8:
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE / 2))
				item->TargetState = 11;
			else
				item->TargetState = 6;
			break;

		case 4:
		case 5:
		case 6:
		case 7:
		case 10:
			enemy = monk->enemy;
			if (enemy == LaraItem)
			{
				if (!(monk->flags & 0xF000) && (item->TouchBits & 0x4000))
				{
					LaraItem->HitPoints -= 150;
					LaraItem->HitStatus = true;

					monk->flags |= 0x1000;
					SoundEffect(SFX_TR2_CRUNCH1, &item->Position, 0);
					CreatureEffect(item, &monkBite, DoBloodSplat);
				}
			}
			else
			{
				if (!(monk->flags & 0xf000) && enemy)
				{
					if (abs(enemy->Position.xPos - item->Position.xPos) < (STEP_SIZE * 2) &&
						abs(enemy->Position.yPos - item->Position.yPos) < (STEP_SIZE * 2) &&
						abs(enemy->Position.zPos - item->Position.zPos) < (STEP_SIZE * 2))
					{
						enemy->HitPoints -= 5;
						enemy->HitStatus = true;

						monk->flags |= 0x1000;
						SoundEffect(SFX_TR2_CRUNCH1, &item->Position, 0);
					}
				}
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso);
	CreatureAnimation(itemNum, angle, tilt);
}