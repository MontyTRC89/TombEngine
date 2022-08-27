#include "framework.h"
#include "Objects/TR1/Entity/tr1_centaur.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/misc.h"
#include "Game/missile.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using std::vector;

namespace TEN::Entities::TR1
{
	constexpr auto CENTAUR_REAR_DAMAGE	 = 200;
	constexpr auto CENTAUR_REAR_RANGE	 = SECTOR(1.5f);
	constexpr auto CENTAUR_REAR_CHANCE	 = 0x60;
	constexpr auto CENTAUR_BOMB_VELOCITY = 20;

	#define CENTAUR_TURN_RATE_MAX Angle::DegToRad(4.0f)

	const auto CentaurRocketBite = BiteInfo(Vector3(11.0f, 415.0f, 41.0f), 13);
	const auto CentaurRearBite = BiteInfo(Vector3(50.0f, 30.0f, 0.0f), 5);
	const vector<int> CentaurAttackJoints = { 0, 3, 4, 7, 8, 16, 17 };

	enum CentaurState
	{
		CENTAUR_STATE_NONE = 0,
		CENTAUR_STATE_IDLE = 1,
		CENTAUR_PROJECTILE_ATTACK = 2,
		CENTAUR_STATE_RUN_FORWARD = 3,
		CENTAUR_STATE_AIM = 4,
		CENTAUR_STATE_DEATH = 5,
		CENTAUR_STATE_WARNING = 6
	};

	// TODO
	enum CentaurAnim
	{
		CENTAUR_ANIM_DEATH = 8,
	};

	void CentaurControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);
		short head = 0;
		short angle = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != CENTAUR_STATE_DEATH)
			{
				item->Animation.AnimNumber = Objects[ID_CENTAUR_MUTANT].animIndex + CENTAUR_ANIM_DEATH;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = CENTAUR_STATE_DEATH;
			}
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;

			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, CENTAUR_TURN_RATE_MAX);

			switch (item->Animation.ActiveState)
			{
			case CENTAUR_STATE_IDLE:
				CreatureJoint(item, 17, 0);
				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (AI.bite && AI.distance < pow(CENTAUR_REAR_RANGE, 2))
					item->Animation.TargetState = CENTAUR_STATE_RUN_FORWARD;
				else if (Targetable(item, &AI))
					item->Animation.TargetState = CENTAUR_STATE_AIM;
				else
					item->Animation.TargetState = CENTAUR_STATE_RUN_FORWARD;

				break;

			case CENTAUR_STATE_RUN_FORWARD:
				if (AI.bite && AI.distance < pow(CENTAUR_REAR_RANGE, 2))
				{
					item->Animation.RequiredState = CENTAUR_STATE_WARNING;
					item->Animation.TargetState = CENTAUR_STATE_IDLE;
				}
				else if (Targetable(item, &AI))
				{
					item->Animation.RequiredState = CENTAUR_STATE_AIM;
					item->Animation.TargetState = CENTAUR_STATE_IDLE;
				}
				else if (GetRandomControl() < CENTAUR_REAR_CHANCE)
				{
					item->Animation.RequiredState = CENTAUR_STATE_WARNING;
					item->Animation.TargetState = CENTAUR_STATE_IDLE;
				}

				break;

			case CENTAUR_STATE_AIM:
				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (Targetable(item, &AI))
					item->Animation.TargetState = CENTAUR_PROJECTILE_ATTACK;
				else
					item->Animation.TargetState = CENTAUR_STATE_IDLE;

				break;

			case CENTAUR_PROJECTILE_ATTACK:
				if (!item->Animation.RequiredState)
				{
					item->Animation.RequiredState = CENTAUR_STATE_AIM;
					CreatureEffect2(item, CentaurRocketBite, CENTAUR_BOMB_VELOCITY, head, BombGun);
				}

				break;

			case CENTAUR_STATE_WARNING:
				if (!item->Animation.RequiredState &&
					item->TestBits(JointBitType::Touch, CentaurAttackJoints))
				{
					CreatureEffect(item, CentaurRearBite, DoBloodSplat);
					DoDamage(creature->Enemy, CENTAUR_REAR_DAMAGE);
					item->Animation.RequiredState = CENTAUR_STATE_IDLE;
				}

				break;
			}
		}

		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNumber, angle, 0);

		if (item->Status == ITEM_DEACTIVATED)
		{
			SoundEffect(SFX_TR1_ATLANTEAN_DEATH, &item->Pose);
			ExplodingDeath(itemNumber, BODY_EXPLODE);
			KillItem(itemNumber);
			item->Status = ITEM_DEACTIVATED;
		}
	}
}
