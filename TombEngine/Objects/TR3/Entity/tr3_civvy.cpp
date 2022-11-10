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
#include "Math/Math.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto CIVVY_ATTACK_DAMAGE = 40;
	constexpr auto CIVVY_SWIPE_DAMAGE  = 50;

	constexpr auto CIVVY_ATTACK0_RANGE = SQUARE(BLOCK(3));
	constexpr auto CIVVY_ATTACK1_RANGE = SQUARE(BLOCK(0.67f));
	constexpr auto CIVVY_ATTACK2_RANGE = SQUARE(BLOCK(1));
	constexpr auto CIVVY_WALK_RANGE	   = SQUARE(BLOCK(1));
	constexpr auto CIVVY_ESCAPE_RANGE  = SQUARE(BLOCK(3));
	constexpr auto CIVVY_AWARE_RANGE   = SQUARE(BLOCK(1));

	constexpr auto CIVVY_WAIT_CHANCE	   = 1.0f / 128;
	constexpr auto CIVVY_STATE_WALK_CHANCE = 1.0f / 128; // Unused.

	constexpr auto CIVVY_VAULT_SHIFT = 260;

	constexpr auto CIVVY_WALK_TURN_RATE_MAX = ANGLE(5.0f);
	constexpr auto CIVVY_RUN_TURN_RATE_MAX	= ANGLE(6.0f);

	const auto CivvyBite = BiteInfo(Vector3::Zero, 13);
	const auto CivvyAttackJoints = std::vector<unsigned int>{ 10, 13 };

	// TODO
	enum CivvyState
	{
		// No state 0.
		CIVVY_STATE_IDLE = 1,
		CIVVY_STATE_WALK_FORWARD = 2,
		CIVVY_PUNCH2 = 3,
		CIVVY_AIM2 = 4,
		CIVVY_WAIT = 5,
		CIVVY_AIM1 = 6,
		CIVVY_AIM0 = 7,
		CIVVY_PUNCH1 = 8,
		CIVVY_PUNCH0 = 9,
		CIVVY_STATE_RUN_FORWARD = 10,
		CIVVY_DEATH = 11,
		CIVVY_CLIMB3 = 12,
		CIVVY_CLIMB1 = 13,
		CIVVY_CLIMB2 = 14,
		CIVVY_FALL3 = 15
	};

	// TODO
	enum CivvyAnim
	{

		CIVVY_ANIM_IDLE = 6,

		CIVVY_ANIM_DEATH = 26,
		CIVVY_CLIMB3_ANIM = 27,
		CIVVY_CLIMB1_ANIM = 28,
		CIVVY_CLIMB2_ANIM = 29,
		CIVVY_FALL3_ANIM = 30
	};

	void InitialiseCivvy(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);
		SetAnimation(item, CIVVY_ANIM_IDLE);
	}

	void CivvyControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short tilt = 0;
		auto extraHeadRot = EulerAngles::Zero;
		auto extraTorsoRot = EulerAngles::Zero;

		if (item->BoxNumber != NO_BOX && (g_Level.Boxes[item->BoxNumber].flags & BLOCKED))
		{
			DoDamage(item, 20);
			DoLotsOfBlood(item->Pose.Position.x, item->Pose.Position.y - (GetRandomControl() & 255) - 32, item->Pose.Position.z, (GetRandomControl() & 127) + 128, GetRandomControl() << 1, item->RoomNumber, 3);
		}

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != CIVVY_DEATH)
			{
				SetAnimation(item, CIVVY_ANIM_DEATH);
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

			AI_INFO laraAI;
			if (creature->Enemy == LaraItem)
			{
				laraAI.angle = AI.angle;
				laraAI.distance = AI.distance;
			}
			else
			{
				int laraDz = LaraItem->Pose.Position.z - item->Pose.Position.z;
				int laraDx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				laraAI.angle = phd_atan(laraDz, laraDx) - item->Pose.Orientation.y;
				laraAI.distance = pow(laraDx, 2) + pow(laraDz, 2);
			}

			GetCreatureMood(item, &AI, true);

			if (creature->Enemy == LaraItem &&
				AI.distance > CIVVY_ESCAPE_RANGE &&
				AI.enemyFacing < ANGLE(67.0f) &&
				AI.enemyFacing > -ANGLE(67.0f))
			{
				creature->Mood = MoodType::Escape;
			}

			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);

			auto* realEnemy = creature->Enemy;
			creature->Enemy = LaraItem;

			if ((laraAI.distance < CIVVY_AWARE_RANGE || item->HitStatus || TargetVisible(item, &laraAI)) &&
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
				if (creature->Alerted || item->Animation.TargetState == CIVVY_STATE_RUN_FORWARD)
				{
					item->Animation.TargetState = CIVVY_STATE_IDLE;
					break;
				}

			case CIVVY_STATE_IDLE:
				creature->MaxTurn = 0;
				creature->Flags = 0;
				extraHeadRot.y = laraAI.angle;

				if (item->AIBits & GUARD)
				{
					extraHeadRot.y = AIGuard(creature);
					if (!(GetRandomControl() & 0xFF))
					{
						if (item->Animation.ActiveState == CIVVY_STATE_IDLE)
							item->Animation.TargetState = CIVVY_WAIT;
						else
							item->Animation.TargetState = CIVVY_STATE_IDLE;
					}

					break;
				}

				else if (item->AIBits & PATROL1)
					item->Animation.TargetState = CIVVY_STATE_WALK_FORWARD;

				else if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead)
						item->Animation.TargetState = CIVVY_STATE_IDLE;
					else
						item->Animation.TargetState = CIVVY_STATE_RUN_FORWARD;
				}
				else if (creature->Mood == MoodType::Bored ||
					(item->AIBits & FOLLOW && (creature->ReachedGoal || laraAI.distance > pow(SECTOR(2), 2))))
				{
					if (item->Animation.RequiredState != NO_STATE)
						item->Animation.TargetState = item->Animation.RequiredState;
					else if (AI.ahead)
						item->Animation.TargetState = CIVVY_STATE_IDLE;
					else
						item->Animation.TargetState = CIVVY_STATE_RUN_FORWARD;
				}
				else if (AI.bite && AI.distance < CIVVY_ATTACK0_RANGE)
					item->Animation.TargetState = CIVVY_AIM0;
				else if (AI.bite && AI.distance < CIVVY_ATTACK1_RANGE)
					item->Animation.TargetState = CIVVY_AIM1;
				else if (AI.bite && AI.distance < CIVVY_WALK_RANGE)
					item->Animation.TargetState = CIVVY_STATE_WALK_FORWARD;
				else
					item->Animation.TargetState = CIVVY_STATE_RUN_FORWARD;

				break;

			case CIVVY_STATE_WALK_FORWARD:
				creature->MaxTurn = CIVVY_WALK_TURN_RATE_MAX;
				extraHeadRot.y = laraAI.angle;

				if (item->AIBits & PATROL1)
				{
					extraHeadRot.y = 0;
					item->Animation.TargetState = CIVVY_STATE_WALK_FORWARD;
				}
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = CIVVY_STATE_RUN_FORWARD;
				else if (creature->Mood == MoodType::Bored)
				{
					if (Random::TestProbability(CIVVY_WAIT_CHANCE))
					{
						item->Animation.TargetState = CIVVY_STATE_IDLE;
						item->Animation.RequiredState = CIVVY_WAIT;
					}
				}
				else if (AI.bite && AI.distance < CIVVY_ATTACK0_RANGE)
					item->Animation.TargetState = CIVVY_STATE_IDLE;
				else if (AI.bite && AI.distance < CIVVY_ATTACK2_RANGE)
					item->Animation.TargetState = CIVVY_AIM2;
				else
					item->Animation.TargetState = CIVVY_STATE_RUN_FORWARD;

				break;

			case CIVVY_STATE_RUN_FORWARD:
				creature->MaxTurn = CIVVY_RUN_TURN_RATE_MAX;
				tilt = angle / 2;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (item->AIBits & GUARD)
					item->Animation.TargetState = CIVVY_WAIT;
				else if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead)
						item->Animation.TargetState = CIVVY_STATE_IDLE;
					break;
				}
				else if ((item->AIBits & FOLLOW) && (creature->ReachedGoal || laraAI.distance > pow(SECTOR(2), 2)))
					item->Animation.TargetState = CIVVY_STATE_IDLE;
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = CIVVY_STATE_WALK_FORWARD;
				else if (AI.ahead && AI.distance < CIVVY_WALK_RANGE)
					item->Animation.TargetState = CIVVY_STATE_WALK_FORWARD;

				break;

			case CIVVY_AIM0:
				creature->MaxTurn = CIVVY_WALK_TURN_RATE_MAX;
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (AI.bite && AI.distance < CIVVY_ATTACK0_RANGE)
					item->Animation.TargetState = CIVVY_PUNCH0;
				else
					item->Animation.TargetState = CIVVY_STATE_IDLE;

				break;

			case CIVVY_AIM1:
				creature->MaxTurn = CIVVY_WALK_TURN_RATE_MAX;
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (AI.ahead && AI.distance < CIVVY_ATTACK1_RANGE)
					item->Animation.TargetState = CIVVY_PUNCH1;
				else
					item->Animation.TargetState = CIVVY_STATE_IDLE;

				break;

			case CIVVY_AIM2:
				creature->MaxTurn = CIVVY_WALK_TURN_RATE_MAX;
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (AI.bite && AI.distance < CIVVY_ATTACK2_RANGE)
					item->Animation.TargetState = CIVVY_PUNCH2;
				else
					item->Animation.TargetState = CIVVY_STATE_WALK_FORWARD;

				break;

			case CIVVY_PUNCH0:
				creature->MaxTurn = CIVVY_WALK_TURN_RATE_MAX;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (!creature->Flags && item->TouchBits.Test(CivvyAttackJoints))
				{
					CreatureEffect(item, CivvyBite, DoBloodSplat);
					DoDamage(creature->Enemy, CIVVY_ATTACK_DAMAGE);
					SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
					creature->Flags = 1;
				}

				break;

			case CIVVY_PUNCH1:
				creature->MaxTurn = CIVVY_WALK_TURN_RATE_MAX;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (!creature->Flags && item->TouchBits.Test(CivvyAttackJoints))
				{
					CreatureEffect(item, CivvyBite, DoBloodSplat);
					DoDamage(creature->Enemy, CIVVY_ATTACK_DAMAGE);
					SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
					creature->Flags = 1;
				}

				if (AI.ahead && AI.distance > CIVVY_ATTACK1_RANGE && AI.distance < CIVVY_ATTACK2_RANGE)
					item->Animation.TargetState = CIVVY_PUNCH2;

				break;

			case CIVVY_PUNCH2:
				creature->MaxTurn = CIVVY_WALK_TURN_RATE_MAX;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (creature->Flags != 2 && item->TouchBits.Test(CivvyAttackJoints))
				{
					DoDamage(creature->Enemy, CIVVY_SWIPE_DAMAGE);
					CreatureEffect(item, CivvyBite, DoBloodSplat);
					SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
					creature->Flags = 2;
				}

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, extraTorsoRot.y);
		CreatureJoint(item, 1, extraTorsoRot.x);
		CreatureJoint(item, 2, extraHeadRot.y);

		if (item->Animation.ActiveState < CIVVY_DEATH)
		{
			switch (CreatureVault(itemNumber, angle, 2, CIVVY_VAULT_SHIFT))
			{
			case 2:
				SetAnimation(item, CIVVY_CLIMB1_ANIM);
				creature->MaxTurn = 0;
				break;

			case 3:
				SetAnimation(item, CIVVY_CLIMB2_ANIM);
				creature->MaxTurn = 0;
				break;

			case 4:
				SetAnimation(item, CIVVY_CLIMB3_ANIM);
				creature->MaxTurn = 0;
				break;

			case -4:
				SetAnimation(item, CIVVY_FALL3_ANIM);
				creature->MaxTurn = 0;
				break;
			}
		}
		else
		{
			creature->MaxTurn = 0;
			CreatureAnimation(itemNumber, angle, 0);
		}
	}
}
