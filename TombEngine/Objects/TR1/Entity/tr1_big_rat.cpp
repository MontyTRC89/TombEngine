#include "framework.h"
#include "Objects/TR1/Entity/tr1_big_rat.h"

#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Specific/level.h"
#include "Specific/prng.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;
using std::vector;

namespace TEN::Entities::Creatures::TR1
{
	constexpr auto BIG_RAT_BITE_ATTACK_DAMAGE	= 20;
	constexpr auto BIG_RAT_POUNCE_ATTACK_DAMAGE = 25;

	constexpr auto BIG_RAT_ALERT_RANGE			   = SQUARE(SECTOR(1.5f));
	constexpr auto BIG_RAT_VISIBILITY_RANGE		   = SQUARE(SECTOR(5));
	constexpr auto BIG_RAT_LAND_BITE_ATTACK_RANGE  = SQUARE(SECTOR(0.34f));
	constexpr auto BIG_RAT_POUNCE_ATTACK_RANGE	   = SQUARE(SECTOR(0.5f));
	constexpr auto BIG_RAT_WATER_BITE_ATTACK_RANGE = SQUARE(SECTOR(0.3f));

	constexpr auto BIG_RAT_REAR_POSE_CHANCE = 1.0f / 128.0f;
	constexpr auto BIG_RAT_SWIM_UP_DOWN_SPEED = 32;
	constexpr auto BIG_RAT_WATER_SURFACE_OFFSET = 10;

	#define BIG_RAT_RUN_TURN_RATE_MAX  ANGLE(6.0f)
	#define BIG_RAT_SWIM_TURN_RATE_MAX ANGLE(3.0f)

	const auto BigRatBite = BiteInfo(Vector3(0.0f, -11.0f, 108.0f), 3);

	enum BigRatState
	{
		BIG_RAT_STATE_NONE = 0,
		BIG_RAT_STATE_IDLE = 1,
		BIG_RAT_STATE_POUNCE_ATTACK = 2,
		BIG_RAT_STATE_RUN_FORWARD = 3,
		BIG_RAT_STATE_LAND_BITE_ATTACK = 4,
		BIG_RAT_STATE_LAND_DEATH = 5,
		BIG_RAT_STATE_REAR_POSE = 6,
		BIG_RAT_STATE_SWIM = 7,
		BIG_RAT_STATE_SWIM_BITE_ATTACK = 8,
		BIG_RAT_STATE_WATER_DEATH = 9
	};

	enum BigRatAnim
	{
		BIG_RAT_ANIM_IDLE = 0,
		BIG_RAT_ANIM_IDLE_TO_RUN_FORWARD = 1,
		BIG_RAT_ANIM_RUN_FORWARD = 2,
		BIG_RAT_ANIM_RUN_FORWARD_TO_IDLE = 3,
		BIG_RAT_ANIM_REAR_POSE = 4,
		BIG_RAT_ANIM_REAR_POSE_TO_IDLE = 5,
		BIG_RAT_ANIM_LAND_BITE_ATTACK = 6,
		BIG_RAT_ANIM_POUNCE_ATTACK = 7,
		BIG_RAT_ANIM_LAND_DEATH = 8,
		BIG_RAT_ANIM_SWIM = 9,
		BIG_RAT_ANIM_WATER_BITE_ATTACK = 10,
		BIG_RAT_ANIM_WATER_DEATH = 11,

		// NOTE: These animations don't exist for the TR2 rat. -- TokyoSU 2022.08.10
		BIG_RAT_ANIM_RUN_FORWARD_TO_SWIM = 12,
		BIG_RAT_ANIM_SWIM_TO_RUN_FORWARD = 13
	};

	void InitialiseBigRat(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);

