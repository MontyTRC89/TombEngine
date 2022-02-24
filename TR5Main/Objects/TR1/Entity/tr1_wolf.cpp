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

enum WolfState
{
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

#define ATTACK_RANGE pow(WALL_SIZE*3/2, 2)
#define STALK_RANGE  pow(WALL_SIZE*3, 2)

#define BITE_DAMAGE   100
#define LUNGE_DAMAGE 50

#define WAKE_CHANCE  0x20
#define SLEEP_CHANCE 0x20
#define HOWL_CHANCE  0x180

#define WALK_TURN  ANGLE(2.0f)
#define RUN_TURN   ANGLE(5.0f)
#define STALK_TURN ANGLE(2.0f)

void InitialiseWolf(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	ClearItem(itemNumber);
	item->FrameNumber = SLEEP_FRAME;
}

void WolfControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	short head;
	short angle;
	short tilt;

	auto* item = &g_Level.Items[itemNumber];
	auto* creatureInfo = (CREATURE_INFO*)item->Data;

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

		angle = CreatureTurn(item, creatureInfo->maximumTurn);

		switch (item->ActiveState)
		{
		case STATE_SLEEP:
			head = 0;

			if (creatureInfo->mood == ESCAPE_MOOD || info.zoneNumber == info.enemyZone)
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
			creatureInfo->maximumTurn = WALK_TURN;

			if (creatureInfo->mood != BORED_MOOD)
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
			else if (creatureInfo->mood == ESCAPE_MOOD)
				item->TargetState = STATE_RUN;
			else if (info.distance < pow(345, 2) && info.bite)
				item->TargetState = STATE_BITE;
			else if (creatureInfo->mood == STALK_MOOD)
				item->TargetState = STATE_STALK;
			else if (creatureInfo->mood == BORED_MOOD)
				item->TargetState = STATE_STOP;
			else
				item->TargetState = STATE_RUN;
			break;

		case STATE_STALK:
			creatureInfo->maximumTurn = STALK_TURN;

			if (creatureInfo->mood == ESCAPE_MOOD)
				item->TargetState = STATE_RUN;
			else if (info.distance < pow(345, 2) && info.bite)
				item->TargetState = STATE_BITE;
			else if (info.distance > pow(3072, 2))
				item->TargetState = STATE_RUN;
			else if (creatureInfo->mood == ATTACK_MOOD)
			{
				if (!info.ahead || info.distance > pow(1536, 2) ||
					(info.enemyFacing < FRONT_ARC && info.enemyFacing > -FRONT_ARC))
					item->TargetState = STATE_RUN;
			}
			else if (GetRandomControl() < HOWL_CHANCE)
			{
				item->RequiredState = STATE_HOWL;
				item->TargetState = STATE_CROUCH;
			}
			else if (creatureInfo->mood == BORED_MOOD)
				item->TargetState = STATE_CROUCH;
			break;

		case 3:
			creatureInfo->maximumTurn = RUN_TURN;
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
			else if (creatureInfo->mood == STALK_MOOD && info.distance < STALK_RANGE)
			{
				item->RequiredState = STATE_STALK;
				item->TargetState = STATE_CROUCH;
			}
			else if (creatureInfo->mood == BORED_MOOD)
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
	CreatureAnimation(itemNumber, angle, tilt);
}
