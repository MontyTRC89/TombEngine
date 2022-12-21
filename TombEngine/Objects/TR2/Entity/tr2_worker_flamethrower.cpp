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
#include "Math/Math.h"

namespace TEN::Entities::Creatures::TR2
{
	const auto WorkerFlamethrowerOffset = Vector3i(0, 140, 0);
	const auto WorkerFlamethrowerBite = BiteInfo(Vector3(0.0f, 250.0f, 32.0f), 9);

	constexpr int WorkerFlamethrowerAttackRange = SQUARE(SECTOR(2));
	constexpr int WorkerFlamethrowerStopRange = SQUARE(SECTOR(2));
	constexpr int WorkerFlamethrowerWalkRange = SQUARE(SECTOR(2));
	constexpr int WorkerFlamethrowerRunRange = SQUARE(SECTOR(4));
	constexpr int WorkerFlamethrowerWalkAngle = ANGLE(5.0f);
	constexpr int WorkerFlamethrowerRunAngle = ANGLE(10.0f);

	enum WorkerFlamethrowerState
	{
		// No state 0.
		WORKFLAME_STATE_STOP = 1,
		WORKFLAME_STATE_WALK = 2,
		WORKFLAME_STATE_RUN = 3,
		WORKFLAME_STATE_WAIT = 4,
		WORKFLAME_STATE_ATTACK = 5,
		WORKFLAME_STATE_WALK_ATTACK = 6,
		WORKFLAME_STATE_DEATH = 7,
		WORKFLAME_STATE_AIM = 8,
		WORKFLAME_STATE_WALK_AIM = 9,
	};

	enum WorkerFlamethrowerAnim
	{
		WORKFLAME_ANIM_IDLE = 12,
		WORKFLAME_ANIM_DEATH = 19
	};

	void InitialiseWorkerFlamethrower(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		InitialiseCreature(itemNumber);
		SetAnimation(item, WORKFLAME_ANIM_IDLE);
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
			if (item->Animation.ActiveState != WORKFLAME_STATE_DEATH)
				SetAnimation(item, WORKFLAME_ANIM_DEATH);
		}
		else
		{
			if (item->Animation.ActiveState != WORKFLAME_STATE_ATTACK && item->Animation.ActiveState != WORKFLAME_STATE_WALK_ATTACK)
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
			case WORKFLAME_STATE_STOP:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = WORKFLAME_STATE_RUN;
				else if (Targetable(item, &AI))
				{
					if (AI.distance < WorkerFlamethrowerAttackRange || AI.zoneNumber != AI.enemyZone)
						item->Animation.TargetState = WORKFLAME_STATE_AIM;
					else
						item->Animation.TargetState = WORKFLAME_STATE_WALK;
				}
				else if (creature->Mood == MoodType::Attack || !AI.ahead)
				{
					if (AI.distance <= WorkerFlamethrowerWalkRange)
						item->Animation.TargetState = WORKFLAME_STATE_WALK;
					else if (AI.distance >= WorkerFlamethrowerRunRange)
						item->Animation.TargetState = WORKFLAME_STATE_RUN;
				}
				else
					item->Animation.TargetState = WORKFLAME_STATE_WAIT;

				break;

			case WORKFLAME_STATE_WALK:
				creature->MaxTurn = WorkerFlamethrowerWalkAngle;

				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = WORKFLAME_STATE_RUN;
				else if (Targetable(item, &AI))
				{
					if (AI.distance < WorkerFlamethrowerStopRange || AI.zoneNumber != AI.enemyZone)
						item->Animation.TargetState = WORKFLAME_STATE_STOP;
					else if (AI.distance < WorkerFlamethrowerAttackRange)
						item->Animation.TargetState = WORKFLAME_STATE_WALK_AIM;
				}
				else if (creature->Mood == MoodType::Attack || !AI.ahead)
				{
					if (AI.distance > WorkerFlamethrowerWalkRange)
						item->Animation.TargetState = WORKFLAME_STATE_RUN;
				}
				else
					item->Animation.TargetState = WORKFLAME_STATE_WAIT;

				break;

			case WORKFLAME_STATE_RUN:
				creature->MaxTurn = WorkerFlamethrowerRunAngle;

				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (creature->Mood != MoodType::Escape)
				{
					if (Targetable(item, &AI))
						item->Animation.TargetState = WORKFLAME_STATE_WALK;
					else if (creature->Mood == MoodType::Bored || creature->Mood == MoodType::Stalk)
						item->Animation.TargetState = WORKFLAME_STATE_WALK;
				}

				break;

			case 4:
				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (Targetable(item, &AI))
					item->Animation.TargetState = WORKFLAME_STATE_ATTACK;
				else
				{
					if (creature->Mood == MoodType::Attack)
						item->Animation.TargetState = WORKFLAME_STATE_STOP;
					else if (!AI.ahead)
						item->Animation.TargetState = WORKFLAME_STATE_STOP;
				}

				break;

			case WORKFLAME_STATE_ATTACK:
			case WORKFLAME_STATE_WALK_ATTACK:
				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (item->Animation.TargetState != WORKFLAME_STATE_STOP &&
					(!Targetable(item, &AI) || creature->Mood == MoodType::Escape || AI.distance > WorkerFlamethrowerAttackRange))
				{
					item->Animation.TargetState = WORKFLAME_STATE_STOP;
				}

				break;

			case WORKFLAME_STATE_AIM:
			case WORKFLAME_STATE_WALK_AIM:
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (Targetable(item, &AI) && AI.distance <= WorkerFlamethrowerAttackRange)
					item->Animation.TargetState = (item->Animation.ActiveState == WORKFLAME_STATE_AIM) ? WORKFLAME_STATE_ATTACK : WORKFLAME_STATE_WALK_ATTACK;
				else
					item->Animation.TargetState = WORKFLAME_STATE_STOP;

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
