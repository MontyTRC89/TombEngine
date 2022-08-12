#include "framework.h"
#include "Objects/TR5/Entity/tr5_lion.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/prng.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;
using std::vector;

namespace TEN::Entities::TR5
{
	constexpr auto LION_POUNCE_ATTACK_DAMAGE = 200;
	constexpr auto LION_BITE_ATTACK_DAMAGE	 = 60;

	constexpr auto LION_POUNCE_ATTACK_RANGE = SQUARE(SECTOR(1));

	const vector<int> LionAttackJoints = { 3, 6, 21 };
	const auto LionBite1 = BiteInfo(Vector3(2.0f, -10.0f, 250.0f), 21);
	const auto LionBite2 = BiteInfo(Vector3(-2.0f, -10.0f, 132.0f), 21);

	enum LionState
	{
		LION_STATE_NONE = 0,
		LION_STATE_IDLE = 1,
		LION_STATE_WALK_FORWARD = 2,
		LION_STATE_RUN_FORWARD = 3,
		LION_STATE_POUNCE_ATTACK = 4,
		LION_STATE_DEATH = 5,
		LION_STATE_ROAR = 6,
		LION_STATE_BITE_ATTACK = 7
	};

	enum LionAnim
	{
		LION_ANIM_IDLE = 0,
		LION_ANIM_IDLE_TO_WALK_FORWARD = 1,
		LION_ANIM_WALK_FORWARD = 2,
		LION_ANIM_WALK_FORWARD_TO_IDLE = 3,
		LION_ANIM_RUN_FORWARD = 4,
		LION_ANIM_IDLE_TO_RUN_FORWARD = 5,
		LION_ANIM_RUN_FORWARD_TO_IDLE = 6,
		LION_ANIM_DEATH_1 = 7,
		LION_ANIM_DEATH_2 = 8,
		LION_ANIM_POUNCE_ATTACK_START = 9,
		LION_ANIM_POUNCE_ATTACK_END = 10,
		LION_ANIM_BITE_ATTACK = 11,
		LION_ANIM_ROAR = 12
	};

	const std::array LionDeathAnims = { LION_ANIM_DEATH_1, LION_ANIM_DEATH_2 };

	void InitialiseLion(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);
		SetAnimation(item, LION_ANIM_IDLE);
	}

	void LionControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		short angle = 0;
		short tilt = 0;
		short joint0 = 0;
		short joint1 = 0;

		if (CreatureActive(itemNumber))
		{
			auto* creature = GetCreatureInfo(item);

			if (item->HitPoints <= 0)
			{
				item->HitPoints = 0;

				if (item->Animation.ActiveState != LION_STATE_DEATH)
					SetAnimation(item, LionDeathAnims[GenerateInt(0, LionDeathAnims.size() - 1)]);
			}
			else
			{
				AI_INFO AI;
				CreatureAIInfo(item, &AI);

				if (AI.ahead)
					joint1 = AI.angle;

				GetCreatureMood(item, &AI, true);
				CreatureMood(item, &AI, true);

				angle = CreatureTurn(item, creature->MaxTurn);
				joint0 = angle * -16;

				switch (item->Animation.ActiveState)
				{
				case LION_STATE_IDLE:
					creature->MaxTurn = 0;

					if (item->Animation.RequiredState)
					{
						item->Animation.TargetState = item->Animation.RequiredState;
						break;
					}

					if (creature->Mood == MoodType::Bored)
					{
						if (!(GetRandomControl() & 0x3F))
							item->Animation.TargetState = LION_STATE_WALK_FORWARD;

						break;
					}

					if (AI.ahead)
					{
						if (item->TestBits(JointBitType::Touch, LionAttackJoints))
						{
							item->Animation.TargetState = LION_STATE_BITE_ATTACK;
							break;
						}

						if (AI.distance < LION_POUNCE_ATTACK_RANGE)
						{
							item->Animation.TargetState = LION_STATE_POUNCE_ATTACK;
							break;
						}
					}

					item->Animation.TargetState = LION_STATE_RUN_FORWARD;
					break;

				case LION_STATE_WALK_FORWARD:
					creature->MaxTurn = ANGLE(2.0f);

					if (creature->Mood == MoodType::Bored)
					{
						if (TestProbability(0.004f))
						{
							item->Animation.TargetState = LION_STATE_IDLE;
							item->Animation.RequiredState = LION_STATE_ROAR;
						}
					}
					else
						item->Animation.TargetState = LION_STATE_IDLE;

					break;

				case LION_STATE_RUN_FORWARD:
					creature->MaxTurn = ANGLE(5.0f);
					tilt = angle;

					if (creature->Mood != MoodType::Bored)
					{
						if (AI.ahead && AI.distance < LION_POUNCE_ATTACK_RANGE)
							item->Animation.TargetState = LION_STATE_IDLE;
						else if (item->TestBits(JointBitType::Touch, LionAttackJoints) && AI.ahead)
							item->Animation.TargetState = LION_STATE_IDLE;
						else if (creature->Mood != MoodType::Escape)
						{
							if (TestProbability(0.004f))
							{
								item->Animation.TargetState = LION_STATE_IDLE;
								item->Animation.RequiredState = LION_STATE_ROAR;
							}
						}
					}
					else
						item->Animation.TargetState = LION_STATE_IDLE;

					break;

				case LION_STATE_POUNCE_ATTACK:
					if (!item->Animation.RequiredState &&
						item->TestBits(JointBitType::Touch, LionAttackJoints))
					{
						item->Animation.RequiredState = LION_STATE_IDLE;
						DoDamage(creature->Enemy, LION_POUNCE_ATTACK_DAMAGE);
						CreatureEffect2(item, LionBite1, 10, item->Pose.Orientation.y, DoBloodSplat);
					}

					break;

				case LION_STATE_BITE_ATTACK:
					creature->MaxTurn = ANGLE(1.0f);

					if (!item->Animation.RequiredState &&
						item->TestBits(JointBitType::Touch, LionAttackJoints))
					{
						item->Animation.RequiredState = LION_STATE_IDLE;
						DoDamage(creature->Enemy, LION_BITE_ATTACK_DAMAGE);
						CreatureEffect2(item, LionBite2, 10, item->Pose.Orientation.y, DoBloodSplat);
					}

					break;
				}
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
