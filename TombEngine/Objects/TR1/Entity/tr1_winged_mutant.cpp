#include "framework.h"
#include "Objects/TR1/Entity/tr1_winged_mutant.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/effects/tomb4fx.h"
#include "Game/misc.h"
#include "Game/missile.h"
#include "Game/people.h"
#include "Game/control/lot.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/trmath.h"

using std::vector;

namespace TEN::Entities::TR1
{
	constexpr auto WING_FLY_SPEED = CLICK(1) / 8;
	constexpr auto WING_CHARGE_DAMAGE = 100;
	constexpr auto WING_LUNGE_DAMAGE = 150;
	constexpr auto WING_PUNCH_DAMAGE = 200;
	constexpr auto WING_PART_DAMAGE = 100;
	constexpr auto WING_POSE_CHANCE = 85;
	constexpr auto WING_UNPOSE_CHANCE = 200;
	constexpr auto WING_FLY_MODE = 0; // itemFlags[0]
	constexpr auto WING_BULLET_MODE = 1; // itemFlags[1]
	constexpr auto WING_WALK_RANGE = SQUARE(SECTOR(4.5f));
	constexpr auto WING_ATTACK3_RANGE = SQUARE(CLICK(1.172f));
	constexpr auto WING_ATTACK2_RANGE = SQUARE(CLICK(2.344f));
	constexpr auto WING_ATTACK1_RANGE = SQUARE(SECTOR(2.5f));
	constexpr auto WING_ATTACK_RANGE = SQUARE(SECTOR(3.75f));
	constexpr auto WING_SHARD_VELOCITY = 250;
	constexpr auto WING_BOMB_VELOCITY = 220;

	#define WING_WALK_TURN ANGLE(2.0f)
	#define WING_RUN_TURN ANGLE(6.0f)

	const auto WingMutantBite = BiteInfo(Vector3(-27.0f, 98.0f, 0.0f), 10);
	const auto WingMutantRocket = BiteInfo(Vector3(51.0f, 213.0f, 0.0f), 14);
	const auto WingMutantShard = BiteInfo(Vector3(-35.0f, 269.0f, 0.0f), 9);
	const vector<int> WingMutantJoints = { WingMutantShard.meshNum, WingMutantBite.meshNum, WingMutantRocket.meshNum };

	enum WingedMutantState
	{
		WING_EMPTY = 0,
		WING_STOP = 1,
		WING_WALK = 2,
		WING_RUN = 3,
		WING_ATTACK1 = 4,
		WING_DEATH = 5,
		WING_POSE = 6,
		WING_ATTACK2 = 7,
		WING_ATTACK3 = 8,
		WING_AIM1 = 9,
		WING_AIM2 = 10,
		WING_SHOOT = 11,
		WING_MUMMY = 12,
		WING_FLY = 13,
	};

	enum WingedMutantAnims : int
	{
		AWING_MUMMY = 0,
		AWING_MUMMY_TO_STOP,
		AWING_STOP,
		AWING_STOP_TO_RUN,
		AWING_RUN,
		AWING_STOP_TO_ATTACK_JUMP,
		AWING_ATTACK_JUMP,
		AWING_STOP_TO_POSE,
		AWING_POSE,
		AWING_POSE_TO_STOP,
		AWING_POSE_TO_WALK,
		AWING_WALK,
		AWING_WALK_TO_STOP,
		AWING_WALK_TO_POSE,
		AWING_RUN_ATTACK_JUMP,
		AWING_STOP_TO_AIM1,
		AWING_AIM1,
		AWING_SHOOT1, // DART
		AWING_AIM1_TO_STOP,
		AWING_STOP_TO_AIM2,
		AWING_SHOOT2, // EXPLODE SPHERE
		AWING_RUN_TO_STOP,
		AWING_AIM2_TO_STOP,
		AWING_STOP_TO_FLY,
		AWING_FLY,
		AWING_FLY_TO_STOP,
		AWING_ATTACK // normal attack (directly to stop)
	};

	enum class WingMutantPaths // used to define pathfinding type
	{
		WING_GROUND = 1,
		WING_FLYING = 2
	};

	enum class WingMutantOcb // NOTE: originally wing mutant don't have ocb, i've added it ! TokyoSU, 5/8/2022
	{
		WING_START_NORMAL = 0,
		WING_START_FLYING = 1,
		WING_START_MUMMY = 2
	};

	enum class WingMutantBulletType
	{
		WING_BULLET_NONE,
		WING_BULLET_DART,
		WING_BULLET_EXPLODE
	};

	static bool IsFlyEnabled(ItemInfo* item)
	{
		return item->ItemFlags[WING_FLY_MODE] == TRUE;
	}

	static void SetFlyMode(ItemInfo* item, bool enabled)
	{
		item->ItemFlags[WING_FLY_MODE] = int(enabled);
	}

	static WingMutantBulletType GetWeaponType(ItemInfo* item)
	{
		return static_cast<WingMutantBulletType>(item->ItemFlags[WING_BULLET_MODE]);
	}

