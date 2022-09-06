#include "framework.h"
#include "Objects/TR1/Entity/tr1_lioness.h"

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

namespace TEN::Entities::TR1
{
	constexpr auto LIONESS_POUNCE_ATTACK_DAMAGE = 100;
	constexpr auto LIONESS_BITE_ATTACK_DAMAGE	 = 30;

	constexpr auto LIONESS_POUNCE_ATTACK_RANGE = SQUARE(SECTOR(1));

	const vector<int> LionAttackJoints = { 3, 6, 21 };
	const auto LionBite1 = BiteInfo(Vector3(2.0f, -10.0f, 250.0f), 21);
	const auto LionBite2 = BiteInfo(Vector3(-2.0f, -10.0f, 132.0f), 21);

	enum LionessState
	{
		LIONESS_STATE_NONE = 0,
		LIONESS_STATE_IDLE = 1,
		LIONESS_STATE_WALK_FORWARD = 2,
		LIONESS_STATE_RUN_FORWARD = 3,
		LIONESS_STATE_POUNCE_ATTACK = 4,
		LIONESS_STATE_DEATH = 5,
		LIONESS_STATE_ROAR = 6,
		LIONESS_STATE_BITE_ATTACK = 7
	};

	enum LionessAnim
	{
		LIONESS_ANIM_IDLE = 0,
		LIONESS_ANIM_IDLE_TO_WALK_FORWARD = 1,
		LIONESS_ANIM_WALK_FORWARD = 2,
		LIONESS_ANIM_WALK_FORWARD_TO_IDLE = 3,
		LIONESS_ANIM_RUN_FORWARD = 4,
		LIONESS_ANIM_IDLE_TO_RUN_FORWARD = 5,
		LIONESS_ANIM_RUN_FORWARD_TO_IDLE = 6,
		LIONESS_ANIM_DEATH_1 = 7,
		LIONESS_ANIM_DEATH_2 = 8,
		LIONESS_ANIM_POUNCE_ATTACK_START = 9,
		LIONESS_ANIM_POUNCE_ATTACK_END = 10,
		LIONESS_ANIM_BITE_ATTACK = 11,
		LIONESS_ANIM_ROAR = 12
	};

	const std::array LionDeathAnims = { LIONESS_ANIM_DEATH_1, LIONESS_ANIM_DEATH_2 };

	void InitialiseLioness(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);
		SetAnimation(item, LIONESS_ANIM_IDLE);
	}

	void LionessControl(short itemNumber)
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

				if (item->Animation.ActiveState != LIONESS_STATE_DEATH)
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
				case LIONESS_STATE_IDLE:
					creature->MaxTurn = 0;

					if (item->Animation.RequiredState)
					{
						item->Animation.TargetState = item->Animation.RequiredState;
						break;
					}

					if (creature->Mood == MoodType::Bored)
					{
						if (!(GetRandomControl() & 0x3F))
							item->Animation.TargetState = LIONESS_STATE_WALK_FORWARD;

						break;
					}

					if (AI.ahead)
					{
						if (item->TestBits(JointBitType::Touch, LionAttackJoints))
						{
							item->Animation.TargetState = LIONESS_STATE_BITE_ATTACK;
							break;
						}

						if (AI.distance < LIONESS_POUNCE_ATTACK_RANGE)
						{
							item->Animation.TargetState = LIONESS_STATE_POUNCE_ATTACK;
							break;
						}
					}

					item->Animation.TargetState = LIONESS_STATE_RUN_FORWARD;
					break;

				case LIONESS_STATE_WALK_FORWARD:
					creature->MaxTurn = ANGLE(2.0f);

					if (creature->Mood == MoodType::Bored)
					{
						if (TestProbability(0.004f))
						{
							item->Animation.TargetState = LIONESS_STATE_IDLE;
							item->Animation.RequiredState = LIONESS_STATE_ROAR;
						}
					}
					else
						item->Animation.TargetState = LIONESS_STATE_IDLE;

					break;

				case LIONESS_STATE_RUN_FORWARD:
					creature->MaxTurn = ANGLE(5.0f);
					tilt = angle;

					if (creature->Mood != MoodType::Bored)
					{
						if (AI.ahead && AI.distance < LIONESS_POUNCE_ATTACK_RANGE)
							item->Animation.TargetState = LIONESS_STATE_IDLE;
						else if (item->TestBits(JointBitType::Touch, LionAttackJoints) && AI.ahead)
							item->Animation.TargetState = LIONESS_STATE_IDLE;
						else if (creature->Mood != MoodType::Escape)
						{
							if (TestProbability(0.004f))
							{
								item->Animation.TargetState = LIONESS_STATE_IDLE;
								item->Animation.RequiredState = LIONESS_STATE_ROAR;
							}
						}
					}
					else
						item->Animation.TargetState = LIONESS_STATE_IDLE;

					break;

				case LIONESS_STATE_POUNCE_ATTACK:
					if (!item->Animation.RequiredState &&
						item->TestBits(JointBitType::Touch, LionAttackJoints))
					{
						item->Animation.RequiredState = LIONESS_STATE_IDLE;
						DoDamage(creature->Enemy, LIONESS_POUNCE_ATTACK_DAMAGE);
						CreatureEffect2(item, LionBite1, 10, item->Pose.Orientation.y, DoBloodSplat);
					}

					break;

				case LIONESS_STATE_BITE_ATTACK:
					creature->MaxTurn = ANGLE(1.0f);

					if (!item->Animation.RequiredState &&
						item->TestBits(JointBitType::Touch, LionAttackJoints))
					{
						item->Animation.RequiredState = LIONESS_STATE_IDLE;
						DoDamage(creature->Enemy, LIONESS_BITE_ATTACK_DAMAGE);
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
