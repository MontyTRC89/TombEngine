#include "framework.h"
#include "tr5_lagoon_witch.h"
#include "items.h"
#include "box.h"
#include "effect2.h"
#include "effect2.h"
#include "tomb4fx.h"
#ifdef NEW_INV
#include "newinv2.h"
#else
#include "inventory.h"
#endif
#include "setup.h"
#include "level.h"
#include "lara.h"

#define STATE_LAGOON_WITCH_SWIM			1
#define STATE_LAGOON_WITCH_STOP			2
#define STATE_LAGOON_WITCH_ATTACK		3
#define STATE_LAGOON_WITCH_DEATH		5

#define ANIMATION_LAGOON_WITCH_DEATH	7

#ifndef NEW_INV
extern Inventory g_Inventory;
#endif

BITE_INFO LagoonWitchBite = { 0, 0, 0, 7 };

void InitialiseLagoonWitch(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	
	ClearItem(itemNumber);

	item->animNumber = Objects[item->objectNumber].animIndex + 1;
	item->goalAnimState = STATE_LAGOON_WITCH_STOP;
	item->currentAnimState = STATE_LAGOON_WITCH_STOP;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->pos.yPos += 512;
}

void LagoonWitchControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;
	
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short angle = 0;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != STATE_LAGOON_WITCH_DEATH)
		{
			item->hitPoints = 0;
			item->currentAnimState = STATE_LAGOON_WITCH_DEATH;
			item->animNumber = obj->animIndex + ANIMATION_LAGOON_WITCH_DEATH;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}
	}
	else
	{
#ifdef NEW_INV
		if (have_i_got_object(ID_PUZZLE_ITEM2))
#else
		if (g_Inventory.IsObjectPresentInInventory(ID_PUZZLE_ITEM2))
#endif
		{
			item->aiBits = 0;
			creature->enemy = LaraItem;
		}

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

		//if (creature->enemy != LaraItem)
		//	phd_atan(lara_item->pos.z_pos - item->pos.z_pos, lara_item->pos.x_pos - item->pos.x_pos);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (info.ahead)
		{
			joint0 = info.angle / 2;
			joint2 = info.angle / 2;
			joint1 = info.xAngle;
		}

		creature->maximumTurn = 0;

		switch (item->currentAnimState)
		{
		case STATE_LAGOON_WITCH_SWIM:
			creature->maximumTurn = 728;
			if (info.distance < SQUARE(1024))
				item->goalAnimState = STATE_LAGOON_WITCH_STOP;
			break;

		case STATE_LAGOON_WITCH_STOP:
			creature->flags = 0;
			creature->maximumTurn = ANGLE(2);
			if (info.distance < SQUARE(768))
				item->goalAnimState = STATE_LAGOON_WITCH_ATTACK;
			else if (info.distance > SQUARE(1024))
				item->goalAnimState = STATE_LAGOON_WITCH_SWIM;
			else
				item->goalAnimState = STATE_LAGOON_WITCH_STOP;
			break;

		case STATE_LAGOON_WITCH_ATTACK:
			creature->maximumTurn = ANGLE(2);
			if (!creature->flags
				&& item->touchBits & 0x3C3C0
				&& item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 29)
			{
				LaraItem->hitPoints -= 100;
				LaraItem->hitStatus = true;
				CreatureEffect2(item, &LagoonWitchBite, 10, item->pos.yRot, DoBloodSplat);
				creature->flags = STATE_LAGOON_WITCH_SWIM;
			}
			break;

		}

		if (creature->reachedGoal)
		{
			ITEM_INFO* enemy = creature->enemy;
			if (enemy)
			{
				if (enemy->flags & 2)
					item->itemFlags[3] = (item->TOSSPAD & 0xFF) - 1;
				item->itemFlags[3]++;
				creature->reachedGoal = false;
				creature->enemy = 0;
			}
		}
	}

	CreatureTilt(item, 0);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureAnimation(itemNumber, angle, 0);
	CreatureUnderwater(item, 341);
}