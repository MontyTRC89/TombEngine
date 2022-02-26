#include "framework.h"
#include "Objects/TR1/Entity/tr1_natla.h"

#include "Game/control/box.h"
#include "Game/itemdata/creature_info.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Game/missile.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/trmath.h"

#define NATLA_NEAR_DEATH 200

enum NatlaState
{
	NATLA_STATE_NONE,
	NATLA_STATE_IDLE,
	NATLA_STATE_FLY,
	NATLA_STATE_RUN,
	NATLA_STATE_AIM,
	NATLA_STATE_SEMI_DEATH,
	NATLA_STATE_SHOOT,
	NATLA_STATE_FALL,
	NATLA_STATE_STAND,
	NATLA_STATE_DEATH
};

BITE_INFO NatlaGunBite = { 5, 220, 7, 4 };

#define NATLA_FLYMODE 0x8000
#define NATLA_TIMER   0x7fff
#define NATLA_FIRE_ARC ANGLE(30.0f)
#define NATLA_FLY_TURN ANGLE(5.0f)
#define NATLA_RUN_TURN ANGLE(6.0f)
#define NATLA_LAND_CHANCE 0x100
#define NATLA_DEATH_TIME (30 * 16)
#define NATLA_SHOT_DAMAGE 100

void NatlaControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	short head = 0;
	short angle = 0;
	short tilt = 0;
	short facing = 0;
	short gun = info->jointRotation[0] * 7 / 8;

	int shoot;
	short timer = info->flags & NATLA_TIMER;

	AI_INFO AIInfo;

	if (item->HitPoints <= 0 && item->HitPoints > -16384)
		item->TargetState = NATLA_STATE_DEATH;
	else if (item->HitPoints <= NATLA_NEAR_DEATH)
	{
		info->LOT.step = CLICK(1);
		info->LOT.drop = -CLICK(1);
		info->LOT.fly = NO_FLYING;
		CreatureAIInfo(item, &AIInfo);

		if (AIInfo.ahead)
			head = AIInfo.angle;

		GetCreatureMood(item, &AIInfo, VIOLENT);
		CreatureMood(item, &AIInfo, VIOLENT);

		angle = CreatureTurn(item, NATLA_RUN_TURN);
		shoot = (AIInfo.angle > -NATLA_FIRE_ARC && AIInfo.angle < NATLA_FIRE_ARC && Targetable(item, &AIInfo));

		if (facing)
		{
			item->Position.yRot += facing;
			facing = 0;
		}

		switch (item->ActiveState)
		{
		case NATLA_STATE_FALL:
			if (item->Position.yPos < item->Floor)
			{
				item->Velocity = 0;
				item->Airborne = true;
			}
			else
			{
				item->Airborne = 0;
				item->TargetState = NATLA_STATE_SEMI_DEATH;
				item->Position.yPos = item->Floor;
				timer = 0;
			}

			break;

		case NATLA_STATE_STAND:
			if (!shoot)
				item->TargetState = NATLA_STATE_RUN;

			if (timer >= 20)
			{
				short FXNumber = CreatureEffect(item, &NatlaGunBite, ShardGun);
				if (FXNumber != NO_ITEM)
				{
					auto* fx = &EffectList[FXNumber];
					gun = fx->pos.xRot;
					SoundEffect(123, &fx->pos, NULL);
				}

				timer = 0;
			}

			break;

		case NATLA_STATE_RUN:
			tilt = angle;

			if (timer >= 20)
			{
				short FXNumber = CreatureEffect(item, &NatlaGunBite, ShardGun);
				if (FXNumber != NO_ITEM)
				{
					auto* fx = &EffectList[FXNumber];
					gun = fx->pos.xRot;
					SoundEffect(123, &fx->pos, NULL);
				}

				timer = 0;
			}

			if (shoot)
				item->TargetState = NATLA_STATE_STAND;

			break;

		case NATLA_STATE_SEMI_DEATH:
			if (timer == NATLA_DEATH_TIME)
			{
				item->TargetState = NATLA_STATE_STAND;
				info->flags = 0;
				timer = 0;
				item->HitPoints = NATLA_NEAR_DEATH;
			}
			else
				item->HitPoints = -16384;

			break;

		case NATLA_STATE_FLY:
			item->TargetState = NATLA_STATE_FALL;
			timer = 0;
			break;

		case NATLA_STATE_IDLE:
		case NATLA_STATE_SHOOT:
		case NATLA_STATE_AIM:
			item->TargetState = NATLA_STATE_SEMI_DEATH;
			item->Flags = 0;
			timer = 0;
			break;
		}
	}
	else
	{
		info->LOT.step = CLICK(1);
		info->LOT.drop = -CLICK(1);
		info->LOT.fly = NO_FLYING;
		CreatureAIInfo(item, &AIInfo);

		shoot = (AIInfo.angle > -NATLA_FIRE_ARC && AIInfo.angle < NATLA_FIRE_ARC && Targetable(item, &AIInfo));

		if (item->ActiveState == NATLA_STATE_FLY && (info->flags & NATLA_FLYMODE))
		{
			if (info->flags & NATLA_FLYMODE && shoot && GetRandomControl() < NATLA_LAND_CHANCE)
				info->flags -= NATLA_FLYMODE;

			if (!(info->flags & NATLA_FLYMODE))
				CreatureMood(item, &AIInfo, VIOLENT);

			info->LOT.step = SECTOR(20);
			info->LOT.drop = -SECTOR(20);
			info->LOT.fly = CLICK(0.25f) / 2;

			CreatureAIInfo(item, &AIInfo);
		}
		else if (!shoot)
			info->flags |= NATLA_FLYMODE;

		if (AIInfo.ahead)
			head = AIInfo.angle;

		if (item->ActiveState != NATLA_STATE_FLY || (info->flags & NATLA_FLYMODE))
			CreatureMood(item, &AIInfo, TIMID);

		item->Position.yRot -= facing;
		angle = CreatureTurn(item, NATLA_FLY_TURN);

		if (item->ActiveState == NATLA_STATE_FLY)
		{
			if (AIInfo.angle > NATLA_FLY_TURN)
				facing += NATLA_FLY_TURN;
			else if (AIInfo.angle < -NATLA_FLY_TURN)
				facing -= NATLA_FLY_TURN;
			else
				facing += AIInfo.angle;

			item->Position.yRot += facing;
		}
		else
		{
			item->Position.yRot += facing - angle;
			facing = 0;
		}

		switch (item->ActiveState)
		{
		case NATLA_STATE_IDLE:
			timer = 0;

			if (info->flags & NATLA_FLYMODE)
				item->TargetState = NATLA_STATE_FLY;
			else
				item->TargetState = NATLA_STATE_AIM;

			break;

		case NATLA_STATE_FLY:
			if (!(info->flags & NATLA_FLYMODE) && item->Position.yPos == item->Floor)
				item->TargetState = NATLA_STATE_IDLE;

			if (timer >= 30)
			{
				short FXNumber = CreatureEffect(item, &NatlaGunBite, BombGun);
				if (FXNumber != NO_ITEM)
				{
					auto* fx = &EffectList[FXNumber];
					gun = fx->pos.xRot;
					SoundEffect(123, &fx->pos, NULL);
				}

				timer = 0;
			}

			break;

		case NATLA_STATE_AIM:
			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (shoot)
				item->TargetState = NATLA_STATE_SHOOT;
			else
				item->TargetState = NATLA_STATE_IDLE;

			break;

		case NATLA_STATE_SHOOT:
			if (!item->RequiredState)
			{
				short FXNumber = CreatureEffect(item, &NatlaGunBite, BombGun);
				if (FXNumber != NO_ITEM)
					gun = EffectList[FXNumber].pos.xRot;

				FXNumber = CreatureEffect(item, &NatlaGunBite, BombGun);
				if (FXNumber != NO_ITEM)
					EffectList[FXNumber].pos.yRot += (short)((GetRandomControl() - 0x4000) / 4);

				FXNumber = CreatureEffect(item, &NatlaGunBite, BombGun);
				if (FXNumber != NO_ITEM)
					EffectList[FXNumber].pos.yRot += (short)((GetRandomControl() - 0x4000) / 4);

				item->RequiredState = NATLA_STATE_IDLE;
			}

			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, -head);

	if (gun)
		CreatureJoint(item, 0, gun);

	timer++;
	info->flags = (info->flags & NATLA_FLYMODE) + timer;

	item->Position.yRot -= facing;
	CreatureAnimation(itemNumber, angle, tilt);
	item->Position.yRot += facing;
}
