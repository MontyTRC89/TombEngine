#include "framework.h"
#include "tr5_lagoon_witch.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/gui.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"

#define STATE_LAGOON_WITCH_SWIM			1
#define STATE_LAGOON_WITCH_STOP			2
#define STATE_LAGOON_WITCH_ATTACK		3
#define STATE_LAGOON_WITCH_DEATH		5

#define ANIMATION_LAGOON_WITCH_DEATH	7

BITE_INFO LagoonWitchBite = { 0, 0, 0, 7 };

void InitialiseLagoonWitch(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	
	ClearItem(itemNumber);

	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
	item->TargetState = STATE_LAGOON_WITCH_STOP;
	item->ActiveState = STATE_LAGOON_WITCH_STOP;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->Position.yPos += 512;
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
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
	OBJECT_INFO* obj = &Objects[item->ObjectNumber];

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != STATE_LAGOON_WITCH_DEATH)
		{
			item->HitPoints = 0;
			item->ActiveState = STATE_LAGOON_WITCH_DEATH;
			item->AnimNumber = obj->animIndex + ANIMATION_LAGOON_WITCH_DEATH;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		}
	}
	else
	{
		if (g_Gui.IsObjectInInventory(ID_PUZZLE_ITEM2))
		{
			item->AIBits = 0;
			creature->enemy = LaraItem;
		}

		if (item->AIBits)
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

		switch (item->ActiveState)
		{
		case STATE_LAGOON_WITCH_SWIM:
			creature->maximumTurn = 728;
			if (info.distance < SQUARE(1024))
				item->TargetState = STATE_LAGOON_WITCH_STOP;
			break;

		case STATE_LAGOON_WITCH_STOP:
			creature->flags = 0;
			creature->maximumTurn = ANGLE(2);
			if (info.distance < SQUARE(768))
				item->TargetState = STATE_LAGOON_WITCH_ATTACK;
			else if (info.distance > SQUARE(1024))
				item->TargetState = STATE_LAGOON_WITCH_SWIM;
			else
				item->TargetState = STATE_LAGOON_WITCH_STOP;
			break;

		case STATE_LAGOON_WITCH_ATTACK:
			creature->maximumTurn = ANGLE(2);
			if (!creature->flags
				&& item->TouchBits & 0x3C3C0
				&& item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 29)
			{
				LaraItem->HitPoints -= 100;
				LaraItem->HitStatus = true;
				CreatureEffect2(item, &LagoonWitchBite, 10, item->Position.yRot, DoBloodSplat);
				creature->flags = STATE_LAGOON_WITCH_SWIM;
			}
			break;

		}

		if (creature->reachedGoal)
		{
			ITEM_INFO* enemy = creature->enemy;
			if (enemy)
			{
				if (enemy->Flags & 2)
					item->ItemFlags[3] = (item->Tosspad & 0xFF) - 1;
				item->ItemFlags[3]++;
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