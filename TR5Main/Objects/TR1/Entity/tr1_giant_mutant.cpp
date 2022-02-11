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
	abort = (CREATURE_INFO*)item->Data;
	head = angle = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != ABORT_DEATH)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + ABORT_DIE_ANIM;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = ABORT_DEATH;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = (short)phd_atan(abort->target.z - item->Position.zPos, abort->target.x - item->Position.xPos) - item->Position.yRot;

		if (item->TouchBits)
		{
			LaraItem->HitPoints -= ABORT_TOUCH_DAMAGE;
			LaraItem->HitStatus = true;
		}

		switch (item->ActiveState)
		{
		case ABORT_SET:
			item->TargetState = ABORT_FALL;
			item->Airborne = true;
			break;

		case ABORT_STOP:
			if (LaraItem->HitPoints <= 0)
				break;

			abort->flags = 0;
			if (angle > ABORT_NEED_TURN)
				item->TargetState = ABORT_TURNR;
			else if (angle < -ABORT_NEED_TURN)
				item->TargetState = ABORT_TURNL;
			else if (info.distance < ABORT_ATTACK_RANGE)
			{
				if (LaraItem->HitPoints <= ABORT_ATTACK_DAMAGE)
				{
					if (info.distance < ABORT_CLOSE_RANGE)
						item->TargetState = ABORT_ATTACK3;
					else
						item->TargetState = ABORT_FORWARD;
				}
				else if (GetRandomControl() < 0x4000)
					item->TargetState = ABORT_ATTACK1;
				else
					item->TargetState = ABORT_ATTACK2;
			}
			else
				item->TargetState = ABORT_FORWARD;
			break;

		case ABORT_FORWARD:
			if (angle < -ABORT_TURN)
				item->TargetState -= ABORT_TURN;
			else if (angle > ABORT_TURN)
				item->TargetState += ABORT_TURN;
			else
				item->TargetState += angle;

			if (angle > ABORT_NEED_TURN || angle < -ABORT_NEED_TURN)
				item->TargetState = ABORT_STOP;
			else if (info.distance < ABORT_ATTACK_RANGE)
				item->TargetState = ABORT_STOP;
			break;

		case ABORT_TURNR:
			if (!abort->flags)
				abort->flags = item->FrameNumber;
			else if (item->FrameNumber - abort->flags > 16 && item->FrameNumber - abort->flags < 23)
				item->Position.yRot += ANGLE(14);

			if (angle < ABORT_NEED_TURN)
				item->TargetState = ABORT_STOP;
			break;

		case ABORT_TURNL:
			if (!abort->flags)
				abort->flags = item->FrameNumber;
			else if (item->FrameNumber - abort->flags > 13 && item->FrameNumber - abort->flags < 23)
				item->Position.yRot -= ANGLE(9);

			if (angle > -ABORT_NEED_TURN)
				item->TargetState = ABORT_STOP;
			break;

		case ABORT_ATTACK1:
			if (!abort->flags && (item->TouchBits & ABORT_TRIGHT))
			{
				LaraItem->HitPoints -= ABORT_ATTACK_DAMAGE;
				LaraItem->HitStatus = true;
				abort->flags = 1;
			}
			break;

		case ABORT_ATTACK2:
			if (!abort->flags && (item->TouchBits & ABORT_TOUCH))
			{
				LaraItem->HitPoints -= ABORT_ATTACK_DAMAGE;
				LaraItem->HitStatus = true;
				abort->flags = 1;
			}
			break;

		case ABORT_ATTACK3:
			if (item->TouchBits & ABORT_TRIGHT || LaraItem->HitPoints <= 0)
			{
				item->TargetState = ABORT_KILL;

				LaraItem->AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex;
				LaraItem->FrameNumber = g_Level.Anims[LaraItem->AnimNumber].frameBase;
				LaraItem->ActiveState = LaraItem->TargetState = 46;
				LaraItem->RoomNumber = item->RoomNumber;
				LaraItem->Position.xPos = item->Position.xPos;
				LaraItem->Position.yPos = item->Position.yPos;
				LaraItem->Position.zPos = item->Position.zPos;
				LaraItem->Position.yRot = item->Position.yRot;
				LaraItem->Position.xRot = LaraItem->Position.zRot = 0;
				LaraItem->Airborne = false;
				LaraItem->HitPoints = -1;
				Lara.Air = -1;
				Lara.Control.HandStatus = HandStatus::Busy;
				Lara.Control.WeaponControl.GunType = WEAPON_NONE;

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

	if (item->ActiveState == ABORT_FALL)
	{
		AnimateItem(item);

		if (item->Position.yPos > item->Floor)
		{
			item->TargetState = ABORT_STOP;
			item->Airborne = false;
			item->Position.yPos = item->Floor;
			Camera.bounce = 500;
		}
	}
	else
		CreatureAnimation(itemNum, 0, 0);

	if (item->Status == ITEM_DEACTIVATED)
	{
		SoundEffect(171, &item->Position, NULL);
		ExplodingDeath(itemNum, 0xffffffff, ABORT_PART_DAMAGE);

		TestTriggers(item, true);

		KillItem(itemNum);
		item->Status = ITEM_DEACTIVATED;
	}
}