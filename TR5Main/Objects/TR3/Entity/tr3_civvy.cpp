#include "framework.h"
#include "Objects/TR3/Entity/tr3_civvy.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO CivvyBite = { 0, 0, 0, 13 };

enum CivvyState
{
	CIVVY_EMPTY,
	CIVVY_STOP,
	CIVVY_WALK,
	CIVVY_PUNCH2,
	CIVVY_AIM2,
	CIVVY_WAIT,
	CIVVY_AIM1,
	CIVVY_AIM0,
	CIVVY_PUNCH1,
	CIVVY_PUNCH0,
	CIVVY_RUN,
	CIVVY_DEATH,
	CIVVY_CLIMB3,
	CIVVY_CLIMB1,
	CIVVY_CLIMB2,
	CIVVY_FALL3
};

// TODO
enum CivvyAnim
{

};

#define CIVVY_WALK_TURN ANGLE(5)
#define CIVVY_RUN_TURN ANGLE(6)
#define CIVVY_HIT_DAMAGE 40
#define CIVVY_SWIPE_DAMAGE 50
#define CIVVY_ATTACK0_RANGE pow(SECTOR(3), 2)
#define CIVVY_ATTACK1_RANGE pow(WALL_SIZE*2/3, 2)
#define CIVVY_ATTACK2_RANGE pow(SECTOR(1), 2)
#define CIVVY_WALK_RANGE pow(SECTOR(1), 2)
#define CIVVY_ESCAPE_RANGE pow(SECTOR(3), 2)
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
#define CIVVY_AWARE_DISTANCE pow(SECTOR(1), 2)

void InitialiseCivvy(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	InitialiseCreature(itemNumber);

	item->AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_STOP_ANIM;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->ActiveState = item->TargetState = CIVVY_STOP;
}

void CivvyControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	short torsoX = 0;
	short torsoY = 0;
	short head = 0;
	short angle = 0;
	short tilt = 0;

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
			info->LOT.step = CLICK(1);
		}
	}
	else
	{
		if (item->AIBits)
			GetAITarget(info);
		else
			info->enemy = LaraItem;

		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		AI_INFO laraAiInfo;
		if (info->enemy == LaraItem)
		{
			laraAiInfo.angle = aiInfo.angle;
			laraAiInfo.distance = aiInfo.distance;
		}
		else
		{
			int laraDz = LaraItem->Position.zPos - item->Position.zPos;
			int laraDx = LaraItem->Position.xPos - item->Position.xPos;
			laraAiInfo.angle = phd_atan(laraDz, laraDx) - item->Position.yRot;
			laraAiInfo.distance = pow(laraDx, 2) + pow(laraDz, 2);
		}

		GetCreatureMood(item, &aiInfo, VIOLENT);

		if (info->enemy == LaraItem &&
			aiInfo.distance > CIVVY_ESCAPE_RANGE &&
			aiInfo.enemyFacing < 0x3000 &&
			aiInfo.enemyFacing > -0x3000)
		{
			info->mood = MoodType::Escape;
		}

		CreatureMood(item, &aiInfo, VIOLENT);

		angle = CreatureTurn(item, info->maximumTurn);

		auto* realEnemy = info->enemy;
		info->enemy = LaraItem;

		if ((laraAiInfo.distance < CIVVY_AWARE_DISTANCE || item->HitStatus || TargetVisible(item, &laraAiInfo)) &&
			!(item->AIBits & FOLLOW))
		{
			if (!info->alerted)
				SoundEffect(300, &item->Position, 0);
			AlertAllGuards(itemNumber);
		}
		info->enemy = realEnemy;

		switch (item->ActiveState)
		{
		case CIVVY_WAIT:
			if (info->alerted || item->TargetState == CIVVY_RUN)
			{
				item->TargetState = CIVVY_STOP;
				break;
			}

		case CIVVY_STOP:
			info->flags = 0;
			info->maximumTurn = 0;
			head = laraAiInfo.angle;

			if (item->AIBits & GUARD)
			{
				head = AIGuard(info);
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

			else if (info->mood == MoodType::Escape)
			{
				if (Lara.target != item && aiInfo.ahead)
					item->TargetState = CIVVY_STOP;
				else
					item->TargetState = CIVVY_RUN;
			}
			else if (info->mood == MoodType::Bored ||
				(item->AIBits & FOLLOW && (info->reachedGoal || laraAiInfo.distance > pow(SECTOR(2), 2))))
			{
				if (item->RequiredState)
					item->TargetState = item->RequiredState;
				else if (aiInfo.ahead)
					item->TargetState = CIVVY_STOP;
				else
					item->TargetState = CIVVY_RUN;
			}
			else if (aiInfo.bite && aiInfo.distance < CIVVY_ATTACK0_RANGE)
				item->TargetState = CIVVY_AIM0;
			else if (aiInfo.bite && aiInfo.distance < CIVVY_ATTACK1_RANGE)
				item->TargetState = CIVVY_AIM1;
			else if (aiInfo.bite && aiInfo.distance < CIVVY_WALK_RANGE)
				item->TargetState = CIVVY_WALK;
			else
				item->TargetState = CIVVY_RUN;

			break;

		case CIVVY_WALK:
			info->maximumTurn = CIVVY_WALK_TURN;
			head = laraAiInfo.angle;

			if (item->AIBits & PATROL1)
			{
				item->TargetState = CIVVY_WALK;
				head = 0;
			}
			else if (info->mood == MoodType::Escape)
				item->TargetState = CIVVY_RUN;
			else if (info->mood == MoodType::Bored)
			{
				if (GetRandomControl() < CIVVY_WAIT_CHANCE)
				{
					item->RequiredState = CIVVY_WAIT;
					item->TargetState = CIVVY_STOP;
				}
			}
			else if (aiInfo.bite && aiInfo.distance < CIVVY_ATTACK0_RANGE)
				item->TargetState = CIVVY_STOP;
			else if (aiInfo.bite && aiInfo.distance < CIVVY_ATTACK2_RANGE)
				item->TargetState = CIVVY_AIM2;
			else
				item->TargetState = CIVVY_RUN;

			break;

		case CIVVY_RUN:
			info->maximumTurn = CIVVY_RUN_TURN;
			tilt = angle / 2;

			if (aiInfo.ahead)
				head = aiInfo.angle;

			if (item->AIBits & GUARD)
				item->TargetState = CIVVY_WAIT;
			else if (info->mood == MoodType::Escape)
			{
				if (Lara.target != item && aiInfo.ahead)
					item->TargetState = CIVVY_STOP;
				break;
			}
			else if ((item->AIBits & FOLLOW) && (info->reachedGoal || laraAiInfo.distance > pow(SECTOR(2), 2)))
				item->TargetState = CIVVY_STOP;
			else if (info->mood == MoodType::Bored)
				item->TargetState = CIVVY_WALK;
			else if (aiInfo.ahead && aiInfo.distance < CIVVY_WALK_RANGE)
				item->TargetState = CIVVY_WALK;

			break;

		case CIVVY_AIM0:
			info->maximumTurn = CIVVY_WALK_TURN;

			if (aiInfo.ahead)
			{
				torsoX = aiInfo.xAngle;
				torsoY = aiInfo.angle;
			}

			if (aiInfo.bite && aiInfo.distance < CIVVY_ATTACK0_RANGE)
				item->TargetState = CIVVY_PUNCH0;
			else
				item->TargetState = CIVVY_STOP;

			info->flags = 0;
			break;

		case CIVVY_AIM1:
			info->maximumTurn = CIVVY_WALK_TURN;

			if (aiInfo.ahead)
			{
				torsoX = aiInfo.xAngle;
				torsoY = aiInfo.angle;
			}

			if (aiInfo.ahead && aiInfo.distance < CIVVY_ATTACK1_RANGE)
				item->TargetState = CIVVY_PUNCH1;
			else
				item->TargetState = CIVVY_STOP;

			info->flags = 0;
			break;

		case CIVVY_AIM2:
			info->maximumTurn = CIVVY_WALK_TURN;
			info->flags = 0;

			if (aiInfo.ahead)
			{
				torsoX = aiInfo.xAngle;
				torsoY = aiInfo.angle;
			}

			if (aiInfo.bite && aiInfo.distance < CIVVY_ATTACK2_RANGE)
				item->TargetState = CIVVY_PUNCH2;
			else
				item->TargetState = CIVVY_WALK;

			break;

		case CIVVY_PUNCH0:
			info->maximumTurn = CIVVY_WALK_TURN;

			if (aiInfo.ahead)
			{
				torsoX = aiInfo.xAngle;
				torsoY = aiInfo.angle;
			}

			if (!info->flags && (item->TouchBits & CIVVY_TOUCH))
			{
				CreatureEffect(item, &CivvyBite, DoBloodSplat);
				SoundEffect(70, &item->Position, 0);
				info->flags = 1;

				LaraItem->HitPoints -= CIVVY_HIT_DAMAGE;
				LaraItem->HitStatus = true;
			}

			break;

		case CIVVY_PUNCH1:
			info->maximumTurn = CIVVY_WALK_TURN;

			if (aiInfo.ahead)
			{
				torsoX = aiInfo.xAngle;
				torsoY = aiInfo.angle;
			}

			if (!info->flags && (item->TouchBits & CIVVY_TOUCH))
			{
				CreatureEffect(item, &CivvyBite, DoBloodSplat);
				SoundEffect(70, &item->Position, 0);
				info->flags = 1;

				LaraItem->HitPoints -= CIVVY_HIT_DAMAGE;
				LaraItem->HitStatus = true;
			}

			if (aiInfo.ahead && aiInfo.distance > CIVVY_ATTACK1_RANGE&& aiInfo.distance < CIVVY_ATTACK2_RANGE)
				item->TargetState = CIVVY_PUNCH2;

			break;

		case CIVVY_PUNCH2:
			info->maximumTurn = CIVVY_WALK_TURN;

			if (aiInfo.ahead)
			{
				torsoX = aiInfo.xAngle;
				torsoY = aiInfo.angle;
			}

			if (info->flags != 2 && (item->TouchBits & CIVVY_TOUCH))
			{
				CreatureEffect(item, &CivvyBite, DoBloodSplat);
				SoundEffect(70, &item->Position, 0);
				info->flags = 2;

				LaraItem->HitPoints -= CIVVY_SWIPE_DAMAGE;
				LaraItem->HitStatus = true;
			}

			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torsoY);
	CreatureJoint(item, 1, torsoX);
	CreatureJoint(item, 2, head);

	if (item->ActiveState < CIVVY_DEATH)
	{
		switch (CreatureVault(itemNumber, angle, 2, CIVVY_VAULT_SHIFT))
		{
		case 2:
			info->maximumTurn = 0;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_CLIMB1_ANIM;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = CIVVY_CLIMB1;
			break;

		case 3:
			info->maximumTurn = 0;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_CLIMB2_ANIM;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = CIVVY_CLIMB2;
			break;

		case 4:
			info->maximumTurn = 0;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_CLIMB3_ANIM;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = CIVVY_CLIMB3;
			break;

		case -4:
			info->maximumTurn = 0;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_FALL3_ANIM;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = CIVVY_FALL3;
			break;
		}
	}
	else
	{
		info->maximumTurn = 0;
		CreatureAnimation(itemNumber, angle, 0);
	}
}
