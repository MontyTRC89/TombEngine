#include "framework.h"
#include "Objects/TR2/Entity/tr2_worker_dualrevolver.h"

#include "Game/control/box.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO workerDualGunL = { -2, 275, 23, 6 };
BITE_INFO workerDualGunR = { 2, 275, 23, 10 };

void WorkerDualGunControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CreatureInfo* dual;
	AI_INFO info;
	short angle, head_x, head_y, torso_x, torso_y, tilt;

	item = &g_Level.Items[itemNum];
	dual = (CreatureInfo*)item->Data;
	angle = head_x = head_y = torso_x = torso_y = tilt = 0;

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
	{
		item->Animation.TargetState = 2;
	}
	else
	{
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, dual->MaxTurn);

		switch (item->Animation.ActiveState)
		{
		case 1:
		case 2:
			dual->MaxTurn = 0;

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (dual->Mood == MoodType::Attack || LaraItem->HitPoints > 0)
			{
				if (Targetable(item, &info))
				{
					if (info.distance <= 0x900000)
						item->Animation.TargetState = 9;
					else
						item->Animation.TargetState = 3;
				}
				else
				{
					switch (dual->Mood)
					{
					case MoodType::Attack:
						if (info.distance > 0x19000000 || !info.ahead)
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
						if (!info.ahead)
							item->Animation.TargetState = 3;
						break;
					}
				}
			}
			else
			{
				item->Animation.TargetState = 1;
			}
			break;
		case 3:
			dual->MaxTurn = ANGLE(3);

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (Targetable(item, &info))
			{
				if (info.distance < 0x900000 || info.zoneNumber != info.enemyZone)
				{
					item->Animation.TargetState = 1;
				}
				else
				{
					if (info.angle >= 0)
						item->Animation.TargetState = 6;
					else
						item->Animation.TargetState = 5;
				}
			}

			if (dual->Mood == MoodType::Escape)
			{
				item->Animation.TargetState = 4;
			}
			else if (dual->Mood == MoodType::Attack || dual->Mood == MoodType::Stalk)
			{
				if (info.distance > 0x19000000 || !info.ahead)
					item->Animation.TargetState = 4;
			}
			else if (LaraItem->HitPoints > 0)
			{
				if (info.ahead)
					item->Animation.TargetState = 1;
			}
			else
			{
				item->Animation.TargetState = 2;
			}
			break;
		case 4:
			dual->MaxTurn = ANGLE(6);

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			tilt = angle / 4;

			if (Targetable(item, &info))
			{
				if (info.zoneNumber == info.enemyZone)
				{
					if (info.angle >= 0)
						item->Animation.TargetState = 6;
					else
						item->Animation.TargetState = 5;
				}
				else
				{
					item->Animation.TargetState = 3;
				}
			}
			else if (dual->Mood == MoodType::Attack)
			{
				if (info.ahead && info.distance < 0x19000000)
					item->Animation.TargetState = 3;
			}
			else if (LaraItem->HitPoints > 0)
			{
				item->Animation.TargetState = 1;
			}
			else
			{
				item->Animation.TargetState = 2;
			}
			break;
		case 5:
			dual->Flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (Targetable(item, &info))
				item->Animation.TargetState = 7;
			else
				item->Animation.TargetState = 3;
			break;
		case 6:
			dual->Flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (Targetable(item, &info))
				item->Animation.TargetState = 8;
			else
				item->Animation.TargetState = 3;
			break;
		case 7:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (!dual->Flags)
			{
				ShotLara(item, &info, &workerDualGunL, torso_y, 50);
				dual->Flags = 1;
			}
			break;
		case 8:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (!dual->Flags)
			{
				ShotLara(item, &info, &workerDualGunR, torso_y, 50);
				dual->Flags = 1;
			}
			break;
		case 9:
			dual->Flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (Targetable(item, &info))
				item->Animation.TargetState = 10;
			else
				item->Animation.TargetState = 1;
			break;
		case 10:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (!dual->Flags)
			{
				ShotLara(item, &info, &workerDualGunL, torso_y, 50);
				ShotLara(item, &info, &workerDualGunR, torso_y, 50);
				dual->Flags = 1;
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head_y);
	CreatureJoint(item, 3, head_x);
	CreatureAnimation(itemNum, angle, tilt);
}