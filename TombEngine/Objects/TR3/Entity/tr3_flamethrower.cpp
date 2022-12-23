#include "framework.h"
#include "Objects/TR3/Entity/tr3_flamethrower.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto FLAMETHROWER_ATTACK_RANGE = SQUARE(SECTOR(1.5f));
	constexpr auto FLAMETHROWER_IDLE_RANGE	 = SQUARE(SECTOR(2));
	constexpr auto FLAMETHROWER_AWARE_RANGE	 = SQUARE(SECTOR(8));

	constexpr auto FLAMETHROWER_WALK_TURN_RATE_MAX = ANGLE(5.0f);

	const auto FlamethrowerOffset = Vector3i(0, 340, 0);
	const auto FlamethrowerBite = BiteInfo(Vector3(0.0f, 340.0f, 64.0f), 7);

	// TODO
	enum FlamethrowerState
	{
		// No state 0.
		FLAMETHROWER_STATE_IDLE = 1,
		FLAMETHROWER_STATE_WALK_FORWARD = 2,
		FLAME_STATE_IDLE = 3,
		FLAME_STATE_WAIT = 4,
		FLAMETHROWER_STATE_WALK_FORWARD_ATTACK = 6,
		FLAME_STATE_DEATH = 7,
		FLAMETHROWER_STATE_WALK_FORWARD_AIM = 9,
		FLAMETHROWER_STATE_AIM_1 = 10,
		FLAMETHROWER_STATE_ATTACK_1 = 11
	};

	enum FlamethrowerAnim
	{
		FLAME_ANIM_IDLE = 12,
		FLAME_ANIM_DEATH = 19
	};

	// TODO: Remove LaraItem dependencies.
	void FlameThrowerControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short tilt = 0;
		auto extraHeadRot = EulerAngles::Zero;
		auto extraTorsoRot = EulerAngles::Zero;

		auto pos = GetJointPosition(item, FlamethrowerBite.meshNum, Vector3i(FlamethrowerBite.Position));

		int randomInt = GetRandomControl();
		if (item->Animation.ActiveState != 6 && item->Animation.ActiveState != 11)
		{
			TriggerDynamicLight(pos.x, pos.y, pos.z, (randomInt & 3) + 6, 24 - ((randomInt / 16) & 3), 16 - ((randomInt / 64) & 3), randomInt & 3);
			TriggerPilotFlame(itemNumber, 9);
		}
		else
		{
			TriggerDynamicLight(pos.x, pos.y, pos.z, (randomInt & 3) + 10, 31 - ((randomInt / 16) & 3), 24 - ((randomInt / 64) & 3), randomInt & 7);
		}

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != FLAME_STATE_DEATH)
				SetAnimation(item, FLAME_ANIM_DEATH);
		}
		else
		{
			if (item->AIBits)
			{
				GetAITarget(creature);
			}
			else if (creature->HurtByLara)
			{
				creature->Enemy = LaraItem;
			}
			else
			{
				creature->Enemy = nullptr;

				ItemInfo* target = nullptr;
				int minDistance = INT_MAX;

				for (auto& currentCreature : ActiveCreatures)
				{
					if (currentCreature->ItemNumber == NO_ITEM || currentCreature->ItemNumber == itemNumber)
						continue;

					target = &g_Level.Items[currentCreature->ItemNumber];
					if (target->ObjectNumber == ID_LARA || target->HitPoints <= 0 || target->ObjectNumber == ID_FLAMETHROWER_BADDY)
						continue;

					int x = target->Pose.Position.x - item->Pose.Position.x;
					int z = target->Pose.Position.z - item->Pose.Position.z;

					int distance = SQUARE(z) + SQUARE(x);
					if (distance < minDistance)
					{
						creature->Enemy = target;
						minDistance = distance;
					}
				}
			}

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			AI_INFO laraAI;
			if (creature->Enemy == LaraItem)
			{
				laraAI.angle = AI.angle;
				laraAI.distance = AI.distance;

				if (!creature->HurtByLara)
					creature->Enemy = nullptr;
			}
			else
			{
				int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;

				laraAI.angle = phd_atan(dz, dz) - item->Pose.Orientation.y;
				laraAI.distance = SQUARE(dz) + SQUARE(dx);

				AI.xAngle -= ANGLE(11.25f);
			}

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);
			auto* realEnemy = creature->Enemy;

			bool canAttack = ((realEnemy != nullptr && !realEnemy->IsLara()) || creature->HurtByLara);

			if (item->HitStatus || laraAI.distance < FLAMETHROWER_AWARE_RANGE || TargetVisible(item, &laraAI))
			{
				if (!creature->Alerted)
					SoundEffect(SFX_TR3_AMERCAN_HOY, &item->Pose);

				AlertAllGuards(itemNumber);
			}

			switch (item->Animation.ActiveState)
			{
			case FLAMETHROWER_STATE_IDLE:
				creature->MaxTurn = 0;
				creature->Flags = 0;
				extraHeadRot.y = laraAI.angle;

				if (item->AIBits & GUARD)
				{
					extraHeadRot.y = AIGuard(creature);

					if (TestProbability(1.0f / 128))
						item->Animation.TargetState = FLAME_STATE_WAIT;

					break;
				}
				else if (item->AIBits & PATROL1)
				{
					item->Animation.TargetState = FLAMETHROWER_STATE_WALK_FORWARD;
				}
				else if (creature->Mood == MoodType::Escape)
				{
					item->Animation.TargetState = FLAMETHROWER_STATE_WALK_FORWARD;
				}
				else if (Targetable(item, &AI) && canAttack)
				{
					if (AI.distance < FLAMETHROWER_ATTACK_RANGE)
						item->Animation.TargetState = FLAMETHROWER_STATE_AIM_1;
					else
						item->Animation.TargetState = FLAMETHROWER_STATE_WALK_FORWARD;
				}
				else if (creature->Mood == MoodType::Bored && AI.ahead && TestProbability(1.0f / 128))
				{
					item->Animation.TargetState = FLAME_STATE_WAIT;
				}
				else if (creature->Mood == MoodType::Attack || TestProbability(1.0f / 128))
				{
					item->Animation.TargetState = FLAMETHROWER_STATE_WALK_FORWARD;
				}

				break;

			case 4:
				extraHeadRot.y = laraAI.angle;

				if (item->AIBits & GUARD)
				{
					extraHeadRot.y = AIGuard(creature);

					if (TestProbability(1.0f / 128))
						item->Animation.TargetState = FLAMETHROWER_STATE_IDLE;

					break;
				}
				else if ((Targetable(item, &AI) &&
					AI.distance < FLAMETHROWER_ATTACK_RANGE && canAttack ||
					creature->Mood != MoodType::Bored ||
					TestProbability(1.0f / 128)))
				{
					item->Animation.TargetState = FLAMETHROWER_STATE_IDLE;
				}

				break;

			case FLAMETHROWER_STATE_WALK_FORWARD:
				creature->MaxTurn = FLAMETHROWER_WALK_TURN_RATE_MAX;
				creature->Flags = 0;
				extraHeadRot.y = laraAI.angle;

				if (item->AIBits & GUARD)
					SetAnimation(item, FLAME_ANIM_IDLE);
				else if (item->AIBits & PATROL1)
					item->Animation.TargetState = FLAMETHROWER_STATE_WALK_FORWARD;
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = FLAMETHROWER_STATE_WALK_FORWARD;
				else if (Targetable(item, &AI) && canAttack)
				{
					if (AI.distance < FLAMETHROWER_IDLE_RANGE)
						item->Animation.TargetState = FLAMETHROWER_STATE_IDLE;
					else if (AI.distance < FLAMETHROWER_ATTACK_RANGE)
						item->Animation.TargetState = FLAMETHROWER_STATE_WALK_FORWARD_AIM;
				}
				else if (creature->Mood == MoodType::Bored && AI.ahead)
					item->Animation.TargetState = FLAMETHROWER_STATE_IDLE;
				else
					item->Animation.TargetState = FLAMETHROWER_STATE_WALK_FORWARD;

				break;

			case FLAMETHROWER_STATE_AIM_1:
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;

					if (Targetable(item, &AI) &&
						AI.distance < FLAMETHROWER_ATTACK_RANGE && canAttack)
					{
						item->Animation.TargetState = FLAMETHROWER_STATE_ATTACK_1;
					}
					else
						item->Animation.TargetState = FLAMETHROWER_STATE_IDLE;
				}

				break;

			case FLAMETHROWER_STATE_WALK_FORWARD_AIM:
				creature->Flags = 0;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;

					if (Targetable(item, &AI) &&
						AI.distance < FLAMETHROWER_ATTACK_RANGE && canAttack)
					{
						item->Animation.TargetState = FLAMETHROWER_STATE_WALK_FORWARD_ATTACK;
					}
					else
						item->Animation.TargetState = FLAMETHROWER_STATE_WALK_FORWARD;
				}

				break;

			case FLAMETHROWER_STATE_ATTACK_1:
				if (creature->Flags < 40)
					creature->Flags += (creature->Flags / 4) + 1;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;

					if (Targetable(item, &AI) &&
						AI.distance < FLAMETHROWER_ATTACK_RANGE && canAttack)
					{
						item->Animation.TargetState = FLAMETHROWER_STATE_ATTACK_1;
					}
					else
						item->Animation.TargetState = FLAMETHROWER_STATE_IDLE;
				}
				else
					item->Animation.TargetState = FLAMETHROWER_STATE_IDLE;

				if (creature->Flags < 40)
					ThrowFire(itemNumber, FlamethrowerBite.meshNum, FlamethrowerOffset, Vector3i(0, creature->Flags * 1.5f, 0));
				else
				{
					ThrowFire(itemNumber, FlamethrowerBite.meshNum, FlamethrowerOffset, Vector3i(0, (Random::GenerateInt() & 63) + 12, 0));
					if (realEnemy)
					{
						/*code*/
					}
				}

				SoundEffect(SFX_TR4_FLAME_EMITTER, &item->Pose);
				break;

			case FLAMETHROWER_STATE_WALK_FORWARD_ATTACK:
				if (creature->Flags < 40)
					creature->Flags += (creature->Flags / 4) + 1;

				if (AI.ahead)
				{
					extraTorsoRot.x = AI.xAngle;
					extraTorsoRot.y = AI.angle;

					if (Targetable(item, &AI) &&
						AI.distance < FLAMETHROWER_ATTACK_RANGE && canAttack)
					{
						item->Animation.TargetState = FLAMETHROWER_STATE_WALK_FORWARD_ATTACK;
					}
					else
						item->Animation.TargetState = FLAMETHROWER_STATE_WALK_FORWARD;
				}
				else
					item->Animation.TargetState = FLAMETHROWER_STATE_WALK_FORWARD;

				if (creature->Flags < 40)
					ThrowFire(itemNumber, FlamethrowerBite.meshNum, FlamethrowerOffset, Vector3i(0, creature->Flags * 1.5f, 0));
				else
				{
					ThrowFire(itemNumber, FlamethrowerBite.meshNum, FlamethrowerOffset, Vector3i(0, (GetRandomControl() & 63) + 12, 0));
					if (realEnemy)
					{
						/*code*/
					}
				}

				SoundEffect(SFX_TR4_FLAME_EMITTER, &item->Pose);
				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, extraTorsoRot.y);
		CreatureJoint(item, 1, extraTorsoRot.x);
		CreatureJoint(item, 2, extraHeadRot.y);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
