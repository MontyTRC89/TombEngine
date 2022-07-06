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

using std::vector;

namespace TEN::Entities::TR1
{
	BITE_INFO BearBite = { 0, 96, 335, 14 };
	const vector<int> BearAttackJoints = { 2, 3, 5, 6, 14, 17 };

	constexpr auto BEAR_RUN_DAMAGE = 3;
	constexpr auto BEAR_ATTACK_DAMAGE = 200;
	constexpr auto BEAR_SLAM_DAMAGE = 200;
	constexpr auto BEAR_PAT_DAMAGE = 400;

	constexpr auto BEAR_ATTACK_RANGE = SECTOR(1);
	constexpr auto BEAR_REAR_RANGE = SECTOR(2);
	constexpr auto BEAR_REAR_SWIPE_ATTACK_RANGE = SECTOR(0.6f);
	constexpr auto BEAR_EAT_RANGE = CLICK(3);

	constexpr auto BEAR_ROAR_CHANCE = 0x50;
	constexpr auto BEAR_REAR_CHANCE = 0x300;
	constexpr auto BEAR_DROP_CHANCE = 0x600;

	#define BEAR_WALK_TURN_RATE_MAX ANGLE(2.0f)
	#define BEAR_RUN_TURN_RATE_MAX ANGLE(5.0f)

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

		short head = 0;
		short angle = 0;

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
				if (creature->Flags && item->TestBits(JointBitType::Touch, BearAttackJoints))
				{
					creature->Flags = 0;
					DoDamage(creature->Enemy, BEAR_SLAM_DAMAGE);
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

			GetCreatureMood(item, &AI, VIOLENT);
			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, creature->MaxTurn);

			if (item->HitStatus)
				creature->Flags = 1;

			const bool laraDead = LaraItem->HitPoints <= 0;

			switch (item->Animation.ActiveState)
			{
			case BEAR_STATE_IDLE:
				if (laraDead)
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

				if (laraDead && item->TestBits(JointBitType::Touch, BearAttackJoints) && AI.ahead)
					item->Animation.TargetState = BEAR_STATE_IDLE;
				else if (creature->Mood != MoodType::Bored)
				{
					item->Animation.TargetState = BEAR_STATE_IDLE;

					if (creature->Mood == MoodType::Escape)
						item->Animation.RequiredState = BEAR_STATE_STROLL;
				}
				else if (GetRandomControl() < BEAR_ROAR_CHANCE)
				{
					item->Animation.RequiredState = BEAR_STATE_ROAR;
					item->Animation.TargetState = BEAR_STATE_IDLE;
				}

				break;

			case BEAR_STATE_RUN_FORWARD:
				creature->MaxTurn = BEAR_RUN_TURN_RATE_MAX;

				if (item->TestBits(JointBitType::Touch, BearAttackJoints))
				{
					DoDamage(creature->Enemy, BEAR_RUN_DAMAGE);
				}

				if (creature->Mood == MoodType::Bored || laraDead)
					item->Animation.TargetState = BEAR_STATE_IDLE;
				else if (AI.ahead && !item->Animation.RequiredState)
				{
					if (!creature->Flags && AI.distance < pow(BEAR_REAR_RANGE, 2) && GetRandomControl() < BEAR_REAR_CHANCE)
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
					item->Animation.RequiredState = BEAR_STATE_STROLL;
					item->Animation.TargetState = BEAR_STATE_REAR;
				}
				else if (AI.ahead && item->TestBits(JointBitType::Touch, BearAttackJoints))
					item->Animation.TargetState = BEAR_STATE_REAR;
				else if (creature->Mood == MoodType::Escape)
				{
					item->Animation.TargetState = BEAR_STATE_REAR;
					item->Animation.RequiredState = BEAR_STATE_STROLL;
				}
				else if (creature->Mood == MoodType::Bored || GetRandomControl() < BEAR_ROAR_CHANCE)
				{
					item->Animation.RequiredState = BEAR_STATE_ROAR;
					item->Animation.TargetState = BEAR_STATE_REAR;
				}
				else if (AI.distance > pow(BEAR_REAR_RANGE, 2) || GetRandomControl() < BEAR_DROP_CHANCE)
				{
					item->Animation.RequiredState = BEAR_STATE_IDLE;
					item->Animation.TargetState = BEAR_STATE_REAR;
				}

				break;

			case BEAR_STATE_REAR_SWIPE_ATTACK:
				if (!item->Animation.RequiredState &&
					item->TestBits(JointBitType::Touch, BearAttackJoints))
				{
					DoDamage(creature->Enemy, BEAR_PAT_DAMAGE);
					item->Animation.RequiredState = BEAR_STATE_REAR;
				}

				break;

			case BEAR_STATE_RUN_SWIPE_ATTACK:
				if (!item->Animation.RequiredState &&
					item->TestBits(JointBitType::Touch, BearAttackJoints))
				{
					CreatureEffect(item, &BearBite, DoBloodSplat);
					DoDamage(creature->Enemy, BEAR_ATTACK_DAMAGE);
					item->Animation.RequiredState = BEAR_STATE_IDLE;
				}

				break;
			}
		}

		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
