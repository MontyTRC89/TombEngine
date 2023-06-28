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
#include "Game/Setup.h"
#include "Specific/level.h"

namespace TEN::Entities::Creatures::TR2
{
	constexpr auto WORKER_SHOTGUN_NUM_SHOTS = 6;

	const auto WorkerShotgunBite = CreatureBiteInfo(Vector3(0, 350, 40), 9);

	// TODO
	enum ShotgunWorkerState
	{

	};

	// TODO
	enum ShotgunWorkerAnim
	{

	};

	static void ShootWorkerShotgun(ItemInfo& item, AI_INFO& ai, const CreatureBiteInfo& bite, short headingAngle, int damage)
	{
		for (int i = 0; i < WORKER_SHOTGUN_NUM_SHOTS; i++)
			ShotLara(&item, &ai, bite, headingAngle, damage);
	}

	void InitializeWorkerShotgun(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		InitializeCreature(itemNumber);
		SetAnimation(&item, 5);
	}

	void WorkerShotgunControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short headingAngle = 0;
		short tiltAngle = 0;
		auto extraHeadRot = EulerAngles::Zero;
		auto extraTorsoRot = EulerAngles::Zero;

		if (creature->MuzzleFlash[0].Delay != 0)
			creature->MuzzleFlash[0].Delay--;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 7)
				SetAnimation(item, 18);
		}
		else
		{
			AI_INFO ai;
			CreatureAIInfo(item, &ai);

			GetCreatureMood(item, &ai, true);
			CreatureMood(item, &ai, true);

			headingAngle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case 2:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (ai.ahead)
				{
					extraHeadRot.x = ai.xAngle;
					extraHeadRot.y = ai.angle;
				}

				if (creature->Mood == MoodType::Escape)
				{
					item->Animation.TargetState = 5;
				}
				else if (Targetable(item, &ai))
				{
					if (ai.distance < SQUARE(BLOCK(3)) || ai.zoneNumber != ai.enemyZone)
					{
						item->Animation.TargetState = (GetRandomControl() >= 0x4000) ? 9 : 8;
					}
					else
					{
						item->Animation.TargetState = 1;
					}
				}
				else if (creature->Mood == MoodType::Attack || !ai.ahead)
				{
					if (ai.distance <= SQUARE(BLOCK(2)))
					{
						item->Animation.TargetState = 1;
					}
					else
					{
						item->Animation.TargetState = 5;
					}
				}
				else
				{
					item->Animation.TargetState = 3;
				}

				break;

			case 3:
				if (ai.ahead)
				{
					extraHeadRot.x = ai.xAngle;
					extraHeadRot.y = ai.angle;
				}

				if (Targetable(item, &ai))
				{
					item->Animation.TargetState = 4;
				}
				else if (creature->Mood == MoodType::Attack || !ai.ahead)
				{
					item->Animation.TargetState = 2;
				}

				break;

			case 1:
				creature->MaxTurn = ANGLE(3.0f);

				if (ai.ahead)
				{
					extraHeadRot.y = ai.angle;
					extraHeadRot.x = ai.xAngle;
				}

				if (creature->Mood == MoodType::Escape)
				{
					item->Animation.TargetState = 5;
				}
				else if (Targetable(item, &ai))
				{
					if (ai.distance < SQUARE(BLOCK(3)) || ai.zoneNumber != ai.enemyZone)
					{
						item->Animation.TargetState = 2;
					}
					else
					{
						item->Animation.TargetState = 6;
					}
				}
				else if (creature->Mood == MoodType::Attack || !ai.ahead)
				{
					if (ai.distance > SQUARE(BLOCK(2)))
						item->Animation.TargetState = 5;
				}
				else
				{
					item->Animation.TargetState = 2;
				}

				break;

			case 5:
				creature->MaxTurn = ANGLE(5.0f);
				tiltAngle = headingAngle / 2;

				if (ai.ahead)
				{
					extraHeadRot.x = ai.xAngle;
					extraHeadRot.y = ai.angle;
				}

				if (creature->Mood != MoodType::Escape)
				{
					if (Targetable(item, &ai))
					{
						item->Animation.TargetState = 1;
					}
					else if (creature->Mood == MoodType::Bored || creature->Mood == MoodType::Stalk)
					{
						item->Animation.TargetState = 1;
					}
				}

				break;

			case 8:
			case 9:
				creature->Flags = 0;

				if (ai.ahead)
				{
					extraTorsoRot.x = ai.xAngle;
					extraTorsoRot.y = ai.angle;
				}

				if (Targetable(item, &ai))
				{
					if (item->Animation.ActiveState == 8)
					{
						item->Animation.TargetState = 4;
					}
					else
					{
						item->Animation.TargetState = 10;
					}
				}

				break;

			case 4:
			case 10:
				if (ai.ahead)
				{
					extraTorsoRot.x = ai.xAngle;
					extraTorsoRot.y = ai.angle;
				}

				if (creature->Flags == 0)
				{
					ShootWorkerShotgun(*item, ai, WorkerShotgunBite, extraTorsoRot.y, 25);
					creature->MuzzleFlash[0].Bite = WorkerShotgunBite;
					creature->MuzzleFlash[0].Delay = 2;
					creature->Flags = 1;
				}

				if (item->Animation.ActiveState == 4 && item->Animation.TargetState != 2 &&
					(creature->Mood == MoodType::Escape || ai.distance > SQUARE(BLOCK(3)) || !Targetable(item, &ai)))
				{
					item->Animation.TargetState = 2;
				}

				break;

			case 6:
				if (ai.ahead)
				{
					extraTorsoRot.x = ai.xAngle;
					extraTorsoRot.y = ai.angle;
				}

				if (creature->Flags == 0)
				{
					creature->MuzzleFlash[0].Bite = WorkerShotgunBite;
					creature->MuzzleFlash[0].Delay = 1;
					ShootWorkerShotgun(*item, ai, WorkerShotgunBite, extraTorsoRot.y, 25);
					creature->Flags = 1;
				}

				break;
			}
		}

		CreatureTilt(item, tiltAngle);
		CreatureJoint(item, 0, extraTorsoRot.y);
		CreatureJoint(item, 1, extraTorsoRot.x);
		CreatureJoint(item, 2, extraHeadRot.y);
		CreatureJoint(item, 3, extraHeadRot.x);
		CreatureAnimation(itemNumber, headingAngle, tiltAngle);
	}
}
