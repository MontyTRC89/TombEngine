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

	item->AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->TargetState = 1;
	item->ActiveState = 1;
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

			if (item->ActiveState != 5)
			{
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + (GetRandomControl() & 1) + 7;
				item->ActiveState = 5;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			}
		}
		else
		{
			AI_INFO aiInfo;
			CreatureAIInfo(item, &aiInfo);

			if (aiInfo.ahead)
				joint1 = aiInfo.angle;

			GetCreatureMood(item, &aiInfo, VIOLENT);
			CreatureMood(item, &aiInfo, VIOLENT);

			angle = CreatureTurn(item, creature->maximumTurn);
			joint0 = -16 * angle;

			switch (item->ActiveState)
			{
			case 1:
				creature->maximumTurn = 0;

				if (item->RequiredState)
				{
					item->TargetState = item->RequiredState;
					break;
				}

				if (!creature->mood)
				{
					if (!(GetRandomControl() & 0x3F))
						item->TargetState = 2;
					break;
				}

				if (aiInfo.ahead)
				{
					if (item->TouchBits & 0x200048)
					{
						item->TargetState = 7;
						break;
					}

					if (aiInfo.distance < pow(SECTOR(1), 2))
					{
						item->TargetState = 4;
						break;
					}
				}

				item->TargetState = 3;
				break;

			case 2:
				creature->maximumTurn = ANGLE(2.0f);

				if (!creature->mood)
				{
					if (GetRandomControl() < 128)
					{
						item->RequiredState = 6;
						item->TargetState = 1;
					}
				}
				else
					item->TargetState = 1;

				break;

			case 3:
				creature->maximumTurn = ANGLE(5.0f);
				tilt = angle;

				if (creature->mood)
				{
					if (aiInfo.ahead && aiInfo.distance < pow(SECTOR(1), 2))
						item->TargetState = 1;
					else if (item->TouchBits & 0x200048 && aiInfo.ahead)
						item->TargetState = 1;
					else if (creature->mood != ESCAPE_MOOD)
					{
						if (GetRandomControl() < 128)
						{
							item->RequiredState = 6;
							item->TargetState = 1;
						}
					}
				}
				else
					item->TargetState = 1;
				
				break;

			case 4:
				if (!item->RequiredState && item->TouchBits & 0x200048)
				{
					CreatureEffect2(item, &LionBite1, 10, item->Position.yRot, DoBloodSplat);
					item->RequiredState = 1;

					LaraItem->HitPoints -= 200;
					LaraItem->HitStatus = true;
				}

				break;
			case 7:
				creature->maximumTurn = ANGLE(1.0f);

				if (!item->RequiredState && item->TouchBits & 0x200048)
				{
					CreatureEffect2(item, &LionBite2, 10, item->Position.yRot, DoBloodSplat);
					item->RequiredState = 1;

					LaraItem->HitPoints -= 60;
					LaraItem->HitStatus = true;
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
