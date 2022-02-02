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
	monk = (CREATURE_INFO*)item->data;
	torso = angle = tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->activeState != 9)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 20 + (GetRandomControl() / 0x4000);
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->activeState = 9;
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

		switch (item->activeState)
		{
		case 1:
			monk->flags &= 0x0FFF;

			if (!MonksAttackLara && info.ahead && Lara.target == item)
			{
				break;
			}
			else if (monk->mood == BORED_MOOD)
			{
				item->targetState = 2;
			}
			else if (monk->mood == ESCAPE_MOOD)
			{
				item->targetState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 2))
			{
				if (GetRandomControl() < 0x7000)
					item->targetState = 4;
				else
					item->targetState = 11;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE))
			{
				item->targetState = 7;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
			{
				item->targetState = 2;
			}
			else
			{
				item->targetState = 3;
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
				item->targetState = 2;
			}
			else if (monk->mood == ESCAPE_MOOD)
			{
				item->targetState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 2))
			{
				random = GetRandomControl();
				if (random < 0x3000)
					item->targetState = 5;
				else if (random < 0x6000)
					item->targetState = 8;
				else
					item->targetState = 1;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
			{
				item->targetState = 2;
			}
			else
			{
				item->targetState = 3;
			}
			break;

		case 2:
			monk->maximumTurn = ANGLE(3);

			if (monk->mood == BORED_MOOD)
			{
				if (!MonksAttackLara && info.ahead && Lara.target == item)
				{
					if (GetRandomControl() < 0x4000)
						item->targetState = 1;
					else
						item->targetState = 11;
				}
			}
			else if (monk->mood == ESCAPE_MOOD)
			{
				item->targetState = 3;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 2))
			{
				if (GetRandomControl() < 0x4000)
					item->targetState = 1;
				else
					item->targetState = 11;
			}
			else if (!info.ahead || info.distance > SQUARE(WALL_SIZE * 2))
			{
				item->targetState = 3;
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
				item->targetState = 1;
			}
			else if (monk->mood == ESCAPE_MOOD)
			{
				break;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 2))
			{
				if (GetRandomControl() < 0x4000)
					item->targetState = 1;
				else
					item->targetState = 11;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 3))
			{
				item->targetState = 10;
			}
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
			{
				if (GetRandomControl() < 0x4000)
					item->targetState = 1;
				else
					item->targetState = 11;
			}
			break;

		case 8:
			if (!info.ahead || info.distance > SQUARE(WALL_SIZE / 2))
				item->targetState = 11;
			else
				item->targetState = 6;
			break;

		case 4:
		case 5:
		case 6:
		case 7:
		case 10:
			enemy = monk->enemy;
			if (enemy == LaraItem)
			{
				if (!(monk->flags & 0xF000) && (item->touchBits & 0x4000))
				{
					LaraItem->hitPoints -= 150;
					LaraItem->hitStatus = true;

					monk->flags |= 0x1000;
					SoundEffect(SFX_TR2_CRUNCH1, &item->pos, 0);
					CreatureEffect(item, &monkBite, DoBloodSplat);
				}
			}
			else
			{
				if (!(monk->flags & 0xf000) && enemy)
				{
					if (abs(enemy->pos.xPos - item->pos.xPos) < (STEP_SIZE * 2) &&
						abs(enemy->pos.yPos - item->pos.yPos) < (STEP_SIZE * 2) &&
						abs(enemy->pos.zPos - item->pos.zPos) < (STEP_SIZE * 2))
					{
						enemy->hitPoints -= 5;
						enemy->hitStatus = true;

						monk->flags |= 0x1000;
						SoundEffect(SFX_TR2_CRUNCH1, &item->pos, 0);
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