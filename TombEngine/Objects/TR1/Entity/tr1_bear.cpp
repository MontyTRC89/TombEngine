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

namespace TEN::Entities::TR1
{
	BITE_INFO BearBite = { 0, 96, 335, 14 };
	const std::vector<int> BearAttackJoints = { 2, 3, 5, 6, 14, 17 };

	constexpr auto BEAR_RUN_DAMAGE = 3;
	constexpr auto BEAR_SLAM_DAMAGE = 200;
	constexpr auto BEAR_ATTACK_DAMAGE = 200;
	constexpr auto BEAR_PAT_DAMAGE = 400;

	#define ROAR_CHANCE 0x50
	#define REAR_CHANCE 0x300
	#define DROP_CHANCE 0x600
	#define REAR_RANGE pow(SECTOR(2), 2)
	#define ATTACK_RANGE pow(SECTOR(1), 2)
	#define PAT_RANGE pow(600, 2)
	#define RUN_TURN ANGLE(5.0f)
	#define WALK_TURN ANGLE(2.0f)
	#define EAT_RANGE pow(CLICK(3), 2)

	enum BearState
	{
		BEAR_STATE_STROLL = 0,
		BEAR_STATE_IDLE = 1,
		BEAR_STATE_WALK = 2,
		BEAR_STATE_RUN = 3,
		BEAR_STATE_REAR = 4,
		BEAR_STATE_ROAR = 5,
		BEAR_STATE_ATTACK_1 = 6,
		BEAR_STATE_ATTACK_2 = 7,
		BEAR_STATE_CHOMP = 8,
		BEAR_STATE_DEATH = 9
	};

	// TODO
	enum BearAnim
	{

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
			case BEAR_STATE_WALK:
			{
				item->Animation.TargetState = BEAR_STATE_REAR;
				break;
			}
			case BEAR_STATE_RUN:
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

					LaraItem->HitPoints -= BEAR_SLAM_DAMAGE;
					LaraItem->HitStatus = true;
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
					if (AI.bite && AI.distance < EAT_RANGE)
						item->Animation.TargetState = BEAR_STATE_CHOMP;
					else
						item->Animation.TargetState = BEAR_STATE_STROLL;
				}
				else if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = BEAR_STATE_STROLL;
				else
					item->Animation.TargetState = BEAR_STATE_RUN;

				break;

			case BEAR_STATE_STROLL:
				creature->MaxTurn = WALK_TURN;

				if (laraDead && item->TestBits(JointBitType::Touch, BearAttackJoints) && AI.ahead)
					item->Animation.TargetState = BEAR_STATE_IDLE;
				else if (creature->Mood != MoodType::Bored)
				{
					item->Animation.TargetState = BEAR_STATE_IDLE;

					if (creature->Mood == MoodType::Escape)
						item->Animation.RequiredState = BEAR_STATE_STROLL;
				}
				else if (GetRandomControl() < ROAR_CHANCE)
				{
					item->Animation.RequiredState = BEAR_STATE_ROAR;
					item->Animation.TargetState = BEAR_STATE_IDLE;
				}

				break;

			case BEAR_STATE_RUN:
				creature->MaxTurn = RUN_TURN;

				if (item->TestBits(JointBitType::Touch, BearAttackJoints))
				{
					LaraItem->HitPoints -= BEAR_RUN_DAMAGE;
					LaraItem->HitStatus = true;
				}

				if (creature->Mood == MoodType::Bored || laraDead)
					item->Animation.TargetState = BEAR_STATE_IDLE;
				else if (AI.ahead && !item->Animation.RequiredState)
				{
					if (!creature->Flags && AI.distance < REAR_RANGE && GetRandomControl() < REAR_CHANCE)
					{
						item->Animation.RequiredState = BEAR_STATE_REAR;
						item->Animation.TargetState = BEAR_STATE_IDLE;
					}
					else if (AI.distance < ATTACK_RANGE)
						item->Animation.TargetState = BEAR_STATE_ATTACK_1;
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
				else if (AI.bite && AI.distance < PAT_RANGE)
					item->Animation.TargetState = BEAR_STATE_ATTACK_2;
				else
					item->Animation.TargetState = BEAR_STATE_WALK;

				break;

			case BEAR_STATE_WALK:
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
				else if (creature->Mood == MoodType::Bored || GetRandomControl() < ROAR_CHANCE)
				{
					item->Animation.RequiredState = BEAR_STATE_ROAR;
					item->Animation.TargetState = BEAR_STATE_REAR;
				}
				else if (AI.distance > REAR_RANGE || GetRandomControl() < DROP_CHANCE)
				{
					item->Animation.RequiredState = BEAR_STATE_IDLE;
					item->Animation.TargetState = BEAR_STATE_REAR;
				}

				break;

			case BEAR_STATE_ATTACK_2:
				if (!item->Animation.RequiredState &&
					item->TestBits(JointBitType::Touch, BearAttackJoints))
				{
					item->Animation.RequiredState = BEAR_STATE_REAR;

					LaraItem->HitPoints -= BEAR_PAT_DAMAGE;
					LaraItem->HitStatus = true;
				}

				break;

			case BEAR_STATE_ATTACK_1:
				if (!item->Animation.RequiredState &&
					item->TestBits(JointBitType::Touch, BearAttackJoints))
				{
					CreatureEffect(item, &BearBite, DoBloodSplat);
					item->Animation.RequiredState = BEAR_STATE_IDLE;

					LaraItem->HitPoints -= BEAR_ATTACK_DAMAGE;
					LaraItem->HitStatus = true;
				}

				break;
			}
		}

		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