		if (TestEnvironment(ENV_FLAG_WATER, item))
			SetAnimation(item, BIG_RAT_ANIM_SWIM);
		else
			SetAnimation(item, BIG_RAT_ANIM_IDLE);
	}

	int GetRatWaterHeight(ItemInfo* item)
	{
		auto* creature = GetCreatureInfo(item);

		auto probe = GetCollision(item);
		int waterDepth = GetWaterSurface(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, probe.RoomNumber);

		if (waterDepth != NO_HEIGHT)
		{
			creature->LOT.Step = SECTOR(20);
			creature->LOT.Drop = -SECTOR(20);
			creature->LOT.Fly = BIG_RAT_SWIM_UP_DOWN_SPEED;
			return waterDepth;
		}
		else
		{
			creature->LOT.Step = CLICK(1);
			creature->LOT.Drop = -CLICK(1);
			creature->LOT.Fly = NO_FLYING;
		}

		return NO_HEIGHT;
	}

	void BigRatControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		int waterHeight = GetRatWaterHeight(item);
		bool isOnWater = waterHeight != NO_HEIGHT;

		short angle = 0;
		short head = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != BIG_RAT_STATE_LAND_DEATH &&
				item->Animation.ActiveState != BIG_RAT_STATE_WATER_DEATH)
			{
				if (isOnWater)
					SetAnimation(item, BIG_RAT_ANIM_WATER_DEATH);
				else
					SetAnimation(item, BIG_RAT_ANIM_LAND_DEATH);
			}

			if (isOnWater)
				CreatureFloat(itemNumber);
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;

			GetCreatureMood(item, &AI, false);
			CreatureMood(item, &AI, false);
			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case BIG_RAT_STATE_IDLE:
				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (AI.bite && AI.distance < BIG_RAT_LAND_BITE_ATTACK_RANGE)
					item->Animation.TargetState = BIG_RAT_STATE_LAND_BITE_ATTACK;
				else
					item->Animation.TargetState = BIG_RAT_STATE_RUN_FORWARD;

				break;

			case BIG_RAT_STATE_RUN_FORWARD:
				creature->MaxTurn = BIG_RAT_RUN_TURN_RATE_MAX;

				TENLog("WaterHeight: " + std::to_string(waterHeight));
				if (isOnWater)
				{
					SetAnimation(item, BIG_RAT_ANIM_SWIM);
					break;
				}

				if (AI.ahead && item->TestBits(JointBitType::Touch, BigRatBite.meshNum))
					item->Animation.TargetState = BIG_RAT_STATE_IDLE;
				else if (AI.bite && AI.distance < BIG_RAT_POUNCE_ATTACK_RANGE)
					item->Animation.TargetState = BIG_RAT_STATE_POUNCE_ATTACK;
				else if (AI.ahead && TestProbability(BIG_RAT_REAR_POSE_CHANCE))
				{
					item->Animation.TargetState = BIG_RAT_STATE_IDLE;
					item->Animation.RequiredState = BIG_RAT_STATE_REAR_POSE;
				}

				break;

			case BIG_RAT_STATE_LAND_BITE_ATTACK:
				if (!item->Animation.RequiredState && AI.ahead &&
					item->TestBits(JointBitType::Touch, BigRatBite.meshNum))
				{
					DoDamage(creature->Enemy, BIG_RAT_BITE_ATTACK_DAMAGE);
					CreatureEffect(item, BigRatBite, DoBloodSplat);
					item->Animation.RequiredState = BIG_RAT_STATE_IDLE;
				}

				break;

			case BIG_RAT_STATE_POUNCE_ATTACK:
				if (!item->Animation.RequiredState && AI.ahead &&
					item->TestBits(JointBitType::Touch, BigRatBite.meshNum))
				{
					DoDamage(creature->Enemy, BIG_RAT_POUNCE_ATTACK_DAMAGE);
					CreatureEffect(item, BigRatBite, DoBloodSplat);
					item->Animation.RequiredState = BIG_RAT_STATE_RUN_FORWARD;
				}

				break;

			case BIG_RAT_STATE_REAR_POSE:
				if (creature->Mood != MoodType::Bored || TestProbability(BIG_RAT_REAR_POSE_CHANCE))
					item->Animation.TargetState = BIG_RAT_STATE_IDLE;

				break;

			case BIG_RAT_STATE_SWIM:
				creature->MaxTurn = BIG_RAT_SWIM_TURN_RATE_MAX;

				if (!isOnWater)
				{
					SetAnimation(item, BIG_RAT_ANIM_RUN_FORWARD);
					break;
				}

				if (AI.ahead && item->TestBits(JointBitType::Touch, BigRatBite.meshNum))
					item->Animation.TargetState = BIG_RAT_STATE_SWIM_BITE_ATTACK;

				break;

			case BIG_RAT_STATE_SWIM_BITE_ATTACK:
				if (!item->Animation.RequiredState && AI.ahead &&
					item->TestBits(JointBitType::Touch, BigRatBite.meshNum))
				{
					DoDamage(creature->Enemy, BIG_RAT_BITE_ATTACK_DAMAGE);
					CreatureEffect(item, BigRatBite, DoBloodSplat);
				}

				break;
			}

		}

		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNumber, angle, 0);

		if (isOnWater)
		{
			CreatureUnderwater(item, 0);
			item->Pose.Position.y = waterHeight - BIG_RAT_WATER_SURFACE_OFFSET;
		}
		else
			item->Pose.Position.y = item->Floor;
	}
}
