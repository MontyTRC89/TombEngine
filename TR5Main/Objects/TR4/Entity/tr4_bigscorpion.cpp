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

	if (item->TriggerFlags == 1)
	{
		item->TargetState = STATE_SCORPION_TROOPS_ATTACK;
		item->ActiveState = STATE_SCORPION_TROOPS_ATTACK;
		item->AnimNumber = Objects[ID_BIG_SCORPION].animIndex + 7;
	}
	else
	{
		item->TargetState = STATE_SCORPION_STOP;
		item->ActiveState = STATE_SCORPION_STOP;
		item->AnimNumber = Objects[ID_BIG_SCORPION].animIndex + 2;
	}

	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
}

void ScorpionControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
	short angle = 0;
	short head = 0;
	short neck = 0;
	short tilt = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short joint3 = 0;
	short roomNumber = item->RoomNumber;

	int x = item->Position.xPos + 682 * phd_sin(item->Position.yRot);
	int z = item->Position.zPos + 682 * phd_cos(item->Position.yRot);

	FLOOR_INFO* floor = GetFloor(x, item->Position.yPos, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, item->Position.yPos, z);
	if (abs(item->Position.yPos - height1) > 512)
		height1 = item->Position.yPos;

	x = item->Position.xPos - 682 * phd_sin(item->Position.yRot);
	z = item->Position.zPos - 682 * phd_cos(item->Position.yRot);

	floor = GetFloor(x, item->Position.yPos, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, item->Position.yPos, z);
	if (abs(item->Position.yPos - height2) > 512)
		height2 = item->Position.yPos;

	short angle1 = phd_atan(1344, height2 - height1);

	x = item->Position.xPos - 682 * phd_sin(item->Position.yRot);
	z = item->Position.zPos + 682 * phd_cos(item->Position.yRot);

	floor = GetFloor(x, item->Position.yPos, z, &roomNumber);
	int height3 = GetFloorHeight(floor, x, item->Position.yPos, z);
	if (abs(item->Position.yPos - height3) > 512)
		height3 = item->Position.yPos;

	x = item->Position.xPos + 682 * phd_sin(item->Position.yRot);
	z = item->Position.zPos - 682 * phd_cos(item->Position.yRot);

	floor = GetFloor(x, item->Position.yPos, z, &roomNumber);
	int height4 = GetFloorHeight(floor, x, item->Position.yPos, z);
	if (abs(item->Position.yPos - height4) > 512)
		height4 = item->Position.yPos;

	short angle2 = phd_atan(1344, height4 - height3);

	if (item->HitPoints <= 0)
	{
		item->HitPoints = 0;
		if (item->ActiveState != STATE_SCORPION_DEATH)
		{
			if (item->TriggerFlags > 0 && item->TriggerFlags < 7)
			{
				CutSeqNum = 4;

				item->AnimNumber = Objects[item->AnimNumber].animIndex + 5;
				item->ActiveState = STATE_SCORPION_DEATH;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				item->Status = ITEM_INVISIBLE;
				creature->maximumTurn = 0;
				
				short linkNum = g_Level.Rooms[item->RoomNumber].itemNumber;
				if (linkNum != NO_ITEM)
				{
					for (linkNum = g_Level.Rooms[item->RoomNumber].itemNumber; linkNum != NO_ITEM; linkNum = g_Level.Items[linkNum].NextItem)
					{
						ITEM_INFO* currentItem = &g_Level.Items[linkNum];
						
						if (currentItem->ObjectNumber == ID_TROOPS && currentItem->TriggerFlags == 1)
						{
							DisableBaddieAI(linkNum);
							KillItem(linkNum);
							currentItem->Flags |= IFLAG_KILLED;
							break;
						}
					}
				}
			}
			else if (item->ActiveState != STATE_SCORPION_DEATH && item->ActiveState != STATE_SCORPION_SPECIAL_DEATH)
			{
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + 5;
				item->ActiveState = STATE_SCORPION_DEATH;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			}
		}
		else if (CutSeqNum == 4)
		{
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameEnd - 1;
			item->Status = ITEM_INVISIBLE;
		}
		else if (item->ActiveState == STATE_SCORPION_DEATH)
		{
			if (item->Status == ITEM_INVISIBLE)
			{
				item->Status = ITEM_ACTIVE;
			}
		}
	}
	else
	{
		if (item->AIBits)
			GetAITarget(creature);
		else
		{
			if (creature->hurtByLara 
				&& item->ActiveState != STATE_SCORPION_TROOPS_ATTACK)
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

						if (currentItem->ObjectNumber != ID_LARA)
						{
							if (currentItem->ObjectNumber != ID_BIG_SCORPION &&
								(currentItem != LaraItem || creature->hurtByLara))
							{
								int dx = currentItem->Position.xPos - item->Position.xPos;
								int dy = currentItem->Position.yPos - item->Position.yPos;
								int dz = currentItem->Position.zPos - item->Position.zPos;

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

		switch (item->ActiveState)
		{
		case STATE_SCORPION_STOP:
			creature->maximumTurn = 0;
			creature->flags = 0;

			if (info.distance > SQUARE(1365))
			{
				item->TargetState = STATE_SCORPION_WALK;
				break;
			}

			if (info.bite)
			{
				creature->maximumTurn = ANGLE(2);

				if (GetRandomControl() & 1 
					&& creature->enemy->HitPoints <= 15
					&& creature->enemy->ObjectNumber == ID_TROOPS)
				{
					item->TargetState = STATE_SCORPION_ATTACK1;
				}
				else
				{
					item->TargetState = STATE_SCORPION_ATTACK2;
				}
			}
			else if (!info.ahead)
			{
				item->TargetState = STATE_SCORPION_WALK;
			}

			break;

		case STATE_SCORPION_WALK:
			creature->maximumTurn = ANGLE(2);

			if (info.distance < SQUARE(1365))
			{
				item->TargetState = STATE_SCORPION_STOP;
			}
			else if (info.distance > SQUARE(853))
			{
				item->TargetState = STATE_SCORPION_RUN;
			}

			break;

		case STATE_SCORPION_RUN:
			creature->maximumTurn = ANGLE(3);

			if (info.distance < SQUARE(1365))
			{
				item->TargetState = STATE_SCORPION_STOP;
			}

			break;

		case STATE_SCORPION_ATTACK1:
		case STATE_SCORPION_ATTACK2:
			creature->maximumTurn = 0;

			if (abs(info.angle) >= ANGLE(2))
			{
				if (info.angle >= 0)
				{
					item->Position.yRot += ANGLE(2);
				}
				else
				{
					item->Position.yRot -= ANGLE(2);
				}
			}
			else
			{
				item->Position.yRot += info.angle;
			}

			if (creature->flags)
			{
				break;
			}

			if (creature->enemy 
				&& creature->enemy != LaraItem 
				&& info.distance < SQUARE(1365))
			{
				creature->enemy->HitPoints -= 15;
				if (creature->enemy->HitPoints <= 0)
				{
					item->TargetState = STATE_SCORPION_SPECIAL_DEATH;
					creature->maximumTurn = 0;
				}

				creature->enemy->HitStatus = true;
				creature->flags = 1;

				CreatureEffect2(
					item,
					&scorpionBite1,
					10,
					item->Position.yRot - ANGLE(180),
					DoBloodSplat);
			}
			else if (item->TouchBits & 0x1B00100)
			{
				LaraItem->HitPoints -= 120;
				LaraItem->HitStatus = true;

				if (item->ActiveState == 5)
				{
					Lara.poisoned += 2048;

					CreatureEffect2(
						item,
						&scorpionBite1,
						10,
						item->Position.yRot - ANGLE(180),
						DoBloodSplat);
				}
				else
				{
					CreatureEffect2(
						item,
						&scorpionBite2,
						10,
						item->Position.yRot - ANGLE(180),
						DoBloodSplat);
				}

				creature->flags = 1;
				if (LaraItem->HitPoints <= 0)
				{
					CreatureKill(item, 6, 7, 442);
					creature->maximumTurn = 0;
					return;
				}
			}

			break;

		case STATE_SCORPION_TROOPS_ATTACK:
			creature->maximumTurn = 0;
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd)
			{
				item->TriggerFlags++;
			}
			if (creature->enemy 
				&& creature->enemy->HitPoints <= 0 
				|| item->TriggerFlags > 6)
			{
				item->TargetState = STATE_SCORPION_SPECIAL_DEATH;
				creature->enemy->HitPoints = 0;
			}

			break;

		default:
			break;
		}
	}

	if ((angle1 - item->Position.xRot) < 256)
		item->Position.xRot = 256;
	else
	{
		if (angle1 <= item->Position.xRot)
			item->Position.xRot -= 256;
		else
			item->Position.xRot += 256;
	}

	if ((angle2 - item->Position.zRot) < 256)
		item->Position.zRot = 256;
	else
	{
		if (angle2 <= item->Position.zRot)
			item->Position.zRot -= 256;
		else
			item->Position.zRot += 256;
	}

	if (!CutSeqNum)
		CreatureAnimation(itemNumber, angle, 0);
}
