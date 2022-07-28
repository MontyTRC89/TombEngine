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

BITE_INFO WorkerDualGunBiteLeft = { -2, 275, 23, 6 };
BITE_INFO WorkerDualGunBiteRight = { 2, 275, 23, 10 };

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

	short tilt = 0;
	short angle = 0;
	short headX = 0;
	short headY = 0;
	short torsoX = 0;
	short torsoY = 0;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 11)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 32;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 11;
		}
	}
	else if (LaraItem->HitPoints <= 0)
		item->Animation.TargetState = 2;
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);

		switch (item->Animation.ActiveState)
		{
		case 1:
		case 2:
			creature->MaxTurn = 0;

			if (AI.ahead)
			{
				headX = AI.xAngle;
				headY = AI.angle;
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
				headX = AI.xAngle;
				headY = AI.angle;
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
				headX = AI.xAngle;
				headY = AI.angle;
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
				torsoX = AI.xAngle;
				torsoY = AI.angle;
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
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			if (Targetable(item, &AI))
				item->Animation.TargetState = 8;
			else
				item->Animation.TargetState = 3;

			break;

		case 7:
			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			if (!creature->Flags)
			{
				ShotLara(item, &AI, &WorkerDualGunBiteLeft, torsoY, 50);
				creature->Flags = 1;
			}

			break;

		case 8:
			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			if (!creature->Flags)
			{
				ShotLara(item, &AI, &WorkerDualGunBiteRight, torsoY, 50);
				creature->Flags = 1;
			}

			break;

		case 9:
			creature->Flags = 0;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			if (Targetable(item, &AI))
				item->Animation.TargetState = 10;
			else
				item->Animation.TargetState = 1;

			break;

		case 10:
			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			if (!creature->Flags)
			{
				ShotLara(item, &AI, &WorkerDualGunBiteLeft, torsoY, 50);
				ShotLara(item, &AI, &WorkerDualGunBiteRight, torsoY, 50);
				creature->Flags = 1;
			}

			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torsoY);
	CreatureJoint(item, 1, torsoX);
	CreatureJoint(item, 2, headY);
	CreatureJoint(item, 3, headX);
	CreatureAnimation(itemNumber, angle, tilt);
}
