#include "framework.h"
#include "Objects/TR2/Entity/tr2_worker_flamethrower.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/missile.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Specific/setup.h"
#include "Specific/level.h"

namespace TEN::Entities::Creatures::TR2
{
	constexpr auto WORKER_FLAME_ATTACK_RANGE = SQUARE(SECTOR(2));
	constexpr auto WORKER_FLAME_IDLE_RANGE	 = SQUARE(SECTOR(2));
	constexpr auto WORKER_FLAME_WALK_RANGE	 = SQUARE(SECTOR(2));
	constexpr auto WORKER_FLAME_RUN_RANGE	 = SQUARE(SECTOR(4));

	constexpr auto WORKER_FLAME_WALK_TURN_RATE_MAX = ANGLE(5.0f);
	constexpr auto WORKER_FLAME_RUN_TURN_RATE_MAX  = ANGLE(10.0f);

	const auto WorkerFlamethrowerOffset = Vector3i(0, 140, 0);
	const auto WorkerFlamethrowerBite = BiteInfo(Vector3(0.0f, 250.0f, 32.0f), 9);

	enum WorkerFlamethrowerState
	{
		// No state 0.
		WORKER_FLAME_STATE_IDLE = 1,
		WORKER_FLAME_STATE_WALK_FORWARD = 2,
		WORKER_FLAME_STATE_RUN = 3,
		WORKER_FLAME_STATE_WAIT = 4,
		WORKER_FLAME_STATE_ATTACK = 5,
		WORKER_FLAME_STATE_WALK_FORWARD_ATTACK = 6,
		WORKER_FLAME_STATE_DEATH = 7,
		WORKER_FLAME_STATE_AIM = 8,
		WORKER_FLAME_STATE_WALK_FORWARD_AIM = 9
	};

	// TODO: Fill out the rest.
	enum WorkerFlamethrowerAnim
	{
		WORKER_FLAME_ANIM_IDLE = 12,
		WORKER_FLAME_ANIM_DEATH = 19
	};

