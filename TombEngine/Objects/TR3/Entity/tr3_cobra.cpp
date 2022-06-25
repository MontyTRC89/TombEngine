#include "framework.h"
#include "Objects/TR3/Entity/tr3_cobra.h"

#include "Game/control/box.h"
#include "Game/itemdata/creature_info.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using std::vector;

namespace TEN::Entities::TR3
{
	BITE_INFO CobraBite = { 0, 0, 0, 13 };
	const vector<int> CobraAttackJoints = { 13 };

	constexpr auto COBRA_BITE_ATTACK_DAMAGE = 80;
	constexpr auto COBRA_BITE_POISON_POTENCY = 1;

	constexpr auto COBRA_ATTACK_RANGE = SECTOR(1);
	constexpr auto COBRA_AWARE_RANGE = SECTOR(1.5f);
	constexpr auto COBRA_SLEEP_RANGE = SECTOR(2.5f);

	constexpr auto COBRA_SLEEP_FRAME = 45;

	constexpr auto PLAYER_DISTURB_VELOCITY = 15;

	enum CobraState
	{
		COBRA_STATE_WAKE_UP = 0,
		COBRA_STATE_IDLE = 1,
		COBRA_STATE_ATTACK = 2,
		COBRA_STATE_SLEEP = 3,
		COBRA_STATE_DEATH = 4
	};

	enum CobraAnim
	{
		COBRA_ANIM_IDLE = 0,
		COBRA_ANIM_WAKE_UP = 1,
		COBRA_ANIM_IDLE_TO_SLEEP = 2,
		COBRA_ANIM_BITE_ATTACK = 3,
		COBRA_ANIM_DEATH = 4
	};

	void InitialiseCobra(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + COBRA_ANIM_IDLE_TO_SLEEP;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase + COBRA_SLEEP_FRAME;
		item->Animation.ActiveState = COBRA_STATE_SLEEP; 
		item->Animation.TargetState = COBRA_STATE_SLEEP;
		item->ItemFlags[2] = item->HitStatus;
	}

	void CobraControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* info = GetCreatureInfo(item);

		short head = 0;
		short angle = 0;
		short tilt = 0;

		if (item->HitPoints <= 0 && item->HitPoints != NOT_TARGETABLE)
		{
			if (item->Animation.ActiveState != COBRA_STATE_DEATH)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + COBRA_ANIM_DEATH;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = COBRA_STATE_DEATH;
			}
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			AI.angle += ANGLE(16.8f);

			GetCreatureMood(item, &AI, 1);
			CreatureMood(item, &AI, 1);

			info->Target.x = LaraItem->Pose.Position.x;
			info->Target.z = LaraItem->Pose.Position.z;
			angle = CreatureTurn(item, info->MaxTurn);

			if (AI.ahead)
				head = AI.angle;

			if (abs(AI.angle) < ANGLE(10.0f))
				item->Pose.Orientation.y += AI.angle;
			else if (AI.angle < 0)
				item->Pose.Orientation.y -= ANGLE(10.0f);
			else
				item->Pose.Orientation.y += ANGLE(10.0f);

			switch (item->Animation.ActiveState)
			{
			case COBRA_STATE_IDLE:
				info->Flags = 0;

				if (AI.distance > pow(COBRA_SLEEP_RANGE, 2))
					item->Animation.TargetState = COBRA_STATE_SLEEP;
				else if (LaraItem->HitPoints > 0 &&
					((AI.ahead && AI.distance < pow(COBRA_ATTACK_RANGE, 2)) || item->HitStatus || LaraItem->Animation.Velocity > PLAYER_DISTURB_VELOCITY))
				{
					item->Animation.TargetState = COBRA_STATE_ATTACK;
				}

				break;

			case COBRA_STATE_SLEEP:
				info->Flags = 0;

				if (item->HitPoints != NOT_TARGETABLE)
				{
					item->HitPoints = NOT_TARGETABLE;
					item->ItemFlags[2] = item->HitPoints;
				}
				if (AI.distance < pow(COBRA_AWARE_RANGE, 2) && LaraItem->HitPoints > 0)
				{
					item->Animation.TargetState = COBRA_STATE_WAKE_UP;
					item->HitPoints = item->ItemFlags[2];
				}

				break;

			case COBRA_STATE_ATTACK:
				if (info->Flags != 1 && item->TestBits(JointBitType::Touch, CobraAttackJoints))
				{
					CreatureEffect(item, &CobraBite, DoBloodSplat);
					DoDamage(info->Enemy, COBRA_BITE_ATTACK_DAMAGE);
					info->Flags = 1;

					if (info->Enemy->IsLara())
						GetLaraInfo(info->Enemy)->PoisonPotency += COBRA_BITE_POISON_POTENCY;
				}

				break;

			case COBRA_STATE_WAKE_UP:
				item->HitPoints = item->ItemFlags[2];
				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, head / 2);
		CreatureJoint(item, 1, head / 2);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
