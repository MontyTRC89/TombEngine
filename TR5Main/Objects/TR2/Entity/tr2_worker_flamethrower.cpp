#include "framework.h"
#include "Objects/TR2/Entity/tr2_worker_flamethrower.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/missile.h"
#include "Game/people.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Specific/trmath.h"

BITE_INFO workerFlameThrower = { 0, 250, 32, 9 };

static void Flame(DWORD x, int y, DWORD z, int speed, WORD yrot, WORD roomNumber)
{
	short fx_number;
	short cam_rot;
	FX_INFO* fx;

	fx_number = CreateNewEffect(roomNumber);
	if (fx_number != NO_ITEM)
	{
		fx = &EffectList[fx_number];
		fx->pos.xPos = x;
		fx->pos.yPos = y;
		fx->pos.zPos = z;
		fx->roomNumber = roomNumber;
		//TODO: complete fx parameters
		fx->shade = 14 * 256;
		fx->counter = 40;
		ShootAtLara(fx);
	}
}

void InitialiseWorkerFlamethrower(short itemNum)
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

void WorkerFlamethrower(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* flame;
	AI_INFO info;
	PHD_VECTOR pos;
	short angle, head_y, head_x, torso_y, torso_x, tilt;

	item = &g_Level.Items[itemNum];
	flame = (CREATURE_INFO*)item->data;
	angle = head_y = head_x = torso_y = torso_x = tilt = 0;

	pos.x = workerFlameThrower.x;
	pos.y = workerFlameThrower.y;
	pos.z = workerFlameThrower.z;
	GetJointAbsPosition(item, &pos, workerFlameThrower.meshNum);

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
		if (item->activeState != 5 && item->activeState != 6)
		{
			TriggerDynamicLight(pos.x, pos.y, pos.z, (GetRandomControl() & 4) + 10, (GetRandomControl() & 7) + 128, (GetRandomControl() & 7) + 64, GetRandomControl() & 7);
			AddFire(pos.x, pos.y, pos.z, 0, item->roomNumber, 0);
		}
		else
		{
			TriggerDynamicLight(pos.x, pos.y, pos.z, (GetRandomControl() & 4) + 14, (GetRandomControl() & 7) + 128, (GetRandomControl() & 7) + 64, GetRandomControl() & 7);
		}

		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, flame->maximumTurn);

		switch (item->activeState)
		{
		case 1:
			flame->flags = 0;
			flame->maximumTurn = 0;

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (flame->mood == ESCAPE_MOOD)
			{
				item->targetState = 3;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < SQUARE(WALL_SIZE * 4) || info.zoneNumber != info.enemyZone)
					item->targetState = 8;
				else
					item->targetState = 2;
			}
			else if (flame->mood == ATTACK_MOOD || !info.ahead)
			{
				if (info.distance <= SQUARE(WALL_SIZE * 2))
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
			flame->maximumTurn = ANGLE(5);

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (flame->mood == ESCAPE_MOOD)
			{
				item->targetState = 3;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < SQUARE(WALL_SIZE * 4) || info.zoneNumber != info.enemyZone)
					item->targetState = 1;
				else
					item->targetState = 6;
			}
			else if (flame->mood == ATTACK_MOOD || !info.ahead)
			{
				if (info.distance > SQUARE(WALL_SIZE * 2))
					item->targetState = 3;
			}
			else
			{
				item->targetState = 4;
			}
			break;

		case 3:
			flame->maximumTurn = ANGLE(10);

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (flame->mood != ESCAPE_MOOD)
			{
				if (Targetable(item, &info))
				{
					item->targetState = 2;
				}
				else if (flame->mood == BORED_MOOD || flame->mood == STALK_MOOD)
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
				if (flame->mood == ATTACK_MOOD)
				{
					item->targetState = 1;
				}
				else if (!info.ahead)
				{
					item->targetState = 1;
				}
			}
			break;

		case 5:
		case 6:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}

			if (item->targetState != 1 && (flame->mood == ESCAPE_MOOD || info.distance > SQUARE(WALL_SIZE * 10) || !Targetable(item, &info)))
			{
				item->targetState = 1;
			}
			break;

		case 8:
		case 9:
			flame->flags = 0;

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
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head_y);
	CreatureJoint(item, 3, head_x);
	CreatureAnimation(itemNum, angle, tilt);
}