#include "framework.h"
#include "Objects/TR3/Entity/tr3_tiger.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Math/Random.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;
using std::vector;

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto TIGER_ATTACK_DAMAGE = 90;

	constexpr auto TIGER_BITE_ATTACK_RANGE	 = SQUARE(SECTOR(0.33f));
	constexpr auto TIGER_POUNCE_ATTACK_RANGE = SQUARE(SECTOR(1));
	constexpr auto TIGER_RUN_ATTACK_RANGE	 = SQUARE(SECTOR(1.5f));

	constexpr auto TIGER_WALK_CHANCE = 1.0f / 32;
	constexpr auto TIGER_ROAR_CHANCE = 1.0f / 340;

	const auto TigerBite = BiteInfo(Vector3(19.0f, -13.0f, 3.0f), 26);
	const vector<int> TigerAttackJoints = { 14, 15, 16, 18, 19, 20, 21, 22, 23, 24, 25, 26 };

	#define TIGER_WALK_TURN_RATE_MAX		  ANGLE(3.0f)
	#define TIGER_RUN_TURN_RATE_MAX			  ANGLE(6.0f)
	#define TIGER_POUNCE_ATTACK_TURN_RATE_MAX ANGLE(3.0f)

	enum TigerState
	{
		TIGER_STATE_DEATH = 0,
		TIGER_STATE_IDLE = 1,
		TIGER_STATE_WALK_FORWARD = 2,
		TIGER_STATE_RUN_FORWARD = 3,
		// No state 4.
		TIGER_STATE_ROAR = 5,
		TIGER_STATE_BITE_ATTACK = 6,
		TIGER_STATE_RUN_SWIPE_ATTACK = 7,
		TIGER_STATE_POUNCE_ATTACK = 8
	};

	enum TigerAnim
	{
		TIGER_ANIM_IDLE_TO_RUN_FORWARD = 0,
		TIGER_ANIM_BITE_ATTACK = 1,
		TIGER_ANIM_RUN_SWIPE_ATTACK = 2,
		TIGER_ANIM_POUNCE_ATTACK_START = 3,
		TIGER_ANIM_ROAR = 4,
		TIGER_ANIM_RUN_FORWARD = 5,
		TIGER_ANIM_RUN_FORWARD_TO_IDLE = 6,
		TIGER_ANIM_IDLE = 7,
		TIGER_ANIM_WALK_FORWARD = 8,
		TIGER_ANIM_IDLE_TO_WALK_FORWARD = 9,
		TIGER_ANIM_WALK_FORWARD_TO_IDLE = 10,
		TIGER_ANIM_DEATH = 11,
		TIGER_ANIM_POUNCE_ATTACK_END = 12
	};

	void TigerControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short headingAngle = 0;
		short tiltAngle = 0;
		auto extraHeadRot = EulerAngles::Zero;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != TIGER_STATE_DEATH)
				SetAnimation(item, TIGER_ANIM_DEATH);
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				extraHeadRot.y = AI.angle;

			GetCreatureMood(item, &AI, true);

			if (creature->Alerted && AI.zoneNumber != AI.enemyZone)
				creature->Mood = MoodType::Escape;

			CreatureMood(item, &AI, true);

			headingAngle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case TIGER_STATE_IDLE:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead)
						item->Animation.TargetState = TIGER_STATE_IDLE;
					else
						item->Animation.TargetState = TIGER_STATE_RUN_FORWARD;
				}
				else if (creature->Mood == MoodType::Bored)
				{
					if (TestProbability(TIGER_ROAR_CHANCE))
						item->Animation.TargetState = TIGER_STATE_ROAR;
					else if (TestProbability(TIGER_WALK_CHANCE))
						item->Animation.TargetState = TIGER_STATE_WALK_FORWARD;
				}
				else if (AI.bite && AI.distance < TIGER_BITE_ATTACK_RANGE)
					item->Animation.TargetState = TIGER_STATE_BITE_ATTACK;
				else if (AI.bite && AI.distance < TIGER_POUNCE_ATTACK_RANGE)
				{
					item->Animation.TargetState = TIGER_STATE_POUNCE_ATTACK;
					creature->MaxTurn = TIGER_POUNCE_ATTACK_TURN_RATE_MAX;
				}
				else if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (creature->Mood != MoodType::Attack && TestProbability(TIGER_ROAR_CHANCE))
					item->Animation.TargetState = TIGER_STATE_ROAR;
				else
					item->Animation.TargetState = TIGER_STATE_RUN_FORWARD;

				break;

			case TIGER_STATE_WALK_FORWARD:
				creature->MaxTurn = TIGER_WALK_TURN_RATE_MAX;

				if (creature->Mood == MoodType::Escape ||
					creature->Mood == MoodType::Attack)
				{
					item->Animation.TargetState = TIGER_STATE_RUN_FORWARD;
				}
				else if (TestProbability(TIGER_ROAR_CHANCE))
				{
					item->Animation.TargetState = TIGER_STATE_IDLE;
					item->Animation.RequiredState = TIGER_STATE_ROAR;
				}

				break;

			case TIGER_STATE_RUN_FORWARD:
				creature->MaxTurn = TIGER_RUN_TURN_RATE_MAX;

				if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = TIGER_STATE_IDLE;
				else if (creature->Flags && AI.ahead)
					item->Animation.TargetState = TIGER_STATE_IDLE;
				else if (AI.bite && AI.distance < TIGER_RUN_ATTACK_RANGE)
				{
					if (LaraItem->Animation.Velocity.z == 0.0f)
						item->Animation.TargetState = TIGER_STATE_IDLE;
					else
						item->Animation.TargetState = TIGER_STATE_RUN_SWIPE_ATTACK;
				}
				else if (creature->Mood != MoodType::Attack && TestProbability(TIGER_ROAR_CHANCE))
				{
					item->Animation.TargetState = TIGER_STATE_IDLE;
					item->Animation.RequiredState = TIGER_STATE_ROAR;
				}
				else if (creature->Mood == MoodType::Escape &&
					Lara.TargetEntity != item && AI.ahead)
				{
					item->Animation.TargetState = TIGER_STATE_IDLE;
				}

				creature->Flags = 0;
				break;

			case TIGER_STATE_BITE_ATTACK:
			case TIGER_STATE_RUN_SWIPE_ATTACK:
			case TIGER_STATE_POUNCE_ATTACK:
				if (!creature->Flags && item->TestBits(JointBitType::Touch, TigerAttackJoints))
				{
					DoDamage(creature->Enemy, TIGER_ATTACK_DAMAGE);
					CreatureEffect(item, TigerBite, DoBloodSplat);
					creature->Flags = 1; // 1 = is attacking.
				}

				break;
			}
		}

		CreatureTilt(item, tiltAngle);
		CreatureJoint(item, 0, extraHeadRot.y);
		CreatureAnimation(itemNumber, headingAngle, tiltAngle);
	}
}
