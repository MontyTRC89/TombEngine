#include "framework.h"
#include "Objects/TR2/Entity/tr2_spear_guardian.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR2
{
	constexpr auto SPEAR_GUARDIAN_WALK_ANGLE_RATE_MAX = ANGLE(3.0f);
	constexpr auto SPEAR_GUARDIAN_RUN_ANGLE_RATE_MAX = ANGLE(5.0f);

	const auto SpearBiteLeft  = BiteInfo(Vector3(0.0f, 0.0f, 920.0f), 11);
	const auto SpearBiteRight = BiteInfo(Vector3(0.0f, 0.0f, 920.0f), 18);

	constexpr auto SPEAR_GUARDIAN_SLASH_RANGE = SQUARE(SECTOR(1));
	constexpr auto SPEAR_GUARDIAN_DOUBLE_ATTACK_RANGE = SQUARE(SECTOR(1));
	constexpr auto SPEAR_GUARDIAN_DOUBLE_ATTACK_WALK_RANGE = SQUARE(SECTOR(1.5f));
	constexpr auto SPEAR_GUARDIAN_RUN_RANGE = SQUARE(SECTOR(3));
	constexpr auto SPEAR_GUARDIAN_ATTACK_RANGE = SQUARE(SECTOR(2));

	constexpr auto SPEAR_GUARDIAN_BASIC_DAMAGE = 75;
	constexpr auto SPEAR_GUARDIAN_POWERFUL_DAMAGE = 120;
	constexpr auto SPEAR_GUARDIAN_SWAPMESH_TIME = 3;

	enum SpearGuardianState
	{
		// No state 0.
		SPEAR_GUARDIAN_STATE_STOP = 1,
		// He wait and will attack in horizontal with this spear if target is in range he will slash, else return to idle or walk or wait if target still in range.
		SPEAR_GUARDIAN_STATE_SLASH_IDLE = 2,
		SPEAR_GUARDIAN_STATE_WALK = 3,
		SPEAR_GUARDIAN_STATE_RUN = 4,
		SPEAR_GUARDIAN_STATE_DOUBLE_ATTACK_FRONT_AIM = 5,
		SPEAR_GUARDIAN_STATE_DOUBLE_ATTACK_FRONT = 6, // Deal damage here.
		SPEAR_GUARDIAN_STATE_WALK_DOUBLE_ATTACK_FRONT_AIM = 7,
		SPEAR_GUARDIAN_STATE_WALK_DOUBLE_ATTACK_FRONT = 8,
		SPEAR_GUARDIAN_STATE_WALK_ATTACK_LEFT_SPEAR_AIM = 9,
		SPEAR_GUARDIAN_STATE_WALK_ATTACK_LEFT_SPEAR = 10,
		SPEAR_GUARDIAN_STATE_WALK_ATTACK_RIGHT_SPEAR_AIM = 11,
		SPEAR_GUARDIAN_STATE_WALK_ATTACK_RIGHT_SPEAR = 12,
		// Prepare the horizontal slash.
		SPEAR_GUARDIAN_STATE_SLASH_AIM = 13,
		SPEAR_GUARDIAN_STATE_SLASH = 14,
		SPEAR_GUARDIAN_STATE_RUN_DOUBLE_ATTACK_FRONT_AIM = 15,
		SPEAR_GUARDIAN_STATE_RUN_DOUBLE_ATTACK_FRONT = 16,
		SPEAR_GUARDIAN_STATE_DEATH = 17, // Artificial state to turn him back to stone, it's not used in this animation sets !
		SPEAR_GUARDIAN_STATE_AWAKE = 18, // Turn from stone to normal.
		SPEAR_GUARDIAN_STATE_KILL_LARA = 19 // Killer move. Need lara extra anims enum !
	};

	// TODO: Found all name, also found some which is more appropriate.
	enum SpearGuardianAnim
	{
		SPEAR_GUARDIAN_ANIM_DOUBLE_ATTACK_FRONT_CANCEL = 0,
		SPEAR_GUARDIAN_ANIM_DOUBLE_ATTACK_FRONT_PREPARE = 1,
		SPEAR_GUARDIAN_ANIM_WALK_DOUBLE_ATTACK_FRONT_CANCEL = 2,
		SPEAR_GUARDIAN_ANIM_WALK_DOUBLE_ATTACK_FRONT_PREPARE = 3,
		SPEAR_GUARDIAN_ANIM_WALK_LEFT_SPEAR_ATTACK_CANCEL = 4,
		SPEAR_GUARDIAN_ANIM_WALK_LEFT_SPEAR_ATTACK_PREPARE = 5,
		SPEAR_GUARDIAN_ANIM_WALK_RIGHT_SPEAR_ATTACK_CANCEL = 6,
		SPEAR_GUARDIAN_ANIM_WALK_RIGHT_SPEAR_ATTACK_PREPARE = 7,
		SPEAR_GUARDIAN_ANIM_SLASH_PREPARE = 8,
		SPEAR_GUARDIAN_ANIM_SLASH_CANCEL = 9,
		SPEAR_GUARDIAN_ANIM_WALK_DOUBLE_ATTACK_FRONT_TO_IDLE_FAST = 10,
		SPEAR_GUARDIAN_ANIM_WALK_DOUBLE_ATTACK_FRONT = 11, // Deal damage.
		SPEAR_GUARDIAN_ANIM_UNKNOWN = 12,

		SPEAR_GUARDIAN_ANIM_WALK_DOUBLE_ATTACK_FRONT_TO_SLASH_PREPARE = 15,

		SPEAR_GUARDIAN_ANIM_SLASH = 24,

		SPEAR_GUARDIAN_ANIM_RUN_TO_WALK = 27,
		SPEAR_GUARDIAN_ANIM_RUN = 28,
		SPEAR_GUARDIAN_ANIM_STAND_TO_SLASH_PREPARE = 29,
		SPEAR_GUARDIAN_ANIM_STAND_TO_WALK = 30,
		SPEAR_GUARDIAN_ANIM_SLASH_PREPARE_TO_STAND = 31,
		SPEAR_GUARDIAN_ANIM_SLASH_PREPARE_TO_WALK = 32,
		SPEAR_GUARDIAN_ANIM_STAND = 33,
		SPEAR_GUARDIAN_ANIM_SLASH_IDLE = 34,
		SPEAR_GUARDIAN_ANIM_WALK = 35,
		SPEAR_GUARDIAN_ANIM_WALK_TO_RUN = 36,
		SPEAR_GUARDIAN_ANIM_RUN_TO_STAND = 37,
		SPEAR_GUARDIAN_ANIM_WALK_TO_STAND = 38,
		SPEAR_GUARDIAN_ANIM_RUN_TO_IDLE = 39,
		SPEAR_GUARDIAN_ANIM_RUN_TO_SLASH_IDLE = 40,
		SPEAR_GUARDIAN_ANIM_RUN_TO_DOUBLE_ATTACK_FRONT = 41,
		SPEAR_GUARDIAN_ANIM_RUN_DOUBLE_ATTACK_FRONT = 42,

		SPEAR_GUARDIAN_ANIM_WALK_TO_SLASH_IDLE = 47,
		SPEAR_GUARDIAN_ANIM_AWAKE = 48,
		SPEAR_GUARDIAN_ANIM_KILL_LARA = 49,
	};

	static void XianDamage(ItemInfo* item, CreatureInfo* creature, int damage)
	{
		if (!(creature->Flags & 1) && item->TouchBits.Test(SpearBiteRight.meshNum))
		{
			DoDamage(creature->Enemy, damage);
			CreatureEffect(item, SpearBiteRight, DoBloodSplat);
			creature->Flags |= 1;
			SoundEffect(SFX_TR2_CRUNCH2, &item->Pose);
		}

		if (!(creature->Flags & 2) && item->TouchBits.Test(SpearBiteLeft.meshNum))
		{
			DoDamage(creature->Enemy, damage);
			CreatureEffect(item, SpearBiteLeft, DoBloodSplat);
			creature->Flags |= 2;
			SoundEffect(SFX_TR2_CRUNCH2, &item->Pose);
		}
	}

	static void SpawnSpearGuardianSmoke(const Vector3i& pos, int roomNumber)
	{
		auto& smoke = *GetFreeParticle();

		bool isUnderwater = TestEnvironment(ENV_FLAG_WATER, pos, roomNumber);
		auto sphere = BoundingSphere(pos.ToVector3(), 16);
		auto effectPos = Random::GeneratePointInSphere(sphere);

		smoke.on = true;
		smoke.blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;

		smoke.x = effectPos.x;
		smoke.y = effectPos.y;
		smoke.z = effectPos.z;
		smoke.xVel = Random::GenerateInt(-BLOCK(0.5f), BLOCK(0.5f));
		smoke.yVel = Random::GenerateInt(-BLOCK(1 / 8.0f), BLOCK(1 / 8.0f));
		smoke.zVel = Random::GenerateInt(-BLOCK(0.5f), BLOCK(0.5f));

		if (isUnderwater)
		{
			smoke.sR = 0;
			smoke.sG = 255;
			smoke.sB = 0;
			smoke.dR = 0;
			smoke.dG = 0;
			smoke.dB = 0;
		}
		else
		{
			smoke.sR = 0;
			smoke.sG = 0;
			smoke.sB = 0;
			smoke.dR = 0;
			smoke.dG = 255;
			smoke.dB = 0;
		}

		smoke.colFadeSpeed = 8;
		smoke.fadeToBlack = 64;
		smoke.sLife = smoke.life = Random::GenerateInt(72, 128);
		smoke.extras = 0;
		smoke.dynamic = -1;

		if (isUnderwater)
		{
			smoke.yVel /= 16;
			smoke.y += 32;
			smoke.friction = 4 | 16;
		}
		else
		{
			smoke.friction = 6;
		}

		smoke.rotAng = Random::GenerateAngle();
		smoke.rotAdd = Random::GenerateAngle(ANGLE(-0.2f), ANGLE(0.2f));
		smoke.flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		smoke.scalar = 3;

		if (isUnderwater)
		{
			smoke.gravity = 0;
			smoke.maxYvel = 0;
		}
		else
		{
			smoke.gravity = Random::GenerateInt(-8, -4);
			smoke.maxYvel = Random::GenerateInt(-8, -4);
		}

		int scale = Random::GenerateInt(128, 172);
		smoke.size = smoke.sSize = scale / 8;
		smoke.dSize = scale;
	}

	static void SwapMeshToJade(ItemInfo& item, int jointIndex, bool useEffect = true)
	{
		if (useEffect)
			SpawnSpearGuardianSmoke(GetJointPosition(&item, jointIndex), item.RoomNumber);
		item.Model.MeshIndex[jointIndex] = Objects[ID_SPEAR_GUARDIAN_STATUE].meshIndex + jointIndex;
	}

	static void SwapJadeToNormal(ItemInfo& item, int jointIndex, bool useEffect = true)
	{
		if (useEffect)
			SpawnSpearGuardianSmoke(GetJointPosition(&item, jointIndex), item.RoomNumber);
		item.Model.MeshIndex[jointIndex] = Objects[ID_SPEAR_GUARDIAN].meshIndex + jointIndex;
	}

	static bool DoSpearGuardianSwapMesh(ItemInfo& item)
	{
		const auto& object = Objects[item.ObjectNumber];
		auto& creature = *GetCreatureInfo(&item);

		// Do mesh swaps.
		if (creature.Flags == 0)
		{
			switch (item.ItemFlags[3])
			{
				// Jade to normal.
			case 0:
				SwapJadeToNormal(item, item.ItemFlags[0]);
				item.ItemFlags[0]++;
				if (item.ItemFlags[0] >= object.nmeshes)
				{
					item.ItemFlags[0] = 0;
					item.ItemFlags[1] = 0;
					item.ItemFlags[3] = 0;
					return true;
				}

				break;

				// Normal to jade.
			case 1:
				SwapMeshToJade(item, item.ItemFlags[0]);
				item.ItemFlags[0]--;

				if (item.ItemFlags[0] < 0)
				{
					item.ItemFlags[0] = 0;
					item.ItemFlags[1] = 1;
					item.ItemFlags[3] = 1;
					return true;
				}

				break;
			}

			creature.Flags = SPEAR_GUARDIAN_SWAPMESH_TIME;
		}
		else
		{
			creature.Flags--;
		}

		return false;
	}

	void InitialiseSpearGuardian(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		InitialiseCreature(itemNumber);
		item->ItemFlags[0] = 0; // Joint index when swapping mesh.
		item->ItemFlags[1] = 1; // Immune state. true = immune to damage.
		item->ItemFlags[2] = 1; // If 1, swap to jade. If 2, swap to normal.
		item->ItemFlags[3] = 0; // If mesh is swapped to jade, then it's true. Otherwise false.
		SetAnimation(item, SPEAR_GUARDIAN_ANIM_AWAKE);
		item->Status &= ~ITEM_INVISIBLE; // This statue is draw by default.

		const auto& object = Objects[item->ObjectNumber];
		for (int jointIndex = 0; jointIndex < object.nmeshes; jointIndex++)
			SwapMeshToJade(*item, jointIndex, false);
	}

	void SpearGuardianControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* object = &Objects[item->ObjectNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short tilt = 0;
		short head = 0;
		short neck = 0;

		bool isLaraAlive = creature->Enemy != nullptr && creature->Enemy->IsLara() && creature->Enemy->HitPoints > 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != SPEAR_GUARDIAN_STATE_DEATH)
			{
				item->ItemFlags[3] = 1;
				item->ItemFlags[0] = object->nmeshes - 1;
				item->Animation.ActiveState = SPEAR_GUARDIAN_STATE_DEATH;
			}

			if (DoSpearGuardianSwapMesh(*item))
				CreatureDie(itemNumber, true);
			return;
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case SPEAR_GUARDIAN_STATE_AWAKE:
				creature->MaxTurn = 0;

				DoSpearGuardianSwapMesh(*item);
				break;

			case SPEAR_GUARDIAN_STATE_STOP:
				creature->MaxTurn = 0;

				if (AI.ahead)
					neck = AI.angle;

				if (creature->Mood == MoodType::Bored)
				{
					if (Random::TestProbability(1 / 64.0f))
						item->Animation.TargetState = SPEAR_GUARDIAN_STATE_SLASH_IDLE;
					else if (Random::TestProbability(1 / 30.0f))
						item->Animation.TargetState = SPEAR_GUARDIAN_STATE_WALK;
				}
				else if (AI.ahead && AI.distance < SPEAR_GUARDIAN_DOUBLE_ATTACK_RANGE)
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_DOUBLE_ATTACK_FRONT_AIM;
				else
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_WALK;

				break;

			case SPEAR_GUARDIAN_STATE_SLASH_IDLE:
				creature->MaxTurn = 0;

				if (AI.ahead)
					neck = AI.angle;

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_WALK;
				else if (creature->Mood == MoodType::Bored)
				{
					if (Random::TestProbability(1 / 64.0f))
						item->Animation.TargetState = SPEAR_GUARDIAN_STATE_STOP;
					else if (Random::TestProbability(1 / 30.0f))
						item->Animation.TargetState = SPEAR_GUARDIAN_STATE_WALK;
				}
				else if (AI.ahead && AI.distance < SPEAR_GUARDIAN_SLASH_RANGE)
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_SLASH_AIM;
				else
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_WALK;

				break;

			case SPEAR_GUARDIAN_STATE_WALK:
				creature->MaxTurn = SPEAR_GUARDIAN_WALK_ANGLE_RATE_MAX;

				if (AI.ahead)
					neck = AI.angle;

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_RUN;
				else if (creature->Mood == MoodType::Bored)
				{
					if (Random::TestProbability(1 / 64.0f))
						item->Animation.TargetState = SPEAR_GUARDIAN_STATE_STOP;
					else if (Random::TestProbability(1 / 30.0f))
						item->Animation.TargetState = SPEAR_GUARDIAN_STATE_SLASH_IDLE;
				}
				else if (AI.ahead && AI.distance < SPEAR_GUARDIAN_ATTACK_RANGE)
				{
					if (AI.distance < SPEAR_GUARDIAN_DOUBLE_ATTACK_WALK_RANGE)
						item->Animation.TargetState = SPEAR_GUARDIAN_STATE_WALK_DOUBLE_ATTACK_FRONT_AIM;
					else if (Random::TestProbability(1 / 2.0f))
						item->Animation.TargetState = SPEAR_GUARDIAN_STATE_WALK_ATTACK_LEFT_SPEAR_AIM;
					else
						item->Animation.TargetState = SPEAR_GUARDIAN_STATE_WALK_ATTACK_RIGHT_SPEAR_AIM;
				}
				else if (!AI.ahead || AI.distance > SPEAR_GUARDIAN_RUN_RANGE)
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_RUN;

				break;

			case SPEAR_GUARDIAN_STATE_RUN:
				creature->MaxTurn = SPEAR_GUARDIAN_RUN_ANGLE_RATE_MAX;

				if (AI.ahead)
					neck = AI.angle;

				if (creature->Mood == MoodType::Escape)
					break;
				else if (creature->Mood == MoodType::Bored)
				{
					if (Random::TestProbability(1 / 2.0f))
						item->Animation.TargetState = SPEAR_GUARDIAN_STATE_STOP;
					else
						item->Animation.TargetState = SPEAR_GUARDIAN_STATE_SLASH_IDLE;
				}
				else if (AI.ahead && AI.distance < SPEAR_GUARDIAN_RUN_RANGE)
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_RUN_DOUBLE_ATTACK_FRONT_AIM;

				break;

			case SPEAR_GUARDIAN_STATE_DOUBLE_ATTACK_FRONT_AIM:
				if (AI.ahead)
					head = AI.angle;

				creature->Flags = 0;
				if (!AI.ahead || AI.distance > SPEAR_GUARDIAN_DOUBLE_ATTACK_RANGE)
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_STOP;
				else
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_DOUBLE_ATTACK_FRONT;

				break;

			case SPEAR_GUARDIAN_STATE_WALK_DOUBLE_ATTACK_FRONT_AIM:
				creature->Flags = 0;

				if (AI.ahead)
					head = AI.angle;

				if (!AI.ahead || AI.distance > SPEAR_GUARDIAN_DOUBLE_ATTACK_WALK_RANGE)
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_WALK;
				else
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_WALK_DOUBLE_ATTACK_FRONT;

				break;

			case SPEAR_GUARDIAN_STATE_WALK_ATTACK_LEFT_SPEAR_AIM:
				creature->Flags = 0;

				if (AI.ahead)
					head = AI.angle;

				if (!AI.ahead || AI.distance > SPEAR_GUARDIAN_RUN_RANGE)
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_WALK;
				else
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_WALK_ATTACK_LEFT_SPEAR;

				break;

			case SPEAR_GUARDIAN_STATE_WALK_ATTACK_RIGHT_SPEAR_AIM:
				if (AI.ahead)
					head = AI.angle;

				creature->Flags = 0;
				if (!AI.ahead || AI.distance > SPEAR_GUARDIAN_RUN_RANGE)
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_WALK;
				else
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_WALK_ATTACK_RIGHT_SPEAR;

				break;

			case SPEAR_GUARDIAN_STATE_SLASH_AIM:
				creature->Flags = 0;

				if (AI.ahead)
					head = AI.angle;

				if (!AI.ahead || AI.distance > SPEAR_GUARDIAN_SLASH_RANGE)
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_SLASH_IDLE;
				else
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_SLASH;

				break;

			case SPEAR_GUARDIAN_STATE_RUN_DOUBLE_ATTACK_FRONT_AIM:
				creature->Flags = 0;

				if (AI.ahead)
					head = AI.angle;

				if (!AI.ahead || AI.distance > SPEAR_GUARDIAN_RUN_RANGE)
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_RUN;
				else
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_RUN_DOUBLE_ATTACK_FRONT;

				break;

			case SPEAR_GUARDIAN_STATE_DOUBLE_ATTACK_FRONT:
				XianDamage(item, creature, SPEAR_GUARDIAN_POWERFUL_DAMAGE);
				break;

			case SPEAR_GUARDIAN_STATE_WALK_DOUBLE_ATTACK_FRONT:
			case SPEAR_GUARDIAN_STATE_WALK_ATTACK_LEFT_SPEAR:
			case SPEAR_GUARDIAN_STATE_WALK_ATTACK_RIGHT_SPEAR:
				XianDamage(item, creature, item->Animation.ActiveState == SPEAR_GUARDIAN_STATE_WALK_DOUBLE_ATTACK_FRONT ?
					SPEAR_GUARDIAN_POWERFUL_DAMAGE : SPEAR_GUARDIAN_BASIC_DAMAGE);

				if (AI.ahead)
					head = AI.angle;

				if (AI.ahead && AI.distance < SPEAR_GUARDIAN_SLASH_RANGE)
				{
					if (Random::TestProbability(1 / 2.0f))
						item->Animation.TargetState = SPEAR_GUARDIAN_STATE_STOP;
					else
						item->Animation.TargetState = SPEAR_GUARDIAN_STATE_SLASH_IDLE;
				}
				else
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_WALK;

				break;

			case SPEAR_GUARDIAN_STATE_SLASH:
				XianDamage(item, creature, SPEAR_GUARDIAN_BASIC_DAMAGE);

				if (AI.ahead)
					head = AI.angle;

				if (AI.ahead && AI.distance < SPEAR_GUARDIAN_SLASH_RANGE)
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_STOP;
				else
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_SLASH_IDLE;

				break;

			case SPEAR_GUARDIAN_STATE_RUN_DOUBLE_ATTACK_FRONT:
				XianDamage(item, creature, SPEAR_GUARDIAN_POWERFUL_DAMAGE);

				if (AI.ahead)
					head = AI.angle;

				if (AI.ahead && AI.distance < SPEAR_GUARDIAN_SLASH_RANGE)
				{
					if (Random::TestProbability(1 / 2.0f))
						item->Animation.TargetState = SPEAR_GUARDIAN_STATE_STOP;
					else
						item->Animation.TargetState = SPEAR_GUARDIAN_STATE_SLASH_IDLE;
				}
				else if (AI.ahead && AI.distance < SPEAR_GUARDIAN_RUN_RANGE)
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_WALK;
				else
					item->Animation.TargetState = SPEAR_GUARDIAN_STATE_RUN;

				break;
			}
		}

		if (isLaraAlive && creature->Enemy->HitPoints <= 0)
		{
			creature->MaxTurn = 0;
			CreatureKill(item, SPEAR_GUARDIAN_ANIM_KILL_LARA, LEA_XianSpearDeath, SPEAR_GUARDIAN_STATE_KILL_LARA, LS_DEATH);
			return;
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, neck);
		CreatureJoint(item, 1, head);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
