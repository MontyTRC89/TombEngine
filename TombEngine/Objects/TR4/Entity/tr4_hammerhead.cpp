#include "framework.h"
#include "tr4_hammerhead.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"
#include "Game/itemdata/creature_info.h"
#include "Game/misc.h"

using std::vector;

namespace TEN::Entities::TR4
{
	BITE_INFO HammerheadBite = { 0, 0, 0, 12 };
	const vector<int> HammerheadBiteAttackJoints = { 10, 12, 13 };

	constexpr auto HAMMERHEAD_BITE_ATTACK_DAMAGE = 120;

	enum HammerheadState
	{
		HAMMERHEAD_STATE_IDLE = 0,
		HAMMERHEAD_STATE_SWIM_SLOW = 1,
		HAMMERHEAD_STATE_SWIM_FAST = 2,
		HAMMERHEAD_STATE_IDLE_BITE_ATTACK = 3,
		HAMMERHEAD_STATE_SWIM_FAST_BITE_ATTACK = 4,
		HAMMERHEAD_STATE_DEATH = 5,
		HAMMERHEAD_STATE_KILL = 6
	};

	enum HammerheadAnim
	{
		HAMMERHEAD_ANIM_SWIM_FAST_BITE_ATTACK_LEFT_END = 0,
		HAMMERHEAD_ANIM_BITE_IDLE_BITE_ATTACK_END = 1,
		HAMMERHEAD_ANIM_BITE_IDLE_BITE_ATTACK_CONTINUE = 2,
		HAMMERHEAD_ANIM_SWIM_FAST_BITE_ATTACK_LEFT_CONTINUE = 3,
		HAMMERHEAD_ANIM_DEATH_START = 4,
		HAMMERHEAD_ANIM_DEATH_CONTINUE = 5,
		HAMMERHEAD_ANIM_BITE_IDLE_BITE_ATTACK_START = 6,
		HAMMERHEAD_ANIM_IDLE_TO_SWIM_SLOW = 7,
		HAMMERHEAD_ANIM_IDLE = 8,
		HAMMERHEAD_ANIM_SWIM_SLOW = 9,
		HAMMERHEAD_ANIM_SWIM_FAST = 10,
		HAMMERHEAD_ANIM_SWIM_SLOW_TO_IDLE = 11,
		HAMMERHEAD_ANIM_SWIM_SLOW_TO_SWIM_FAST = 12,
		HAMMERHEAD_ANIM_SWIM_FAST_BITE_ATTACK_LEFT_START = 13,
		HAMMERHEAD_ANIM_SWIM_FAST_TO_IDLE = 14,
		HAMMERHEAD_ANIM_SWIM_FAST_TO_SWIM_SLOW = 15,
		HAMMERHEAD_ANIM_SWIM_FAST_BITE_ATTACK_RIGHT_CONTINUE = 16,
		HAMMERHEAD_ANIM_SWIM_FAST_BITE_ATTACK_RIGHT_END = 17,
		HAMMERHEAD_ANIM_SWIM_FAST_BITE_ATTACK_RIGHT_START = 18,
		HAMMERHEAD_ANIM_KILL = 19
	};

	void InitialiseHammerhead(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + HAMMERHEAD_ANIM_IDLE;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = HAMMERHEAD_STATE_IDLE;
		item->Animation.TargetState = HAMMERHEAD_STATE_IDLE;
	}

	void HammerheadControl(short itemNumber)
	{
		if (CreatureActive(itemNumber))
		{
			auto* item = &g_Level.Items[itemNumber];
			auto* creature = GetCreatureInfo(item);

			if (item->HitPoints > 0)
			{
				if (item->AIBits)
					GetAITarget(creature);
				else if (creature->HurtByLara)
					creature->Enemy = LaraItem;

				AI_INFO AI;
				CreatureAIInfo(item, &AI);

				if (creature->Enemy != LaraItem)
					phd_atan(LaraItem->Pose.Position.z - item->Pose.Position.z, LaraItem->Pose.Position.x - item->Pose.Position.x);

				GetCreatureMood(item, &AI, VIOLENT);
				CreatureMood(item, &AI, VIOLENT);

				short angle = CreatureTurn(item, creature->MaxTurn);

				switch (item->Animation.ActiveState)
				{
				case HAMMERHEAD_STATE_IDLE:
					item->Animation.TargetState = HAMMERHEAD_STATE_SWIM_SLOW;
					creature->Flags = 0;
					break;

				case HAMMERHEAD_STATE_SWIM_SLOW:
					creature->MaxTurn = ANGLE(7.0f);

					if (AI.distance <= pow(SECTOR(1), 2))
					{
						if (AI.distance < pow(682, 2))
							item->Animation.TargetState = HAMMERHEAD_STATE_IDLE_BITE_ATTACK;
					}
					else
						item->Animation.TargetState = HAMMERHEAD_STATE_SWIM_FAST;

					break;

				case HAMMERHEAD_STATE_SWIM_FAST:
					if (AI.distance < pow(SECTOR(1), 2))
						item->Animation.TargetState = HAMMERHEAD_STATE_SWIM_SLOW;

					break;

				case HAMMERHEAD_STATE_IDLE_BITE_ATTACK:
					if (!creature->Flags)
					{
						if (item->TestBits(JointBitType::Touch, HammerheadBiteAttackJoints))
						{
							DoDamage(creature->Enemy, HAMMERHEAD_BITE_ATTACK_DAMAGE);
							CreatureEffect(item, &HammerheadBite, DoBloodSplat);
							creature->Flags = 1;
						}
					}

					break;

				default:
					break;
				}

				CreatureTilt(item, 0);
				CreatureJoint(item, 0, angle * -2);
				CreatureJoint(item, 1, angle * -2);
				CreatureJoint(item, 2, angle * -2);
				CreatureJoint(item, 3, angle * 2);

				// NOTE: in TR2 shark there was a call to CreatureKill with special kill anim
				// Hammerhead seems to not have it in original code but this check is still there as a leftover
				if (item->Animation.ActiveState == HAMMERHEAD_STATE_KILL)
					AnimateItem(item);
				else
				{
					CreatureAnimation(itemNumber, angle, 0);
					CreatureUnderwater(item, 341);
				}
			}
			else
			{
				item->HitPoints = 0;

				if (item->Animation.ActiveState != HAMMERHEAD_STATE_DEATH)
				{
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + HAMMERHEAD_ANIM_DEATH_START;
					item->Animation.ActiveState = HAMMERHEAD_STATE_DEATH;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.FrameNumber].frameBase;
				}

				CreatureFloat(itemNumber);
			}
		}
	}
}
