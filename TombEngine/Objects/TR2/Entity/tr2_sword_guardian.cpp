#include "framework.h"
#include "Objects/TR2/Entity/tr2_sword_guardian.h"

#include "Game/animation.h"
#include "collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

namespace TEN::Entities::Creatures::TR2
{
	constexpr auto SWORD_GUARDIAN_FLY_TURN_RATE_MAX = ANGLE(7.0f);
	constexpr auto SWORD_GUARDIAN_WALK_TURN_RATE_MAX = ANGLE(9.0f);

	const auto SwordBite = BiteInfo(Vector3(0.0f, 37.0f, 550.0f), 15);

	constexpr auto SWORD_GUARDIAN_ATTACK_RANGE = SQUARE(SECTOR(1));
	constexpr auto SWORD_GUARDIAN_WALK_ATTACK_RANGE = SQUARE(SECTOR(2));

	constexpr auto SWORD_GUARDIAN_SWAPMESH_TIME = 3;
	constexpr auto SWORD_GUARDIAN_ATTACK_DAMAGE = 300;

	enum SwordGuardianState
	{
		SWORD_GUARDIAN_STATE_STOP = 1,
		SWORD_GUARDIAN_STATE_WALK = 2,
		SWORD_GUARDIAN_STATE_ATTACK_FRONT_AIM = 3,
		SWORD_GUARDIAN_STATE_ATTACK_FRONT = 4,
		SWORD_GUARDIAN_STATE_ATTACK_HORIZONTAL_AIM = 5,
		SWORD_GUARDIAN_STATE_ATTACK_HORIZONTAL = 6,
		SWORD_GUARDIAN_STATE_WAIT = 7,
		SWORD_GUARDIAN_STATE_FLY = 8,
		SWORD_GUARDIAN_STATE_AWAKE = 9,
		SWORD_GUARDIAN_STATE_WALK_ATTACK_AIM = 10,
		SWORD_GUARDIAN_STATE_WALK_ATTACK = 11,
		SWORD_GUARDIAN_STATE_DEATH = 12
	};

