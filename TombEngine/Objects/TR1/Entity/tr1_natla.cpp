#include "framework.h"
#include "tr1_natla.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Game/missile.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/trmath.h"

namespace TEN::Entities::TR1
{
	// TODO: Organise.
	constexpr auto NATLA_NEAR_DEATH = 200;
	constexpr auto NATLA_FLYMODE = 0x8000;
	constexpr auto NATLA_TIMER = 0x7FFF;
	constexpr auto NATLA_LAND_CHANCE = 0x100;
	constexpr auto NATLA_DEATH_TIME = (FPS * 16);	// 16 seconds.
	constexpr auto NATLA_SHOT_DAMAGE = 100;

	const auto NatlaGunBite = BiteInfo(Vector3(5.0f, 220.0f, 7.0f), 4);

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

	void NatlaControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short head = 0;
		short angle = 0;
		short tilt = 0;
		short facing = 0;
		short gun = creature->JointRotation[0] * 7 / 8;

		int shoot;
		short timer = creature->Flags & NATLA_TIMER;

		AI_INFO AI;

		if (item->HitPoints <= 0 && item->HitPoints != NOT_TARGETABLE)
			item->Animation.TargetState = NATLA_STATE_DEATH;
		else if (item->HitPoints <= NATLA_NEAR_DEATH)
		{
			creature->LOT.Step = CLICK(1);
			creature->LOT.Drop = -CLICK(1);
			creature->LOT.Fly = NO_FLYING;
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, ANGLE(6.0f));
			shoot = (AI.angle > -ANGLE(30.0f) && AI.angle < ANGLE(30.0f) && Targetable(item, &AI));

			if (facing)
			{
				item->Pose.Orientation.y += facing;
				facing = 0;
			}

