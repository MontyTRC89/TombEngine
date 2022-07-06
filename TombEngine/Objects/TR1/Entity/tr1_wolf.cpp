#include "framework.h"
#include "Objects/TR1/Entity/tr1_wolf.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using std::vector;

namespace TEN::Entities::TR1
{
	BITE_INFO WolfBite = { 0, -14, 174, 6 };
	const vector<int> WolfAttackJoints = { 0, 1, 2, 3, 6, 8, 9, 10, 12, 13, 14 };

	constexpr auto WOLF_BITE_DAMAGE = 100;
	constexpr auto WOLF_LUNGE_DAMAGE = 50;

	constexpr auto WOLF_ATTACK_RANGE = SECTOR(1.5f);
	constexpr auto WOLF_STALK_RANGE = SECTOR(2);

	constexpr auto WOLF_WAKE_CHANCE = 0x20;
	constexpr auto WOLF_SLEEP_CHANCE = 0x20;
	constexpr auto WOLF_HOWL_CHANCE = 0x180;

	constexpr auto WOLF_SLEEP_FRAME = 96;

	#define WOLF_WALK_TURN_ANGLE ANGLE(2.0f)
	#define WOLF_RUN_TURN_ANGLE ANGLE(5.0f)
	#define WOLF_STALK_TURN_ANGLE ANGLE(2.0f)

	enum WolfState
	{
		WOLF_STATE_NONE = 0,
		WOLF_STATE_IDLE = 1,
		WOLF_STATE_WALK = 2,
		WOLF_STATE_RUN = 3,
		WOLF_STATE_JUMP = 4,
		WOLF_STATE_STALK = 5,
		WOLF_STATE_ATTACK = 6,
		WOLF_STATE_HOWL = 7,
		WOLF_STATE_SLEEP = 8,
		WOLF_STATE_CROUCH = 9,
		WOLF_STATE_FAST_TURN = 10,
		WOLF_STATE_DEATH = 11,
		WOLF_STATE_BITE = 12
	};

	// TODO
	enum WolfAnim
	{
		WOLF_ANIM_DEATH = 20,
	};

	void InitialiseWolf(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);
		item->Animation.FrameNumber = WOLF_SLEEP_FRAME;
	}

	void WolfControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short head = 0;
		short angle = 0;
		short tilt = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != WOLF_STATE_DEATH)
			{
				item->Animation.AnimNumber = Objects[ID_WOLF].animIndex + WOLF_ANIM_DEATH + (short)(GetRandomControl() / 11000);
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = WOLF_STATE_DEATH;
			}
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;

			GetCreatureMood(item, &AI, TIMID);
			CreatureMood(item, &AI, TIMID);

			if (item->Animation.ActiveState != WOLF_STATE_SLEEP)
				angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case WOLF_STATE_SLEEP:
				head = 0;

				if (creature->Mood == MoodType::Escape || AI.zoneNumber == AI.enemyZone)
				{
					item->Animation.RequiredState = WOLF_STATE_CROUCH;
					item->Animation.TargetState = WOLF_STATE_IDLE;
				}
				else if (GetRandomControl() < WOLF_WAKE_CHANCE)
				{
					item->Animation.RequiredState = WOLF_STATE_WALK;
					item->Animation.TargetState = WOLF_STATE_IDLE;
				}

				break;

			case WOLF_STATE_IDLE:
				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else
					item->Animation.TargetState = WOLF_STATE_WALK;
				break;

			case WOLF_STATE_WALK:
				creature->MaxTurn = WOLF_WALK_TURN_ANGLE;

				if (creature->Mood != MoodType::Bored)
				{
					item->Animation.TargetState = WOLF_STATE_STALK;
					item->Animation.RequiredState = WOLF_STATE_NONE;
				}
				else if (GetRandomControl() < WOLF_SLEEP_CHANCE)
				{
					item->Animation.RequiredState = WOLF_STATE_SLEEP;
					item->Animation.TargetState = WOLF_STATE_IDLE;
				}

				break;

			case WOLF_STATE_CROUCH:
				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = WOLF_STATE_RUN;
				else if (AI.distance < pow(345, 2) && AI.bite)
					item->Animation.TargetState = WOLF_STATE_BITE;
				else if (creature->Mood == MoodType::Stalk)
					item->Animation.TargetState = WOLF_STATE_STALK;
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = WOLF_STATE_IDLE;
				else
					item->Animation.TargetState = WOLF_STATE_RUN;

				break;

			case WOLF_STATE_STALK:
				creature->MaxTurn = WOLF_STALK_TURN_ANGLE;

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = WOLF_STATE_RUN;
				else if (AI.distance < pow(345, 2) && AI.bite)
					item->Animation.TargetState = WOLF_STATE_BITE;
				else if (AI.distance > pow(SECTOR(3), 2))
					item->Animation.TargetState = WOLF_STATE_RUN;
				else if (creature->Mood == MoodType::Attack)
				{
					if (!AI.ahead || AI.distance > pow(SECTOR(1.5f), 2) ||
						(AI.enemyFacing < FRONT_ARC && AI.enemyFacing > -FRONT_ARC))
					{
						item->Animation.TargetState = WOLF_STATE_RUN;
					}
				}
				else if (GetRandomControl() < WOLF_HOWL_CHANCE)
				{
					item->Animation.RequiredState = WOLF_STATE_HOWL;
					item->Animation.TargetState = WOLF_STATE_CROUCH;
				}
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = WOLF_STATE_CROUCH;

				break;

			case WOLF_STATE_RUN:
				creature->MaxTurn = WOLF_RUN_TURN_ANGLE;
				tilt = angle;

				if (AI.ahead && AI.distance < pow(WOLF_ATTACK_RANGE, 2))
				{
					if (AI.distance > (pow(WOLF_ATTACK_RANGE, 2) / 2) &&
						(AI.enemyFacing > FRONT_ARC || AI.enemyFacing < -FRONT_ARC))
					{
						item->Animation.RequiredState = WOLF_STATE_STALK;
						item->Animation.TargetState = WOLF_STATE_CROUCH;
					}
					else
					{
						item->Animation.TargetState = WOLF_STATE_ATTACK;
						item->Animation.RequiredState = WOLF_STATE_NONE;
					}
				}
				else if (creature->Mood == MoodType::Stalk && AI.distance < pow(WOLF_STALK_RANGE, 2))
				{
					item->Animation.RequiredState = WOLF_STATE_STALK;
					item->Animation.TargetState = WOLF_STATE_CROUCH;
				}
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = WOLF_STATE_CROUCH;

				break;

			case WOLF_STATE_ATTACK:
				tilt = angle;

				if (!item->Animation.RequiredState && item->TestBits(JointBitType::Touch, WolfAttackJoints))
				{
					CreatureEffect(item, &WolfBite, DoBloodSplat);
					DoDamage(creature->Enemy, WOLF_LUNGE_DAMAGE);
					item->Animation.RequiredState = WOLF_STATE_RUN;
				}

				item->Animation.TargetState = WOLF_STATE_RUN;
				break;

			case WOLF_STATE_BITE:
				if (AI.ahead && !item->Animation.RequiredState &&
					item->TestBits(JointBitType::Touch, WolfAttackJoints))
				{
					CreatureEffect(item, &WolfBite, DoBloodSplat);
					DoDamage(creature->Enemy, WOLF_BITE_DAMAGE);
					item->Animation.RequiredState = WOLF_STATE_CROUCH;
				}

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
