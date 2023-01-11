#include "framework.h"
#include "Objects/TR2/Entity/tr2_spider.h"

#include "Game/animation.h"
#include "Game/control/control.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

namespace TEN::Entities::Creatures::TR2
{
	constexpr auto SMALL_SPIDER_SMALL_JUMP_RANGE = SQUARE(SECTOR(0.2f));
	constexpr auto SMALL_SPIDER_BIG_JUMP_RANGE = SQUARE(SECTOR(0.5f));
	constexpr auto SMALL_SPIDER_TURN_RATE_MAX = ANGLE(8.0f);
	constexpr auto SMALL_SPIDER_DAMAGE = 25;
	constexpr auto SMALL_SPIDER_STOP_CHANCE = 0x100;
	constexpr auto BIG_SPIDER_STOP_RANGE = SQUARE(CLICK(3)) + 15;
	constexpr auto BIG_SPIDER_TURN_RATE_MAX = ANGLE(4.0f);
	constexpr auto BIG_SPIDER_DAMAGE = 100;
	constexpr auto BIG_SPIDER_STOP_CHANCE = 0x200;

	const auto SpiderBite = BiteInfo(Vector3(0.0f, 0.0f, 41.0f), 1);

	enum SmallSpiderState
	{
		// No state 0.
		SPIDER_STATE_STOP = 1,
		SPIDER_STATE_WALK = 2,
		SPIDER_STATE_RUN = 3,
		SPIDER_STATE_STOP_ATTACK = 4,
		SPIDER_STATE_BIG_JUMP_ATTACK = 5, // Not used by big_spider
		SPIDER_STATE_SMALL_JUMP_ATTACK = 6, // Not used by big_spider
		SPIDER_STATE_DIE = 7
	};

	enum SmallSpiderAnims
	{
		SMALL_SPIDER_ANIM_START_STOP = 0,
		SMALL_SPIDER_ANIM_ATTACK_TO_STOP = 1,
		SMALL_SPIDER_ANIM_STOP_JUMP_ATTACK = 2,
		SMALL_SPIDER_ANIM_WALK_JUMP_ATTACK = 3,
		SMALL_SPIDER_ANIM_DEATH = 4,
		SMALL_SPIDER_ANIM_START_STOP_ATTACK = 5,
		SMALL_SPIDER_ANIM_STOP = 6,
		SMALL_SPIDER_ANIM_WALK = 7,
		SMALL_SPIDER_ANIM_RUN = 8
	};

	enum BigSpiderAnims
	{
		BIG_SPIDER_ANIM_START_STOP = 0,
		BIG_SPIDER_ANIM_ATTACK_TO_STOP = 1,
		BIG_SPIDER_ANIM_DEATH = 2,
		BIG_SPIDER_ANIM_START_STOP_ATTACK = 3,
		BIG_SPIDER_ANIM_STOP = 4,
		BIG_SPIDER_ANIM_WALK = 5,
		BIG_SPIDER_ANIM_RUN = 6,
		BIG_SPIDER_WALK_TO_STOP = 7,
		BIG_SPIDER_RUN_TO_STOP = 8
	};

	void DoSpiderBloodEffect(ItemInfo* item)
	{
		auto pos = GetJointPosition(item, SpiderBite.meshNum, SpiderBite.Position);
		DoBloodSplat(pos.x, pos.y, pos.z, 10, item->Pose.Position.y, item->RoomNumber);
	}

