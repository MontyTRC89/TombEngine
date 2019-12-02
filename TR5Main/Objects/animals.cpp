#include "newobjects.h"
#include "..\Global\global.h"
#include "..\Game\Box.h"
#include "..\Game\items.h"
#include "..\Game\lot.h"
#include "..\Game\control.h"
#include "..\Game\effects.h"
#include "..\Game\effect2.h"
#include "..\Game\draw.h"
#include "..\Game\sphere.h"
#include "..\Game\people.h"
#include "..\Game\debris.h"
#include "..\Game\Lara.h"

BITE_INFO wildboardBiteInfo = { 0, 0, 0, 14 };
BITE_INFO smallScorpionBiteInfo1 = { 0, 0, 0, 0 };
BITE_INFO smallScorpionBiteInfo2 = { 0, 0, 0, 23 };
BITE_INFO batBiteInfo = { 0, 16, 45, 4 };
BITE_INFO barracudaBite = { 2, -60, 121, 7 };
BITE_INFO sharkBite = { 17, -22, 344, 12 };
BITE_INFO tigerBite = { 19, -13, 3, 26 };
BITE_INFO cobraBite = { 0, 0, 0, 13 };
BITE_INFO raptorBite = { 0, 66, 318, 22 };
BITE_INFO eagleBite = { 15, 46, 21, 6 };
BITE_INFO crowBite = { 2, 10, 60, 14 };
BITE_INFO wolfBite = { 0, -14, 174, 6 };
BITE_INFO bearBite = { 0, 96, 335, 14 };
BITE_INFO apeBite = { 0, -19, 75, 15 };
BITE_INFO mouseBite = { 0, 0, 57, 2 };
BITE_INFO scorpionBite1 = { 0, 0, 0, 8 };
BITE_INFO scorpionBite2 = { 0, 0, 0, 23 };
BITE_INFO harpyBite1 = { 0, 0, 0, 4 };
BITE_INFO harpyBite2 = { 0, 0, 0, 2 };
BITE_INFO harpyBite3 = { 0, 0, 0, 21 };
BITE_INFO harpyAttack1 = { 0, 128, 0, 2 };
BITE_INFO harpyAttack2 = { 0, 128, 0, 4 };
BITE_INFO crocodileBiteInfo = { 0, -156, 500, 9 };
BITE_INFO sphinxBiteInfo = { 0, 0, 0, 6 };
BITE_INFO monkeyBite = { 10, 10, 11, 13 };
BITE_INFO yetiBiteR = { 12, 101, 19, 10 };
BITE_INFO yetiBiteL = { 12, 101, 19, 13 };
BITE_INFO spiderBite = { 0, 0, 41, 1 };
BITE_INFO birdyBiteL = { 0, 224, 0, 19 };
BITE_INFO birdyBiteR = { 0, 224, 0, 22 };

extern LaraExtraInfo g_LaraExtra;

void InitialiseWildBoar(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[ID_WILD_BOAR].animIndex + 6;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
}

void WildBoarControl(short itemNum)
{
	short angle = 0;
	short head = 0;
	short neck = 0;
	short tilt = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short joint3 = 0;

	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	if (item->hitPoints > 0)
	{
		int dx = LaraItem->pos.xPos - item->pos.xPos;
		int dz = LaraItem->pos.zPos - item->pos.zPos;
		int laraDistance = dx * dx + dz * dz;

		if (item->aiBits & GUARD)
		{
			GetAITarget(creature);
		}
		else
		{
			creature->enemy = LaraItem;
			
			CREATURE_INFO* baddie = &BaddieSlots[0];
			CREATURE_INFO* found = &BaddieSlots[0];
			int minDistance = 0x7FFFFFFF;

			for (int i = 0; i < NUM_SLOTS; i++, baddie++)
			{
				if (baddie->itemNum == NO_ITEM || baddie->itemNum == itemNum)
					continue;

				ITEM_INFO* target = &Items[baddie->itemNum];
				if (target->objectNumber != ID_WILD_BOAR)
				{
					int dx2 = target->pos.xPos - item->pos.xPos;
					int dz2 = target->pos.zPos - item->pos.zPos;
					int distance = dx2 * dx2 + dz2 * dz2;

					if (distance < minDistance && distance < laraDistance)
					{
						creature->enemy = target;
						minDistance = distance;
					}
				}
			}
		}

		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		if (item->flags)
			creature->mood = ESCAPE_MOOD;
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);
		
		if (info.ahead)
		{
			joint1 = info.angle >> 1;
			joint3 = info.angle >> 1;
		}

		switch (item->currentAnimState)
		{
		case 1:
			creature->maximumTurn = 0;
			if (info.ahead && info.distance || item->flags)
			{
				item->goalAnimState = 2;
			}
			else if (GetRandomControl() & 0x7F)
			{
				joint1 = AIGuard(creature) >> 1;
				joint3 = joint1;
			}
			else
			{
				item->goalAnimState = 3;
			}
			break;

		case 3:
			creature->maximumTurn = 0;
			if (info.ahead && info.distance)
			{
				item->goalAnimState = 1;
			}
			else if (!(GetRandomControl() & 0x7F))
			{
				item->goalAnimState = 1;
			}
			break;

		case 2:
			if (info.distance >= 0x400000)
			{
				creature->maximumTurn = 1092;
				item->flags = 0;
			}
			else
			{
				creature->maximumTurn = 546;
				joint0 = -info.distance;
				joint2 = -info.distance;
			}
			if (!item->flags && (info.distance < 0x10000 && info.bite))
			{
				item->goalAnimState = 4;
				if (creature->enemy == LaraItem)
				{
					LaraItem->hitPoints -= 30;
					LaraItem->hitStatus = true;
				}

				CreatureEffect2(item, &wildboardBiteInfo, 3, item->pos.yRot, DoBloodSplat);
				item->flags = 1;
			}
			break;

		case 4:
			creature->maximumTurn = 0;
			break;

		}
	}
	else
	{
		item->hitPoints = 0;
		if (item->currentAnimState != 5)
		{
			item->animNumber = Objects[ID_WILD_BOAR].animIndex + 5;
			item->currentAnimState = 5;
			item->frameNumber = Anims[item->animNumber].frameBase;
		}
	}

	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureJoint(item, 3, joint3);
	
	CreatureAnimation(itemNum, angle, 0);
}

void InitialiseSmallScorpion(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[ID_SMALL_SCORPION].animIndex + 2;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
}

void SmallScorpionControl(short itemNum)
{
	short angle = 0;
	short head = 0;
	short neck = 0;
	short tilt = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short joint3 = 0;

	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	if (item->hitPoints > 0)
	{
		int dx = LaraItem->pos.xPos - item->pos.xPos;
		int dz = LaraItem->pos.zPos - item->pos.zPos;
		int laraDistance = dx * dx + dz * dz;

		if (item->aiBits & GUARD)
			GetAITarget(creature);
		else
			creature->enemy = LaraItem;

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
			if (info.distance > 116281)
			{
				item->goalAnimState = 2;
			}
			else if (info.bite)
			{
				creature->maximumTurn = 1092;
				if (GetRandomControl() & 1 /*|| creature->enemy->objectNumber == 59 && creature->enemy->hitPoints <= 2*/)
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

		case 3:
			creature->maximumTurn = 1456;
			if (info.distance < 116281)
			{
				item->goalAnimState = 1;
			}
			break;

		case 2:
			creature->maximumTurn = 1092;
			if (info.distance >= 116281)
			{
				if (info.distance > 45369)
				{
					item->goalAnimState = 3;
				}
			}
			else
			{
				item->goalAnimState = 1;
			}
			break;

		case 4:
		case 5:
			creature->maximumTurn = 0;
			if (abs(info.angle) >= 1092)
			{
				if (info.angle >= 0)
				{
					item->pos.yRot += 1092;
				}
				else
				{
					item->pos.yRot -= 1092;
				}
			}
			else
			{
				item->pos.yRot += info.angle;
			}
			if (!creature->flags)
			{
				if (item->touchBits & 0x1B00100)
				{
					if (item->frameNumber > Anims[item->animNumber].frameBase + 20 && 
						item->frameNumber < Anims[item->animNumber].frameBase + 32)
					{
						LaraItem->hitPoints -= 20;
						LaraItem->hitStatus = true;

						BITE_INFO* biteInfo;
						short rot;

						if (item->currentAnimState == 5)
						{
							rot = item->pos.yRot + -ANGLE(180);
							biteInfo = &smallScorpionBiteInfo1;
						}
						else
						{
							rot = item->pos.yRot + -ANGLE(180);
							biteInfo = &smallScorpionBiteInfo2;
						}
						CreatureEffect2(item, biteInfo, 3, rot, DoBloodSplat);
						creature->flags = 1;
					}
				}
			}
			break;
		}
	}
	else
	{
		item->hitPoints = 0;
		if (item->currentAnimState != 6 && item->currentAnimState != 7)
		{
			item->animNumber = Objects[ID_SMALL_SCORPION].animIndex + 5;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 6;
		}
	}

	CreatureAnimation(itemNum, angle, 0);
}

void InitialiseScorpion(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	if (item->triggerFlags == 1)
	{
		item->goalAnimState = 8;
		item->currentAnimState = 8;
		item->animNumber = Objects[ID_SCORPION].animIndex + 7;
	}
	else
	{
		item->goalAnimState = 1;
		item->currentAnimState = 1;
		item->animNumber = Objects[ID_SCORPION].animIndex + 2;
	}

	item->frameNumber = Anims[item->animNumber].frameBase;
}

void ScorpionControl(short itemNum)
{
	short angle = 0;
	short head = 0;
	short neck = 0;
	short tilt = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short joint3 = 0;

	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

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

						if (currentItem->objectNumber != ID_SCORPION &&
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

void InitialiseBat(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[ID_BAT].animIndex + 5;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 6;
	item->currentAnimState = 6;
}

void BatControl(short itemNum)
{
	short angle = 0;
	short head = 0;
	short neck = 0;
	short tilt = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short joint3 = 0;

	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	if (item->hitPoints > 0)
	{
		int dx = LaraItem->pos.xPos - item->pos.xPos;
		int dz = LaraItem->pos.zPos - item->pos.zPos;
		int laraDistance = dx * dx + dz * dz;

		if (item->aiBits)
		{
			GetAITarget(creature);
		}
		else
		{
			creature->enemy = LaraItem;

			CREATURE_INFO* baddie = &BaddieSlots[0];
			CREATURE_INFO* found = &BaddieSlots[0];
			int minDistance = 0x7FFFFFFF;

			for (int i = 0; i < NUM_SLOTS; i++, baddie++)
			{
				if (baddie->itemNum == NO_ITEM || baddie->itemNum == itemNum)
					continue;

				ITEM_INFO* target = &Items[baddie->itemNum];
				if (target->objectNumber != ID_WILD_BOAR)
				{
					int dx2 = target->pos.xPos - item->pos.xPos;
					int dz2 = target->pos.zPos - item->pos.zPos;
					int distance = dx2 * dx2 + dz2 * dz2;

					if (distance < minDistance)
					{
						creature->enemy = target;
						minDistance = distance;
					}
				}
			}
		}

		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, TIMID);
		if (item->flags)
			creature->mood = ESCAPE_MOOD;
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, ANGLE(20));

		switch (item->currentAnimState)
		{
		case 2:
			if (info.distance < SQUARE(256) || !(GetRandomControl() & 0x3F))
			{
				creature->flags = 0;
			}
			if (!creature->flags)
			{
				if (item->touchBits
					|| creature->enemy != LaraItem
					&& info.distance < SQUARE(256)
					&& info.ahead
					&& abs(item->pos.yPos - creature->enemy->pos.yPos) < 896)
				{
					item->goalAnimState = 3;
				}
			}
			break;

		case 3:
			if (!creature->flags
				&& (item->touchBits
					|| creature->enemy != LaraItem
					&& info.distance < SQUARE(256)
					&& info.ahead
					&& abs(item->pos.yPos - creature->enemy->pos.yPos) < 896))
			{
				CreatureEffect(item, &batBiteInfo, DoBloodSplat);
				if (creature->enemy == LaraItem)
				{
					LaraItem->hitPoints -= 2;
					LaraItem->hitStatus = true;
				}
				creature->flags = 1;
			}
			else
			{
				item->goalAnimState = 2;
				creature->mood = BORED_MOOD;
			}
			break;

		case 6:
			if (info.distance < SQUARE(5120) || 
				item->hitStatus || 
				creature->hurtByLara)
			{
				item->goalAnimState = 1;
			}
			break;

		}
	}
	else if (item->currentAnimState == 3)
	{
		item->animNumber = Objects[ID_BAT].animIndex + 1;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->goalAnimState = 2;
		item->currentAnimState = 2;
	}
	else
	{
		if (item->pos.yPos >= item->floor)
		{
			item->goalAnimState = 5;
			item->pos.yPos = item->floor;
			item->gravityStatus = false;
		}
		else
		{
			item->gravityStatus = true;
			item->animNumber = Objects[ID_BAT].animIndex + 3;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->goalAnimState = 4;
			item->currentAnimState = 4;
			item->speed = 0;
		}
	}

	CreatureAnimation(itemNum, angle, 0);
}

void BarracudaControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO *)item->data;
	short angle = 0;
	short head = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 6)
		{
			item->animNumber = Objects[ID_BARRACUDA].animIndex + 6;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 6;
		}

		CreatureFloat(itemNum);
		return;
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, creature->maximumTurn);

		switch (item->currentAnimState)
		{
		case 1:
			creature->flags = 0;

			if (creature->mood == BORED_MOOD)
				item->goalAnimState = 2;
			else if (info.ahead && info.distance < 680)
				item->goalAnimState = 4;
			else if (creature->mood == STALK_MOOD)
				item->goalAnimState = 2;
			else
				item->goalAnimState = 3;
			break;

		case 2:
			creature->maximumTurn = ANGLE(2);

			if (creature->mood == BORED_MOOD)
				break;
			else if (info.ahead && (item->touchBits & 0xE0))
				item->goalAnimState = 1;
			else if (creature->mood != STALK_MOOD)
				item->goalAnimState = 3;
			break;

		case 3:
			creature->maximumTurn = ANGLE(4);
			creature->flags = 0;

			if (creature->mood == BORED_MOOD)
				item->goalAnimState = 2;
			else if (info.ahead && info.distance < 340)
				item->goalAnimState = 5;
			else if (info.ahead && info.distance < 680)
				item->goalAnimState = 1;
			else if (creature->mood == STALK_MOOD)
				item->goalAnimState = 2;
			break;

		case 4:
		case 5:
			if (info.ahead)
				head = info.angle;

			if (!creature->flags && (item->touchBits & 0xE0))
			{
				LaraItem->hitPoints -= 100;
				LaraItem->hitStatus = true;
				CreatureEffect(item, &barracudaBite, DoBloodSplat);

				creature->flags = 1;
			}
			break;
		}
	}

	CreatureJoint(item, 0, head);

	CreatureAnimation(itemNum, angle, 0);
	CreatureUnderwater(item, STEP_SIZE);
}

void SharkControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO *)item->data;
	short angle = 0;
	short head = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 5)
		{
			item->animNumber = Objects[ID_SHARK].animIndex + 4;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 5;
		}
		CreatureFloat(itemNum);
		return;
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		switch (item->currentAnimState)
		{
		case 0:
			creature->flags = 0;
			creature->maximumTurn = 0;

			if (info.ahead && info.distance < SQUARE(768) && info.zoneNumber == info.enemyZone)
				item->goalAnimState = 3;
			else
				item->goalAnimState = 1;
			break;

		case 1:
			creature->maximumTurn = ANGLE(1) / 2;
			if (creature->mood == BORED_MOOD)
				break;
			else if (info.ahead && info.distance < SQUARE(768))
				item->goalAnimState = 0;
			else if (creature->mood == ESCAPE_MOOD || info.distance > SQUARE(3072) || !info.ahead)
				item->goalAnimState = 2;
			break;

		case 2:
			creature->flags = 0;
			creature->maximumTurn = ANGLE(2);

			if (creature->mood == BORED_MOOD)
				item->goalAnimState = 1;
			else if (creature->mood == ESCAPE_MOOD)
				break;
			else if (info.ahead && info.distance < SQUARE(1365) && info.zoneNumber == info.enemyZone)
			{
				if (GetRandomControl() < 0x800)
					item->goalAnimState = 0;
				else if (info.distance < SQUARE(768))
					item->goalAnimState = 4;
			}
			break;

		case 3:
		case 4:
			if (info.ahead)
				head = info.angle;

			if (!creature->flags && (item->touchBits & 0x3400))
			{
				LaraItem->hitPoints -= 400;
				LaraItem->hitStatus = true;
				CreatureEffect(item, &sharkBite, DoBloodSplat);

				creature->flags = 1;
			}
			break;
		}
	}

	if (item->currentAnimState != 6)
	{
		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNum, angle, 0);
		CreatureUnderwater(item, 340);
	}
	else
		AnimateItem(item);
}

void TigerControl(short itemNum)
{
	short head = 0;
	short angle = 0;
	short tilt = 0;

	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO *)item->data;
	
	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 9)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 11;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 9;
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, 1);

		if (creature->alerted && info.zoneNumber != info.enemyZone)
			creature->mood = ESCAPE_MOOD;

		CreatureMood(item, &info, 1);

		angle = CreatureTurn(item, creature->maximumTurn);

		switch (item->currentAnimState)
		{
		case 1:
			creature->maximumTurn = 0;
			creature->flags = 0;

			if (creature->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 3;
			}
			else if (creature->mood == BORED_MOOD)
			{
				short random = GetRandomControl();
				if (random < 0x60)
					item->goalAnimState = 5;
				else if (random < 0x460);
				item->goalAnimState = 2;
			}
			else if (info.bite && info.distance < SQUARE(340))
				item->goalAnimState = 6;
			else if (info.bite && info.distance < SQUARE(1024))
			{
				creature->maximumTurn = ANGLE(3);
				item->goalAnimState = 8;
			}
			else if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else if (creature->mood != ATTACK_MOOD && GetRandomControl() < 0x60)
				item->goalAnimState = 5;
			else
				item->goalAnimState = 3;
			break;

		case 2:
			creature->maximumTurn = ANGLE(3);

			if (creature->mood == ESCAPE_MOOD || creature->mood == ATTACK_MOOD)
				item->goalAnimState = 3;
			else if (GetRandomControl() < 0x60)
			{
				item->goalAnimState = 1;
				item->requiredAnimState = 5;
			}
			break;

		case 3:
			creature->maximumTurn = ANGLE(6);

			if (creature->mood == BORED_MOOD)
				item->goalAnimState = 1;
			else if (creature->flags && info.ahead)
				item->goalAnimState = 1;
			else if (info.bite && info.distance < SQUARE(1536))
			{
				if (LaraItem->speed == 0)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 7;
			}
			else if (creature->mood != ATTACK_MOOD && GetRandomControl() < 0x60)
			{
				item->requiredAnimState = 5;
				item->goalAnimState = 1;
			}
			else if (creature->mood == ESCAPE_MOOD && Lara.target != item && info.ahead)
				item->goalAnimState = 1;

			creature->flags = 0;
			break;

		case 6:
		case 7:
		case 8:
			if (!creature->flags && (item->touchBits & 0x7FDC000))
			{
				LaraItem->hitStatus = true;
				LaraItem->hitPoints -= 90;
				CreatureEffect(item, &tigerBite, DoBloodSplat);
				creature->flags = 1;
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, tilt);
}

void InitialiseCobra(short itemNum)
{
	InitialiseCreature(itemNum);

	ITEM_INFO* item = &Items[itemNum];
	item->animNumber = Objects[item->objectNumber].animIndex + 2;
	item->frameNumber = Anims[item->animNumber].frameBase + 45;
	item->currentAnimState = item->goalAnimState = 3;
	item->itemFlags[2] = item->hitStatus;
	item->hitPoints = Objects[item->objectNumber].hitPoints;
}

void CobraControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	short head = 0;
	short angle = 0;
	short tilt = 0;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO *)item->data;
	
	if (item->hitPoints <= 0 && item->hitPoints != -16384)
	{
		if (item->currentAnimState != 4)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 4;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 4;
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		info.angle += 0xC00;

		GetCreatureMood(item, &info, 1);
		CreatureMood(item, &info, 1);
		
		creature->target.x = LaraItem->pos.xPos;
		creature->target.z = LaraItem->pos.zPos;
		angle = CreatureTurn(item, creature->maximumTurn);

		if (info.ahead)
			head = info.angle;

		if (abs(info.angle) < ANGLE(10))
			item->pos.yRot += info.angle;
		else if (info.angle < 0)
			item->pos.yRot -= ANGLE(10);
		else
			item->pos.yRot += ANGLE(10);

		switch (item->currentAnimState)
		{
		case 1:
			creature->flags = 0;
			if (info.distance > SQUARE(2560))
				item->goalAnimState = 3;
			else if ((LaraItem->hitPoints > 0) && ((info.ahead && info.distance < SQUARE(1024)) || item->hitStatus || (LaraItem->speed > 15)))
				item->goalAnimState = 2;
			break;

		case 3:
			creature->flags = 0;
			if (item->hitPoints != -16384)
			{
				item->itemFlags[2] = item->hitPoints;
				item->hitPoints = -16384;
			}
			if (info.distance < SQUARE(1536) && LaraItem->hitPoints > 0)
			{
				item->goalAnimState = 0;
				item->hitPoints = item->itemFlags[2];
			}
			break;

		case 2:
			if (creature->flags != 1 && (item->touchBits & 0x2000))
			{
				creature->flags = 1;
				LaraItem->hitPoints -= 80;
				LaraItem->hitStatus = true;
				Lara.poisoned = 0x100;

				CreatureEffect(item, &cobraBite, DoBloodSplat);
			}
			break;

		case 0:
			item->hitPoints = item->itemFlags[2];
			break;

		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head >> 1);
	CreatureJoint(item, 1, head >> 1);
	CreatureAnimation(itemNum, angle, tilt);
}

void RaptorControl(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	if (item->status == ITEM_INVISIBLE)
	{
		if (!EnableBaddieAI(itemNum, 0))
			return;
		item->status = ITEM_ACTIVE;
	}
	CREATURE_INFO* creature = (CREATURE_INFO *)item->data;
	
	short head = 0;
	short neck = 0;
	short angle = 0;
	short tilt = 0;

	ITEM_INFO* nearestItem = NULL;
	int minDistance = 0x7FFFFFFF;
	
	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 5)
		{
			if (GetRandomControl() > 0x4000)
				item->animNumber = Objects[item->objectNumber].animIndex + 9;
			else
				item->animNumber = Objects[item->objectNumber].animIndex + 10;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 5;
		}
	}
	else
	{
		if (creature->enemy == NULL || !(GetRandomControl() & 0x7F)) 													   // Decide on target - this can be Lara, another creature, or an ambush point
		{
			CREATURE_INFO* currentCreature = BaddieSlots;
			ITEM_INFO* target = NULL;
			for (int i = 0; i < NUM_SLOTS; i++)
			{
				if (currentCreature->itemNum == NO_ITEM || currentCreature->itemNum == itemNum)
				{
					currentCreature++;
					continue;
				}

				target = &Items[currentCreature->itemNum];

				int x = (target->pos.xPos - item->pos.xPos) >> 6;
				int y = (target->pos.yPos - item->pos.yPos) >> 6;
				int z = (target->pos.zPos - item->pos.zPos) >> 6;
				int distance = x * x + y * y + z * z;
				if (distance < minDistance && item->hitPoints > 0)
				{
					nearestItem = target;
					minDistance = distance;
				}

				currentCreature++;
			}
			
			if (nearestItem != NULL && (nearestItem->objectNumber != ID_RAPTOR || (GetRandomControl() < 0x400 && minDistance < SQUARE(2048))))
				creature->enemy = nearestItem;
			
			int x = (LaraItem->pos.xPos - item->pos.xPos) >> 6;
			int y = (LaraItem->pos.yPos - item->pos.yPos) >> 6;
			int z = (LaraItem->pos.zPos - item->pos.zPos) >> 6;
			int distance = x * x + y * y + z * z;
			if (distance <= minDistance)
				creature->enemy = LaraItem;
		}

		if (item->aiBits)
			GetAITarget(creature);

		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		if (creature->mood == BORED_MOOD)
			creature->maximumTurn >>= 1;

		angle = CreatureTurn(item, creature->maximumTurn);
		neck = -(angle * 6);

		switch (item->currentAnimState)
		{
		case 1:
			creature->maximumTurn = 0;
			creature->flags &= ~1;

			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else if (creature->flags & 2)
			{
				creature->flags &= ~2;
				item->goalAnimState = 6;
			}
			else if ((item->touchBits & 0xFF7C00) || (info.distance < SQUARE(585) && info.bite))
				item->goalAnimState = 8;
			else if (info.bite && info.distance < SQUARE(1536))
				item->goalAnimState = 4;
			else if (creature->mood == ESCAPE_MOOD && Lara.target != item && info.ahead && !item->hitStatus)
				item->goalAnimState = 1;
			else if (creature->mood == BORED_MOOD)
				item->goalAnimState = 2;
			else
				item->goalAnimState = 3;
			break;

		case 2:
			creature->maximumTurn = ANGLE(2);
			creature->flags &= ~1;

			if (creature->mood != BORED_MOOD)
				item->goalAnimState = 1;
			else if (info.ahead && GetRandomControl() < 0x80)
			{
				item->requiredAnimState = 6;
				item->goalAnimState = 1;
				creature->flags &= ~2;

			}
			break;

		case 3:
			tilt = angle;
			creature->maximumTurn = ANGLE(4);
			creature->flags &= ~1;

			if (item->touchBits & 0xFF7C00)
				item->goalAnimState = 1;
			else if (creature->flags & 2)
			{
				item->requiredAnimState = 6;
				item->goalAnimState = 1;
				creature->flags &= ~2;
			}
			else if (info.bite && info.distance < SQUARE(1536))
			{
				if (item->goalAnimState == 3)
				{
					if (GetRandomControl() < 0x2000)
						item->goalAnimState = 1;
					else
						item->goalAnimState = 7;
				}
			}
			else if (info.ahead && creature->mood != ESCAPE_MOOD && GetRandomControl() < 0x80)
			{
				item->requiredAnimState = 6;
				item->goalAnimState = 1;
			}
			else if (creature->mood == BORED_MOOD || (creature->mood == ESCAPE_MOOD && Lara.target != item && info.ahead))
				item->goalAnimState = 1;
			break;

		case 4:
			tilt = angle;
			creature->maximumTurn = ANGLE(2);
			if (creature->enemy == LaraItem)
			{
				if (!(creature->flags & 1) && (item->touchBits & 0xFF7C00))
				{
					creature->flags |= 1;
					CreatureEffect(item, &raptorBite, DoBloodSplat);
					if (LaraItem->hitPoints <= 0)
						creature->flags |= 2;
					LaraItem->hitPoints -= 100;
					LaraItem->hitStatus = 1;
					item->requiredAnimState = 1;
				}
			}
			else
			{
				if (!(creature->flags & 1) && creature->enemy)
				{
					if (abs(creature->enemy->pos.xPos - item->pos.xPos) < 512 &&
						abs(creature->enemy->pos.yPos - item->pos.yPos) < 512 &&
						abs(creature->enemy->pos.zPos - item->pos.zPos) < 512)
					{
						creature->enemy->hitPoints -= 100 >> 2;
						creature->enemy->hitStatus = 1;
						if (creature->enemy->hitPoints <= 0)
							creature->flags |= 2;
						creature->flags |= 1;
						CreatureEffect(item, &raptorBite, DoBloodSplat);

					}
				}
			}

			break;

		case 8:
			tilt = angle;
			creature->maximumTurn = ANGLE(2);
			if (creature->enemy == LaraItem)
			{
				if (!(creature->flags & 1) && (item->touchBits & 0xFF7C00))
				{
					creature->flags |= 1;
					CreatureEffect(item, &raptorBite, DoBloodSplat);
					if (LaraItem->hitPoints <= 0)
						creature->flags |= 2;
					LaraItem->hitPoints -= 100;
					LaraItem->hitStatus = 1;

					item->requiredAnimState = 1;
				}
			}
			else
			{
				if (!(creature->flags & 1) && creature->enemy)
				{
					if (abs(creature->enemy->pos.xPos - item->pos.xPos) < 512 &&
						abs(creature->enemy->pos.yPos - item->pos.yPos) < 512 &&
						abs(creature->enemy->pos.zPos - item->pos.zPos) < 512)
					{
						creature->enemy->hitPoints -= 100 >> 2;
						creature->enemy->hitStatus = 1;
						if (creature->enemy->hitPoints <= 0)
							creature->flags |= 2;
						creature->flags |= 1;
						CreatureEffect(item, &raptorBite, DoBloodSplat);

					}
				}
			}

			break;

		case 7:
			tilt = angle;
			creature->maximumTurn = ANGLE(2);
			if (creature->enemy == LaraItem)
			{
				if (!(creature->flags & 1) && (item->touchBits & 0xFF7C00))
				{
					creature->flags |= 1;
					CreatureEffect(item, &raptorBite, DoBloodSplat);

					LaraItem->hitPoints -= 100;
					LaraItem->hitStatus = 1;
					if (LaraItem->hitPoints <= 0)
						creature->flags |= 2;
					item->requiredAnimState = 3;
				}
			}
			else
			{
				if (!(creature->flags & 1) && creature->enemy)
				{
					if (abs(creature->enemy->pos.xPos - item->pos.xPos) < 512 &&
						abs(creature->enemy->pos.yPos - item->pos.yPos) < 512 &&
						abs(creature->enemy->pos.zPos - item->pos.zPos) < 512)
					{
						creature->enemy->hitPoints -= 100 >> 2;
						creature->enemy->hitStatus = 1;
						if (creature->enemy->hitPoints <= 0)
							creature->flags |= 2;
						creature->flags |= 1;
						CreatureEffect(item, &raptorBite, DoBloodSplat);

					}
				}
			}

			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head >> 1);
	CreatureJoint(item, 1, head >> 1);
	CreatureJoint(item, 2, neck);
	CreatureJoint(item, 3, neck);
	CreatureAnimation(itemNum, angle, tilt);
}

