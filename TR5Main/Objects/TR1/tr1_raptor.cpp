#include "newobjects.h"
#include "box.h"
#include "effects.h"
#include "../specific/setup.h"
#include "level.h"
#include "lara.h"

enum RAPTOR_STATE
{
	RAPTOR_EMPTY,
	RAPTOR_STOP,
	RAPTOR_WALK,
	RAPTOR_RUN,
	RAPTOR_ATTACK1,
	RAPTOR_DEATH,
	RAPTOR_WARNING,
	RAPTOR_ATTACK2,
	RAPTOR_ATTACK3
};

BITE_INFO raptorBite = { 0, 66, 318, 22 };

/* Raptor has two deaths */
#define RAPTOR_DIE_ANIM 9
#define RAPTOR_ROAR_CHANCE 0x100
#define RAPTOR_LUNGE_RANGE SQUARE(WALL_SIZE*3/2)
#define RAPTOR_ATTACK_RANGE SQUARE(WALL_SIZE*3/2)
#define RAPTOR_CLOSE_RANGE SQUARE(680)
#define RAPTOR_RUN_TURN ANGLE(4)
#define RAPTOR_WALK_TURN ANGLE(1)
#define RAPTOR_LUNGE_DAMAGE 100
#define RAPTOR_BITE_DAMAGE 100
#define RAPTOR_CHARGE_DAMAGE 100
#define RAPTOR_TOUCH (0xFF7C00)

void Tr1RaptorControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* raptor;
	AI_INFO info;
	short head, angle, tilt;

	item = &Items[itemNum];
	raptor = (CREATURE_INFO*)item->data;
	head = angle = tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != RAPTOR_DEATH)
		{
			item->animNumber = Objects[ID_RAPTOR].animIndex + RAPTOR_DIE_ANIM + (short)(GetRandomControl() / 16200);
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = RAPTOR_DEATH;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, raptor->maximumTurn);

		switch (item->currentAnimState)
		{
			case RAPTOR_STOP:
				if (item->requiredAnimState)
				{
					item->goalAnimState = item->requiredAnimState;
				}
				else if (item->touchBits & RAPTOR_TOUCH)
				{
					item->goalAnimState = RAPTOR_ATTACK3;
				}
				else if (info.distance < RAPTOR_CLOSE_RANGE && info.bite)
				{
					item->goalAnimState = RAPTOR_ATTACK3;
				}
				else if (info.bite && info.distance < RAPTOR_LUNGE_RANGE)
				{
					item->goalAnimState = RAPTOR_ATTACK1;
				}
				else if (raptor->mood == BORED_MOOD)
				{
					item->goalAnimState = RAPTOR_WALK;
				}
				else
				{
					item->goalAnimState = RAPTOR_RUN;
				}

				break;

			case RAPTOR_WALK:
				raptor->maximumTurn = RAPTOR_WALK_TURN;

				if (raptor->mood != BORED_MOOD)
				{
					item->goalAnimState = RAPTOR_STOP;
				}
				else if (info.ahead && GetRandomControl() < RAPTOR_ROAR_CHANCE)
				{
					item->requiredAnimState = RAPTOR_WARNING;
					item->goalAnimState = RAPTOR_STOP;
				}
				break;

			case RAPTOR_RUN:
				tilt = angle / 3;
				raptor->maximumTurn = RAPTOR_RUN_TURN;

				if (item->touchBits & RAPTOR_TOUCH)
				{
					item->goalAnimState = RAPTOR_STOP;
				}
				else if (info.bite && info.distance < RAPTOR_ATTACK_RANGE)
				{
					if (item->goalAnimState == RAPTOR_RUN)
					{
						if (GetRandomControl() < 0x2000)
						{
							item->goalAnimState = RAPTOR_STOP;
						}
						else
						{
							item->goalAnimState = RAPTOR_ATTACK2;
						}
					}
				}
				else if (info.ahead && raptor->mood != ESCAPE_MOOD && GetRandomControl() < RAPTOR_ROAR_CHANCE)
				{
					item->requiredAnimState = RAPTOR_WARNING;
					item->goalAnimState = RAPTOR_STOP;
				}
				else if (raptor->mood == BORED_MOOD)
				{
					item->goalAnimState = RAPTOR_STOP;
				}
				break;

			case RAPTOR_ATTACK1:
				tilt = angle;

				/* Lunge attack */
				if (!item->requiredAnimState && info.ahead && (item->touchBits & RAPTOR_TOUCH))
				{
					CreatureEffect(item, &raptorBite, DoBloodSplat);

					LaraItem->hitPoints -= RAPTOR_LUNGE_DAMAGE;
					LaraItem->hitStatus = true;

					item->requiredAnimState = RAPTOR_STOP;
				}
				break;

			case RAPTOR_ATTACK3:
				tilt = angle;

				/* Close bite attack */
				if (!item->requiredAnimState && (item->touchBits & RAPTOR_TOUCH))
				{
					CreatureEffect(item, &raptorBite, DoBloodSplat);

					LaraItem->hitPoints -= RAPTOR_BITE_DAMAGE;
					LaraItem->hitStatus = true;

					item->requiredAnimState = RAPTOR_STOP;
				}
				break;

			case RAPTOR_ATTACK2:
				tilt = angle;

				/* Charge attack */
				if (!item->requiredAnimState && info.ahead && (item->touchBits & RAPTOR_TOUCH))
				{
					CreatureEffect(item, &raptorBite, DoBloodSplat);

					LaraItem->hitPoints -= RAPTOR_CHARGE_DAMAGE;
					LaraItem->hitStatus = true;

					item->requiredAnimState = RAPTOR_RUN;
				}

				break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, tilt);
}