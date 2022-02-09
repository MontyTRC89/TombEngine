#include "framework.h"
#include "Objects/TR1/Entity/tr1_wolf.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO wolfBite = { 0, -14, 174, 6 };

enum wolfStates {
	STATE_EMPTY, 
	STATE_STOP, 
	STATE_WALK, 
	STATE_RUN,
	STATE_JUMP, 
	STATE_STALK, 
	STATE_ATTACK,
	STATE_HOWL, 
	STATE_SLEEP, 
	STATE_CROUCH, 
	STATE_FASTTURN, 
	STATE_DEATH, 
	STATE_BITE
};

#define TOUCH (0x774f)

#define SLEEP_FRAME 96

#define DIE_ANIM  20

#define ATTACK_RANGE SQUARE(WALL_SIZE*3/2)
#define STALK_RANGE  SQUARE(WALL_SIZE*3)

#define BITE_DAMAGE   100
#define LUNGE_DAMAGE 50

#define WAKE_CHANCE  0x20
#define SLEEP_CHANCE 0x20
#define HOWL_CHANCE  0x180

#define WALK_TURN  ANGLE(2)
#define RUN_TURN   ANGLE(5)
#define STALK_TURN ANGLE(2)

void InitialiseWolf(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	ClearItem(itemNum);
	item->FrameNumber = SLEEP_FRAME;
}

void WolfControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	short head;
	short angle;
	short tilt;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
	head = angle = tilt = 0;
	AI_INFO info;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != STATE_DEATH)
		{
			item->AnimNumber = Objects[ID_WOLF].animIndex + DIE_ANIM + (short)(GetRandomControl() / 11000);
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = STATE_DEATH;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, creature->maximumTurn);

		switch (item->ActiveState)
		{
		case STATE_SLEEP:
			head = 0;

			if (creature->mood == ESCAPE_MOOD || info.zoneNumber == info.enemyZone)
			{
				item->RequiredState = STATE_CROUCH;
				item->TargetState = STATE_STOP;
			}
			else if (GetRandomControl() < WAKE_CHANCE)
			{
				item->RequiredState = STATE_WALK;
				item->TargetState = STATE_STOP;
			}
			break;

		case STATE_STOP:
			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else
				item->TargetState = STATE_WALK;
			break;

		case STATE_WALK:
			creature->maximumTurn = WALK_TURN;

			if (creature->mood != BORED_MOOD)
			{
				item->TargetState = STATE_STALK;
				item->RequiredState = STATE_EMPTY;
			}
			else if (GetRandomControl() < SLEEP_CHANCE)
			{
				item->RequiredState = STATE_SLEEP;
				item->TargetState = STATE_STOP;
			}
			break;

		case STATE_CROUCH:
			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (creature->mood == ESCAPE_MOOD)
				item->TargetState = STATE_RUN;
			else if (info.distance < SQUARE(345) && info.bite)
				item->TargetState = STATE_BITE;
			else if (creature->mood == STALK_MOOD)
				item->TargetState = STATE_STALK;
			else if (creature->mood == BORED_MOOD)
				item->TargetState = STATE_STOP;
			else
				item->TargetState = STATE_RUN;
			break;

		case STATE_STALK:
			creature->maximumTurn = STALK_TURN;

			if (creature->mood == ESCAPE_MOOD)
				item->TargetState = STATE_RUN;
			else if (info.distance < SQUARE(345) && info.bite)
				item->TargetState = STATE_BITE;
			else if (info.distance > SQUARE(3072))
				item->TargetState = STATE_RUN;
			else if (creature->mood == ATTACK_MOOD)
			{
				if (!info.ahead || info.distance > SQUARE(1536) ||
					(info.enemyFacing < FRONT_ARC && info.enemyFacing > -FRONT_ARC))
					item->TargetState = STATE_RUN;
			}
			else if (GetRandomControl() < HOWL_CHANCE)
			{
				item->RequiredState = STATE_HOWL;
				item->TargetState = STATE_CROUCH;
			}
			else if (creature->mood == BORED_MOOD)
				item->TargetState = STATE_CROUCH;
			break;

		case 3:
			creature->maximumTurn = RUN_TURN;
			tilt = angle;

			if (info.ahead && info.distance < ATTACK_RANGE)
			{
				if (info.distance > (ATTACK_RANGE / 2) &&
					(info.enemyFacing > FRONT_ARC || info.enemyFacing < -FRONT_ARC))
				{
					item->RequiredState = STATE_STALK;
					item->TargetState = STATE_CROUCH;
				}
				else
				{
					item->TargetState = STATE_ATTACK;
					item->RequiredState = STATE_EMPTY;
				}
			}
			else if (creature->mood == STALK_MOOD && info.distance < STALK_RANGE)
			{
				item->RequiredState = STATE_STALK;
				item->TargetState = STATE_CROUCH;
			}
			else if (creature->mood == BORED_MOOD)
				item->TargetState = STATE_CROUCH;
			break;

		case STATE_ATTACK:
			tilt = angle;
			if (!item->RequiredState && (item->TouchBits & TOUCH))
			{
				CreatureEffect(item, &wolfBite, DoBloodSplat);
				LaraItem->HitPoints -= LUNGE_DAMAGE;
				LaraItem->HitStatus = true;
				item->RequiredState = STATE_RUN;
			}
			item->TargetState = STATE_RUN;
			break;

		case 12:
			if (!item->RequiredState && (item->TouchBits & TOUCH) && info.ahead)
			{
				CreatureEffect(item, &wolfBite, DoBloodSplat);
				LaraItem->HitPoints -= BITE_DAMAGE;
				LaraItem->HitStatus = true;
				item->RequiredState = STATE_CROUCH;
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, tilt);
}