	static void SetWeaponType(ItemInfo* item, WingMutantBulletType type)
	{
		item->ItemFlags[WING_BULLET_MODE] = static_cast<int>(type);
	}

	static void SwitchPathfinding(CreatureInfo* creature, WingMutantPaths path)
	{
		switch (path)
		{
		case WingMutantPaths::WING_GROUND:
			creature->LOT.Step = CLICK(1);
			creature->LOT.Drop = -CLICK(1);
			creature->LOT.Fly = NO_FLYING;
			break;
		case WingMutantPaths::WING_FLYING:
			creature->LOT.Step = SECTOR(30);
			creature->LOT.Drop = -SECTOR(30);
			creature->LOT.Fly = WING_FLY_SPEED;
			break;
		}
	}

	static void WingedInitOCB(ItemInfo* item, CreatureInfo* creature)
	{
		switch ((WingMutantOcb)item->TriggerFlags)
		{
		case WingMutantOcb::WING_START_FLYING:
			SwitchPathfinding(creature, WingMutantPaths::WING_FLYING);
			SetAnimation(item, AWING_FLY);
			SetFlyMode(item, true);
			item->TriggerFlags = 0;
			break;
		case WingMutantOcb::WING_START_MUMMY:
			SwitchPathfinding(creature, WingMutantPaths::WING_GROUND);
			SetAnimation(item, AWING_MUMMY);
			SetFlyMode(item, false);
			item->TriggerFlags = 0;
			break;
		}
	}

	// NOTE: not exist in the original game ! TokyoSU, 5/8/2022
	void InitialiseWingedMutant(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		InitialiseCreature(itemNumber);
		SetFlyMode(item, false); // define flying mode (TRUE = fly)
		SetWeaponType(item, WingMutantBulletType::WING_BULLET_NONE); // define bullet that the entity shoot (there are 2)
		SetAnimation(item, AWING_STOP);
	}

