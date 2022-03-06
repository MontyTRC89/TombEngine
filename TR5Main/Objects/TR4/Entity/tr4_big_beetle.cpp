#include "framework.h"
#include "tr4_big_beetle.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Specific/trmath.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Game/itemdata/creature_info.h"

namespace TEN::Entities::TR4
{
	BITE_INFO BigBeetleBite = { 0, 0, 0, 12 };

	// TODO
	enum BigBeetleState
	{

	};

	// TODO
	enum BigBeetleAnim
	{

	};

	void InitialiseBigBeetle(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->AnimNumber = Objects[item->ObjectNumber].animIndex + 3;
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		item->TargetState = 1;
		item->ActiveState = 1;
	}

	void BigBeetleControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* info = GetCreatureInfo(item);

		short angle = 0;

		if (item->HitPoints <= 0)
		{
			if (item->ActiveState != 6)
			{
				if (item->ActiveState != 7)
				{
					if (item->ActiveState == 8)
					{
						item->Position.xRot = 0;
						item->Position.yPos = item->Floor;
					}
					else
					{
						item->AnimNumber = Objects[item->ObjectNumber].animIndex + 5;
						item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
						item->ActiveState = 6;
						item->Velocity = 0;
						item->Airborne = true;
						item->Position.xRot = 0;
					}
				}
				else if (item->Position.yPos >= item->Floor)
				{
					item->Position.yPos = item->Floor;
					item->VerticalVelocity = 0;
					item->Airborne = false;
					item->TargetState = 8;
				}
			}

			item->Position.xRot = 0;
		}
		else
		{
			AI_INFO aiInfo;
			CreatureAIInfo(item, &aiInfo);

			GetCreatureMood(item, &aiInfo, VIOLENT);

			if (info->flags)
				info->mood = MoodType::Escape;

			CreatureMood(item, &aiInfo, VIOLENT);

			angle = CreatureTurn(item, info->maximumTurn);

			if (item->HitStatus ||
				aiInfo.distance > pow(SECTOR(3), 2) ||
				!(GetRandomControl() & 0x7F))
			{
				info->flags = 0;
			}

			switch (item->ActiveState)
			{
			case 1:
				item->Position.yPos = item->Floor;
				info->maximumTurn = ANGLE(1.0f);

				if (item->HitStatus ||
					item->AIBits == MODIFY ||
					info->hurtByLara ||
					aiInfo.distance < pow(SECTOR(3), 2))
				{
					item->TargetState = 2;
				}

				break;

			case 3:
				info->maximumTurn = ANGLE(7.0f);

				if (item->RequiredState)
					item->TargetState = item->RequiredState;
				else if (aiInfo.ahead)
				{
					if (aiInfo.distance < pow(CLICK(1), 2))
						item->TargetState = 9;
				}

				break;

			case 4u:
				info->maximumTurn = ANGLE(7.0f);

				if (aiInfo.ahead)
				{
					if (aiInfo.distance < pow(CLICK(1), 2))
						item->TargetState = 4;
				}
				else if (aiInfo.distance < pow(CLICK(1), 2))
					item->TargetState = 9;
				else
				{
					item->RequiredState = 3;
					item->TargetState = 9;
				}

				if (!info->flags)
				{
					if (item->TouchBits & 0x60)
					{
						LaraItem->HitPoints -= 50;
						LaraItem->HitStatus = true;

						CreatureEffect2(
							item,
							&BigBeetleBite,
							5,
							-1,
							DoBloodSplat);

						info->flags = 1;
					}
				}

				break;

			case 5:
				info->flags = 0;

				item->Position.yPos += 51;
				if (item->Position.yPos > item->Floor)
					item->Position.yPos = item->Floor;

				break;

			case 9u:
				info->maximumTurn = ANGLE(7.0f);

				if (item->RequiredState)
					item->TargetState = item->RequiredState;
				else if (!item->HitStatus &&
					item->AIBits != MODIFY &&
					GetRandomControl() >= 384 &&
					(info->mood != MoodType::Bored && GetRandomControl() >= 128 ||
						info->hurtByLara ||
						item->AIBits == MODIFY))
				{
					if (aiInfo.ahead)
					{
						if (aiInfo.distance < pow(CLICK(1), 2) && !info->flags)
							item->TargetState = 4;
					}
				}
				else
					item->TargetState = 3;

				break;

			default:
				break;

			}
		}

		CreatureTilt(item, 2 * angle);
		CreatureAnimation(itemNumber, angle, angle);
	}
}
