#include "framework.h"
#include "Objects/TR3/Entity/tr3_civvy.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO civvy_hit = { 0,0,0, 13 };

enum civvy_anims {
	CIVVY_EMPTY, CIVVY_STOP, CIVVY_WALK, CIVVY_PUNCH2, CIVVY_AIM2, CIVVY_WAIT, CIVVY_AIM1, CIVVY_AIM0, CIVVY_PUNCH1, CIVVY_PUNCH0,
	CIVVY_RUN, CIVVY_DEATH, CIVVY_CLIMB3, CIVVY_CLIMB1, CIVVY_CLIMB2, CIVVY_FALL3
};

#define CIVVY_WALK_TURN ANGLE(5)
#define CIVVY_RUN_TURN ANGLE(6)
#define CIVVY_HIT_DAMAGE 40
#define CIVVY_SWIPE_DAMAGE 50
#define CIVVY_ATTACK0_RANGE SQUARE(WALL_SIZE/3)
#define CIVVY_ATTACK1_RANGE SQUARE(WALL_SIZE*2/3)
#define CIVVY_ATTACK2_RANGE SQUARE(WALL_SIZE)
#define CIVVY_WALK_RANGE SQUARE(WALL_SIZE)
#define CIVVY_ESCAPE_RANGE SQUARE(WALL_SIZE*3)
#define CIVVY_WALK_CHANCE 0x100
#define CIVVY_WAIT_CHANCE 0x100
#define CIVVY_DIE_ANIM 26
#define CIVVY_STOP_ANIM 6
#define CIVVY_CLIMB1_ANIM 28
#define CIVVY_CLIMB2_ANIM 29
#define CIVVY_CLIMB3_ANIM 27
#define CIVVY_FALL3_ANIM  30
#define CIVVY_TOUCH 0x2400
#define CIVVY_VAULT_SHIFT 260
#define CIVVY_AWARE_DISTANCE SQUARE(WALL_SIZE)

void InitialiseCivvy(short item_number)
{
	ITEM_INFO* item;

	item = &g_Level.Items[item_number];
	InitialiseCreature(item_number);
	item->animNumber = Objects[item->objectNumber].animIndex + CIVVY_STOP_ANIM;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->currentAnimState = item->goalAnimState = CIVVY_STOP;
}

