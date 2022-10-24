#include "framework.h"
#include "Objects/TR1/Entity/tr1_bear.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;
using std::vector;

namespace TEN::Entities::Creatures::TR1
{
	constexpr auto BEAR_RUN_DAMAGE	  = 3;
	constexpr auto BEAR_ATTACK_DAMAGE = 200;
	constexpr auto BEAR_SLAM_DAMAGE	  = 200;
	constexpr auto BEAR_PAT_DAMAGE	  = 400;

	constexpr auto BEAR_ATTACK_RANGE			= SECTOR(1);
	constexpr auto BEAR_REAR_RANGE				= SECTOR(2);
	constexpr auto BEAR_REAR_SWIPE_ATTACK_RANGE = SECTOR(0.6f);
	constexpr auto BEAR_EAT_RANGE				= CLICK(3);
	
	constexpr auto BEAR_ROAR_CHANCE = 1.0f / 400;
	constexpr auto BEAR_REAR_CHANCE = 1.0f / 40;
	constexpr auto BEAR_DROP_CHANCE = 1.0f / 22;

	#define BEAR_WALK_TURN_RATE_MAX ANGLE(2.0f)
	#define BEAR_RUN_TURN_RATE_MAX	ANGLE(5.0f)

	const auto BearBite = BiteInfo(Vector3(0.0f, 96.0f, 335.0f), 14);
	const vector<uint> BearAttackJoints = { 2, 3, 5, 6, 14, 17 };

	enum BearState
	{
		BEAR_STATE_STROLL = 0,
		BEAR_STATE_IDLE = 1,
		BEAR_STATE_WALK_FORWARD = 2,
		BEAR_STATE_RUN_FORWARD = 3,
		BEAR_STATE_REAR = 4,
		BEAR_STATE_ROAR = 5,
		BEAR_STATE_RUN_SWIPE_ATTACK = 6,
		BEAR_STATE_REAR_SWIPE_ATTACK = 7,
		BEAR_STATE_EAT = 8,
		BEAR_STATE_DEATH = 9
	};

	// TODO
	enum BearAnim
	{
		BEAR_ANIM_INACTIVE = 0,
		BEAR_ANIM_IDLE_TO_RUN_FORWARD = 1,
		BEAR_ANIM_RUN_FORWARD = 2,
		BEAR_ANIM_RUN_FORWARD_TO_IDLE = 3,
		BEAR_ANIM_REAR_TO_IDLE = 4,

		BEAR_ANIM_REAR_IDLE = 7,
		BEAR_ANIM_IDLE_TO_REAR = 8,

		BEAR_ANIM_ROAR = 11,
		BEAR_ANIM_RUN_SWIPE_ATTACK_START = 12,
		BEAR_ANIM_RUN_SWIPE_ATTACK_END = 13,

		BEAR_ANIM_EAT = 15,
		BEAR_ANIM_IDLE_TO_WALK_FORWARD = 16,
		BEAR_ANIM_TO_WALK_FORWARD = 17,
		BEAR_ANIM_WALK_FORWARD_TO_IDLE = 18,

		BEAR_ANIM_DEATH = 20
	};

	void BearControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short head = 0;

		if (item->HitPoints <= 0)
		{
			angle = CreatureTurn(item, ANGLE(1.0f));

			switch (item->Animation.ActiveState)
			{
			case BEAR_STATE_WALK_FORWARD:
			{
				item->Animation.TargetState = BEAR_STATE_REAR;
				break;
			}
			case BEAR_STATE_RUN_FORWARD:
			case BEAR_STATE_STROLL:
			{
				item->Animation.TargetState = BEAR_STATE_IDLE;
				break;
			}
			case BEAR_STATE_REAR:
			{
				item->Animation.TargetState = BEAR_STATE_DEATH;
				creature->Flags = 1;
				break;
			}
			case BEAR_STATE_IDLE:
			{
				item->Animation.TargetState = BEAR_STATE_DEATH;
				creature->Flags = 0;
				break;
			}
			case BEAR_STATE_DEATH:
			{
				if (creature->Flags && item->TouchBits.Test(BearAttackJoints))
				{
					DoDamage(creature->Enemy, BEAR_SLAM_DAMAGE);
					creature->Flags = 0;
				}

				break;
			}
			}
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);

			if (item->HitStatus)
				creature->Flags = 1;

			bool isLaraDead = LaraItem->HitPoints <= 0;

