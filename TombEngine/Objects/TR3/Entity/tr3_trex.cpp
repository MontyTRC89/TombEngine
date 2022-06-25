#include "framework.h"
#include "Objects/TR3/Entity/tr3_trex.h"

#include "Game/camera.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using std::vector;

namespace TEN::Entities::TR3
{
	const vector<int> TRexAttackJoints = { 12, 13 };

	#define LARA_ANIM_TREX_DEATH 4

	enum TRexState
	{
		TREX_STATE_NONE = 0,
		TREX_STATE_IDLE = 1,
		TREX_STATE_WALK_FORWARD = 2,
		TREX_STATE_RUN_FORWARD = 3,
		// 4? Perhaps it was used as a hacky way of storing the index for Lara's trex death animation. @Sezz 2022.05.24
		TREX_STATE_DEATH = 5,
		TREX_STATE_ROAR = 6,
		TREX_STATE_ATTACK = 7,
		TREX_STATE_KILL = 8
	};

	enum TRexAnim
	{
		TREX_ANIM_IDLE = 0,
		TREX_ANIM_RUN_FORWARD = 1,
		TREX_ANIM_WALK_FORWARD = 2,
		TREX_ANIM_IDLE_TO_WALK_FORWARD = 3,
		TREX_ANIM_WALK_FORWARD_TO_IDLE = 4,
		TREX_ANIM_ROAR = 5,
		TREX_ANIM_ATTACK = 6,
		TREX_ANIM_IDLE_TO_RUN_START = 7,
		TREX_ANIM_IDLE_TO_RUN_END = 8,
		TREX_ANIM_RUN_FORWARD_TO_IDLE = 9,
		TREX_ANIM_DEATH = 10,
		TREX_ANIM_KILL = 11
	};

	void LaraTRexDeath(ItemInfo* tRexItem, ItemInfo* laraItem)
	{
		tRexItem->Animation.TargetState = TREX_STATE_KILL;

		if (laraItem->RoomNumber != tRexItem->RoomNumber)
			ItemNewRoom(Lara.ItemNumber, tRexItem->RoomNumber);

		laraItem->Pose.Position = tRexItem->Pose.Position;
		laraItem->Pose.Orientation = Vector3Shrt(0, tRexItem->Pose.Orientation.y, 0);
		laraItem->Animation.IsAirborne = false;

		laraItem->Animation.AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex + LARA_ANIM_TREX_DEATH;
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
		laraItem->Animation.ActiveState = LS_DEATH;
		laraItem->Animation.TargetState = LS_DEATH;

		laraItem->HitPoints = NOT_TARGETABLE;
		Lara.Air = -1;
		Lara.Control.HandStatus = HandStatus::Busy;
		Lara.Control.Weapon.GunType = LaraWeaponType::None;

		Camera.flags = 1;
		Camera.targetAngle = ANGLE(170.0f);
		Camera.targetElevation = -ANGLE(25.0f);
	}

	void TRexControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* info = GetCreatureInfo(item);

		short head = 0;
		short angle = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState == TREX_STATE_IDLE)
				item->Animation.TargetState = TREX_STATE_DEATH;
			else
				item->Animation.TargetState = TREX_STATE_IDLE;
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;

			GetCreatureMood(item, &AI, VIOLENT);
			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, info->MaxTurn);

			if (item->TouchBits)
				DoDamage(LaraItem, (item->Animation.ActiveState == TREX_STATE_RUN_FORWARD) ? 10 : 1);

			info->Flags = (info->Mood != MoodType::Escape && !AI.ahead && AI.enemyFacing > -FRONT_ARC && AI.enemyFacing < FRONT_ARC);

			if (AI.distance > pow(1500, 2) &&
				AI.distance < pow(SECTOR(4), 2) &&
				AI.bite && !info->Flags)
			{
				info->Flags = 1;
			}

			switch (item->Animation.ActiveState)
			{
			case TREX_STATE_IDLE:
				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (AI.distance < pow(1500, 2) && AI.bite)
					item->Animation.TargetState = TREX_STATE_ATTACK;
				else if (info->Mood == MoodType::Bored || info->Flags)
					item->Animation.TargetState = TREX_STATE_WALK_FORWARD;
				else
					item->Animation.TargetState = TREX_STATE_RUN_FORWARD;

				break;

			case TREX_STATE_WALK_FORWARD:
				info->MaxTurn = ANGLE(2.0f);

				if (info->Mood != MoodType::Bored || !info->Flags)
					item->Animation.TargetState = TREX_STATE_IDLE;
				else if (AI.ahead && GetRandomControl() < 0x200)
				{
					item->Animation.RequiredState = TREX_STATE_ROAR;
					item->Animation.TargetState = TREX_STATE_IDLE;
				}

				break;

			case TREX_STATE_RUN_FORWARD:
				info->MaxTurn = ANGLE(4.0f);

				if (AI.distance < pow(SECTOR(5), 2) && AI.bite)
					item->Animation.TargetState = TREX_STATE_IDLE;
				else if (info->Flags)
					item->Animation.TargetState = TREX_STATE_IDLE;
				else if (info->Mood != MoodType::Escape && AI.ahead && GetRandomControl() < 0x200)
				{
					item->Animation.RequiredState = TREX_STATE_ROAR;
					item->Animation.TargetState = TREX_STATE_IDLE;
				}
				else if (info->Mood == MoodType::Bored)
					item->Animation.TargetState = TREX_STATE_IDLE;

				break;

			case TREX_STATE_ATTACK:
				if (item->TestBits(JointBitType::Touch, TRexAttackJoints))
				{
					item->Animation.TargetState = TREX_STATE_KILL;

					DoDamage(LaraItem, 1500);
					LaraTRexDeath(item, LaraItem);
				}

				item->Animation.RequiredState = TREX_STATE_WALK_FORWARD;
				break;
			}
		}

		CreatureJoint(item, 0, (short)(head * 2));
		info->JointRotation[1] = info->JointRotation[0];

		CreatureAnimation(itemNumber, angle, 0);

		item->Collidable = true;
	}
}
