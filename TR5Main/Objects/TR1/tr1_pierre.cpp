#include "framework.h"
#include "newobjects.h"
#include "items.h"
#include "box.h"
#include "people.h"
#include "lot.h"
#include "setup.h"
#include "camera.h"
#include "level.h"

enum PIERRE_STATE {
	PEOPLE_EMPTY, PEOPLE_STOP, PEOPLE_WALK, PEOPLE_RUN, PEOPLE_AIM,
	PEOPLE_DEATH, PEOPLE_POSE, PEOPLE_SHOOT
};

#define PIERRE_DIE_ANIM 12
#define PIERRE_WIMP_CHANCE 0x2000
#define PIERRE_RUN_HITPOINTS 40
#define PIERRE_DISAPPEAR 10
#define PEOPLE_WALK_TURN ANGLE(3)
#define PEOPLE_RUN_TURN ANGLE(6)
#define PEOPLE_POSE_CHANCE 0x60
#define PEOPLE_WALK_RANGE SQUARE(WALL_SIZE*3)
#define PEOPLE_SHOT_DAMAGE 50

BITE_INFO pierre_gun1 = { 60, 200, 0, 11 };
BITE_INFO pierre_gun2 = { -57, 200, 0, 14 };

short pierre_item;

void Tr1PierreControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* people;
	short angle, head, tilt;
	AI_INFO info;
	GAME_VECTOR start, end;

	item = &Items[itemNum];

	/* Avoid packs of Pierre's */
	if (pierre_item != NO_ITEM)
	{
		if (pierre_item != itemNum)
		{
			/* Don't kill the one true Pierre */
			if (item->flags & ONESHOT)
				KillItem(pierre_item);
			else
				KillItem(itemNum);
		}
	}
	else
	{
		pierre_item = itemNum;
	}

	people = (CREATURE_INFO*)item->data;
	head = angle = tilt = 0;

	/* If not ONESHOT, death cannot happen, but Pierre will run away */
	if (item->hitPoints <= PIERRE_RUN_HITPOINTS && !(item->flags & ONESHOT))
	{
		item->hitPoints = PIERRE_RUN_HITPOINTS;
		people->flags++;
	}

	/* Has Pierre been killed? */
	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != PEOPLE_DEATH)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + PIERRE_DIE_ANIM;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = PEOPLE_DEATH;
			// drop magnum/scion_item2/key_item1.
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		/* Flags set means it is time to run away */
		if (people->flags)
		{
			/* Lie anout situation to fool CreatureMood */
			info.enemyZone = -1;
			item->hitStatus = true;
		}
		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		angle = CreatureTurn(item, people->maximumTurn);

		/* Decide on Pierre's action based on mood, anim state and AI info */
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
				ShotLara(item, &info, &pierre_gun1, head, PEOPLE_SHOT_DAMAGE);
				ShotLara(item, &info, &pierre_gun2, head, PEOPLE_SHOT_DAMAGE);
				item->requiredAnimState = PEOPLE_AIM;
			}

			if (people->mood == ESCAPE_MOOD && PIERRE_WIMP_CHANCE < GetRandomControl())
				item->requiredAnimState = PEOPLE_STOP;
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, 0);

	/* When Pierre goes out of sight when he's escaping, you've lost him */
	if (people->flags)
	{
		start.x = item->pos.xPos;
		start.y = item->pos.yPos - WALL_SIZE;
		start.z = item->pos.zPos;

		end.x = Camera.pos.x;
		end.y = Camera.pos.y;
		end.z = Camera.pos.z;
		end.roomNumber = Camera.pos.roomNumber;

		if (!LOS(&end, &start))
		{
			if (people->flags > PIERRE_DISAPPEAR)
			{
				item->hitPoints = -16384;
				DisableBaddieAI(itemNum);
				KillItem(itemNum);
				pierre_item = NO_ITEM;
			}
		}
		else
		{
			people->flags = 1; // reset timer
		}
	}

	/* If area Pierre in gets flooded then make him disappear */
	if (GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber) != NO_HEIGHT)
	{
		item->hitPoints = -16384;
		DisableBaddieAI(itemNum);
		KillItem(itemNum);
		pierre_item = NO_ITEM;
	}
}