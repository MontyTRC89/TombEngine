#include "framework.h"
#include "Objects/TR1/Entity/tr1_bear.h"

#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Specific/level.h"

using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR1
{
	constexpr auto BEAR_RUN_COLLISION_DAMAGE		= 3;
	constexpr auto BEAR_ATTACK_CHARGE_DAMAGE		= 200;
	constexpr auto BEAR_ATTACK_CLAW_DAMAGE		= 400;
	constexpr auto BEAR_DEATH_CRUSH_DAMAGE		= 200;
	
	constexpr auto BEAR_LOW_ATTACK_RANGE			= SQUARE(BLOCK(1));
	constexpr auto BEAR_HIGH_STANCE_RANGE		= SQUARE(BLOCK(2));
	constexpr auto BEAR_TRANSITION_RANGE		= SQUARE(BLOCK(3 / 5.0f));
	constexpr auto BEAR_EAT_RANGE				= SQUARE(BLOCK(3 / 4.0f));
	
	constexpr auto BEAR_ROAR_CHANCE			= 1 / 400.0f;
	constexpr auto BEAR_TO_HIGH_STANCE_CHANCE	= 1 / 40.0f;
	constexpr auto BEAR_TO_LOW_STANCE_CHANCE	= 1 / 22.0f;

	constexpr auto BEAR_WALK_TURN_RATE_MAX		= ANGLE(2.0f);
	constexpr auto BEAR_RUN_TURN_RATE_MAX		= ANGLE(5.0f);

	const auto BearBite = CreatureBiteInfo(Vector3(0, 96, 335), 14);
	const auto BearAttackJoints = std::vector<unsigned int>{ 2, 3, 5, 6, 14, 17 };

	enum BearState
	{
		// Low Stance (4-Feet)
		BEAR_STATE_LOW_WALK = 0,
		BEAR_STATE_LOW_IDLE = 1,
		BEAR_STATE_LOW_RUN = 3,
		BEAR_STATE_LOW_ATTACK = 6,
		BEAR_STATE_LOW_EAT = 8,

		// High Stance (2-Feet)
		BEAR_STATE_HIGH_WALK = 2,
		BEAR_STATE_HIGH_IDLE = 4,
		BEAR_STATE_HIGH_ATTACK = 7,
		
		// For Both Stances
		BEAR_STATE_ROAR = 5,
		BEAR_STATE_DEATH = 9
	};

	enum BearAnim
	{
		// Low Stance (4-Feet)
		BEAR_ANIM_LOW_IDLE = 0,
		BEAR_ANIM_LOW_IDLE_TO_RUN = 1,
		BEAR_ANIM_LOW_RUN = 2,
		BEAR_ANIM_LOW_RUN_TO_IDLE = 3,
		BEAR_ANIM_LOW_ROAR = 11,
		BEAR_ANIM_LOW_RUNING_ATTACK_START = 12,
		BEAR_ANIM_LOW_RUNING_ATTACK_END = 13,
		BEAR_ANIM_LOW_EAT = 15,
		BEAR_ANIM_LOW_IDLE_TO_WALK = 16,
		BEAR_ANIM_LOW_WALK = 17,
		BEAR_ANIM_LOW_WALK_TO_IDLE = 18,
		BEAR_ANIM_LOW_DEATH = 20,

		// High Stance (2-Feet)
		BEAR_ANIM_HIGH_ROAR = 5,
		BEAR_ANIM_HIGH_WALK = 6,
		BEAR_ANIM_HIGH_IDLE = 7,
		BEAR_ANIM_HIGH_IDLE_TO_WALK = 9,
		BEAR_ANIM_HIGH_WALK_TO_IDLE = 10,
		BEAR_ANIM_HIGH_ATTACK = 14,
		BEAR_ANIM_HIGH_DEATH = 19,

		// Stance transitions
		BEAR_ANIM_HIGH_IDLE_TO_LOW_IDLE = 4,
		BEAR_ANIM_LOW_IDLE_TO_HIGH_IDLE = 8,
	};

	void BearControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* object = &Objects[item->ObjectNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short head = 0;

		if (item->HitPoints <= 0)
		{
			angle = CreatureTurn(item, ANGLE(1.0f));

			switch (item->Animation.ActiveState)
			{
				//Low stance
				case BEAR_STATE_LOW_RUN:
				case BEAR_STATE_LOW_WALK:
				{
					item->Animation.TargetState = BEAR_STATE_LOW_IDLE;
					break;
				}
				case BEAR_STATE_LOW_IDLE:
				{
					item->Animation.TargetState = BEAR_STATE_DEATH;
					creature->Flags = 0;
					break;
				}

				//High stance
				case BEAR_STATE_HIGH_WALK:
				{
					item->Animation.TargetState = BEAR_STATE_HIGH_IDLE;
					break;
				}
				case BEAR_STATE_HIGH_IDLE:
				{
					item->Animation.TargetState = BEAR_STATE_DEATH;
					creature->Flags = 1;
					break;
				}
				
				//Both stances
				case BEAR_STATE_DEATH:
				{
					if (creature->Flags && item->TouchBits.Test(BearAttackJoints))
					{
						DoDamage(creature->Enemy, BEAR_DEATH_CRUSH_DAMAGE);
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

			auto pointColl = GetCollision(item);
			int distanceToCeiling = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);

			switch (item->Animation.ActiveState)
			{
				//Low Stance
				case BEAR_STATE_LOW_IDLE:
					if (isLaraDead)
					{
						if (AI.bite && AI.distance < BEAR_EAT_RANGE)
							item->Animation.TargetState = BEAR_STATE_LOW_EAT;
						else
							item->Animation.TargetState = BEAR_STATE_LOW_WALK;
					}
					else if (item->Animation.RequiredState != NO_STATE)
						item->Animation.TargetState = item->Animation.RequiredState;
					else if (creature->Mood == MoodType::Bored)
						item->Animation.TargetState = BEAR_STATE_LOW_WALK;
					else
						item->Animation.TargetState = BEAR_STATE_LOW_RUN;

					break;

				case BEAR_STATE_LOW_WALK:
					creature->MaxTurn = BEAR_WALK_TURN_RATE_MAX;

					if (isLaraDead && item->TouchBits.Test(BearAttackJoints) && AI.ahead)
						item->Animation.TargetState = BEAR_STATE_LOW_IDLE; //Going to EAT state.
					else if (creature->Mood != MoodType::Bored)
					{
						item->Animation.TargetState = BEAR_STATE_LOW_IDLE; //Going to RUN state.
					}
					else if (Random::TestProbability(BEAR_ROAR_CHANCE))
					{
						item->Animation.TargetState = BEAR_STATE_LOW_IDLE;
						item->Animation.RequiredState = BEAR_STATE_ROAR;
					}

					break;

				case BEAR_STATE_LOW_RUN:
					creature->MaxTurn = BEAR_RUN_TURN_RATE_MAX;

					if (item->TouchBits.Test(BearAttackJoints))
						DoDamage(creature->Enemy, BEAR_RUN_COLLISION_DAMAGE);

					if (creature->Mood == MoodType::Bored ||
						isLaraDead ||
						item->Animation.RequiredState != NO_STATE)
					{
						//Going to WALK, EAT or the Required State.
						item->Animation.TargetState = BEAR_STATE_LOW_IDLE;
					}
					else if (AI.ahead)
					{
						if (Random::TestProbability(BEAR_TO_HIGH_STANCE_CHANCE))
						{
							if (AI.distance < BEAR_HIGH_STANCE_RANGE && !creature->Flags && distanceToCeiling > CLICK(7))
							{
								item->Animation.RequiredState = BEAR_STATE_HIGH_IDLE;
								item->Animation.TargetState = BEAR_STATE_LOW_IDLE;
							}
						}
						else if (AI.distance < BEAR_LOW_ATTACK_RANGE)
						{
							item->Animation.TargetState = BEAR_STATE_LOW_ATTACK;
						}
					}

					break;

				case BEAR_STATE_LOW_ATTACK:
					if (item->Animation.RequiredState == NO_STATE &&
						item->TouchBits.Test(BearAttackJoints))
					{
						DoDamage(creature->Enemy, BEAR_ATTACK_CHARGE_DAMAGE);
						CreatureEffect(item, BearBite, DoBloodSplat);
						item->Animation.RequiredState = BEAR_STATE_LOW_IDLE;
					}

					break;

				//High Stance
				case BEAR_STATE_HIGH_IDLE:
					if (creature->Flags)
					{
						item->Animation.RequiredState = BEAR_STATE_LOW_WALK;
						item->Animation.TargetState = BEAR_STATE_LOW_IDLE;
					}
					else if (item->Animation.RequiredState != NO_STATE)
					{
						item->Animation.TargetState = item->Animation.RequiredState;
					}
					else if (creature->Mood == MoodType::Bored ||
						creature->Mood == MoodType::Escape ||
						AI.distance > BEAR_HIGH_STANCE_RANGE ||
						distanceToCeiling <= CLICK(7))
					{
						item->Animation.TargetState = BEAR_STATE_LOW_IDLE;
					}
					else if (abs(LaraItem->Pose.Position.y - item->Pose.Position.y) <= CLICK(2) &&
							 item->TouchBits.Test(BearAttackJoints))
					{
						item->Animation.TargetState = BEAR_STATE_HIGH_ATTACK;
					}
					else
					{
						item->Animation.TargetState = BEAR_STATE_HIGH_WALK;
					}

					break;

				case BEAR_STATE_HIGH_WALK:
					if (creature->Flags)
					{
						item->Animation.TargetState = BEAR_STATE_HIGH_IDLE;
						item->Animation.RequiredState = BEAR_STATE_LOW_WALK;
					}
					else if (item->TouchBits.Test(BearAttackJoints))
					{
						item->Animation.TargetState = BEAR_STATE_HIGH_IDLE; //Going to ATTACK state
					}
					else if (creature->Mood == MoodType::Escape)
					{
						item->Animation.TargetState = BEAR_STATE_HIGH_IDLE;
						item->Animation.RequiredState = BEAR_STATE_LOW_RUN;
					}
					else if (	creature->Mood == MoodType::Bored || 
								Random::TestProbability(BEAR_ROAR_CHANCE))
					{
						//Will roar on random moment, or when it get bored if that happens during this state.
						item->Animation.TargetState = BEAR_STATE_HIGH_IDLE;
						item->Animation.RequiredState = BEAR_STATE_ROAR;
					}
					else if (	AI.distance > BEAR_HIGH_STANCE_RANGE ||
								distanceToCeiling <= CLICK(7) ||
								(AI.distance > BEAR_TRANSITION_RANGE && Random::TestProbability(BEAR_TO_LOW_STANCE_CHANCE)))
					{
						item->Animation.TargetState = BEAR_STATE_HIGH_IDLE;
						item->Animation.RequiredState = BEAR_STATE_LOW_IDLE;
					}

					break;

				case BEAR_STATE_HIGH_ATTACK:
					if (item->Animation.RequiredState == NO_STATE &&
						item->TouchBits.Test(BearAttackJoints))
					{
						DoDamage(creature->Enemy, BEAR_ATTACK_CLAW_DAMAGE);
						item->Animation.RequiredState = BEAR_STATE_HIGH_IDLE;
					}

					break;
			}
		}

		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNumber, angle, 0);

		if (item->Animation.ActiveState != BEAR_STATE_HIGH_WALK &&
			item->Animation.ActiveState != BEAR_STATE_HIGH_IDLE &&
			item->Animation.ActiveState != BEAR_STATE_HIGH_ATTACK)
		{
			auto radius = Vector2(object->radius);
			AlignEntityToSurface(item, radius);
		}
	}
}
