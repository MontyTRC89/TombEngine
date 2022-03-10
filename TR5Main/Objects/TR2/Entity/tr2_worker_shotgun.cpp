#include "framework.h"
#include "Objects/TR2/Entity/tr2_worker_shotgun.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO WorkerShotgunBite = { 0, 281, 40, 9 };

// TODO
enum ShotgunWorkerState
{

};

// TODO
enum ShotgunWorkerAnim
{

};

static void ShotLaraWithShotgun(ITEM_INFO* item, AI_INFO* info, BITE_INFO* bite, short angleY, int damage)
{
	ShotLara(item, info, bite, angleY, damage);
	ShotLara(item, info, bite, angleY, damage);
	ShotLara(item, info, bite, angleY, damage);
	ShotLara(item, info, bite, angleY, damage);
	ShotLara(item, info, bite, angleY, damage);
	ShotLara(item, info, bite, angleY, damage);
}

void InitialiseWorkerShotgun(short itemNum)
{
	auto* item = &g_Level.Items[itemNum];

	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 5;
	ClearItem(itemNum);

	auto* anim = &g_Level.Anims[item->AnimNumber];
	item->FrameNumber = anim->frameBase;
	item->ActiveState = anim->ActiveState;
}

void WorkerShotgunControl(short itemNumber)
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
		if (item->ActiveState != 7)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 18;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 7;
		}
	}
	else
	{
		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		GetCreatureMood(item, &aiInfo, VIOLENT);
		CreatureMood(item, &aiInfo, VIOLENT);

		angle = CreatureTurn(item, info->MaxTurn);

		switch (item->ActiveState)
		{
		case 2:
			info->Flags = 0;
			info->MaxTurn = 0;

			if (aiInfo.ahead)
			{
				headY = aiInfo.angle;
				headX = aiInfo.xAngle;
			}

			if (info->Mood == MoodType::Escape)
			{
				item->TargetState = 5;
			}
			else if (Targetable(item, &aiInfo))
			{
				if (aiInfo.distance <= 0x900000 || aiInfo.zoneNumber != aiInfo.enemyZone)
					item->TargetState = (GetRandomControl() >= 0x4000) ? 9 : 8;
				else
					item->TargetState = 1;
			}
			else if (info->Mood == MoodType::Attack || !aiInfo.ahead)
				item->TargetState = (aiInfo.distance <= 0x400000) ? 1 : 5;
			else
				item->TargetState = 3;
			
			break;

		case 3:
			if (aiInfo.ahead)
			{
				headY = aiInfo.angle;
				headX = aiInfo.xAngle;
			}

			if (Targetable(item, &aiInfo))
				item->TargetState = 4;
			else if (info->Mood == MoodType::Attack || !aiInfo.ahead)
				item->TargetState = 2;
			
			break;

		case 1:
			info->MaxTurn = 546;

			if (aiInfo.ahead)
			{
				headY = aiInfo.angle;
				headX = aiInfo.xAngle;
			}

			if (info->Mood == MoodType::Escape)
				item->TargetState = 5;
			else if (Targetable(item, &aiInfo))
			{
				if (aiInfo.distance < 0x900000 || aiInfo.zoneNumber != aiInfo.enemyZone)
					item->TargetState = 2;
				else
					item->TargetState = 6;
			}
			else if (info->Mood == MoodType::Attack || !aiInfo.ahead)
			{
				if (aiInfo.distance > 0x400000)
					item->TargetState = 5;
			}
			else
				item->TargetState = 2;
			
			break;

		case 5:
			info->MaxTurn = 910;
			tilt = angle / 2;

			if (aiInfo.ahead)
			{
				headX = aiInfo.xAngle;
				headY = aiInfo.angle;
			}

			if (info->Mood != MoodType::Escape)
			{
				if (Targetable(item, &aiInfo))
					item->TargetState = 1;
				else if (info->Mood == MoodType::Bored || info->Mood == MoodType::Stalk)
					item->TargetState = 1;
			}
			
			break;

		case 8:
		case 9:
			info->Flags = 0;

			if (aiInfo.ahead)
			{
				torsoY = aiInfo.angle;
				torsoX = aiInfo.xAngle;
			}

			if (Targetable(item, &aiInfo))
				item->TargetState = (item->ActiveState == 8) ? 4 : 10;
			
			break;

		case 4:
		case 10:
			if (aiInfo.ahead)
			{
				torsoX = aiInfo.xAngle;
				torsoY = aiInfo.angle;
			}

			if (!info->Flags)
			{
				ShotLaraWithShotgun(item, &aiInfo, &WorkerShotgunBite, torsoY, 25);
				info->FiredWeapon = 2;
				info->Flags = 1;
			}

			if (item->ActiveState == 4 && item->TargetState != 2 &&
				(info->Mood == MoodType::Escape || aiInfo.distance > 0x900000 || !Targetable(item, &aiInfo)))
			{
				item->TargetState = 2;
			}
			
			break;

		case 6:
			if (aiInfo.ahead)
			{
				torsoX = aiInfo.xAngle;
				torsoY = aiInfo.angle;
			}

			if (!info->Flags)
			{
				ShotLaraWithShotgun(item, &aiInfo, &WorkerShotgunBite, torsoY, 25);
				info->FiredWeapon = 2;
				info->Flags = 1;
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
