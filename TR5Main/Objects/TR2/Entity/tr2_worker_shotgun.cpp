#include "framework.h"
#include "Objects/TR2/Entity/tr2_worker_shotgun.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/people.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO workerShotgun = { 0, 281, 40, 9 };

static void ShotLara_WithShotgun(ITEM_INFO* item, AI_INFO* info, BITE_INFO* bite, short angle_y, int damage)
{
	ShotLara(item, info, bite, angle_y, damage);
	ShotLara(item, info, bite, angle_y, damage);
	ShotLara(item, info, bite, angle_y, damage);
	ShotLara(item, info, bite, angle_y, damage);
	ShotLara(item, info, bite, angle_y, damage);
	ShotLara(item, info, bite, angle_y, damage);
}

void InitialiseWorkerShotgun(short itemNum)
{
	ANIM_STRUCT* anim;
	ITEM_INFO* item;
	item = &g_Level.Items[itemNum];
	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 5;

	ClearItem(itemNum);

	anim = &g_Level.Anims[item->AnimNumber];
	item->FrameNumber = anim->frameBase;
	item->ActiveState = anim->ActiveState;
}

void WorkerShotgunControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* shotgun;
	AI_INFO info;
	short angle, head_y, head_x, torso_y, torso_x, tilt;

	item = &g_Level.Items[itemNum];
	shotgun = (CREATURE_INFO*)item->Data;
	angle = head_y = head_x = torso_y = torso_x = tilt = 0;

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
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, shotgun->maximumTurn);

		switch (item->ActiveState)
		{
		case 2:
			shotgun->flags = 0;
			shotgun->maximumTurn = 0;

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (shotgun->mood == ESCAPE_MOOD)
			{
				item->TargetState = 5;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance <= 0x900000 || info.zoneNumber != info.enemyZone)
				{
					item->TargetState = (GetRandomControl() >= 0x4000) ? 9 : 8;
				}
				else
				{
					item->TargetState = 1;
				}
			}
			else if (shotgun->mood == ATTACK_MOOD || !info.ahead)
			{
				item->TargetState = (info.distance <= 0x400000) ? 1 : 5;
			}
			else
			{
				item->TargetState = 3;
			}
			break;

		case 3:
			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (Targetable(item, &info))
			{
				item->TargetState = 4;
			}
			else if (shotgun->mood == ATTACK_MOOD || !info.ahead)
			{
				item->TargetState = 2;
			}
			break;

		case 1:
			shotgun->maximumTurn = 546;

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (shotgun->mood == ESCAPE_MOOD)
			{
				item->TargetState = 5;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < 0x900000 || info.zoneNumber != info.enemyZone)
					item->TargetState = 2;
				else
					item->TargetState = 6;
			}
			else if (shotgun->mood == ATTACK_MOOD || !info.ahead)
			{
				if (info.distance > 0x400000)
					item->TargetState = 5;
			}
			else
			{
				item->TargetState = 2;
			}
			break;

		case 5:
			shotgun->maximumTurn = 910;
			tilt = (angle / 2);

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (shotgun->mood != ESCAPE_MOOD)
			{
				if (Targetable(item, &info))
				{
					item->TargetState = 1;
				}
				else if (shotgun->mood == BORED_MOOD || shotgun->mood == STALK_MOOD)
				{
					item->TargetState = 1;
				}
			}
			break;

		case 8:
		case 9:
			shotgun->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (Targetable(item, &info))
			{
				item->TargetState = (item->ActiveState == 8) ? 4 : 10;
			}
			break;

		case 4:
		case 10:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (!shotgun->flags)
			{
				ShotLara_WithShotgun(item, &info, &workerShotgun, torso_y, 25);
				item->FiredWeapon = 2;
				shotgun->flags = 1;
			}

			if (item->ActiveState == 4 && item->TargetState != 2 && (shotgun->mood == ESCAPE_MOOD || info.distance > 0x900000 || !Targetable(item, &info)))
			{
				item->TargetState = 2;
			}
			break;

		case 6:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (!shotgun->flags)
			{
				ShotLara_WithShotgun(item, &info, &workerShotgun, torso_y, 25);
				item->FiredWeapon = 2;
				shotgun->flags = 1;
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