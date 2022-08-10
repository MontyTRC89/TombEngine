#include "framework.h"
#include "Objects/TR2/Entity/tr2_rat.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

namespace TEN::Entities::TR2
{
	constexpr auto RAT_DAMAGE = 20;
	constexpr auto RAT_ATTACK_RANGE = SQUARE(CLICK(0.7f));
	constexpr auto RAT_WAIT1_CHANCE = 1280;
	constexpr auto RAT_WAIT2_CHANCE = RAT_WAIT1_CHANCE * 2;

	const auto RatBite = BiteInfo(Vector3(0.0f, 0.0f, 57.0f), 2);

	#define RAT_TURN_RATE_MAX ANGLE(6.0f)

	enum RatState
	{
		RAT_STATE_NONE = 0,
		RAT_STATE_WALK,
		RAT_STATE_IDLE,
		RAT_STATE_SQUEAK,
		RAT_STATE_ATTACK1,
		RAT_STATE_ATTACK2,
		RAT_STATE_DEATH
	};

	enum RatAnim
	{
		RAT_ANIM_IDLE = 0,
		RAT_ANIM_IDLE_TO_WALK,
		RAT_ANIM_WALK,
		RAT_ANIM_WALK_TO_IDLE,
		RAT_ANIM_SQUEAK,
		RAT_ANIM_IDLE_TO_ATTACK1,
		RAT_ANIM_ATTACK1,
		RAT_ANIM_ATTACK1_TO_IDLE,
		RAT_ANIM_ATTACK2,
		RAT_ANIM_DEATH
	};

	void RatControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		int random = 0;
		short head = 0;
		short angle = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != RAT_STATE_DEATH)
				SetAnimation(item, RAT_ANIM_DEATH);
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;

			GetCreatureMood(item, &AI, false);
			CreatureMood(item, &AI, false);

			angle = CreatureTurn(item, RAT_TURN_RATE_MAX);

			switch (item->Animation.ActiveState)
			{
			case RAT_STATE_ATTACK1:
				if (creature->Mood == MoodType::Bored || creature->Mood == MoodType::Stalk)
				{
					random = GetRandomControl();
					if (random < RAT_WAIT1_CHANCE)
						item->Animation.RequiredState = RAT_STATE_SQUEAK;
					else if (random > RAT_WAIT2_CHANCE)
						item->Animation.RequiredState = RAT_STATE_WALK;
				}
				else if (AI.distance < RAT_ATTACK_RANGE)
					item->Animation.RequiredState = RAT_STATE_ATTACK2;
				else
					item->Animation.RequiredState = RAT_STATE_WALK;

				if (item->Animation.RequiredState)
					item->Animation.TargetState = RAT_STATE_IDLE;

				break;

			case RAT_STATE_IDLE:
				creature->MaxTurn = 0;

				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;

				break;

			case RAT_STATE_WALK:
				creature->MaxTurn = RAT_TURN_RATE_MAX;

				if (creature->Mood == MoodType::Bored || creature->Mood == MoodType::Stalk)
				{
					random = GetRandomControl();
					if (random < RAT_WAIT1_CHANCE)
					{
						item->Animation.RequiredState = RAT_STATE_SQUEAK;
						item->Animation.TargetState = RAT_STATE_IDLE;
					}
					else if (random < RAT_WAIT2_CHANCE)
						item->Animation.TargetState = RAT_STATE_IDLE;
				}
				else if (AI.ahead && AI.distance < RAT_ATTACK_RANGE)
					item->Animation.TargetState = RAT_STATE_IDLE;

				break;

			case RAT_STATE_ATTACK2:
				if (!item->Animation.RequiredState && item->TestBits(JointBitType::Touch, RatBite.meshNum))
				{
					DoDamage(creature->Enemy, RAT_DAMAGE);
					CreatureEffect(item, RatBite, DoBloodSplat);
					item->Animation.RequiredState = RAT_STATE_IDLE;
				}

				break;

			case RAT_STATE_SQUEAK:
				creature->MaxTurn = 0;
				if (GetRandomControl() < RAT_WAIT1_CHANCE)
					item->Animation.TargetState = RAT_STATE_IDLE;

				break;
			}
		}

		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
