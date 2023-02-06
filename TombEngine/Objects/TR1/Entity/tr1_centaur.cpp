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
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR1
{
	constexpr auto CENTAUR_REAR_DAMAGE = 200;
	constexpr auto CENTAUR_REAR_RANGE = BLOCK(3 / 2.0f);
	constexpr auto CENTAUR_REAR_CHANCE = 1 / 340.0f;
	constexpr auto CENTAUR_BOMB_VELOCITY = CLICK(1);

	constexpr auto CENTAUR_TURN_RATE_MAX = ANGLE(4.0f);

	const auto CentaurRocketBite = BiteInfo(Vector3(11.0f, 415.0f, 41.0f), 13);
	const auto CentaurRearBite	 = BiteInfo(Vector3(50.0f, 30.0f, 0.0f), 5);
	const auto CentaurAttackJoints = std::vector<unsigned int>{ 0, 3, 4, 7, 8, 16, 17 };

	enum CentaurState
	{
		// No state 0.
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

		short angle = 0;
		short head = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != CENTAUR_STATE_DEATH)
				SetAnimation(item, CENTAUR_ANIM_DEATH);
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
				if (item->Animation.RequiredState != NO_STATE)
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
					item->Animation.TargetState = CENTAUR_STATE_IDLE;
					item->Animation.RequiredState = CENTAUR_STATE_WARNING;
				}
				else if (Targetable(item, &AI))
				{
					item->Animation.TargetState = CENTAUR_STATE_IDLE;
					item->Animation.RequiredState = CENTAUR_STATE_AIM;
				}
				else if (Random::TestProbability(CENTAUR_REAR_CHANCE))
				{
					item->Animation.TargetState = CENTAUR_STATE_IDLE;
					item->Animation.RequiredState = CENTAUR_STATE_WARNING;
				}

				break;

			case CENTAUR_STATE_AIM:
				if (item->Animation.RequiredState != NO_STATE)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (Targetable(item, &AI))
					item->Animation.TargetState = CENTAUR_PROJECTILE_ATTACK;
				else
					item->Animation.TargetState = CENTAUR_STATE_IDLE;

				break;

			case CENTAUR_PROJECTILE_ATTACK:
				if (item->Animation.RequiredState == NO_STATE)
				{
					item->Animation.RequiredState = CENTAUR_STATE_AIM;
					CreatureEffect2(item, CentaurRocketBite, CENTAUR_BOMB_VELOCITY, head, BombGun);
				}

				break;

			case CENTAUR_STATE_WARNING:
				if (item->Animation.RequiredState == NO_STATE &&
					item->TouchBits.Test(CentaurAttackJoints))
				{
					DoDamage(creature->Enemy, CENTAUR_REAR_DAMAGE);
					CreatureEffect(item, CentaurRearBite, DoBloodSplat);
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
