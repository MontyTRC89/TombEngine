#include "framework.h"
#include "tr2_worker_flamethrower.h"
#include "box.h"
#include "sphere.h"
#include "effect2.h"
#include "people.h"
#include "items.h"
#include "missile.h"
#include "tomb4fx.h"
#include "setup.h"
#include "level.h"
#include "effect.h"
#include "trmath.h"

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
		/*phd_GetVectorAngles(fx->pos.xPos - Camera.pos.x,
							fx->pos.yPos - Camera.pos.y,
							fx->pos.zPos - Camera.pos.z, &cam_rot);
		fx->pos.x_rot = fx->pos.z_rot = 0;
		fx->pos.y_rot = cam_rot;
		fx->speed = 200;*/
		//fx->objectNumber = Utils.getObjects(TR2_DRAGON_FIRE);
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
	item->currentAnimState = anim->currentAnimState;
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

	// get the exact flame start position
	pos.x = workerFlameThrower.x;
	pos.y = workerFlameThrower.y;
	pos.z = workerFlameThrower.z;
	GetJointAbsPosition(item, &pos, workerFlameThrower.meshNum);

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 7)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 19;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = 7;
		}
	}
	else
	{
		if (item->currentAnimState != 5 && item->currentAnimState != 6)
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

		switch (item->currentAnimState)
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
				item->goalAnimState = 3;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < SQUARE(WALL_SIZE * 4) || info.zoneNumber != info.enemyZone)
					item->goalAnimState = 8;
				else
					item->goalAnimState = 2;
			}
			else if (flame->mood == ATTACK_MOOD || !info.ahead)
			{
				if (info.distance <= SQUARE(WALL_SIZE * 2))
					item->goalAnimState = 2;
				else
					item->goalAnimState = 3;
			}
			else
			{
				item->goalAnimState = 4;
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
				item->goalAnimState = 3;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < SQUARE(WALL_SIZE * 4) || info.zoneNumber != info.enemyZone)
					item->goalAnimState = 1;
				else
					item->goalAnimState = 6;
			}
			else if (flame->mood == ATTACK_MOOD || !info.ahead)
			{
				if (info.distance > SQUARE(WALL_SIZE * 2))
					item->goalAnimState = 3;
			}
			else
			{
				item->goalAnimState = 4;
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
					item->goalAnimState = 2;
				}
				else if (flame->mood == BORED_MOOD || flame->mood == STALK_MOOD)
				{
					item->goalAnimState = 2;
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
				item->goalAnimState = 5;
			}
			else
			{
				if (flame->mood == ATTACK_MOOD)
				{
					item->goalAnimState = 1;
				}
				else if (!info.ahead)
				{
					item->goalAnimState = 1;
				}
			}
			break;

		case 5:
		case 6:
			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;

				//CreatureEffect(item, &workerFlameThrower, Flame);
			}

			if (item->goalAnimState != 1 && (flame->mood == ESCAPE_MOOD || info.distance > SQUARE(WALL_SIZE * 10) || !Targetable(item, &info)))
			{
				item->goalAnimState = 1;
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
				item->goalAnimState = (item->currentAnimState == 8) ? 5 : 11;
			}
			else
			{
				item->goalAnimState = 1;
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