	void SmallSpiderControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != SPIDER_STATE_DIE)
			{
				SetAnimation(item, SMALL_SPIDER_ANIM_DEATH);
				ExplodingDeath(itemNumber, 0);
				DisableEntityAI(itemNumber);
				KillItem(itemNumber);
			}
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);
			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case SPIDER_STATE_STOP:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (creature->Mood == MoodType::Bored)
				{
					if (GetRandomControl() < SMALL_SPIDER_STOP_CHANCE)
						item->Animation.TargetState = SPIDER_STATE_WALK;
				}
				else if (AI.ahead && item->TouchBits.TestAny())
					item->Animation.TargetState = SPIDER_STATE_STOP_ATTACK;
				else if (creature->Mood == MoodType::Stalk)
					item->Animation.TargetState = SPIDER_STATE_WALK;
				else if (creature->Mood == MoodType::Escape || creature->Mood == MoodType::Attack)
					item->Animation.TargetState = SPIDER_STATE_RUN;

				break;

			case SPIDER_STATE_WALK:
				creature->MaxTurn = SMALL_SPIDER_TURN_RATE_MAX;
				if (creature->Mood == MoodType::Bored)
				{
					if (GetRandomControl() < SMALL_SPIDER_STOP_CHANCE)
						item->Animation.TargetState = SPIDER_STATE_STOP;
					else
						break;
				}
				else if (creature->Mood == MoodType::Escape || creature->Mood == MoodType::Attack)
					item->Animation.TargetState = SPIDER_STATE_RUN;

				break;

			case SPIDER_STATE_RUN:
				creature->MaxTurn = SMALL_SPIDER_TURN_RATE_MAX;
				creature->Flags = 0;

				if (creature->Mood == MoodType::Bored || creature->Mood == MoodType::Stalk)
					item->Animation.TargetState = SPIDER_STATE_WALK;
				else if (AI.ahead && item->TouchBits.TestAny())
					item->Animation.TargetState = SPIDER_STATE_STOP;
				else if (AI.ahead && AI.distance < SMALL_SPIDER_SMALL_JUMP_RANGE)
					item->Animation.TargetState = SPIDER_STATE_SMALL_JUMP_ATTACK;
				else if (AI.ahead && AI.distance < SMALL_SPIDER_BIG_JUMP_RANGE)
					item->Animation.TargetState = SPIDER_STATE_BIG_JUMP_ATTACK;

				break;

			case SPIDER_STATE_STOP_ATTACK:
			case SPIDER_STATE_BIG_JUMP_ATTACK:
			case SPIDER_STATE_SMALL_JUMP_ATTACK:
				creature->MaxTurn = 0;
				if (!creature->Flags && item->TouchBits.TestAny())
				{
					DoSpiderBloodEffect(item);
					DoDamage(creature->Enemy, SMALL_SPIDER_DAMAGE);
					creature->Flags = 1;
				}

				break;
			}
		}

		if (item->Animation.ActiveState == SPIDER_STATE_STOP ||
			item->Animation.ActiveState == SPIDER_STATE_WALK ||
			item->Animation.ActiveState == SPIDER_STATE_RUN)
		{
			if (CreatureVault(itemNumber, angle, 2, 0) == 2) // only 2 block climb.
			{
				creature->MaxTurn = 0;
				SetAnimation(item, SMALL_SPIDER_ANIM_WALK_JUMP_ATTACK); // serve as climb...
				return;
			}
		}
		else
		{
			CreatureAnimation(itemNumber, angle, 0);
		}
	}

	void BigSpiderControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;

		AI_INFO AI;
		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != 7)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 2;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = 7;
			}
		}
		else
		{
			CreatureAIInfo(item, &AI);
			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);
			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case SPIDER_STATE_STOP:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (creature->Mood == MoodType::Bored)
				{
					if (GetRandomControl() < BIG_SPIDER_STOP_CHANCE)
						item->Animation.TargetState = SPIDER_STATE_STOP;
					else
						break;
				}
				else if (AI.ahead && AI.distance < BIG_SPIDER_STOP_RANGE)
					item->Animation.TargetState = SPIDER_STATE_STOP_ATTACK;
				else if (creature->Mood == MoodType::Stalk)
					item->Animation.TargetState = SPIDER_STATE_WALK;
				else if (creature->Mood == MoodType::Escape || creature->Mood == MoodType::Attack)
					item->Animation.TargetState = SPIDER_STATE_RUN;

				break;

			case SPIDER_STATE_WALK:
				creature->MaxTurn = BIG_SPIDER_TURN_RATE_MAX;
				if (creature->Mood == MoodType::Bored)
				{
					if (GetRandomControl() < BIG_SPIDER_STOP_CHANCE)
						item->Animation.TargetState = SPIDER_STATE_STOP;
					else
						break;
				}
				else if (creature->Mood == MoodType::Escape || creature->Mood == MoodType::Attack)
					item->Animation.TargetState = SPIDER_STATE_RUN;

				break;

			case SPIDER_STATE_RUN:
				creature->MaxTurn = BIG_SPIDER_TURN_RATE_MAX;
				creature->Flags = 0;

				if (creature->Mood == MoodType::Bored || creature->Mood == MoodType::Stalk)
					item->Animation.TargetState = SPIDER_STATE_WALK;
				else if (AI.ahead && item->TouchBits.TestAny())
					item->Animation.TargetState = SPIDER_STATE_STOP;

				break;

			case SPIDER_STATE_STOP_ATTACK:
				creature->MaxTurn = 0;
				if (!creature->Flags && item->TouchBits.TestAny())
				{
					DoSpiderBloodEffect(item);
					DoDamage(creature->Enemy, BIG_SPIDER_DAMAGE);
					creature->Flags = 1;
				}

				break;
			}
		}

		CreatureAnimation(itemNumber, angle, 0);
	}
}
