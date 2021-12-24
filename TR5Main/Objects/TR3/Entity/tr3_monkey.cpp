#include "framework.h"
#include "Objects/TR3/Entity/tr3_monkey.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO monkeyBite = { 10, 10, 11, 13 };

void InitialiseMonkey(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->animNumber = Objects[ID_MONKEY].animIndex + 2;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->currentAnimState = 6;
	item->goalAnimState = 6;
}

void MonkeyControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

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
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
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
			

			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				CREATURE_INFO* currentCreature = ActiveCreatures[i];
				if (currentCreature->itemNum == NO_ITEM || currentCreature->itemNum == itemNumber)
					continue;

				ITEM_INFO* target = &g_Level.Items[currentCreature->itemNum];
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
				item->meshBits = 0xFFFFFEFF;
			else
				item->meshBits = -1;
		}
		else
		{
			if (item->carriedItem != NO_ITEM)
				item->meshBits = 0xFFFF6E6F;
			else
				item->meshBits = 0xFFFF6F6F;
		}

		AI_INFO info;
		AI_INFO laraInfo;

		CreatureAIInfo(item, &info);


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

			laraInfo.angle = phd_atan(dz, dz) - item->pos.yRot;
			laraInfo.distance = SQUARE(dx) + SQUARE(dz);
		}

		GetCreatureMood(item, &info, VIOLENT);
		if (Lara.Vehicle != NO_ITEM)
			creature->mood = ESCAPE_MOOD;
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		ITEM_INFO* enemy = creature->enemy;
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
				item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 12)
			{
				if (creature->enemy->roomNumber == NO_ROOM ||
					creature->enemy->status == ITEM_INVISIBLE ||
					creature->enemy->flags & -32768)
					creature->enemy = NULL;
				else
				{
					item->carriedItem = creature->enemy - g_Level.Items.data();
					RemoveDrawnItem(creature->enemy - g_Level.Items.data());
					creature->enemy->roomNumber = NO_ROOM;
					creature->enemy->carriedItem = NO_ITEM;


					for (int i = 0; i < ActiveCreatures.size(); i++)
					{
						CREATURE_INFO* currentCreature = ActiveCreatures[i];
						if (currentCreature->itemNum == NO_ITEM || currentCreature->itemNum == itemNumber)
							continue;

						ITEM_INFO* target = &g_Level.Items[currentCreature->itemNum];
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
			else if (creature->enemy->objectNumber == ID_AI_AMBUSH && item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 12)
			{
				item->aiBits = 0;

				ITEM_INFO* carriedItem = &g_Level.Items[item->carriedItem];

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
						enemy->hitPoints -= 20;
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
						enemy->hitPoints -= 20;
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
						enemy->hitPoints -= 25;
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
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 17;
			break;

		case 3:
			creature->maximumTurn = 0;
			item->animNumber = Objects[ID_MONKEY].animIndex + 18;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 16;
			break;

		case 4:
			creature->maximumTurn = 0;
			item->animNumber = Objects[ID_MONKEY].animIndex + 17;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 15;
			break;

		case -2:
			creature->maximumTurn = 0;
			item->animNumber = Objects[ID_MONKEY].animIndex + 22;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 20;
			break;

		case -3:
			creature->maximumTurn = 0;
			item->animNumber = Objects[ID_MONKEY].animIndex + 21;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 19;
			break;

		case -4:
			creature->maximumTurn = 0;
			item->animNumber = Objects[ID_MONKEY].animIndex + 20;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
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
