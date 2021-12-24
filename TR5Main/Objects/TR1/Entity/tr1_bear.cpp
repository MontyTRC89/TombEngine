#include "framework.h"
#include "Objects/TR1/Entity/tr1_bear.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO bearBite = { 0, 96, 335, 14 };

enum bearStates{
	BEAR_STROLL,
	BEAR_STOP,
	BEAR_WALK,
	BEAR_RUN,
	BEAR_REAR,
	BEAR_ROAR,
	BEAR_ATTACK1,
	BEAR_ATTACK2,
	BEAR_EAT,
	BEAR_DEATH
};
#define TOUCH 0x2406C


#define ROAR_CHANCE 0x50
#define REAR_CHANCE 0x300
#define DROP_CHANCE 0x600

#define REAR_RANGE   SQUARE(WALL_SIZE*2)
#define ATTACK_RANGE SQUARE(WALL_SIZE)
#define PAT_RANGE    SQUARE(600)

#define RUN_TURN  ANGLE(5)
#define WALK_TURN ANGLE(2)

#define EAT_RANGE     SQUARE(WALL_SIZE*3/4)


#define CHARGE_DAMAGE 3
#define SLAM_DAMAGE   200
#define ATTACK_DAMAGE 200
#define PAT_DAMAGE    400

void BearControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	short head = 0;
	short angle;

	if (item->hitPoints <= 0)
	{
		angle = CreatureTurn(item, ANGLE(1));

		switch (item->currentAnimState)
		{
			case BEAR_WALK:
			{
				item->goalAnimState = BEAR_REAR;
				break;
			}
			case BEAR_RUN:
			case BEAR_STROLL:
			{
				item->goalAnimState = BEAR_STOP;
				break;
			}
			case BEAR_REAR:
			{
				creature->flags = 1;
				item->goalAnimState = BEAR_DEATH;
				break;
			}
			case BEAR_STOP:
			{
				creature->flags = 0;
				item->goalAnimState = BEAR_DEATH;
				break;
			}
			case BEAR_DEATH:
			{
				if (creature->flags && (item->touchBits & TOUCH))
				{
					LaraItem->hitPoints -= SLAM_DAMAGE;
					LaraItem->hitStatus = 1;
					creature->flags = 0;
				}

				break;
			}
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		if (item->hitStatus)
			creature->flags = 1;

		const bool Laradead = (LaraItem->hitPoints <= 0);

		switch (item->currentAnimState)
		{
		case BEAR_STOP:
			if (Laradead)
			{
				if (info.bite && info.distance < EAT_RANGE)
				{
					item->goalAnimState = BEAR_EAT;
				}
				else
				{
					item->goalAnimState = BEAR_STROLL;
				}
			}
			else if (item->requiredAnimState)
			{
				item->goalAnimState = item->requiredAnimState;
			}
			else if (creature->mood == BORED_MOOD)
			{
				item->goalAnimState = BEAR_STROLL;
			}
			else
			{
				item->goalAnimState = BEAR_RUN;
			}
			break;

		case BEAR_STROLL:
			creature->maximumTurn = WALK_TURN;

			if (Laradead && (item->touchBits & TOUCH) && info.ahead)
			{
				item->goalAnimState = BEAR_STOP;
			}
			else if (creature->mood != BORED_MOOD)
			{
				item->goalAnimState = BEAR_STOP;
				if (creature->mood == ESCAPE_MOOD)
				{
					item->requiredAnimState = BEAR_STROLL;
				}
			}
			else if (GetRandomControl() < ROAR_CHANCE)
			{
				item->requiredAnimState = BEAR_ROAR;
				item->goalAnimState = BEAR_STOP;
			}
			break;

		case BEAR_RUN:
			creature->maximumTurn = RUN_TURN;

			if (item->touchBits & TOUCH)
			{
				LaraItem->hitPoints -= CHARGE_DAMAGE;
				LaraItem->hitStatus = true;
			}

			if (creature->mood == BORED_MOOD || Laradead)
			{
				item->goalAnimState = BEAR_STOP;
			}
			else if (info.ahead && !item->requiredAnimState)
			{
				if (!creature->flags && info.distance < REAR_RANGE && GetRandomControl() < REAR_CHANCE)
				{
					item->requiredAnimState = BEAR_REAR;
					item->goalAnimState = BEAR_STOP;
				}
				else if (info.distance < ATTACK_RANGE)
				{
					item->goalAnimState = BEAR_ATTACK1;
				}
			}
			break;

		case BEAR_REAR:
			if (creature->flags)
			{
				item->requiredAnimState = BEAR_STROLL;
				item->goalAnimState = BEAR_STOP;
			}
			else if (item->requiredAnimState)
			{
				item->goalAnimState = item->requiredAnimState;
			}
			else if (creature->mood == BORED_MOOD || creature->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = BEAR_STOP;
			}
			else if (info.bite && info.distance < PAT_RANGE)
			{
				item->goalAnimState = BEAR_ATTACK2;
			}
			else
			{
				item->goalAnimState = BEAR_WALK;
			}
			break;

		case BEAR_WALK:
			if (creature->flags)
			{
				item->requiredAnimState = BEAR_STROLL;
				item->goalAnimState = BEAR_REAR;
			}
			else if (info.ahead && (item->touchBits & TOUCH))
			{
				item->goalAnimState = BEAR_REAR;
			}
			else if (creature->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = BEAR_REAR;
				item->requiredAnimState = BEAR_STROLL;
			}
			else if (creature->mood == BORED_MOOD || GetRandomControl() < ROAR_CHANCE)
			{
				item->requiredAnimState = BEAR_ROAR;
				item->goalAnimState = BEAR_REAR;
			}
			else if (info.distance > REAR_RANGE || GetRandomControl() < DROP_CHANCE)
			{
				item->requiredAnimState = BEAR_STOP;
				item->goalAnimState = BEAR_REAR;
			}
			break;

		case BEAR_ATTACK2:
			if (!item->requiredAnimState && (item->touchBits & TOUCH))
			{
				LaraItem->hitPoints -= PAT_DAMAGE;
				LaraItem->hitStatus = true;
				item->requiredAnimState = BEAR_REAR;
			}

			break;

		case BEAR_ATTACK1:
			if (!item->requiredAnimState && (item->touchBits & TOUCH))
			{
				CreatureEffect(item, &bearBite, DoBloodSplat);
				LaraItem->hitPoints -= ATTACK_DAMAGE;
				LaraItem->hitStatus = true;
				item->requiredAnimState = BEAR_STOP;
			}
			break;

		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, 0);
}