	void InitialiseWorkerFlamethrower(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);
		SetAnimation(item, WORKER_FLAME_ANIM_IDLE);
	}

	void WorkerFlamethrower(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short tilt = 0;
		auto extraHeadRot = EulerAngles::Zero;
		auto extraTorsoRot = EulerAngles::Zero;

		auto pos = GetJointPosition(item, WorkerFlamethrowerBite.meshNum, Vector3i(WorkerFlamethrowerBite.Position));

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != WORKER_FLAME_STATE_DEATH)
				SetAnimation(item, WORKER_FLAME_ANIM_DEATH);
		}
		else
		{
			if (item->Animation.ActiveState != WORKER_FLAME_STATE_ATTACK && item->Animation.ActiveState != WORKER_FLAME_STATE_WALK_FORWARD_ATTACK)
			{
				TriggerDynamicLight(pos.x, pos.y, pos.z, (GetRandomControl() & 4) + 10, (GetRandomControl() & 7) + 128, (GetRandomControl() & 7) + 64, GetRandomControl() & 7);
				TriggerPilotFlame(itemNumber, WorkerFlamethrowerBite.meshNum);
			}
			else
			{
				TriggerDynamicLight(pos.x, pos.y, pos.z, (GetRandomControl() & 4) + 14, (GetRandomControl() & 7) + 128, (GetRandomControl() & 7) + 64, GetRandomControl() & 7);
				ThrowFire(itemNumber, WorkerFlamethrowerBite.meshNum, WorkerFlamethrowerOffset, WorkerFlamethrowerOffset);
			}

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case WORKER_FLAME_STATE_IDLE:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = WORKER_FLAME_STATE_RUN;
				else if (Targetable(item, &AI))
				{
					if (AI.distance < WORKER_FLAME_ATTACK_RANGE || AI.zoneNumber != AI.enemyZone)
						item->Animation.TargetState = WORKER_FLAME_STATE_AIM;
					else
						item->Animation.TargetState = WORKER_FLAME_STATE_WALK_FORWARD;
				}
				else if (creature->Mood == MoodType::Attack || !AI.ahead)
				{
					if (AI.distance <= WORKER_FLAME_WALK_RANGE)
						item->Animation.TargetState = WORKER_FLAME_STATE_WALK_FORWARD;
					else if (AI.distance >= WORKER_FLAME_RUN_RANGE)
						item->Animation.TargetState = WORKER_FLAME_STATE_RUN;
				}
				else
					item->Animation.TargetState = WORKER_FLAME_STATE_WAIT;

				break;

			case WORKER_FLAME_STATE_WALK_FORWARD:
				creature->MaxTurn = WORKER_FLAME_WALK_TURN_RATE_MAX;

				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = WORKER_FLAME_STATE_RUN;
				else if (Targetable(item, &AI))
				{
					if (AI.distance < WORKER_FLAME_IDLE_RANGE || AI.zoneNumber != AI.enemyZone)
						item->Animation.TargetState = WORKER_FLAME_STATE_IDLE;
					else if (AI.distance < WORKER_FLAME_ATTACK_RANGE)
						item->Animation.TargetState = WORKER_FLAME_STATE_WALK_FORWARD_AIM;
				}
				else if (creature->Mood == MoodType::Attack || !AI.ahead)
				{
					if (AI.distance > WORKER_FLAME_WALK_RANGE)
						item->Animation.TargetState = WORKER_FLAME_STATE_RUN;
				}
				else
					item->Animation.TargetState = WORKER_FLAME_STATE_WAIT;

				break;

			case WORKER_FLAME_STATE_RUN:
				creature->MaxTurn = WORKER_FLAME_RUN_TURN_RATE_MAX;

				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (creature->Mood != MoodType::Escape)
				{
					if (Targetable(item, &AI))
						item->Animation.TargetState = WORKER_FLAME_STATE_WALK_FORWARD;
					else if (creature->Mood == MoodType::Bored || creature->Mood == MoodType::Stalk)
						item->Animation.TargetState = WORKER_FLAME_STATE_WALK_FORWARD;
				}

				break;

			case 4:
				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (Targetable(item, &AI))
					item->Animation.TargetState = WORKER_FLAME_STATE_ATTACK;
				else
				{
					if (creature->Mood == MoodType::Attack)
						item->Animation.TargetState = WORKER_FLAME_STATE_IDLE;
					else if (!AI.ahead)
						item->Animation.TargetState = WORKER_FLAME_STATE_IDLE;
				}

				break;

			case WORKER_FLAME_STATE_ATTACK:
			case WORKER_FLAME_STATE_WALK_FORWARD_ATTACK:
				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (item->Animation.TargetState != WORKER_FLAME_STATE_IDLE &&
					(!Targetable(item, &AI) || creature->Mood == MoodType::Escape || AI.distance > WORKER_FLAME_ATTACK_RANGE))
				{
					item->Animation.TargetState = WORKER_FLAME_STATE_IDLE;
				}

				break;

			case WORKER_FLAME_STATE_AIM:
			case WORKER_FLAME_STATE_WALK_FORWARD_AIM:
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (Targetable(item, &AI) && AI.distance <= WORKER_FLAME_ATTACK_RANGE)
					item->Animation.TargetState = (item->Animation.ActiveState == WORKER_FLAME_STATE_AIM) ? WORKER_FLAME_STATE_ATTACK : WORKER_FLAME_STATE_WALK_FORWARD_ATTACK;
				else
					item->Animation.TargetState = WORKER_FLAME_STATE_IDLE;

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, extraTorsoRot.y);
		CreatureJoint(item, 1, extraTorsoRot.x);
		CreatureJoint(item, 2, extraHeadRot.y);
		CreatureJoint(item, 3, extraHeadRot.x);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