			switch (item->Animation.ActiveState)
			{
			case BEAR_STATE_IDLE:
				if (isLaraDead)
				{
					if (AI.bite && AI.distance < pow(BEAR_EAT_RANGE, 2))
						item->Animation.TargetState = BEAR_STATE_EAT;
					else
						item->Animation.TargetState = BEAR_STATE_STROLL;
				}
				else if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = BEAR_STATE_STROLL;
				else
					item->Animation.TargetState = BEAR_STATE_RUN_FORWARD;

				break;

			case BEAR_STATE_STROLL:
				creature->MaxTurn = BEAR_WALK_TURN_RATE_MAX;

				if (isLaraDead && item->TouchBits.Test(BearAttackJoints) && AI.ahead)
					item->Animation.TargetState = BEAR_STATE_IDLE;
				else if (creature->Mood != MoodType::Bored)
				{
					item->Animation.TargetState = BEAR_STATE_IDLE;

					if (creature->Mood == MoodType::Escape)
						item->Animation.RequiredState = BEAR_STATE_STROLL;
				}
				else if (TestProbability(BEAR_ROAR_CHANCE))
				{
					item->Animation.TargetState = BEAR_STATE_IDLE;
					item->Animation.RequiredState = BEAR_STATE_ROAR;
				}

				break;

			case BEAR_STATE_RUN_FORWARD:
				creature->MaxTurn = BEAR_RUN_TURN_RATE_MAX;

				if (item->TouchBits.Test(BearAttackJoints))
					DoDamage(creature->Enemy, BEAR_RUN_DAMAGE);

				if (creature->Mood == MoodType::Bored || isLaraDead)
					item->Animation.TargetState = BEAR_STATE_IDLE;
				else if (AI.ahead && !item->Animation.RequiredState)
				{
					if (AI.distance < pow(BEAR_REAR_RANGE, 2) &&
						TestProbability(BEAR_REAR_CHANCE) &&
						!creature->Flags)
					{
						item->Animation.RequiredState = BEAR_STATE_REAR;
						item->Animation.TargetState = BEAR_STATE_IDLE;
					}
					else if (AI.distance < pow(BEAR_ATTACK_RANGE, 2))
						item->Animation.TargetState = BEAR_STATE_RUN_SWIPE_ATTACK;
				}

				break;

			case BEAR_STATE_REAR:
				if (creature->Flags)
				{
					item->Animation.RequiredState = BEAR_STATE_STROLL;
					item->Animation.TargetState = BEAR_STATE_IDLE;
				}
				else if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (creature->Mood == MoodType::Bored || creature->Mood == MoodType::Escape)
					item->Animation.TargetState = BEAR_STATE_IDLE;
				else if (AI.bite && AI.distance < pow(BEAR_REAR_SWIPE_ATTACK_RANGE, 2))
					item->Animation.TargetState = BEAR_STATE_REAR_SWIPE_ATTACK;
				else
					item->Animation.TargetState = BEAR_STATE_WALK_FORWARD;

				break;

			case BEAR_STATE_WALK_FORWARD:
				if (creature->Flags)
				{
					item->Animation.TargetState = BEAR_STATE_REAR;
					item->Animation.RequiredState = BEAR_STATE_STROLL;
				}
				else if (AI.ahead && item->TouchBits.Test(BearAttackJoints))
					item->Animation.TargetState = BEAR_STATE_REAR;
				else if (creature->Mood == MoodType::Escape)
				{
					item->Animation.TargetState = BEAR_STATE_REAR;
					item->Animation.RequiredState = BEAR_STATE_STROLL;
				}
				else if (creature->Mood == MoodType::Bored || TestProbability(BEAR_ROAR_CHANCE))
				{
					item->Animation.TargetState = BEAR_STATE_REAR;
					item->Animation.RequiredState = BEAR_STATE_ROAR;
				}
				else if (AI.distance > pow(BEAR_REAR_RANGE, 2) || TestProbability(BEAR_DROP_CHANCE))
				{
					item->Animation.TargetState = BEAR_STATE_REAR;
					item->Animation.RequiredState = BEAR_STATE_IDLE;
				}

				break;

			case BEAR_STATE_REAR_SWIPE_ATTACK:
				if (!item->Animation.RequiredState &&
					item->TouchBits.Test(BearAttackJoints))
				{
					DoDamage(creature->Enemy, BEAR_PAT_DAMAGE);
					item->Animation.RequiredState = BEAR_STATE_REAR;
				}

				break;

			case BEAR_STATE_RUN_SWIPE_ATTACK:
				if (!item->Animation.RequiredState &&
					item->TouchBits.Test(BearAttackJoints))
				{
					DoDamage(creature->Enemy, BEAR_ATTACK_DAMAGE);
					CreatureEffect(item, BearBite, DoBloodSplat);
					item->Animation.RequiredState = BEAR_STATE_IDLE;
				}

				break;
			}
		}

		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
