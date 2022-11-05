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
	const auto WorkerFlamethrowerOffset = Vector3i(0, 140, 0);
	const auto WorkerFlamethrowerBite = BiteInfo(Vector3(0.0f, 250.0f, 32.0f), 9);

	// TODO
	enum WorkerFlamethrowerState
	{

	};

	// TODO
	enum WorkerFlamethrowerAnim
	{

	};

	void InitialiseWorkerFlamethrower(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 12;

		ClearItem(itemNumber);

		auto* anim = &g_Level.Anims[item->Animation.AnimNumber];
		item->Animation.FrameNumber = anim->frameBase;
		item->Animation.ActiveState = anim->ActiveState;
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
			if (item->Animation.ActiveState != 7)
				SetAnimation(item, 19);
		}
		else
		{
			if (item->Animation.ActiveState != 5 && item->Animation.ActiveState != 6)
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
			case 1:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = 3;
				else if (Targetable(item, &AI))
				{
					if (AI.distance < pow(SECTOR(4), 2) || AI.zoneNumber != AI.enemyZone)
						item->Animation.TargetState = 8;
					else
						item->Animation.TargetState = 2;
				}
				else if (creature->Mood == MoodType::Attack || !AI.ahead)
				{
					if (AI.distance <= pow(SECTOR(2), 2))
						item->Animation.TargetState = 2;
					else
						item->Animation.TargetState = 3;
				}
				else
					item->Animation.TargetState = 4;

				break;

			case 2:
				creature->MaxTurn = ANGLE(5.0f);

				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = 3;
				else if (Targetable(item, &AI))
				{
					if (AI.distance < pow(SECTOR(4), 2) || AI.zoneNumber != AI.enemyZone)
						item->Animation.TargetState = 1;
					else
						item->Animation.TargetState = 6;
				}
				else if (creature->Mood == MoodType::Attack || !AI.ahead)
				{
					if (AI.distance > pow(SECTOR(2), 2))
						item->Animation.TargetState = 3;
				}
				else
					item->Animation.TargetState = 4;

				break;

			case 3:
				creature->MaxTurn = ANGLE(10.0f);

				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (creature->Mood != MoodType::Escape)
				{
					if (Targetable(item, &AI))
						item->Animation.TargetState = 2;
					else if (creature->Mood == MoodType::Bored || creature->Mood == MoodType::Stalk)
						item->Animation.TargetState = 2;
				}

				break;

			case 4:
				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (Targetable(item, &AI))
					item->Animation.TargetState = 5;
				else
				{
					if (creature->Mood == MoodType::Attack)
						item->Animation.TargetState = 1;
					else if (!AI.ahead)
						item->Animation.TargetState = 1;
				}

				break;

			case 5:
			case 6:
				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (item->Animation.TargetState != 1 &&
					(creature->Mood == MoodType::Escape || AI.distance > pow(SECTOR(10), 2) || !Targetable(item, &AI)))
				{
					item->Animation.TargetState = 1;
				}

				break;

			case 8:
			case 9:
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (Targetable(item, &AI))
					item->Animation.TargetState = (item->Animation.ActiveState == 8) ? 5 : 11;
				else
					item->Animation.TargetState = 1;

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
