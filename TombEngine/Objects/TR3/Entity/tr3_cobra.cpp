#include "framework.h"
#include "Objects/TR3/Entity/tr3_cobra.h"

#include "Game/control/box.h"
#include "Game/control/los.h"
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
	constexpr auto COBRA_BITE_ATTACK_DAMAGE	 = 80;
	constexpr auto COBRA_BITE_POISON_POTENCY = 8;

	constexpr auto COBRA_ATTACK_RANGE = SQUARE(SECTOR(1));
	constexpr auto COBRA_AWARE_RANGE  = SQUARE(SECTOR(1.5f));
	constexpr auto COBRA_SLEEP_RANGE  = SQUARE(SECTOR(2.5f));

	constexpr auto PLAYER_DISTURB_VELOCITY = 15;

	constexpr auto COBRA_SLEEP_FRAME = 45;

	const auto CobraBite = BiteInfo(Vector3::Zero, 13);
	const vector<int> CobraAttackJoints = { 13 };

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
		SetAnimation(item, COBRA_ANIM_IDLE_TO_SLEEP, COBRA_SLEEP_FRAME);
		item->ItemFlags[2] = item->HitStatus;
	}

	void CobraControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* info = GetCreatureInfo(item);

		int h = GetBoundsAccurate(item)->Height();

		short head = 0;
		short angle = 0;
		short tilt = 0;

		if (item->HitPoints <= 0 && item->HitPoints != NOT_TARGETABLE)
		{
			if (item->Animation.ActiveState != COBRA_STATE_DEATH)
				SetAnimation(item, COBRA_ANIM_DEATH);
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			AI.angle += ANGLE(16.8f);

			GetCreatureMood(item, &AI, 1);
			CreatureMood(item, &AI, 1);

			bool enemyMoving  = false;
			bool enemyVisible = false;
			if (info->Enemy && (GlobalCounter & 2))
			{
				auto src = GameVector(info->Enemy->Pose.Position, info->Enemy->RoomNumber);
				auto dest = GameVector(item->Pose.Position, item->RoomNumber);
				enemyVisible = LOS(&src, &dest);

				enemyMoving = info->Enemy->Animation.Velocity > PLAYER_DISTURB_VELOCITY ||
							  abs(info->Enemy->Animation.VerticalVelocity) > PLAYER_DISTURB_VELOCITY;
			}

			if (enemyVisible && item->Animation.ActiveState != COBRA_STATE_SLEEP)
			{
				info->Target.x = info->Enemy->Pose.Position.x;
				info->Target.z = info->Enemy->Pose.Position.z;
				angle = CreatureTurn(item, info->MaxTurn);

				if (AI.ahead)
					head = AI.angle;

				if (abs(AI.angle) < ANGLE(10.0f))
					item->Pose.Orientation.y += AI.angle;
				else if (AI.angle < 0)
					item->Pose.Orientation.y -= ANGLE(10.0f);
				else
					item->Pose.Orientation.y += ANGLE(10.0f);
			}

			switch (item->Animation.ActiveState)
			{
			case COBRA_STATE_IDLE:
				info->Flags = NULL;

				if (AI.distance > COBRA_SLEEP_RANGE)
				{
					item->Animation.TargetState = COBRA_STATE_SLEEP;
				}
				else if (info->Enemy->HitPoints > 0 && enemyVisible &&
						((AI.ahead && AI.distance < COBRA_ATTACK_RANGE && AI.verticalDistance <= GetBoundsAccurate(item)->Height()) ||
						item->HitStatus || enemyMoving))
				{
					item->Animation.TargetState = COBRA_STATE_ATTACK;
				}

				break;

			case COBRA_STATE_SLEEP:
				info->Flags = NULL;

				if (item->HitPoints != NOT_TARGETABLE)
				{
					item->ItemFlags[2] = item->HitPoints;
					item->HitPoints = NOT_TARGETABLE;
				}

				if (AI.distance < COBRA_AWARE_RANGE && info->Enemy->HitPoints > 0)
				{
					item->Animation.TargetState = COBRA_STATE_WAKE_UP;
					item->HitPoints = item->ItemFlags[2];
				}

				break;

			case COBRA_STATE_ATTACK:
				if (info->Flags != 1 &&
					item->TestBits(JointBitType::Touch, CobraAttackJoints))
				{
					DoDamage(info->Enemy, COBRA_BITE_ATTACK_DAMAGE);
					CreatureEffect(item, CobraBite, DoBloodSplat);
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
