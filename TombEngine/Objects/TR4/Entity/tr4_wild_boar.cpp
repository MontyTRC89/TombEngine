#include "framework.h"
#include "tr4_wild_boar.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Game/control/lot.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"

namespace TEN::Entities::TR4
{
	BITE_INFO WildBoatBiteInfo = { 0, 0, 0, 14 };

	constexpr auto WILD_BOAR_ATTACK_DAMAGE = 30;

	constexpr auto WILD_BOAR_ATTACK_RANGE = CLICK(1);

	enum WildBoarState
	{
		BOAR_STATE_NONE = 0,
		BOAR_STATE_IDLE = 1,
		BOAR_STATE_RUN_FORWARD = 2,
		BOAR_STATE_GRAZE = 3,
		BOAR_STATE_ATTACK = 4,
		BOAR_STATE_DEATH = 5,
	};

	enum WildBoarAnim
	{
		BOAR_ANIM_RUN_FORWARD = 0,
		BOAR_ANIM_GRAZE_START = 1,
		BOAR_ANIM_GRAZE_CONTINUE = 2,
		BOAR_ANIM_GRAZE_END = 3,
		BOAR_ANIM_ATTACK = 4,
		BOAR_ANIM_DEATH = 5,
		BOAR_ANIM_IDLE = 6,
		BOAR_ANIM_IDLE_TO_RUN_FORWARD = 7,
		BOAR_ANIM_RUN_FORWARD_TO_IDLE = 8
	};

	void InitialiseWildBoar(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->Animation.AnimNumber = Objects[ID_WILD_BOAR].animIndex + BOAR_ANIM_IDLE;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = BOAR_STATE_IDLE;
		item->Animation.TargetState = BOAR_STATE_IDLE;
	}

	void WildBoarControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short head = 0;
		short neck = 0;
		short tilt = 0;
		short joint0 = 0;
		short joint1 = 0;
		short joint2 = 0;
		short joint3 = 0;

		if (item->HitPoints > 0)
		{
			int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
			int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
			int laraDistance = pow(dx, 2) + pow(dz, 2);

			if (item->AIBits & GUARD)
				GetAITarget(creature);
			else
			{
				creature->Enemy = LaraItem;

				int minDistance = INT_MAX;

				for (int i = 0; i < ActiveCreatures.size(); i++)
				{
					auto* currentItem = ActiveCreatures[i];

					if (currentItem->ItemNumber == NO_ITEM || currentItem->ItemNumber == itemNumber)
						continue;

					auto* target = &g_Level.Items[currentItem->ItemNumber];
					if (target->ObjectNumber != ID_WILD_BOAR)
					{
						int dx2 = target->Pose.Position.x - item->Pose.Position.x;
						int dz2 = target->Pose.Position.z - item->Pose.Position.z;
						int distance = pow(dx2, 2) + pow(dz2, 2);

						if (distance < minDistance &&
							distance < laraDistance)
						{
							creature->Enemy = target;
							minDistance = distance;
						}
					}
				}
			}

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, VIOLENT);

			if (item->Flags)
				creature->Mood = MoodType::Escape;

			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, creature->MaxTurn);

			if (AI.ahead)
			{
				joint1 = AI.angle / 2;
				joint3 = AI.angle / 2;
			}

			switch (item->Animation.ActiveState)
			{
			case BOAR_STATE_IDLE:
				creature->MaxTurn = 0;

				if (AI.ahead && AI.distance || item->Flags)
					item->Animation.TargetState = BOAR_STATE_RUN_FORWARD;
				else if (GetRandomControl() & 0x7F)
				{
					joint1 = AIGuard(creature) / 2;
					joint3 = joint1;
				}
				else
					item->Animation.TargetState = BOAR_STATE_GRAZE;

				break;

			case BOAR_STATE_GRAZE:
				creature->MaxTurn = 0;

				if (AI.ahead && AI.distance)
					item->Animation.TargetState = BOAR_STATE_IDLE;
				else if (!(GetRandomControl() & 0x7F))
					item->Animation.TargetState = BOAR_STATE_IDLE;

				break;

			case BOAR_STATE_RUN_FORWARD:
				if (AI.distance >= pow(SECTOR(2), 2))
				{
					creature->MaxTurn = ANGLE(6.0f);
					item->Flags = 0;
				}
				else
				{
					creature->MaxTurn = ANGLE(3.0f);
					joint0 = -AI.distance;
					joint2 = -AI.distance;
				}

				if (!item->Flags && (AI.distance < pow(WILD_BOAR_ATTACK_RANGE, 2) && AI.bite))
				{
					item->Animation.TargetState = BOAR_STATE_ATTACK;

					DoDamage(creature->Enemy, WILD_BOAR_ATTACK_DAMAGE);
					CreatureEffect2(item, &WildBoatBiteInfo, 3, item->Pose.Orientation.y, DoBloodSplat);
					item->Flags = 1;
				}

				break;

			case BOAR_STATE_ATTACK:
				creature->MaxTurn = 0;
				break;
			}
		}
		else
		{
			item->HitPoints = 0;

			if (item->Animation.ActiveState != BOAR_STATE_DEATH)
			{
				item->Animation.AnimNumber = Objects[ID_WILD_BOAR].animIndex + BOAR_ANIM_DEATH;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = BOAR_STATE_DEATH;
			}
		}

		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);
		CreatureJoint(item, 3, joint3);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
