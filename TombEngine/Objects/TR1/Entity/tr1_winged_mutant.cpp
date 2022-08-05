#include "framework.h"
#include "tr1_winged_mutant.h"

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

	enum WingMutantPaths
	{
		WING_GROUND = 1,
		WING_FLYING = 2
	};

	enum WingMutantOcb
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
		if (item->TriggerFlags != 0)
			printf("TriggerFlags: %d\n", item->TriggerFlags);
		if (item->TriggerFlags & WING_START_FLYING)
		{
			SwitchPathfinding(creature, WING_FLYING);
			SetAnimation(item, WING_FLY);
			item->ItemFlags[0] = TRUE;
			item->TriggerFlags &= ~WING_START_FLYING;
		}
		else if (item->TriggerFlags & WING_START_MUMMY)
		{
			SwitchPathfinding(creature, WING_GROUND);
			SetAnimation(item, WING_MUMMY);
			item->ItemFlags[0] = FALSE;
			item->TriggerFlags &= ~WING_START_MUMMY;
		}
	}

	// NOTE: not exist in the original game ! TokyoSU, 5/8/2022
	void InitialiseWingedMutant(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		item->ItemFlags[0] = FALSE;

		InitialiseCreature(itemNumber);
		SetAnimation(item, WING_STOP);
	}

	// item->ItemFlags[0] will serve as fly detection
	void WingedMutantControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);
		short head = 0;
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
				angle = AI.angle;

			switch (item->Animation.ActiveState)
			{

			}
		}

		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
