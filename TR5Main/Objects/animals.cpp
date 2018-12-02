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

void __cdecl InitialiseWildBoar(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[ID_WILD_BOAR].animIndex + 6;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
}

void __cdecl WildBoarControl(__int16 itemNum)
{
	__int16 angle = 0;
	__int16 head = 0;
	__int16 neck = 0;
	__int16 tilt = 0;
	__int16 joint0 = 0;
	__int16 joint1 = 0;
	__int16 joint2 = 0;
	__int16 joint3 = 0;

	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	if (item->hitPoints > 0)
	{
		__int32 dx = LaraItem->pos.xPos - item->pos.xPos;
		__int32 dz = LaraItem->pos.zPos - item->pos.zPos;
		__int32 laraDistance = dx * dx + dz * dz;

		if (item->aiBits & GUARD)
		{
			GetAITarget(creature);
		}
		else
		{
			creature->enemy = LaraItem;
			
			CREATURE_INFO* baddie = &BaddieSlots[0];
			CREATURE_INFO* found = &BaddieSlots[0];
			__int32 minDistance = 0x7FFFFFFF;

			for (__int32 i = 0; i < NUM_SLOTS; i++, baddie++)
			{
				if (baddie->itemNum == NO_ITEM || baddie->itemNum == itemNum)
					continue;

				ITEM_INFO* target = &Items[baddie->itemNum];
				if (target->objectNumber != ID_WILD_BOAR)
				{
					__int32 dx2 = target->pos.xPos - item->pos.xPos;
					__int32 dz2 = target->pos.zPos - item->pos.zPos;
					__int32 distance = dx2 * dx2 + dz2 * dz2;

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
			creature->mood = MOOD_TYPE::ESCAPE_MOOD;
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
			if (!item->flags && (/*v23 < 50 && v25 < 50 || */info.distance < 0x10000 && info.bite))
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

void __cdecl InitialiseSmallScorpion(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[ID_SMALL_SCORPION].animIndex + 2;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
}

void __cdecl SmallScorpionControl(__int16 itemNum)
{
	__int16 angle = 0;
	__int16 head = 0;
	__int16 neck = 0;
	__int16 tilt = 0;
	__int16 joint0 = 0;
	__int16 joint1 = 0;
	__int16 joint2 = 0;
	__int16 joint3 = 0;

	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	if (item->hitPoints > 0)
	{
		__int32 dx = LaraItem->pos.xPos - item->pos.xPos;
		__int32 dz = LaraItem->pos.zPos - item->pos.zPos;
		__int32 laraDistance = dx * dx + dz * dz;

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
						__int16 rot;

						if (item->currentAnimState == 5)
						{
							rot = item->pos.yRot + -32768;
							biteInfo = &smallScorpionBiteInfo1;
						}
						else
						{
							rot = item->pos.yRot + -32768;
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

void __cdecl InitialiseScorpion(__int16 itemNum)
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

void __cdecl ScorpionControl(__int16 itemNum)
{
	__int16 angle = 0;
	__int16 head = 0;
	__int16 neck = 0;
	__int16 tilt = 0;
	__int16 joint0 = 0;
	__int16 joint1 = 0;
	__int16 joint2 = 0;
	__int16 joint3 = 0;

	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	__int16 roomNumber = item->roomNumber;

	__int32 x = item->pos.xPos + 682 * SIN(item->pos.yRot) >> W2V_SHIFT;
	__int32 z = item->pos.zPos + 682 * COS(item->pos.yRot) >> W2V_SHIFT;

	FLOOR_INFO* floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	__int32 height1 = GetFloorHeight(floor, x, item->pos.yPos, z);
	if (abs(item->pos.yPos - height1) > 512)
		height1 = item->pos.yPos;

	x = item->pos.xPos - 682 * SIN(item->pos.yRot) >> W2V_SHIFT;
	z = item->pos.zPos - 682 * COS(item->pos.yRot) >> W2V_SHIFT;

	floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	__int32 height2 = GetFloorHeight(floor, x, item->pos.yPos, z);
	if (abs(item->pos.yPos - height2) > 512)
		height2 = item->pos.yPos;

	__int16 angle1 = ATAN(1344, height2 - height1);

	x = item->pos.xPos - 682 * SIN(item->pos.yRot) >> W2V_SHIFT;
	z = item->pos.zPos + 682 * COS(item->pos.yRot) >> W2V_SHIFT;

	floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	__int32 height3 = GetFloorHeight(floor, x, item->pos.yPos, z);
	if (abs(item->pos.yPos - height3) > 512)
		height3 = item->pos.yPos;

	x = item->pos.xPos + 682 * SIN(item->pos.yRot) >> W2V_SHIFT;
	z = item->pos.zPos - 682 * COS(item->pos.yRot) >> W2V_SHIFT;

	floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	__int32 height4 = GetFloorHeight(floor, x, item->pos.yPos, z);
	if (abs(item->pos.yPos - height4) > 512)
		height4 = item->pos.yPos;

	__int16 angle2 = ATAN(1344, height4 - height3);

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
				__int32 minDistance = 0x7FFFFFFF;

				for (__int32 i = 0; i < NUM_SLOTS; i++)
				{
					baddy = &BaddieSlots[i];

					if (baddy->itemNum != NO_ITEM && baddy->itemNum != itemNum)
					{
						ITEM_INFO* currentItem = &Items[baddy->itemNum];

						if (currentItem->objectNumber != ID_SCORPION &&
							(currentItem != LaraItem || creature->hurtByLara))
						{
							__int32 dx = currentItem->pos.xPos - item->pos.xPos;
							__int32 dy = currentItem->pos.yPos - item->pos.yPos;
							__int32 dz = currentItem->pos.zPos - item->pos.zPos;
							__int32 distance = dx * dx + dy * dy + dz * dz;

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

void __cdecl InitialiseBat(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[ID_BAT].animIndex + 5;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 6;
	item->currentAnimState = 6;
}

void __cdecl BatControl(__int16 itemNum)
{
	__int16 angle = 0;
	__int16 head = 0;
	__int16 neck = 0;
	__int16 tilt = 0;
	__int16 joint0 = 0;
	__int16 joint1 = 0;
	__int16 joint2 = 0;
	__int16 joint3 = 0;

	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	if (item->hitPoints > 0)
	{
		__int32 dx = LaraItem->pos.xPos - item->pos.xPos;
		__int32 dz = LaraItem->pos.zPos - item->pos.zPos;
		__int32 laraDistance = dx * dx + dz * dz;

		if (item->aiBits & GUARD)
		{
			GetAITarget(creature);
		}
		else
		{
			creature->enemy = LaraItem;

			CREATURE_INFO* baddie = &BaddieSlots[0];
			CREATURE_INFO* found = &BaddieSlots[0];
			__int32 minDistance = 0x7FFFFFFF;

			for (__int32 i = 0; i < NUM_SLOTS; i++, baddie++)
			{
				if (baddie->itemNum == NO_ITEM || baddie->itemNum == itemNum)
					continue;

				ITEM_INFO* target = &Items[baddie->itemNum];
				if (target->objectNumber != ID_WILD_BOAR)
				{
					__int32 dx2 = target->pos.xPos - item->pos.xPos;
					__int32 dz2 = target->pos.zPos - item->pos.zPos;
					__int32 distance = dx2 * dx2 + dz2 * dz2;

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
			creature->mood = MOOD_TYPE::ESCAPE_MOOD;
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, 3640);

		switch (item->currentAnimState)
		{
		case 2:
			if (info.distance < 0x10000 || !(GetRandomControl() & 0x3F))
			{
				creature->flags = 0;
			}
			if (!creature->flags)
			{
				if (item->touchBits
					|| creature->enemy != LaraItem
					&& info.distance < 0x10000
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
					&& info.distance < 0x10000
					&& info.ahead/*
					&& (item->pos.yPos - v19->pos.yPos, (signed int)((HIDWORD(v20) ^ v20) - HIDWORD(v20)) < 896)*/))
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
				creature->mood = MOOD_TYPE::BORED_MOOD;
			}
			break;

		case 6:
			if (info.distance < 26214400 || item->hitStatus || creature->flags & 0x10)
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

void __cdecl BarracudaControl(__int16 itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO *)item->data;
	__int16 angle = 0;
	__int16 head = 0;

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

	CreatureJoint(item, head, 0);

	CreatureAnimation(itemNum, angle, 0);
	CreatureUnderwater(item, STEP_SIZE);
}

void __cdecl SharkControl(__int16 itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO *)item->data;
	__int16 angle = 0;
	__int16 head = 0;

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

void __cdecl TigerControl(__int16 itemNum)
{
	__int16 head = 0;
	__int16 angle = 0;
	__int16 tilt = 0;

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
			creature->mood = MOOD_TYPE::ESCAPE_MOOD;

		CreatureMood(item, &info, 1);

		angle = CreatureTurn(item, creature->maximumTurn);

		switch (item->currentAnimState)
		{
		case 1:
			creature->maximumTurn = 0;
			creature->flags = 0;

			if (creature->mood == MOOD_TYPE::ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 3;
			}
			else if (creature->mood == MOOD_TYPE::BORED_MOOD)
			{
				__int16 random = GetRandomControl();
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

			if (creature->mood == MOOD_TYPE::ESCAPE_MOOD || creature->mood == MOOD_TYPE::ATTACK_MOOD)
				item->goalAnimState = 3;
			else if (GetRandomControl() < 0x60)
			{
				item->goalAnimState = 1;
				item->requiredAnimState = 5;
			}
			break;

		case 3:
			creature->maximumTurn = ANGLE(6);

			if (creature->mood == MOOD_TYPE::BORED_MOOD)
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
			else if (creature->mood != MOOD_TYPE::ATTACK_MOOD && GetRandomControl() < 0x60)
			{
				item->requiredAnimState = 5;
				item->goalAnimState = 1;
			}
			else if (creature->mood == MOOD_TYPE::ESCAPE_MOOD && Lara.target != item && info.ahead)
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

void __cdecl InitialiseCobra(__int16 itemNum)
{
	InitialiseCreature(itemNum);

	ITEM_INFO* item = &Items[itemNum];
	item->animNumber = Objects[item->objectNumber].animIndex + 2;
	item->frameNumber = Anims[item->animNumber].frameBase + 45;
	item->currentAnimState = item->goalAnimState = 3;
	item->itemFlags[2] = item->hitStatus;
	item->hitPoints = Objects[item->objectNumber].hitPoints;
}

void __cdecl CobraControl(__int16 itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	__int16 head = 0;
	__int16 angle = 0;
	__int16 tilt = 0;

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
	CreatureAnimation(itemNum, angle, 0);
}

void __cdecl RaptorControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	if (item->status == ITEM_INVISIBLE)
	{
		if (!EnableBaddieAI(itemNum, 0))
			return;
		item->status = ITEM_ACTIVE;
	}
	CREATURE_INFO* creature = (CREATURE_INFO *)item->data;
	
	__int16 head = 0;
	__int16 neck = 0;
	__int16 angle = 0;
	__int16 tilt = 0;

	ITEM_INFO* nearestItem = NULL;
	__int32 minDistance = 0x7FFFFFFF;
	
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
			for (__int32 i = 0; i < NUM_SLOTS; i++)
			{
				if (currentCreature->itemNum == NO_ITEM || currentCreature->itemNum == itemNum)
				{
					currentCreature++;
					continue;
				}

				target = &Items[currentCreature->itemNum];

				__int32 x = (target->pos.xPos - item->pos.xPos) >> 6;
				__int32 y = (target->pos.yPos - item->pos.yPos) >> 6;
				__int32 z = (target->pos.zPos - item->pos.zPos) >> 6;
				__int32 distance = x * x + y * y + z * z;
				if (distance < minDistance && item->hitPoints > 0)
				{
					nearestItem = target;
					minDistance = distance;
				}

				currentCreature++;
			}
			
			if (nearestItem != NULL && (nearestItem->objectNumber != ID_RAPTOR || (GetRandomControl() < 0x400 && minDistance < SQUARE(2048))))
				creature->enemy = nearestItem;
			
			__int32 x = (LaraItem->pos.xPos - item->pos.xPos) >> 6;
			__int32 y = (LaraItem->pos.yPos - item->pos.yPos) >> 6;
			__int32 z = (LaraItem->pos.zPos - item->pos.zPos) >> 6;
			__int32 distance = x * x + y * y + z * z;
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

		if (creature->mood == MOOD_TYPE::BORED_MOOD)
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
			else if (creature->mood == MOOD_TYPE::ESCAPE_MOOD && Lara.target != item && info.ahead && !item->hitStatus)
				item->goalAnimState = 1;
			else if (creature->mood == MOOD_TYPE::BORED_MOOD)
				item->goalAnimState = 2;
			else
				item->goalAnimState = 3;
			break;

		case 2:
			creature->maximumTurn = ANGLE(2);
			creature->flags &= ~1;

			if (creature->mood != MOOD_TYPE::BORED_MOOD)
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
			else if (info.ahead && creature->mood != MOOD_TYPE::ESCAPE_MOOD && GetRandomControl() < 0x80)
			{
				item->requiredAnimState = 6;
				item->goalAnimState = 1;
			}
			else if (creature->mood == MOOD_TYPE::BORED_MOOD || (creature->mood == MOOD_TYPE::ESCAPE_MOOD && Lara.target != item && info.ahead))
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

void InitialiseEagle(__int16 itemNum)
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

void EagleControl(__int16 itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO *)item->data;

	__int16 angle = 0;

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
			if (creature->mood != MOOD_TYPE::BORED_MOOD)
				item->goalAnimState = 1;
			break;

		case 2:
			item->pos.yPos = item->floor;
			if (creature->mood == MOOD_TYPE::BORED_MOOD)
				break;
			else
				item->goalAnimState = 1;
			break;

		case 1:
			creature->flags = 0;

			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			if (creature->mood == MOOD_TYPE::BORED_MOOD)
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

void __cdecl BearControl(__int16 itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	__int16 head = 0;
	__int16 angle;

	if (item->hitPoints <= 0)
	{
		angle = CreatureTurn(item, ANGLE(1));

		switch (item->currentAnimState)
		{
		case 2:
		{
			item->goalAnimState = 4;
			break;
		}

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

			break;
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
			else if (creature->mood == MOOD_TYPE::BORED_MOOD)
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
			else if (creature->mood != MOOD_TYPE::BORED_MOOD)
			{
				item->goalAnimState = 1;
				if (creature->mood == MOOD_TYPE::ESCAPE_MOOD)
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

			if (creature->mood == MOOD_TYPE::BORED_MOOD || LaraItem->hitPoints <= 0)
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
			else if (creature->mood == MOOD_TYPE::BORED_MOOD || creature->mood == MOOD_TYPE::ESCAPE_MOOD)
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
			else if (creature->mood == MOOD_TYPE::ESCAPE_MOOD)
			{
				item->goalAnimState = 4;
				item->requiredAnimState = 0;
			}
			else if (creature->mood == MOOD_TYPE::BORED_MOOD || GetRandomControl() < 0x50)
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

		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNum, angle, 0);
	}
}


void __cdecl InitialiseWolf(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->frameNumber = 96;
}


void __cdecl WolfControl(__int16 itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	
	__int16 head = 0;
	__int16 angle = 0;
	__int16 tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 11)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 20 + (__int16)(GetRandomControl() / 11000);
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

			if (creature->mood == MOOD_TYPE::ESCAPE_MOOD || info.zoneNumber == info.enemyZone)
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

			if (creature->mood != MOOD_TYPE::BORED_MOOD)
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
			else if (creature->mood == MOOD_TYPE::ESCAPE_MOOD)
				item->goalAnimState = 3;
			else if (info.distance < SQUARE(345) && info.bite)
				item->goalAnimState = 12;
			else if (creature->mood == MOOD_TYPE::STALK_MOOD)
				item->goalAnimState = 5;
			else if (creature->mood == MOOD_TYPE::BORED_MOOD)
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
			else if (creature->mood == MOOD_TYPE::ATTACK_MOOD)
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
			else if (creature->mood == MOOD_TYPE::BORED_MOOD)
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
			else if (creature->mood == MOOD_TYPE::STALK_MOOD && info.distance < SQUARE(3072))
			{
				item->requiredAnimState = 5;
				item->goalAnimState = 9;
			}
			else if (creature->mood == MOOD_TYPE::BORED_MOOD)
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

void TyrannosaurControl(__int16 itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*) item->data;
	
	__int16 head = 0;
	__int16 angle = 0;

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

		creature->flags = (creature->mood != MOOD_TYPE::ESCAPE_MOOD && !info.ahead &&
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
			else if (creature->mood == MOOD_TYPE::BORED_MOOD || creature->flags)
				item->goalAnimState = 2;
			else
				item->goalAnimState = 3;
			break;

		case 2:
			creature->maximumTurn = ANGLE(2);

			if (creature->mood != MOOD_TYPE::BORED_MOOD || !creature->flags)
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
			else if (creature->mood == MOOD_TYPE::BORED_MOOD)
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

	CreatureJoint(item, 0, (__int16)(head * 2));
	creature->jointRotation[1] = creature->jointRotation[0];

	CreatureAnimation(itemNum, angle, 0);

	item->collidable = true;
}

void __cdecl ApeControl(__int16 itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*) item->data;
	
	__int16 head = 0;
	__int16 angle = 0;
	__int16 random = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 5)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 7 + (__int16)(GetRandomControl() / 0x4000);
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
				random = (__int16)(GetRandomControl() >> 5);
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
			else if (creature->mood != MOOD_TYPE::ESCAPE_MOOD)
			{
				random = (__int16)GetRandomControl();
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

		__int32 vault = CreatureVault(itemNum, angle, 2, 75);

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

void __cdecl RatControl(__int16 itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*) item->data;
	
	__int16 head = 0;
	__int16 angle = 0;
	__int16 random = 0;

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
			if (creature->mood == MOOD_TYPE::BORED_MOOD || creature->mood == MOOD_TYPE::STALK_MOOD)
			{
				__int16 random = (__int16)GetRandomControl();
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

			if (creature->mood == MOOD_TYPE::BORED_MOOD || creature->mood == MOOD_TYPE::STALK_MOOD)
			{
				random = (__int16)GetRandomControl();
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

void __cdecl InitialiseLittleBeetle(__int16 itemNum)
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

void __cdecl LittleBeetleControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
}

void __cdecl InitialiseHarpy(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[ID_HARPY].animIndex + 4;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
}

void __cdecl HarpyControl(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	if (!CreatureActive(itemNum))
		return;

	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	__int16 angle = 0;
	__int16 joint0 = 0;
	__int16 joint1 = 0;
	__int16 joint2 = 0;

	if (item->hitPoints <= 0)
	{
		__int16 state = item->currentAnimState - 9;
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
		__int32 minDistance = 0x7FFFFFFF;

		creature->enemy = NULL;

		for (__int32 i = 0; i < NUM_SLOTS; i++, baddie++)
		{
			if (baddie->itemNum == NO_ITEM || baddie->itemNum == itemNum)
				continue;

			ITEM_INFO* target = &Items[baddie->itemNum];

			if (target->objectNumber == ID_LARA_DOUBLE)
			{
				__int32 dx = target->pos.xPos - item->pos.xPos;
				__int32 dz = target->pos.zPos - item->pos.zPos;
				__int32 distance = dx * dx + dz * dz;

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

		__int32 height = 0;
		__int32 dy = 0;

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
			creature->maximumTurn = 1274;
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
			if (!creature->enemy || creature->enemy->pos.yPos < item->pos.yPos + 2048 
				|| item->floor < item->pos.yPos + 2048)
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

void __cdecl HarpySparks2(__int32 x, __int32 y, __int32 z, __int32 xv, __int32 yv, __int32 zv)
{
	__int32 dx = LaraItem->pos.xPos - x;
	__int32 dz = LaraItem->pos.zPos - z;

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

void __cdecl HarpyAttack(ITEM_INFO* item, __int16 itemNum)
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
		for (__int32 i = 0; i < 2; i++)
		{
			__int32 dx = (GetRandomControl() & 0x7FF) + pos1.x - 1024;
			__int32 dy = (GetRandomControl() & 0x7FF) + pos1.y - 1024;
			__int32 dz = (GetRandomControl() & 0x7FF) + pos1.z - 1024;

			HarpySparks2(dx, dy, dz, 8 * (pos1.x - dx), 8 * (pos1.y - dy), 8 * (pos1.z - dz));
		
			dx = (GetRandomControl() & 0x7FF) + pos2.x - 1024;
			dy = (GetRandomControl() & 0x7FF) + pos2.y - 1024;
			dz = (GetRandomControl() & 0x7FF) + pos2.z - 1024;

			HarpySparks2(dx, dy, dz, 8 * (pos2.x - dx), 8 * (pos2.y - dy), 8 * (pos2.z - dz));
		}
	}

	__int32 something = 2 * item->itemFlags[0];
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
			
			__int16 angles[2];
			phd_GetVectorAngles(pos3.x - pos1.x, 
								pos3.y - pos1.y,
								pos3.z - pos1.z,
								angles);

			pos.xRot = angles[0];
			pos.yRot = angles[1];
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

			__int16 angles[2];
			phd_GetVectorAngles(pos3.x - pos1.x,
				pos3.y - pos1.y,
				pos3.z - pos1.z,
				angles);

			pos.xRot = angles[0];
			pos.yRot = angles[1];
			pos.zRot = 0;

			HarpyBubbles(&pos, item->roomNumber, 2);
		}
	}
}

void __cdecl HarpyBubbles(PHD_3DPOS* pos, __int16 roomNumber, __int32 count)
{
	__int16 fxNumber = CreateNewEffect(roomNumber);
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
		fx->objectNumber = ID_BUBBLES;
		fx->speed = (GetRandomControl() & 0x1F) + 96;
		fx->flag1 = count;
		fx->frameNumber = Objects[ID_BUBBLES].meshIndex + 2 * count;
	}
}

void __cdecl HarpySparks1(__int16 itemNum, byte num, __int32 size)
{
	ITEM_INFO* item = &Items[itemNum];

	__int32 dx = LaraItem->pos.xPos - item->pos.xPos;
	__int32 dz = LaraItem->pos.zPos - item->pos.zPos;

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

void __cdecl InitialiseCrocodile(__int16 itemNum)
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

void __cdecl CrocodileControl(__int16 itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	__int32 x = item->pos.xPos + SIN(item->pos.yRot) << 10 >> W2V_SHIFT;
	__int32 y = item->pos.yPos;
	__int32 z = item->pos.zPos + COS(item->pos.yRot) << 10 >> W2V_SHIFT;

	__int16 roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	__int32 height1 = GetFloorHeight(floor, x, y, z);

	if (abs(y - height1) > 512)
		height1 = y;

	x = item->pos.xPos - SIN(item->pos.yRot) << 10 >> W2V_SHIFT;
	y = item->pos.yPos;
	z = item->pos.zPos - COS(item->pos.yRot) << 10 >> W2V_SHIFT;

	roomNumber = item->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	__int32 height2 = GetFloorHeight(floor, x, y, z);

	if (abs(y - height2) > 512)
		height2 = y;

	__int16 at = ATAN(2048, height2 - height1);
	__int16 angle = 0;
	__int16 joint0 = 0;
	__int16 joint2 = 0;

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

	__int16 xRot = item->pos.xRot;

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

void __cdecl InitialiseSphinx(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	ClearItem(itemNum);

	item->animNumber = Objects[item->animNumber].animIndex + 1;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
}

void __cdecl SphinxControl(__int16 itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	__int32 x = item->pos.yPos + 614 * SIN(item->pos.yRot) >> W2V_SHIFT;
	__int32 y = item->pos.yPos;
	__int32 z = item->pos.yPos + 614 * COS(item->pos.yRot) >> W2V_SHIFT;

	__int16 roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	__int32 height1 = GetFloorHeight(floor, x, y, z);

	if (item->currentAnimState == 5 && floor->stopper)
	{
		ROOM_INFO* room = &Rooms[item->roomNumber];

		for (__int32 i = 0; i < room->numMeshes; i++)
		{
			MESH_INFO* mesh = &room->mesh[i];

			if (mesh->z >> 10 == z >> 10 && mesh->x >> 10 == x >> 10 && mesh->staticNumber >= 50)
			{
				ShatterObject(NULL, mesh, -64, item->roomNumber, 0);
				SoundEffect(347, &item->pos, 0);

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
	__int32 height2= GetFloorHeight(floor, x, y, z);

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

	__int16 angle = CreatureTurn(item, creature->maximumTurn);

	__int32 dx = abs(item->itemFlags[2] - item->pos.xPos);
	__int32 dz = abs(item->itemFlags[3] - item->pos.zPos);

	switch (item->currentAnimState)
	{
	case 1:
		creature->maximumTurn = 0;

		if (info.distance < SQUARE(1024) || item->triggerFlags)
		{
			item->goalAnimState = 3;
		}
		else if (GetRandomControl() == 0)
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
		else if (GetRandomControl() == 0)
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

		if (!creature->flags)
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
		if (dx >= 50 || dz >= 50 || item->animNumber != Objects[item->objectNumber].animIndex)
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
		//v36 = (signed __int16)item->currentAnimState - 1;
		
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