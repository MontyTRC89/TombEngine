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

namespace TEN::Entities::TR1
{
	constexpr auto WING_FLY_SPEED = CLICK(1) / 8;
	constexpr auto WING_PUNCH_DAMAGE = 150;
	constexpr auto WING_JUMP_DAMAGE = 100;

	#define WING_WALK_TURN ANGLE(2.0f)
	#define WING_RUN_TURN ANGLE(6.0f)

	const auto WingMutantBite = BiteInfo(Vector3(-27.0f, 98.0f, 0.0f), 10);
	const auto WingMutantRocket = BiteInfo(Vector3(51.0f, 213.0f, 0.0f), 14);
	const auto WingMutantShard = BiteInfo(Vector3(-35.0f, 269.0f, 0.0f), 9);

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

	enum WingedMutantAnims
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

	enum WingMutantPaths // used to define pathfinding type
	{
		WING_GROUND = 1,
		WING_FLYING = 2
	};

	enum WingMutantOcb // NOTE: originally wing mutant don't have ocb, i've added it !
	{
		WING_START_NORMAL = 0,
		WING_START_FLYING = 1,
		WING_START_MUMMY = 2
	};

	static void SwitchPathfinding(CreatureInfo* creature, WingMutantPaths path)
	{
		switch (path)
		{
		case WING_GROUND:
			creature->LOT.Step = CLICK(1);
			creature->LOT.Drop = -CLICK(1);
			creature->LOT.Fly = NO_FLYING;
			break;
		case WING_FLYING:
			creature->LOT.Step = SECTOR(30);
			creature->LOT.Drop = -SECTOR(30);
			creature->LOT.Fly = WING_FLY_SPEED;
			break;
		}
	}

	static void WingedInitOCB(ItemInfo* item, CreatureInfo* creature)
	{
		if (item->TriggerFlags == WING_START_FLYING)
		{
			SwitchPathfinding(creature, WING_FLYING);
			SetAnimation(item, AWING_FLY);
			item->ItemFlags[0] = TRUE;
			item->TriggerFlags = 0;
		}
		else if (item->TriggerFlags == WING_START_MUMMY)
		{
			SwitchPathfinding(creature, WING_GROUND);
			SetAnimation(item, AWING_MUMMY);
			item->ItemFlags[0] = FALSE;
			item->TriggerFlags = 0;
		}
	}

	// NOTE: not exist in the original game ! TokyoSU, 5/8/2022
	void InitialiseWingedMutant(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		item->ItemFlags[0] = FALSE; // define flying mode ! (true: fly)
		InitialiseCreature(itemNumber);
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
			ExplodingDeath(itemNumber, BODY_EXPLODE);
			SoundEffect(SFX_TR1_ATLANTEAN_DEATH, &item->Pose);
			DisableEntityAI(itemNumber);
			KillItem(itemNumber);
			item->Status = ITEM_DEACTIVATED;
			return;
		}
		else
		{
			AI_INFO AI;
			SwitchPathfinding(creature, item->ItemFlags[0] ? WING_FLYING : WING_GROUND);
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;

			GetCreatureMood(item, &AI, item->ItemFlags[0]); // true = FLY = AGGRESIVE !
			CreatureMood(item, &AI, item->ItemFlags[0]);
			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case WING_MUMMY:
				if (TargetVisible(item, creature, &AI) || creature->HurtByLara)
					item->Animation.TargetState = WING_STOP;
				break;


			}
		}

		CreatureJoint(item, 0, -head);
		CreatureJoint(item, 1, torso);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