void InitialiseEagle(short itemNum)
{
	InitialiseCreature(itemNum);

	ITEM_INFO* item = &Items[itemNum];
	if (item->objectNumber == ID_CROW)
	{
		item->animNumber = Objects[ID_CROW].animIndex + 14;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->currentAnimState = item->goalAnimState = 7;
	}
	else
	{
		item->animNumber = Objects[ID_EAGLE].animIndex + 5;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->currentAnimState = item->goalAnimState = 2;
	}
}

void EagleControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO *)item->data;

	short angle = 0;

	if (item->hitPoints <= 0)
	{
		switch (item->currentAnimState)
		{
		case 4:
			if (item->pos.yPos > item->floor)
			{
				item->pos.yPos = item->floor;
				item->gravityStatus = 0;
				item->fallspeed = 0;
				item->goalAnimState = 5;
			}
			break;

		case 5:
			item->pos.yPos = item->floor;
			break;

		default:
			if (item->objectNumber == ID_CROW)
				item->animNumber = Objects[ID_CROW].animIndex + 1;
			else
				item->animNumber = Objects[ID_EAGLE].animIndex + 8;

			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 4;
			item->gravityStatus = 1;
			item->speed = 0;
			break;
		}
		item->pos.xRot = 0;
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, ANGLE(3));

		switch (item->currentAnimState)
		{
		case 7:
			item->pos.yPos = item->floor;
			if (creature->mood != BORED_MOOD)
				item->goalAnimState = 1;
			break;

		case 2:
			item->pos.yPos = item->floor;
			if (creature->mood == BORED_MOOD)
				break;
			else
				item->goalAnimState = 1;
			break;

		case 1:
			creature->flags = 0;

			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			if (creature->mood == BORED_MOOD)
				item->goalAnimState = 2;
			else if (info.ahead && info.distance < SQUARE(512))
				item->goalAnimState = 6;
			else
				item->goalAnimState = 3;
			break;

		case 3:
			if (creature->mood == BORED_MOOD)
			{
				item->requiredAnimState = 2;
				item->goalAnimState = 1;
			}
			else if (info.ahead && info.distance < SQUARE(512))
				item->goalAnimState = 6;
			break;

		case 6:
			if (!creature->flags && item->touchBits)
			{
				LaraItem->hitPoints -= 20;
				LaraItem->hitStatus = true;

				if (item->objectNumber == ID_CROW)
					CreatureEffect(item, &crowBite, DoBloodSplat);
				else
					CreatureEffect(item, &eagleBite, DoBloodSplat);

				creature->flags = 1;
			}
			break;
		}
	}

	CreatureAnimation(itemNum, angle, 0);
}

void BearControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	short head = 0;
	short angle = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 9)
		{
			angle = CreatureTurn(item, ANGLE(1));

			switch (item->currentAnimState)
			{
				case 2:
					item->goalAnimState = 4;
					break;
				case 3:
				case 0:
					item->goalAnimState = 1;
					break;

				case 4:
					creature->flags = 1;
					item->goalAnimState = 9;
					break;

				case 1:
					creature->flags = 0;
					item->goalAnimState = 9;
					break;

				case 9:
					if (creature->flags && (item->touchBits & 0x2406C))
					{
						LaraItem->hitPoints -= 200;
						LaraItem->hitStatus = 1;
						creature->flags = 0;
					}

					item->animNumber = Objects[item->objectNumber].animIndex + 20;
					item->frameNumber = Anims[item->animNumber].frameBase;
					item->currentAnimState = 9;
					break;
			}
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (item->hitStatus)
			creature->flags = 1;

		switch (item->currentAnimState)
		{
		case 1:
			if (LaraItem->hitPoints <= 0)
			{
				if (info.bite && info.distance < SQUARE(768))
				{
					item->goalAnimState = 8;
				}
				else
				{
					item->goalAnimState = 0;
				}
			}
			else if (item->requiredAnimState)
			{
				item->goalAnimState = item->requiredAnimState;
			}
			else if (creature->mood == BORED_MOOD)
			{
				item->goalAnimState = 0;
			}
			else
			{
				item->goalAnimState = 3;
			}
			break;

		case 0:
			creature->maximumTurn = ANGLE(2);

			if (LaraItem->hitPoints <= 0 && (item->touchBits & 0x2406C) && info.ahead)
			{
				item->goalAnimState = 1;
			}
			else if (creature->mood != BORED_MOOD)
			{
				item->goalAnimState = 1;
				if (creature->mood == ESCAPE_MOOD)
				{
					item->requiredAnimState = 0;
				}
			}
			else if (GetRandomControl() < 0x50)
			{
				item->requiredAnimState = 5;
				item->goalAnimState = 1;
			}
			break;

		case 3:
			creature->maximumTurn = ANGLE(5);

			// if the bear slams you it hurts
			if (item->touchBits & 0x2406C)
			{
				LaraItem->hitPoints -= 3;
				LaraItem->hitStatus = true;
			}

			if (creature->mood == BORED_MOOD || LaraItem->hitPoints <= 0)
			{
				item->goalAnimState = 1;
			}
			else if (info.ahead && !item->requiredAnimState)
			{
				// bear may rear up, but not after he's taken some bullets!
				if (!creature->flags && info.distance < SQUARE(2048) && GetRandomControl() < 0x300)
				{
					item->requiredAnimState = 4;
					item->goalAnimState = 1;
				}
				else if (info.distance < SQUARE(1024))
				{
					item->goalAnimState = 6;
				}
			}
			break;

		case 4:
			if (creature->flags)
			{
				item->requiredAnimState = 0;
				item->goalAnimState = 1;
			}
			else if (item->requiredAnimState)
			{
				item->goalAnimState = item->requiredAnimState;
			}
			else if (creature->mood == BORED_MOOD || creature->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = 1;
			}
			else if (info.bite && info.distance < SQUARE(600))
			{
				item->goalAnimState = 7;
			}
			else
			{
				item->goalAnimState = 2;
			}
			break;

		case 2:
			if (creature->flags)
			{
				item->requiredAnimState = 0;
				item->goalAnimState = 4;
			}
			else if (info.ahead && (item->touchBits & 0x2406C))
			{
				item->goalAnimState = 4;
			}
			else if (creature->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = 4;
				item->requiredAnimState = 0;
			}
			else if (creature->mood == BORED_MOOD || GetRandomControl() < 0x50)
			{
				item->requiredAnimState = 5;
				item->goalAnimState = 4;
			}
			else if (info.distance > SQUARE(2048) || GetRandomControl() < 0x600)
			{
				item->requiredAnimState = 1;
				item->goalAnimState = 4;
			}
			break;

		case 7:
			if (!item->requiredAnimState && (item->touchBits & 0x2406C))
			{
				LaraItem->hitPoints -= 400;
				LaraItem->hitStatus = true;
				item->requiredAnimState = 4;
			}

			break;

		case 6:
			if (!item->requiredAnimState && (item->touchBits & 0x2406C))
			{
				CreatureEffect(item, &bearBite, DoBloodSplat);
				LaraItem->hitPoints -= 200;
				LaraItem->hitStatus = true;
				item->requiredAnimState = 1;
			}
			break;

		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, 0);
}


void InitialiseWolf(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	ClearItem(itemNum);
	item->frameNumber = 96;
}


void WolfControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	
	short head = 0;
	short angle = 0;
	short tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 11)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 20 + (short)(GetRandomControl() / 11000);
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 11;
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, creature->maximumTurn);

		switch (item->currentAnimState)
		{
		case 8:
			head = 0;

			if (creature->mood == ESCAPE_MOOD || info.zoneNumber == info.enemyZone)
			{
				item->requiredAnimState = 9;
				item->goalAnimState = 1;
			}
			else if (GetRandomControl() < 0x20)
			{
				item->requiredAnimState = 2;
				item->goalAnimState = 1;
			}
			break;

		case 1:
			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else
				item->goalAnimState = 2;
			break;

		case 2:
			creature->maximumTurn = ANGLE(2);

			if (creature->mood != BORED_MOOD)
			{
				item->goalAnimState = 5;
				item->requiredAnimState = 0;
			}
			else if (GetRandomControl() < 0x20)
			{
				item->requiredAnimState = 8;
				item->goalAnimState = 1;
			}
			break;

		case 9:
			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else if (creature->mood == ESCAPE_MOOD)
				item->goalAnimState = 3;
			else if (info.distance < SQUARE(345) && info.bite)
				item->goalAnimState = 12;
			else if (creature->mood == STALK_MOOD)
				item->goalAnimState = 5;
			else if (creature->mood == BORED_MOOD)
				item->goalAnimState = 1;
			else
				item->goalAnimState = 3;
			break;

		case 5:
			creature->maximumTurn = ANGLE(2);

			if (creature->mood == ESCAPE_MOOD)
				item->goalAnimState = 3;
			else if (info.distance < SQUARE(345) && info.bite)
				item->goalAnimState = 12;
			else if (info.distance > SQUARE(3072))
				item->goalAnimState = 3;
			else if (creature->mood == ATTACK_MOOD)
			{
				if (!info.ahead || info.distance > SQUARE(1536) ||
					(info.enemyFacing < FRONT_ARC && info.enemyFacing > -FRONT_ARC))
					item->goalAnimState = 3;
			}
			else if (GetRandomControl() < 0x180)
			{
				item->requiredAnimState = 7;
				item->goalAnimState = 9;
			}
			else if (creature->mood == BORED_MOOD)
				item->goalAnimState = 9;
			break;

		case 3:
			creature->maximumTurn = ANGLE(5);
			tilt = angle;

			if (info.ahead && info.distance < SQUARE(1536))
			{
				if (info.distance > (SQUARE(1536) / 2) &&
					(info.enemyFacing > FRONT_ARC || info.enemyFacing < -FRONT_ARC))
				{
					item->requiredAnimState = 5;
					item->goalAnimState = 9;
				}
				else
				{
					item->goalAnimState = 6;
					item->requiredAnimState = 0;
				}
			}
			else if (creature->mood == STALK_MOOD && info.distance < SQUARE(3072))
			{
				item->requiredAnimState = 5;
				item->goalAnimState = 9;
			}
			else if (creature->mood == BORED_MOOD)
				item->goalAnimState = 9;
			break;

		case 6:
			tilt = angle;
			if (!item->requiredAnimState && (item->touchBits & 0x774F))
			{
				CreatureEffect(item, &wolfBite, DoBloodSplat);
				LaraItem->hitPoints -= 50;
				LaraItem->hitStatus = true;
				item->requiredAnimState = 3;
			}
			item->goalAnimState = 3;
			break;

		case 12:
			if (!item->requiredAnimState && (item->touchBits & 0x774F) && info.ahead)
			{
				CreatureEffect(item, &wolfBite, DoBloodSplat);
				LaraItem->hitPoints -= 100;
				LaraItem->hitStatus = true;
				item->requiredAnimState = 9;
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, tilt);
}

