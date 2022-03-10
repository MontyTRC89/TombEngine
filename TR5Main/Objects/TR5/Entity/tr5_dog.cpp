#include "framework.h"
#include "tr5_dog.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/control/control.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"
#include "Game/items.h"

static BYTE DogAnims[] = { 20, 21, 22, 20 };
static BITE_INFO DogBite = { 0, 0, 100, 3 };

void InitialiseTr5Dog(short itemNum)
{
    ITEM_INFO* item;

    item = &g_Level.Items[itemNum];
    item->ActiveState = 1;
    item->AnimNumber = Objects[item->ObjectNumber].animIndex + 8;
    if (!item->TriggerFlags)
    {
        item->AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
        // TODO: item->flags2 ^= (item->flags2 ^ ((item->flags2 & 0xFE) + 2)) & 6;
    }
    item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
}

void Tr5DogControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	short angle = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CreatureInfo* creature = (CreatureInfo*)item->Data;
	OBJECT_INFO* obj = &Objects[item->ObjectNumber];

	if (item->HitPoints <= 0)
	{
		if (item->AnimNumber == obj->animIndex + 1)
		{
			item->HitPoints = obj->HitPoints;
		}
		else if (item->ActiveState != 11)
		{
			item->AnimNumber = obj->animIndex + DogAnims[GetRandomControl() & 3];
			item->ActiveState = 11;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		}
	}
	else
	{
		if (item->AIBits)
			GetAITarget(creature);
		else
			creature->Enemy = LaraItem;

		AI_INFO info;
		CreatureAIInfo(item, &info);

		int distance;
		if (creature->Enemy == LaraItem)
		{
			distance = info.distance;
		}
		else
		{
			int dx = LaraItem->Position.xPos - item->Position.xPos;
			int dz = LaraItem->Position.zPos - item->Position.zPos;
			phd_atan(dz, dx);
			distance = SQUARE(dx) + SQUARE(dz);
		}

		if (info.ahead)
		{
			joint2 = info.xAngle; // Maybe swapped
			joint1 = info.angle;
		}

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		if (creature->Mood == MoodType::Bored)
			creature->MaxTurn /= 2;

		angle = CreatureTurn(item, creature->MaxTurn);
		joint0 = 4 * angle;


		if (creature->HurtByLara || distance < SQUARE(3072) && !(item->AIBits & MODIFY))
		{
			AlertAllGuards(itemNumber);
			item->AIBits &= ~MODIFY;
		}

		short random = GetRandomControl();
		int frame = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;

		switch (item->ActiveState)
		{
		case 0:
		case 8:
			joint1 = 0;
			joint2 = 0;
			if (creature->Mood != MoodType::Bored && (item->AIBits) != MODIFY)
			{
				item->TargetState = 1;
			}
			else
			{
				creature->Flags++;
				creature->MaxTurn = 0;
				if (creature->Flags > 300 && random < 128)
					item->TargetState = 1;
			}
			break;

		case 1:
		case 9:
			if (item->ActiveState == 9 && item->RequiredState)
			{
				item->TargetState = item->RequiredState;
				break;
			}

			creature->MaxTurn = 0;
			if (item->AIBits & GUARD)
			{
				joint1 = AIGuard(creature);
				if (GetRandomControl())
					break;
				if (item->ActiveState == 1)
				{
					item->TargetState = 9;
					break;
				}
			}
			else
			{
				if (item->ActiveState == 9 && random < 128)
				{
					item->TargetState = 1;
					break;
				}

				if (item->AIBits & PATROL1)
				{
					if (item->ActiveState == 1)
						item->TargetState = 2;
					else
						item->TargetState = 1;
					break;
				}

				if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity == item || !info.ahead || item->HitStatus)
					{
						item->RequiredState = 3;
						item->TargetState = 9;
					}
					else
					{
						item->TargetState = 1;
					}
					break;
				}

				if (creature->Mood != MoodType::Bored)
				{
					item->RequiredState = 3;
					if (item->ActiveState == 1)
						item->TargetState = 9;
					break;
				}

				creature->Flags = 0;
				creature->MaxTurn = ANGLE(1);

				if (random < 256)
				{
					if (item->AIBits & MODIFY)
					{
						if (item->ActiveState == 1)
						{
							item->TargetState = 8;
							creature->Flags = 0;
							break;
						}
					}
				}

				if (random >= 4096)
				{
					if (!(random & 0x1F))
						item->TargetState = 7;
					break;
				}

				if (item->ActiveState == 1)
				{
					item->TargetState = 2;
					break;
				}
			}
			item->TargetState = 1;
			break;

		case 2:
			creature->MaxTurn = ANGLE(3);
			if (item->AIBits & PATROL1)
			{
				item->TargetState = 2;
				break;
			}

			if (creature->Mood == MoodType::Bored && random < 256)
			{
				item->TargetState = 1;
				break;
			}
			item->TargetState = 5;
			break;

		case 3:
			creature->MaxTurn = ANGLE(6);
			if (creature->Mood == MoodType::Escape)
			{
				if (Lara.TargetEntity != item && info.ahead)
					item->TargetState = 9;
			}
			else if (creature->Mood != MoodType::Bored)
			{
				if (info.bite && info.distance < SQUARE(1024))
				{
					item->TargetState = 6;
				}
				else if (info.distance < SQUARE(1536))
				{
					item->RequiredState = 5;
					item->TargetState = 9;
				}
			}
			else
			{
				item->TargetState = 9;
			}
			break;

		case 5:
			creature->MaxTurn = ANGLE(3);
			if (creature->Mood != MoodType::Bored)
			{
				if (creature->Mood == MoodType::Escape)
				{
					item->TargetState = 3;
				}
				else if (info.bite && info.distance < SQUARE(341))
				{
					item->TargetState = 12;
					item->RequiredState = 5;
				}
				else if (info.distance > SQUARE(1536) || item->HitStatus)
				{
					item->TargetState = 3;
				}
			}
			else
			{
				item->TargetState = 9;
			}
			break;
		case 6:
			if (info.bite
				&& item->TouchBits & 0x6648
				&& frame >= 4
				&& frame <= 14)
			{
				CreatureEffect2(item, &DogBite, 2, -1, DoBloodSplat);
				LaraItem->HitPoints -= 20;
				LaraItem->HitStatus = true;
			}
			item->TargetState = 3;
			break;

		case 7:
			joint1 = 0;
			joint2 = 0;
			break;

		case 12:
			if (info.bite
				&& item->TouchBits & 0x48
				&& (frame >= 9
					&& frame <= 12
					|| frame >= 22
					&& frame <= 25))
			{
				CreatureEffect2(item, &DogBite, 2, -1, DoBloodSplat);
				LaraItem->HitPoints -= 10;
				LaraItem->HitStatus = true;
			}
			break;
		default:
			break;
		}
	}

	CreatureTilt(item, 0);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureAnimation(itemNumber, angle, 0);
}