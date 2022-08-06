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
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/trmath.h"

using std::vector;

namespace TEN::Entities::TR1
{
	constexpr auto WINGED_MUTANT_IDLE_JUMP_ATTACK_DAMAGE = 150;
	constexpr auto WINGED_MUTANT_RUN_JUMP_ATTACK_DAMAGE	 = 100;
	constexpr auto WINGED_MUTANT_SWIPE_ATTACK_DAMAGE	 = 200;

	constexpr auto WINGED_MUTANT_WALK_RANGE				= SQUARE(SECTOR(4.5f));
	constexpr auto WINGED_MUTANT_SWIPE_ATTACK_RANGE		= SQUARE(CLICK(1.17f));
	constexpr auto WINGED_MUTANT_RUN_JUMP_ATTACK_RANGE	= SQUARE(CLICK(2.34f));
	constexpr auto WINGED_MUTANT_IDLE_JUMP_ATTACK_RANGE = SQUARE(SECTOR(2.5f));
	constexpr auto WINGED_MUTANT_ATTACK_RANGE			= SQUARE(SECTOR(3.75f));

	constexpr auto WINGED_MUTANT_POSE_CHANCE   = 85;
	constexpr auto WINGED_MUTANT_UNPOSE_CHANCE = 200;

	constexpr auto WINGED_MUTANT_FLY_VELOCITY	= CLICK(1) / 8;
	constexpr auto WINGED_MUTANT_SHARD_VELOCITY = 250;
	constexpr auto WINGED_MUTANT_BOMB_VELOCITY  = 220;
	constexpr auto WINGED_MUTANT_FLY_MODE	 = 0; // itemFlags[0]
	constexpr auto WINGED_MUTANT_BULLET_MODE = 1; // itemFlags[1]

	#define WINGED_MUTANT_WALK_FORWARD_TURN_RATE_MAX ANGLE(2.0f)
	#define WINGED_MUTANT_RUN_FORWARD_TURN_RATE_MAX	 ANGLE(6.0f)

	const auto WingedMutantBite		  = BiteInfo(Vector3(-27.0f, 98.0f, 0.0f), 10);
	const auto WingedMutantRocketBite = BiteInfo(Vector3(51.0f, 213.0f, 0.0f), 14);
	const auto WingedMutantShardBite  = BiteInfo(Vector3(-35.0f, 269.0f, 0.0f), 9);
	const vector<int> WingedMutantJoints = { WingedMutantShardBite.meshNum, WingedMutantBite.meshNum, WingedMutantRocketBite.meshNum };

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

	// Defines pathfinding type.
	enum class WingedMutantPath
	{
		Ground,
		Aerial
	};

	// NOTE: Originally, winged mutants did not have OCB. -- TokyoSU 5/8/2022
	enum class WingedMutantOcb
	{
		Normal,
		Aerial,
		Inactive
	};

	enum class WingedMutantProjectileType
	{
		None,
		Dart,
		Bomb
	};

	static bool IsFlyEnabled(ItemInfo* item)
	{
		return item->ItemFlags[WINGED_MUTANT_FLY_MODE] == true;
	}

	static void SetFlyMode(ItemInfo* item, bool enabled)
	{
		item->ItemFlags[WINGED_MUTANT_FLY_MODE] = (int)enabled;
	}

	static WingedMutantProjectileType GetProjectileType(ItemInfo* item)
	{
		return (WingedMutantProjectileType)item->ItemFlags[WINGED_MUTANT_BULLET_MODE];
	}

	static void SetWeaponType(ItemInfo* item, WingedMutantProjectileType type)
	{
		item->ItemFlags[WINGED_MUTANT_BULLET_MODE] = (int)type;
	}

	static void SwitchPathfinding(CreatureInfo* creature, WingedMutantPath path)
	{
		switch (path)
		{
		case WingedMutantPath::Ground:
			creature->LOT.Step = CLICK(1);
			creature->LOT.Drop = -CLICK(1);
			creature->LOT.Fly = NO_FLYING;
			break;

		case WingedMutantPath::Aerial:
			creature->LOT.Step = SECTOR(30);
			creature->LOT.Drop = -SECTOR(30);
			creature->LOT.Fly = WINGED_MUTANT_FLY_VELOCITY;
			break;
		}
	}

	static void WingedInitOCB(ItemInfo* item, CreatureInfo* creature)
	{
		switch ((WingedMutantOcb)item->TriggerFlags)
		{
		case WingedMutantOcb::Aerial:
			SetAnimation(item, WMUTANT_ANIM_FLY);
			SwitchPathfinding(creature, WingedMutantPath::Aerial);
			SetFlyMode(item, true);
			item->TriggerFlags = 0;
			break;

		case WingedMutantOcb::Inactive:
			SetAnimation(item, WMUTANT_ANIM_INACTIVE);
			SwitchPathfinding(creature, WingedMutantPath::Ground);
			SetFlyMode(item, false);
			item->TriggerFlags = 0;
			break;
		}
	}

