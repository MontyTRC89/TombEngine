#include "framework.h"
#include "tr5_brownbeast.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

BITE_INFO BrownBeastBite1 = { 0, 0, 0, 16 };
BITE_INFO BrownBeastBite2 = { 0, 0, 0, 22 };

void InitialiseBrownBeast(short itemNum)
{
    ITEM_INFO* item;

    item = &g_Level.Items[itemNum];
    ClearItem(itemNum);
    item->AnimNumber = Objects[item->ObjectNumber].animIndex;
    item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
    item->TargetState = 1;
    item->ActiveState = 1;
}

void ControlBrowsBeast(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;

		short angle  = 0;

		if (item->HitPoints <= 0)
		{
			item->HitPoints = 0;
			if (item->ActiveState != 7)
			{
				item->AnimNumber = Objects[ID_BROWN_BEAST].animIndex + 10;
				item->ActiveState = 7;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			}
		}
		else
		{
			if (item->AIBits)
			{
				GetAITarget(creature);
			}
			else if (creature->hurtByLara)
			{
				creature->enemy = LaraItem;
			}

			AI_INFO info;
			CreatureAIInfo(item, &info);

			int distance;

			if (creature->enemy == LaraItem)
			{
				distance = info.distance;
			}
			else
			{
				int dx = LaraItem->Position.xPos - item->Position.xPos;
				int dz = LaraItem->Position.zPos - item->Position.zPos;
				phd_atan(dz, dz);
				distance = SQUARE(dx) + SQUARE(dz);
			}

			GetCreatureMood(item, &info, VIOLENT);
			CreatureMood(item, &info, VIOLENT);

			angle = CreatureTurn(item, creature->maximumTurn);
			creature->maximumTurn = ANGLE(7);
			switch (item->ActiveState)
			{
			case 1:
				creature->flags = 0;
				if (creature->mood == ATTACK_MOOD)
				{
					if (distance <= SQUARE(1024))
					{
						if (GetRandomControl() & 1)
							item->TargetState = 4;
						else
							item->TargetState = 6;
					}
					else if (GetRandomControl() & 1)
					{
						item->TargetState = 2;
					}
					else
					{
						item->TargetState = 3;
					}
				}
				else
				{
					item->TargetState = 1;
				}
				break;

			case 2:
			case 3:
				if (distance < SQUARE(1024) || creature->mood != ATTACK_MOOD)
					item->TargetState = 1;
				SoundEffect(SFX_TR5_IMP_BARREL_ROLL, &item->Position, 0);
				break;

			case 4:
			case 6:
				creature->maximumTurn = 0;
				if (abs(info.angle) >= ANGLE(2))
				{
					if (info.angle > 0)
						item->Position.yRot += ANGLE(2);
					else
						item->Position.yRot -= ANGLE(2);
				}
				else
				{
					item->Position.yRot += info.angle;
				}

				if (creature->flags)
					break;

				if (item->TouchBits & 0x3C000)
				{
					if (item->AnimNumber == Objects[ID_BROWN_BEAST].animIndex + 8)
					{
						if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 19
							&& item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 25)
						{
							CreatureEffect2(item, &BrownBeastBite1, 20, item->Position.yRot, DoBloodSplat);
							LaraItem->HitPoints -= 150;
							LaraItem->HitStatus = true;
							creature->flags |= 1;
							break;
						}
					}

					if (item->AnimNumber == Objects[ID_BROWN_BEAST].animIndex + 2)
					{
						if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 6 
							&& item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 16)
						{
							CreatureEffect2(item, &BrownBeastBite1, 20, item->Position.yRot, DoBloodSplat);
							LaraItem->HitPoints -= 150;
							LaraItem->HitStatus = true;
							creature->flags |= 1;
							break;
						}
					}
				}

				if (!(item->TouchBits & 0xF00000))
					break;

				if (item->AnimNumber == Objects[ID_BROWN_BEAST].animIndex + 8)
				{
					if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 13
						&& item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 20)
					{
						CreatureEffect2(item, &BrownBeastBite2, 20, item->Position.yRot, DoBloodSplat);
						LaraItem->HitPoints -= 150;
						LaraItem->HitStatus = true;
						creature->flags |= 2;
						break;
					}
				}

				if (item->AnimNumber == Objects[ID_BROWN_BEAST].animIndex + 2)
				{
					if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 33
						&& item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 43)
					{
						CreatureEffect2(item, &BrownBeastBite2, 20, item->Position.yRot, DoBloodSplat);
						LaraItem->HitPoints -= 150;
						LaraItem->HitStatus = true;
						creature->flags |= 2;
						break;
					}
				}

				break;

			default:
				break;

			}
		}

		CreatureAnimation(itemNumber, angle, 0);
	}
}