void CivvyControl(short item_number)
{
	if (!CreatureActive(item_number))
		return;

	ITEM_INFO* item, *real_enemy;
	CREATURE_INFO* civvy;
	short angle, torso_y, torso_x, head, tilt;
	int lara_dx, lara_dz;
	AI_INFO info, lara_info;

	item = &g_Level.Items[item_number];
	civvy = (CREATURE_INFO*)item->data;
	torso_y = torso_x = head = angle = tilt = 0;

	if (item->boxNumber != NO_BOX && (g_Level.Boxes[item->boxNumber].flags & BLOCKED))
	{
		DoLotsOfBlood(item->pos.xPos, item->pos.yPos - (GetRandomControl() & 255) - 32, item->pos.zPos, (GetRandomControl() & 127) + 128, GetRandomControl() << 1, item->roomNumber, 3);
		item->hitPoints -= 20;
	}

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != CIVVY_DEATH)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + CIVVY_DIE_ANIM;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = CIVVY_DEATH;
			civvy->LOT.step = STEP_SIZE;
		}
	}
	else
	{
		if (item->aiBits)
			GetAITarget(civvy);
		else
			civvy->enemy = LaraItem;

		CreatureAIInfo(item, &info);

		if (civvy->enemy == LaraItem)
		{
			lara_info.angle = info.angle;
			lara_info.distance = info.distance;
		}
		else
		{
			lara_dz = LaraItem->pos.zPos - item->pos.zPos;
			lara_dx = LaraItem->pos.xPos - item->pos.xPos;
			lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.yRot;
			lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
		}

		GetCreatureMood(item, &info, VIOLENT);

		if (civvy->enemy == LaraItem && info.distance > CIVVY_ESCAPE_RANGE&& info.enemyFacing < 0x3000 && info.enemyFacing > -0x3000)
			civvy->mood = ESCAPE_MOOD;

		CreatureMood(item, &info, VIOLENT);


		angle = CreatureTurn(item, civvy->maximumTurn);

		real_enemy = civvy->enemy;
		civvy->enemy = LaraItem;

		if ((lara_info.distance < CIVVY_AWARE_DISTANCE || item->hitStatus || TargetVisible(item, &lara_info)) && !(item->aiBits & FOLLOW))
		{
			if (!civvy->alerted)
				SoundEffect(300, &item->pos, 0);
			AlertAllGuards(item_number);
		}
		civvy->enemy = real_enemy;

		switch (item->currentAnimState)
		{
		case CIVVY_WAIT:
			if (civvy->alerted || item->goalAnimState == CIVVY_RUN)
			{
				item->goalAnimState = CIVVY_STOP;
				break;
			}

		case CIVVY_STOP:
			civvy->flags = 0;
			civvy->maximumTurn = 0;
			head = lara_info.angle;

			if (item->aiBits & GUARD)
			{
				head = AIGuard(civvy);
				if (!(GetRandomControl() & 0xFF))
				{
					if (item->currentAnimState == CIVVY_STOP)
						item->goalAnimState = CIVVY_WAIT;
					else
						item->goalAnimState = CIVVY_STOP;
				}
				break;
			}

			else if (item->aiBits & PATROL1)
				item->goalAnimState = CIVVY_WALK;

			else if (civvy->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead)
					item->goalAnimState = CIVVY_STOP;
				else
					item->goalAnimState = CIVVY_RUN;
			}
			else if (civvy->mood == BORED_MOOD || ((item->aiBits & FOLLOW) && (civvy->reachedGoal || lara_info.distance > SQUARE(WALL_SIZE * 2))))
			{
				if (item->requiredAnimState)
					item->goalAnimState = item->requiredAnimState;
				else if (info.ahead)
					item->goalAnimState = CIVVY_STOP;
				else
					item->goalAnimState = CIVVY_RUN;
			}
			else if (info.bite && info.distance < CIVVY_ATTACK0_RANGE)
				item->goalAnimState = CIVVY_AIM0;
			else if (info.bite && info.distance < CIVVY_ATTACK1_RANGE)
				item->goalAnimState = CIVVY_AIM1;
			else if (info.bite && info.distance < CIVVY_WALK_RANGE)
				item->goalAnimState = CIVVY_WALK;
			else
				item->goalAnimState = CIVVY_RUN;
			break;

		case CIVVY_WALK:
			head = lara_info.angle;

			civvy->maximumTurn = CIVVY_WALK_TURN;

			if (item->aiBits & PATROL1)
			{
				item->goalAnimState = CIVVY_WALK;
				head = 0;
			}
			else if (civvy->mood == ESCAPE_MOOD)
				item->goalAnimState = CIVVY_RUN;
			else if (civvy->mood == BORED_MOOD)
			{
				if (GetRandomControl() < CIVVY_WAIT_CHANCE)
				{
					item->requiredAnimState = CIVVY_WAIT;
					item->goalAnimState = CIVVY_STOP;
				}
			}
			else if (info.bite && info.distance < CIVVY_ATTACK0_RANGE)
				item->goalAnimState = CIVVY_STOP;
			else if (info.bite && info.distance < CIVVY_ATTACK2_RANGE)
				item->goalAnimState = CIVVY_AIM2;
			else
				item->goalAnimState = CIVVY_RUN;
			break;

		case CIVVY_RUN:
			if (info.ahead)
				head = info.angle;

			civvy->maximumTurn = CIVVY_RUN_TURN;
			tilt = angle / 2;

			if (item->aiBits & GUARD)
				item->goalAnimState = CIVVY_WAIT;
			else if (civvy->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead)
					item->goalAnimState = CIVVY_STOP;
				break;
			}
			else if ((item->aiBits & FOLLOW) && (civvy->reachedGoal || lara_info.distance > SQUARE(WALL_SIZE * 2)))
				item->goalAnimState = CIVVY_STOP;
			else if (civvy->mood == BORED_MOOD)
				item->goalAnimState = CIVVY_WALK;
			else if (info.ahead && info.distance < CIVVY_WALK_RANGE)
				item->goalAnimState = CIVVY_WALK;
			break;

		case CIVVY_AIM0:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}
			civvy->maximumTurn = CIVVY_WALK_TURN;

			civvy->flags = 0;
			if (info.bite && info.distance < CIVVY_ATTACK0_RANGE)
				item->goalAnimState = CIVVY_PUNCH0;
			else
				item->goalAnimState = CIVVY_STOP;
			break;

		case CIVVY_AIM1:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}
			civvy->maximumTurn = CIVVY_WALK_TURN;

			civvy->flags = 0;
			if (info.ahead && info.distance < CIVVY_ATTACK1_RANGE)
				item->goalAnimState = CIVVY_PUNCH1;
			else
				item->goalAnimState = CIVVY_STOP;
			break;

		case CIVVY_AIM2:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}
			civvy->maximumTurn = CIVVY_WALK_TURN;
			civvy->flags = 0;

			if (info.bite && info.distance < CIVVY_ATTACK2_RANGE)
				item->goalAnimState = CIVVY_PUNCH2;
			else
				item->goalAnimState = CIVVY_WALK;
			break;

		case CIVVY_PUNCH0:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}
			civvy->maximumTurn = CIVVY_WALK_TURN;

			if (!civvy->flags && (item->touchBits & CIVVY_TOUCH))
			{
				LaraItem->hitPoints -= CIVVY_HIT_DAMAGE;
				LaraItem->hitStatus = true;
				CreatureEffect(item, &civvy_hit, DoBloodSplat);
				SoundEffect(70, &item->pos, 0);

				civvy->flags = 1;
			}
			break;

		case CIVVY_PUNCH1:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}
			civvy->maximumTurn = CIVVY_WALK_TURN;

			if (!civvy->flags && (item->touchBits & CIVVY_TOUCH))
			{
				LaraItem->hitPoints -= CIVVY_HIT_DAMAGE;
				LaraItem->hitStatus = true;
				CreatureEffect(item, &civvy_hit, DoBloodSplat);
				SoundEffect(70, &item->pos, 0);

				civvy->flags = 1;
			}

			if (info.ahead && info.distance > CIVVY_ATTACK1_RANGE&& info.distance < CIVVY_ATTACK2_RANGE)
				item->goalAnimState = CIVVY_PUNCH2;
			break;

		case CIVVY_PUNCH2:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}
			civvy->maximumTurn = CIVVY_WALK_TURN;

			if (civvy->flags != 2 && (item->touchBits & CIVVY_TOUCH))
			{
				LaraItem->hitPoints -= CIVVY_SWIPE_DAMAGE;
				LaraItem->hitStatus = true;
				CreatureEffect(item, &civvy_hit, DoBloodSplat);
				SoundEffect(70, &item->pos, 0);

				civvy->flags = 2;
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);

	if (item->currentAnimState < CIVVY_DEATH)
	{
		switch (CreatureVault(item_number, angle, 2, CIVVY_VAULT_SHIFT))
		{
		case 2:
			civvy->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + CIVVY_CLIMB1_ANIM;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = CIVVY_CLIMB1;
			break;

		case 3:
			civvy->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + CIVVY_CLIMB2_ANIM;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = CIVVY_CLIMB2;
			break;

		case 4:
			civvy->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + CIVVY_CLIMB3_ANIM;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = CIVVY_CLIMB3;
			break;
		case -4:
			civvy->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + CIVVY_FALL3_ANIM;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = CIVVY_FALL3;
			break;
		}
	}
	else
	{
		civvy->maximumTurn = 0;
		CreatureAnimation(item_number, angle, 0);
	}
}

