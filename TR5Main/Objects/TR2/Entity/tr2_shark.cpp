#include "framework.h"
#include "Objects/TR2/Entity/tr2_shark.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO SharkBite = { 17, -22, 344, 12 };

void SharkControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	short angle = 0;
	short head = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 5)
		{
			item->AnimNumber = Objects[ID_SHARK].animIndex + 4;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 5;
		}

		CreatureFloat(itemNumber);
		return;
	}
	else
	{
		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		GetCreatureMood(item, &aiInfo, VIOLENT);
		CreatureMood(item, &aiInfo, VIOLENT);

		angle = CreatureTurn(item, info->MaxTurn);

		switch (item->ActiveState)
		{
		case 0:
			info->Flags = 0;
			info->MaxTurn = 0;

			if (aiInfo.ahead && aiInfo.distance < pow(SECTOR(0.75f), 2) && aiInfo.zoneNumber == aiInfo.enemyZone)
				item->TargetState = 3;
			else
				item->TargetState = 1;
			break;

		case 1:
			info->MaxTurn = ANGLE(0.5f);

			if (info->Mood == MoodType::Bored)
				break;
			else if (aiInfo.ahead && aiInfo.distance < pow(SECTOR(0.75f), 2))
				item->TargetState = 0;
			else if (info->Mood == MoodType::Escape || aiInfo.distance > pow(SECTOR(3), 2) || !aiInfo.ahead)
				item->TargetState = 2;

			break;

		case 2:
			info->MaxTurn = ANGLE(2.0f);
			info->Flags = 0;

			if (info->Mood == MoodType::Bored)
				item->TargetState = 1;
			else if (info->Mood == MoodType::Escape)
				break;
			else if (aiInfo.ahead && aiInfo.distance < pow(1365, 2) && aiInfo.zoneNumber == aiInfo.enemyZone)
			{
				if (GetRandomControl() < 0x800)
					item->TargetState = 0;
				else if (aiInfo.distance < pow(SECTOR(0.75f), 2))
					item->TargetState = 4;
			}

			break;

		case 3:
		case 4:
			if (aiInfo.ahead)
				head = aiInfo.angle;

			if (!info->Flags && item->TouchBits & 0x3400)
			{
				CreatureEffect(item, &SharkBite, DoBloodSplat);
				info->Flags = 1;

				LaraItem->HitPoints -= 400;
				LaraItem->HitStatus = true;
			}

			break;
		}
	}

	if (item->ActiveState != 6)
	{
		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNumber, angle, 0);
		CreatureUnderwater(item, 340);
	}
	else
		AnimateItem(item);
}
