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
#include "Specific/setup.h"

using std::vector;

namespace TEN::Entities::TR1
{
	constexpr auto BIG_RAT_BITE_DAMAGE	 = 20;
	constexpr auto BIG_RAT_CHARGE_DAMAGE = 25;

	constexpr auto DEFAULT_SWIM_UPDOWN_SPEED = 32;

	constexpr auto BIG_RAT_ALERT_RANGE		= SQUARE(SECTOR(1.5f));
	constexpr auto BIG_RAT_VISIBILITY_RANGE = SQUARE(SECTOR(5));
	constexpr auto BIG_RAT_BITE_RANGE		= SQUARE(SECTOR(0.34f));
	constexpr auto BIG_RAT_CHARGE_RANGE		= SQUARE(SECTOR(0.5f));
	constexpr auto BIG_RAT_WATER_BITE_RANGE = SQUARE(SECTOR(0.3f));

	constexpr auto BIG_RAT_POSE_CHANCE = 0x100;

	#define BIG_RAT_RUN_TURN_RATE_MAX  ANGLE(6.0f)
	#define BIG_RAT_SWIM_TURN_RATE_MAX ANGLE(3.0f)

	const auto BigRatBite = BiteInfo(Vector3(0.0f, -11.0f, 108.0f), 3);

	enum BigRatState
	{
		BIG_RAT_STATE_EMPTY = 0,
		BIG_RAT_STATE_IDLE = 1,
		BIG_RAT_STATE_CHARGE_ATTACK = 2,
		BIG_RAT_STATE_RUN = 3,
		BIG_RAT_STATE_BITE_ATTACK = 4,
		BIG_RAT_STATE_LAND_DEATH = 5,
		BIG_RAT_STATE_POSE = 6,
		BIG_RAT_STATE_SWIM = 7,
		BIG_RAT_STATE_SWIM_ATTACK = 8,
		BIG_RAT_STATE_WATER_DEATH = 9
	};

	// TODO
	enum BigRatAnim
	{
		BIG_RAT_ANIM_IDLE = 0,
		BIG_RAT_ANIM_STOP_TO_RUN = 1,
		BIG_RAT_ANIM_RUN = 2,
		BIG_RAT_ANIM_RUN_TO_STOP = 3,
		BIG_RAT_ANIM_POSE = 4,
		BIG_RAT_ANIM_POSE_TO_STOP = 5,
		BIG_RAT_ANIM_LAND_BITE_ATTACK = 6,
		BIG_RAT_ANIM_CHARGE_ATTACK = 7,
		BIG_RAT_ANIM_LAND_DEATH = 8,
		BIG_RAT_ANIM_SWIM = 9,
		BIG_RAT_ANIM_WATER_BITE = 10,
		BIG_RAT_ANIM_WATER_DEATH = 11,
		BIG_RAT_ANIM_RUN_TO_SWIM = 12, // NOTE: These animation (12 & 13) not exist for the rat ! -- TokyoSU, 10/8/2022
		BIG_RAT_ANIM_SWIM_TO_RUN = 13
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

	static int RatIsInWater(ItemInfo* item, CreatureInfo* creature)
	{
		auto probe = GetCollision(item);
		int waterDepth = GetWaterSurface(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, probe.RoomNumber);
		if (waterDepth != NO_HEIGHT)
		{
			creature->LOT.Step = SECTOR(20);
			creature->LOT.Drop = -SECTOR(20);
			creature->LOT.Fly = DEFAULT_SWIM_UPDOWN_SPEED;
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

		int waterHeight = RatIsInWater(item, creature);
		short head = 0;
		short angle = 0;
		bool onWater = waterHeight != NO_HEIGHT;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != BIG_RAT_STATE_LAND_DEATH &&
				item->Animation.ActiveState != BIG_RAT_STATE_WATER_DEATH)
			{
				if (onWater)
					SetAnimation(item, BIG_RAT_ANIM_WATER_DEATH);
				else
					SetAnimation(item, BIG_RAT_ANIM_LAND_DEATH);
			}

			if (onWater)
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
				else if (AI.bite && AI.distance < BIG_RAT_BITE_RANGE)
					item->Animation.TargetState = BIG_RAT_STATE_BITE_ATTACK;
				else
					item->Animation.TargetState = BIG_RAT_STATE_RUN;

				break;

			case BIG_RAT_STATE_RUN:
				creature->MaxTurn = BIG_RAT_RUN_TURN_RATE_MAX;

				TENLog("WaterHeight: " + std::to_string(waterHeight));
				if (onWater)
				{
					SetAnimation(item, BIG_RAT_ANIM_SWIM);
					break;
				}

				if (AI.ahead && item->TestBits(JointBitType::Touch, BigRatBite.meshNum))
					item->Animation.TargetState = BIG_RAT_STATE_IDLE;
				else if (AI.bite && AI.distance < BIG_RAT_CHARGE_RANGE)
					item->Animation.TargetState = BIG_RAT_STATE_CHARGE_ATTACK;
				else if (AI.ahead && GetRandomControl() < BIG_RAT_POSE_CHANCE)
				{
					item->Animation.TargetState = BIG_RAT_STATE_IDLE;
					item->Animation.RequiredState = BIG_RAT_STATE_POSE;
				}

				break;

			case BIG_RAT_STATE_BITE_ATTACK:
				if (!item->Animation.RequiredState && AI.ahead &&
					item->TestBits(JointBitType::Touch, BigRatBite.meshNum))
				{
					DoDamage(creature->Enemy, BIG_RAT_BITE_DAMAGE);
					CreatureEffect(item, BigRatBite, DoBloodSplat);
					item->Animation.RequiredState = BIG_RAT_STATE_IDLE;
				}

				break;

			case BIG_RAT_STATE_CHARGE_ATTACK:
				if (!item->Animation.RequiredState && AI.ahead &&
					item->TestBits(JointBitType::Touch, BigRatBite.meshNum))
				{
					DoDamage(creature->Enemy, BIG_RAT_CHARGE_DAMAGE);
					CreatureEffect(item, BigRatBite, DoBloodSplat);
					item->Animation.RequiredState = BIG_RAT_STATE_RUN;
				}

				break;

			case BIG_RAT_STATE_POSE:
				if (creature->Mood != MoodType::Bored || GetRandomControl() < BIG_RAT_POSE_CHANCE)
					item->Animation.TargetState = BIG_RAT_STATE_IDLE;

				break;

			case BIG_RAT_STATE_SWIM:
				creature->MaxTurn = BIG_RAT_SWIM_TURN_RATE_MAX;

				if (!onWater)
				{
					SetAnimation(item, BIG_RAT_ANIM_RUN);
					break;
				}

				if (AI.ahead && item->TestBits(JointBitType::Touch, BigRatBite.meshNum))
					item->Animation.TargetState = BIG_RAT_STATE_SWIM_ATTACK;

				break;

			case BIG_RAT_STATE_SWIM_ATTACK:
				if (!item->Animation.RequiredState && AI.ahead &&
					item->TestBits(JointBitType::Touch, BigRatBite.meshNum))
				{
					DoDamage(creature->Enemy, BIG_RAT_BITE_DAMAGE);
					CreatureEffect(item, BigRatBite, DoBloodSplat);
				}
				break;
			}

		}

		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNumber, angle, 0);

		if (onWater)
		{
			CreatureUnderwater(item, 0);
			item->Pose.Position.y = waterHeight - 10;
		}
		else
			item->Pose.Position.y = item->Floor;
	}
}
