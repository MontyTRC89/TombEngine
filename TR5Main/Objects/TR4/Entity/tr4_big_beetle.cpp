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

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 3;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
		item->Animation.TargetState = 1;
		item->Animation.ActiveState = 1;
	}

	void BigBeetleControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 6)
			{
				if (item->Animation.ActiveState != 7)
				{
					if (item->Animation.ActiveState == 8)
					{
						item->Position.xRot = 0;
						item->Position.yPos = item->Floor;
					}
					else
					{
						item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 5;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
						item->Animation.ActiveState = 6;
						item->Animation.Velocity = 0;
						item->Animation.Airborne = true;
						item->Position.xRot = 0;
					}
				}
				else if (item->Position.yPos >= item->Floor)
				{
					item->Position.yPos = item->Floor;
					item->Animation.VerticalVelocity = 0;
					item->Animation.Airborne = false;
					item->Animation.TargetState = 8;
				}
			}

			item->Position.xRot = 0;
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, VIOLENT);

			if (creature->Flags)
				creature->Mood = MoodType::Escape;

			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, creature->MaxTurn);

			if (item->HitStatus ||
				AI.distance > pow(SECTOR(3), 2) ||
				!(GetRandomControl() & 0x7F))
			{
				creature->Flags = 0;
			}

			switch (item->Animation.ActiveState)
			{
			case 1:
				item->Position.yPos = item->Floor;
				creature->MaxTurn = ANGLE(1.0f);

				if (item->HitStatus ||
					item->AIBits == MODIFY ||
					creature->HurtByLara ||
					AI.distance < pow(SECTOR(3), 2))
				{
					item->Animation.TargetState = 2;
				}

				break;

			case 3:
				creature->MaxTurn = ANGLE(7.0f);

				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (AI.ahead)
				{
					if (AI.distance < pow(CLICK(1), 2))
						item->Animation.TargetState = 9;
				}

				break;

			case 4u:
				creature->MaxTurn = ANGLE(7.0f);

				if (AI.ahead)
				{
					if (AI.distance < pow(CLICK(1), 2))
						item->Animation.TargetState = 4;
				}
				else if (AI.distance < pow(CLICK(1), 2))
					item->Animation.TargetState = 9;
				else
				{
					item->Animation.RequiredState = 3;
					item->Animation.TargetState = 9;
				}

				if (!creature->Flags)
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

						creature->Flags = 1;
					}
				}

				break;

			case 5:
				creature->Flags = 0;

				item->Position.yPos += 51;
				if (item->Position.yPos > item->Floor)
					item->Position.yPos = item->Floor;

				break;

			case 9u:
				creature->MaxTurn = ANGLE(7.0f);

				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (!item->HitStatus &&
					item->AIBits != MODIFY &&
					GetRandomControl() >= 384 &&
					(creature->Mood != MoodType::Bored && GetRandomControl() >= 128 ||
						creature->HurtByLara ||
						item->AIBits == MODIFY))
				{
					if (AI.ahead)
					{
						if (AI.distance < pow(CLICK(1), 2) && !creature->Flags)
							item->Animation.TargetState = 4;
					}
				}
				else
					item->Animation.TargetState = 3;

				break;

			default:
				break;

			}
		}

		CreatureTilt(item, 2 * angle);
		CreatureAnimation(itemNumber, angle, angle);
	}
}
