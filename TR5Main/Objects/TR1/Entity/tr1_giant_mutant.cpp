#include "framework.h"
#include "Objects/TR1/Entity/tr1_giant_mutant.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

enum abortion_anims {
	ABORT_EMPTY, ABORT_STOP, ABORT_TURNL, ABORT_TURNR, ABORT_ATTACK1, ABORT_ATTACK2,
	ABORT_ATTACK3, ABORT_FORWARD, ABORT_SET, ABORT_FALL, ABORT_DEATH, ABORT_KILL
};

#define ABORT_NEED_TURN ANGLE(45)
#define ABORT_TURN ANGLE(3)
#define ABORT_ATTACK_RANGE SQUARE(2600)
#define ABORT_CLOSE_RANGE SQUARE(2250)
#define ABORT_ATTACK1_CHANCE 11000
#define ABORT_ATTACK2_CHANCE 22000
#define ABORT_TLEFT 0x7ff0
#define ABORT_TRIGHT 0x3ff8000
#define ABORT_TOUCH	(ABORT_TLEFT|ABORT_TRIGHT)
#define ABORT_PART_DAMAGE 250
#define ABORT_ATTACK_DAMAGE 500
#define ABORT_TOUCH_DAMAGE 5
#define ABORT_DIE_ANIM 13

void AbortionControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item;
	CREATURE_INFO* abort;
	AI_INFO info;
	FLOOR_INFO* floor;
	short head, angle;

	item = &g_Level.Items[itemNum];
	abort = (CREATURE_INFO*)item->data;
	head = angle = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != ABORT_DEATH)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + ABORT_DIE_ANIM;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = ABORT_DEATH;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = (short)phd_atan(abort->target.z - item->pos.zPos, abort->target.x - item->pos.xPos) - item->pos.yRot;

		if (item->touchBits)
		{
			LaraItem->hitPoints -= ABORT_TOUCH_DAMAGE;
			LaraItem->hitStatus = true;
		}

		switch (item->currentAnimState)
		{
		case ABORT_SET:
			item->goalAnimState = ABORT_FALL;
			item->gravityStatus = true;
			break;

		case ABORT_STOP:
			if (LaraItem->hitPoints <= 0)
				break;

			abort->flags = 0;
			if (angle > ABORT_NEED_TURN)
				item->goalAnimState = ABORT_TURNR;
			else if (angle < -ABORT_NEED_TURN)
				item->goalAnimState = ABORT_TURNL;
			else if (info.distance < ABORT_ATTACK_RANGE)
			{
				if (LaraItem->hitPoints <= ABORT_ATTACK_DAMAGE)
				{
					if (info.distance < ABORT_CLOSE_RANGE)
						item->goalAnimState = ABORT_ATTACK3;
					else
						item->goalAnimState = ABORT_FORWARD;
				}
				else if (GetRandomControl() < 0x4000)
					item->goalAnimState = ABORT_ATTACK1;
				else
					item->goalAnimState = ABORT_ATTACK2;
			}
			else
				item->goalAnimState = ABORT_FORWARD;
			break;

		case ABORT_FORWARD:
			if (angle < -ABORT_TURN)
				item->goalAnimState -= ABORT_TURN;
			else if (angle > ABORT_TURN)
				item->goalAnimState += ABORT_TURN;
			else
				item->goalAnimState += angle;

			if (angle > ABORT_NEED_TURN || angle < -ABORT_NEED_TURN)
				item->goalAnimState = ABORT_STOP;
			else if (info.distance < ABORT_ATTACK_RANGE)
				item->goalAnimState = ABORT_STOP;
			break;

		case ABORT_TURNR:
			if (!abort->flags)
				abort->flags = item->frameNumber;
			else if (item->frameNumber - abort->flags > 16 && item->frameNumber - abort->flags < 23)
				item->pos.yRot += ANGLE(14);

			if (angle < ABORT_NEED_TURN)
				item->goalAnimState = ABORT_STOP;
			break;

		case ABORT_TURNL:
			if (!abort->flags)
				abort->flags = item->frameNumber;
			else if (item->frameNumber - abort->flags > 13 && item->frameNumber - abort->flags < 23)
				item->pos.yRot -= ANGLE(9);

			if (angle > -ABORT_NEED_TURN)
				item->goalAnimState = ABORT_STOP;
			break;

		case ABORT_ATTACK1:
			if (!abort->flags && (item->touchBits & ABORT_TRIGHT))
			{
				LaraItem->hitPoints -= ABORT_ATTACK_DAMAGE;
				LaraItem->hitStatus = true;
				abort->flags = 1;
			}
			break;

		case ABORT_ATTACK2:
			if (!abort->flags && (item->touchBits & ABORT_TOUCH))
			{
				LaraItem->hitPoints -= ABORT_ATTACK_DAMAGE;
				LaraItem->hitStatus = true;
				abort->flags = 1;
			}
			break;

		case ABORT_ATTACK3:
			if (item->touchBits & ABORT_TRIGHT || LaraItem->hitPoints <= 0)
			{
				item->goalAnimState = ABORT_KILL;

				LaraItem->animNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex;
				LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
				LaraItem->currentAnimState = LaraItem->goalAnimState = 46;
				LaraItem->roomNumber = item->roomNumber;
				LaraItem->pos.xPos = item->pos.xPos;
				LaraItem->pos.yPos = item->pos.yPos;
				LaraItem->pos.zPos = item->pos.zPos;
				LaraItem->pos.yRot = item->pos.yRot;
				LaraItem->pos.xRot = LaraItem->pos.zRot = 0;
				LaraItem->gravityStatus = false;
				LaraItem->hitPoints = -1;
				Lara.air = -1;
				Lara.gunStatus = LG_HANDS_BUSY;
				Lara.gunType = WEAPON_NONE;

				Camera.targetDistance = SECTOR(2);
				Camera.flags = CF_FOLLOW_CENTER;
			}
			break;

		case ABORT_KILL:
			Camera.targetDistance = SECTOR(2);
			Camera.flags = CF_FOLLOW_CENTER;
			break;
		}
	}

	CreatureJoint(item, 0, head);

	if (item->currentAnimState == ABORT_FALL)
	{
		AnimateItem(item);

		if (item->pos.yPos > item->floor)
		{
			item->goalAnimState = ABORT_STOP;
			item->gravityStatus = false;
			item->pos.yPos = item->floor;
			Camera.bounce = 500;
		}
	}
	else
		CreatureAnimation(itemNum, 0, 0);

	if (item->status == ITEM_DEACTIVATED)
	{
		SoundEffect(171, &item->pos, NULL);
		ExplodingDeath(itemNum, 0xffffffff, ABORT_PART_DAMAGE);

		TestTriggers(item, true);

		KillItem(itemNum);
		item->status = ITEM_DEACTIVATED;
	}
}