#include "framework.h"
#include "tr5_lion.h"

#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Game/control/box.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"

BITE_INFO LionBite1 = { -2, -10, 250, 21 };
BITE_INFO LionBite2 = { -2, -10, 132, 21 };

void InitialiseLion(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = 1;
	item->Animation.ActiveState = 1;
}

void LionControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	short angle = 0;
	short tilt = 0;
	short joint0 = 0;
	short joint1 = 0;

	if (CreatureActive(itemNumber))
	{
		auto* creature = GetCreatureInfo(item);

		if (item->HitPoints <= 0)
		{
			item->HitPoints = 0;

			if (item->Animation.ActiveState != 5)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + (GetRandomControl() & 1) + 7;
				item->Animation.ActiveState = 5;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			}
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				joint1 = AI.angle;

			GetCreatureMood(item, &AI, VIOLENT);
			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, creature->MaxTurn);
			joint0 = -16 * angle;

			switch (item->Animation.ActiveState)
			{
			case 1:
				creature->MaxTurn = 0;

				if (item->Animation.RequiredState)
				{
					item->Animation.TargetState = item->Animation.RequiredState;
					break;
				}

				if (creature->Mood == MoodType::Bored)
				{
					if (!(GetRandomControl() & 0x3F))
						item->Animation.TargetState = 2;
					break;
				}

				if (AI.ahead)
				{
					if (item->TouchBits & 0x200048)
					{
						item->Animation.TargetState = 7;
						break;
					}

					if (AI.distance < pow(SECTOR(1), 2))
					{
						item->Animation.TargetState = 4;
						break;
					}
				}

				item->Animation.TargetState = 3;
				break;

			case 2:
				creature->MaxTurn = ANGLE(2.0f);

				if (creature->Mood == MoodType::Bored)
				{
					if (GetRandomControl() < 128)
					{
						item->Animation.RequiredState = 6;
						item->Animation.TargetState = 1;
					}
				}
				else
					item->Animation.TargetState = 1;

				break;

			case 3:
				creature->MaxTurn = ANGLE(5.0f);
				tilt = angle;

				if (creature->Mood != MoodType::Bored)
				{
					if (AI.ahead && AI.distance < pow(SECTOR(1), 2))
						item->Animation.TargetState = 1;
					else if (item->TouchBits & 0x200048 && AI.ahead)
						item->Animation.TargetState = 1;
					else if (creature->Mood != MoodType::Escape)
					{
						if (GetRandomControl() < 128)
						{
							item->Animation.RequiredState = 6;
							item->Animation.TargetState = 1;
						}
					}
				}
				else
					item->Animation.TargetState = 1;
				
				break;

			case 4:
				if (!item->Animation.RequiredState && item->TouchBits & 0x200048)
				{
					DoDamage(creature->Enemy, 200);
					CreatureEffect2(item, &LionBite1, 10, item->Pose.Orientation.y, DoBloodSplat);
					item->Animation.RequiredState = 1;
				}

				break;

			case 7:
				creature->MaxTurn = ANGLE(1.0f);

				if (!item->Animation.RequiredState && item->TouchBits & 0x200048)
				{
					DoDamage(creature->Enemy, 60);
					CreatureEffect2(item, &LionBite2, 10, item->Pose.Orientation.y, DoBloodSplat);
					item->Animation.RequiredState = 1;
				}

				break;
			}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureAnimation(itemNumber, angle, 0);
}
