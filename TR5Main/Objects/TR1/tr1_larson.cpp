#include "../newobjects.h"
#include "../../Game/Box.h"
#include "../../Game/people.h"
#include "../../Specific/setup.h"
#include "../../Specific/level.h"
#include "../../Game/control.h"

enum LARSON_STATE {
	PEOPLE_EMPTY, PEOPLE_STOP, PEOPLE_WALK, PEOPLE_RUN, PEOPLE_AIM,
	PEOPLE_DEATH, PEOPLE_POSE, PEOPLE_SHOOT
};

#define PEOPLE_WALK_TURN ANGLE(3)
#define PEOPLE_RUN_TURN ANGLE(6)
#define PEOPLE_POSE_CHANCE 0x60
#define PEOPLE_WALK_RANGE SQUARE(WALL_SIZE*3)
#define PEOPLE_SHOT_DAMAGE 50
#define LARSON_DIE_ANIM 15

BITE_INFO larson_gun = { -60, 170, 0, 14 };

void Tr1LarsonControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* people;
	AI_INFO info;
	short angle, head, tilt;

	item = &Items[itemNum];
	people = (CREATURE_INFO*)item->data;
	head = angle = tilt = 0;

	/* Has person been killed? */
	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != PEOPLE_DEATH)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + LARSON_DIE_ANIM;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = PEOPLE_DEATH;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, people->maximumTurn);

		switch (item->currentAnimState)
		{
		case PEOPLE_STOP:
			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else if (people->mood == BORED_MOOD)
				item->goalAnimState = (GetRandomControl() < PEOPLE_POSE_CHANCE) ? PEOPLE_POSE : PEOPLE_WALK;
			else if (people->mood == ESCAPE_MOOD)
				item->goalAnimState = PEOPLE_RUN;
			else
				item->goalAnimState = PEOPLE_WALK;
			break;

		case PEOPLE_POSE:
			if (people->mood != BORED_MOOD)
				item->goalAnimState = PEOPLE_STOP;
			else if (GetRandomControl() < PEOPLE_POSE_CHANCE)
			{
				item->requiredAnimState = PEOPLE_WALK;
				item->goalAnimState = PEOPLE_STOP;
			}
			break;

		case PEOPLE_WALK:
			people->maximumTurn = PEOPLE_WALK_TURN;

			if (people->mood == BORED_MOOD && GetRandomControl() < PEOPLE_POSE_CHANCE)
			{
				item->requiredAnimState = PEOPLE_POSE;
				item->goalAnimState = PEOPLE_STOP;
			}
			else if (people->mood == ESCAPE_MOOD)
			{
				item->requiredAnimState = PEOPLE_RUN;
				item->goalAnimState = PEOPLE_STOP;
			}
			else if (Targetable(item, &info))
			{
				item->requiredAnimState = PEOPLE_AIM;
				item->goalAnimState = PEOPLE_STOP;
			}
			else if (!info.ahead || info.distance > PEOPLE_WALK_RANGE)
			{
				item->requiredAnimState = PEOPLE_RUN;
				item->goalAnimState = PEOPLE_STOP;
			}
			break;

		case PEOPLE_RUN:
			people->maximumTurn = PEOPLE_RUN_TURN;
			tilt = angle / 2;

			if (people->mood == BORED_MOOD && GetRandomControl() < PEOPLE_POSE_CHANCE)
			{
				item->requiredAnimState = PEOPLE_POSE;
				item->goalAnimState = PEOPLE_STOP;
			}
			else if (Targetable(item, &info))
			{
				item->requiredAnimState = PEOPLE_AIM;
				item->goalAnimState = PEOPLE_STOP;
			}
			else if (info.ahead && info.distance < PEOPLE_WALK_RANGE)
			{
				item->requiredAnimState = PEOPLE_WALK;
				item->goalAnimState = PEOPLE_STOP;
			}
			break;

		case PEOPLE_AIM:
			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			else if (Targetable(item, &info))
				item->goalAnimState = PEOPLE_SHOOT;
			else
				item->goalAnimState = PEOPLE_STOP;
			break;

		case PEOPLE_SHOOT:
			/* Required state is set after this so only one shot is fired */
			if (!item->requiredAnimState)
			{
				ShotLara(item, &info, &larson_gun, head, PEOPLE_SHOT_DAMAGE);
				item->requiredAnimState = PEOPLE_AIM;
			}

			if (people->mood == ESCAPE_MOOD)
				item->requiredAnimState = PEOPLE_STOP;
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, tilt);
}