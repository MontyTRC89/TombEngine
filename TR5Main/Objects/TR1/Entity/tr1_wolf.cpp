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
	item->frameNumber = SLEEP_FRAME;
}

void WolfControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	short head;
	short angle;
	short tilt;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	head = angle = tilt = 0;
	AI_INFO info;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != STATE_DEATH)
		{
			item->animNumber = Objects[ID_WOLF].animIndex + DIE_ANIM + (short)(GetRandomControl() / 11000);
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = STATE_DEATH;
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

		switch (item->currentAnimState)
		{
		case STATE_SLEEP:
			head = 0;

			if (creature->mood == ESCAPE_MOOD || info.zoneNumber == info.enemyZone)
			{
				item->requiredAnimState = STATE_CROUCH;
				item->goalAnimState = STATE_STOP;
			}
			else if (GetRandomControl() < WAKE_CHANCE)
			{
				item->requiredAnimState = STATE_WALK;
				item->goalAnimState = STATE_STOP;
			}
			break;

		case STATE_STOP:
			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else
				item->goalAnimState = STATE_WALK;
			break;

		case STATE_WALK:
			creature->maximumTurn = WALK_TURN;

			if (creature->mood != BORED_MOOD)
			{
				item->goalAnimState = STATE_STALK;
				item->requiredAnimState = STATE_EMPTY;
			}
			else if (GetRandomControl() < SLEEP_CHANCE)
			{
				item->requiredAnimState = STATE_SLEEP;
				item->goalAnimState = STATE_STOP;
			}
			break;

		case STATE_CROUCH:
			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else if (creature->mood == ESCAPE_MOOD)
				item->goalAnimState = STATE_RUN;
			else if (info.distance < SQUARE(345) && info.bite)
				item->goalAnimState = STATE_BITE;
			else if (creature->mood == STALK_MOOD)
				item->goalAnimState = STATE_STALK;
			else if (creature->mood == BORED_MOOD)
				item->goalAnimState = STATE_STOP;
			else
				item->goalAnimState = STATE_RUN;
			break;

		case STATE_STALK:
			creature->maximumTurn = STALK_TURN;

			if (creature->mood == ESCAPE_MOOD)
				item->goalAnimState = STATE_RUN;
			else if (info.distance < SQUARE(345) && info.bite)
				item->goalAnimState = STATE_BITE;
			else if (info.distance > SQUARE(3072))
				item->goalAnimState = STATE_RUN;
			else if (creature->mood == ATTACK_MOOD)
			{
				if (!info.ahead || info.distance > SQUARE(1536) ||
					(info.enemyFacing < FRONT_ARC && info.enemyFacing > -FRONT_ARC))
					item->goalAnimState = STATE_RUN;
			}
			else if (GetRandomControl() < HOWL_CHANCE)
			{
				item->requiredAnimState = STATE_HOWL;
				item->goalAnimState = STATE_CROUCH;
			}
			else if (creature->mood == BORED_MOOD)
				item->goalAnimState = STATE_CROUCH;
			break;

		case 3:
			creature->maximumTurn = RUN_TURN;
			tilt = angle;

			if (info.ahead && info.distance < ATTACK_RANGE)
			{
				if (info.distance > (ATTACK_RANGE / 2) &&
					(info.enemyFacing > FRONT_ARC || info.enemyFacing < -FRONT_ARC))
				{
					item->requiredAnimState = STATE_STALK;
					item->goalAnimState = STATE_CROUCH;
				}
				else
				{
					item->goalAnimState = STATE_ATTACK;
					item->requiredAnimState = STATE_EMPTY;
				}
			}
			else if (creature->mood == STALK_MOOD && info.distance < STALK_RANGE)
			{
				item->requiredAnimState = STATE_STALK;
				item->goalAnimState = STATE_CROUCH;
			}
			else if (creature->mood == BORED_MOOD)
				item->goalAnimState = STATE_CROUCH;
			break;

		case STATE_ATTACK:
			tilt = angle;
			if (!item->requiredAnimState && (item->touchBits & TOUCH))
			{
				CreatureEffect(item, &wolfBite, DoBloodSplat);
				LaraItem->hitPoints -= LUNGE_DAMAGE;
				LaraItem->hitStatus = true;
				item->requiredAnimState = STATE_RUN;
			}
			item->goalAnimState = STATE_RUN;
			break;

		case 12:
			if (!item->requiredAnimState && (item->touchBits & TOUCH) && info.ahead)
			{
				CreatureEffect(item, &wolfBite, DoBloodSplat);
				LaraItem->hitPoints -= BITE_DAMAGE;
				LaraItem->hitStatus = true;
				item->requiredAnimState = STATE_CROUCH;
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, tilt);
}
