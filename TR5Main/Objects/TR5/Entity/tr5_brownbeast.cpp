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
    item->animNumber = Objects[item->objectNumber].animIndex;
    item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
    item->goalAnimState = 1;
    item->currentAnimState = 1;
}

void ControlBrowsBeast(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

		short angle  = 0;

		if (item->hitPoints <= 0)
		{
			item->hitPoints = 0;
			if (item->currentAnimState != 7)
			{
				item->animNumber = Objects[ID_BROWN_BEAST].animIndex + 10;
				item->currentAnimState = 7;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			}
		}
		else
		{
			if (item->aiBits)
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
				int dx = LaraItem->pos.xPos - item->pos.xPos;
				int dz = LaraItem->pos.zPos - item->pos.zPos;
				phd_atan(dz, dz);
				distance = SQUARE(dx) + SQUARE(dz);
			}

			GetCreatureMood(item, &info, VIOLENT);
			CreatureMood(item, &info, VIOLENT);

			angle = CreatureTurn(item, creature->maximumTurn);
			creature->maximumTurn = ANGLE(7);
			switch (item->currentAnimState)
			{
			case 1:
				creature->flags = 0;
				if (creature->mood == ATTACK_MOOD)
				{
					if (distance <= SQUARE(1024))
					{
						if (GetRandomControl() & 1)
							item->goalAnimState = 4;
						else
							item->goalAnimState = 6;
					}
					else if (GetRandomControl() & 1)
					{
						item->goalAnimState = 2;
					}
					else
					{
						item->goalAnimState = 3;
					}
				}
				else
				{
					item->goalAnimState = 1;
				}
				break;

			case 2:
			case 3:
				if (distance < SQUARE(1024) || creature->mood != ATTACK_MOOD)
					item->goalAnimState = 1;
				SoundEffect(SFX_TR5_IMP_BARREL_ROLL, &item->pos, 0);
				break;

			case 4:
			case 6:
				creature->maximumTurn = 0;
				if (abs(info.angle) >= ANGLE(2))
				{
					if (info.angle > 0)
						item->pos.yRot += ANGLE(2);
					else
						item->pos.yRot -= ANGLE(2);
				}
				else
				{
					item->pos.yRot += info.angle;
				}

				if (creature->flags)
					break;

				if (item->touchBits & 0x3C000)
				{
					if (item->animNumber == Objects[ID_BROWN_BEAST].animIndex + 8)
					{
						if (item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 19
							&& item->frameNumber < g_Level.Anims[item->animNumber].frameBase + 25)
						{
							CreatureEffect2(item, &BrownBeastBite1, 20, item->pos.yRot, DoBloodSplat);
							LaraItem->hitPoints -= 150;
							LaraItem->hitStatus = true;
							creature->flags |= 1;
							break;
						}
					}

					if (item->animNumber == Objects[ID_BROWN_BEAST].animIndex + 2)
					{
						if (item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 6 
							&& item->frameNumber < g_Level.Anims[item->animNumber].frameBase + 16)
						{
							CreatureEffect2(item, &BrownBeastBite1, 20, item->pos.yRot, DoBloodSplat);
							LaraItem->hitPoints -= 150;
							LaraItem->hitStatus = true;
							creature->flags |= 1;
							break;
						}
					}
				}

				if (!(item->touchBits & 0xF00000))
					break;

				if (item->animNumber == Objects[ID_BROWN_BEAST].animIndex + 8)
				{
					if (item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 13
						&& item->frameNumber < g_Level.Anims[item->animNumber].frameBase + 20)
					{
						CreatureEffect2(item, &BrownBeastBite2, 20, item->pos.yRot, DoBloodSplat);
						LaraItem->hitPoints -= 150;
						LaraItem->hitStatus = true;
						creature->flags |= 2;
						break;
					}
				}

				if (item->animNumber == Objects[ID_BROWN_BEAST].animIndex + 2)
				{
					if (item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 33
						&& item->frameNumber < g_Level.Anims[item->animNumber].frameBase + 43)
					{
						CreatureEffect2(item, &BrownBeastBite2, 20, item->pos.yRot, DoBloodSplat);
						LaraItem->hitPoints -= 150;
						LaraItem->hitStatus = true;
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