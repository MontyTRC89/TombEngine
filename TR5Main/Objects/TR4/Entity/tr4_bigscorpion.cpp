#include "framework.h"
#include "tr4_bigscorpion.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Specific/setup.h"
#include "Game/control/lot.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"

int CutSeqNum;

BITE_INFO scorpionBite1 = { 0, 0, 0, 8 };
BITE_INFO scorpionBite2 = { 0, 0, 0, 23 };

enum SCORPION_STATES {
	STATE_SCORPION_STOP = 1,
	STATE_SCORPION_WALK = 2,
	STATE_SCORPION_RUN = 3,
	STATE_SCORPION_ATTACK1 = 4,
	STATE_SCORPION_ATTACK2 = 5,
	STATE_SCORPION_DEATH = 6,
	STATE_SCORPION_SPECIAL_DEATH = 7,
	STATE_SCORPION_TROOPS_ATTACK = 8
};

void InitialiseScorpion(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	if (item->triggerFlags == 1)
	{
		item->goalAnimState = STATE_SCORPION_TROOPS_ATTACK;
		item->currentAnimState = STATE_SCORPION_TROOPS_ATTACK;
		item->animNumber = Objects[ID_BIG_SCORPION].animIndex + 7;
	}
	else
	{
		item->goalAnimState = STATE_SCORPION_STOP;
		item->currentAnimState = STATE_SCORPION_STOP;
		item->animNumber = Objects[ID_BIG_SCORPION].animIndex + 2;
	}

	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
}

void ScorpionControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
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

	int x = item->pos.xPos + 682 * phd_sin(item->pos.yRot);
	int z = item->pos.zPos + 682 * phd_cos(item->pos.yRot);

	FLOOR_INFO* floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, item->pos.yPos, z);
	if (abs(item->pos.yPos - height1) > 512)
		height1 = item->pos.yPos;

	x = item->pos.xPos - 682 * phd_sin(item->pos.yRot);
	z = item->pos.zPos - 682 * phd_cos(item->pos.yRot);

	floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, item->pos.yPos, z);
	if (abs(item->pos.yPos - height2) > 512)
		height2 = item->pos.yPos;

	short angle1 = phd_atan(1344, height2 - height1);

	x = item->pos.xPos - 682 * phd_sin(item->pos.yRot);
	z = item->pos.zPos + 682 * phd_cos(item->pos.yRot);

	floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	int height3 = GetFloorHeight(floor, x, item->pos.yPos, z);
	if (abs(item->pos.yPos - height3) > 512)
		height3 = item->pos.yPos;

	x = item->pos.xPos + 682 * phd_sin(item->pos.yRot);
	z = item->pos.zPos - 682 * phd_cos(item->pos.yRot);

	floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	int height4 = GetFloorHeight(floor, x, item->pos.yPos, z);
	if (abs(item->pos.yPos - height4) > 512)
		height4 = item->pos.yPos;

	short angle2 = phd_atan(1344, height4 - height3);

	if (item->hitPoints <= 0)
	{
		item->hitPoints = 0;
		if (item->currentAnimState != STATE_SCORPION_DEATH)
		{
			if (item->triggerFlags > 0 && item->triggerFlags < 7)
			{
				CutSeqNum = 4;

				item->animNumber = Objects[item->animNumber].animIndex + 5;
				item->currentAnimState = STATE_SCORPION_DEATH;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->status = ITEM_INVISIBLE;
				creature->maximumTurn = 0;
				
				short linkNum = g_Level.Rooms[item->roomNumber].itemNumber;
				if (linkNum != NO_ITEM)
				{
					for (linkNum = g_Level.Rooms[item->roomNumber].itemNumber; linkNum != NO_ITEM; linkNum = g_Level.Items[linkNum].nextItem)
					{
						ITEM_INFO* currentItem = &g_Level.Items[linkNum];
						
						if (currentItem->objectNumber == ID_TROOPS && currentItem->triggerFlags == 1)
						{
							DisableBaddieAI(linkNum);
							KillItem(linkNum);
							currentItem->flags |= IFLAG_KILLED;
							break;
						}
					}
				}
			}
			else if (item->currentAnimState != STATE_SCORPION_DEATH && item->currentAnimState != STATE_SCORPION_SPECIAL_DEATH)
			{
				item->animNumber = Objects[item->objectNumber].animIndex + 5;
				item->currentAnimState = STATE_SCORPION_DEATH;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			}
		}
		else if (CutSeqNum == 4)
		{
			item->frameNumber = g_Level.Anims[item->animNumber].frameEnd - 1;
			item->status = ITEM_INVISIBLE;
		}
		else if (item->currentAnimState == STATE_SCORPION_DEATH)
		{
			if (item->status == ITEM_INVISIBLE)
			{
				item->status = ITEM_ACTIVE;
			}
		}
	}
	else
	{
		if (item->aiBits)
			GetAITarget(creature);
		else
		{
			if (creature->hurtByLara 
				&& item->currentAnimState != STATE_SCORPION_TROOPS_ATTACK)
			{
				creature->enemy = LaraItem;
			}
			else
			{
				creature->enemy = NULL;
				int minDistance = 0x7FFFFFFF;

				for (int i = 0; i < ActiveCreatures.size(); i++)
				{
					CREATURE_INFO* baddy = ActiveCreatures[i];

					if (baddy->itemNum != NO_ITEM && baddy->itemNum != itemNumber)
					{
						ITEM_INFO* currentItem = &g_Level.Items[baddy->itemNum];

						if (currentItem->objectNumber != ID_LARA)
						{
							if (currentItem->objectNumber != ID_BIG_SCORPION &&
								(currentItem != LaraItem || creature->hurtByLara))
							{
								int dx = currentItem->pos.xPos - item->pos.xPos;
								int dy = currentItem->pos.yPos - item->pos.yPos;
								int dz = currentItem->pos.zPos - item->pos.zPos;

								int distance = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);

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
		}

		AI_INFO info;

		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		switch (item->currentAnimState)
		{
		case STATE_SCORPION_STOP:
			creature->maximumTurn = 0;
			creature->flags = 0;

			if (info.distance > SQUARE(1365))
			{
				item->goalAnimState = STATE_SCORPION_WALK;
				break;
			}

			if (info.bite)
			{
				creature->maximumTurn = ANGLE(2);

				if (GetRandomControl() & 1 
					&& creature->enemy->hitPoints <= 15
					&& creature->enemy->objectNumber == ID_TROOPS)
				{
					item->goalAnimState = STATE_SCORPION_ATTACK1;
				}
				else
				{
					item->goalAnimState = STATE_SCORPION_ATTACK2;
				}
			}
			else if (!info.ahead)
			{
				item->goalAnimState = STATE_SCORPION_WALK;
			}

			break;

		case STATE_SCORPION_WALK:
			creature->maximumTurn = ANGLE(2);

			if (info.distance < SQUARE(1365))
			{
				item->goalAnimState = STATE_SCORPION_STOP;
			}
			else if (info.distance > SQUARE(853))
			{
				item->goalAnimState = STATE_SCORPION_RUN;
			}

			break;

		case STATE_SCORPION_RUN:
			creature->maximumTurn = ANGLE(3);

			if (info.distance < SQUARE(1365))
			{
				item->goalAnimState = STATE_SCORPION_STOP;
			}

			break;

		case STATE_SCORPION_ATTACK1:
		case STATE_SCORPION_ATTACK2:
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

			if (creature->enemy 
				&& creature->enemy != LaraItem 
				&& info.distance < SQUARE(1365))
			{
				creature->enemy->hitPoints -= 15;
				if (creature->enemy->hitPoints <= 0)
				{
					item->goalAnimState = STATE_SCORPION_SPECIAL_DEATH;
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
					Lara.poisoned += 2048;

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

		case STATE_SCORPION_TROOPS_ATTACK:
			creature->maximumTurn = 0;
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
			{
				item->triggerFlags++;
			}
			if (creature->enemy 
				&& creature->enemy->hitPoints <= 0 
				|| item->triggerFlags > 6)
			{
				item->goalAnimState = STATE_SCORPION_SPECIAL_DEATH;
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

	if (!CutSeqNum)
		CreatureAnimation(itemNumber, angle, 0);
}
