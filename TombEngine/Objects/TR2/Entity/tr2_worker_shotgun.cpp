#include "framework.h"
#include "Objects/TR2/Entity/tr2_worker_shotgun.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Specific/level.h"
#include "Specific/setup.h"

namespace TEN::Entities::Creatures::TR2
{
	constexpr auto WORKER_SHOTGUN_NUM_SHOTS = 6;

	const auto WorkerShotgunBite = BiteInfo(Vector3(0.0f, 281.0f, 40.0f), 9);

	// TODO
	enum ShotgunWorkerState
	{

	};

	// TODO
	enum ShotgunWorkerAnim
	{

	};

	void ShotLaraWithShotgun(ItemInfo* item, AI_INFO* info, BiteInfo bite, short angleY, int damage)
	{
		for (int i = 0; i < WORKER_SHOTGUN_NUM_SHOTS; i++)
			ShotLara(item, info, bite, angleY, damage);
	}

	void InitialiseWorkerShotgun(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);
		SetAnimation(item, 5);
	}

	void WorkerShotgunControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short tilt = 0;
		auto extraHeadRot = EulerAngles::Zero;
		auto extraTorsoRot = EulerAngles::Zero;

		if (creature->FiredWeapon)
		{
			auto pos = GetJointPosition(item, WorkerShotgunBite.meshNum, Vector3i(WorkerShotgunBite.Position));
			TriggerDynamicLight(pos.x, pos.y, pos.z, (creature->FiredWeapon * 2) + 4, 24, 16, 4);
			creature->FiredWeapon--;
		}

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 7)
				SetAnimation(item, 18);
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case 2:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = 5;
				else if (Targetable(item, &AI))
				{
					if (AI.distance < pow(SECTOR(3), 2) || AI.zoneNumber != AI.enemyZone)
						item->Animation.TargetState = (GetRandomControl() >= 0x4000) ? 9 : 8;
					else
						item->Animation.TargetState = 1;
				}
				else if (creature->Mood == MoodType::Attack || !AI.ahead)
				{
					if (AI.distance <= pow(SECTOR(2), 2))
						item->Animation.TargetState = 1;
					else
						item->Animation.TargetState = 5;
				}
				else
					item->Animation.TargetState = 3;

				break;

			case 3:
				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (Targetable(item, &AI))
					item->Animation.TargetState = 4;
				else if (creature->Mood == MoodType::Attack || !AI.ahead)
					item->Animation.TargetState = 2;

				break;

			case 1:
				creature->MaxTurn = ANGLE(3.0f);

				if (AI.ahead)
				{
					extraHeadRot.y = AI.angle;
					extraHeadRot.x = AI.xAngle;
				}

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = 5;
				else if (Targetable(item, &AI))
				{
					if (AI.distance < pow(SECTOR(3), 2) || AI.zoneNumber != AI.enemyZone)
						item->Animation.TargetState = 2;
					else
						item->Animation.TargetState = 6;
				}
				else if (creature->Mood == MoodType::Attack || !AI.ahead)
				{
					if (AI.distance > pow(SECTOR(2), 2))
						item->Animation.TargetState = 5;
				}
				else
					item->Animation.TargetState = 2;

				break;

			case 5:
				creature->MaxTurn = ANGLE(5.0f);
				tilt = angle / 2;

				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (creature->Mood != MoodType::Escape)
				{
					if (Targetable(item, &AI))
						item->Animation.TargetState = 1;
					else if (creature->Mood == MoodType::Bored || creature->Mood == MoodType::Stalk)
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
				{
					if (item->Animation.ActiveState == 8)
						item->Animation.TargetState = 4;
					else
						item->Animation.TargetState = 10;
				}

				break;

			case 4:
			case 10:
				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (!creature->Flags)
				{
					ShotLaraWithShotgun(item, &AI, WorkerShotgunBite, extraTorsoRot.y, 25);
					creature->FiredWeapon = 2;
					creature->Flags = 1;
				}

				if (item->Animation.ActiveState == 4 && item->Animation.TargetState != 2 &&
					(creature->Mood == MoodType::Escape || AI.distance > pow(SECTOR(3), 2) || !Targetable(item, &AI)))
				{
					item->Animation.TargetState = 2;
				}

				break;

			case 6:
				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (!creature->Flags)
				{
					ShotLaraWithShotgun(item, &AI, WorkerShotgunBite, extraTorsoRot.y, 25);
					creature->FiredWeapon = 2;
					creature->Flags = 1;
				}

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
