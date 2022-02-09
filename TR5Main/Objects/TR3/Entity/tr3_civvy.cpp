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
	item->AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_STOP_ANIM;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->ActiveState = item->TargetState = CIVVY_STOP;
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
	civvy = (CREATURE_INFO*)item->Data;
	torso_y = torso_x = head = angle = tilt = 0;

	if (item->BoxNumber != NO_BOX && (g_Level.Boxes[item->BoxNumber].flags & BLOCKED))
	{
		DoLotsOfBlood(item->Position.xPos, item->Position.yPos - (GetRandomControl() & 255) - 32, item->Position.zPos, (GetRandomControl() & 127) + 128, GetRandomControl() << 1, item->RoomNumber, 3);
		item->HitPoints -= 20;
	}

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != CIVVY_DEATH)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_DIE_ANIM;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = CIVVY_DEATH;
			civvy->LOT.step = STEP_SIZE;
		}
	}
	else
	{
		if (item->AIBits)
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
			lara_dz = LaraItem->Position.zPos - item->Position.zPos;
			lara_dx = LaraItem->Position.xPos - item->Position.xPos;
			lara_info.angle = phd_atan(lara_dz, lara_dx) - item->Position.yRot;
			lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
		}

		GetCreatureMood(item, &info, VIOLENT);

		if (civvy->enemy == LaraItem && info.distance > CIVVY_ESCAPE_RANGE&& info.enemyFacing < 0x3000 && info.enemyFacing > -0x3000)
			civvy->mood = ESCAPE_MOOD;

		CreatureMood(item, &info, VIOLENT);


		angle = CreatureTurn(item, civvy->maximumTurn);

		real_enemy = civvy->enemy;
		civvy->enemy = LaraItem;

		if ((lara_info.distance < CIVVY_AWARE_DISTANCE || item->HitStatus || TargetVisible(item, &lara_info)) && !(item->AIBits & FOLLOW))
		{
			if (!civvy->alerted)
				SoundEffect(300, &item->Position, 0);
			AlertAllGuards(item_number);
		}
		civvy->enemy = real_enemy;

		switch (item->ActiveState)
		{
		case CIVVY_WAIT:
			if (civvy->alerted || item->TargetState == CIVVY_RUN)
			{
				item->TargetState = CIVVY_STOP;
				break;
			}

		case CIVVY_STOP:
			civvy->flags = 0;
			civvy->maximumTurn = 0;
			head = lara_info.angle;

			if (item->AIBits & GUARD)
			{
				head = AIGuard(civvy);
				if (!(GetRandomControl() & 0xFF))
				{
					if (item->ActiveState == CIVVY_STOP)
						item->TargetState = CIVVY_WAIT;
					else
						item->TargetState = CIVVY_STOP;
				}
				break;
			}

			else if (item->AIBits & PATROL1)
				item->TargetState = CIVVY_WALK;

			else if (civvy->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead)
					item->TargetState = CIVVY_STOP;
				else
					item->TargetState = CIVVY_RUN;
			}
			else if (civvy->mood == BORED_MOOD || ((item->AIBits & FOLLOW) && (civvy->reachedGoal || lara_info.distance > SQUARE(WALL_SIZE * 2))))
			{
				if (item->RequiredState)
					item->TargetState = item->RequiredState;
				else if (info.ahead)
					item->TargetState = CIVVY_STOP;
				else
					item->TargetState = CIVVY_RUN;
			}
			else if (info.bite && info.distance < CIVVY_ATTACK0_RANGE)
				item->TargetState = CIVVY_AIM0;
			else if (info.bite && info.distance < CIVVY_ATTACK1_RANGE)
				item->TargetState = CIVVY_AIM1;
			else if (info.bite && info.distance < CIVVY_WALK_RANGE)
				item->TargetState = CIVVY_WALK;
			else
				item->TargetState = CIVVY_RUN;
			break;

		case CIVVY_WALK:
			head = lara_info.angle;

			civvy->maximumTurn = CIVVY_WALK_TURN;

			if (item->AIBits & PATROL1)
			{
				item->TargetState = CIVVY_WALK;
				head = 0;
			}
			else if (civvy->mood == ESCAPE_MOOD)
				item->TargetState = CIVVY_RUN;
			else if (civvy->mood == BORED_MOOD)
			{
				if (GetRandomControl() < CIVVY_WAIT_CHANCE)
				{
					item->RequiredState = CIVVY_WAIT;
					item->TargetState = CIVVY_STOP;
				}
			}
			else if (info.bite && info.distance < CIVVY_ATTACK0_RANGE)
				item->TargetState = CIVVY_STOP;
			else if (info.bite && info.distance < CIVVY_ATTACK2_RANGE)
				item->TargetState = CIVVY_AIM2;
			else
				item->TargetState = CIVVY_RUN;
			break;

		case CIVVY_RUN:
			if (info.ahead)
				head = info.angle;

			civvy->maximumTurn = CIVVY_RUN_TURN;
			tilt = angle / 2;

			if (item->AIBits & GUARD)
				item->TargetState = CIVVY_WAIT;
			else if (civvy->mood == ESCAPE_MOOD)
			{
				if (Lara.target != item && info.ahead)
					item->TargetState = CIVVY_STOP;
				break;
			}
			else if ((item->AIBits & FOLLOW) && (civvy->reachedGoal || lara_info.distance > SQUARE(WALL_SIZE * 2)))
				item->TargetState = CIVVY_STOP;
			else if (civvy->mood == BORED_MOOD)
				item->TargetState = CIVVY_WALK;
			else if (info.ahead && info.distance < CIVVY_WALK_RANGE)
				item->TargetState = CIVVY_WALK;
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
				item->TargetState = CIVVY_PUNCH0;
			else
				item->TargetState = CIVVY_STOP;
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
				item->TargetState = CIVVY_PUNCH1;
			else
				item->TargetState = CIVVY_STOP;
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
				item->TargetState = CIVVY_PUNCH2;
			else
				item->TargetState = CIVVY_WALK;
			break;

		case CIVVY_PUNCH0:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}
			civvy->maximumTurn = CIVVY_WALK_TURN;

			if (!civvy->flags && (item->TouchBits & CIVVY_TOUCH))
			{
				LaraItem->HitPoints -= CIVVY_HIT_DAMAGE;
				LaraItem->HitStatus = true;
				CreatureEffect(item, &civvy_hit, DoBloodSplat);
				SoundEffect(70, &item->Position, 0);

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

			if (!civvy->flags && (item->TouchBits & CIVVY_TOUCH))
			{
				LaraItem->HitPoints -= CIVVY_HIT_DAMAGE;
				LaraItem->HitStatus = true;
				CreatureEffect(item, &civvy_hit, DoBloodSplat);
				SoundEffect(70, &item->Position, 0);

				civvy->flags = 1;
			}

			if (info.ahead && info.distance > CIVVY_ATTACK1_RANGE&& info.distance < CIVVY_ATTACK2_RANGE)
				item->TargetState = CIVVY_PUNCH2;
			break;

		case CIVVY_PUNCH2:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}
			civvy->maximumTurn = CIVVY_WALK_TURN;

			if (civvy->flags != 2 && (item->TouchBits & CIVVY_TOUCH))
			{
				LaraItem->HitPoints -= CIVVY_SWIPE_DAMAGE;
				LaraItem->HitStatus = true;
				CreatureEffect(item, &civvy_hit, DoBloodSplat);
				SoundEffect(70, &item->Position, 0);

				civvy->flags = 2;
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head);

	if (item->ActiveState < CIVVY_DEATH)
	{
		switch (CreatureVault(item_number, angle, 2, CIVVY_VAULT_SHIFT))
		{
		case 2:
			civvy->maximumTurn = 0;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_CLIMB1_ANIM;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = CIVVY_CLIMB1;
			break;

		case 3:
			civvy->maximumTurn = 0;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_CLIMB2_ANIM;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = CIVVY_CLIMB2;
			break;

		case 4:
			civvy->maximumTurn = 0;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_CLIMB3_ANIM;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = CIVVY_CLIMB3;
			break;
		case -4:
			civvy->maximumTurn = 0;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_FALL3_ANIM;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = CIVVY_FALL3;
			break;
		}
	}
	else
	{
		civvy->maximumTurn = 0;
		CreatureAnimation(item_number, angle, 0);
	}
}

