#include "objects.h"
#include "..\Global\global.h"
#include "..\Game\Box.h"
#include "..\Game\items.h"
#include "..\Game\lot.h"
#include "..\Game\control.h"
#include "..\Game\effects.h"
#include "..\Game\draw.h"
#include "..\Game\sphere.h"

BITE_INFO wildboardBiteInfo = { 0, 0, 0, 14 };
BITE_INFO smallScorpionBiteInfo1 = { 0, 0, 0, 0 };
BITE_INFO smallScorpionBiteInfo2 = { 0, 0, 0, 23 };
BITE_INFO batBiteInfo = { 0, 16, 45, 4 };
BITE_INFO barracudaBite = { 2, -60, 121, 7 };
BITE_INFO sharkBite = { 17, -22, 344, 12 };
BITE_INFO tigerBite = { 19, -13, 3, 26 };
BITE_INFO cobraBite = { 0, 0, 0, 13 };
BITE_INFO raptorBite = { 0, 66, 318, 22 };

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
			
			if (nearestItem->objectNumber != ID_RAPTOR || (GetRandomControl() < 0x400 && minDistance < SQUARE(2048)))
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