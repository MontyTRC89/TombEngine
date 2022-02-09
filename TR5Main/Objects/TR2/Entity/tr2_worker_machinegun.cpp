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
	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 12;

	ClearItem(itemNum);

	anim = &g_Level.Anims[item->AnimNumber];
	item->FrameNumber = anim->frameBase;
	item->ActiveState = anim->ActiveState;
}

void WorkerMachineGunControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* machinegun;
	AI_INFO info;
	short angle, head_y, head_x, torso_y, torso_x, tilt;

	item = &g_Level.Items[itemNum];
	machinegun = (CREATURE_INFO*)item->Data;
	angle = head_y = head_x = torso_y = torso_x = tilt = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != 7)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 19;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = 7;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, machinegun->maximumTurn);

		switch (item->ActiveState)
		{
		case 1:
			machinegun->flags = 0;
			machinegun->maximumTurn = 0;

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (machinegun->mood == ESCAPE_MOOD)
			{
				item->TargetState = 3;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < 0x900000 || info.zoneNumber != info.enemyZone)
					item->TargetState = (GetRandomControl() < 0x4000) ? 8 : 10;
				else
					item->TargetState = 2;
			}
			else if (machinegun->mood == ATTACK_MOOD || !info.ahead)
			{
				if (info.distance <= 0x400000)
					item->TargetState = 2;
				else
					item->TargetState = 3;
			}
			else
			{
				item->TargetState = 4;
			}
			break;

		case 2:
			machinegun->maximumTurn = 546;

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (machinegun->mood == ESCAPE_MOOD)
			{
				item->TargetState = 3;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < 0x900000 || info.zoneNumber != info.enemyZone)
					item->TargetState = 1;
				else
					item->TargetState = 6;
			}
			else if (machinegun->mood == ATTACK_MOOD || !info.ahead)
			{
				if (info.distance > 0x400000)
					item->TargetState = 3;
			}
			else
			{
				item->TargetState = 4;
			}
			break;

		case 3:
			machinegun->maximumTurn = 910;

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (machinegun->mood != ESCAPE_MOOD)
			{
				if (Targetable(item, &info))
				{
					item->TargetState = 2;
				}
				else if (machinegun->mood == BORED_MOOD || machinegun->mood == STALK_MOOD)
				{
					item->TargetState = 2;
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
				item->TargetState = 5;
			}
			else
			{
				if (machinegun->mood == ATTACK_MOOD)
				{
					item->TargetState = 1;
				}
				else if (!info.ahead)
				{
					item->TargetState = 1;
				}
			}
			break;

		case 8:
		case 10:
			machinegun->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (Targetable(item, &info))
			{
				item->TargetState = (item->ActiveState == 8) ? 5 : 11;
			}
			else
			{
				item->TargetState = 1;
			}
			break;

		case 9:
			machinegun->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (Targetable(item, &info))
			{
				item->TargetState = 6;
			}
			else
			{
				item->TargetState = 2;
			}
			break;

		case 5:
		case 11:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (machinegun->flags)
			{
				machinegun->flags--;
			}
			else
			{
				ShotLara(item, &info, &workerMachineGun, torso_y, 30);
				item->FiredWeapon = 1;
				machinegun->flags = 5;
			}

			if (item->TargetState != 1 && (machinegun->mood == ESCAPE_MOOD || info.distance > 0x900000 || !Targetable(item, &info)))
			{
				item->TargetState = 1;
			}
			break;

		case 6:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (machinegun->flags)
			{
				machinegun->flags--;
			}
			else
			{
				ShotLara(item, &info, &workerMachineGun, torso_y, 30);
				item->FiredWeapon = 1;
				machinegun->flags = 5;
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
