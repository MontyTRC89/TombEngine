#include "framework.h"
#include "tr4_big_beetle.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Specific/trmath.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Game/itemdata/creature_info.h"

using std::vector;

namespace TEN::Entities::TR4
{
	BITE_INFO BigBeetleBite = { 0, 0, 0, 12 };
	const vector<int> BigBeetleAttackJoints = { 5, 6 };

	constexpr auto BIG_BEETLE_ATTACK_DAMAGE = 50;

	constexpr auto BIG_BEETLE_ATTACK_RANGE = CLICK(1);
	constexpr auto BIG_BEETLE_AWARE_RANGE = CLICK(3);

	enum BigBeetleState
	{
		BBEETLE_STATE_NONE = 0,
		BBEETLE_STATE_IDLE = 1,
		BBEETLE_STATE_TAKE_OFF = 2,
		BBEETLE_STATE_FLY_FORWARD = 3,
		BBEETLE_STATE_ATTACK = 4,
		BBEETLE_STATE_LAND = 5,
		BBEETLE_STATE_DEATH_START = 6,
		BBEETLE_STATE_DEATH_FALL = 7,
		BBEETLE_STATE_DEATH_END = 8,
		BBEETLE_STATE_FLY_IDLE = 9,
	};

	enum BigBeetleAnim
	{
		BBEETLE_ANIM_IDLE_TO_FLY_FORWARD = 0,
		BBEETLE_ANIM_FLY_FORWARD = 1,
		BBEETLE_ANIM_FLY_FORWARD_TO_IDLE = 2,
		BBEETLE_ANIM_IDLE = 3,
		BBEETLE_ANIM_ATTACK = 4,
		BBEETLE_ANIM_DEATH_START = 5,
		BBEETLE_ANIM_DEATH_FALL = 6,
		BBEETLE_ANIM_DEATH_END = 7,
		BBEETLE_ANIM_FLY_IDLE = 8,
		BBEETLE_ANIM_FLY_FORWARD_TO_FLY_IDLE = 9,
		BBEETLE_ANIM_FLY_IDLE_TO_FLY_FORWARD = 10,
	};

	// TODO
	enum BigBeetleFlags
	{

	};

	void InitialiseBigBeetle(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + BBEETLE_ANIM_IDLE;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = BBEETLE_STATE_IDLE;
		item->Animation.TargetState = BBEETLE_STATE_IDLE;
	}

	void BigBeetleControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != BBEETLE_STATE_DEATH_START)
			{
				if (item->Animation.ActiveState != BBEETLE_STATE_DEATH_FALL)
				{
					if (item->Animation.ActiveState == BBEETLE_STATE_DEATH_END)
					{
						item->Pose.Orientation.x = 0;
						item->Pose.Position.y = item->Floor;
					}
					else
					{
						item->Pose.Orientation.x = 0;
						item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + BBEETLE_ANIM_DEATH_START;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
						item->Animation.ActiveState = BBEETLE_STATE_DEATH_START;
						item->Animation.IsAirborne = true;
						item->Animation.Velocity = 0;
					}
				}
				else if (item->Pose.Position.y >= item->Floor)
				{
					item->Pose.Position.y = item->Floor;
					item->Animation.IsAirborne = false;
					item->Animation.VerticalVelocity = 0;
					item->Animation.TargetState = BBEETLE_STATE_DEATH_END;
				}
			}

			item->Pose.Orientation.x = 0;
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, VIOLENT);

			if (creature->Flags)
				creature->Mood = MoodType::Escape;

			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, creature->MaxTurn);

			if (item->HitStatus ||
				AI.distance > pow(BIG_BEETLE_AWARE_RANGE, 2) ||
				!(GetRandomControl() & 0x7F))
			{
				creature->Flags = 0;
			}

			switch (item->Animation.ActiveState)
			{
			case BBEETLE_STATE_IDLE:
				item->Pose.Position.y = item->Floor;
				creature->MaxTurn = ANGLE(1.0f);

				if (item->HitStatus ||
					item->AIBits == MODIFY ||
					creature->HurtByLara ||
					AI.distance < pow(BIG_BEETLE_AWARE_RANGE, 2))
				{
					item->Animation.TargetState = BBEETLE_STATE_TAKE_OFF;
				}

				break;

			case BBEETLE_STATE_FLY_FORWARD:
				creature->MaxTurn = ANGLE(7.0f);

				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (AI.ahead)
				{
					if (AI.distance < pow(BIG_BEETLE_ATTACK_RANGE, 2))
						item->Animation.TargetState = BBEETLE_STATE_FLY_IDLE;
				}

				break;

			case BBEETLE_STATE_ATTACK:
				creature->MaxTurn = ANGLE(7.0f);

				if (AI.ahead)
				{
					if (AI.distance < pow(BIG_BEETLE_ATTACK_RANGE, 2))
						item->Animation.TargetState = BBEETLE_STATE_ATTACK;
				}
				else if (AI.distance < pow(BIG_BEETLE_ATTACK_RANGE, 2))
					item->Animation.TargetState = BBEETLE_STATE_FLY_IDLE;
				else
				{
					item->Animation.RequiredState = BBEETLE_STATE_FLY_FORWARD;
					item->Animation.TargetState = BBEETLE_STATE_FLY_IDLE;
				}

				if (!creature->Flags)
				{
					if (item->TestBits(JointBitType::Touch, BigBeetleAttackJoints))
					{
						DoDamage(creature->Enemy, BIG_BEETLE_ATTACK_DAMAGE);

						CreatureEffect2(
							item,
							&BigBeetleBite,
							5,
							-1,
							DoBloodSplat);
						creature->Flags = 1;
					}
				}

				break;

			case BBEETLE_STATE_LAND:
				creature->Flags = 0;

				item->Pose.Position.y += 51;
				if (item->Pose.Position.y > item->Floor)
					item->Pose.Position.y = item->Floor;

				break;

			case BBEETLE_STATE_FLY_IDLE:
				creature->MaxTurn = ANGLE(7.0f);

				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (!item->HitStatus && item->AIBits != MODIFY && GetRandomControl() >= 384 &&
					((creature->Mood != MoodType::Bored && GetRandomControl() >= 128) ||
						creature->HurtByLara || item->AIBits == MODIFY))
				{
					if (AI.ahead)
					{
						if (AI.distance < pow(BIG_BEETLE_ATTACK_RANGE, 2) && !creature->Flags)
							item->Animation.TargetState = BBEETLE_STATE_ATTACK;
					}
				}
				else
					item->Animation.TargetState = BBEETLE_STATE_FLY_FORWARD;

				break;

			default:
				break;

			}
		}

		CreatureTilt(item, angle * 2);
		CreatureAnimation(itemNumber, angle, angle);
	}
}