	// NOTE: Doesn't exist in the original game. -- TokyoSU 5/8/2022
	void InitialiseWingedMutant(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);
		SetFlyMode(item, false); // Define fly mode (true = fly)
		SetWeaponType(item, WingedMutantProjectileType::None); // Define projectile type.
		SetAnimation(item, WMUTANT_ANIM_IDLE);
	}

	// item->ItemFlags[0] serves as fly detection.
	void WingedMutantControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short head = 0;
		short torso = 0; // Only when shooting.
		short angle = 0;

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
			SwitchPathfinding(creature, WingedMutantPath::Ground);
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;
			else
			{
				head = 0;
				torso = 0;
			}

			GetCreatureMood(item, &AI, IsFlyEnabled(item)); // true = FLY = AGGRESIVE
			CreatureMood(item, &AI, IsFlyEnabled(item));
			angle = CreatureTurn(item, creature->MaxTurn);

			bool shoot1 = false;
			bool shoot2 = false;

			if (item->ObjectNumber == ID_WINGED_MUMMY)
			{
				if (item->Animation.ActiveState == WMUTANT_STATE_FLY)
				{
					if (IsFlyEnabled(item) &&
						creature->Mood != MoodType::Escape &&
						AI.zoneNumber == AI.enemyZone)
					{
						SetFlyMode(item, false);
					}

					SwitchPathfinding(creature, WingedMutantPath::Aerial);
					CreatureAIInfo(item, &AI);
				}
				else if ((AI.zoneNumber != AI.enemyZone &&
						!IsFlyEnabled(item) &&
						!shoot1 && !shoot2 &&
						(!AI.ahead || creature->Mood == MoodType::Bored)) ||
					creature->Mood == MoodType::Escape)
				{
					SetFlyMode(item, true);
				}
			}

			if (item->ObjectNumber != ID_MUTANT &&
				Targetable(item, creature, &AI) &&
			   (AI.zoneNumber != AI.enemyZone || AI.distance > WINGED_MUTANT_ATTACK_RANGE))
			{
				if (AI.angle > 0 && AI.angle < ANGLE(45.0f))
					shoot1 = true;
				else if (AI.angle < 0 && angle > -ANGLE(45.0f))
					shoot2 = true;
			}
			// Not targetable.
			else
			{
				shoot1 = false;
				shoot2 = false;
			}

			switch ((WingedMutantState)item->Animation.ActiveState)
			{
			case WMUTANT_STATE_INACTIVE:
				creature->MaxTurn = 0;

				if (TargetVisible(item, creature, &AI) || creature->HurtByLara)
					item->Animation.TargetState = WMUTANT_STATE_IDLE;

				break;

			case WMUTANT_STATE_IDLE:
				torso = 0;
				creature->MaxTurn = 0;
				SetWeaponType(item, WingedMutantProjectileType::None);

				if (IsFlyEnabled(item))
					item->Animation.TargetState = WMUTANT_STATE_FLY;
				else if (item->TestBits(JointBitType::Touch, WingedMutantJoints[1]))
					item->Animation.TargetState = WMUTANT_STATE_SWIPE_ATTACK;
				else if (AI.bite && AI.distance < WINGED_MUTANT_SWIPE_ATTACK_RANGE)
					item->Animation.TargetState = WMUTANT_STATE_SWIPE_ATTACK;
				else if (AI.bite && AI.distance < WINGED_MUTANT_IDLE_JUMP_ATTACK_RANGE)
					item->Animation.TargetState = WMUTANT_STATE_IDLE_JUMP_ATTACK;
				else if (shoot1)
					item->Animation.TargetState = WMUTANT_STATE_AIM_DART;
				else if (shoot2)
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

				if (shoot1 || shoot2 || IsFlyEnabled(item))
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (creature->Mood == MoodType::Stalk)
				{
					if (AI.distance < WINGED_MUTANT_WALK_RANGE)
					{
						if (AI.zoneNumber == AI.enemyZone ||
							GetRandomControl() < WINGED_MUTANT_UNPOSE_CHANCE)
						{
							item->Animation.TargetState = WMUTANT_STATE_WALK_FORWARD;
						}
					}
					else
						item->Animation.TargetState = WMUTANT_STATE_IDLE;
				}
				else if (creature->Mood == MoodType::Bored &&
					GetRandomControl() < WINGED_MUTANT_UNPOSE_CHANCE)
					item->Animation.TargetState = WMUTANT_STATE_WALK_FORWARD;
				else if (creature->Mood == MoodType::Attack ||
					creature->Mood == MoodType::Escape)
				{
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				}

				break;

			case WMUTANT_STATE_WALK_FORWARD:
				creature->MaxTurn = WINGED_MUTANT_WALK_FORWARD_TURN_RATE_MAX;

				if (shoot1 || shoot2 || IsFlyEnabled(item))
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (creature->Mood == MoodType::Attack || creature->Mood == MoodType::Escape)
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (creature->Mood == MoodType::Bored ||
					(creature->Mood == MoodType::Stalk && AI.zoneNumber != AI.enemyZone))
				{
					if (GetRandomControl() < WINGED_MUTANT_POSE_CHANCE)
						item->Animation.TargetState = WMUTANT_STATE_POSE;
				}
				else if (creature->Mood == MoodType::Stalk && AI.distance > WINGED_MUTANT_WALK_RANGE)
					item->Animation.TargetState = WMUTANT_STATE_IDLE;

				break;

			case WMUTANT_STATE_RUN_FORWARD:
				creature->MaxTurn = WINGED_MUTANT_RUN_FORWARD_TURN_RATE_MAX;

				if (IsFlyEnabled(item))
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (item->TestBits(JointBitType::Touch, WingedMutantJoints[1]))
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (AI.bite && AI.distance < WINGED_MUTANT_IDLE_JUMP_ATTACK_RANGE)
					item->Animation.TargetState = WMUTANT_STATE_IDLE_JUMP_ATTACK;
				else if (AI.bite && AI.distance < WINGED_MUTANT_RUN_JUMP_ATTACK_RANGE)
					item->Animation.TargetState = WMUTANT_STATE_RUN_JUMP_ATTACK;
				else if (shoot1 || shoot2)
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (creature->Mood == MoodType::Bored ||
					(creature->Mood == MoodType::Stalk && AI.distance < WINGED_MUTANT_WALK_RANGE))
				{
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				}

				break;

			case WMUTANT_STATE_IDLE_JUMP_ATTACK:
				if (item->Animation.RequiredState == WMUTANT_STATE_NONE &&
					item->TestBits(JointBitType::Touch, WingedMutantJoints[1]))
				{
					DoDamage(creature->Enemy, WINGED_MUTANT_IDLE_JUMP_ATTACK_DAMAGE);
					CreatureEffect(item, WingedMutantBite, DoBloodSplat);
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				}

				break;

			case WMUTANT_STATE_RUN_JUMP_ATTACK:
				if (item->Animation.RequiredState == WMUTANT_STATE_NONE &&
					item->TestBits(JointBitType::Touch, WingedMutantJoints[1]))
				{
					DoDamage(creature->Enemy, WINGED_MUTANT_RUN_JUMP_ATTACK_DAMAGE);
					CreatureEffect(item, WingedMutantBite, DoBloodSplat);
					item->Animation.TargetState = WMUTANT_STATE_RUN_FORWARD;
				}

				break;

			case WMUTANT_STATE_SWIPE_ATTACK:
				if (item->Animation.RequiredState == WMUTANT_STATE_NONE &&
					item->TestBits(JointBitType::Touch, WingedMutantJoints[1]))
				{
					DoDamage(creature->Enemy, WINGED_MUTANT_SWIPE_ATTACK_DAMAGE);
					CreatureEffect(item, WingedMutantBite, DoBloodSplat);
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				}

				break;

			case WMUTANT_STATE_AIM_DART:
				torso = AI.angle / 2;
				creature->MaxTurn = 0;
				SetWeaponType(item, WingedMutantProjectileType::Dart);

				if (shoot1)
					item->Animation.TargetState = WMUTANT_STATE_SHOOT;
				else
					item->Animation.TargetState = WMUTANT_STATE_IDLE;

				break;

			case WMUTANT_STATE_AIM_BOMB:
				torso = AI.angle / 2;
				creature->MaxTurn = 0;

				SetWeaponType(item, WingedMutantProjectileType::Bomb);
				if (shoot2)
					item->Animation.TargetState = WMUTANT_STATE_SHOOT;
				else
					item->Animation.TargetState = WMUTANT_STATE_IDLE;

				break;

			case WMUTANT_STATE_SHOOT:
				torso = AI.angle / 2;
				creature->MaxTurn = 0;

				switch (GetProjectileType(item))
				{
				case WingedMutantProjectileType::Dart:
					CreatureEffect2(item, WingedMutantShardBite, WINGED_MUTANT_SHARD_VELOCITY, torso, ShardGun);
					break;

				case WingedMutantProjectileType::Bomb:
					CreatureEffect2(item, WingedMutantRocketBite, WINGED_MUTANT_BOMB_VELOCITY, torso, BombGun);
					break;
				}

				SetWeaponType(item, WingedMutantProjectileType::None); // Reset weapon type since it has shot.
				break;

			case WMUTANT_STATE_FLY:
				if (!IsFlyEnabled(item) && item->Pose.Position.y == item->Floor)
					item->Animation.TargetState = WMUTANT_STATE_IDLE; // Switch to ground mode.

				break;
			}
		}

		CreatureJoint(item, 0, torso);
		CreatureJoint(item, 1, head);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
