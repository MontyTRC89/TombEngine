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

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_STOP_ANIM;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.ActiveState = item->Animation.TargetState = CIVVY_STOP;
}

void CivvyControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short torsoX = 0;
	short torsoY = 0;
	short head = 0;
	short angle = 0;
	short tilt = 0;

	if (item->BoxNumber != NO_BOX && (g_Level.Boxes[item->BoxNumber].flags & BLOCKED))
	{
		DoLotsOfBlood(item->Pose.Position.x, item->Pose.Position.y - (GetRandomControl() & 255) - 32, item->Pose.Position.z, (GetRandomControl() & 127) + 128, GetRandomControl() << 1, item->RoomNumber, 3);
		item->HitPoints -= 20;
	}

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != CIVVY_DEATH)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_DIE_ANIM;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = CIVVY_DEATH;
			creature->LOT.Step = CLICK(1);
		}
	}
	else
	{
		if (item->AIBits)
			GetAITarget(creature);
		else
			creature->Enemy = LaraItem;

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		AI_INFO laraAiInfo;
		if (creature->Enemy == LaraItem)
		{
			laraAiInfo.angle = AI.angle;
			laraAiInfo.distance = AI.distance;
		}
		else
		{
			int laraDz = LaraItem->Pose.Position.z - item->Pose.Position.z;
			int laraDx = LaraItem->Pose.Position.x - item->Pose.Position.x;
			laraAiInfo.angle = phd_atan(laraDz, laraDx) - item->Pose.Orientation.y;
			laraAiInfo.distance = pow(laraDx, 2) + pow(laraDz, 2);
		}

		GetCreatureMood(item, &AI, VIOLENT);

		if (creature->Enemy == LaraItem &&
			AI.distance > CIVVY_ESCAPE_RANGE &&
			AI.enemyFacing < 0x3000 &&
			AI.enemyFacing > -0x3000)
		{
			creature->Mood = MoodType::Escape;
		}

		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);

		auto* realEnemy = creature->Enemy;
		creature->Enemy = LaraItem;

		if ((laraAiInfo.distance < CIVVY_AWARE_DISTANCE || item->HitStatus || TargetVisible(item, &laraAiInfo)) &&
			!(item->AIBits & FOLLOW))
		{
			if (!creature->Alerted)
				SoundEffect(SFX_TR3_AMERCAN_HOY, &item->Pose);
			AlertAllGuards(itemNumber);
		}
		creature->Enemy = realEnemy;

		switch (item->Animation.ActiveState)
		{
		case CIVVY_WAIT:
			if (creature->Alerted || item->Animation.TargetState == CIVVY_RUN)
			{
				item->Animation.TargetState = CIVVY_STOP;
				break;
			}

		case CIVVY_STOP:
			creature->Flags = 0;
			creature->MaxTurn = 0;
			head = laraAiInfo.angle;

			if (item->AIBits & GUARD)
			{
				head = AIGuard(creature);
				if (!(GetRandomControl() & 0xFF))
				{
					if (item->Animation.ActiveState == CIVVY_STOP)
						item->Animation.TargetState = CIVVY_WAIT;
					else
						item->Animation.TargetState = CIVVY_STOP;
				}

				break;
			}

			else if (item->AIBits & PATROL1)
				item->Animation.TargetState = CIVVY_WALK;

			else if (creature->Mood == MoodType::Escape)
			{
				if (Lara.TargetEntity != item && AI.ahead)
					item->Animation.TargetState = CIVVY_STOP;
				else
					item->Animation.TargetState = CIVVY_RUN;
			}
			else if (creature->Mood == MoodType::Bored ||
				(item->AIBits & FOLLOW && (creature->ReachedGoal || laraAiInfo.distance > pow(SECTOR(2), 2))))
			{
				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (AI.ahead)
					item->Animation.TargetState = CIVVY_STOP;
				else
					item->Animation.TargetState = CIVVY_RUN;
			}
			else if (AI.bite && AI.distance < CIVVY_ATTACK0_RANGE)
				item->Animation.TargetState = CIVVY_AIM0;
			else if (AI.bite && AI.distance < CIVVY_ATTACK1_RANGE)
				item->Animation.TargetState = CIVVY_AIM1;
			else if (AI.bite && AI.distance < CIVVY_WALK_RANGE)
				item->Animation.TargetState = CIVVY_WALK;
			else
				item->Animation.TargetState = CIVVY_RUN;

			break;

		case CIVVY_WALK:
			creature->MaxTurn = CIVVY_WALK_TURN;
			head = laraAiInfo.angle;

			if (item->AIBits & PATROL1)
			{
				item->Animation.TargetState = CIVVY_WALK;
				head = 0;
			}
			else if (creature->Mood == MoodType::Escape)
				item->Animation.TargetState = CIVVY_RUN;
			else if (creature->Mood == MoodType::Bored)
			{
				if (GetRandomControl() < CIVVY_WAIT_CHANCE)
				{
					item->Animation.RequiredState = CIVVY_WAIT;
					item->Animation.TargetState = CIVVY_STOP;
				}
			}
			else if (AI.bite && AI.distance < CIVVY_ATTACK0_RANGE)
				item->Animation.TargetState = CIVVY_STOP;
			else if (AI.bite && AI.distance < CIVVY_ATTACK2_RANGE)
				item->Animation.TargetState = CIVVY_AIM2;
			else
				item->Animation.TargetState = CIVVY_RUN;

			break;

		case CIVVY_RUN:
			creature->MaxTurn = CIVVY_RUN_TURN;
			tilt = angle / 2;

			if (AI.ahead)
				head = AI.angle;

			if (item->AIBits & GUARD)
				item->Animation.TargetState = CIVVY_WAIT;
			else if (creature->Mood == MoodType::Escape)
			{
				if (Lara.TargetEntity != item && AI.ahead)
					item->Animation.TargetState = CIVVY_STOP;
				break;
			}
			else if ((item->AIBits & FOLLOW) && (creature->ReachedGoal || laraAiInfo.distance > pow(SECTOR(2), 2)))
				item->Animation.TargetState = CIVVY_STOP;
			else if (creature->Mood == MoodType::Bored)
				item->Animation.TargetState = CIVVY_WALK;
			else if (AI.ahead && AI.distance < CIVVY_WALK_RANGE)
				item->Animation.TargetState = CIVVY_WALK;

			break;

		case CIVVY_AIM0:
			creature->MaxTurn = CIVVY_WALK_TURN;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			if (AI.bite && AI.distance < CIVVY_ATTACK0_RANGE)
				item->Animation.TargetState = CIVVY_PUNCH0;
			else
				item->Animation.TargetState = CIVVY_STOP;

			creature->Flags = 0;
			break;

		case CIVVY_AIM1:
			creature->MaxTurn = CIVVY_WALK_TURN;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			if (AI.ahead && AI.distance < CIVVY_ATTACK1_RANGE)
				item->Animation.TargetState = CIVVY_PUNCH1;
			else
				item->Animation.TargetState = CIVVY_STOP;

			creature->Flags = 0;
			break;

		case CIVVY_AIM2:
			creature->MaxTurn = CIVVY_WALK_TURN;
			creature->Flags = 0;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			if (AI.bite && AI.distance < CIVVY_ATTACK2_RANGE)
				item->Animation.TargetState = CIVVY_PUNCH2;
			else
				item->Animation.TargetState = CIVVY_WALK;

			break;

		case CIVVY_PUNCH0:
			creature->MaxTurn = CIVVY_WALK_TURN;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			if (!creature->Flags && (item->TouchBits & CIVVY_TOUCH))
			{
				CreatureEffect(item, &CivvyBite, DoBloodSplat);
				SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
				creature->Flags = 1;

				LaraItem->HitPoints -= CIVVY_HIT_DAMAGE;
				LaraItem->HitStatus = true;
			}

			break;

		case CIVVY_PUNCH1:
			creature->MaxTurn = CIVVY_WALK_TURN;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			if (!creature->Flags && (item->TouchBits & CIVVY_TOUCH))
			{
				CreatureEffect(item, &CivvyBite, DoBloodSplat);
				SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
				creature->Flags = 1;

				LaraItem->HitPoints -= CIVVY_HIT_DAMAGE;
				LaraItem->HitStatus = true;
			}

			if (AI.ahead && AI.distance > CIVVY_ATTACK1_RANGE&& AI.distance < CIVVY_ATTACK2_RANGE)
				item->Animation.TargetState = CIVVY_PUNCH2;

			break;

		case CIVVY_PUNCH2:
			creature->MaxTurn = CIVVY_WALK_TURN;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			if (creature->Flags != 2 && (item->TouchBits & CIVVY_TOUCH))
			{
				CreatureEffect(item, &CivvyBite, DoBloodSplat);
				SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
				creature->Flags = 2;

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

	if (item->Animation.ActiveState < CIVVY_DEATH)
	{
		switch (CreatureVault(itemNumber, angle, 2, CIVVY_VAULT_SHIFT))
		{
		case 2:
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_CLIMB1_ANIM;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = CIVVY_CLIMB1;
			break;

		case 3:
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_CLIMB2_ANIM;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = CIVVY_CLIMB2;
			break;

		case 4:
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_CLIMB3_ANIM;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = CIVVY_CLIMB3;
			break;

		case -4:
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_FALL3_ANIM;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = CIVVY_FALL3;
			break;
		}
	}
	else
	{
		creature->MaxTurn = 0;
		CreatureAnimation(itemNumber, angle, 0);
	}
}