void LaraTyrannosaurDeath(ITEM_INFO* item)
{
	item->goalAnimState = 8;  

	if (LaraItem->roomNumber != item->roomNumber)
		ItemNewRoom(Lara.itemNumber, item->roomNumber);

	LaraItem->pos.xPos = item->pos.xPos;
	LaraItem->pos.yPos = item->pos.yPos;
	LaraItem->pos.zPos = item->pos.zPos;
	LaraItem->pos.yRot = item->pos.yRot;
	LaraItem->pos.xRot = 0;
	LaraItem->pos.zRot = 0;
	LaraItem->gravityStatus = false;

	LaraItem->animNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex + 1;
	LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase; 
	LaraItem->currentAnimState = STATE_LARA_DEATH;
	LaraItem->goalAnimState = STATE_LARA_DEATH;
	//LaraSwapMeshExtra();

	LaraItem->hitPoints = -16384;
	Lara.air = -1;
	Lara.gunStatus = LG_HANDS_BUSY;
	Lara.gunType = WEAPON_NONE;

	Camera.flags = 1;
	Camera.targetAngle = ANGLE(170);
	Camera.targetElevation = -ANGLE(25);
}

void TyrannosaurControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*) item->data;
	
	short head = 0;
	short angle = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState == 1)
			item->goalAnimState = 5;
		else
			item->goalAnimState = 1;
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (item->touchBits)
			LaraItem->hitPoints -= (item->currentAnimState == 3) ? 10 : 1;

		creature->flags = (creature->mood != ESCAPE_MOOD && !info.ahead &&
			info.enemyFacing > -FRONT_ARC && info.enemyFacing < FRONT_ARC);

		if (!creature->flags && info.distance > SQUARE(1500) && info.distance < SQUARE(4096) && info.bite)
			creature->flags = 1;

		switch (item->currentAnimState)
		{
		case 1:
			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else if (info.distance < SQUARE(1500) && info.bite)
				item->goalAnimState = 7;
			else if (creature->mood == BORED_MOOD || creature->flags)
				item->goalAnimState = 2;
			else
				item->goalAnimState = 3;
			break;

		case 2:
			creature->maximumTurn = ANGLE(2);

			if (creature->mood != BORED_MOOD || !creature->flags)
				item->goalAnimState = 1;
			else if (info.ahead && GetRandomControl() < 0x200)
			{
				item->requiredAnimState = 6;
				item->goalAnimState = 1;
			}
			break;

		case 3:
			creature->maximumTurn = ANGLE(4);

			if (info.distance < SQUARE(5120) && info.bite)
				item->goalAnimState = 1;
			else if (creature->flags)
				item->goalAnimState = 1;
			else if (creature->mood != ESCAPE_MOOD && info.ahead && GetRandomControl() < 0x200)
			{
				item->requiredAnimState = 6;
				item->goalAnimState = 1;
			}
			else if (creature->mood == BORED_MOOD)
				item->goalAnimState = 1;
			break;

		case 7:
			if (item->touchBits & 0x3000)
			{
				LaraItem->hitPoints -= 1500;
				LaraItem->hitStatus = true;
				item->goalAnimState = 8;
				if (LaraItem == LaraItem)
					LaraTyrannosaurDeath(item);
			}

			item->requiredAnimState = 2;
			break;
		}
	}

	CreatureJoint(item, 0, (short)(head * 2));
	creature->jointRotation[1] = creature->jointRotation[0];

	CreatureAnimation(itemNum, angle, 0);

	item->collidable = true;
}

void ApeControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*) item->data;
	
	short head = 0;
	short angle = 0;
	short random = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 5)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 7 + (short)(GetRandomControl() / 0x4000);
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 5;
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (item->hitStatus || info.distance < SQUARE(2048))
			creature->flags |= 1;

		switch (item->currentAnimState)
		{
		case 1:
			if (creature->flags & 2)
			{
				item->pos.yRot -= ANGLE(90);
				creature->flags -= 2;
			}
			else if (item->flags & 4)
			{
				item->pos.yRot += ANGLE(90);
				creature->flags -= 4;
			}

			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else if (info.bite && info.distance < SQUARE(430))
				item->goalAnimState = 4;
			else if (!(creature->flags & 1) &&
				info.zoneNumber == info.enemyZone && info.ahead)
			{
				random = (short)(GetRandomControl() >> 5);
				if (random < 0xA0)
					item->goalAnimState = 10;
				else if (random < 0x140)
					item->goalAnimState = 6;
				else if (random < 0x1E0)
					item->goalAnimState = 7;
				else if (random < 0x2F0)
				{
					item->goalAnimState = 8;
					creature->maximumTurn = 0;
				}
				else
				{
					item->goalAnimState = 9;
					creature->maximumTurn = 0;
				}
			}
			else
				item->goalAnimState = 3;
			break;

		case 3:
			creature->maximumTurn = ANGLE(5);

			if (creature->flags == 0 && info.angle > -ANGLE(45) && info.angle < ANGLE(45))
				item->goalAnimState = 1;
			else if (info.ahead && (item->touchBits & 0xFF00))
			{
				item->requiredAnimState = 4;
				item->goalAnimState = 1;
			}
			else if (creature->mood != ESCAPE_MOOD)
			{
				random = (short)GetRandomControl();
				if (random < 0xA0)
				{
					item->requiredAnimState = 10;
					item->goalAnimState = 1;
				}
				else if (random < 0x140)
				{
					item->requiredAnimState = 6;
					item->goalAnimState = 1;
				}
				else if (random < 0x1E0)
				{
					item->requiredAnimState = 7;
					item->goalAnimState = 1;
				}
			}
			break;

		case 8:
			if (!(creature->flags & 4))
			{
				item->pos.yRot -= ANGLE(90);
				creature->flags |= 4;
			}

			item->goalAnimState = 1;
			break;

		case 9:
			if (!(creature->flags & 2))
			{
				item->pos.yRot += ANGLE(90);
				creature->flags |= 2;
			}

			item->goalAnimState = 1;
			break;

		case 4:
			if (!item->requiredAnimState && (item->touchBits & 0xFF00))
			{
				CreatureEffect(item, &apeBite, DoBloodSplat);

				LaraItem->hitPoints -= 200;
				LaraItem->hitStatus = true;

				item->requiredAnimState = 1;
			}
			break;
		}
	}

	CreatureJoint(item, 0, head);

	if (item->currentAnimState != 11)
	{
		if (creature->flags & 2)
		{
			item->pos.yRot -= ANGLE(90);
			creature->flags -= 2;
		}
		else if (item->flags & 4)
		{
			item->pos.yRot += ANGLE(90);
			creature->flags -= 4;
		}

		int vault = CreatureVault(itemNum, angle, 2, 75);

		switch (vault)
		{
		case 2:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 19;
			item->currentAnimState = 11;
			item->frameNumber = Anims[item->animNumber].frameBase;
			break;

		default:
			return;
		}
	}
	else
		CreatureAnimation(itemNum, angle, 0);
}

void RatControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*) item->data;
	
	short head = 0;
	short angle = 0;
	short random = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 6)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 9;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 6;
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, ANGLE(6));

		switch (item->currentAnimState)
		{
		case 4:
			if (creature->mood == BORED_MOOD || creature->mood == STALK_MOOD)
			{
				short random = (short)GetRandomControl();
				if (random < 0x500)
					item->requiredAnimState = 3;
				else if (random > 0xA00)
					item->requiredAnimState = 1;
			}
			else if (info.distance < SQUARE(340))
				item->requiredAnimState = 5;
			else
				item->requiredAnimState = 1;

			if (item->requiredAnimState)
				item->goalAnimState = 2;
			break;

		case 2:
			creature->maximumTurn = 0;

			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			break;

		case 1:
			creature->maximumTurn = ANGLE(6);

			if (creature->mood == BORED_MOOD || creature->mood == STALK_MOOD)
			{
				random = (short)GetRandomControl();
				if (random < 0x500)
				{
					item->requiredAnimState = 3;
					item->goalAnimState = 2;
				}
				else if (random < 0xA00)
					item->goalAnimState = 2;
			}
			else if (info.ahead && info.distance < SQUARE(340))
				item->goalAnimState = 2;
			break;

		case 5:
			if (!item->requiredAnimState && (item->touchBits & 0x7F))
			{
				CreatureEffect(item, &mouseBite, DoBloodSplat);
				LaraItem->hitPoints -= 20;
				LaraItem->hitStatus = true;
				item->requiredAnimState = 2;
			}
			break;

		case 3:
			if (GetRandomControl() < 0x500)
				item->goalAnimState = 2;
			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, 0);
}

void InitialiseLittleBeetle(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	item->itemFlags[0] = (item->triggerFlags / 1000) & 1;
	item->itemFlags[1] = (item->triggerFlags / 1000) & 2;
	item->itemFlags[2] = (item->triggerFlags / 1000) & 4;

	item->triggerFlags = item->triggerFlags % 1000;

	if (!item->itemFlags[1])
	{
		if (item->pos.yRot <= 4096 || item->pos.yRot >= 28672)
		{
			if (!(item->pos.yRot >= -4096 || item->pos.yRot <= -28672))
				item->pos.xPos += 512;
		}
		else
		{
			item->pos.xPos -= 512;
		}

		if (item->pos.yRot <= -8192 || item->pos.yRot >= 0x2000)
		{
			if (item->pos.yRot < -20480 || item->pos.yRot > 20480)
			{
				item->pos.zPos += 512;
			}
		}
		else
		{
			item->pos.zPos -= 512;
		}
	}
}

void LittleBeetleControl(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
}

void InitialiseHarpy(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[ID_HARPY].animIndex + 4;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
}

void HarpyControl(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (!CreatureActive(itemNum))
		return;

	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	short angle = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;

	if (item->hitPoints <= 0)
	{
		short state = item->currentAnimState - 9;
		item->hitPoints = 0;

		if (state)
		{
			state--;
			if (state)
			{
				if (state == 1)
				{
					item->pos.xRot = 0;
					item->pos.yPos = item->floor;
				}
				else
				{
					item->animNumber = obj->animIndex + 5;
					item->frameNumber = Anims[item->animNumber].frameBase;
					item->currentAnimState = 9;
					item->speed = 0;
					item->gravityStatus = true;
					item->pos.xRot = 0;
				}

				CreatureTilt(item, 0);

				CreatureJoint(item, 0, joint0);
				CreatureJoint(item, 1, joint1);
				CreatureJoint(item, 2, joint2);

				CreatureAnimation(itemNum, angle, 0);

				return;
			}
		}
		else
		{
			item->goalAnimState = 10;
		}

		if (item->pos.yPos >= item->floor)
		{
			item->pos.yPos = item->floor;
			item->fallspeed = 0;
			item->goalAnimState = 11;
			item->gravityStatus = false;
		}

		item->pos.xRot = 0;
	}
	else
	{
		creature->enemy = LaraItem;

		CREATURE_INFO* baddie = &BaddieSlots[0];
		int minDistance = 0x7FFFFFFF;

		creature->enemy = NULL;

		for (int i = 0; i < NUM_SLOTS; i++, baddie++)
		{
			if (baddie->itemNum == NO_ITEM || baddie->itemNum == itemNum)
				continue;

			ITEM_INFO* target = &Items[baddie->itemNum];

			if (target->objectNumber == ID_LARA_DOUBLE)
			{
				int dx = target->pos.xPos - item->pos.xPos;
				int dz = target->pos.zPos - item->pos.zPos;
				int distance = dx * dx + dz * dz;

				if (distance < minDistance)
				{
					creature->enemy = target;
					minDistance = distance;
				}
			}
		}

		AI_INFO info;

		CreatureAIInfo(item, &info);

		if (creature->enemy != LaraItem)
		{
			//phd_atan(LaraItem->pos.zPos - item->pos.zPos, LaraItem->pos.xPos - item->pos.xPos);
		}

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (info.ahead)
		{
			joint0 = info.angle >> 1;
			joint1 = info.angle >> 1;
			joint0 = info.xAngle;
		}

		int height = 0;
		int dy = 0;

		switch (item->currentAnimState)
		{
		case 1:
			creature->flags = 0;
			creature->maximumTurn = ANGLE(7);

			if (creature->enemy)
			{
				height = item->pos.yPos + 2048;
				if (creature->enemy->pos.yPos > height && item->floor > height)
				{
					item->goalAnimState = 3;
					break;
				}
			}
			if (info.ahead)
			{
				dy = abs(creature->enemy->pos.yPos - item->pos.yPos);
				if (dy <= 1024)
				{
					if (info.distance < SQUARE(341))
					{
						item->goalAnimState = 6;
						break;
					}
					if (dy <= 1024 && info.distance < SQUARE(2048))
					{
						item->goalAnimState = 4;
						break;
					}
				}
			}
			if (creature->enemy != LaraItem
				|| !Targetable(item, &info)
				|| info.distance <= SQUARE(3584)
				|| !(GetRandomControl() & 1))
			{
				item->goalAnimState = 2;
				break;
			}
			item->goalAnimState = 8;
			item->itemFlags[0] = 0;
			break;

		case 2:
			creature->maximumTurn = ANGLE(7);
			creature->flags = 0;

			if (item->requiredAnimState)
			{
				item->goalAnimState = item->requiredAnimState;
				if (item->requiredAnimState == 8)
				{
					item->itemFlags[0] = 0;
				}
				break;
			}
			if (item->hitStatus)
			{
				item->goalAnimState = 7;
				break;
			}
			if (info.ahead)
			{
				if (info.distance >= SQUARE(341))
				{
					if (info.ahead && info.distance >= SQUARE(2048) &&
						info.distance > SQUARE(3584) && GetRandomControl() & 1)
					{
						item->goalAnimState = 8;
						item->itemFlags[0] = 0;
					}
					else
					{
						item->goalAnimState = 4;
					}
				}
				else
				{
					item->goalAnimState = 6;
				}

				break;
			}
			if (GetRandomControl() & 1)
			{
				item->goalAnimState = 7;
				break;
			}
			if (!info.ahead)
			{
				item->goalAnimState = 4;
				break;
			}
			if (info.distance >= SQUARE(341))
			{
				if (info.ahead && info.distance >= SQUARE(2048) && 
					info.distance > SQUARE(3584) && GetRandomControl() & 1)
				{
					item->goalAnimState = 8;
					item->itemFlags[0] = 0;
				}
				else
				{
					item->goalAnimState = 4;
				}
			}
			else
			{
				item->goalAnimState = 6;
			}

			break;

		case 3:
			if (!creature->enemy || creature->enemy->pos.yPos < item->pos.yPos + 2048)
			{
				item->goalAnimState = 1;
			}

			break;

		case 4:
			creature->maximumTurn = ANGLE(2);

			if (info.ahead && info.distance < SQUARE(2048))
			{
				item->goalAnimState = 5;
			}
			else
			{
				item->goalAnimState = 13;
			}

			break;

		case 5:
			creature->maximumTurn = ANGLE(2);
			item->goalAnimState = 2;

			if (item->touchBits & 0x14
				|| creature->enemy && creature->enemy != LaraItem && 
				abs(creature->enemy->pos.yPos - item->pos.yPos) <= 1024 && 
				info.distance < SQUARE(2048))
			{
				LaraItem->hitPoints -= 10;
				LaraItem->hitStatus = true;

				if (item->touchBits & 0x10)
				{
					CreatureEffect2(
						item,
						&harpyBite1,
						5,
						-1,
						DoBloodSplat);
				}
				else
				{
					CreatureEffect2(
						item,
						&harpyBite2,
						5,
						-1,
						DoBloodSplat);
				}
			}

			break;

		case 6:
			creature->maximumTurn = ANGLE(2);

			if (creature->flags == 0
				&& (item->touchBits & 0x300000
					|| creature->enemy && creature->enemy != LaraItem && 
					abs(creature->enemy->pos.yPos - item->pos.yPos) <= 1024 && 
					info.distance < SQUARE(2048)))
			{
				LaraItem->hitPoints -= 100;
				LaraItem->hitStatus = true;

				CreatureEffect2(
					item,
					&harpyBite3,
					10,
					-1,
					DoBloodSplat);

				if (creature->enemy == LaraItem)
				{
					Lara.dpoisoned += 2048;
				}

				creature->flags = 1;
			}
			
			break;

		case 8:
			// Flame attack
			HarpyAttack(item, itemNum);
			break;

		case 12:
			if (info.ahead && info.distance > SQUARE(3584))
			{
				item->goalAnimState = 2;
				item->requiredAnimState = 8;
			}
			else if (GetRandomControl() & 1)
			{
				item->goalAnimState = 1;
			}

			break;

		case 13:
			item->goalAnimState = 2;
			break;

		default:
			break;
		}
	}

	CreatureTilt(item, 0);

	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);

	CreatureAnimation(itemNum, angle, 0);
}

