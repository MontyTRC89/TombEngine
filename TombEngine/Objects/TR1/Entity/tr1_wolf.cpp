#include "framework.h"
#include "Objects/TR1/Entity/tr1_wolf.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Specific/level.h"

using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR1
{
	constexpr auto WOLF_BITE_DAMAGE	 = 100;
	constexpr auto WOLF_LUNGE_DAMAGE = 50;

	constexpr auto WOLF_ATTACK_RANGE = SQUARE(BLOCK(1.5f));
	constexpr auto WOLF_STALK_RANGE	 = SQUARE(BLOCK(2));

	constexpr auto WOLF_WAKE_CHANCE	 = 1.0f / 1024;
	constexpr auto WOLF_SLEEP_CHANCE = 1.0f / 1024;
	constexpr auto WOLF_HOWL_CHANCE  = 1.0f / 85;

	constexpr auto WOLF_SLEEP_FRAME = 96;

	constexpr auto WOLF_WALK_TURN_RATE_MAX	= ANGLE(2.0f);
	constexpr auto WOLF_RUN_TURN_RATE_MAX	= ANGLE(5.0f);
	constexpr auto WOLF_STALK_TURN_RATE_MAX = ANGLE(2.0f);

	const auto WolfBite = CreatureBiteInfo(Vector3(0, -14, 174), 6);
	const auto WolfAttackJoints = std::vector<unsigned int>{ 0, 1, 2, 3, 6, 8, 9, 10, 12, 13, 14 };

	enum WolfState
	{
		// No state 0.
		WOLF_STATE_IDLE = 1,
		WOLF_STATE_WALK = 2,
		WOLF_STATE_RUN = 3,
		WOLF_STATE_JUMP = 4,
		WOLF_STATE_STALK = 5,
		WOLF_STATE_ATTACK = 6,
		WOLF_STATE_HOWL = 7,
		WOLF_STATE_SLEEP = 8,
		WOLF_STATE_CROUCH = 9,
		WOLF_STATE_FAST_TURN = 10,
		WOLF_STATE_DEATH = 11,
		WOLF_STATE_BITE = 12
	};

	// TODO
	enum WolfAnim
	{
		WOLF_ANIM_DEATH = 20,
	};

	void InitializeWolf(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		InitializeCreature(itemNumber);
		item->Animation.FrameNumber = WOLF_SLEEP_FRAME;
	}

	void WolfControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short tilt = 0;
		short head = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != WOLF_STATE_DEATH)
			{
				item->Animation.AnimNumber = Objects[ID_WOLF].animIndex + WOLF_ANIM_DEATH + (short)(GetRandomControl() / 11000);
				item->Animation.FrameNumber = GetAnimData(item).frameBase;
				item->Animation.ActiveState = WOLF_STATE_DEATH;
			}
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;

			GetCreatureMood(item, &AI, false);
			CreatureMood(item, &AI, false);

			if (item->Animation.ActiveState != WOLF_STATE_SLEEP)
				angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case WOLF_STATE_SLEEP:
				head = 0;

				if (creature->Mood == MoodType::Escape || AI.zoneNumber == AI.enemyZone)
				{
					item->Animation.RequiredState = WOLF_STATE_CROUCH;
					item->Animation.TargetState = WOLF_STATE_IDLE;
				}
				else if (Random::TestProbability(WOLF_WAKE_CHANCE))
				{
					item->Animation.RequiredState = WOLF_STATE_WALK;
					item->Animation.TargetState = WOLF_STATE_IDLE;
				}

				break;

			case WOLF_STATE_IDLE:
				if (item->Animation.RequiredState != NO_STATE)
					item->Animation.TargetState = item->Animation.RequiredState;
				else
					item->Animation.TargetState = WOLF_STATE_WALK;
				break;

			case WOLF_STATE_WALK:
				creature->MaxTurn = WOLF_WALK_TURN_RATE_MAX;

				if (creature->Mood != MoodType::Bored)
				{
					item->Animation.TargetState = WOLF_STATE_STALK;
					item->Animation.RequiredState = NO_STATE;
				}
				else if (Random::TestProbability(WOLF_SLEEP_CHANCE))
				{
					item->Animation.RequiredState = WOLF_STATE_SLEEP;
					item->Animation.TargetState = WOLF_STATE_IDLE;
				}

				break;

			case WOLF_STATE_CROUCH:
				if (item->Animation.RequiredState != NO_STATE)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = WOLF_STATE_RUN;
				else if (AI.distance < pow(345, 2) && AI.bite)
					item->Animation.TargetState = WOLF_STATE_BITE;
				else if (creature->Mood == MoodType::Stalk)
					item->Animation.TargetState = WOLF_STATE_STALK;
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = WOLF_STATE_IDLE;
				else
					item->Animation.TargetState = WOLF_STATE_RUN;

				break;

			case WOLF_STATE_STALK:
				creature->MaxTurn = WOLF_STALK_TURN_RATE_MAX;

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = WOLF_STATE_RUN;
				else if (AI.distance < pow(345, 2) && AI.bite)
					item->Animation.TargetState = WOLF_STATE_BITE;
				else if (AI.distance > pow(BLOCK(3), 2))
					item->Animation.TargetState = WOLF_STATE_RUN;
				else if (creature->Mood == MoodType::Attack)
				{
					if (!AI.ahead || AI.distance > pow(BLOCK(1.5f), 2) ||
						(AI.enemyFacing < FRONT_ARC && AI.enemyFacing > -FRONT_ARC))
					{
						item->Animation.TargetState = WOLF_STATE_RUN;
					}
				}
				else if (Random::TestProbability(WOLF_HOWL_CHANCE))
				{
					item->Animation.RequiredState = WOLF_STATE_HOWL;
					item->Animation.TargetState = WOLF_STATE_CROUCH;
				}
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = WOLF_STATE_CROUCH;

				break;

			case WOLF_STATE_RUN:
				creature->MaxTurn = WOLF_RUN_TURN_RATE_MAX;
				tilt = angle;

				if (AI.ahead && AI.distance < WOLF_ATTACK_RANGE)
				{
					if (AI.distance > (WOLF_ATTACK_RANGE / 2) &&
						(AI.enemyFacing > FRONT_ARC || AI.enemyFacing < -FRONT_ARC))
					{
						item->Animation.RequiredState = WOLF_STATE_STALK;
						item->Animation.TargetState = WOLF_STATE_CROUCH;
					}
					else
					{
						item->Animation.TargetState = WOLF_STATE_ATTACK;
						item->Animation.RequiredState = NO_STATE;
					}
				}
				else if (creature->Mood == MoodType::Stalk &&
					AI.distance < WOLF_STALK_RANGE)
				{
					item->Animation.RequiredState = WOLF_STATE_STALK;
					item->Animation.TargetState = WOLF_STATE_CROUCH;
				}
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = WOLF_STATE_CROUCH;

				break;

			case WOLF_STATE_ATTACK:
				tilt = angle;

				if (item->Animation.RequiredState == NO_STATE &&
					item->TouchBits.Test(WolfAttackJoints))
				{
					item->Animation.RequiredState = WOLF_STATE_RUN;
					DoDamage(creature->Enemy, WOLF_LUNGE_DAMAGE);
					CreatureEffect(item, WolfBite, DoBloodSplat);
				}

				item->Animation.TargetState = WOLF_STATE_RUN;
				break;

			case WOLF_STATE_BITE:
				if (AI.ahead && !item->Animation.RequiredState &&
					item->TouchBits.Test(WolfAttackJoints))
				{
					item->Animation.RequiredState = WOLF_STATE_CROUCH;
					DoDamage(creature->Enemy, WOLF_BITE_DAMAGE);
					CreatureEffect(item, WolfBite, DoBloodSplat);
				}

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
