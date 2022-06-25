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
		if (item->Animation.ActiveState != 5)
		{
			item->Animation.AnimNumber = Objects[ID_SHARK].animIndex + 4;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 5;
		}

		CreatureFloat(itemNumber);
		return;
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, info->MaxTurn);

		switch (item->Animation.ActiveState)
		{
		case 0:
			info->Flags = 0;
			info->MaxTurn = 0;

			if (AI.ahead && AI.distance < pow(SECTOR(0.75f), 2) && AI.zoneNumber == AI.enemyZone)
				item->Animation.TargetState = 3;
			else
				item->Animation.TargetState = 1;
			break;

		case 1:
			info->MaxTurn = ANGLE(0.5f);

			if (info->Mood == MoodType::Bored)
				break;
			else if (AI.ahead && AI.distance < pow(SECTOR(0.75f), 2))
				item->Animation.TargetState = 0;
			else if (info->Mood == MoodType::Escape || AI.distance > pow(SECTOR(3), 2) || !AI.ahead)
				item->Animation.TargetState = 2;

			break;

		case 2:
			info->MaxTurn = ANGLE(2.0f);
			info->Flags = 0;

			if (info->Mood == MoodType::Bored)
				item->Animation.TargetState = 1;
			else if (info->Mood == MoodType::Escape)
				break;
			else if (AI.ahead && AI.distance < pow(1365, 2) && AI.zoneNumber == AI.enemyZone)
			{
				if (GetRandomControl() < 0x800)
					item->Animation.TargetState = 0;
				else if (AI.distance < pow(SECTOR(0.75f), 2))
					item->Animation.TargetState = 4;
			}

			break;

		case 3:
		case 4:
			if (AI.ahead)
				head = AI.angle;

			if (!info->Flags && item->TouchBits & 0x3400)
			{
				CreatureEffect(item, &SharkBite, DoBloodSplat);
				DoDamage(info->Enemy, 400);
				info->Flags = 1;
			}

			break;
		}
	}

	if (item->Animation.ActiveState != 6)
	{
		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNumber, angle, 0);
		CreatureUnderwater(item, 340);
	}
	else
		AnimateItem(item);
}