	// item->ItemFlags[0] will serve as fly detection
	void WingedMutantControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);
		short head = 0;
		short torso = 0; // only when shooting !
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
			SwitchPathfinding(creature, WingMutantPaths::WING_GROUND);
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;
			else
			{
				head = 0;
				torso = 0;
			}

			GetCreatureMood(item, &AI, IsFlyEnabled(item)); // true = FLY = AGGRESIVE !
			CreatureMood(item, &AI, IsFlyEnabled(item));
			angle = CreatureTurn(item, creature->MaxTurn);

			bool shoot1 = false;
			bool shoot2 = false;

			if (item->ObjectNumber == ID_WINGED_MUMMY)
			{
				if (item->Animation.ActiveState == WING_FLY)
				{
					if (IsFlyEnabled(item) &&
						creature->Mood != MoodType::Escape &&
						AI.zoneNumber == AI.enemyZone)
					{
						SetFlyMode(item, false);
					}

					SwitchPathfinding(creature, WingMutantPaths::WING_FLYING);
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
			   (AI.zoneNumber != AI.enemyZone || AI.distance > WING_ATTACK_RANGE))
			{
				if (AI.angle > 0 && AI.angle < ANGLE(45.0f))
					shoot1 = true;
				else if (AI.angle < 0 && angle > -ANGLE(45.0f))
					shoot2 = true;
			}
			else // if not targetable:
			{
				shoot1 = false;
				shoot2 = false;
			}

			switch ((WingedMutantState)item->Animation.ActiveState)
			{
			case WING_MUMMY:
				creature->MaxTurn = 0;
				if (TargetVisible(item, creature, &AI) || creature->HurtByLara)
					item->Animation.TargetState = WING_STOP;

				break;

			case WING_STOP:
				torso = 0;
				creature->MaxTurn = 0;
				SetWeaponType(item, WingMutantBulletType::WING_BULLET_NONE);

				if (IsFlyEnabled(item))
					item->Animation.TargetState = WING_FLY;
				else if (item->TestBits(JointBitType::Touch, WingMutantJoints[1]))
					item->Animation.TargetState = WING_ATTACK3;
				else if (AI.bite && AI.distance < WING_ATTACK3_RANGE)
					item->Animation.TargetState = WING_ATTACK3;
				else if (AI.bite && AI.distance < WING_ATTACK1_RANGE)
					item->Animation.TargetState = WING_ATTACK1;
				else if (shoot1)
					item->Animation.TargetState = WING_AIM1;
				else if (shoot2)
					item->Animation.TargetState = WING_AIM2;
				else if (creature->Mood == MoodType::Bored ||
					(creature->Mood == MoodType::Stalk && AI.distance < WING_WALK_RANGE))
					item->Animation.TargetState = WING_POSE;
				else
					item->Animation.TargetState = WING_RUN;

				break;

			case WING_POSE:
				creature->MaxTurn = 0;
				head = 0; // pose have custom animation for the head !
				if (shoot1 || shoot2 || IsFlyEnabled(item))
					item->Animation.TargetState = WING_STOP;
				else if (creature->Mood == MoodType::Stalk)
				{
					if (AI.distance < WING_WALK_RANGE)
					{
						if (AI.zoneNumber == AI.enemyZone ||
							GetRandomControl() < WING_UNPOSE_CHANCE)
							item->Animation.TargetState = WING_WALK;
					}
					else
						item->Animation.TargetState = WING_STOP;
				}
				else if (creature->Mood == MoodType::Bored &&
					GetRandomControl() < WING_UNPOSE_CHANCE)
					item->Animation.TargetState = WING_WALK;
				else if (creature->Mood == MoodType::Attack ||
					creature->Mood == MoodType::Escape)
					item->Animation.TargetState = WING_STOP;

				break;

			case WING_WALK:
				creature->MaxTurn = WING_WALK_TURN;
				if (shoot1 || shoot2 || IsFlyEnabled(item))
					item->Animation.TargetState = WING_STOP;
				else if (creature->Mood == MoodType::Attack || creature->Mood == MoodType::Escape)
					item->Animation.TargetState = WING_STOP;
				else if (creature->Mood == MoodType::Bored ||
					(creature->Mood == MoodType::Stalk && AI.zoneNumber != AI.enemyZone))
				{
					if (GetRandomControl() < WING_POSE_CHANCE)
						item->Animation.TargetState = WING_POSE;
				}
				else if (creature->Mood == MoodType::Stalk && AI.distance > WING_WALK_RANGE)
					item->Animation.TargetState = WING_STOP;

				break;

			case WING_RUN:
				creature->MaxTurn = WING_RUN_TURN;
				if (IsFlyEnabled(item))
					item->Animation.TargetState = WING_STOP;
				else if (item->TestBits(JointBitType::Touch, WingMutantJoints[1]))
					item->Animation.TargetState = WING_STOP;
				else if (AI.bite && AI.distance < WING_ATTACK1_RANGE)
					item->Animation.TargetState = WING_ATTACK1;
				else if (AI.bite && AI.distance < WING_ATTACK2_RANGE)
					item->Animation.TargetState = WING_ATTACK2;
				else if (AI.ahead && AI.distance < WING_ATTACK2_RANGE)
					item->Animation.TargetState = WING_ATTACK2;
				else if (shoot1 || shoot2)
					item->Animation.TargetState = WING_STOP;
				else if (creature->Mood == MoodType::Bored ||
					(creature->Mood == MoodType::Stalk && AI.distance < WING_WALK_RANGE))
				{
					item->Animation.TargetState = WING_STOP;
				}

				break;

			case WING_ATTACK1:
				if (item->Animation.RequiredState == WING_EMPTY && item->TestBits(JointBitType::Touch, WingMutantJoints[1]))
				{
					CreatureEffect(item, WingMutantBite, DoBloodSplat);
					DoDamage(creature->Enemy, WING_LUNGE_DAMAGE);
					item->Animation.TargetState = WING_STOP;
				}
				break;

			case WING_ATTACK2:
				if (item->Animation.RequiredState == WING_EMPTY && item->TestBits(JointBitType::Touch, WingMutantJoints[1]))
				{
					CreatureEffect(item, WingMutantBite, DoBloodSplat);
					DoDamage(creature->Enemy, WING_CHARGE_DAMAGE);
					item->Animation.TargetState = WING_RUN;
				}
				break;

			case WING_ATTACK3:
				if (item->Animation.RequiredState == WING_EMPTY && item->TestBits(JointBitType::Touch, WingMutantJoints[1]))
				{
					CreatureEffect(item, WingMutantBite, DoBloodSplat);
					DoDamage(creature->Enemy, WING_PUNCH_DAMAGE);
					item->Animation.TargetState = WING_STOP;
				}
				break;

			case WING_AIM1:
				torso = AI.angle / 2;
				creature->MaxTurn = 0;
				SetWeaponType(item, WingMutantBulletType::WING_BULLET_DART);
				if (shoot1)
					item->Animation.TargetState = WING_SHOOT;
				else
					item->Animation.TargetState = WING_STOP;

				break;

			case WING_AIM2:
				torso = AI.angle / 2;
				creature->MaxTurn = 0;
				SetWeaponType(item, WingMutantBulletType::WING_BULLET_EXPLODE);
				if (shoot2)
					item->Animation.TargetState = WING_SHOOT;
				else
					item->Animation.TargetState = WING_STOP;

				break;

			case WING_SHOOT:
				torso = AI.angle / 2;
				creature->MaxTurn = 0;
				switch (GetWeaponType(item))
				{
				case WingMutantBulletType::WING_BULLET_DART:
					CreatureEffect2(item, WingMutantShard, WING_SHARD_VELOCITY, torso, ShardGun);
					break;
				case WingMutantBulletType::WING_BULLET_EXPLODE:
					CreatureEffect2(item, WingMutantRocket, WING_BOMB_VELOCITY, torso, BombGun);
					break;
				}
				SetWeaponType(item, WingMutantBulletType::WING_BULLET_NONE); // reset weapon type since it shooted !

				break;

			case WING_FLY:
				if (!IsFlyEnabled(item) && item->Pose.Position.y == item->Floor)
					item->Animation.TargetState = WING_STOP; // switch to ground mode

				break;
			}
		}

		CreatureJoint(item, 0, torso);
		CreatureJoint(item, 1, head);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
