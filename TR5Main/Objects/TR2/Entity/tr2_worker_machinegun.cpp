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
	item->animNumber = Objects[item->objectNumber].animIndex + 12;

	ClearItem(itemNum);

	anim = &g_Level.Anims[item->animNumber];
	item->frameNumber = anim->frameBase;
	item->activeState = anim->activeState;
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
	machinegun = (CREATURE_INFO*)item->data;
	angle = head_y = head_x = torso_y = torso_x = tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->activeState != 7)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 19;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->activeState = 7;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, machinegun->maximumTurn);

		switch (item->activeState)
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
				item->targetState = 3;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < 0x900000 || info.zoneNumber != info.enemyZone)
					item->targetState = (GetRandomControl() < 0x4000) ? 8 : 10;
				else
					item->targetState = 2;
			}
			else if (machinegun->mood == ATTACK_MOOD || !info.ahead)
			{
				if (info.distance <= 0x400000)
					item->targetState = 2;
				else
					item->targetState = 3;
			}
			else
			{
				item->targetState = 4;
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
				item->targetState = 3;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < 0x900000 || info.zoneNumber != info.enemyZone)
					item->targetState = 1;
				else
					item->targetState = 6;
			}
			else if (machinegun->mood == ATTACK_MOOD || !info.ahead)
			{
				if (info.distance > 0x400000)
					item->targetState = 3;
			}
			else
			{
				item->targetState = 4;
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
					item->targetState = 2;
				}
				else if (machinegun->mood == BORED_MOOD || machinegun->mood == STALK_MOOD)
				{
					item->targetState = 2;
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
				item->targetState = 5;
			}
			else
			{
				if (machinegun->mood == ATTACK_MOOD)
				{
					item->targetState = 1;
				}
				else if (!info.ahead)
				{
					item->targetState = 1;
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
				item->targetState = (item->activeState == 8) ? 5 : 11;
			}
			else
			{
				item->targetState = 1;
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
				item->targetState = 6;
			}
			else
			{
				item->targetState = 2;
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
				item->firedWeapon = 1;
				machinegun->flags = 5;
			}

			if (item->targetState != 1 && (machinegun->mood == ESCAPE_MOOD || info.distance > 0x900000 || !Targetable(item, &info)))
			{
				item->targetState = 1;
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
				item->firedWeapon = 1;
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
