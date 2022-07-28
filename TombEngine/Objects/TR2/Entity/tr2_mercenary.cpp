#include "framework.h"
#include "Objects/TR2/Entity/tr2_mercenary.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/trmath.h"

BITE_INFO MercenaryUziBite = { 0, 150, 19, 17 };
BITE_INFO MercenaryAutoPistolBite = { 0, 230, 9, 17 };

// TODO
enum MercenaryState
{

};

// TODO
enum MercenaryAnim
{

};

void MercenaryUziControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short angle = 0;
	short tilt = 0;
	short headX = 0;
	short headY = 0;
	short torsoX = 0;
	short torsoY = 0;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 13)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 14;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 13;
		}
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, TIMID);
		CreatureMood(item, &AI, TIMID);

		angle = CreatureTurn(item, creature->MaxTurn);

		switch (item->Animation.ActiveState)
		{
		case 1:
			if (AI.ahead)
			{
				headX = AI.xAngle;
				headY = AI.angle;
			}

			creature->MaxTurn = 0;

			if (creature->Mood == MoodType::Escape)
				item->Animation.TargetState = 2;
			else if (Targetable(item, &AI))
			{
				if (AI.distance > pow(SECTOR(2), 2))
					item->Animation.TargetState = 2;

				if (GetRandomControl() >= 0x2000)
				{
					if (GetRandomControl() >= 0x4000)
						item->Animation.TargetState = 11;
					else
						item->Animation.TargetState = 7;
				}
				else
					item->Animation.TargetState = 5;
			}
			else
			{
				if (creature->Mood == MoodType::Attack)
					item->Animation.TargetState = 3;
				else if (!AI.ahead)
					item->Animation.TargetState = 2;
				else
					item->Animation.TargetState = 1;
			}

			break;

		case 2:
			creature->MaxTurn = ANGLE(7.0f);

			if (AI.ahead)
			{
				headX = AI.xAngle;
				headY = AI.angle;
			}

			if (creature->Mood == MoodType::Escape)
				item->Animation.TargetState = 3;
			else if (Targetable(item, &AI))
			{
				if (AI.distance <= pow(SECTOR(2), 2) || AI.zoneNumber != AI.enemyZone)
					item->Animation.TargetState = 1;
				else
					item->Animation.TargetState = 12;
			}
			else if (creature->Mood == MoodType::Attack)
				item->Animation.TargetState = 3;
			else
			{
				if (AI.ahead)
					item->Animation.TargetState = 2;
				else
					item->Animation.TargetState = 1;
			
			}
			
			break;

		case 3:
			tilt = angle / 3;
			creature->MaxTurn = ANGLE(10.0f);

			if (AI.ahead)
			{
				headX = AI.xAngle;
				headY = AI.angle;
			}

			if (creature->Mood != MoodType::Escape)
			{
				if (Targetable(item, &AI))
					item->Animation.TargetState = 1;
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = 2;
			}

			break;

		case 5:
		case 7:
		case 8:
		case 9:
			creature->MaxTurn = 0;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			if (!ShotLara(item, &AI, &MercenaryUziBite, torsoY, 8))
				item->Animation.TargetState = 1;

			if (AI.distance < pow(SECTOR(2), 2))
				item->Animation.TargetState = 1;

			break;

		case 10:
		case 14:
			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			if (!ShotLara(item, &AI, &MercenaryUziBite, torsoY, 8))
				item->Animation.TargetState = 1;

			if (AI.distance < pow(SECTOR(2), 2))
				item->Animation.TargetState = 2;

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

void MercenaryAutoPistolControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short angle = 0;
	short tilt = 0;
	short headX = 0;
	short headY = 0;
	short torsoX = 0;
	short torsoY = 0;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 11)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 9;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 11;
		}
	}
	else
	{
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);

		switch (item->Animation.ActiveState)
		{
		case 2:
			creature->MaxTurn = 0;

			if (AI.ahead)
			{
				headX = AI.xAngle;
				headY = AI.angle;
			}

			if (creature->Mood == MoodType::Escape)
				item->Animation.TargetState = 4;
			else if (Targetable(item, &AI))
			{
				if (AI.distance <= pow(SECTOR(2), 2))
				{
					if (GetRandomControl() >= 0x2000)
					{
						if (GetRandomControl() >= 0x4000)
							item->Animation.TargetState = 5;
						else
							item->Animation.TargetState = 8;
					}
					else
						item->Animation.TargetState = 7;
				}
				else
					item->Animation.TargetState = 3;
			}
			else
			{
				if (creature->Mood == MoodType::Attack)
					item->Animation.TargetState = 4;
				if (!AI.ahead || GetRandomControl() < 0x100)
					item->Animation.TargetState = 3;
			}

			break;

		case 3:
			creature->MaxTurn = ANGLE(7.0f);

			if (AI.ahead)
			{
				headX = AI.xAngle;
				headY = AI.angle;
			}

			if (creature->Mood == MoodType::Escape)
				item->Animation.TargetState = 4;
			else if (Targetable(item, &AI))
			{
				if (AI.distance < pow(SECTOR(2), 2) || AI.zoneNumber == AI.enemyZone || GetRandomControl() < 1024)
					item->Animation.TargetState = 2;
				else
					item->Animation.TargetState = 1;
			}
			else if (creature->Mood == MoodType::Escape)
				item->Animation.TargetState = 4;
			else if (AI.ahead && GetRandomControl() < 1024)
				item->Animation.TargetState = 2;
			
			break;

		case 4:
			tilt = angle / 3;
			creature->MaxTurn = ANGLE(10.0f);

			if (AI.ahead)
			{
				headX = AI.xAngle;
				headY = AI.angle;
			}

			if (creature->Mood != MoodType::Escape && (creature->Mood == MoodType::Escape || Targetable(item, &AI)))
				item->Animation.TargetState = 2;

			break;

		case 1:
		case 5:
		case 6:
			creature->Flags = 0;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			break;

		case 7:
		case 8:
		case 13:
			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;

				if (!creature->Flags)
				{
					if (GetRandomControl() < 0x2000)
						item->Animation.TargetState = 2;

					ShotLara(item, &AI, &MercenaryAutoPistolBite, torsoY, 50);
					creature->Flags = 1;
				}
			}
			else
				item->Animation.TargetState = 2;
			
			break;

		case 9:
			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;

				if (AI.distance < pow(SECTOR(2), 2))
					item->Animation.TargetState = 3;

				if (creature->Flags != 1)
				{
					if (!ShotLara(item, &AI, &MercenaryAutoPistolBite, torsoY, 50))
						item->Animation.TargetState = 3;

					creature->Flags = 1;
				}
			}
			else
				item->Animation.TargetState = 3;
			
			break;

		case 12:
			creature->Flags = 0;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			if (Targetable(item, &AI))
				item->Animation.TargetState = 13;
			else
				item->Animation.TargetState = 2;

			break;

		case 10:
			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;

				if (AI.distance < pow(SECTOR(2), 2))
					item->Animation.TargetState = 3;

				if (creature->Flags != 2)
				{
					if (!ShotLara(item, &AI, &MercenaryAutoPistolBite, torsoY, 50))
						item->Animation.TargetState = 3;

					creature->Flags = 2;
				}
			}
			else
				item->Animation.TargetState = 3;
			
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
