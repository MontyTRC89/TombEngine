#include "framework.h"
#include "Objects/TR3/Entity/tr3_raptor.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/prng.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;
using std::vector;

namespace TEN::Entities::TR3
{
	constexpr auto RAPTOR_ATTACK_DAMAGE = 100;

	#define RAPTOR_WALK_TURN_RATE_MAX	ANGLE(2.0f)
	#define RAPTOR_RUN_TURN_RATE_MAX	ANGLE(2.0f)
	#define RAPTOR_ATTACK_TURN_RATE_MAX ANGLE(2.0f)

	const auto RaptorBite = BiteInfo(Vector3(0.0f, 66.0f, 318.0f), 22);
	const vector<int> RaptorAttackJoints = { 10, 11, 12, 13, 14, 16, 17, 18, 19, 20, 21, 22, 23 };

	enum RaptorState
	{
		RAPTOR_STATE_NONE = 0,
		RAPTOR_STATE_IDLE = 1,
		RAPTOR_STATE_WALK_FORWARD = 2,
		RAPTOR_STATE_RUN_FORWARD = 3,
		RAPTOR_STATE_JUMP_ATTACK = 4,
		RAPTOR_STATE_DEATH = 5,
		RAPTOR_STATE_ROAR = 6,
		RAPTOR_STATE_RUN_BITE_ATTACK = 7,
		RAPTOR_STATE_BITE_ATTACK = 8
	};

	enum RaptorAnim
	{
		RAPTOR_ANIM_IDLE = 0,
		RAPTOR_ANIM_RUN_FORWARD = 1,
		RAPTOR_ANIM_RUN_FORWARD_TO_IDLE = 2,
		RAPTOR_ANIM_IDLE_TO_RUN_FORWARD = 3,
		RAPTOR_ANIM_ROAR = 4,
		RAPTOR_ANIM_WALK_FORWARD = 5,
		RAPTOR_ANIM_WALK_FORWARD_TO_IDLE = 6,
		RAPTOR_ANIM_IDLE_TO_WALK_FORWARD = 7,
		RAPTOR_ANIM_RUN_BITE_ATTACK = 8,
		RAPTOR_ANIM_DEATH_1 = 9,
		RAPTOR_ANIM_DEATH_2 = 10,
		RAPTOR_ANIM_JUMP_ATTACK_START = 11,
		RAPTOR_ANIM_JUMP_ATTACK_END = 12,
		RAPTOR_ANIM_BITE_ATTACK = 13
	};

	// TODO
	enum RaptorFlags
	{

	};

	const std::array RaptorDeathAnims = { RAPTOR_ANIM_DEATH_1, RAPTOR_ANIM_DEATH_2, };

