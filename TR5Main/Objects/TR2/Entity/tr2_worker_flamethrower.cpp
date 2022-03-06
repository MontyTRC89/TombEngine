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
	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 12;

	ClearItem(itemNum);

	anim = &g_Level.Anims[item->AnimNumber];
	item->FrameNumber = anim->frameBase;
	item->ActiveState = anim->ActiveState;
}

void WorkerFlamethrower(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CreatureInfo* flame;
	AI_INFO info;
	PHD_VECTOR pos;
	short angle, head_y, head_x, torso_y, torso_x, tilt;

	item = &g_Level.Items[itemNum];
	flame = (CreatureInfo*)item->Data;
	angle = head_y = head_x = torso_y = torso_x = tilt = 0;

	pos.x = workerFlameThrower.x;
	pos.y = workerFlameThrower.y;
	pos.z = workerFlameThrower.z;
	GetJointAbsPosition(item, &pos, workerFlameThrower.meshNum);

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
		if (item->ActiveState != 5 && item->ActiveState != 6)
		{
			TriggerDynamicLight(pos.x, pos.y, pos.z, (GetRandomControl() & 4) + 10, (GetRandomControl() & 7) + 128, (GetRandomControl() & 7) + 64, GetRandomControl() & 7);
			AddFire(pos.x, pos.y, pos.z, 0, item->RoomNumber, 0);
		}
		else
		{
			TriggerDynamicLight(pos.x, pos.y, pos.z, (GetRandomControl() & 4) + 14, (GetRandomControl() & 7) + 128, (GetRandomControl() & 7) + 64, GetRandomControl() & 7);
		}

		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, flame->MaxTurn);

		switch (item->ActiveState)
		{
		case 1:
			flame->Flags = 0;
			flame->MaxTurn = 0;

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (flame->Mood == MoodType::Escape)
			{
				item->TargetState = 3;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < SQUARE(WALL_SIZE * 4) || info.zoneNumber != info.enemyZone)
					item->TargetState = 8;
				else
					item->TargetState = 2;
			}
			else if (flame->Mood == MoodType::Attack || !info.ahead)
			{
				if (info.distance <= SQUARE(WALL_SIZE * 2))
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
			flame->MaxTurn = ANGLE(5);

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (flame->Mood == MoodType::Escape)
			{
				item->TargetState = 3;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < SQUARE(WALL_SIZE * 4) || info.zoneNumber != info.enemyZone)
					item->TargetState = 1;
				else
					item->TargetState = 6;
			}
			else if (flame->Mood == MoodType::Attack || !info.ahead)
			{
				if (info.distance > SQUARE(WALL_SIZE * 2))
					item->TargetState = 3;
			}
			else
			{
				item->TargetState = 4;
			}
			break;

		case 3:
			flame->MaxTurn = ANGLE(10);

			if (info.ahead)
			{
				head_y = info.angle;
				head_x = info.xAngle;
			}

			if (flame->Mood != MoodType::Escape)
			{
				if (Targetable(item, &info))
				{
					item->TargetState = 2;
				}
				else if (flame->Mood == MoodType::Bored || flame->Mood == MoodType::Stalk)
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
				if (flame->Mood == MoodType::Attack)
				{
					item->TargetState = 1;
				}
				else if (!info.ahead)
				{
					item->TargetState = 1;
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

			if (item->TargetState != 1 && (flame->Mood == MoodType::Escape || info.distance > SQUARE(WALL_SIZE * 10) || !Targetable(item, &info)))
			{
				item->TargetState = 1;
			}
			break;

		case 8:
		case 9:
			flame->Flags = 0;

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
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head_y);
	CreatureJoint(item, 3, head_x);
	CreatureAnimation(itemNum, angle, tilt);
}