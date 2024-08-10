#include "framework.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/Effects/enemy_missile.h"

using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR3
{
	const auto SealMutantGasBite			   = CreatureBiteInfo(Vector3(0.0f, 48.0f, 140.0f), 10);
	const auto SealMutantAttackTargetObjectIds = { ID_LARA, ID_FLAMETHROWER_BADDY, ID_WORKER_FLAMETHROWER };

	enum SealMutantState
	{
		SEAL_MUTANT_STATE_IDLE = 0,
		SEAL_MUTANT_STATE_WALK = 1,
		SEAL_MUTANT_STATE_ATTACK = 2,
		SEAL_MUTANT_STATE_DEATH = 3,
		SEAL_MUTANT_STATE_TRAP = 4
	};

	enum SealMutantAnim
	{
		SEAL_MUTANT_ANIM_IDLE = 0,
		SEAL_MUTANT_ANIM_IDLE_TO_WALK = 1,
		SEAL_MUTANT_ANIM_WALK = 2,
		SEAL_MUTANT_ANIM_WALK_TO_IDLE = 3,
		SEAL_MUTANT_ANIM_ATTACK = 4,
		SEAL_MUTANT_ANIM_DEATH = 5,
		SEAL_MUTANT_ANIM_TRAP = 6
	};

	enum SealMutantItemFlags
	{
		IF_SEAL_MUTANT_FLAME_TIMER = 0
	};

	enum SealMutantOcb
	{
		OCB_NORMAL_BEHAVIOUR = 0,
		OCB_TRAP = 1
	};

	static void ThrowSealMutantGas(const ItemInfo& item, const ItemInfo* enemy, float vel)
	{
		constexpr auto GAS_COUNT			 = 3;
		constexpr auto VEL_MULT				 = 5.0f;
		constexpr auto PLAYER_CROUCH_GRAVITY = 32.0f;

		float gravity = 0.0f;
		if (enemy != nullptr && enemy->IsLara())
		{
			const auto& player = GetLaraInfo(*enemy);
			player.Control.IsLow;
		}
		
		auto velVector = Vector3(0.0f, gravity, vel * VEL_MULT);
		auto startColor = Color(Random::GenerateFloat(0.25f, 0.5f), Random::GenerateFloat(0.25f, 0.5f), 0.1f);
		auto endColor = Color(Random::GenerateFloat(0.05f, 0.1f), Random::GenerateFloat(0.05f, 0.1f), 0.0f);

		for (int i = 0; i < GAS_COUNT; i++)
			ThrowPoison(item, SealMutantGasBite, velVector, startColor, endColor);
	}

	void ControlSealMutant(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto& item = g_Level.Items[itemNumber];
		auto& creature = *GetCreatureInfo(&item);

		short headingAngle = 0;
		auto headOrient = EulerAngles::Identity;
		auto torsoOrient = EulerAngles::Identity;

		int speed = 0;

		if (item.TestOcb(OCB_TRAP))
		{
			if (item.Animation.ActiveState != SEAL_MUTANT_STATE_TRAP)
			{
				SetAnimation(item, SEAL_MUTANT_ANIM_TRAP);
			}
			else
			{
				const auto& deathAnim = GetAnimData(item.Animation.AnimNumber);
				if ((item.Animation.FrameNumber >= (deathAnim.frameBase + 1)) && (item.Animation.FrameNumber <= (deathAnim.frameEnd - 8)))
				{
					speed = item.Animation.FrameNumber - (deathAnim.frameBase + 1);
					if (speed > 24)
					{
						speed = (deathAnim.frameEnd - item.Animation.FrameNumber) - 8;
						if (speed <= 0)
							speed = 1;

						if (speed > 24)
							speed = (GetRandomControl() & 0xF) + 8;

						ThrowSealMutantGas(item, nullptr, speed);
					}
				}
			}

			CreatureAnimation(itemNumber, 0, 0);
			return;
		}


		AI_INFO ai;
		ItemInfo* target = nullptr;
		CreatureBiteInfo bonePos;
		Vector3i boneEffectPos;

		if (item.GetFlagField(IF_SEAL_MUTANT_FLAME_TIMER) > 80)
			item.HitPoints = 0;

		if (item.HitPoints <= 0)
		{
			const auto& prevAnim = GetAnimData(item.Animation.AnimNumber);
			if (item.Animation.ActiveState != SEAL_MUTANT_STATE_DEATH)
			{
				SetAnimation(item, SEAL_MUTANT_ANIM_DEATH);
			}
			else if (item.GetFlagField(IF_SEAL_MUTANT_FLAME_TIMER) > 80)
			{
				for (int boneID = 9; boneID < 17; boneID++)
				{
					bonePos.Position = Vector3::Zero;
					bonePos.BoneID = boneID;
					boneEffectPos = GetJointPosition(item, bonePos);
					TriggerFireFlame(boneEffectPos.x, boneEffectPos.y, boneEffectPos.z, FlameType::Medium);
				}

				const auto& animData = GetAnimData(item.Animation.AnimNumber);
				int c = item.Animation.FrameNumber - animData.frameBase;
				if (c > 16)
				{
					c = item.Animation.FrameNumber - animData.frameEnd;
					if (c > 16)
						c = 16;
				}

				Color color;
				color.z = GetRandomControl();
				color.x = (c * (255 - (((byte)color.z >> 4) & 0x1F))) >> 4;
				color.y = (c * (192 - (((byte)color.z >> 6) & 0x3F))) >> 4;
				color.z = (c * ((byte)color.z & 0x3F)) >> 4;
				TriggerDynamicLight(item.Pose.Position.ToVector3(), color, 12.0f);
			}
			else if ((item.Animation.FrameNumber >= (prevAnim.frameBase + 1)) && (item.Animation.FrameNumber <= (prevAnim.frameEnd - 8)))
			{
				speed = item.Animation.FrameNumber - (prevAnim.frameBase + 1);
				if (speed > 24)
				{
					speed = (prevAnim.frameEnd - item.Animation.FrameNumber) - 8;
					if (speed <= 0)
						speed = 1;

					if (speed > 24)
						speed = (GetRandomControl() & 0xF) + 8;

					ThrowSealMutantGas(item, creature.Enemy, speed);
				}
			}
		}
		else
		{
			if (item.AIBits)
			{
				GetAITarget(&creature);
			}
			else
			{
				TargetNearestEntity(&item, &creature, SealMutantAttackTargetObjectIds, false);
			}
			
			CreatureAIInfo(&item, &ai);
			GetCreatureMood(&item, &ai, ai.zoneNumber == ai.enemyZone);
			if (creature.Enemy != nullptr && creature.Enemy->ObjectNumber == ID_LARA)
			{
				const auto& player = GetLaraInfo(*creature.Enemy);
				if (player.Status.Poison >= (LARA_POISON_MAX * 2))
					creature.Mood = MoodType::Escape;
			}

			CreatureMood(&item, &ai, ai.zoneNumber == ai.enemyZone);
			headingAngle = CreatureTurn(&item, creature.MaxTurn);
			
			target = creature.Enemy;
			creature.Enemy = LaraItem;
			if (ai.distance < SQUARE(BLOCK(1)) || item.HitStatus || TargetVisible(&item, &ai))
				AlertAllGuards(itemNumber);

			creature.Enemy = target;

			switch (item.Animation.ActiveState)
			{
			case SEAL_MUTANT_STATE_IDLE:
				creature.MaxTurn = 0;
				creature.Flags = 0;
				headOrient.x = -ai.xAngle;
				headOrient.y = ai.angle;
				torsoOrient.x = 0;
				torsoOrient.z = 0;

				if (item.AIBits & GUARD)
				{
					item.Animation.TargetState = SEAL_MUTANT_STATE_IDLE;
					headOrient.x = 0;
					headOrient.y = AIGuard(&creature);
				}
				else if (item.AIBits & PATROL1)
				{
					item.Animation.TargetState = SEAL_MUTANT_STATE_WALK;
					headOrient.x = 0;
					headOrient.y = 0;
				}
				else if (creature.Mood == MoodType::Escape)
				{
					item.Animation.TargetState = SEAL_MUTANT_STATE_WALK;
				}
				else if (Targetable(&item, &ai) && ai.distance < 0x400000)
				{
					item.Animation.TargetState = SEAL_MUTANT_STATE_ATTACK;
				}
				else if (item.Animation.RequiredState != NO_VALUE)
				{
					item.Animation.TargetState = item.Animation.RequiredState;
				}
				else
				{
					item.Animation.TargetState = SEAL_MUTANT_STATE_WALK;
				}

				break;

			case SEAL_MUTANT_STATE_WALK:
				creature.MaxTurn = ANGLE(3.0f);

				if (ai.ahead)
				{
					headOrient.x = -ai.xAngle;
					headOrient.y = ai.angle;
				}

				if (item.AIBits & PATROL1)
				{
					item.Animation.TargetState = SEAL_MUTANT_STATE_WALK;
					headOrient.y = 0;
				}
				else if (Targetable(&item, &ai) && ai.distance < SQUARE(BLOCK(4)))
				{
					item.Animation.TargetState = SEAL_MUTANT_STATE_IDLE;
				}

				break;

			case SEAL_MUTANT_STATE_ATTACK:
				if (ai.ahead)
				{
					headOrient.x = -ai.xAngle;
					headOrient.y = ai.angle;
					torsoOrient.x = -ai.xAngle / 2;
					torsoOrient.z = ai.angle / 2;
				}

				const auto& anim = GetAnimData(item.Animation.AnimNumber);
				if ((item.Animation.FrameNumber >= (anim.frameBase + 35)) && (item.Animation.FrameNumber <= (anim.frameBase + 58)))
				{
					if (creature.Flags < 24)
						creature.Flags += 3;

					speed = 0;
					if (creature.Flags < 24)
					{
						speed = creature.Flags;
					}
					else
					{
						speed = (GetRandomControl() & 0xF) + 8;
					}

					ThrowSealMutantGas(item, creature.Enemy, speed);
					if (creature.Enemy && creature.Enemy->ObjectNumber != ID_LARA)
						creature.Enemy->HitStatus = true;
				}

				break;
			}
		}

		CreatureJoint(&item, 0, torsoOrient.z);
		CreatureJoint(&item, 1, torsoOrient.x);
		CreatureJoint(&item, 2, headOrient.y);
		CreatureAnimation(itemNumber, headingAngle, 0);
	}
}