	void RaptorControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short head = 0;
		short neck = 0;
		short angle = 0;
		short tilt = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != RAPTOR_STATE_DEATH)
				SetAnimation(item, RaptorDeathAnims[GenerateInt(0, RaptorDeathAnims.size() - 1)]);
		}
		else
		{
			if (creature->Enemy == nullptr || !(GetRandomControl() & 0x7F)) // TODO: Probability is 0.004f or 0.996f?
			{
				ItemInfo* nearestItem = nullptr;
				int minDistance = INT_MAX;

				for (int i = 0; i < ActiveCreatures.size(); i++)
				{
					auto* currentCreature = ActiveCreatures[i];
					if (currentCreature->ItemNumber == NO_ITEM || currentCreature->ItemNumber == itemNumber)
					{
						currentCreature++;
						continue;
					}

					auto* targetItem = &g_Level.Items[currentCreature->ItemNumber];

					int x = (targetItem->Pose.Position.x - item->Pose.Position.x) / 64;
					int y = (targetItem->Pose.Position.y - item->Pose.Position.y) / 64;
					int z = (targetItem->Pose.Position.z - item->Pose.Position.z) / 64;

					int distance = pow(x, 2) + pow(y, 2) + pow(z, 2);
					if (distance < minDistance && item->HitPoints > 0)
					{
						nearestItem = targetItem;
						minDistance = distance;
					}

					currentCreature++;
				}

				if (nearestItem != nullptr &&
					(nearestItem->ObjectNumber != ID_RAPTOR ||
						(TestProbability(0.03f) && minDistance < pow(SECTOR(2), 2))))
				{
					creature->Enemy = nearestItem;
				}

				int x = (LaraItem->Pose.Position.x - item->Pose.Position.x) / 64;
				int y = (LaraItem->Pose.Position.y - item->Pose.Position.y) / 64;
				int z = (LaraItem->Pose.Position.z - item->Pose.Position.z) / 64;

				int distance = pow(x, 2) + pow(y, 2) + pow(z, 2);
				if (distance <= minDistance)
					creature->Enemy = LaraItem;
			}

			if (item->AIBits)
				GetAITarget(creature);

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			if (creature->Mood == MoodType::Bored)
				creature->MaxTurn /= 2;

			angle = CreatureTurn(item, creature->MaxTurn);
			neck = -angle * 6;

			switch (item->Animation.ActiveState)
			{
			case RAPTOR_STATE_IDLE:
				creature->MaxTurn = 0;
				creature->Flags &= ~1;

				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (creature->Flags & 2)
				{
					creature->Flags &= ~2;
					item->Animation.TargetState = RAPTOR_STATE_ROAR;
				}
				else if (item->TestBits(JointBitType::Touch, RaptorAttackJoints) ||
					(AI.distance < pow(585, 2) && AI.bite))
				{
					item->Animation.TargetState = RAPTOR_STATE_BITE_ATTACK;
				}
				else if (AI.bite && AI.distance < pow(SECTOR(1.5f), 2))
					item->Animation.TargetState = RAPTOR_STATE_JUMP_ATTACK;
				else if (creature->Mood == MoodType::Escape &&
					Lara.TargetEntity != item && AI.ahead && !item->HitStatus)
				{
					item->Animation.TargetState = RAPTOR_STATE_IDLE;
				}
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = RAPTOR_STATE_WALK_FORWARD;
				else
					item->Animation.TargetState = RAPTOR_STATE_RUN_FORWARD;

				break;

			case RAPTOR_STATE_WALK_FORWARD:
				creature->MaxTurn = RAPTOR_WALK_TURN_RATE_MAX;
				creature->Flags &= ~1;

				if (creature->Mood != MoodType::Bored)
					item->Animation.TargetState = RAPTOR_STATE_IDLE;
				else if (AI.ahead && TestProbability(0.004f))
				{
					item->Animation.TargetState = RAPTOR_STATE_IDLE;
					item->Animation.RequiredState = RAPTOR_STATE_ROAR;
					creature->Flags &= ~2;
				}

				break;

			case RAPTOR_STATE_RUN_FORWARD:
				creature->MaxTurn = RAPTOR_RUN_TURN_RATE_MAX;
				creature->Flags &= ~1;
				tilt = angle;

				if (item->TestBits(JointBitType::Touch, RaptorAttackJoints))
					item->Animation.TargetState = RAPTOR_STATE_IDLE;
				else if (creature->Flags & 2)
				{
					item->Animation.TargetState = RAPTOR_STATE_IDLE;
					item->Animation.RequiredState = RAPTOR_STATE_ROAR;
					creature->Flags &= ~2;
				}
				else if (AI.bite && AI.distance < pow(SECTOR(1.5f), 2))
				{
					if (item->Animation.TargetState == RAPTOR_STATE_RUN_FORWARD)
					{
						if (TestProbability(0.25f))
							item->Animation.TargetState = RAPTOR_STATE_IDLE;
						else
							item->Animation.TargetState = RAPTOR_STATE_RUN_BITE_ATTACK;
					}
				}
				else if (AI.ahead && creature->Mood != MoodType::Escape &&
					TestProbability(0.004f))
				{
					item->Animation.TargetState = RAPTOR_STATE_IDLE;
					item->Animation.RequiredState = RAPTOR_STATE_ROAR;
				}
				else if (creature->Mood == MoodType::Bored ||
					(creature->Mood == MoodType::Escape && Lara.TargetEntity != item && AI.ahead))
				{
					item->Animation.TargetState = RAPTOR_STATE_IDLE;
				}

				break;

			case RAPTOR_STATE_JUMP_ATTACK:
				creature->MaxTurn = RAPTOR_ATTACK_TURN_RATE_MAX;
				tilt = angle;

				if (creature->Enemy->IsLara())
				{
					if (!(creature->Flags & 1) &&
						item->TestBits(JointBitType::Touch, RaptorAttackJoints))
					{
						DoDamage(creature->Enemy, RAPTOR_ATTACK_DAMAGE);
						CreatureEffect(item, RaptorBite, DoBloodSplat);
						creature->Flags |= 1;

						if (LaraItem->HitPoints <= 0)
							creature->Flags |= 2;

						item->Animation.RequiredState = RAPTOR_STATE_IDLE;
					}
				}
				else
				{
					if (!(creature->Flags & 1) && creature->Enemy != nullptr)
					{
						auto direction = creature->Enemy->Pose.Position - item->Pose.Position;
						if (abs(direction.x) < SECTOR(0.5f) &&
							abs(direction.y) < SECTOR(0.5f) &&
							abs(direction.z) < SECTOR(0.5f))
						{
							if (creature->Enemy->HitPoints <= 0)
								creature->Flags |= 2;

							DoDamage(creature->Enemy, 25);
							CreatureEffect(item, RaptorBite, DoBloodSplat);
							creature->Flags |= 1;
						}
					}
				}

				break;

			case RAPTOR_STATE_BITE_ATTACK:
				creature->MaxTurn = RAPTOR_ATTACK_TURN_RATE_MAX;
				tilt = angle;

				if (creature->Enemy->IsLara())
				{
					if (!(creature->Flags & 1) &&
						item->TestBits(JointBitType::Touch, RaptorAttackJoints))
					{
						DoDamage(creature->Enemy, RAPTOR_ATTACK_DAMAGE);
						CreatureEffect(item, RaptorBite, DoBloodSplat);
						creature->Flags |= 1;

						if (LaraItem->HitPoints <= 0)
							creature->Flags |= 2;

						item->Animation.RequiredState = RAPTOR_STATE_IDLE;
					}
				}
				else
				{
					if (!(creature->Flags & 1) && creature->Enemy != nullptr)
					{
						auto direction = creature->Enemy->Pose.Position - item->Pose.Position;
						if (abs(direction.x) < SECTOR(0.5f) &&
							abs(direction.y) < SECTOR(0.5f) &&
							abs(direction.z) < SECTOR(0.5f))
						{
							if (creature->Enemy->HitPoints <= 0)
								creature->Flags |= 2;

							DoDamage(creature->Enemy, 25);
							CreatureEffect(item, RaptorBite, DoBloodSplat);
							creature->Flags |= 1;
						}
					}
				}

				break;

			case RAPTOR_STATE_RUN_BITE_ATTACK:
				creature->MaxTurn = RAPTOR_ATTACK_TURN_RATE_MAX;
				tilt = angle;

				if (creature->Enemy->IsLara())
				{
					if (!(creature->Flags & 1) &&
						item->TestBits(JointBitType::Touch, RaptorAttackJoints))
					{
						DoDamage(creature->Enemy, RAPTOR_ATTACK_DAMAGE);
						CreatureEffect(item, RaptorBite, DoBloodSplat);
						item->Animation.RequiredState = RAPTOR_STATE_RUN_FORWARD;
						creature->Flags |= 1;

						if (LaraItem->HitPoints <= 0)
							creature->Flags |= 2;
					}
				}
				else
				{
					if (!(creature->Flags & 1) && creature->Enemy != nullptr)
					{
						auto direction = creature->Enemy->Pose.Position - item->Pose.Position;
						if (abs(direction.x) < SECTOR(0.5f) &&
							abs(direction.y) < SECTOR(0.5f) &&
							abs(direction.z) < SECTOR(0.5f))
						{
							if (creature->Enemy->HitPoints <= 0)
								creature->Flags |= 2;

							DoDamage(creature->Enemy, 25);
							CreatureEffect(item, RaptorBite, DoBloodSplat);
							creature->Flags |= 1;
						}
					}
				}

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, head / 2);
		CreatureJoint(item, 1, head / 2);
		CreatureJoint(item, 2, neck);
		CreatureJoint(item, 3, neck);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
