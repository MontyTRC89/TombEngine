#include "framework.h"
#include "Objects/TR2/Entity/tr2_worker_machinegun.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/people.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO workerMachineGun = { 0, 308, 32, 9 };

void InitialiseWorkerMachineGun(short itemNum)
{
	ANIM_STRUCT* anim;
	ITEM_INFO* item;
	item = &g_Level.Items[itemNum];
	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 12;

	ClearItem(itemNum);

	anim = &g_Level.Anims[item->Animation.AnimNumber];
	item->Animation.FrameNumber = anim->frameBase;
	item->Animation.ActiveState = anim->ActiveState;
}

void WorkerMachineGunControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CreatureInfo* machinegun;
	AI_INFO info;
	short angle, head_y, head_x, torso_y, torso_x, tilt;

	item = &g_Level.Items[itemNum];
	machinegun = (CreatureInfo*)item->Data;
	angle = head_y = head_x = torso_y = torso_x = tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != 7)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 19;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 7;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, machinegun->MaxTurn);

		switch (item->Animation.ActiveState)
		{
		case 1:
			machinegun->Flags = 0;
			machinegun->MaxTurn = 0;

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (machinegun->Mood == MoodType::Escape)
			{
				item->Animation.TargetState = 3;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < 0x900000 || info.zoneNumber != info.enemyZone)
					item->Animation.TargetState = (GetRandomControl() < 0x4000) ? 8 : 10;
				else
					item->Animation.TargetState = 2;
			}
			else if (machinegun->Mood == MoodType::Attack || !info.ahead)
			{
				if (info.distance <= 0x400000)
					item->Animation.TargetState = 2;
				else
					item->Animation.TargetState = 3;
			}
			else
			{
				item->Animation.TargetState = 4;
			}
			break;

		case 2:
			machinegun->MaxTurn = 546;

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (machinegun->Mood == MoodType::Escape)
			{
				item->Animation.TargetState = 3;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < 0x900000 || info.zoneNumber != info.enemyZone)
					item->Animation.TargetState = 1;
				else
					item->Animation.TargetState = 6;
			}
			else if (machinegun->Mood == MoodType::Attack || !info.ahead)
			{
				if (info.distance > 0x400000)
					item->Animation.TargetState = 3;
			}
			else
			{
				item->Animation.TargetState = 4;
			}
			break;

		case 3:
			machinegun->MaxTurn = 910;

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (machinegun->Mood != MoodType::Escape)
			{
				if (Targetable(item, &info))
				{
					item->Animation.TargetState = 2;
				}
				else if (machinegun->Mood == MoodType::Bored || machinegun->Mood == MoodType::Stalk)
				{
					item->Animation.TargetState = 2;
				}
			}
			break;

		case 4:
			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (Targetable(item, &info))
			{
				item->Animation.TargetState = 5;
			}
			else
			{
				if (machinegun->Mood == MoodType::Attack)
				{
					item->Animation.TargetState = 1;
				}
				else if (!info.ahead)
				{
					item->Animation.TargetState = 1;
				}
			}
			break;

		case 8:
		case 10:
			machinegun->Flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (Targetable(item, &info))
			{
				item->Animation.TargetState = (item->Animation.ActiveState == 8) ? 5 : 11;
			}
			else
			{
				item->Animation.TargetState = 1;
			}
			break;

		case 9:
			machinegun->Flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (Targetable(item, &info))
			{
				item->Animation.TargetState = 6;
			}
			else
			{
				item->Animation.TargetState = 2;
			}
			break;

		case 5:
		case 11:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (machinegun->Flags)
			{
				machinegun->Flags--;
			}
			else
			{
				ShotLara(item, &info, &workerMachineGun, torso_y, 30);
				machinegun->FiredWeapon = 1;
				machinegun->Flags = 5;
			}

			if (item->Animation.TargetState != 1 && (machinegun->Mood == MoodType::Escape || info.distance > 0x900000 || !Targetable(item, &info)))
			{
				item->Animation.TargetState = 1;
			}
			break;

		case 6:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (machinegun->Flags)
			{
				machinegun->Flags--;
			}
			else
			{
				ShotLara(item, &info, &workerMachineGun, torso_y, 30);
				machinegun->FiredWeapon = 1;
				machinegun->Flags = 5;
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
