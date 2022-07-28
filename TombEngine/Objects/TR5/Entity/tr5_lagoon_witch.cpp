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
#include "Game/misc.h"

BITE_INFO LagoonWitchBite = { 0, 0, 0, 7 };

enum LagoonWitchState
{
	WITCH_STATE_SWIM = 1,
	WITCH_STATE_IDLE = 2,
	WITCH_STATE_ATTACK = 3,
	WITCH_STATE_DEATH = 5
};

// TODO
enum LagoonWitchAnim
{
	WITCH_ANIM_DEATH = 7
};

void InitialiseLagoonWitch(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	
	ClearItem(itemNumber);

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
	item->Animation.TargetState = WITCH_STATE_IDLE;
	item->Animation.ActiveState = WITCH_STATE_IDLE;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Pose.Position.y += CLICK(2);
}

void LagoonWitchControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;
	
	short angle = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);
	auto* object = &Objects[item->ObjectNumber];

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != WITCH_STATE_DEATH)
		{
			item->HitPoints = 0;
			item->Animation.ActiveState = WITCH_STATE_DEATH;
			item->Animation.AnimNumber = object->animIndex + WITCH_ANIM_DEATH;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		}
	}
	else
	{
		if (g_Gui.IsObjectInInventory(ID_PUZZLE_ITEM2))
		{
			item->AIBits = 0;
			creature->Enemy = LaraItem;
		}

		if (item->AIBits)
			GetAITarget(creature);
		else if (creature->HurtByLara)
			creature->Enemy = LaraItem;

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		//if (creature->enemy != LaraItem)
		//	phd_atan(lara_item->pos.z_pos - item->pos.z_pos, lara_item->pos.x_pos - item->pos.x_pos);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);

		if (AI.ahead)
		{
			joint0 = AI.angle / 2;
			joint1 = AI.xAngle;
			joint2 = AI.angle / 2;
		}

		creature->MaxTurn = 0;

		switch (item->Animation.ActiveState)
		{
		case WITCH_STATE_SWIM:
			creature->MaxTurn = ANGLE(4.0f);

			if (AI.distance < pow(SECTOR(1), 2))
				item->Animation.TargetState = WITCH_STATE_IDLE;

			break;

		case WITCH_STATE_IDLE:
			creature->MaxTurn = ANGLE(2.0f);
			creature->Flags = 0;

			if (AI.distance < pow(CLICK(3), 2))
				item->Animation.TargetState = WITCH_STATE_ATTACK;
			else if (AI.distance > pow(SECTOR(1), 2))
				item->Animation.TargetState = WITCH_STATE_SWIM;
			else
				item->Animation.TargetState = WITCH_STATE_IDLE;

			break;

		case WITCH_STATE_ATTACK:
			creature->MaxTurn = ANGLE(2.0f);

			if (!creature->Flags &&
				item->TouchBits & 0x3C3C0 &&
				item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 29)
			{
				DoDamage(creature->Enemy, 100);
				CreatureEffect2(item, &LagoonWitchBite, 10, item->Pose.Orientation.y, DoBloodSplat);
				creature->Flags = WITCH_STATE_SWIM;
			}

			break;
		}

		if (creature->ReachedGoal)
		{
			auto* enemy = creature->Enemy;

			if (enemy)
			{
				if (enemy->Flags & 2)
					item->ItemFlags[3] = (creature->Tosspad & 0xFF) - 1;

				item->ItemFlags[3]++;
				creature->ReachedGoal = false;
				creature->Enemy = 0;
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
