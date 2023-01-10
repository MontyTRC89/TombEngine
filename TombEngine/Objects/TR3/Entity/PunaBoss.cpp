#include "framework.h"
#include "Objects/TR3/Entity/PunaBoss.h"

#include "Game/control/box.h"
#include "Game/control/los.h"
#include "Game/effects/effects.h"
#include "Game/effects/item_fx.h"
#include "Game/effects/lightning.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/misc.h"
#include "Math/Math.h"
#include "Objects/Effects/Boss.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Boss;
using namespace TEN::Effects::Items;
using namespace TEN::Effects::Lightning;

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto PUNA_LIGHTNING_DAMAGE = 350;

	constexpr auto PUNA_ATTACK_RANGE = BLOCK(20);
	constexpr auto PUNA_ALERT_RANGE	 = BLOCK(2.5f);

	constexpr auto PUNA_TURN_RATE_MAX			 = ANGLE(3.0f);
	constexpr auto PUNA_CHAIR_TURN_RATE_MAX		 = ANGLE(7.0f);
	constexpr auto PUNA_HEAD_X_ANGLE_MAX		 = ANGLE(20.0f);
	constexpr auto PUNA_ADJUST_LIGHTNING_X_ANGLE = ANGLE(3.0f);

	constexpr auto PUNA_EXPLOSION_NUM_MAX	= 120;
	constexpr auto PUNA_HEAD_ATTACK_NUM_MAX = 4;
	constexpr auto PUNA_EFFECT_COLOR		= Vector4(0.0f, 0.75f, 0.75f, 1.0f);

	const auto PunaBossHeadBite = BiteInfo(Vector3::Zero, 8);
	const auto PunaBossHandBite = BiteInfo(Vector3::Zero, 14);

	enum PunaState
	{
		PUNA_STATE_IDLE = 0,
		PUNA_STATE_HEAD_ATTACK = 1,
		PUNA_STATE_HAND_ATTACK = 2,
		PUNA_STATE_DEATH = 3
	};

	enum PunaAnim
	{
		PUNA_ANIM_IDLE = 0,
		PUNA_ANIM_HEAD_ATTACK = 1,
		PUNA_ANIM_HAND_ATTACK = 2,
		PUNA_ANIM_DEATH = 3
	};

	enum class PunaAttackType
	{
		AwaitPlayer,
		DeathLaser,
		SummonLaser,
		Wait // Used while an active lizard is nearby.
	};

	void InitialisePuna(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);
		SetAnimation(&item, PUNA_ANIM_IDLE);
		CheckForRequiredObjects(item);

		// Save Puna's default angle. It will be used while waiting (i.e. an active lizard is nearby).
		// NOTE: Since Puna is oriented to face away from the player, add 180 degrees.
		item.SetFlagField(BOSSFlag_Rotation, item.Pose.Orientation.y + ANGLE(180.0f));
		item.SetFlagField(BOSSFlag_AttackType, (int)PunaAttackType::AwaitPlayer); // normal behaviour at start.
		item.SetFlagField(BOSSFlag_ShieldIsEnabled, 1); // activated at start.
		item.SetFlagField(BOSSFlag_AttackCount, 0);
		item.SetFlagField(BOSSFlag_DeathCount, 0);
		item.SetFlagField(BOSSFlag_ItemNumber, NO_ITEM);
		item.SetFlagField(BOSSFlag_ExplodeCount, 0);

		// If there is no lizard nearby, remove the lizard flag.
		if (!IsLizardActiveNearby(item, true))
			item.ClearFlags(BOSSFlag_Object, BOSS_Lizard);
	}

	void PunaControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;
		
		auto& item = g_Level.Items[itemNumber];
		auto& object = Objects[item.ObjectNumber];
		auto& creature = *GetCreatureInfo(&item);

		static auto targetPos = Vector3i::Zero;

		auto headOrient = EulerAngles::Zero;
		short headingAngle = 0;
		short prevYOrient = 0;

		bool hasTurned = false;
		bool isLizardActiveNearby = IsLizardActiveNearby(item);

		if (item.HitPoints <= 0)
		{
			if (item.Animation.ActiveState != PUNA_STATE_DEATH)
			{
				SetAnimation(&item, PUNA_ANIM_DEATH);
				SoundEffect(SFX_TR3_PUNA_BOSS_DEATH, &item.Pose);
				item.ItemFlags[BOSSFlag_DeathCount] = 1;
				creature.MaxTurn = 0;
			}

			int frameEnd = g_Level.Anims[object.animIndex + PUNA_ANIM_DEATH].frameEnd;
			if (item.Animation.FrameNumber >= frameEnd)
			{
				// Avoid having the object stop working.
				item.Animation.FrameNumber = frameEnd;
				item.MeshBits.ClearAll();

				if (item.GetFlagField(BOSSFlag_ExplodeCount) < PUNA_EXPLOSION_NUM_MAX)
					item.ItemFlags[BOSSFlag_ExplodeCount]++;

				if (item.GetFlagField(BOSSFlag_ExplodeCount) < PUNA_EXPLOSION_NUM_MAX)
					ExplodeBoss(itemNumber, item, 61, PUNA_EFFECT_COLOR); // Do explosion effect.

				return;
			}
			else
			{
				auto deathCount = item.GetFlagField(BOSSFlag_DeathCount);
				item.Pose.Orientation.z = (Random::GenerateInt() % deathCount) - (item.ItemFlags[BOSSFlag_DeathCount] >> 1);

				if (deathCount < 2048)
					item.ItemFlags[BOSSFlag_DeathCount] += 32;
			}
		}
		else
		{
			prevYOrient = item.Pose.Orientation.y;

			AI_INFO AI;
			CreatureAIInfo(&item, &AI);
			if (AI.ahead)
			{
				headOrient.x = AI.xAngle;
				headOrient.y = AI.angle;
			}

			if (item.TestFlagField(BOSSFlag_AttackType, (int)PunaAttackType::AwaitPlayer) && creature.Enemy != nullptr)
			{
				float distance = Vector3i::Distance(creature.Enemy->Pose.Position, item.Pose.Position);

				if (distance <= BLOCK(2.5f))
					item.SetFlagField(BOSSFlag_AttackType, (int)PunaAttackType::DeathLaser);

				// Rotate the object on puna boss chair.
				creature.JointRotation[0] += PUNA_CHAIR_TURN_RATE_MAX;
				return;
			}

			// Get target.
			if (item.TestFlagField(BOSSFlag_AttackType, (int)PunaAttackType::DeathLaser))
			{
				creature.Target = creature.Enemy->Pose.Position;
			}
			else if (item.TestFlags(BOSSFlag_Object, BOSS_Lizard) &&
				item.TestFlagField(BOSSFlag_AttackType, (int)PunaAttackType::SummonLaser) &&
				item.TestFlagField(BOSSFlag_ItemNumber, NO_ITEM) &&
				!item.TestFlagField(BOSSFlag_AttackType, (int)PunaAttackType::Wait) && isLizardActiveNearby)
			{
				// Get random lizard item number.
				item.SetFlagField(BOSSFlag_ItemNumber, (short)GetLizardItemNumber(item));
				creature.Target = GetLizardTargetPosition(item);
			}
			else if (item.TestFlags(BOSSFlag_Object, BOSS_Lizard) &&
				item.TestFlagField(BOSSFlag_AttackType, (int)PunaAttackType::Wait) &&
				!item.TestFlagField(BOSSFlag_ItemNumber, NO_ITEM))
			{
				// Rotate to idle position while player fights lizard.
				auto targetOrient = EulerAngles(item.Pose.Orientation.x, item.GetFlagField(BOSSFlag_Rotation), item.Pose.Orientation.z);
				item.Pose.Orientation.InterpolateConstant(targetOrient, ANGLE(3.0f));

				// Check if target is dead.
				auto& summonItem = g_Level.Items[item.GetFlagField(BOSSFlag_ItemNumber)];

				if (summonItem.HitPoints <= 0)
				{
					// Reset the attack type, attack count, itemNumber, and restart the sequence.
					item.SetFlagField(BOSSFlag_AttackType, (int)PunaAttackType::DeathLaser);
					item.SetFlagField(BOSSFlag_AttackCount, 0);
					item.SetFlagField(BOSSFlag_ItemNumber, NO_ITEM);
				}
			}

			if (item.HitStatus)
				SoundEffect(SFX_TR3_PUNA_BOSS_TAKE_HIT, &item.Pose);

			short headYOrient = GetPunaHeadOrientToTarget(item, creature.Target.ToVector3());
			headingAngle = CreatureTurn(&item, creature.MaxTurn);

			switch (item.Animation.ActiveState)
			{
			case PUNA_STATE_IDLE:
				creature.MaxTurn = PUNA_TURN_RATE_MAX;
				item.SetFlagField(BOSSFlag_ShieldIsEnabled, 1);

				if ((item.Animation.TargetState != PUNA_STATE_HAND_ATTACK && item.Animation.TargetState != PUNA_STATE_HEAD_ATTACK) &&
					AI.angle > ANGLE(-1.0f) && AI.angle < ANGLE(1.0f) &&
					creature.Enemy->HitPoints > 0 &&
					item.GetFlagField(BOSSFlag_AttackCount) < PUNA_HEAD_ATTACK_NUM_MAX &&
					!item.TestFlagField(BOSSFlag_AttackType, (int)PunaAttackType::SummonLaser) && !item.TestFlagField(BOSSFlag_AttackType, (int)PunaAttackType::Wait))
				{
					creature.MaxTurn = 0;
					targetPos = creature.Target;
					targetPos.y -= CLICK(2);
					item.SetFlagField(BOSSFlag_AttackType, (int)PunaAttackType::DeathLaser);

					if (Random::TestProbability(1 / 3.0f))
						item.Animation.TargetState = PUNA_STATE_HEAD_ATTACK;
					else
						item.Animation.TargetState = PUNA_STATE_HAND_ATTACK;

					if (item.TestFlags(BOSSFlag_Object, BOSS_Lizard) && isLizardActiveNearby)
						item.ItemFlags[BOSSFlag_AttackCount]++;
				}
				else if (item.ItemFlags[BOSSFlag_AttackCount] >= PUNA_HEAD_ATTACK_NUM_MAX &&
					creature.Enemy->HitPoints > 0 && 
					item.ItemFlags[BOSSFlag_AttackType] != (int)PunaAttackType::Wait)
				{
					item.SetFlagField(BOSSFlag_AttackType, (int)PunaAttackType::SummonLaser);

					if (!item.TestFlagField(BOSSFlag_ItemNumber, NO_ITEM))
					{
						if (headYOrient > ANGLE(-1.0f) && headYOrient < ANGLE(1.0f))
						{
							targetPos = creature.Target;
							targetPos.y -= CLICK(2);
							item.Animation.TargetState = PUNA_STATE_HAND_ATTACK;
						}
					}
				}

				break;

			case PUNA_STATE_HEAD_ATTACK:
				item.SetFlagField(BOSSFlag_ShieldIsEnabled, 0);
				creature.MaxTurn = 0;

				if (item.Animation.FrameNumber == GetFrameNumber(&item, 14))
					DoPunaLightning(item, targetPos.ToVector3(), PunaBossHeadBite, 10, false);

				break;

			case PUNA_STATE_HAND_ATTACK:
				item.SetFlagField(BOSSFlag_ShieldIsEnabled, 0);
				creature.MaxTurn = 0;

				if (item.Animation.FrameNumber == GetFrameNumber(&item, 30))
				{
					if (item.TestFlags(BOSSFlag_Object, BOSS_Lizard) &&
						item.TestFlagField(BOSSFlag_AttackType, (int)PunaAttackType::SummonLaser) &&
						!item.TestFlagField(BOSSFlag_ItemNumber, NO_ITEM) && isLizardActiveNearby)
					{
						DoPunaLightning(item, targetPos.ToVector3(), PunaBossHandBite, 5, true);
					}
					else
					{
						DoPunaLightning(item, targetPos.ToVector3(), PunaBossHandBite, 10, false);
					}
				}

				break;
			}
		}

		// Rotate chair.
		creature.JointRotation[0] += PUNA_CHAIR_TURN_RATE_MAX;

		CreatureJoint(&item, 1, headOrient.y);
		CreatureJoint(&item, 2, headOrient.x, PUNA_HEAD_X_ANGLE_MAX);
		CreatureAnimation(itemNumber, headingAngle, 0);

		// Emit sound while chair is rotating.
		if (prevYOrient != item.Pose.Orientation.y && !hasTurned)
		{
			hasTurned = true;
			SoundEffect(SFX_TR3_PUNA_BOSS_TURN_CHAIR, &item.Pose);
		}
		else if (prevYOrient == item.Pose.Orientation.y)
		{
			hasTurned = false;
			StopSoundEffect(SFX_TR3_PUNA_BOSS_CHAIR_2);
			StopSoundEffect(SFX_TR3_PUNA_BOSS_TURN_CHAIR);
		}
	}

	void DoPunaLightning(ItemInfo& item, const Vector3& pos, const BiteInfo& bite, int intensity, bool isSummon)
	{
		const auto& creature = *GetCreatureInfo(&item);

		auto origin = GameVector(GetJointPosition(&item, bite.meshNum, bite.Position), item.RoomNumber);

		auto direction = pos - origin.ToVector3();
		direction.Normalize();
		auto target = GameVector(origin.ToVector3() + (direction * PUNA_ATTACK_RANGE), creature.Enemy->RoomNumber);

		if (isSummon)
		{
			TriggerLightning((Vector3i*)&origin, (Vector3i*)&target, intensity, 0, 255, 0, 30, LI_SPLINE | LI_THINOUT, 50, 10);
			TriggerDynamicLight(origin.x, origin.y, origin.z, 20, 0, 255, 0);
			item.SetFlagField(BOSSFlag_AttackType, (int)PunaAttackType::Wait);

			SpawnLizard(item);
		}
		else
		{
			auto hitPos = Vector3i::Zero;
			MESH_INFO* mesh = nullptr;
			TriggerLightning((Vector3i*)&origin, (Vector3i*)&target, intensity, 0, 255, 255, 30, LI_SPLINE | LI_THINOUT, 50, 10);
			TriggerDynamicLight(origin.x, origin.y, origin.z, 20, 0, 255, 255);

			if (ObjectOnLOS2(&origin, &target, &hitPos, &mesh, ID_LARA) == GetLaraInfo(creature.Enemy)->ItemNumber)
			{
				if (creature.Enemy->HitPoints <= PUNA_LIGHTNING_DAMAGE)
				{
					ItemElectricBurn(creature.Enemy);
					DoDamage(creature.Enemy, PUNA_LIGHTNING_DAMAGE);
				}
				else
				{
					DoDamage(creature.Enemy, PUNA_LIGHTNING_DAMAGE);
				}
			}
		}
	}

	short GetPunaHeadOrientToTarget(ItemInfo& item, const Vector3& target)
	{
		if (!item.TestFlags(BOSSFlag_Object, BOSS_Lizard))
			return NO_ITEM;

		auto pos = GetJointPosition(&item, PunaBossHeadBite.meshNum).ToVector3();
		auto orient = Geometry::GetOrientToPoint(pos, target);
		return (orient.y - item.Pose.Orientation.y);
	}

	std::vector<int> GetLizardEntityList(const ItemInfo& item)
	{
		auto entityList = std::vector<int>{};

		for (auto& currentEntity : g_Level.Items)
		{
			if (currentEntity.ObjectNumber == ID_LIZARD &&
				currentEntity.RoomNumber == item.RoomNumber &&
				currentEntity.HitPoints > 0 &&
				currentEntity.Status == ITEM_INVISIBLE &&
				!(currentEntity.Flags & IFLAG_KILLED))
			{
				entityList.push_back(currentEntity.Index);
			}
		}

		return entityList;
	}

	Vector3 GetLizardTargetPosition(ItemInfo& item)
	{
		if (!item.TestFlagField(BOSSFlag_ItemNumber, NO_ITEM))
		{
			const auto& targetEntity = g_Level.Items[item.GetFlagField(BOSSFlag_ItemNumber)];
			return targetEntity.Pose.Position.ToVector3();
		}

		// Failsafe.
		const auto& creature = *GetCreatureInfo(&item);
		return creature.Target.ToVector3();
	}

	int GetLizardItemNumber(const ItemInfo& item)
	{
		if (!item.TestFlags(BOSSFlag_Object, BOSS_Lizard))
			return NO_ITEM;

		auto lizardList = GetLizardEntityList(item);
		if (lizardList.empty())
			return NO_ITEM;

		if (lizardList.size() == 1)
			return lizardList[0];
		else
			return lizardList[Random::GenerateInt(0, lizardList.size() - 1)];
	}

	bool IsLizardActiveNearby(const ItemInfo& item, bool isInitializing)
	{
		for (auto& currentEntity : g_Level.Items)
		{
			// Check if the entity is a lizard.
			if (currentEntity.ObjectNumber != ID_LIZARD)
				continue;

			// Check if the entity is in the same room as Puna.
			if (currentEntity.RoomNumber != item.RoomNumber)
				continue;

			// If the enity is currently initializing, return early.
			if (isInitializing)
				return true;

			// Check status of entity.
			if (currentEntity.HitPoints > 0 &&
				currentEntity.Status == ITEM_INVISIBLE &&
				!(currentEntity.Flags & IFLAG_KILLED))
			{
				return true;
			}
		}

		return false;
	}

	void SpawnLizard(ItemInfo& item)
	{
		if (!item.TestFlagField(BOSSFlag_ItemNumber, NO_ITEM))
		{
			auto itemNumber = item.GetFlagField(BOSSFlag_ItemNumber);
			auto& item = g_Level.Items[itemNumber];

			for (int i = 0; i < 20; i++)
				SpawnSummonSmoke(item.Pose.Position.ToVector3());

			AddActiveItem(itemNumber);
			item.ItemFlags[0] = 1; // Flag 1 = spawned lizard.
		}
	}

	void SpawnSummonSmoke(const Vector3& pos)
	{
		auto& smoke = *GetFreeParticle();

		smoke.sR = 16;
		smoke.sG = 64;
		smoke.sB = 0;
		smoke.dR = 8;
		smoke.dG = 32;
		smoke.dB = 0;
		smoke.colFadeSpeed = 16 + (GetRandomControl() & 7);
		smoke.fadeToBlack = 64;
		smoke.sLife = smoke.life = (GetRandomControl() & 15) + 96;

		smoke.blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		smoke.extras = 0;
		smoke.dynamic = -1;

		smoke.x = pos.x + ((GetRandomControl() & 127) - 64);
		smoke.y = pos.y - (GetRandomControl() & 31);
		smoke.z = pos.z + ((GetRandomControl() & 127) - 64);
		smoke.xVel = ((GetRandomControl() & 255) - 128);
		smoke.yVel = -(GetRandomControl() & 15) - 16;
		smoke.zVel = ((GetRandomControl() & 255) - 128);
		smoke.friction = 0;

		if (Random::TestProbability(1 / 2.0f))
		{
			smoke.rotAng = GetRandomControl() & 4095;
			smoke.flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_WIND;

			if (GetRandomControl() & 1)
				smoke.rotAdd = -(GetRandomControl() & 7) - 4;
			else
				smoke.rotAdd = (GetRandomControl() & 7) + 4;
		}
		else
		{
			smoke.flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_WIND;
		}

		smoke.spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex;
		smoke.scalar = 3;
		smoke.gravity = -(GetRandomControl() & 7) - 8;
		smoke.maxYvel = -(GetRandomControl() & 7) - 4;
		int size = (GetRandomControl() & 128) + 256;
		smoke.size = smoke.sSize = size >> 1;
		smoke.dSize = size;
		smoke.on = true;
	}
}
