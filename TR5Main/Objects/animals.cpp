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
						creature->enemy = item;
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