void HarpySparks2(int x, int y, int z, int xv, int yv, int zv)
{
	int dx = LaraItem->pos.xPos - x;
	int dz = LaraItem->pos.zPos - z;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

		spark->on = true;
		spark->sR = 0;
		spark->sG = 0;
		spark->sB = 0;
		spark->dR = spark->dG = (GetRandomControl() & 0x7F) + 64;
		spark->dB = 0;
		spark->life = 16;
		spark->sLife = 16;
		spark->colFadeSpeed = 4;
		spark->y = y;
		spark->transType = 2;
		spark->fadeToBlack = 4;
		spark->x = x;
		spark->z = z;
		spark->xVel = xv;
		spark->yVel = yv;
		spark->zVel = zv;
		spark->friction = 34;
		spark->scalar = 1;
		spark->sSize = spark->size = (GetRandomControl() & 3) + 4;
		spark->maxYvel = 0;
		spark->gravity = 0;
		spark->dSize = (GetRandomControl() & 1) + 1;
		spark->flags = 0;
	}
}

void HarpyAttack(ITEM_INFO* item, short itemNum)
{
	item->itemFlags[0]++;

	PHD_VECTOR pos1;

	pos1.x = harpyAttack1.x;
	pos1.y = harpyAttack1.y;
	pos1.z = harpyAttack1.z;

	GetJointAbsPosition(item, &pos1, harpyAttack1.meshNum);

	PHD_VECTOR pos2;

	pos2.x = harpyAttack2.x;
	pos2.y = harpyAttack2.y;
	pos2.z = harpyAttack2.z;

	GetJointAbsPosition(item, &pos2, harpyAttack2.meshNum);

	if (item->itemFlags[0] >= 24 && item->itemFlags[0] <= 47 && (GetRandomControl() & 0x1F) < item->itemFlags[0])
	{
		for (int i = 0; i < 2; i++)
		{
			int dx = (GetRandomControl() & 0x7FF) + pos1.x - 1024;
			int dy = (GetRandomControl() & 0x7FF) + pos1.y - 1024;
			int dz = (GetRandomControl() & 0x7FF) + pos1.z - 1024;

			HarpySparks2(dx, dy, dz, 8 * (pos1.x - dx), 8 * (pos1.y - dy), 8 * (pos1.z - dz));
		
			dx = (GetRandomControl() & 0x7FF) + pos2.x - 1024;
			dy = (GetRandomControl() & 0x7FF) + pos2.y - 1024;
			dz = (GetRandomControl() & 0x7FF) + pos2.z - 1024;

			HarpySparks2(dx, dy, dz, 8 * (pos2.x - dx), 8 * (pos2.y - dy), 8 * (pos2.z - dz));
		}
	}

	int something = 2 * item->itemFlags[0];
	if (something > 64)
	{
		something = 64;
	}
	if (something < 80)
	{
		if ((Wibble & 0xF) == 8)
		{
			HarpySparks1(itemNum, 4, something);
		}
		else if (!(Wibble & 0xF))
		{
			HarpySparks1(itemNum, 5, something);
		}
	}

	if (item->itemFlags[0] >= 61)
	{
		if (item->itemFlags[0] <= 65 && GlobalCounter & 1)
		{
			PHD_VECTOR pos3;
			
			pos3.x = harpyAttack1.x;
			pos3.y = harpyAttack1.y * 2;
			pos3.z = harpyAttack1.z;

			GetJointAbsPosition(item, &pos3, harpyAttack1.meshNum);

			PHD_3DPOS pos;

			pos.xPos = pos1.x;
			pos.yPos = pos1.y;
			pos.zPos = pos1.z;
			
			short angles[2];
			phd_GetVectorAngles(pos3.x - pos1.x, 
								pos3.y - pos1.y,
								pos3.z - pos1.z,
								angles);

			pos.xRot = angles[1];
			pos.yRot = angles[0];
			pos.zRot = 0;

			HarpyBubbles(&pos, item->roomNumber, 2);
		}
		if (item->itemFlags[0] >= 61 && item->itemFlags[0] <= 65 && !(GlobalCounter & 1))
		{
			PHD_VECTOR pos3;

			pos3.x = harpyAttack2.x;
			pos3.y = harpyAttack2.y * 2;
			pos3.z = harpyAttack2.z;

			GetJointAbsPosition(item, &pos3, harpyAttack2.meshNum);

			PHD_3DPOS pos;

			pos.xPos = pos1.x;
			pos.yPos = pos1.y;
			pos.zPos = pos1.z;

			short angles[2];
			phd_GetVectorAngles(pos3.x - pos1.x,
				pos3.y - pos1.y,
				pos3.z - pos1.z,
				angles);

			pos.xRot = angles[1];
			pos.yRot = angles[0];
			pos.zRot = 0;

			HarpyBubbles(&pos, item->roomNumber, 2);
		}
	}
}

void HarpyBubbles(PHD_3DPOS* pos, short roomNumber, int count)
{
	short fxNumber = CreateNewEffect(roomNumber);
	if (fxNumber != -1)
	{
		FX_INFO* fx = &Effects[fxNumber];

		fx->pos.xPos = pos->xPos;
		fx->pos.yPos = pos->yPos - (GetRandomControl() & 0x3F) - 32;
		fx->pos.zPos = pos->zPos;
		fx->pos.xRot = pos->xRot;
		fx->pos.yRot = pos->yRot;
		fx->pos.zRot = 0;
		fx->roomNumber = roomNumber;
		fx->counter = 2 * GetRandomControl() + -32768;
		fx->objectNumber = ID_ENERGY_BUBBLES;
		fx->speed = (GetRandomControl() & 0x1F) + 96;
		fx->flag1 = count;
		fx->frameNumber = Objects[ID_ENERGY_BUBBLES].meshIndex + 2 * count;
	}
}

void HarpySparks1(short itemNum, byte num, int size)
{
	ITEM_INFO* item = &Items[itemNum];

	int dx = LaraItem->pos.xPos - item->pos.xPos;
	int dz = LaraItem->pos.zPos - item->pos.zPos;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

		spark->on = true;
		spark->sR = 0;
		spark->sG = 0;
		spark->sB = 0;
		spark->dB = 0;
		spark->dG = spark->dR = (GetRandomControl() & 0x7F) + 32;
		spark->fadeToBlack = 8;
		spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
		spark->transType = 2;
		spark->life = spark->sLife = (GetRandomControl() & 7) + 20;
		spark->y = 0;
		spark->x = (GetRandomControl() & 0xF) - 8;
		spark->z = (GetRandomControl() & 0xF) - 8;
		spark->yVel = 0;
		spark->xVel = GetRandomControl() - 128;
		spark->friction = 5;
		spark->flags = 4762;
		spark->zVel = GetRandomControl() - 128;
		spark->rotAng = GetRandomControl() & 0xFFF;
		if (GetRandomControl() & 1)
		{
			spark->rotAdd = -32 - (GetRandomControl() & 0x1F);
		}
		else
		{
			spark->rotAdd = (GetRandomControl() & 0x1F) + 32;
		}
		spark->maxYvel = 0;
		spark->gravity = (GetRandomControl() & 0x1F) + 16;
		spark->fxObj = itemNum;
		spark->nodeNumber = num;
		spark->scalar = 2;
		spark->sSize = spark->size = GetRandomControl() & 0xF + size;
		spark->dSize = spark->size >> 4;
	}
}

void InitialiseCrocodile(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	OBJECT_INFO* obj = &Objects[item->objectNumber];
	ROOM_INFO* room = &Rooms[item->roomNumber];

	ClearItem(itemNum);

	if (room->flags & ENV_FLAG_WATER)
	{
		item->animNumber = obj->animIndex + 12;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->goalAnimState = 8;
		item->currentAnimState = 8;
	}
	else
	{
		item->animNumber = obj->animIndex;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->goalAnimState = 1;
		item->currentAnimState = 1;
	}
}

void CrocodileControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	int x = item->pos.xPos + SIN(item->pos.yRot) << 10 >> W2V_SHIFT;
	int y = item->pos.yPos;
	int z = item->pos.zPos + COS(item->pos.yRot) << 10 >> W2V_SHIFT;

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, y, z);

	if (abs(y - height1) > 512)
		height1 = y;

	x = item->pos.xPos - SIN(item->pos.yRot) << 10 >> W2V_SHIFT;
	y = item->pos.yPos;
	z = item->pos.zPos - COS(item->pos.yRot) << 10 >> W2V_SHIFT;

	roomNumber = item->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, y, z);

	if (abs(y - height2) > 512)
		height2 = y;

	short at = ATAN(2048, height2 - height1);
	short angle = 0;
	short joint0 = 0;
	short joint2 = 0;

	if (item->hitPoints <= 0)
	{
		item->hitPoints = 0;
		
		if (item->currentAnimState != 7 && item->currentAnimState != 10)
		{
			if (Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
			{
				item->animNumber = obj->animIndex + 16;
				item->goalAnimState = 10;
				item->frameNumber = Anims[item->animNumber].frameBase;
				item->currentAnimState = 10;
				item->hitPoints = -16384;
			}
			else
			{
				item->animNumber = obj->animIndex + 11;
				item->goalAnimState = 7;
				item->frameNumber = Anims[item->animNumber].frameBase;
				item->currentAnimState = 7;
			}
		}
		
		if (Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
			CreatureFloat(itemNum);
	}
	else
	{
		if (item->aiBits)
			GetAITarget(creature);
		else if (creature->hurtByLara)
			creature->enemy = LaraItem;

		AI_INFO info;
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (item->hitStatus
			|| info.distance < SQUARE(1536)
			|| TargetVisible(item, &info) && info.distance < SQUARE(5120))
		{
			if (!creature->alerted)
				creature->alerted = true;
			AlertAllGuards(itemNum);
		}

		joint0 = 4 * angle;

		switch (item->currentAnimState)
		{
		case 1:
			creature->maximumTurn = 0;

			if (item->aiBits & GUARD)
			{
				joint0 = item->itemFlags[0];
				item->goalAnimState = 1;
				item->itemFlags[0] += item->itemFlags[1];

				if (!(GetRandomControl() & 0x1F))
				{
					if (GetRandomControl() & 1)
					{
						item->itemFlags[1] = 0;
					}
					else
					{
						item->itemFlags[1] = (GetRandomControl() & 1) != 0 ? 12 : -12;
					}
				}
				
				if (item->itemFlags[0] <= 1024)
				{
					if (item->itemFlags[0] < -1024)
					{
						item->itemFlags[0] = -1024;
					}
				}
				else
				{
					item->itemFlags[0] = 1024;
				}
			}
			else if (info.angle && info.distance < SQUARE(768))
			{
				item->goalAnimState = 5;
			}
			else
			{
				if (!info.ahead || info.distance >= SQUARE(1024))
				{
					item->goalAnimState = 2;
					break;
				}
				item->goalAnimState = 3;
			}

			break;

		case 2:
			creature->maximumTurn = ANGLE(3);
			
			if (item->requiredAnimState)
			{
				item->goalAnimState = item->requiredAnimState;
			}
			else
			{
				if (info.angle && info.distance < SQUARE(768))
				{
					item->goalAnimState = 1;
				}
				if (info.ahead)
				{
					if (info.distance < SQUARE(1024))
					{
						item->goalAnimState = 3;
					}
				}
			}

			break;

		case 3:
			creature->maximumTurn = ANGLE(3);
			creature->LOT.step = 256;
			creature->LOT.drop = -256;
			
			if (item->requiredAnimState)
			{
				item->goalAnimState = item->requiredAnimState;
			}
			else if (info.angle && info.distance < SQUARE(768))
			{
				item->goalAnimState = 1;
			}
			else if (!info.ahead || info.distance > SQUARE(1536))
			{
				item->goalAnimState = 2;
			}

			break;

		case 5:
			if (item->frameNumber == Anims[item->animNumber].frameBase)
			{
				item->requiredAnimState = 0;
			}
			if (info.angle && item->touchBits & 0x300)
			{
				if (!item->requiredAnimState)
				{
					CreatureEffect2(
						item,
						&crocodileBiteInfo,
						10,
						-1,
						DoBloodSplat);

					LaraItem->hitPoints -= 120;
					LaraItem->hitStatus = true;

					item->requiredAnimState = 1;
				}
			}
			else
			{
				item->goalAnimState = 1;
			}

			break;

		case 8:
			creature->maximumTurn = ANGLE(3);
			creature->LOT.step = 20480;
			creature->LOT.drop = -20480;

			if (item->requiredAnimState)
			{
				item->goalAnimState = item->requiredAnimState;
			}
			else if (info.angle)
			{
				if (item->touchBits & 0x300)
				{
					item->goalAnimState = 9;
				}
			}

			break;

		case 9:
			if (item->frameNumber == Anims[item->animNumber].frameBase)
			{
				item->requiredAnimState = 0;
			}
			if (info.angle && item->touchBits & 0x300)
			{
				if (!item->requiredAnimState)
				{
					CreatureEffect2(
						item,
						&crocodileBiteInfo,
						10,
						-1,
						DoBloodSplat);

					LaraItem->hitPoints -= 120;
					LaraItem->hitStatus = true;

					item->requiredAnimState = 8;
				}
			}
			else
			{
				item->goalAnimState = 8;
			}

			break;

		default:
			break;
		}
	}

	CreatureTilt(item, 0);

	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint0);
	CreatureJoint(item, 2, -joint0);
	CreatureJoint(item, 3, -joint0);

	short xRot = item->pos.xRot;

	if (!(abs(angle - item->pos.xRot) < 256 || item->currentAnimState >= 8))
	{
		if (angle <= xRot)
		{
			if (angle < xRot)
				item->pos.xRot = xRot - 256;
		}
		else
		{
			item->pos.xRot = xRot + 256;
		}
	}
	else
	{
		if (item->currentAnimState < 8)
			item->pos.xRot = angle;

		CreatureAnimation(itemNum, angle, 0);

		roomNumber = item->roomNumber;

		if (item->currentAnimState == 8)
		{
			GetFloor(
				item->pos.xPos + (SIN(item->pos.yRot) << 10 >> W2V_SHIFT),
				item->pos.yPos,
				item->pos.zPos + (COS(item->pos.yRot) << 10 >> W2V_SHIFT),
				&roomNumber);
		}
		else
		{
			GetFloor(
				item->pos.xPos + (SIN(item->pos.yRot) << 9 >> W2V_SHIFT),
				item->pos.yPos,
				item->pos.zPos + (COS(item->pos.yRot) << 10 >> W2V_SHIFT),
				&roomNumber);
		}

		if (Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
		{
			if (Rooms[roomNumber].flags & ENV_FLAG_WATER)
			{
				if (item->currentAnimState == 2)
				{
					item->requiredAnimState = 3;
					item->goalAnimState = 3;
				}
				else if (item->currentAnimState == 3)
				{
					item->requiredAnimState = 8;
					item->goalAnimState = 8;
				}
				else if (item->animNumber != obj->animIndex + 17)
				{
					creature->LOT.step = 20480;
					creature->LOT.drop = -20480;
					creature->LOT.fly = 16;
					
					CreatureUnderwater(item, 256);
				}
			}
			else
			{
				item->requiredAnimState = 3;
				item->goalAnimState = 3;
				creature->LOT.step = 256;
				creature->LOT.drop = -256;
				creature->LOT.fly = 0;

				CreatureUnderwater(item, 0);
			}
		}
		else
		{
			creature->LOT.fly = 0;
		}
		
		return;
	}
}

void InitialiseSphinx(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[item->animNumber].animIndex + 1;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
}

void SphinxControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	int x = item->pos.xPos + 614 * SIN(item->pos.yRot) >> W2V_SHIFT;
	int y = item->pos.yPos;
	int z = item->pos.zPos + 614 * COS(item->pos.yRot) >> W2V_SHIFT;

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, y, z);

	if (item->currentAnimState == 5 && floor->stopper)
	{
		ROOM_INFO* room = &Rooms[item->roomNumber];

		for (int i = 0; i < room->numMeshes; i++)
		{
			MESH_INFO* mesh = &room->mesh[i];

			if (mesh->z >> 10 == z >> 10 && mesh->x >> 10 == x >> 10 && mesh->staticNumber >= 50)
			{
				ShatterObject(NULL, mesh, -64, item->roomNumber, 0);
				SoundEffect(SFX_TR4_HIT_ROCK_ID347, &item->pos, 0);

				mesh->Flags &= ~0x100;
				floor->stopper = false;

				TestTriggers(TriggerIndex, 1, 0);
			}
		}
	}

	x = item->pos.xPos - 614 * SIN(item->pos.yRot) >> W2V_SHIFT;
	y = item->pos.yPos;
	z = item->pos.zPos - 614 * COS(item->pos.yRot) >> W2V_SHIFT;

	roomNumber = item->roomNumber;

	floor = GetFloor(x, y, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, y, z);

	ATAN(1228, height2 - height1);

	if (item->aiBits)
		GetAITarget(creature);
	else
		creature->enemy = LaraItem;

	AI_INFO info;
	CreatureAIInfo(item, &info);

	if (creature->enemy != LaraItem)
		ATAN(LaraItem->pos.zPos - item->pos.zPos, LaraItem->pos.xPos - item->pos.xPos);

	GetCreatureMood(item, &info, VIOLENT);
	CreatureMood(item, &info, VIOLENT);

	short angle = CreatureTurn(item, creature->maximumTurn);

	int dx = abs(item->itemFlags[2] - item->pos.xPos);
	int dz = abs(item->itemFlags[3] - item->pos.zPos);

	switch (item->currentAnimState)
	{
	case 1:
		creature->maximumTurn = 0;

		if (info.distance < SQUARE(1024) || item->triggerFlags)
		{
			item->goalAnimState = 3;
		}
		
		if (GetRandomControl() == 0)
		{
			item->goalAnimState = 2;
		}

		break;

	case 2:
		creature->maximumTurn = 0;

		if (info.distance < SQUARE(1024) || item->triggerFlags)
		{
			item->goalAnimState = 3;
		}
		
		if (GetRandomControl() == 0)
		{
			item->goalAnimState = 1;
		}

		break;

	case 4:
		creature->maximumTurn = ANGLE(3);
		if (info.distance > SQUARE(1024) && abs(info.angle) <= 512 || item->requiredAnimState == 5)
		{
			item->goalAnimState = 5;
		}
		else if (info.distance < SQUARE(2048) && item->goalAnimState != 5)
		{
			if (height2 <= item->pos.yPos + 256 && height2 >= item->pos.yPos - 256)
			{
				item->goalAnimState = 9;
				item->requiredAnimState = 6;
			}
		}

		break;

	case 5:
		creature->maximumTurn = 60;

		if (creature->flags == 0)
		{
			if (item->touchBits & 0x40)
			{
				CreatureEffect2(
					item,
					&sphinxBiteInfo,
					20,
					-1,
					DoBloodSplat);

				LaraItem->hitPoints -= 200;
				creature->flags = 1;
			}
		}
		if (dx >= 50 || dz >= 50 || item->animNumber != Objects[ID_SPHINX].animIndex)
		{
			if (info.distance > SQUARE(2048) && abs(info.angle) > 512)
			{
				item->goalAnimState = 9;
			}
		}
		else
		{
			item->goalAnimState = 7;
			item->requiredAnimState = 6;
			creature->maximumTurn = 0;
		}

		break;

	case 6:
		creature->maximumTurn = ANGLE(3);
		if (info.distance > SQUARE(2048) || height2 > item->pos.yPos + 256 || height2 < item->pos.yPos - 256)
		{
			item->goalAnimState = 9;
			item->requiredAnimState = 5;
		}

		break;

	case 7:
		//v32 = item->roomNumber;
		//v36 = (signed short)item->currentAnimState - 1;
		
		roomNumber = item->roomNumber;
		
		floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		
		if (item->frameNumber == Anims[item->animNumber].frameBase)
		{
			TestTriggers(TriggerIndex, 1, 0);

			if (item->touchBits & 0x40)
			{
				CreatureEffect2(
					item,
					&sphinxBiteInfo,
					50,
					-1,
					DoBloodSplat);

				LaraItem->hitPoints = 0;
			}
		}

		break;

	case 9:
		creature->flags = 0;

		if (item->requiredAnimState == 6)
		{
			item->goalAnimState = 6;
		}
		else
		{
			item->goalAnimState = 4;
		}

		break;

	default:
		break;
	}

	item->itemFlags[2] = item->pos.xPos;
	item->itemFlags[3] = item->pos.zPos;

	CreatureAnimation(itemNum, angle, 0);
}

void InitialiseMonkey(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	InitialiseCreature(itemNumber);

	item->animNumber = Objects[ID_MONKEY].animIndex + 2;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->currentAnimState = 6;
	item->goalAnimState = 6;
}

void MonkeyControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &Items[itemNumber];
	CREATURE_INFO* creature  = (CREATURE_INFO*)item->data;
	
	short headX = 0;
	short headY = 0;
	short torsoY = 0;
	short angle = 0;
	short tilt = 0;
	int x = 0;
	int z = 0;
	int distance = 0;
	int dx = 0;
	int dz = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 11)
		{
			item->meshBits = -1; 
			item->animNumber = Objects[ID_MONKEY].animIndex + 14;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 11;
		}
	}
	else
	{
		GetAITarget(creature);
		
		if (creature->hurtByLara)
			creature->enemy = LaraItem;
		else
		{
			int minDistance = 0x7FFFFFFF;
			creature->enemy = NULL;
			CREATURE_INFO* currentCreature = BaddieSlots;

			for (int i = 0; i < NUM_SLOTS; i++, currentCreature++)
			{
				if (currentCreature->itemNum == NO_ITEM || currentCreature->itemNum == itemNumber)
					continue;

				ITEM_INFO * target = &Items[currentCreature->itemNum];
				if (target->objectNumber == ID_LARA || target->objectNumber == ID_MONKEY)
					continue;

				if (target->objectNumber == ID_SMALLMEDI_ITEM)
				{
					x = target->pos.xPos - item->pos.xPos;
					z = target->pos.zPos - item->pos.zPos;
					distance = SQUARE(x) + SQUARE(z);

					if (distance < minDistance)
					{
						creature->enemy = target;
						minDistance = distance;
					}
				}
			}
		}

		if (item->aiBits != MODIFY)
		{
			if (item->carriedItem != NO_ITEM)
				item->meshBits = 0xFFFFFEFF;  // Swap the head
			else
				item->meshBits = -1;
		}
		else
		{
			if (item->carriedItem != NO_ITEM)
				item->meshBits = 0xFFFF6E6F; //Swap the head and the feet
			else
				item->meshBits = 0xFFFF6F6F; //Just swap the feet
		}

		AI_INFO info;
		AI_INFO laraInfo;

		CreatureAIInfo(item, &info);

		// Monkeys are friendly until Lara hurts them
		if (!creature->hurtByLara && creature->enemy == LaraItem)
			creature->enemy = NULL;

		if (creature->enemy == LaraItem)
		{
			laraInfo.angle = info.angle;
			laraInfo.distance = info.distance;
		}
		else
		{
			dx = LaraItem->pos.xPos - item->pos.xPos;
			dz = LaraItem->pos.zPos - item->pos.zPos;
			
			laraInfo.angle = ATAN(dz, dz) - item->pos.yRot; //only need to fill out the bits of laraInfo that will be needed by TargetVisible
			laraInfo.distance = SQUARE(dx) + SQUARE(dz);
		}

		GetCreatureMood(item, &info, VIOLENT);
		if (g_LaraExtra.Vehicle != NO_ITEM)
			creature->mood = ESCAPE_MOOD;
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		ITEM_INFO* enemy = creature->enemy; //TargetVisible uses enemy, so need to fill this in as lara if we're doing other things
		creature->enemy = LaraItem;
		
		if (item->hitStatus)
			AlertAllGuards(itemNumber);

		creature->enemy = enemy;

		switch (item->currentAnimState)
		{
		case 6:
			creature->flags = 0;
			creature->maximumTurn = 0;

			torsoY = laraInfo.angle;

			if (item->aiBits & GUARD)
			{
				torsoY = AIGuard(creature);
				if (!(GetRandomControl() & 0xF))
				{
					if (GetRandomControl() & 0x1)
						item->goalAnimState = 8;
					else
						item->goalAnimState = 7;
				}
				break;
			}

			else if (item->aiBits & PATROL1)
				item->goalAnimState = 2;

			else if (creature->mood == ESCAPE_MOOD)
				item->goalAnimState = 3;
			else if (creature->mood == BORED_MOOD)
			{
				if (item->requiredAnimState)
					item->goalAnimState = item->requiredAnimState;
				else if (!(GetRandomControl() & 0xF))
					item->goalAnimState = 2;
				else if (!(GetRandomControl() & 0xF))
				{
					if (GetRandomControl() & 0x1)
						item->goalAnimState = 8;
					else
						item->goalAnimState = 7;
				}
			}
			else if ((item->aiBits & FOLLOW) && (creature->reachedGoal || laraInfo.distance > SQUARE(2048)))
			{
				if (item->requiredAnimState)
					item->goalAnimState = item->requiredAnimState;
				else if (info.ahead)
					item->goalAnimState = 6;
				else
					item->goalAnimState = 3;
			}
			else if (info.bite && info.distance < SQUARE(682))
				item->goalAnimState = 3;
			else if (info.bite && info.distance < SQUARE(682))
				item->goalAnimState = 2;
			else
				item->goalAnimState = 3;

			break;

		case 3:
			creature->flags = 0;
			creature->maximumTurn = 0;

			torsoY = laraInfo.angle;

			if (item->aiBits & GUARD)
			{
				torsoY = AIGuard(creature);
				if (!(GetRandomControl() & 15))
				{
					if (GetRandomControl() & 1)
						item->goalAnimState = 10;
					else
						item->goalAnimState = 6;
				}
				break;
			}
			else if (item->aiBits & PATROL1)
				item->goalAnimState = 2;
			else if (creature->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead)
					item->goalAnimState = 3;
				else
					item->goalAnimState = 4;
			}
			else if (creature->mood == BORED_MOOD)
			{
				if (item->requiredAnimState)
					item->goalAnimState = item->requiredAnimState;
				else if (!(GetRandomControl() & 15))
					item->goalAnimState = 2;
				else if (!(GetRandomControl() & 15))
				{
					if (GetRandomControl() & 1)
						item->goalAnimState = 10;
					else
						item->goalAnimState = 6;
				}
			}
			else if ((item->aiBits & FOLLOW) && (creature->reachedGoal || laraInfo.distance > SQUARE(2048)))
			{
				if (item->requiredAnimState)
					item->goalAnimState = item->requiredAnimState;
				else if (info.ahead)
					item->goalAnimState = 6;
				else
					item->goalAnimState = 4;
			}
			else if (info.bite && info.distance < SQUARE(341))
			{
				if (LaraItem->pos.yPos < item->pos.yPos)
					item->goalAnimState = 13;
				else
					item->goalAnimState = 12;
			}
			else if (info.bite && info.distance < SQUARE(682))
				item->goalAnimState = 14;
			else if (info.bite && info.distance < SQUARE(682))
				item->goalAnimState = 2;
			else if (info.distance < SQUARE(682) && creature->enemy != LaraItem && creature->enemy != NULL
				&& creature->enemy->objectNumber != ID_AI_PATROL1 && creature->enemy->objectNumber != ID_AI_PATROL2
				&& abs(item->pos.yPos - creature->enemy->pos.yPos) < 256)
				item->goalAnimState = 5;
			else if (info.bite && info.distance < SQUARE(1024))
				item->goalAnimState = 9;
			else
				item->goalAnimState = 4;

			break;

		case 5:
			creature->reachedGoal = true;	

			if (creature->enemy == NULL)
				break;
			else if ((creature->enemy->objectNumber == ID_SMALLMEDI_ITEM ||
				creature->enemy->objectNumber == ID_KEY_ITEM4) &&
				item->frameNumber == Anims[item->animNumber].frameBase + 12)
			{
				if (creature->enemy->roomNumber == NO_ROOM ||
					creature->enemy->status == ITEM_INVISIBLE ||
					creature->enemy->flags & -32768)
					creature->enemy = NULL;
				else
				{
					// Pickup the item!
					item->carriedItem = creature->enemy - Items;
					RemoveDrawnItem(creature->enemy - Items);
					creature->enemy->roomNumber = NO_ROOM;
					creature->enemy->carriedItem = NO_ITEM;

					// Stop other entities to interest to this item
					CREATURE_INFO * currentCreature = BaddieSlots;
					for (int i = 0; i < NUM_SLOTS; i++, currentCreature++)
					{
						if (currentCreature->itemNum == NO_ITEM || currentCreature->itemNum == itemNumber)
							continue;

						ITEM_INFO * target = &Items[currentCreature->itemNum];
						if (currentCreature->enemy == creature->enemy)
							currentCreature->enemy = NULL;
					}

					creature->enemy = NULL;

					if (item->aiBits != MODIFY)
					{
						item->aiBits |= AMBUSH;
						item->aiBits |= MODIFY;
					}
				}
			}
			else if (creature->enemy->objectNumber == ID_AI_AMBUSH && item->frameNumber == Anims[item->animNumber].frameBase + 12)
			{
				item->aiBits = 0;

				ITEM_INFO* carriedItem = &Items[item->carriedItem];

				carriedItem->pos.xPos = item->pos.xPos;
				carriedItem->pos.yPos = item->pos.yPos;
				carriedItem->pos.zPos = item->pos.zPos;

				ItemNewRoom(item->carriedItem, item->roomNumber);
				item->carriedItem = NO_ITEM;

				carriedItem->aiBits = GUARD;
				creature->enemy = NULL;
			}
			else
			{
				creature->maximumTurn = 0;
				if (abs(info.angle) < ANGLE(7))
					item->pos.yRot += info.angle;
				else if (info.angle < 0)
					item->pos.yRot -= ANGLE(7);
				else
					item->pos.yRot += ANGLE(7);
			}

			break;

		case 2:
			torsoY = laraInfo.angle;
			creature->maximumTurn = ANGLE(7);

			if (item->aiBits & PATROL1)
			{
				item->goalAnimState = 2;
				torsoY = 0;
			}
			else if (creature->mood == ESCAPE_MOOD)
				item->goalAnimState = 4;
			else if (creature->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 256)
				{
					item->goalAnimState = 6;
				}
			}
			else if (info.bite && info.distance < SQUARE(682))
				item->goalAnimState = 3;

			break;

		case 4:
			if (info.ahead)
				torsoY = info.angle;

			creature->maximumTurn = ANGLE(11);
			tilt = angle / 2;

			if (item->aiBits & GUARD)
				item->goalAnimState = 3;
			else if (creature->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead)
					item->goalAnimState = 3;
				break;
			}
			else if ((item->aiBits & FOLLOW) && (creature->reachedGoal || laraInfo.distance > SQUARE(2048)))
				item->goalAnimState = 3;	 
			else if (creature->mood == BORED_MOOD)
				item->goalAnimState = 9;
			else if (info.distance < SQUARE(682))
				item->goalAnimState = 3;
			else if (info.bite && info.distance < SQUARE(1024))
				item->goalAnimState = 9;

			break;

		case 12:
			if (info.ahead)
			{
				headY = info.angle;
				headX = info.xAngle;
			}

			creature->maximumTurn = 0;
			if (abs(info.angle) < ANGLE(7))
				item->pos.yRot += info.angle;
			else if (info.angle < 0)
				item->pos.yRot -= ANGLE(7);
			else
				item->pos.yRot += ANGLE(7);

			if (enemy == LaraItem)
			{
				if (!creature->flags && (item->touchBits & 0x2400))
				{
					LaraItem->hitPoints -= 40;
					LaraItem->hitStatus = true;
					CreatureEffect(item, &monkeyBite, DoBloodSplat);

					creature->flags = 1;
				}
			}
			else
			{
				if (!creature->flags && enemy)
				{
					if (abs(enemy->pos.xPos - item->pos.xPos) < 256 &&
						abs(enemy->pos.yPos - item->pos.yPos) <= 256 &&
						abs(enemy->pos.zPos - item->pos.zPos) < 256)
					{
						enemy->hitPoints -= 40 >> 1;
						enemy->hitStatus = true;
						creature->flags = 1;
						CreatureEffect(item, &monkeyBite, DoBloodSplat);
					}
				}
			}

			break;

		case 13:
			if (info.ahead)
			{
				headY = info.angle;
				headX = info.xAngle;
			}

			creature->maximumTurn = 0;
			if (abs(info.angle) < ANGLE(7))
				item->pos.yRot += info.angle;
			else if (info.angle < 0)
				item->pos.yRot -= ANGLE(7);
			else
				item->pos.yRot += ANGLE(7);

			if (enemy == LaraItem)
			{
				if (!creature->flags && (item->touchBits & 0x2400))
				{
					LaraItem->hitPoints -= 40;
					LaraItem->hitStatus = true;
					CreatureEffect(item, &monkeyBite, DoBloodSplat);

					creature->flags = 1;
				}
			}
			else
			{
				if (!creature->flags && enemy)
				{
					if (abs(enemy->pos.xPos - item->pos.xPos) < 256 &&
						abs(enemy->pos.yPos - item->pos.yPos) <= 256 &&
						abs(enemy->pos.zPos - item->pos.zPos) < 256)
					{
						enemy->hitPoints -= 40 >> 1;
						enemy->hitStatus = true;
						creature->flags = 1;
						CreatureEffect(item, &monkeyBite, DoBloodSplat);
					}
				}
			}

			break;

		case 14:
			if (info.ahead)
			{
				headY = info.angle;
				headX = info.xAngle;
			}

			creature->maximumTurn = 0;
			if (abs(info.angle) < ANGLE(7))
				item->pos.yRot += info.angle;
			else if (info.angle < 0)
				item->pos.yRot -= ANGLE(7);
			else
				item->pos.yRot += ANGLE(7);

			if (enemy == LaraItem)
			{
				if (creature->flags != 1 && (item->touchBits & 0x2400))
				{
					LaraItem->hitPoints -= 50;
					LaraItem->hitStatus = true;
					CreatureEffect(item, &monkeyBite, DoBloodSplat);

					creature->flags = 1;
				}
			}
			else
			{
				if (creature->flags != 1 && enemy)
				{
					if (abs(enemy->pos.xPos - item->pos.xPos) < 256 &&
						abs(enemy->pos.yPos - item->pos.yPos) <= 256 &&
						abs(enemy->pos.zPos - item->pos.zPos) < 256)
					{
						enemy->hitPoints -= 50 >> 1;
						enemy->hitStatus = true;
						creature->flags = 1;
						CreatureEffect(item, &monkeyBite, DoBloodSplat);
					}
				}
			}


			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, headY);
	CreatureJoint(item, 1, headX);
	CreatureJoint(item, 2, torsoY);

	if (item->currentAnimState < 15) 
	{
		switch (CreatureVault(itemNumber, angle, 2, 128))
		{
		case 2:
			creature->maximumTurn = 0;
			item->animNumber = Objects[ID_MONKEY].animIndex + 19;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 17;
			break;

		case 3:
			creature->maximumTurn = 0;
			item->animNumber = Objects[ID_MONKEY].animIndex + 18;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 16;
			break;

		case 4:
			creature->maximumTurn = 0;
			item->animNumber = Objects[ID_MONKEY].animIndex + 17;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 15;
			break;

		case -2:
			creature->maximumTurn = 0;
			item->animNumber = Objects[ID_MONKEY].animIndex + 22;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 20;
			break;

		case -3:
			creature->maximumTurn = 0;
			item->animNumber = Objects[ID_MONKEY].animIndex + 21;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 19;
			break;

		case -4:
			creature->maximumTurn = 0;
			item->animNumber = Objects[ID_MONKEY].animIndex + 20;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 18;
			break;
		}
	}
	else
	{
		creature->maximumTurn = 0;
		CreatureAnimation(itemNumber, angle, tilt);
	}
}

void InitialiseYeti(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	InitialiseCreature(itemNum);
	item->animNumber = Objects[item->objectNumber].animIndex + 19;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->currentAnimState = Anims[item->animNumber].currentAnimState;
}

void YetiControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	if (item->data == NULL) // safe in case data is empty.
		return;
	CREATURE_INFO* yeti = (CREATURE_INFO*)item->data;
	AI_INFO info;
	short angle = 0, torso = 0, head = 0, tilt = 0;
	bool lara_alive;

	lara_alive = (LaraItem->hitPoints > 0);

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 8)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 31;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 8;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, yeti->maximumTurn);

		switch (item->currentAnimState)
		{
			case 2:
				if (info.ahead)
					head = info.angle;

				yeti->flags = 0;
				yeti->maximumTurn = 0;

				if (yeti->mood == ESCAPE_MOOD)
				{
					item->goalAnimState = 1;
				}
				else if (item->requiredAnimState)
				{
					item->goalAnimState = item->requiredAnimState;
				}
				else if (yeti->mood == BORED_MOOD)
				{
					if (GetRandomControl() < 0x100 || !lara_alive)
						item->goalAnimState = 7;
					else if (GetRandomControl() < 0x200)
						item->goalAnimState = 9;
					else if (GetRandomControl() < 0x300)
						item->goalAnimState = 3;
				}
				else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 2) && GetRandomControl() < 0x4000)
				{
					item->goalAnimState = 4;
				}
				else if (info.ahead && info.distance < SQUARE(STEP_SIZE))
				{
					item->goalAnimState = 5;
				}
				else if (yeti->mood == STALK_MOOD)
				{
					item->goalAnimState = 3;
				}
				else
				{
					item->goalAnimState = 1;
				}
				break;
			case 7:
				if (info.ahead)
					head = info.angle;

				if (yeti->mood == ESCAPE_MOOD || item->hitStatus)
				{
					item->goalAnimState = 2;
				}
				else if (yeti->mood == BORED_MOOD)
				{
					if (lara_alive)
					{
						if (GetRandomControl() < 0x100)
						{
							item->goalAnimState = 2;
						}
						else if (GetRandomControl() < 0x200)
						{
							item->goalAnimState = 9;
						}
						else if (GetRandomControl() < 0x300)
						{
							item->goalAnimState = 2;
							item->requiredAnimState = 3;
						}
					}
				}
				else if (GetRandomControl() < 0x200)
				{
					item->goalAnimState = 2;
				}
				break;
			case 9:
				if (info.ahead)
					head = info.angle;

				if (yeti->mood == ESCAPE_MOOD || item->hitStatus)
				{
					item->goalAnimState = 2;
				}
				else if (yeti->mood == BORED_MOOD)
				{
					if (GetRandomControl() < 0x100 || !lara_alive)
					{
						item->goalAnimState = 7;
					}
					else if (GetRandomControl() < 0x200)
					{
						item->goalAnimState = 2;
					}
					else if (GetRandomControl() < 0x300)
					{
						item->goalAnimState = 2;
						item->requiredAnimState = 3;
					}
				}
				else if (GetRandomControl() < 0x200)
				{
					item->goalAnimState = 2;
				}
				break;
			case 3:
				if (info.ahead)
					head = info.angle;

				yeti->maximumTurn = ANGLE(4);

				if (yeti->mood == ESCAPE_MOOD)
				{
					item->goalAnimState = 1;
				}
				else if (yeti->mood == BORED_MOOD)
				{
					if (GetRandomControl() < 0x100 || !lara_alive)
					{
						item->goalAnimState = 2;
						item->requiredAnimState = 7;
					}
					else if (GetRandomControl() < 0x200)
					{
						item->goalAnimState = 2;
						item->requiredAnimState = 9;
					}
					else if (GetRandomControl() < 0x300)
					{
						item->goalAnimState = 2;
					}
				}
				else if (yeti->mood == ATTACK_MOOD)
				{
					if (info.ahead && info.distance < SQUARE(STEP_SIZE))
						item->goalAnimState = 2;
					else if (info.distance < SQUARE(WALL_SIZE * 2))
						item->goalAnimState = 1;
				}
				break;
			case 1:
				if (info.ahead)
					head = info.angle;

				yeti->flags = 0;
				yeti->maximumTurn = ANGLE(6);
				tilt = (angle / 4);

				if (yeti->mood == ESCAPE_MOOD)
				{
					break;
				}
				else if (yeti->mood == BORED_MOOD)
				{
					item->goalAnimState = 3;
				}
				else if (info.ahead && info.distance < SQUARE(STEP_SIZE))
				{
					item->goalAnimState = 2;
				}
				else if (info.ahead && info.distance < SQUARE(WALL_SIZE * 2))
				{
					item->goalAnimState = 6;
				}
				else if (yeti->mood == STALK_MOOD)
				{
					item->goalAnimState = 3;
				}
				break;
			case 4:
				if (info.ahead)
					torso = info.angle;

				if (!yeti->flags && (item->touchBits & 0x1400))
				{
					CreatureEffect(item, &yetiBiteR, DoBloodSplat);
					LaraItem->hitPoints -= 100;
					LaraItem->hitStatus = true;

					yeti->flags = 1;
				}
				break;
			case 5:
				if (info.ahead)
					torso = info.angle;
				yeti->maximumTurn = ANGLE(4);

				if (!yeti->flags && (item->touchBits & (0x0700 | 0x1400)))
				{
					if (item->touchBits & 0x0700)
						CreatureEffect(item, &yetiBiteL, DoBloodSplat);
					if (item->touchBits & 0x1400)
						CreatureEffect(item, &yetiBiteR, DoBloodSplat);

					LaraItem->hitPoints -= 150;
					LaraItem->hitStatus = true;

					yeti->flags = 1;
				}
				break;
			case 6:
				if (info.ahead)
					torso = info.angle;

				if (!yeti->flags && (item->touchBits & (0x0700 | 0x1400)))
				{
					if (item->touchBits & 0x0700)
						CreatureEffect(item, &yetiBiteL, DoBloodSplat);
					if (item->touchBits & 0x1400)
						CreatureEffect(item, &yetiBiteR, DoBloodSplat);

					LaraItem->hitPoints -= 200;
					LaraItem->hitStatus = true;

					yeti->flags = 1;
				}
				break;
			case 10:
			case 11:
			case 12:
			case 13:
				yeti->maximumTurn = 0; // stop yeti to rotate when climbing !
				break;
		}
	}

	// if lara dead, play special kill anim:
	if (!lara_alive)
	{
		yeti->maximumTurn = 0;
		CreatureKill(item, 31, 14, 103); // TODO: need to create a function for old extra anim use.
		return;
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso);
	CreatureJoint(item, 1, head);
	
	if (item->currentAnimState < 10)
	{
		switch (CreatureVault(itemNum, angle, 2, 300))
		{
			case 2:
				item->animNumber = Objects[item->objectNumber].animIndex + 34;
				item->frameNumber = Anims[item->animNumber].frameBase;
				item->currentAnimState = 10;
				break;
			case 3:
				item->animNumber = Objects[item->objectNumber].animIndex + 33;
				item->frameNumber = Anims[item->animNumber].frameBase;
				item->currentAnimState = 11;
				break;
			case 4:
				item->animNumber = Objects[item->objectNumber].animIndex + 32;
				item->frameNumber = Anims[item->animNumber].frameBase;
				item->currentAnimState = 12;
				break;
			case -4:
				item->animNumber = Objects[item->objectNumber].animIndex + 35;
				item->frameNumber = Anims[item->animNumber].frameBase;
				item->currentAnimState = 13;
				break;
		}
	}
	else
	{
		CreatureAnimation(itemNum, angle, tilt);
	}
}

// fix blood pos for small spider.
void S_SpiderBite(ITEM_INFO* item)
{
	PHD_VECTOR pos;
	pos.x = spiderBite.x;
	pos.y = spiderBite.y;
	pos.z = spiderBite.z;
	GetJointAbsPosition(item, &pos, spiderBite.meshNum);
	DoBloodSplat(pos.x, pos.y, pos.z, 10, item->pos.yPos, item->roomNumber);
}

void SpiderLeap(short itemNum, ITEM_INFO* item, short angle)
{
	GAME_VECTOR vec;

	vec.x = item->pos.xPos;
	vec.y = item->pos.yPos;
	vec.z = item->pos.zPos;
	vec.roomNumber = item->roomNumber;

	CreatureAnimation(itemNum, angle, 0);

	/* is this move less than 1.5 STEP_L height, if so dont do leap */
	if (item->pos.yPos > (vec.y - (STEP_SIZE * 3) / 2))
		return;

	/* else move spider back and do a jump instead ! */
	item->pos.xPos = vec.x;
	item->pos.yPos = vec.y;
	item->pos.zPos = vec.z;
	if (item->roomNumber != vec.roomNumber)
		ItemNewRoom(item->roomNumber, vec.roomNumber);

	item->animNumber = Objects[item->objectNumber].animIndex + 2;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->currentAnimState = 5;

	CreatureAnimation(itemNum, angle, 0);
}

void SmallSpiderControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* spider;
	AI_INFO info;
	short angle;

	item = &Items[itemNum];
	spider = (CREATURE_INFO*)item->data;
	angle = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 7)
		{
			ExplodingDeath(itemNum, -1, 256);
			DisableBaddieAI(itemNum);
			item->currentAnimState = 7;
			KillItem(itemNum);
		}
	}
	else
	{
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, ANGLE(8));

		switch (item->currentAnimState)
		{
		case 1:
			spider->flags = 0;

			if (spider->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x100)
					item->goalAnimState = 2;
				else
					break;
			}
			else if (info.ahead && item->touchBits)
			{
				item->goalAnimState = 4;
			}
			else if (spider->mood == STALK_MOOD)
			{
				item->goalAnimState = 2;
			}
			else if (spider->mood == ESCAPE_MOOD || spider->mood == ATTACK_MOOD)
			{
				item->goalAnimState = 3;
			}
			break;

		case 2:
			if (spider->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x100)
					item->goalAnimState = 1;
				else
					break;
			}
			else if (spider->mood == ESCAPE_MOOD || spider->mood == ATTACK_MOOD)
			{
				item->goalAnimState = 3;
			}
			break;

		case 3:
			spider->flags = 0;

			if (spider->mood == BORED_MOOD || spider->mood == STALK_MOOD)
				item->goalAnimState = 2;
			else if (info.ahead && item->touchBits)
				item->goalAnimState = 1;
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 5))
				item->goalAnimState = 6;
			else if (info.ahead && info.distance < SQUARE(WALL_SIZE / 2))
				item->goalAnimState = 5;
			break;

		case 4:
		case 5:
		case 6:
			if (!spider->flags && item->touchBits)
			{
				S_SpiderBite(item);
				LaraItem->hitPoints -= 25;
				LaraItem->hitStatus = 1;
				spider->flags = 1;
			}
			break;
		}
	}


	if (item->currentAnimState == 5 || item->currentAnimState == 4)
		CreatureAnimation(itemNum, angle, 0);
	else
		SpiderLeap(itemNum, item, angle);
}

void BigSpiderControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* spider;
	AI_INFO info;
	short angle;

	item = &Items[itemNum];
	spider = (CREATURE_INFO*)item->data;
	angle = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 7)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 2;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 7;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, TRUE);
		CreatureMood(item, &info, TRUE);
		angle = CreatureTurn(item, ANGLE(4));

		switch (item->currentAnimState)
		{
		case 1:
			spider->flags = 0;

			if (spider->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x200)
					item->goalAnimState = 2;
				else
					break;
			}
			else if (info.ahead && info.distance < (SQUARE(STEP_SIZE * 3) + 15)) // +15 fix attack distance !
			{
				item->goalAnimState = 4;
			}
			else if (spider->mood == STALK_MOOD)
			{
				item->goalAnimState = 2;
			}
			else if (spider->mood == ESCAPE_MOOD || spider->mood == ATTACK_MOOD)
			{
				item->goalAnimState = 3;
			}
			break;

		case 2:
			if (spider->mood == BORED_MOOD)
			{
				if (GetRandomControl() < 0x200)
					item->goalAnimState = 1;
				else
					break;
			}
			else if (spider->mood == ESCAPE_MOOD || spider->mood == ATTACK_MOOD)
			{
				item->goalAnimState = 3;
			}
			break;

		case 3:
			spider->flags = 0;

			if (spider->mood == BORED_MOOD || spider->mood == STALK_MOOD)
				item->goalAnimState = 2;
			else if (info.ahead && item->touchBits)
				item->goalAnimState = 1;
			break;

		case 4:
		case 5:
		case 6:
			if (!spider->flags && item->touchBits)
			{
				S_SpiderBite(item);
				LaraItem->hitPoints -= 100;
				LaraItem->hitStatus = 1;
				spider->flags = 1;
			}
			break;
		}
	}

	CreatureAnimation(itemNum, angle, 0);
}

void BirdMonsterControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* monster;
	AI_INFO info;
	short angle, head;

	item = &Items[itemNum];
	monster = (CREATURE_INFO*)item->data;
	angle = head = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 9)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 20;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 9;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, monster->maximumTurn);

		switch (item->currentAnimState)
		{
		case 1:
			monster->maximumTurn = 0;

			if (info.ahead && info.distance < SQUARE(WALL_SIZE))
			{
				if (GetRandomControl() < 0x4000)
					item->goalAnimState = 3;
				else
					item->goalAnimState = 10;
			}
			else if (info.ahead && (monster->mood == BORED_MOOD || monster->mood == STALK_MOOD))
			{
				if (info.zoneNumber != info.enemyZone)
				{
					item->goalAnimState = 2;
					monster->mood = ESCAPE_MOOD; // fix some problem with the GetCreatureMood()/CreatureMood().
				}
				else
					item->goalAnimState = 8;
			}
			else
			{
				item->goalAnimState = 2;
			}
			break;
		case 8:
			monster->maximumTurn = 0;

			if (monster->mood != BORED_MOOD || !info.ahead)
				item->goalAnimState = 1;
			break;
		case 2:
			monster->maximumTurn = ANGLE(4);

			if (info.ahead && info.distance < SQUARE(WALL_SIZE*2))
				item->goalAnimState = 5;
			else if ((monster->mood == BORED_MOOD || monster->mood == STALK_MOOD) && info.ahead)
				item->goalAnimState = 1;
			break;
		case 3:
			monster->flags = 0;

			if (info.ahead && info.distance < SQUARE(WALL_SIZE))
				item->goalAnimState = 4;
			else
				item->goalAnimState = 1;
			break;
		case 5:
			monster->flags = 0;

			if (info.ahead && info.distance < SQUARE(WALL_SIZE*2))
				item->goalAnimState = 6;
			else
				item->goalAnimState = 1;
			break;
		case 10:
			monster->flags = 0;

			if (info.ahead && info.distance < SQUARE(WALL_SIZE))
				item->goalAnimState = 11;
			else
				item->goalAnimState = 1;
			break;
		case 4:
		case 6:
		case 11:
		case 7:
			if (!(monster->flags & 1) && (item->touchBits & 0x600000))
			{
				CreatureEffect(item, &birdyBiteR, DoBloodSplat);
				LaraItem->hitPoints -= 200;
				LaraItem->hitStatus = true;
				monster->flags |= 1;
			}

			if (!(monster->flags & 2) && (item->touchBits & 0x0C0000))
			{
				CreatureEffect(item, &birdyBiteL, DoBloodSplat);
				LaraItem->hitPoints -= 200;
				LaraItem->hitStatus = true;
				monster->flags |= 2;
			}
			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, 0);
}