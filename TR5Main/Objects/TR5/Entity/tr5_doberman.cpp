#include "framework.h"
#include "tr5_doberman.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"
#include "Game/items.h"

BITE_INFO DobermanBite = { 0, 0x1E, 0x8D, 0x14 };

void InitialiseDoberman(short itemNum)
{
    ITEM_INFO* item;

    item = &g_Level.Items[itemNum];
    if (item->TriggerFlags)
    {
        item->Animation.ActiveState = 5;
        item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 6;
		// TODO: item->flags2 ^= (item->flags2 ^ ((item->flags2 & 0xFE) + 2)) & 6;
    }
    else
    {
        item->Animation.ActiveState = 6;
        item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 10;
    }
    item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
}

void DobermanControl(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		short angle = 0;
		short tilt = 0;
		short joint = 0;
		
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CreatureInfo* creature = (CreatureInfo*)item->Data;
		
		if (item->HitPoints > 0)
		{
			AI_INFO info;
			CreatureAIInfo(item, &info);

			if (info.ahead)
				joint = info.angle;

			GetCreatureMood(item, &info, TIMID);
			CreatureMood(item, &info, TIMID);

			angle = CreatureTurn(item, creature->MaxTurn);
		
			int random;

			switch (item->Animation.ActiveState)
			{
			case 1:
				creature->MaxTurn = ANGLE(3);
				if (creature->Mood != MoodType::Bored)
				{
					item->Animation.TargetState = 2;
				}
				else
				{
					random = GetRandomControl();
					if (random < 768)
					{
						item->Animation.RequiredState = 4;
						item->Animation.TargetState = 3;
						break;
					}
					if (random < 1536)
					{
						item->Animation.RequiredState = 5;
						item->Animation.TargetState = 3;
						break;
					}
					if (random < 2816)
					{
						item->Animation.TargetState = 3;
						break;
					}
				}
				break;

			case 2:
				tilt = angle;
				creature->MaxTurn = ANGLE(6);
				if (creature->Mood == MoodType::Bored)
				{
					item->Animation.TargetState = 3;
					break;
				}
				if (info.distance < SQUARE(768))
					item->Animation.TargetState = 8;
				break;

			case 3:
				creature->MaxTurn = 0;
				creature->Flags = 0;
				if (creature->Mood != MoodType::Bored)
				{
					if (creature->Mood != MoodType::Escape 
						&& info.distance < SQUARE(341)
						&& info.ahead)
						item->Animation.TargetState = 7;
					else
						item->Animation.TargetState = 2;
				}
				else
				{
					if (item->Animation.RequiredState)
					{
						item->Animation.TargetState = item->Animation.RequiredState;
					}
					else
					{
						random = GetRandomControl();
						if (random >= 768)
						{
							if (random >= 1536)
							{
								if (random < 9728)
									item->Animation.TargetState = 1;
							}
							else
							{
								item->Animation.TargetState = 5;
							}
						}
						else
						{
							item->Animation.TargetState = 4;
						}
					}
				}
				break;

			case 4:
				if (creature->Mood != MoodType::Bored || GetRandomControl() < 1280)
				{
					item->Animation.TargetState = 3;
				}
				break;

			case 5:
				if (creature->Mood != MoodType::Bored || GetRandomControl() < 256)
				{
					item->Animation.TargetState = 3;
				}
				break;

			case 6:
				if (creature->Mood != MoodType::Bored || GetRandomControl() < 512)
				{
					item->Animation.TargetState = 3;
				}
				break;

			case 7:
				creature->MaxTurn = ANGLE(1) / 2;
				if (creature->Flags != 1 
					&& info.ahead 
					&& item->TouchBits & 0x122000)
				{
					CreatureEffect(item, &DobermanBite, DoBloodSplat);
					LaraItem->HitPoints -= 30;
					LaraItem->HitStatus = true;
					creature->Flags = 1;
				}

				if (info.distance <= SQUARE(341) || info.distance >= SQUARE(682))
					item->Animation.TargetState = 3;
				else
					item->Animation.TargetState = 9;
				break;

			case 8:
				if (creature->Flags != 2 && item->TouchBits & 0x122000)
				{
					CreatureEffect(item, &DobermanBite, DoBloodSplat);
					LaraItem->HitPoints -= 80;
					LaraItem->HitStatus = true;
					creature->Flags = 2;
				}
				if (info.distance >= SQUARE(341))
				{
					if (info.distance < SQUARE(682))
						item->Animation.TargetState = 9;
				}
				else
				{
					item->Animation.TargetState = 7;
				}
				break;
			case 9:
				creature->MaxTurn = ANGLE(6);
				if (creature->Flags != 3 && item->TouchBits & 0x122000)
				{
					CreatureEffect(item, &DobermanBite, DoBloodSplat);
					LaraItem->HitPoints -= 50;
					LaraItem->HitStatus = true;
					creature->Flags = 3;
				}
				if (info.distance < SQUARE(341))
					item->Animation.TargetState = 7;
				break;
			default:
				break;
			}
		}
		else if (item->Animation.ActiveState != 10)
		{
			item->Animation.AnimNumber = Objects[ID_DOG].animIndex + 13;
			item->Animation.ActiveState = 10;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, 0);
		CreatureJoint(item, 1, joint);
		CreatureJoint(item, 2, 0);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
