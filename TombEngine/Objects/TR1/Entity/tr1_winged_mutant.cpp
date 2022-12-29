#include "framework.h"
#include "Objects/TR1/Entity/tr1_winged_mutant.h"

#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Game/missile.h"
#include "Game/people.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR1
{
	constexpr auto WINGED_MUTANT_IDLE_JUMP_ATTACK_DAMAGE = 150;
	constexpr auto WINGED_MUTANT_RUN_JUMP_ATTACK_DAMAGE  = 100;
	constexpr auto WINGED_MUTANT_SWIPE_ATTACK_DAMAGE     = 200;

	constexpr auto WINGED_MUTANT_WALK_RANGE				= SQUARE(SECTOR(4.5f));
	constexpr auto WINGED_MUTANT_SWIPE_ATTACK_RANGE		= SQUARE(CLICK(1.17f));
	constexpr auto WINGED_MUTANT_RUN_JUMP_ATTACK_RANGE	= SQUARE(CLICK(2.34f));
	constexpr auto WINGED_MUTANT_IDLE_JUMP_ATTACK_RANGE = SQUARE(SECTOR(2.5f));
	constexpr auto WINGED_MUTANT_ATTACK_RANGE			= SQUARE(SECTOR(3.75f));

	constexpr auto WINGED_MUTANT_POSE_CHANCE   = 1.0f / 400;
	constexpr auto WINGED_MUTANT_UNPOSE_CHANCE = 1.0f / 164;

	constexpr auto WINGED_MUTANT_FLY_VELOCITY	= CLICK(1) / 8;
	constexpr auto WINGED_MUTANT_SHARD_VELOCITY = 250;
	constexpr auto WINGED_MUTANT_BOMB_VELOCITY  = 220;

	const auto WINGED_MUTANT_WALK_FORWARD_TURN_RATE_MAX = ANGLE(2.0f);
	const auto WINGED_MUTANT_RUN_FORWARD_TURN_RATE_MAX	= ANGLE(6.0f);

	const auto WingedMutantBite		  = BiteInfo(Vector3(-27.0f, 98.0f, 0.0f), 10);
	const auto WingedMutantRocketBite = BiteInfo(Vector3(51.0f, 213.0f, 0.0f), 14);
	const auto WingedMutantShardBite  = BiteInfo(Vector3(-35.0f, 269.0f, 0.0f), 9);
	const auto WingedMutantJoints = std::vector<unsigned int>{ 9, 10, 14 };

	enum WingedMutantState
	{
		WMUTANT_STATE_NONE = 0,
		WMUTANT_STATE_IDLE = 1,
		WMUTANT_STATE_WALK_FORWARD = 2,
		WMUTANT_STATE_RUN_FORWARD = 3,
		WMUTANT_STATE_IDLE_JUMP_ATTACK = 4,
		WMUTANT_STATE_DEATH = 5,
		WMUTANT_STATE_POSE = 6,
		WMUTANT_STATE_RUN_JUMP_ATTACK = 7,
		WMUTANT_STATE_SWIPE_ATTACK = 8,
		WMUTANT_STATE_AIM_DART = 9,
		WMUTANT_STATE_AIM_BOMB = 10,
		WMUTANT_STATE_SHOOT = 11,
		WMUTANT_STATE_INACTIVE = 12,
		WMUTANT_STATE_FLY = 13,
	};

	enum WingedMutantAnim
	{
		WMUTANT_ANIM_INACTIVE = 0,
		WMUTANT_ANIM_INACTIVE_TO_IDLE = 1,
		WMUTANT_ANIM_IDLE = 2,
		WMUTANT_ANIM_IDLE_TO_RUN = 3,
		WMUTANT_ANIM_RUN_FORWARD = 4,
		WMUTANT_ANIM_IDLE_JUMP_ATTACK_START = 5,
		WMUTANT_ANIM_IDLE_JUMP_ATTACK_END = 6,
		WMUTANT_ANIM_IDLE_TO_POSE = 7,
		WMUTANT_ANIM_POSE = 8,
		WMUTANT_ANIM_POSE_TO_IDLE = 9,
		WMUTANT_ANIM_POSE_TO_WALK_FORWARD = 10,
		WMUTANT_ANIM_WALK_FORWARD = 11,
		WMUTANT_ANIM_WALK_FORWARD_TO_IDLE = 12,
		WMUTANT_ANIM_WALK_FORWARD_TO_POSE = 13,
		WMUTANT_ANIM_RUN_JUMP_ATTACK = 14,
		WMUTANT_ANIM_IDLE_TO_AIM_1 = 15,
		WMUTANT_ANIM_AIM_DART = 16,
		WMUTANT_ANIM_SHOOT_DART = 17,
		WMUTANT_ANIM_AIM_DART_TO_IDLE = 18,
		WMUTANT_ANIM_IDLE_TO_AIM_BOMB = 19,
		WMUTANT_ANIM_SHOOT_BOMB = 20,
		WMUTANT_ANIM_RUN_FORWARD_TO_IDLE = 21,
		WMUTANT_ANIM_AIM_BOMB_TO_IDLE = 22,
		WMUTANT_ANIM_IDLE_TO_FLY = 23,
		WMUTANT_ANIM_FLY = 24,
		WMUTANT_ANIM_FLY_TO_IDLE = 25,
		WMUTANT_ANIM_SWIPE_ATTACK = 26
	};

	enum WingedMutantPathFinding
	{
		WMUTANT_PATH_GROUND = 1,
		WMUTANT_PATH_AERIAL = 2
	};

	// NOTE: Originally, winged mutants did not have OCBs. -- TokyoSU 5/8/2022
	enum WingedMutantOcb
	{
		WMUTANT_OCB_START_AERIAL		= (1 << 0),
		WMUTANT_OCB_START_INACTIVE		= (1 << 1),
		WMUTANT_OCB_START_POSE			= (1 << 2),
		WMUTANT_OCB_NO_WINGS			= (1 << 3),
		WMUTANT_OCB_DISABLE_DART_WEAPON = (1 << 4),
		WMUTANT_OCB_DISABLE_BOMB_WEAPON = (1 << 5)
	};

	enum WingedMutantProjectileType
	{
		WMUTANT_PROJ_NONE,
		WMUTANT_PROJ_DART,
		WMUTANT_PROJ_BOMB
	};

	enum WingedMutantConfig
	{
		WMUTANT_CONF_CAN_FLY,
		WMUTANT_CONF_PATHFINDING_MODE,
		WMUTANT_CONF_PROJECTILE_MODE,
		WMUTANT_CONF_NO_WINGS,
		WMUTANT_CONF_DISABLE_DART_WEAPON,
		WMUTANT_CONF_DISABLE_BOMB_WEAPON
	};

	void SwitchPathfinding(CreatureInfo* creature, WingedMutantPathFinding path)
	{
		switch (path)
		{
		case WMUTANT_PATH_GROUND:
			creature->LOT.Step = CLICK(1);
			creature->LOT.Drop = -CLICK(1);
			creature->LOT.Fly = NO_FLYING;
			break;

		case WMUTANT_PATH_AERIAL:
			creature->LOT.Step = SECTOR(30);
			creature->LOT.Drop = -SECTOR(30);
			creature->LOT.Fly = WINGED_MUTANT_FLY_VELOCITY;
			break;
		}
	}

	WingedMutantProjectileType CanTargetLara(ItemInfo* item, CreatureInfo* creature, AI_INFO* AI)
	{
		if (Targetable(item, AI) &&  (AI->zoneNumber != AI->enemyZone || AI->distance > WINGED_MUTANT_ATTACK_RANGE))
		{
			if ((AI->angle > 0 && AI->angle < ANGLE(45.0f)) &&
				item->TestFlagEqual(WMUTANT_OCB_DISABLE_DART_WEAPON, false))
			{
				return WMUTANT_PROJ_DART;
			}
			else if ((AI->angle < 0 && AI->angle > -ANGLE(45.0f)) &&
				item->TestFlagEqual(WMUTANT_OCB_DISABLE_BOMB_WEAPON, false))
			{
				return WMUTANT_PROJ_BOMB;
			}
		}

		// Cannot be targeted.
		return WMUTANT_PROJ_NONE;
	}

	void WingedInitOCB(ItemInfo* item, CreatureInfo* creature)
	{
		if (item->TestOcb(WMUTANT_OCB_START_AERIAL))
		{
			SwitchPathfinding(creature, WMUTANT_PATH_AERIAL);
			SetAnimation(item, WMUTANT_ANIM_FLY);
			item->SetFlag(WMUTANT_CONF_PATHFINDING_MODE, WMUTANT_PATH_AERIAL);
		}
		else if (item->TestOcb(WMUTANT_OCB_START_INACTIVE))
		{
			SwitchPathfinding(creature, WMUTANT_PATH_GROUND);
			SetAnimation(item, WMUTANT_ANIM_INACTIVE);
			item->SetFlag(WMUTANT_CONF_PATHFINDING_MODE, WMUTANT_PATH_GROUND);
		}
		else if (item->TestOcb(WMUTANT_OCB_START_POSE))
		{
			SwitchPathfinding(creature, WMUTANT_PATH_GROUND);
			SetAnimation(item, WMUTANT_ANIM_INACTIVE);
			item->SetFlag(WMUTANT_CONF_PATHFINDING_MODE, WMUTANT_PATH_GROUND);
		}

		// Remove OCBs since we don't need them anymore.
		if (item->TestOcb(WMUTANT_OCB_START_AERIAL))
			item->RemoveOcb(WMUTANT_OCB_START_AERIAL);
		if (item->TestOcb(WMUTANT_OCB_START_INACTIVE))
			item->RemoveOcb(WMUTANT_OCB_START_INACTIVE);
		if (item->TestOcb(WMUTANT_OCB_START_POSE))
			item->RemoveOcb(WMUTANT_OCB_START_POSE);
	}

	// NOTE: Doesn't exist in the original game. -- TokyoSU 5/8/2022
	void InitialiseWingedMutant(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);
		item->SetFlag(WMUTANT_CONF_PATHFINDING_MODE, WMUTANT_PATH_GROUND);
		item->SetFlag(WMUTANT_CONF_PROJECTILE_MODE, WMUTANT_PROJ_NONE);

		if (item->TestOcb(WMUTANT_OCB_NO_WINGS))
		{
			item->SetFlag(WMUTANT_CONF_CAN_FLY, false);
			item->MeshBits = 0xFFE07FFF;
		}
		else
			item->SetFlag(WMUTANT_CONF_CAN_FLY, true);

		if (item->TestOcb(WMUTANT_OCB_DISABLE_BOMB_WEAPON))
			item->SetFlag(WMUTANT_CONF_DISABLE_BOMB_WEAPON, true);
		if (item->TestOcb(WMUTANT_OCB_DISABLE_DART_WEAPON))
			item->SetFlag(WMUTANT_CONF_DISABLE_DART_WEAPON, true);

		if (item->TestOcb(WMUTANT_OCB_DISABLE_BOMB_WEAPON))
			item->RemoveOcb(WMUTANT_OCB_DISABLE_BOMB_WEAPON);
		if (item->TestOcb(WMUTANT_OCB_DISABLE_DART_WEAPON))
			item->RemoveOcb(WMUTANT_OCB_DISABLE_DART_WEAPON);
		if (item->TestOcb(WMUTANT_OCB_NO_WINGS))
			item->RemoveOcb(WMUTANT_OCB_NO_WINGS);
	}

	void WingedMutantControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short head = 0;
		short torso = 0; // Only when shooting.

		bool flyEnabled = item->TestFlagEqual(WMUTANT_CONF_CAN_FLY, true);
		bool flyStatus = item->TestFlagEqual(WMUTANT_CONF_PATHFINDING_MODE, WMUTANT_PATH_AERIAL);

		WingedInitOCB(item, creature);

		if (item->HitPoints <= 0)
		{
			CreatureDie(itemNumber, true);
			SoundEffect(SFX_TR1_ATLANTEAN_EXPLODE, &item->Pose);
			return;
		}
		else
		{
			AI_INFO AI;
			SwitchPathfinding(creature, WMUTANT_PATH_GROUND);
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;
			else
			{
				head = 0;
				torso = 0;
			}

			GetCreatureMood(item, &AI, flyStatus);
			CreatureMood(item, &AI, flyStatus);
			angle = CreatureTurn(item, creature->MaxTurn);

			auto shootType = CanTargetLara(item, creature, &AI);
			if (flyEnabled)
			{
				if (item->Animation.ActiveState == WMUTANT_STATE_FLY)
				{
					if (flyStatus && creature->Mood != MoodType::Escape &&
						AI.zoneNumber == AI.enemyZone)
					{
						item->SetFlag(WMUTANT_CONF_PATHFINDING_MODE, WMUTANT_PATH_GROUND);
					}

					SwitchPathfinding(creature, WMUTANT_PATH_AERIAL);
					CreatureAIInfo(item, &AI);
				}
				else if ((AI.zoneNumber != AI.enemyZone &&
					!flyStatus && shootType == WMUTANT_PROJ_NONE &&
					(!AI.ahead || creature->Mood == MoodType::Bored)) ||
					creature->Mood == MoodType::Escape)
				{
					item->SetFlag(WMUTANT_CONF_PATHFINDING_MODE, WMUTANT_PATH_AERIAL);
				}
			}

			switch (item->Animation.ActiveState)
			{
			case WMUTANT_STATE_INACTIVE:
				creature->MaxTurn = 0;

				if (TargetVisible(item, &AI) || creature->HurtByLara)
					item->Animation.TargetState = WMUTANT_STATE_IDLE;

				break;

			case WMUTANT_STATE_IDLE:
				torso = 0;
				creature->MaxTurn = 0;
				item->SetFlag(WMUTANT_CONF_PATHFINDING_MODE, WMUTANT_PROJ_NONE);

				if (flyStatus && flyEnabled)
					item->Animation.TargetState = WMUTANT_STATE_FLY;
				else if (item->TouchBits.Test(WingedMutantJoints[1]))
					item->Animation.TargetState = WMUTANT_STATE_SWIPE_ATTACK;
				else if (AI.bite && AI.distance < WINGED_MUTANT_IDLE_JUMP_ATTACK_RANGE)
					item->Animation.TargetState = WMUTANT_STATE_IDLE_JUMP_ATTACK;
				else if (AI.bite && AI.distance < WINGED_MUTANT_SWIPE_ATTACK_RANGE)
					item->Animation.TargetState = WMUTANT_STATE_SWIPE_ATTACK;
				else if (shootType == WMUTANT_PROJ_DART)
					item->Animation.TargetState = WMUTANT_STATE_AIM_DART;
				else if (shootType == WMUTANT_PROJ_BOMB)
					item->Animation.TargetState = WMUTANT_STATE_AIM_BOMB;
				else if (creature->Mood == MoodType::Bored ||
					(creature->Mood == MoodType::Stalk && AI.distance < WINGED_MUTANT_WALK_RANGE))
				{
					item->Animation.TargetState = WMUTANT_STATE_POSE;
				}
				else
					item->Animation.TargetState = WMUTANT_STATE_RUN_FORWARD;

				break;

			case WMUTANT_STATE_POSE:
				head = 0; // Pose has an animation for the head.
				creature->MaxTurn = 0;

				if (shootType != WMUTANT_PROJ_NONE || (flyStatus && flyEnabled))
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (creature->Mood == MoodType::Stalk)
				{
					if (AI.distance < WINGED_MUTANT_WALK_RANGE)
					{
						if (AI.zoneNumber == AI.enemyZone ||
							Random::TestProbability(WINGED_MUTANT_UNPOSE_CHANCE))
						{
							item->Animation.TargetState = WMUTANT_STATE_WALK_FORWARD;
						}
					}
					else
						item->Animation.TargetState = WMUTANT_STATE_IDLE;
				}
				else if (creature->Mood == MoodType::Bored && Random::TestProbability(WINGED_MUTANT_UNPOSE_CHANCE))
					item->Animation.TargetState = WMUTANT_STATE_WALK_FORWARD;
				else if (creature->Mood == MoodType::Attack ||
					creature->Mood == MoodType::Escape)
					item->Animation.TargetState = WMUTANT_STATE_IDLE;

				break;

			case WMUTANT_STATE_WALK_FORWARD:
				creature->MaxTurn = WINGED_MUTANT_WALK_FORWARD_TURN_RATE_MAX;

				if (shootType != WMUTANT_PROJ_NONE || (flyStatus && flyEnabled))
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (creature->Mood == MoodType::Attack || creature->Mood == MoodType::Escape)
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (creature->Mood == MoodType::Bored ||
					(creature->Mood == MoodType::Stalk && AI.zoneNumber != AI.enemyZone))
				{
					if (Random::TestProbability(WINGED_MUTANT_POSE_CHANCE))
						item->Animation.TargetState = WMUTANT_STATE_POSE;
				}
				else if (creature->Mood == MoodType::Stalk &&
					AI.distance > WINGED_MUTANT_WALK_RANGE)
				{
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				}

				break;

			case WMUTANT_STATE_RUN_FORWARD:
				creature->MaxTurn = WINGED_MUTANT_RUN_FORWARD_TURN_RATE_MAX;

				if (flyStatus && flyEnabled)
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (item->TouchBits.Test(WingedMutantJoints[1]))
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (AI.bite && AI.distance < WINGED_MUTANT_RUN_JUMP_ATTACK_RANGE)
					item->Animation.TargetState = WMUTANT_STATE_RUN_JUMP_ATTACK;
				else if (AI.bite && AI.distance < WINGED_MUTANT_SWIPE_ATTACK_RANGE)
					item->Animation.TargetState = WMUTANT_STATE_SWIPE_ATTACK;
				else if (AI.ahead && AI.distance < WINGED_MUTANT_SWIPE_ATTACK_RANGE)
					item->Animation.TargetState = WMUTANT_STATE_SWIPE_ATTACK;
				else if (shootType != WMUTANT_PROJ_NONE)
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (creature->Mood == MoodType::Bored ||
					(creature->Mood == MoodType::Stalk && AI.distance < WINGED_MUTANT_WALK_RANGE))
				{
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				}

				break;

			case WMUTANT_STATE_IDLE_JUMP_ATTACK:
				if (item->Animation.RequiredState == WMUTANT_STATE_NONE &&
					item->TouchBits.Test(WingedMutantJoints[1]))
				{
					DoDamage(creature->Enemy, WINGED_MUTANT_IDLE_JUMP_ATTACK_DAMAGE);
					CreatureEffect(item, WingedMutantBite, DoBloodSplat);
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				}

				break;

			case WMUTANT_STATE_RUN_JUMP_ATTACK:
				if (item->Animation.RequiredState == WMUTANT_STATE_NONE &&
					item->TouchBits.Test(WingedMutantJoints[1]))
				{
					DoDamage(creature->Enemy, WINGED_MUTANT_RUN_JUMP_ATTACK_DAMAGE);
					CreatureEffect(item, WingedMutantBite, DoBloodSplat);
					item->Animation.TargetState = WMUTANT_STATE_RUN_FORWARD;
				}

				break;

			case WMUTANT_STATE_SWIPE_ATTACK:
				if (item->Animation.RequiredState == WMUTANT_STATE_NONE &&
					item->TouchBits.Test(WingedMutantJoints[1]))
				{
					DoDamage(creature->Enemy, WINGED_MUTANT_SWIPE_ATTACK_DAMAGE);
					CreatureEffect(item, WingedMutantBite, DoBloodSplat);
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				}

				break;

			case WMUTANT_STATE_AIM_DART:
				torso = AI.angle / 2;
				creature->MaxTurn = 0;
				item->SetFlag(WMUTANT_CONF_PROJECTILE_MODE, WMUTANT_PROJ_DART);

				if (shootType == WMUTANT_PROJ_DART)
					item->Animation.TargetState = WMUTANT_STATE_SHOOT;
				else
					item->Animation.TargetState = WMUTANT_STATE_IDLE;

				break;

			case WMUTANT_STATE_AIM_BOMB:
				torso = AI.angle / 2;
				creature->MaxTurn = 0;
				item->SetFlag(WMUTANT_CONF_PROJECTILE_MODE, WMUTANT_PROJ_BOMB);

				if (shootType == WMUTANT_PROJ_BOMB)
					item->Animation.TargetState = WMUTANT_STATE_SHOOT;
				else
					item->Animation.TargetState = WMUTANT_STATE_IDLE;

				break;

			case WMUTANT_STATE_SHOOT:
			{
				torso = AI.angle / 2;
				creature->MaxTurn = 0;

				bool isDart = item->TestFlagEqual(WMUTANT_CONF_PROJECTILE_MODE, WMUTANT_PROJ_DART);
				bool isBomb = item->TestFlagEqual(WMUTANT_CONF_PROJECTILE_MODE, WMUTANT_PROJ_BOMB);

				if (isDart)
					CreatureEffect2(item, WingedMutantShardBite, WINGED_MUTANT_SHARD_VELOCITY, torso, ShardGun);
				else if (isBomb)
					CreatureEffect2(item, WingedMutantRocketBite, WINGED_MUTANT_BOMB_VELOCITY, torso, BombGun);

				item->SetFlag(WMUTANT_CONF_PROJECTILE_MODE, WMUTANT_PROJ_NONE);
				break;
			}

			case WMUTANT_STATE_FLY:
				if (!flyStatus && item->Pose.Position.y == item->Floor)
					item->Animation.TargetState = WMUTANT_STATE_IDLE; // Switch to ground mode.

				break;
			}
		}

		CreatureJoint(item, 0, torso);
		CreatureJoint(item, 1, head);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
