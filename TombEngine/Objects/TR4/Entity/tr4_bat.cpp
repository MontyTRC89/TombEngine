#include "framework.h"
#include "Objects/TR4/Entity/tr4_bat.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/misc.h"
#include "Game/Lara/lara.h"
#include "Game/control/lot.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Specific/prng.h"
#include "Specific/setup.h"
#include "Specific/trmath.h"

using namespace TEN::Math::Random;

namespace TEN::Entities::TR4
{
	constexpr auto BAT_ATTACK_DAMAGE = 50;

	constexpr auto BAT_UNFURL_HEIGHT_RANGE = SECTOR(0.87f);
	constexpr auto BAT_ATTACK_RANGE		   = SQUARE(CLICK(1));
	constexpr auto BAT_AWARE_RANGE		   = SQUARE(SECTOR(5));

	#define BAT_ANGLE ANGLE(20.0f)

	const auto BatBite = BiteInfo(Vector3(0.0f, 16.0f, 45.0f), 4);

	enum BatState
	{
		BAT_STATE_NONE = 0,
		BAT_STATE_DROP_FROM_CEILING = 1,
		BAT_STATE_FLY = 2,
		BAT_STATE_ATTACK = 3,
		BAT_STATE_DEATH_FALL = 4,
		BAT_STATE_DEATH = 5,
		BAT_STATE_IDLE = 6
	};

	enum BatAnim
	{
		BAT_ANIM_DROP_FROM_CEILING = 0,
		BAT_ANIM_FLY = 1,
		BAT_ANIM_ATTACK = 2,
		BAT_ANIM_DEATH_FALL = 3,
		BAT_ANIM_DEATH = 4,
		BAT_ANIM_IDLE = 5,
	};

	bool IsBatCollideTarget(ItemInfo* item)
	{
		return (item->TouchBits >= 0);
	}

	void InitialiseBat(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);
		SetAnimation(item, BAT_ANIM_IDLE);
	}

	void BatControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;

		if (item->HitPoints > 0)
		{
			if (item->AIBits)
				GetAITarget(creature);
			else
				creature->Enemy = LaraItem;

			// NOTE: Changed from false to true, otherwise the bat seems to ignore Lara. 
			// I feel fine with bat always true,
			// but I will also inspect GetCreatureMood and CreatureMood functions for bugs. @TokyoSU

			AI_INFO AI;
			CreatureAIInfo(item, &AI);
			GetCreatureMood(item, &AI, true);

			if (creature->Flags)
				creature->Mood = MoodType::Escape;

			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, BAT_ANGLE);

			switch (item->Animation.ActiveState)
			{
			case BAT_STATE_IDLE:
				if (AI.distance < BAT_AWARE_RANGE || item->HitStatus || creature->HurtByLara)
					item->Animation.TargetState = BAT_STATE_DROP_FROM_CEILING;

				break;

			case BAT_STATE_FLY:
				if (AI.distance < BAT_ATTACK_RANGE || TestProbability(0.015f))
					creature->Flags = NULL;

				if (!creature->Flags)
				{
					if (item->TouchBits ||
						(!creature->Enemy->IsLara() &&
						AI.distance < BAT_ATTACK_RANGE && AI.ahead &&
						abs(item->Pose.Position.y - creature->Enemy->Pose.Position.y) < BAT_UNFURL_HEIGHT_RANGE))
					{
						item->Animation.TargetState = BAT_STATE_ATTACK;
					}
				}

				break;

			case BAT_STATE_ATTACK:
				if (!creature->Flags &&
					(item->TouchBits || !creature->Enemy->IsLara()) &&
					AI.distance < BAT_ATTACK_RANGE && AI.ahead &&
					abs(item->Pose.Position.y - creature->Enemy->Pose.Position.y) < BAT_UNFURL_HEIGHT_RANGE)
				{
					DoDamage(creature->Enemy, BAT_ATTACK_DAMAGE);
					CreatureEffect(item, BatBite, DoBloodSplat);
					creature->Flags = 1;
				}
				else
				{
					item->Animation.TargetState = BAT_STATE_FLY;
					creature->Mood = MoodType::Bored;
				}

				break;
			}
		}
		else if (item->Animation.ActiveState == BAT_STATE_ATTACK)
			SetAnimation(item, BAT_ANIM_FLY);
		else
		{
			if (item->Pose.Position.y >= item->Floor)
			{
				item->Animation.TargetState = BAT_STATE_DEATH;
				item->Animation.IsAirborne = false;
				item->Pose.Position.y = item->Floor;
			}
			else
				SetAnimation(item, BAT_ANIM_DEATH_FALL);
				item->Animation.IsAirborne = true;
				item->Animation.Velocity = 0;
		}

		CreatureAnimation(itemNumber, angle, 0);
	}
}
