#include "framework.h"
#include "Objects/TR2/Entity/tr2_worker_machinegun.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Game/Setup.h"
#include "Specific/level.h"

namespace TEN::Entities::Creatures::TR2
{
	const auto WorkerMachineGunBite = CreatureBiteInfo(Vector3i(0, 380, 37), 9);

	// TODO
	enum WorkerMachineGunState
	{

	};

	// TODO
	enum WorkerMachineGunAnim
	{

	};

	void InitializeWorkerMachineGun(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		InitializeCreature(itemNumber);
		SetAnimation(item, 12);
	}

	static void Shoot(ItemInfo* item, CreatureInfo* creature, AI_INFO* ai, const EulerAngles& extraTorsoRot)
	{
		ShotLara(item, ai, WorkerMachineGunBite, extraTorsoRot.y, 30);
		creature->MuzzleFlash[0].Bite = WorkerMachineGunBite;
		creature->MuzzleFlash[0].Delay = 2;
	}

	void WorkerMachineGunControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short tilt = 0;
		auto extraHeadRot = EulerAngles::Zero;
		auto extraTorsoRot = EulerAngles::Zero;

		if (creature->MuzzleFlash[0].Delay != 0)
			creature->MuzzleFlash[0].Delay--;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 7)
				SetAnimation(item, 19);
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
					if (AI.distance < pow(SECTOR(3), 2) || AI.zoneNumber != AI.enemyZone)
						item->Animation.TargetState = (GetRandomControl() < 0x4000) ? 8 : 10;
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
				creature->MaxTurn = ANGLE(3.0f);

				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = 3;
				else if (Targetable(item, &AI))
				{
					if (AI.distance < pow(SECTOR(3), 2) || AI.zoneNumber != AI.enemyZone)
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
				creature->MaxTurn = ANGLE(5.0f);

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

			case 8:
			case 10:
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

			case 9:
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (Targetable(item, &AI))
					item->Animation.TargetState = 6;
				else
					item->Animation.TargetState = 2;

				break;

			case 5:
			case 11:
				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (item->Animation.AnimNumber == GetAnimIndex(*item, 2))
				{
					if (item->Animation.FrameNumber == GetFrameIndex(item, 0))
						Shoot(item, creature, &AI, extraTorsoRot);
					else if (item->Animation.FrameNumber == GetFrameIndex(item, 6))
						Shoot(item, creature, &AI, extraTorsoRot);
					else if (item->Animation.FrameNumber == GetFrameIndex(item, 12))
						Shoot(item, creature, &AI, extraTorsoRot);
				}
				else if (item->Animation.AnimNumber == GetAnimIndex(*item, 21) &&
					item->Animation.FrameNumber == GetFrameIndex(item, 0))
				{
					Shoot(item, creature, &AI, extraTorsoRot);
				}

				if (item->Animation.TargetState != 1 &&
					(creature->Mood == MoodType::Escape || AI.distance > pow(SECTOR(3), 2) || !Targetable(item, &AI)))
				{
					item->Animation.TargetState = 1;
				}

				break;

			case 6:
				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (item->Animation.FrameNumber == GetFrameIndex(item, 0))
					Shoot(item, creature, &AI, extraTorsoRot);
				else if (item->Animation.FrameNumber == GetFrameIndex(item, 2))
					Shoot(item, creature, &AI, extraTorsoRot);
				else if (item->Animation.FrameNumber == GetFrameIndex(item, 6))
					Shoot(item, creature, &AI, extraTorsoRot);
				else if (item->Animation.FrameNumber == GetFrameIndex(item, 12))
					Shoot(item, creature, &AI, extraTorsoRot);

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
