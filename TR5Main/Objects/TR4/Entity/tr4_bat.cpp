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

	constexpr auto BAT_ANIM_FALLING = 3;
	constexpr auto BAT_ANIM_IDLE = 5;
	constexpr auto BAT_ATTACK_RANGE = SQUARE(CLICK(1));
	constexpr auto BAT_TARGETING_RANGE = SQUARE(SECTOR(5));
	constexpr auto BAT_TARGET_YPOS = 896;
	constexpr auto BAT_DAMAGE = 2;

	enum BatState
	{
		BAT_STATE_NONE,
		BAT_STATE_START,
		BAT_STATE_FLY,
		BAT_STATE_ATTACK,
		BAT_STATE_FALL,
		BAT_STATE_DEATH,
		BAT_STATE_IDLE
	};

	// TODO
	enum BatAnim
	{

	};

	static bool isBatCollideTarget(ITEM_INFO* item)
	{
		return item->TouchBits >= 0;
	}

	void InitialiseBat(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);

		item->AnimNumber = Objects[item->ObjectNumber].animIndex + BAT_ANIM_IDLE;
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		item->TargetState = BAT_STATE_IDLE;
		item->ActiveState = BAT_STATE_IDLE;
	}

	void BatControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		ITEM_INFO* target;
		CreatureInfo* slots;
		int distance, bestdistance;
		short angle;

		auto* item = &g_Level.Items[itemNumber];
		auto* info = GetCreatureInfo(item);

		angle = 0;

		if (item->HitPoints > 0)
		{
			if (item->AIBits)
				GetAITarget(info);
			else
			{
				info->Enemy = LaraItem;

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
							x = target->pos.xPos - item->pos.xPos;
							z = target->pos.zPos - item->pos.zPos;
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

			if (info->Flags)
				info->Mood = MoodType::Escape;

			CreatureMood(item, &aiInfo, VIOLENT);

			angle = CreatureTurn(item, BAT_ANGLE);

			switch (item->ActiveState)
			{
			case BAT_STATE_IDLE:
				if (aiInfo.distance < BAT_TARGETING_RANGE
					|| item->HitStatus
					|| info->HurtByLara)
					item->TargetState = BAT_STATE_START;

				break;

			case BAT_STATE_FLY:
				if (aiInfo.distance < BAT_ATTACK_RANGE || !(GetRandomControl() & 0x3F))
					info->Flags = 0;

				if (!info->Flags)
				{
					if (item->TouchBits
						|| info->Enemy != LaraItem
						&& aiInfo.distance < BAT_ATTACK_RANGE
						&& aiInfo.ahead
						&& abs(item->Position.yPos - info->Enemy->Position.yPos) < BAT_TARGET_YPOS)
					{
						item->TargetState = BAT_STATE_ATTACK;
					}
				}

				break;

			case BAT_STATE_ATTACK:
				if (!info->Flags
					&& (item->TouchBits
						|| info->Enemy != LaraItem)
					&& aiInfo.distance < BAT_ATTACK_RANGE
					&& aiInfo.ahead &&
					abs(item->Position.yPos - info->Enemy->Position.yPos) < BAT_TARGET_YPOS)
				{
					CreatureEffect(item, &BatBite, DoBloodSplat);
					if (info->Enemy == LaraItem)
					{
						LaraItem->HitPoints -= BAT_DAMAGE;
						LaraItem->HitStatus = true;
					}
					info->Flags = 1;
				}
				else
				{
					item->TargetState = BAT_STATE_FLY;
					info->Mood = MoodType::Bored;
				}

				break;
			}
		}
		else if (item->ActiveState == BAT_STATE_ATTACK)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->TargetState = BAT_STATE_FLY;
			item->ActiveState = BAT_STATE_FLY;
		}
		else
		{
			if (item->Position.yPos >= item->Floor)
			{
				item->TargetState = BAT_STATE_DEATH;
				item->Position.yPos = item->Floor;
				item->Airborne = false;
			}
			else
			{
				item->Airborne = true;
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + BAT_ANIM_FALLING;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				item->TargetState = BAT_STATE_FALL;
				item->ActiveState = BAT_STATE_FALL;
				item->Velocity = 0;
			}
		}

		CreatureAnimation(itemNumber, angle, 0);
	}
}
