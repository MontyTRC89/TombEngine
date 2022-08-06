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
	constexpr auto WING_CHARGE_DAMAGE = 100;
	constexpr auto WING_LUNGE_DAMAGE  = 150;
	constexpr auto WING_PUNCH_DAMAGE  = 200;
	constexpr auto WING_PART_DAMAGE	  = 100;

	constexpr auto WING_MUTANT_WALK_RANGE	  = SQUARE(SECTOR(4.5f));
	constexpr auto WING_MUTANT_ATTACK_3_RANGE = SQUARE(CLICK(1.17f));
	constexpr auto WING_MUTANT_ATTACK_2_RANGE = SQUARE(CLICK(2.34f));
	constexpr auto WING_MUTANT_ATTACK_1_RANGE = SQUARE(SECTOR(2.5f));
	constexpr auto WING_MUTANT_ATTACK_RANGE   = SQUARE(SECTOR(3.75f));

	constexpr auto WING_MUTANT_POSE_CHANCE	 = 85;
	constexpr auto WING_MUTANT_UNPOSE_CHANCE = 200;

	constexpr auto WING_MUTANT_FLY_VELOCITY	  = CLICK(1) / 8;
	constexpr auto WING_MUTANT_SHARD_VELOCITY = 250;
	constexpr auto WING_MUTANT_BOMB_VELOCITY  = 220;
	constexpr auto WING_MUTANT_FLY_MODE	   = 0; // itemFlags[0]
	constexpr auto WING_MUTANT_BULLET_MODE = 1; // itemFlags[1]

	#define WMUTANT_STATE_WALK_FORWARD_TURN ANGLE(2.0f)
	#define WMUTANT_STATE_RUN_FORWARD_TURN	ANGLE(6.0f)

	const auto WingMutantBite	= BiteInfo(Vector3(-27.0f, 98.0f, 0.0f), 10);
	const auto WingMutantRocket = BiteInfo(Vector3(51.0f, 213.0f, 0.0f), 14);
	const auto WingMutantShard	= BiteInfo(Vector3(-35.0f, 269.0f, 0.0f), 9);
	const vector<int> WingMutantJoints = { WingMutantShard.meshNum, WingMutantBite.meshNum, WingMutantRocket.meshNum };

	enum WingMutantState
	{
		WMUTANT_STATE_NONE = 0,
		WMUTANT_STATE_IDLE = 1,
		WMUTANT_STATE_WALK_FORWARD = 2,
		WMUTANT_STATE_RUN_FORWARD = 3,
		WMUTANT_STATE_ATTACK_1 = 4,
		WMUTANT_STATE_DEATH = 5,
		WMUTANT_STATE_POSE = 6,
		WMUTANT_STATE_ATTACK_2 = 7,
		WMUTANT_STATE_ATTACK_3 = 8,
		WMUTANT_STATE_AIM_1 = 9,
		WMUTANT_STATE_AIM_2 = 10,
		WMUTANT_STATE_SHOOT = 11,
		WMUTANT_STATE_MUMMY = 12,
		WMUTANT_STATE_FLY = 13,
	};

	enum WingMutantAnim
	{
		WMUTANT_ANIM_MUMMY = 0,
		WMUTANT_ANIM_MUMMY_TO_STOP = 1,
		WMUTANT_ANIM_IDLE = 2,
		WMUTANT_ANIM_IDLE_TO_RUN = 3,
		WMUTANT_ANIM_RUN = 4,
		WMUTANT_ANIM_IDLE_TO_ATTACK_JUMP = 5,
		WMUTANT_ANIM_ATTACK_JUMP,
		WMUTANT_ANIM_IDLE_TO_POSE,
		WMUTANT_ANIM_POSE,
		WMUTANT_ANIM_POSE_TO_STOP,
		WMUTANT_ANIM_POSE_TO_WALK,
		WMUTANT_ANIM_WALK,
		WMUTANT_ANIM_WALK_TO_STOP,
		WMUTANT_ANIM_WALK_TO_POSE,
		WMUTANT_ANIM_RUN_ATTACK_JUMP,
		WMUTANT_ANIM_IDLE_TO_AIM1,
		WMUTANT_ANIM_AIM1,
		WMUTANT_ANIM_SHOOT1, // DART
		WMUTANT_ANIM_AIM1_TO_STOP,
		WMUTANT_ANIM_IDLE_TO_AIM2,
		WMUTANT_ANIM_SHOOT2, // BOMB
		WMUTANT_ANIM_RUN_TO_STOP,
		WMUTANT_ANIM_AIM2_TO_STOP,
		WMUTANT_ANIM_IDLE_TO_FLY,
		WMUTANT_ANIM_FLY,
		WMUTANT_ANIM_FLY_TO_STOP,
		WMUTANT_ANIM_ATTACK // Normal attack (directly to idle)
	};

	// Defines pathfinding type.
	enum class WingMutantPaths
	{
		Ground = 1,
		Flying = 2
	};

	// NOTE: Originally, winged mutants did not have OCB. -- TokyoSU 5/8/2022
	enum class WingMutantOcb
	{
		StartNormal = 0,
		StartFlying = 1,
		StartMummy	= 2
	};

	enum class WingMutantBulletType
	{
		None,
		Dart,
		Bomb
	};

	static bool IsFlyEnabled(ItemInfo* item)
	{
		return item->ItemFlags[WING_MUTANT_FLY_MODE] == true;
	}

	static void SetFlyMode(ItemInfo* item, bool enabled)
	{
		item->ItemFlags[WING_MUTANT_FLY_MODE] = (int)enabled;
	}

	static WingMutantBulletType GetWeaponType(ItemInfo* item)
	{
		return static_cast<WingMutantBulletType>(item->ItemFlags[WING_MUTANT_BULLET_MODE]);
	}

	static void SetWeaponType(ItemInfo* item, WingMutantBulletType type)
	{
		item->ItemFlags[WING_MUTANT_BULLET_MODE] = (int)type;
	}

	static void SwitchPathfinding(CreatureInfo* creature, WingMutantPaths path)
	{
		switch (path)
		{
		case WingMutantPaths::Ground:
			creature->LOT.Step = CLICK(1);
			creature->LOT.Drop = -CLICK(1);
			creature->LOT.Fly = NO_FLYING;
			break;

		case WingMutantPaths::Flying:
			creature->LOT.Step = SECTOR(30);
			creature->LOT.Drop = -SECTOR(30);
			creature->LOT.Fly = WING_MUTANT_FLY_VELOCITY;
			break;
		}
	}

	static void WingedInitOCB(ItemInfo* item, CreatureInfo* creature)
	{
		switch ((WingMutantOcb)item->TriggerFlags)
		{
		case WingMutantOcb::StartFlying:
			SetAnimation(item, WMUTANT_ANIM_FLY);
			SwitchPathfinding(creature, WingMutantPaths::Flying);
			SetFlyMode(item, true);
			item->TriggerFlags = 0;
			break;

		case WingMutantOcb::StartMummy:
			SetAnimation(item, WMUTANT_ANIM_MUMMY);
			SwitchPathfinding(creature, WingMutantPaths::Ground);
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
		SetFlyMode(item, false); // Define flying mode (TRUE = fly)
		SetWeaponType(item, WingMutantBulletType::None); // Define bullet type the entity shoots (two types).
		SetAnimation(item, WMUTANT_ANIM_IDLE);
	}

	// item->ItemFlags[0] will serve as fly detection
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
			SwitchPathfinding(creature, WingMutantPaths::Ground);
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

					SwitchPathfinding(creature, WingMutantPaths::Flying);
					CreatureAIInfo(item, &AI);
				}
				else if ((AI.zoneNumber != AI.enemyZone &&
					!IsFlyEnabled(item) &&
					!shoot1 &&
					!shoot2 &&
					(!AI.ahead || creature->Mood == MoodType::Bored)) ||
					creature->Mood == MoodType::Escape)
				{
					SetFlyMode(item, true);
				}
			}

			if (item->ObjectNumber != ID_MUTANT &&
				Targetable(item, creature, &AI) &&
			   (AI.zoneNumber != AI.enemyZone || AI.distance > WING_MUTANT_ATTACK_RANGE))
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

			switch ((WingMutantState)item->Animation.ActiveState)
			{
			case WMUTANT_STATE_MUMMY:
				creature->MaxTurn = 0;
				if (TargetVisible(item, creature, &AI) || creature->HurtByLara)
					item->Animation.TargetState = WMUTANT_STATE_IDLE;

				break;

			case WMUTANT_STATE_IDLE:
				torso = 0;
				creature->MaxTurn = 0;
				SetWeaponType(item, WingMutantBulletType::None);

				if (IsFlyEnabled(item))
					item->Animation.TargetState = WMUTANT_STATE_FLY;
				else if (item->TestBits(JointBitType::Touch, WingMutantJoints[1]))
					item->Animation.TargetState = WMUTANT_STATE_ATTACK_3;
				else if (AI.bite && AI.distance < WING_MUTANT_ATTACK_3_RANGE)
					item->Animation.TargetState = WMUTANT_STATE_ATTACK_3;
				else if (AI.bite && AI.distance < WING_MUTANT_ATTACK_1_RANGE)
					item->Animation.TargetState = WMUTANT_STATE_ATTACK_1;
				else if (shoot1)
					item->Animation.TargetState = WMUTANT_STATE_AIM_1;
				else if (shoot2)
					item->Animation.TargetState = WMUTANT_STATE_AIM_2;
				else if (creature->Mood == MoodType::Bored ||
					(creature->Mood == MoodType::Stalk && AI.distance < WING_MUTANT_WALK_RANGE))
					item->Animation.TargetState = WMUTANT_STATE_POSE;
				else
					item->Animation.TargetState = WMUTANT_STATE_RUN_FORWARD;

				break;

			case WMUTANT_STATE_POSE:
				head = 0; // Pose has a custom animation for the head.
				creature->MaxTurn = 0;

				if (shoot1 || shoot2 || IsFlyEnabled(item))
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (creature->Mood == MoodType::Stalk)
				{
					if (AI.distance < WING_MUTANT_WALK_RANGE)
					{
						if (AI.zoneNumber == AI.enemyZone ||
							GetRandomControl() < WING_MUTANT_UNPOSE_CHANCE)
							item->Animation.TargetState = WMUTANT_STATE_WALK_FORWARD;
					}
					else
						item->Animation.TargetState = WMUTANT_STATE_IDLE;
				}
				else if (creature->Mood == MoodType::Bored &&
					GetRandomControl() < WING_MUTANT_UNPOSE_CHANCE)
					item->Animation.TargetState = WMUTANT_STATE_WALK_FORWARD;
				else if (creature->Mood == MoodType::Attack ||
					creature->Mood == MoodType::Escape)
					item->Animation.TargetState = WMUTANT_STATE_IDLE;

				break;

			case WMUTANT_STATE_WALK_FORWARD:
				creature->MaxTurn = WMUTANT_STATE_WALK_FORWARD_TURN;

				if (shoot1 || shoot2 || IsFlyEnabled(item))
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (creature->Mood == MoodType::Attack || creature->Mood == MoodType::Escape)
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (creature->Mood == MoodType::Bored ||
					(creature->Mood == MoodType::Stalk && AI.zoneNumber != AI.enemyZone))
				{
					if (GetRandomControl() < WING_MUTANT_POSE_CHANCE)
						item->Animation.TargetState = WMUTANT_STATE_POSE;
				}
				else if (creature->Mood == MoodType::Stalk && AI.distance > WING_MUTANT_WALK_RANGE)
					item->Animation.TargetState = WMUTANT_STATE_IDLE;

				break;

			case WMUTANT_STATE_RUN_FORWARD:
				creature->MaxTurn = WMUTANT_STATE_RUN_FORWARD_TURN;

				if (IsFlyEnabled(item))
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (item->TestBits(JointBitType::Touch, WingMutantJoints[1]))
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (AI.bite && AI.distance < WING_MUTANT_ATTACK_1_RANGE)
					item->Animation.TargetState = WMUTANT_STATE_ATTACK_1;
				else if (AI.bite && AI.distance < WING_MUTANT_ATTACK_2_RANGE)
					item->Animation.TargetState = WMUTANT_STATE_ATTACK_2;
				else if (AI.ahead && AI.distance < WING_MUTANT_ATTACK_2_RANGE)
					item->Animation.TargetState = WMUTANT_STATE_ATTACK_2;
				else if (shoot1 || shoot2)
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				else if (creature->Mood == MoodType::Bored ||
					(creature->Mood == MoodType::Stalk && AI.distance < WING_MUTANT_WALK_RANGE))
				{
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				}

				break;

			case WMUTANT_STATE_ATTACK_1:
				if (item->Animation.RequiredState == WMUTANT_STATE_NONE && item->TestBits(JointBitType::Touch, WingMutantJoints[1]))
				{
					DoDamage(creature->Enemy, WING_LUNGE_DAMAGE);
					CreatureEffect(item, WingMutantBite, DoBloodSplat);
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				}

				break;

			case WMUTANT_STATE_ATTACK_2:
				if (item->Animation.RequiredState == WMUTANT_STATE_NONE && item->TestBits(JointBitType::Touch, WingMutantJoints[1]))
				{
					DoDamage(creature->Enemy, WING_CHARGE_DAMAGE);
					CreatureEffect(item, WingMutantBite, DoBloodSplat);
					item->Animation.TargetState = WMUTANT_STATE_RUN_FORWARD;
				}

				break;

			case WMUTANT_STATE_ATTACK_3:
				if (item->Animation.RequiredState == WMUTANT_STATE_NONE && item->TestBits(JointBitType::Touch, WingMutantJoints[1]))
				{
					DoDamage(creature->Enemy, WING_PUNCH_DAMAGE);
					CreatureEffect(item, WingMutantBite, DoBloodSplat);
					item->Animation.TargetState = WMUTANT_STATE_IDLE;
				}

				break;

			case WMUTANT_STATE_AIM_1:
				torso = AI.angle / 2;
				creature->MaxTurn = 0;
				SetWeaponType(item, WingMutantBulletType::Dart);

				if (shoot1)
					item->Animation.TargetState = WMUTANT_STATE_SHOOT;
				else
					item->Animation.TargetState = WMUTANT_STATE_IDLE;

				break;

			case WMUTANT_STATE_AIM_2:
				torso = AI.angle / 2;
				creature->MaxTurn = 0;

				SetWeaponType(item, WingMutantBulletType::Bomb);
				if (shoot2)
					item->Animation.TargetState = WMUTANT_STATE_SHOOT;
				else
					item->Animation.TargetState = WMUTANT_STATE_IDLE;

				break;

			case WMUTANT_STATE_SHOOT:
				torso = AI.angle / 2;
				creature->MaxTurn = 0;

				switch (GetWeaponType(item))
				{
				case WingMutantBulletType::Dart:
					CreatureEffect2(item, WingMutantShard, WING_MUTANT_SHARD_VELOCITY, torso, ShardGun);
					break;

				case WingMutantBulletType::Bomb:
					CreatureEffect2(item, WingMutantRocket, WING_MUTANT_BOMB_VELOCITY, torso, BombGun);
					break;
				}

				SetWeaponType(item, WingMutantBulletType::None); // Reset weapon type since it has shot.
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
