#include "framework.h"
#include "Objects/TR1/Entity/tr1_bear.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO BearBite = { 0, 96, 335, 14 };

#define TOUCH 0x2406C
#define ROAR_CHANCE 0x50
#define REAR_CHANCE 0x300
#define DROP_CHANCE 0x600
#define REAR_RANGE pow(SECTOR(2), 2)
#define ATTACK_RANGE pow(SECTOR(1), 2)
#define PAT_RANGE pow(600, 2)
#define RUN_TURN ANGLE(5.0f)
#define WALK_TURN ANGLE(2.0f)
#define EAT_RANGE pow(CLICK(3), 2)
#define CHARGE_DAMAGE 3
#define SLAM_DAMAGE 200
#define ATTACK_DAMAGE 200
#define PAT_DAMAGE 400

enum BearState
{
	BEAR_STATE_STROLL = 0,
	BEAR_STATE_IDLE = 1,
	BEAR_STATE_WALK = 2,
	BEAR_STATE_RUN = 3,
	BEAR_STATE_REAR = 4,
	BEAR_STATE_ROAR = 5,
	BEAR_STATE_ATTACK_1 = 6,
	BEAR_STATE_ATTACK_2 = 7,
	BEAR_STATE_CHOMP = 8,
	BEAR_STATE_DEATH = 9
};

// TODO
enum BearAnim
{

};

void BearControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short head = 0;
	short angle = 0;

	if (item->HitPoints <= 0)
	{
		angle = CreatureTurn(item, ANGLE(1.0f));

		switch (item->ActiveState)
		{
			case BEAR_STATE_WALK:
			{
				item->TargetState = BEAR_STATE_REAR;
				break;
			}
			case BEAR_STATE_RUN:
			case BEAR_STATE_STROLL:
			{
				item->TargetState = BEAR_STATE_IDLE;
				break;
			}
			case BEAR_STATE_REAR:
			{
				item->TargetState = BEAR_STATE_DEATH;
				creature->flags = 1;
				break;
			}
			case BEAR_STATE_IDLE:
			{
				item->TargetState = BEAR_STATE_DEATH;
				creature->flags = 0;
				break;
			}
			case BEAR_STATE_DEATH:
			{
				if (creature->flags && item->TouchBits & TOUCH)
				{
					creature->flags = 0;

					LaraItem->HitPoints -= SLAM_DAMAGE;
					LaraItem->HitStatus = 1;
				}

				break;
			}
		}
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		if (AI.ahead)
			head = AI.angle;

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (item->HitStatus)
			creature->flags = 1;

		const bool laraDead = LaraItem->HitPoints <= 0;

		switch (item->ActiveState)
		{
		case BEAR_STATE_IDLE:
			if (laraDead)
			{
				if (AI.bite && AI.distance < EAT_RANGE)
					item->TargetState = BEAR_STATE_CHOMP;
				else
					item->TargetState = BEAR_STATE_STROLL;
			}
			else if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (creature->mood == MoodType::Bored)
				item->TargetState = BEAR_STATE_STROLL;
			else
				item->TargetState = BEAR_STATE_RUN;
			
			break;

		case BEAR_STATE_STROLL:
			creature->maximumTurn = WALK_TURN;

			if (laraDead && item->TouchBits & TOUCH && AI.ahead)
				item->TargetState = BEAR_STATE_IDLE;
			else if (creature->mood != MoodType::Bored)
			{
				item->TargetState = BEAR_STATE_IDLE;

				if (creature->mood == MoodType::Escape)
					item->RequiredState = BEAR_STATE_STROLL;
			}
			else if (GetRandomControl() < ROAR_CHANCE)
			{
				item->RequiredState = BEAR_STATE_ROAR;
				item->TargetState = BEAR_STATE_IDLE;
			}

			break;

		case BEAR_STATE_RUN:
			creature->maximumTurn = RUN_TURN;

			if (item->TouchBits & TOUCH)
			{
				LaraItem->HitPoints -= CHARGE_DAMAGE;
				LaraItem->HitStatus = true;
			}

			if (creature->mood == MoodType::Bored || laraDead)
				item->TargetState = BEAR_STATE_IDLE;
			else if (AI.ahead && !item->RequiredState)
			{
				if (!creature->flags && AI.distance < REAR_RANGE && GetRandomControl() < REAR_CHANCE)
				{
					item->RequiredState = BEAR_STATE_REAR;
					item->TargetState = BEAR_STATE_IDLE;
				}
				else if (AI.distance < ATTACK_RANGE)
					item->TargetState = BEAR_STATE_ATTACK_1;
			}

			break;

		case BEAR_STATE_REAR:
			if (creature->flags)
			{
				item->RequiredState = BEAR_STATE_STROLL;
				item->TargetState = BEAR_STATE_IDLE;
			}
			else if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (creature->mood == MoodType::Bored || creature->mood == MoodType::Escape)
				item->TargetState = BEAR_STATE_IDLE;
			else if (AI.bite && AI.distance < PAT_RANGE)
				item->TargetState = BEAR_STATE_ATTACK_2;
			else
				item->TargetState = BEAR_STATE_WALK;
			
			break;

		case BEAR_STATE_WALK:
			if (creature->flags)
			{
				item->RequiredState = BEAR_STATE_STROLL;
				item->TargetState = BEAR_STATE_REAR;
			}
			else if (AI.ahead && (item->TouchBits & TOUCH))
				item->TargetState = BEAR_STATE_REAR;
			else if (creature->mood == MoodType::Escape)
			{
				item->TargetState = BEAR_STATE_REAR;
				item->RequiredState = BEAR_STATE_STROLL;
			}
			else if (creature->mood == MoodType::Bored || GetRandomControl() < ROAR_CHANCE)
			{
				item->RequiredState = BEAR_STATE_ROAR;
				item->TargetState = BEAR_STATE_REAR;
			}
			else if (AI.distance > REAR_RANGE || GetRandomControl() < DROP_CHANCE)
			{
				item->RequiredState = BEAR_STATE_IDLE;
				item->TargetState = BEAR_STATE_REAR;
			}

			break;

		case BEAR_STATE_ATTACK_2:
			if (!item->RequiredState && item->TouchBits & TOUCH)
			{
				item->RequiredState = BEAR_STATE_REAR;

				LaraItem->HitPoints -= PAT_DAMAGE;
				LaraItem->HitStatus = true;
			}

			break;

		case BEAR_STATE_ATTACK_1:
			if (!item->RequiredState && (item->TouchBits & TOUCH))
			{
				CreatureEffect(item, &BearBite, DoBloodSplat);
				item->RequiredState = BEAR_STATE_IDLE;

				LaraItem->HitPoints -= ATTACK_DAMAGE;
				LaraItem->HitStatus = true;
			}

			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNumber, angle, 0);
}
