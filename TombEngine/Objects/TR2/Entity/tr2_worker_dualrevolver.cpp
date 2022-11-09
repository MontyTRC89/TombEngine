#include "framework.h"
#include "Objects/TR2/Entity/tr2_worker_dualrevolver.h"

#include "Game/control/box.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

namespace TEN::Entities::Creatures::TR2
{
	const auto WorkerDualGunBiteLeft  = BiteInfo(Vector3(-2.0f, 275.0f, 23.0f), 6);
	const auto WorkerDualGunBiteRight = BiteInfo(Vector3(2.0f, 275.0f, 23.0f), 10);

	// TODO
	enum WorkerDualGunState
	{

	};

	// TODO
	enum WorkerDualGunAnim
	{

	};

	void WorkerDualGunControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short tilt = 0;
		auto extraHeadRot = EulerAngles::Zero;
		auto extraTorsoRot = EulerAngles::Zero;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 11)
				SetAnimation(item, 32);
		}
		else if (LaraItem->HitPoints <= 0)
			item->Animation.TargetState = 2;
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
			case 2:
				creature->MaxTurn = 0;

				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (creature->Mood == MoodType::Attack || LaraItem->HitPoints > 0)
				{
					if (Targetable(item, &AI))
					{
						if (AI.distance <= pow(SECTOR(3), 2))
							item->Animation.TargetState = 9;
						else
							item->Animation.TargetState = 3;
					}
					else
					{
						switch (creature->Mood)
						{
						case MoodType::Attack:
							if (AI.distance > pow(SECTOR(20), 2) || !AI.ahead)
								item->Animation.TargetState = 4;
							else
								item->Animation.TargetState = 3;

							break;

						case MoodType::Escape:
							item->Animation.TargetState = 4;
							break;

						case MoodType::Stalk:
							item->Animation.TargetState = 3;
							break;

						default:
							if (!AI.ahead)
								item->Animation.TargetState = 3;

							break;
						}
					}
				}
				else
					item->Animation.TargetState = 1;

				break;

			case 3:
				creature->MaxTurn = ANGLE(3.0f);

				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (Targetable(item, &AI))
				{
					if (AI.distance < pow(SECTOR(3), 2) || AI.zoneNumber != AI.enemyZone)
						item->Animation.TargetState = 1;
					else
					{
						if (AI.angle >= 0)
							item->Animation.TargetState = 6;
						else
							item->Animation.TargetState = 5;
					}
				}

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = 4;
				else if (creature->Mood == MoodType::Attack || creature->Mood == MoodType::Stalk)
				{
					if (AI.distance > pow(SECTOR(20), 2) || !AI.ahead)
						item->Animation.TargetState = 4;
				}
				else if (LaraItem->HitPoints > 0)
				{
					if (AI.ahead)
						item->Animation.TargetState = 1;
				}
				else
					item->Animation.TargetState = 2;

				break;

			case 4:
				creature->MaxTurn = ANGLE(6.0f);
				tilt = angle / 4;

				if (AI.ahead)
				{
					extraHeadRot.x = AI.xAngle;
					extraHeadRot.y = AI.angle;
				}

				if (Targetable(item, &AI))
				{
					if (AI.zoneNumber == AI.enemyZone)
					{
						if (AI.angle >= 0)
							item->Animation.TargetState = 6;
						else
							item->Animation.TargetState = 5;
					}
					else
						item->Animation.TargetState = 3;
				}
				else if (creature->Mood == MoodType::Attack)
				{
					if (AI.ahead && AI.distance < pow(SECTOR(20), 2))
						item->Animation.TargetState = 3;
				}
				else if (LaraItem->HitPoints > 0)
					item->Animation.TargetState = 1;
				else
					item->Animation.TargetState = 2;

				break;

			case 5:
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (Targetable(item, &AI))
					item->Animation.TargetState = 7;
				else
					item->Animation.TargetState = 3;

				break;

			case 6:
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (Targetable(item, &AI))
					item->Animation.TargetState = 8;
				else
					item->Animation.TargetState = 3;

				break;

			case 7:
				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (!creature->Flags)
				{
					ShotLara(item, &AI, WorkerDualGunBiteLeft, extraTorsoRot.y, 50);
					creature->Flags = 1;
				}

				break;

			case 8:
				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (!creature->Flags)
				{
					ShotLara(item, &AI, WorkerDualGunBiteRight, extraTorsoRot.y, 50);
					creature->Flags = 1;
				}

				break;

			case 9:
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (Targetable(item, &AI))
					item->Animation.TargetState = 10;
				else
					item->Animation.TargetState = 1;

				break;

			case 10:
				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;
				}

				if (!creature->Flags)
				{
					ShotLara(item, &AI, WorkerDualGunBiteLeft, extraTorsoRot.y, 50);
					ShotLara(item, &AI, WorkerDualGunBiteRight, extraTorsoRot.y, 50);
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
