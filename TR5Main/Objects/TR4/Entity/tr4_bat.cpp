#include "framework.h"
#include "tr4_bat.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/misc.h"
#include "Game/Lara/lara.h"
#include "Game/control/lot.h"
#include "Specific/setup.h"
#include "Specific/trmath.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"

namespace TEN::Entities::TR4
{
	static BITE_INFO BatBite = { 0, 16, 45, 4 };

	#define BAT_ANGLE ANGLE(20.0f)

	constexpr auto BAT_ATTACK_RANGE = SQUARE(CLICK(1));
	constexpr auto BAT_TARGETING_RANGE = SQUARE(SECTOR(5));
	constexpr auto BAT_TARGET_YPOS = 896;
	constexpr auto BAT_DAMAGE = 2;

	enum BatState
	{
		BAT_STATE_NONE = 0,
		BAT_STATE_START = 1,
		BAT_STATE_FLY = 2,
		BAT_STATE_ATTACK = 3,
		BAT_STATE_FALL = 4,
		BAT_STATE_DEATH = 5,
		BAT_STATE_IDLE = 6
	};

	// TODO
	enum BatAnim
	{
		BAT_ANIM_FALL = 3,

		BAT_ANIM_IDLE = 5
	};

	static bool isBatCollideTarget(ItemInfo* item)
	{
		return item->TouchBits >= 0;
	}

	void InitialiseBat(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + BAT_ANIM_IDLE;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.TargetState = BAT_STATE_IDLE;
		item->Animation.ActiveState = BAT_STATE_IDLE;
	}

	void BatControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		ItemInfo* target;
		CreatureInfo* slots;
		int distance, bestdistance;
		short angle;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		angle = 0;

		if (item->HitPoints > 0)
		{
			if (item->AIBits)
				GetAITarget(creature);
			else
			{
				creature->Enemy = LaraItem;

				// NOTE: it seems weird, bat could target any enemy including dogs for example

				// check if voncroy are in range !
				// take in account that the bat will always target voncroy if he exist and triggered !
				// the bat will ignore lara completly !
				/*bestdistance = MAXINT;
				bat->enemy = LaraItem;

				slots = &BaddieSlots[0];
				for (int i = 0; i < NUM_SLOTS; i++, slots++)
				{
					if (slots->itemNum != NO_ITEM && slots->itemNum != itemNumber)
					{
						target = &g_Level.Items[slots->itemNum];
						if (target->objectNumber == ID_VON_CROY)
						{
							int x, z;
							x = target->pos.Position.x - item->pos.Position.x;
							z = target->pos.Position.z - item->pos.Position.z;
							distance = SQUARE(x) + SQUARE(z);
							if (distance < bestdistance)
							{
								bat->enemy = target;
								bestdistance = distance;
							}
						}
					}
				}*/
			}

			// NOTE: chaned from TIMID to VIOLENT, otherwise the bat seems to ignore Lara. 
			// Personally, i feel fine with bat always VIOLENT, but I will inspect also GetCreatureMood and CreatureMood functions
			// for bugs.

			AI_INFO aiInfo;
			CreatureAIInfo(item, &aiInfo);

			GetCreatureMood(item, &aiInfo, VIOLENT);

			if (creature->Flags)
				creature->Mood = MoodType::Escape;

			CreatureMood(item, &aiInfo, VIOLENT);

			angle = CreatureTurn(item, BAT_ANGLE);

			switch (item->Animation.ActiveState)
			{
			case BAT_STATE_IDLE:
				if (aiInfo.distance < BAT_TARGETING_RANGE
					|| item->HitStatus
					|| creature->HurtByLara)
					item->Animation.TargetState = BAT_STATE_START;

				break;

			case BAT_STATE_FLY:
				if (aiInfo.distance < BAT_ATTACK_RANGE || !(GetRandomControl() & 0x3F))
					creature->Flags = 0;

				if (!creature->Flags)
				{
					if (item->TouchBits
						|| creature->Enemy != LaraItem
						&& aiInfo.distance < BAT_ATTACK_RANGE
						&& aiInfo.ahead
						&& abs(item->Pose.Position.y - creature->Enemy->Pose.Position.y) < BAT_TARGET_YPOS)
					{
						item->Animation.TargetState = BAT_STATE_ATTACK;
					}
				}

				break;

			case BAT_STATE_ATTACK:
				if (!creature->Flags
					&& (item->TouchBits
						|| creature->Enemy != LaraItem)
					&& aiInfo.distance < BAT_ATTACK_RANGE
					&& aiInfo.ahead &&
					abs(item->Pose.Position.y - creature->Enemy->Pose.Position.y) < BAT_TARGET_YPOS)
				{
					CreatureEffect(item, &BatBite, DoBloodSplat);
					if (creature->Enemy == LaraItem)
					{
						LaraItem->HitPoints -= BAT_DAMAGE;
						LaraItem->HitStatus = true;
					}
					creature->Flags = 1;
				}
				else
				{
					item->Animation.TargetState = BAT_STATE_FLY;
					creature->Mood = MoodType::Bored;
				}

				break;
			}
		}
		else if (item->Animation.ActiveState == BAT_STATE_ATTACK)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.TargetState = BAT_STATE_FLY;
			item->Animation.ActiveState = BAT_STATE_FLY;
		}
		else
		{
			if (item->Pose.Position.y >= item->Floor)
			{
				item->Animation.TargetState = BAT_STATE_DEATH;
				item->Pose.Position.y = item->Floor;
				item->Animation.Airborne = false;
			}
			else
			{
				item->Animation.Airborne = true;
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + BAT_ANIM_FALL;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.TargetState = BAT_STATE_FALL;
				item->Animation.ActiveState = BAT_STATE_FALL;
				item->Animation.Velocity = 0;
			}
		}

		CreatureAnimation(itemNumber, angle, 0);
	}
}
