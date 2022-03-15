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
	auto* info = GetCreatureInfo(item);

	short angle = 0;
	short headX = 0;
	short headY = 0;
	short torsoX = 0;
	short torsoY = 0;
	short tilt = 0;

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

		angle = CreatureTurn(item, info->MaxTurn);

		switch (item->Animation.ActiveState)
		{
		case 1:
			if (AI.ahead)
			{
				headX = AI.xAngle;
				headY = AI.angle;
			}

			info->MaxTurn = 0;

			if (info->Mood == MoodType::Escape)
				item->Animation.TargetState = 2;
			else if (Targetable(item, &AI))
			{
				if (AI.distance > 0x400000)
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
				if (info->Mood == MoodType::Attack)
					item->Animation.TargetState = 3;
				else if (!AI.ahead)
					item->Animation.TargetState = 2;
				else
					item->Animation.TargetState = 1;
			}

			break;

		case 2:
			if (AI.ahead)
			{
				headX = AI.xAngle;
				headY = AI.angle;
			}

			info->MaxTurn = ANGLE(7.0f);

			if (info->Mood == MoodType::Escape)
				item->Animation.TargetState = 3;
			else if (Targetable(item, &AI))
			{
				if (AI.distance <= 0x400000 || AI.zoneNumber != AI.enemyZone)
					item->Animation.TargetState = 1;
				else
					item->Animation.TargetState = 12;
			}
			else if (info->Mood == MoodType::Attack)
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
			if (AI.ahead)
			{
				headX = AI.xAngle;
				headY = AI.angle;
			}

			info->MaxTurn = ANGLE(10.0f);
			tilt = angle / 3;

			if (info->Mood != MoodType::Escape)
			{
				if (Targetable(item, &AI))
					item->Animation.TargetState = 1;
				else if (info->Mood == MoodType::Bored)
					item->Animation.TargetState = 2;
			}

			break;

		case 5:
		case 7:
		case 8:
		case 9:
			info->MaxTurn = 0;

			if (AI.ahead)
			{
				torsoX = AI.xAngle;
				torsoY = AI.angle;
			}

			if (!ShotLara(item, &AI, &MercenaryUziBite, torsoY, 8))
				item->Animation.TargetState = 1;

			if (AI.distance < 0x400000)
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

			if (AI.distance < 0x400000)
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
	auto* info = GetCreatureInfo(item);

	short angle = 0;
	short headX = 0;
	short headY = 0;
	short torsoX = 0;
	short torsoY = 0;
	short tilt = 0;

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
		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		GetCreatureMood(item, &aiInfo, VIOLENT);
		CreatureMood(item, &aiInfo, VIOLENT);

		angle = CreatureTurn(item, info->MaxTurn);

		switch (item->Animation.ActiveState)
		{
		case 2:
			info->MaxTurn = 0;

			if (aiInfo.ahead)
			{
				headX = aiInfo.xAngle;
				headY = aiInfo.angle;
			}

			if (info->Mood == MoodType::Escape)
				item->Animation.TargetState = 4;
			else if (Targetable(item, &aiInfo))
			{
				if (aiInfo.distance <= 0x400000)
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
				if (info->Mood == MoodType::Attack)
					item->Animation.TargetState = 4;
				if (!aiInfo.ahead || GetRandomControl() < 0x100)
					item->Animation.TargetState = 3;
			}

			break;

		case 3:
			info->MaxTurn = ANGLE(7.0f);

			if (aiInfo.ahead)
			{
				headX = aiInfo.xAngle;
				headY = aiInfo.angle;
			}

			if (info->Mood == MoodType::Escape)
				item->Animation.TargetState = 4;
			else if (Targetable(item, &aiInfo))
			{
				if (aiInfo.distance < 0x400000 || aiInfo.zoneNumber == aiInfo.enemyZone || GetRandomControl() < 1024)
					item->Animation.TargetState = 2;
				else
					item->Animation.TargetState = 1;
			}
			else if (info->Mood == MoodType::Escape)
				item->Animation.TargetState = 4;
			else if (aiInfo.ahead && GetRandomControl() < 1024)
				item->Animation.TargetState = 2;
			
			break;

		case 4:
			info->MaxTurn = ANGLE(10.0f);
			tilt = angle / 3;

			if (aiInfo.ahead)
			{
				headX = aiInfo.xAngle;
				headY = aiInfo.angle;
			}

			if (info->Mood != MoodType::Escape && (info->Mood == MoodType::Escape || Targetable(item, &aiInfo)))
				item->Animation.TargetState = 2;

			break;

		case 1:
		case 5:
		case 6:
			info->Flags = 0;

			if (aiInfo.ahead)
			{
				torsoX = aiInfo.xAngle;
				torsoY = aiInfo.angle;
			}

			break;

		case 7:
		case 8:
		case 13:
			if (aiInfo.ahead)
			{
				torsoX = aiInfo.xAngle;
				torsoY = aiInfo.angle;

				if (!info->Flags)
				{
					if (GetRandomControl() < 0x2000)
						item->Animation.TargetState = 2;

					ShotLara(item, &aiInfo, &MercenaryAutoPistolBite, torsoY, 50);
					info->Flags = 1;
				}
			}
			else
				item->Animation.TargetState = 2;
			
			break;

		case 9:
			if (aiInfo.ahead)
			{
				torsoX = aiInfo.xAngle;
				torsoY = aiInfo.angle;

				if (aiInfo.distance < 0x400000)
					item->Animation.TargetState = 3;

				if (info->Flags != 1)
				{
					if (!ShotLara(item, &aiInfo, &MercenaryAutoPistolBite, torsoY, 50))
						item->Animation.TargetState = 3;
					info->Flags = 1;
				}
			}
			else
				item->Animation.TargetState = 3;
			
			break;

		case 12:
			info->Flags = 0;

			if (aiInfo.ahead)
			{
				torsoX = aiInfo.xAngle;
				torsoY = aiInfo.angle;
			}

			if (Targetable(item, &aiInfo))
				item->Animation.TargetState = 13;
			else
				item->Animation.TargetState = 2;

			break;

		case 10:
			if (aiInfo.ahead)
			{
				torsoX = aiInfo.xAngle;
				torsoY = aiInfo.angle;

				if (aiInfo.distance < 0x400000)
					item->Animation.TargetState = 3;

				if (info->Flags != 2)
				{
					if (!ShotLara(item, &aiInfo, &MercenaryAutoPistolBite, torsoY, 50))
						item->Animation.TargetState = 3;

					info->Flags = 2;
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