			switch (item->Animation.ActiveState)
			{
			case NATLA_STATE_FALL:
				if (item->Pose.Position.y < item->Floor)
				{
					item->Animation.IsAirborne = true;
					item->Animation.Velocity = 0;
				}
				else
				{
					item->Animation.TargetState = NATLA_STATE_SEMI_DEATH;
					item->Animation.IsAirborne = 0;
					item->Pose.Position.y = item->Floor;
					timer = 0;
				}

				break;

			case NATLA_STATE_STAND:
				if (!shoot)
					item->Animation.TargetState = NATLA_STATE_RUN;

				if (timer >= 20)
				{
					short FXNumber = CreatureEffect(item, NatlaGunBite, ShardGun);
					if (FXNumber != NO_ITEM)
					{
						auto* fx = &EffectList[FXNumber];
						gun = fx->pos.Orientation.x;
						SoundEffect(SFX_TR1_ATLANTEAN_BALL, &fx->pos);
					}

					timer = 0;
				}

				break;

			case NATLA_STATE_RUN:
				tilt = angle;

				if (timer >= 20)
				{
					short FXNumber = CreatureEffect(item, NatlaGunBite, ShardGun);
					if (FXNumber != NO_ITEM)
					{
						auto* fx = &EffectList[FXNumber];
						gun = fx->pos.Orientation.x;
						SoundEffect(SFX_TR1_ATLANTEAN_BALL, &fx->pos);
					}

					timer = 0;
				}

				if (shoot)
					item->Animation.TargetState = NATLA_STATE_STAND;

				break;

			case NATLA_STATE_SEMI_DEATH:
				if (timer == NATLA_DEATH_TIME)
				{
					item->Animation.TargetState = NATLA_STATE_STAND;
					creature->Flags = 0;
					timer = 0;
					item->HitPoints = NATLA_NEAR_DEATH;
				}
				else
					item->HitPoints = NOT_TARGETABLE;

				break;

			case NATLA_STATE_FLY:
				item->Animation.TargetState = NATLA_STATE_FALL;
				timer = 0;
				break;

			case NATLA_STATE_IDLE:
			case NATLA_STATE_SHOOT:
			case NATLA_STATE_AIM:
				item->Animation.TargetState = NATLA_STATE_SEMI_DEATH;
				item->Flags = 0;
				timer = 0;
				break;
			}
		}
		else
		{
			creature->LOT.Step = CLICK(1);
			creature->LOT.Drop = -CLICK(1);
			creature->LOT.Fly = NO_FLYING;
			CreatureAIInfo(item, &AI);

			shoot = (AI.angle > -ANGLE(30.0f) && AI.angle < ANGLE(30.0f) && Targetable(item, &AI));

			if (item->Animation.ActiveState == NATLA_STATE_FLY && (creature->Flags & NATLA_FLYMODE))
			{
				if (creature->Flags & NATLA_FLYMODE && shoot && GetRandomControl() < NATLA_LAND_CHANCE)
					creature->Flags -= NATLA_FLYMODE;

				if (!(creature->Flags & NATLA_FLYMODE))
					CreatureMood(item, &AI, true);

				creature->LOT.Step = SECTOR(20);
				creature->LOT.Drop = -SECTOR(20);
				creature->LOT.Fly = CLICK(0.25f) / 2;

				CreatureAIInfo(item, &AI);
			}
			else if (!shoot)
				creature->Flags |= NATLA_FLYMODE;

			if (AI.ahead)
				head = AI.angle;

			if (item->Animation.ActiveState != NATLA_STATE_FLY || (creature->Flags & NATLA_FLYMODE))
				CreatureMood(item, &AI, false);

			item->Pose.Orientation.y -= facing;
			angle = CreatureTurn(item, ANGLE(5.0f));

			if (item->Animation.ActiveState == NATLA_STATE_FLY)
			{
				if (AI.angle > ANGLE(5.0f))
					facing += ANGLE(5.0f);
				else if (AI.angle < -ANGLE(5.0f))
					facing -= ANGLE(5.0f);
				else
					facing += AI.angle;

				item->Pose.Orientation.y += facing;
			}
			else
			{
				item->Pose.Orientation.y += facing - angle;
				facing = 0;
			}

			switch (item->Animation.ActiveState)
			{
			case NATLA_STATE_IDLE:
				timer = 0;

				if (creature->Flags & NATLA_FLYMODE)
					item->Animation.TargetState = NATLA_STATE_FLY;
				else
					item->Animation.TargetState = NATLA_STATE_AIM;

				break;

			case NATLA_STATE_FLY:
				if (!(creature->Flags & NATLA_FLYMODE) && item->Pose.Position.y == item->Floor)
					item->Animation.TargetState = NATLA_STATE_IDLE;

				if (timer >= 30)
				{
					short FXNumber = CreatureEffect(item, NatlaGunBite, BombGun);
					if (FXNumber != NO_ITEM)
					{
						auto* fx = &EffectList[FXNumber];
						gun = fx->pos.Orientation.x;
						SoundEffect(SFX_TR1_ATLANTEAN_WINGS, &fx->pos);
					}

					timer = 0;
				}

				break;

			case NATLA_STATE_AIM:
				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (shoot)
					item->Animation.TargetState = NATLA_STATE_SHOOT;
				else
					item->Animation.TargetState = NATLA_STATE_IDLE;

				break;

			case NATLA_STATE_SHOOT:
				if (!item->Animation.RequiredState)
				{
					short FXNumber = CreatureEffect(item, NatlaGunBite, BombGun);
					if (FXNumber != NO_ITEM)
						gun = EffectList[FXNumber].pos.Orientation.x;

					FXNumber = CreatureEffect(item, NatlaGunBite, BombGun);
					if (FXNumber != NO_ITEM)
						EffectList[FXNumber].pos.Orientation.y += (short)((GetRandomControl() - 0x4000) / 4);

					FXNumber = CreatureEffect(item, NatlaGunBite, BombGun);
					if (FXNumber != NO_ITEM)
						EffectList[FXNumber].pos.Orientation.y += (short)((GetRandomControl() - 0x4000) / 4);

					item->Animation.RequiredState = NATLA_STATE_IDLE;
				}

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, -head);

		if (gun)
			CreatureJoint(item, 0, gun);

		timer++;
		creature->Flags = (creature->Flags & NATLA_FLYMODE) + timer;

		item->Pose.Orientation.y -= facing;
		CreatureAnimation(itemNumber, angle, tilt);
		item->Pose.Orientation.y += facing;
	}
}
