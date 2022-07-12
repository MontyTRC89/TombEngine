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

	constexpr auto BAT_DAMAGE = 50;

	constexpr auto BAT_UNFURL_HEIGHT_RANGE = SECTOR(0.87f);
	constexpr auto BAT_ATTACK_RANGE = CLICK(1);
	constexpr auto BAT_AWARE_RANGE = SECTOR(5);

	#define BAT_ANGLE ANGLE(20.0f)

	enum BatState
	{
		BAT_STATE_NONE = 0,
		BAT_STATE_DROP_FROM_CEILING = 1,
		BAT_STATE_FLY = 2,
		BAT_STATE_ATTACK = 3,
		BAT_STATE_DEATH_FALL = 4,
		BAT_STATE_DEATH = 5,
		BAT_STATE_IDLE = 6
	};

	enum BatAnim
	{
		BAT_ANIM_DROP_FROM_CEILING = 0,
		BAT_ANIM_FLY = 1,
		BAT_ANIM_ATTACK = 2,
		BAT_ANIM_DEATH_FALL = 3,
		BAT_ANIM_DEATH = 4,
		BAT_ANIM_IDLE = 5,
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
		item->Animation.ActiveState = BAT_STATE_IDLE;
		item->Animation.TargetState = BAT_STATE_IDLE;
	}

	void BatControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;

		if (item->HitPoints > 0)
		{
			if (item->AIBits)
				GetAITarget(creature);
			else
				creature->Enemy = LaraItem;

			// NOTE: Changed from TIMID to VIOLENT, otherwise the bat seems to ignore Lara. 
			// I feel fine with bat always VIOLENT,
			// but I will also inspect GetCreatureMood and CreatureMood functions for bugs. @TokyoSU

			AI_INFO AI;
			CreatureAIInfo(item, &AI);
			GetCreatureMood(item, &AI, VIOLENT);

			if (creature->Flags)
				creature->Mood = MoodType::Escape;

			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, BAT_ANGLE);

			switch (item->Animation.ActiveState)
			{
			case BAT_STATE_IDLE:
				if (AI.distance < pow(BAT_AWARE_RANGE , 2)|| item->HitStatus || creature->HurtByLara)
					item->Animation.TargetState = BAT_STATE_DROP_FROM_CEILING;

				break;

			case BAT_STATE_FLY:
				if (AI.distance < pow(BAT_ATTACK_RANGE, 2) || !(GetRandomControl() & 0x3F))
					creature->Flags = 0;

				if (!creature->Flags)
				{
					if (item->TouchBits ||
						(creature->Enemy != LaraItem &&
						AI.distance < pow(BAT_ATTACK_RANGE, 2) && AI.ahead &&
						abs(item->Pose.Position.y - creature->Enemy->Pose.Position.y) < BAT_UNFURL_HEIGHT_RANGE))
					{
						item->Animation.TargetState = BAT_STATE_ATTACK;
					}
				}

				break;

			case BAT_STATE_ATTACK:
				if (!creature->Flags &&
					(item->TouchBits || creature->Enemy != LaraItem) &&
					AI.distance < pow(BAT_ATTACK_RANGE, 2) && AI.ahead &&
					abs(item->Pose.Position.y - creature->Enemy->Pose.Position.y) < BAT_UNFURL_HEIGHT_RANGE)
				{
					CreatureEffect(item, &BatBite, DoBloodSplat);
					DoDamage(creature->Enemy, BAT_DAMAGE);

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
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + BAT_ANIM_FLY;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = BAT_STATE_FLY;
			item->Animation.TargetState = BAT_STATE_FLY;
		}
		else
		{
			if (item->Pose.Position.y >= item->Floor)
			{
				item->Animation.TargetState = BAT_STATE_DEATH;
				item->Animation.IsAirborne = false;
				item->Pose.Position.y = item->Floor;
			}
			else
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + BAT_ANIM_DEATH_FALL;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = BAT_STATE_DEATH_FALL;
				item->Animation.TargetState = BAT_STATE_DEATH_FALL;
				item->Animation.IsAirborne = true;
				item->Animation.Velocity = 0;
			}
		}

		CreatureAnimation(itemNumber, angle, 0);
	}
}
