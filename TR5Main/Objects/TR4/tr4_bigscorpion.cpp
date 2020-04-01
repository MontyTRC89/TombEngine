#include "../newobjects.h"
#include "../../Game/Box.h"
#include "../../Game/effects.h"
#include "../../Game/items.h"
#include "../../Specific/setup.h"
#include "../../Game/lot.h"

BITE_INFO scorpionBite1 = { 0, 0, 0, 8 };
BITE_INFO scorpionBite2 = { 0, 0, 0, 23 };

void InitialiseScorpion(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	if (item->triggerFlags == 1)
	{
		item->goalAnimState = 8;
		item->currentAnimState = 8;
		item->animNumber = Objects[ID_BIG_SCORPION].animIndex + 7;
	}
	else
	{
		item->goalAnimState = 1;
		item->currentAnimState = 1;
		item->animNumber = Objects[ID_BIG_SCORPION].animIndex + 2;
	}

	item->frameNumber = Anims[item->animNumber].frameBase;
}

void ScorpionControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	short angle = 0;
	short head = 0;
	short neck = 0;
	short tilt = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short joint3 = 0;
	short roomNumber = item->roomNumber;

	int x = item->pos.xPos + 682 * SIN(item->pos.yRot) >> W2V_SHIFT;
	int z = item->pos.zPos + 682 * COS(item->pos.yRot) >> W2V_SHIFT;

	FLOOR_INFO* floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, item->pos.yPos, z);
	if (abs(item->pos.yPos - height1) > 512)
		height1 = item->pos.yPos;

	x = item->pos.xPos - 682 * SIN(item->pos.yRot) >> W2V_SHIFT;
	z = item->pos.zPos - 682 * COS(item->pos.yRot) >> W2V_SHIFT;

	floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, item->pos.yPos, z);
	if (abs(item->pos.yPos - height2) > 512)
		height2 = item->pos.yPos;

	short angle1 = ATAN(1344, height2 - height1);

	x = item->pos.xPos - 682 * SIN(item->pos.yRot) >> W2V_SHIFT;
	z = item->pos.zPos + 682 * COS(item->pos.yRot) >> W2V_SHIFT;

	floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	int height3 = GetFloorHeight(floor, x, item->pos.yPos, z);
	if (abs(item->pos.yPos - height3) > 512)
		height3 = item->pos.yPos;

	x = item->pos.xPos + 682 * SIN(item->pos.yRot) >> W2V_SHIFT;
	z = item->pos.zPos - 682 * COS(item->pos.yRot) >> W2V_SHIFT;

	floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	int height4 = GetFloorHeight(floor, x, item->pos.yPos, z);
	if (abs(item->pos.yPos - height4) > 512)
		height4 = item->pos.yPos;

	short angle2 = ATAN(1344, height4 - height3);

	if (item->hitPoints <= 0)
	{
		item->hitPoints = 0;

		if (item->currentAnimState != 6 && item->currentAnimState != 7)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 5;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 6;
		}
	}
	else
	{
		if (item->aiBits)
			GetAITarget(creature);
		else
		{
			if (creature->hurtByLara && item->currentAnimState != 8)
				creature->enemy = LaraItem;
			{
				// Search for active troops
				creature->enemy = NULL;
				CREATURE_INFO* baddy = &BaddieSlots[0];
				int minDistance = 0x7FFFFFFF;

				for (int i = 0; i < NUM_SLOTS; i++)
				{
					baddy = &BaddieSlots[i];

					if (baddy->itemNum != NO_ITEM && baddy->itemNum != itemNum)
					{
						ITEM_INFO* currentItem = &Items[baddy->itemNum];

						if (currentItem->objectNumber != ID_BIG_SCORPION &&
							(currentItem != LaraItem || creature->hurtByLara))
						{
							int dx = currentItem->pos.xPos - item->pos.xPos;
							int dy = currentItem->pos.yPos - item->pos.yPos;
							int dz = currentItem->pos.zPos - item->pos.zPos;
							int distance = dx * dx + dy * dy + dz * dz;

							if (distance < minDistance)
							{
								minDistance = distance;
								creature->enemy = currentItem;
							}
						}
					}
				}
			}
		}


		AI_INFO info;

		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		switch (item->currentAnimState)
		{
		case 1:
			creature->maximumTurn = 0;
			creature->flags = 0;

			if (info.distance > SQUARE(1365))
			{
				item->goalAnimState = 2;
				break;
			}

			if (info.bite)
			{
				creature->maximumTurn = ANGLE(2);

				if (GetRandomControl() & 1 || creature->enemy->objectNumber == ID_TROOPS &&
					creature->enemy->hitPoints <= 15)
				{
					item->goalAnimState = 4;
				}
				else
				{
					item->goalAnimState = 5;
				}
			}
			else if (!info.ahead)
			{
				item->goalAnimState = 2;
			}

			break;

		case 2:
			creature->maximumTurn = ANGLE(2);

			if (info.distance < SQUARE(1365))
			{
				item->goalAnimState = 1;
				break;
			}

			if (info.distance > SQUARE(853))
			{
				item->goalAnimState = 3;
			}

			break;

		case 3:
			creature->maximumTurn = ANGLE(3);

			if (info.distance >= SQUARE(1365))
			{
				break;
			}

			item->goalAnimState = 1;

			break;

		case 4:
		case 5:
			creature->maximumTurn = 0;

			if (abs(info.angle) >= ANGLE(2))
			{
				if (info.angle >= 0)
				{
					item->pos.yRot += ANGLE(2);
				}
				else
				{
					item->pos.yRot -= ANGLE(2);
				}
			}
			else
			{
				item->pos.yRot += info.angle;
			}
			if (creature->flags)
			{
				break;
			}

			if (creature->enemy && creature->enemy != LaraItem && info.distance < SQUARE(1365))
			{
				creature->enemy->hitPoints -= 15;
				if (creature->enemy->hitPoints <= 0)
				{
					item->goalAnimState = 7;
					creature->maximumTurn = 0;
				}

				creature->enemy->hitStatus = true;
				creature->flags = 1;

				CreatureEffect2(
					item,
					&scorpionBite1,
					10,
					item->pos.yRot - ANGLE(180),
					DoBloodSplat);
			}
			else if (item->touchBits & 0x1B00100)
			{
				LaraItem->hitPoints -= 120;
				LaraItem->hitStatus = true;

				if (item->currentAnimState == 5)
				{
					Lara.dpoisoned += 2048;

					CreatureEffect2(
						item,
						&scorpionBite1,
						10,
						item->pos.yRot - ANGLE(180),
						DoBloodSplat);
				}
				else
				{
					CreatureEffect2(
						item,
						&scorpionBite2,
						10,
						item->pos.yRot - ANGLE(180),
						DoBloodSplat);
				}

				creature->flags = 1;
				if (LaraItem->hitPoints <= 0)
				{
					CreatureKill(item, 6, 7, 442);
					creature->maximumTurn = 0;
					return;
				}
			}

			break;

		case 8:
			creature->maximumTurn = 0;
			if (item->frameNumber == Anims[item->animNumber].frameEnd)
			{
				item->triggerFlags++;
			}
			if (creature->enemy && creature->enemy->hitPoints <= 0 || item->triggerFlags > 6)
			{
				item->goalAnimState = 7;
				creature->enemy->hitPoints = 0;
			}

			break;

		default:
			break;
		}
	}

	if ((angle1 - item->pos.xRot) < 256)
		item->pos.xRot = 256;
	else
	{
		if (angle1 <= item->pos.xRot)
			item->pos.xRot -= 256;
		else
			item->pos.xRot += 256;
	}

	if ((angle2 - item->pos.zRot) < 256)
		item->pos.zRot = 256;
	else
	{
		if (angle2 <= item->pos.zRot)
			item->pos.zRot -= 256;
		else
			item->pos.zRot += 256;
	}

	CreatureAnimation(itemNum, angle, 0);
}