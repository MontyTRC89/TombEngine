#include "framework.h"
#include "tr4_big_beetle.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Specific/trmath.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Game/itemdata/creature_info.h"

namespace TEN::Entities::TR4
{
	BITE_INFO BitBeetleBite = { 0,0,0,12 };

	void InitialiseBigBeetle(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

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

		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;

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
						item->Airborne = true;
						item->ActiveState = 6;
						item->Velocity = 0;
						item->Position.xRot = 0;
					}
				}
				else if (item->Position.yPos >= item->Floor)
				{
					item->Position.yPos = item->Floor;
					item->Airborne = false;
					item->VerticalVelocity = 0;
					item->TargetState = 8;
				}
			}
			item->Position.xRot = 0;
		}
		else
		{
			AI_INFO info;
			CreatureAIInfo(item, &info);

			GetCreatureMood(item, &info, VIOLENT);
			if (creature->flags)
				creature->mood = ESCAPE_MOOD;
			CreatureMood(item, &info, VIOLENT);

			angle = CreatureTurn(item, creature->maximumTurn);

			if (info.distance > SQUARE(3072)
				|| !(GetRandomControl() & 0x7F)
				|| item->HitStatus)
			{
				creature->flags = 0;
			}

			switch (item->ActiveState)
			{
			case 1:
				item->Position.yPos = item->Floor;
				creature->maximumTurn = ANGLE(1);

				if (item->HitStatus
					|| info.distance < SQUARE(3072)
					|| creature->hurtByLara
					|| item->AIBits == MODIFY)
				{
					item->TargetState = 2;
				}

				break;

			case 3:
				creature->maximumTurn = ANGLE(7);

				if (item->RequiredState)
				{
					item->TargetState = item->RequiredState;
				}
				else if (info.ahead)
				{
					if (info.distance < SQUARE(256))
					{
						item->TargetState = 9;
					}
				}

				break;

			case 4u:
				creature->maximumTurn = ANGLE(7);

				if (info.ahead)
				{
					if (info.distance < SQUARE(256))
					{
						item->TargetState = 4;
					}
				}
				else if (info.distance < SQUARE(256))
				{
					item->TargetState = 9;
				}
				else
				{
					item->RequiredState = 3;
					item->TargetState = 9;
				}

				if (!creature->flags)
				{
					if (item->TouchBits & 0x60)
					{
						LaraItem->HitPoints -= 50;
						LaraItem->HitStatus = true;
						CreatureEffect2(
							item,
							&BitBeetleBite,
							5,
							-1,
							DoBloodSplat);
						creature->flags = 1;
					}
				}

				break;

			case 5:
				creature->flags = 0;

				item->Position.yPos += 51;
				if (item->Position.yPos > item->Floor)
					item->Position.yPos = item->Floor;

				break;

			case 9u:
				creature->maximumTurn = ANGLE(7);

				if (item->RequiredState)
				{
					item->TargetState = item->RequiredState;
				}
				else if (!item->HitStatus
					&& GetRandomControl() >= 384
					&& item->AIBits != MODIFY
					&& (creature->mood && GetRandomControl() >= 128
						|| creature->hurtByLara
						|| item->AIBits == MODIFY))
				{
					if (info.ahead)
					{
						if (info.distance < SQUARE(256) && !creature->flags)
						{
							item->TargetState = 4;
						}
					}
				}
				else
				{
					item->TargetState = 3;
				}

				break;

			default:
				break;

			}
		}

		CreatureTilt(item, 2 * angle);
		CreatureAnimation(itemNumber, angle, angle);
	}
}
