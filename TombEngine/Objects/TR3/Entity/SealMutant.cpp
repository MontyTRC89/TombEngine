#include "framework.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/Effects/enemy_missile.h"

namespace TEN::Entities::Creatures::TR3
{
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
		SEAL_MUTANT_ANIM_WALKING = 2,
		SEAL_MUTANT_ANIM_WALK_TO_IDLE = 3,
		SEAL_MUTANT_ANIM_ATTACKING = 4,
		SEAL_MUTANT_ANIM_DEATH = 5,
		SEAL_MUTANT_ANIM_TRAP = 6
	};

	enum SealMutantItemFlags
	{
		IF_SEAL_MUTANT_FLAMER_TIMER = 0
	};

	enum SealMutantOcb
	{
		OCB_NORMAL_BEHAVIOUR = 0,
		OCB_TRAP_MODE = 1 // Enable the seal mutant that wakes up, spits poison then dies.
	};

	const auto SealMutantGasBite = CreatureBiteInfo(0.0f, 48.0f, 140.0f, 10);
	const auto SealMutantTargetList = { ID_FLAMETHROWER_BADDY, ID_WORKER_FLAMETHROWER, ID_LARA };
	constexpr auto SealMutantGasSpeedMultiplier = 5; // Speed increase for the gas to touch lara at more than 1.5 blocks.
	constexpr auto SealMutantGasCrouchGravity = 32; // Avoid the seal mutant to not hit lara if she is crouching !

	static void ThrowSealMutantGas(short itemNumber, ItemInfo* enemy, int speed)
	{
		constexpr auto THROW_COUNT = 2;
		int gravity = (enemy != nullptr && enemy->IsLaraCrouching()) ? SealMutantGasCrouchGravity : 0;
		for (int i = 0; i < THROW_COUNT; i++)
			ThrowPoison(itemNumber, SealMutantGasBite, Vector3i(0, gravity, speed * SealMutantGasSpeedMultiplier), Vector3i((GetRandomControl() & 0x3F) + 128, (GetRandomControl() & 0x3F) + 128, 32), Vector3i((GetRandomControl() & 0xF) + 32, (GetRandomControl() & 0xF) + 32, 0));
		ThrowPoison(itemNumber, SealMutantGasBite, Vector3i(0, gravity, speed * SealMutantGasSpeedMultiplier), Vector3i((GetRandomControl() & 0x3F) + 128, (GetRandomControl() & 0x3F) + 128, 32), Vector3i((GetRandomControl() & 0xF) + 32, (GetRandomControl() & 0xF) + 32, 0));
	}

	void SealMutantControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		int speed = 0;
		auto* item = &g_Level.Items[itemNumber];
		if (item->TestOcb(OCB_TRAP_MODE))
		{
			if (item->Animation.ActiveState != SEAL_MUTANT_STATE_TRAP)
			{
				SetAnimation(item, SEAL_MUTANT_ANIM_TRAP);
			}
			else
			{
				auto& deathAnim = GetAnimData(item->Animation.AnimNumber);
				if ((item->Animation.FrameNumber >= (deathAnim.frameBase + 1)) && (item->Animation.FrameNumber <= (deathAnim.frameEnd - 8)))
				{
					speed = item->Animation.FrameNumber - (deathAnim.frameBase + 1);
					if (speed > 24)
					{
						speed = (deathAnim.frameEnd - item->Animation.FrameNumber) - 8;
						if (speed <= 0)
							speed = 1;
						if (speed > 24)
							speed = (GetRandomControl() & 0xF) + 8;
						ThrowSealMutantGas(itemNumber, nullptr, speed);
					}
				}
			}
			CreatureAnimation(itemNumber, 0, 0);
			return;
		}
		auto& creature = *GetCreatureInfo(item);
		AI_INFO ai;
		ItemInfo* target = NULL;
		CreatureBiteInfo bonePosition;
		Vector3i boneEffectPosition;
		short headingAngle = 0;
		short headY = 0;
		short headX = 0;
		short torsoZ = 0;
		short torsoX = 0;

		if (item->GetFlagField(IF_SEAL_MUTANT_FLAMER_TIMER) > 80)
			item->HitPoints = 0;

		if (item->HitPoints <= 0)
		{
			auto& beforeSetAnim = GetAnimData(item->Animation.AnimNumber);
			if (item->Animation.ActiveState != SEAL_MUTANT_STATE_DEATH)
			{
				SetAnimation(item, SEAL_MUTANT_ANIM_DEATH);
			}
			else if (item->GetFlagField(IF_SEAL_MUTANT_FLAMER_TIMER) > 80)
			{
				for (int boneIndex = 9; boneIndex < 17; boneIndex++)
				{
					bonePosition.Position.x = 0;
					bonePosition.Position.y = 0;
					bonePosition.Position.z = 0;
					bonePosition.BoneID = boneIndex;
					boneEffectPosition = GetJointPosition(*item, bonePosition);
					TriggerFireFlame(boneEffectPosition.x, boneEffectPosition.y, boneEffectPosition.z, FlameType::Medium);
				}
				auto& animData = GetAnimData(item->Animation.AnimNumber);
				int c = item->Animation.FrameNumber - animData.frameBase;
				if (c > 16)
				{
					c = item->Animation.FrameNumber - animData.frameEnd;
					if (c > 16)
						c = 16;
				}
				Color color;
				color.z = GetRandomControl();
				color.x = (c * (255 - (((byte)color.z >> 4) & 0x1F))) >> 4;
				color.y = (c * (192 - (((byte)color.z >> 6) & 0x3F))) >> 4;
				color.z = (c * ((byte)color.z & 0x3F)) >> 4;
				TriggerDynamicLight(item->Pose.Position.ToVector3(), color, 12.0f);
			}
			else if ((item->Animation.FrameNumber >= (beforeSetAnim.frameBase + 1)) && (item->Animation.FrameNumber <= (beforeSetAnim.frameEnd - 8)))
			{
				speed = item->Animation.FrameNumber - (beforeSetAnim.frameBase + 1);
				if (speed > 24)
				{
					speed = (beforeSetAnim.frameEnd - item->Animation.FrameNumber) - 8;
					if (speed <= 0)
						speed = 1;
					if (speed > 24)
						speed = (GetRandomControl() & 0xF) + 8;
					ThrowSealMutantGas(itemNumber, creature.Enemy, speed);
				}
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(&creature);
			else
				TargetNearestEntity(item, &creature, SealMutantTargetList, false);
			
			CreatureAIInfo(item, &ai);
			GetCreatureMood(item, &ai, ai.zoneNumber == ai.enemyZone);
			if (creature.Enemy && creature.Enemy->ObjectNumber == ID_LARA && GetLaraInfo(creature.Enemy)->Status.Poison >= 256)
				creature.Mood = MoodType::Escape;
			CreatureMood(item, &ai, ai.zoneNumber == ai.enemyZone);
			headingAngle = CreatureTurn(item, creature.MaxTurn);
			
			target = creature.Enemy;
			creature.Enemy = LaraItem; // TODO: replace global LaraItem reference !
			if (ai.distance < 0x100000 || item->HitStatus || TargetVisible(item, &ai))
				AlertAllGuards(itemNumber);
			creature.Enemy = target;

			switch (item->Animation.ActiveState)
			{
			case SEAL_MUTANT_STATE_IDLE:
				creature.MaxTurn = 0;
				creature.Flags = 0;
				headY = ai.angle;
				headX = -ai.xAngle;
				torsoX = 0;
				torsoZ = 0;

				if (item->AIBits & GUARD)
				{
					headY = AIGuard(&creature);
					headX = 0;
					item->Animation.TargetState = SEAL_MUTANT_STATE_IDLE;
				}
				else if (item->AIBits & PATROL1)
				{
					headY = 0;
					headX = 0;
					item->Animation.TargetState = SEAL_MUTANT_STATE_WALK;
				}
				else if (creature.Mood == MoodType::Escape)
					item->Animation.TargetState = SEAL_MUTANT_STATE_WALK;
				else if (Targetable(item, &ai) && ai.distance < 0x400000)
					item->Animation.TargetState = SEAL_MUTANT_STATE_ATTACK;
				else if (item->Animation.RequiredState != NO_VALUE)
					item->Animation.TargetState = item->Animation.RequiredState;
				else
					item->Animation.TargetState = SEAL_MUTANT_STATE_WALK;

				break;
			case SEAL_MUTANT_STATE_WALK:
				creature.MaxTurn = 546;
				if (ai.ahead)
				{
					headY = ai.angle;
					headX = -ai.xAngle;
				}

				if (item->AIBits & PATROL1)
				{
					headY = 0;
					item->Animation.TargetState = SEAL_MUTANT_STATE_WALK;
				}
				else if (Targetable(item, &ai) && ai.distance < 0x400000)
					item->Animation.TargetState = SEAL_MUTANT_STATE_IDLE;

				break;
			case SEAL_MUTANT_STATE_ATTACK:
				if (ai.ahead)
				{
					headY = ai.angle;
					headX = -ai.xAngle;
					torsoZ = ai.angle / 2;
					torsoX = -ai.xAngle / 2;
				}

				auto& animData = GetAnimData(item->Animation.AnimNumber);
				if ((item->Animation.FrameNumber >= (animData.frameBase + 35)) && (item->Animation.FrameNumber <= (animData.frameBase + 58)))
				{
					if (creature.Flags < 24)
						creature.Flags += 3;
					speed = 0;
					if (creature.Flags < 24)
						speed = creature.Flags;
					else
						speed = (GetRandomControl() & 0xF) + 8;
					ThrowSealMutantGas(itemNumber, creature.Enemy, speed);
					if (creature.Enemy && creature.Enemy->ObjectNumber != ID_LARA)
						creature.Enemy->HitStatus = true;
				}

				break;
			}
		}

		CreatureJoint(item, 0, torsoZ);
		CreatureJoint(item, 1, torsoX);
		CreatureJoint(item, 2, headY);
		CreatureAnimation(itemNumber, headingAngle, 0);
	}
}