	// TODO: Found all name, also found some which is more appropriate.
	enum SwordGuardianAnim
	{
		SWORD_GUARDIAN_ANIM_AWAKE = 0
	};

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
			smoke.sR = 255;
			smoke.sG = 255;
			smoke.sB = 255;
			smoke.dR = 0;
			smoke.dG = 0;
			smoke.dB = 0;
		}
		else
		{
			smoke.sR = 0;
			smoke.sG = 0;
			smoke.sB = 0;
			smoke.dR = 255;
			smoke.dG = 255;
			smoke.dB = 255;
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
		item.Model.MeshIndex[jointIndex] = Objects[ID_SWORD_GUARDIAN_STATUE].meshIndex + jointIndex;
	}

	static void SwapJadeToNormal(ItemInfo& item, int jointIndex, bool useEffect = true)
	{
		if (useEffect)
			SpawnSpearGuardianSmoke(GetJointPosition(&item, jointIndex), item.RoomNumber);
		item.Model.MeshIndex[jointIndex] = Objects[ID_SWORD_GUARDIAN].meshIndex + jointIndex;
	}

	static bool DoSwordGuardianSwapMesh(ItemInfo& item)
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

			creature.Flags = SWORD_GUARDIAN_SWAPMESH_TIME;
		}
		else
		{
			creature.Flags--;
		}

		return false;
	}

	static void SwordGuardianFly(ItemInfo* item)
	{
		auto pos = Vector3i(
			(GetRandomControl() * 256 / 32768) + item->Pose.Position.x - 128,
			(GetRandomControl() * 256 / 32768) + item->Pose.Position.y - 256,
			(GetRandomControl() * 256 / 32768) + item->Pose.Position.z - 128);

		SpawnSpearGuardianSmoke(pos, item->RoomNumber);
		SoundEffect(SFX_TR2_WARRIOR_HOVER, &item->Pose);
	}

	void InitialiseSwordGuardian(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		InitialiseCreature(itemNumber);
		item->ItemFlags[0] = 0; // Joint index when swapping mesh.
		item->ItemFlags[1] = 1; // Immune state. true = immune to damage.
		item->ItemFlags[2] = 1; // If 1, swap to jade. If 2, swap to normal.
		item->ItemFlags[3] = 0; // If mesh is swapped to jade, then it's true. Otherwise false.
		SetAnimation(item, SWORD_GUARDIAN_ANIM_AWAKE);
		item->Status &= ~ITEM_INVISIBLE; // This statue is draw by default.

		const auto& object = Objects[item->ObjectNumber];
		for (int jointIndex = 0; jointIndex < object.nmeshes; jointIndex++)
			SwapMeshToJade(*item, jointIndex, false);
	}

	void SwordGuardianControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* object = &Objects[item->ObjectNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short neck = 0;
		short torso = 0;

		bool isLaraAlive = creature->Enemy != nullptr && creature->Enemy->IsLara() && creature->Enemy->HitPoints > 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != SWORD_GUARDIAN_STATE_DEATH)
			{
				item->ItemFlags[3] = 1;
				item->ItemFlags[0] = object->nmeshes - 1;
				item->Animation.ActiveState = SWORD_GUARDIAN_STATE_DEATH;
			}

			if (DoSwordGuardianSwapMesh(*item))
				CreatureDie(itemNumber, true);
			return;
		}
		else
		{

			AI_INFO AI;
			creature->LOT.Step = CLICK(1);
			creature->LOT.Drop = -CLICK(1);
			creature->LOT.Fly = NO_FLYING;
			creature->LOT.Zone = ZoneType::Basic;
			CreatureAIInfo(item, &AI);
			if (AI.enemyZone != AI.zoneNumber && item->Animation.ActiveState == SWORD_GUARDIAN_STATE_FLY)
			{
				creature->LOT.Step = BLOCK(20);
				creature->LOT.Drop = -BLOCK(20);
				creature->LOT.Fly = 64;
				creature->LOT.Zone = ZoneType::Flyer;
				CreatureAIInfo(item, &AI);
			}

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case SWORD_GUARDIAN_STATE_AWAKE:
				creature->MaxTurn = 0;

				DoSwordGuardianSwapMesh(*item);
				break;

			case SWORD_GUARDIAN_STATE_STOP:
				creature->MaxTurn = 0;

				if (AI.ahead)
					neck = AI.angle;

				if (!isLaraAlive)
					item->Animation.TargetState = SWORD_GUARDIAN_STATE_WAIT;
				else if (AI.bite && AI.distance < SWORD_GUARDIAN_ATTACK_RANGE)
				{
					if (Random::TestProbability(1 / 2.0f))
						item->Animation.TargetState = SWORD_GUARDIAN_STATE_ATTACK_HORIZONTAL_AIM;
					else
						item->Animation.TargetState = SWORD_GUARDIAN_STATE_ATTACK_FRONT_AIM;
				}
				else if (AI.zoneNumber != AI.enemyZone)
					item->Animation.TargetState = SWORD_GUARDIAN_STATE_FLY;
				else
					item->Animation.TargetState = SWORD_GUARDIAN_STATE_WALK;

				break;

			case SWORD_GUARDIAN_STATE_WALK:
				creature->MaxTurn = SWORD_GUARDIAN_WALK_TURN_RATE_MAX;

				if (AI.ahead)
					neck = AI.angle;

				if (!isLaraAlive)
					item->Animation.TargetState = SWORD_GUARDIAN_STATE_STOP;
				else if (AI.bite && AI.distance < SWORD_GUARDIAN_WALK_ATTACK_RANGE)
					item->Animation.TargetState = SWORD_GUARDIAN_STATE_WALK_ATTACK_AIM;
				else if (AI.zoneNumber != AI.enemyZone)
					item->Animation.TargetState = SWORD_GUARDIAN_STATE_STOP;

				break;

			case SWORD_GUARDIAN_STATE_ATTACK_FRONT_AIM:
				creature->Flags = 0;

				if (AI.ahead)
					torso = AI.angle;

				if (!AI.bite || AI.distance > SWORD_GUARDIAN_ATTACK_RANGE)
					item->Animation.TargetState = SWORD_GUARDIAN_STATE_STOP;
				else
					item->Animation.TargetState = SWORD_GUARDIAN_STATE_ATTACK_FRONT;
				break;

			case SWORD_GUARDIAN_STATE_ATTACK_HORIZONTAL_AIM:
				creature->Flags = 0;

				if (AI.ahead)
					torso = AI.angle;

				if (!AI.bite || AI.distance > SWORD_GUARDIAN_ATTACK_RANGE)
					item->Animation.TargetState = SWORD_GUARDIAN_STATE_STOP;
				else
					item->Animation.TargetState = SWORD_GUARDIAN_STATE_ATTACK_HORIZONTAL;

				break;

			case SWORD_GUARDIAN_STATE_WALK_ATTACK_AIM:
				creature->Flags = 0;

				if (AI.ahead)
					torso = AI.angle;

				if (!AI.bite || AI.distance > SWORD_GUARDIAN_WALK_ATTACK_RANGE)
					item->Animation.TargetState = SWORD_GUARDIAN_STATE_STOP;
				else
					item->Animation.TargetState = SWORD_GUARDIAN_STATE_WALK_ATTACK;

				break;

			case SWORD_GUARDIAN_STATE_FLY:
				creature->MaxTurn = SWORD_GUARDIAN_FLY_TURN_RATE_MAX;
				if (AI.ahead)
					neck = AI.angle;
				SwordGuardianFly(item);
				if (creature->LOT.Fly == NO_FLYING)
					item->Animation.TargetState = SWORD_GUARDIAN_STATE_STOP;

				break;

			case SWORD_GUARDIAN_STATE_ATTACK_FRONT:
			case SWORD_GUARDIAN_STATE_ATTACK_HORIZONTAL:
			case SWORD_GUARDIAN_STATE_WALK_ATTACK:
				if (AI.ahead)
					torso = AI.angle;

				if (!creature->Flags && item->TouchBits.Test(SwordBite.meshNum))
				{
					DoDamage(creature->Enemy, SWORD_GUARDIAN_ATTACK_DAMAGE);
					CreatureEffect(item, SwordBite, DoBloodSplat);
					creature->Flags = 1;
				}

				break;
			}
		}

		CreatureJoint(item, 0, neck);
		CreatureJoint(item, 1, torso);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
