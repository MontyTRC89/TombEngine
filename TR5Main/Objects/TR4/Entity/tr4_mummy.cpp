#include "framework.h"
#include "tr4_mummy.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Game/control/control.h"
#include "Specific/level.h"
#include "Game/itemdata/creature_info.h"
#include "Game/misc.h"

BITE_INFO MummyBite1 = { 0, 0, 0, 11 };
BITE_INFO MummyBite2 = { 0, 0, 0, 14 };

enum MymmyState
{
	MUMMY_STATE_ARMS_CROSSED = 0,
	MUMMY_STATE_IDLE = 1,
	MUMMY_STATE_WALK = 2,
	MUMMY_STATE_WALK_ARMS_UP = 3,
	MUMMY_STATE_WALK_HIT = 4,
	MUMMY_STATE_PUSHED_BACK = 5,
	MUMMY_STATE_ARMS_UP_PUSHED_BACK = 6,
	MUMMY_STATE_COLLAPSE = 7,
	MUMMY_STATE_LYING_DOWN = 8,
	MUMMY_STATE_GET_UP = 9,
	MUMMY_STATE_HIT = 10
};

enum MummyAnim
{
	MUMMY_ANIM_STAND = 0,
	MUMMY_ANIM_WALK = 1,
	MUMMY_ANIM_WALK_ARMS_UP = 2,
	MUMMY_ANIM_PUSHED_BACK = 3,
	MUMMY_ANIM_WALK_TO_WALK_ARMS_UP_RIGHT = 4,
	MUMMY_ANIM_WALK_ARMS_UP_TO_WALK_LEFT = 5,
	MUMMY_ANIM_WALK_ARMS_UP_TO_STAND = 6,
	MUMMY_ANIM_STAND_TO_WALK_ARMS_UP = 7,
	MUMMY_ANIM_STAND_TO_WALK = 8,
	MUMMY_ANIM_WALK_TO_STAND = 9,
	MUMMY_ANIM_COLLAPSE_START = 10,
	MUMMY_ANIM_COLLAPSE_END = 11,
	MUMMY_ANIM_LYING_DOWN = 12,
	MUMMY_ANIM_GET_UP = 13,
	MUMMY_ANIM_HIT_RIGHT = 14,
	MUMMY_ANIM_HIT_LEFT = 15,
	MUMMY_ANIM_WALK_HIT = 16,
	MUMMY_ANIM_ARMS_CROSSED_TO_STAND_START = 17,
	MUMMY_ANIM_ARMS_CROSSED_TO_STAND_END = 18,
	MUMMY_ANIM_ARMS_CROSSED = 19,
	MUMMY_ANIM_ARMS_UP_PUSHED_BACK = 20
};

void InitialiseMummy(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	if (item->TriggerFlags == 2)
	{
		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + MUMMY_ANIM_LYING_DOWN;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.TargetState = MUMMY_STATE_LYING_DOWN;
		item->Animation.ActiveState = MUMMY_STATE_LYING_DOWN;
		item->Status = ITEM_INVISIBLE;
	}
	else
	{
		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + MUMMY_ANIM_ARMS_CROSSED;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.TargetState = MUMMY_STATE_ARMS_CROSSED;
		item->Animation.ActiveState = MUMMY_STATE_ARMS_CROSSED;
	}
}

void MummyControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	float tilt = 0;
	float angle = 0;
	float joint0 = 0;
	float joint1 = 0;
	float joint2 = 0;

	if (item->AIBits)
		GetAITarget(creature);
	else if (creature->HurtByLara)
		creature->Enemy = LaraItem;

	AI_INFO AI;
	CreatureAIInfo(item, &AI);

	if (item->HitStatus)
	{
		if (AI.distance < pow(SECTOR(3), 2))
		{
			if (item->Animation.ActiveState != MUMMY_ANIM_STAND_TO_WALK_ARMS_UP &&
				item->Animation.ActiveState != MUMMY_ANIM_WALK_ARMS_UP_TO_WALK_LEFT &&
				item->Animation.ActiveState != MUMMY_ANIM_STAND_TO_WALK)
			{
				if (GetRandomControl() & 3 ||
					Lara.Control.Weapon.GunType != LaraWeaponType::Shotgun &&
					Lara.Control.Weapon.GunType != LaraWeaponType::HK &&
					Lara.Control.Weapon.GunType != LaraWeaponType::Revolver)
				{
					if (!(GetRandomControl() & 7) ||
						Lara.Control.Weapon.GunType == LaraWeaponType::Shotgun ||
						Lara.Control.Weapon.GunType == LaraWeaponType::HK ||
						Lara.Control.Weapon.GunType == LaraWeaponType::Revolver)
					{
						if (item->Animation.ActiveState == MUMMY_STATE_WALK_ARMS_UP ||
							item->Animation.ActiveState == MUMMY_STATE_WALK_HIT)
						{
							item->Animation.ActiveState = MUMMY_STATE_ARMS_UP_PUSHED_BACK;
							item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + MUMMY_ANIM_ARMS_UP_PUSHED_BACK;
						}
						else
						{
							item->Animation.ActiveState = MUMMY_STATE_PUSHED_BACK;
							item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + MUMMY_ANIM_PUSHED_BACK;
						}

						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
						item->Pose.Orientation.y += AI.angle;
					}
				}
				else
				{
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + MUMMY_ANIM_COLLAPSE_START;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					item->Animation.ActiveState = MUMMY_STATE_COLLAPSE;
					item->Pose.Orientation.y += AI.angle;
					creature->MaxTurn = 0;
				}
			}
		}
	}
	else
	{
		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);

		if (AI.ahead)
		{
			joint0 = AI.angle / 2;
			joint1 = AI.angle / 2;
			joint2 = AI.xAngle;
		}

		switch (item->Animation.ActiveState)
		{
		case MUMMY_STATE_IDLE:
			creature->MaxTurn = 0;
			creature->Flags = 0;

			if (AI.distance <= pow(SECTOR(0.5f), 2) ||
				AI.distance >= pow(SECTOR(7), 2))
			{
				if (AI.distance - pow(SECTOR(0.5f), 2) <= 0)
					item->Animation.TargetState = MUMMY_STATE_HIT;
				else
				{
					item->Animation.TargetState = MUMMY_STATE_IDLE;
					joint0 = 0;
					joint1 = 0;
					joint2 = 0;

					if (item->TriggerFlags > -100 && item->TriggerFlags & 0x8000 < 0)
						item->TriggerFlags++;
				}
			}
			else
				item->Animation.TargetState = MUMMY_STATE_WALK;

			break;

		case MUMMY_STATE_WALK:
			if (item->TriggerFlags == 1)
			{
				creature->MaxTurn = 0;

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
					item->TriggerFlags = 0;
			}
			else
			{
				creature->MaxTurn = EulerAngle::DegToRad(7.0f);

				if (AI.distance >= pow(SECTOR(3), 2))
				{
					if (AI.distance > pow(SECTOR(7), 2))
						item->Animation.TargetState = MUMMY_STATE_IDLE;
				}
				else
					item->Animation.TargetState = MUMMY_STATE_WALK_ARMS_UP;
			}

			break;

		case MUMMY_STATE_WALK_ARMS_UP:
			creature->MaxTurn = EulerAngle::DegToRad(7.0f);
			creature->Flags = 0;

			if (AI.distance < pow(SECTOR(0.5f), 2))
			{
				item->Animation.TargetState = MUMMY_STATE_IDLE;
				break;
			}

			if (AI.distance > pow(SECTOR(3), 2) && AI.distance < pow(SECTOR(7), 2))
			{
				item->Animation.TargetState = MUMMY_STATE_WALK;
				break;
			}

			if (AI.distance <= pow(682, 2))
				item->Animation.TargetState = MUMMY_STATE_WALK_HIT;
			else if (AI.distance > pow(SECTOR(7), 2))
				item->Animation.TargetState = MUMMY_STATE_IDLE;

			break;

		case MUMMY_STATE_ARMS_CROSSED:
			creature->MaxTurn = 0;

			if (AI.distance < pow(SECTOR(1), 2) || item->TriggerFlags > -1)
				item->Animation.TargetState = MUMMY_STATE_WALK;

			break;

		case MUMMY_STATE_LYING_DOWN:
			item->HitPoints = 0;
			creature->MaxTurn = 0;
			joint0 = 0;
			joint1 = 0;
			joint2 = 0;

			if (AI.distance < pow(SECTOR(1), 2) || !(GetRandomControl() & 0x7F))
			{
				item->Animation.TargetState = MUMMY_STATE_GET_UP;
				item->HitPoints = Objects[item->ObjectNumber].HitPoints;
			}

			break;

		case MUMMY_STATE_WALK_HIT:
		case MUMMY_STATE_HIT:
			creature->MaxTurn = 0;

			if (abs(AI.angle) >= EulerAngle::DegToRad(7.0f))
			{
				if (AI.angle >= 0)
					item->Pose.Orientation.y += EulerAngle::DegToRad(7.0f);
				else
					item->Pose.Orientation.y -= EulerAngle::DegToRad(7.0f);
			}
			else
				item->Pose.Orientation.y += AI.angle;

			if (!creature->Flags)
			{
				if (item->TouchBits & 0x4800)
				{
					if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase &&
						item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameEnd)
					{
						LaraItem->HitPoints -= 100;
						LaraItem->HitStatus = true;

						if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + MUMMY_ANIM_HIT_LEFT)
						{
							CreatureEffect2(
								item,
								&MummyBite1,
								5,
								-1,
								DoBloodSplat);
						}
						else
						{
							CreatureEffect2(
								item,
								&MummyBite2,
								5,
								-1,
								DoBloodSplat);
						}

						creature->Flags = 1;
					}
				}
			}

			break;

		default:
			break;
		}
	}

	CreatureTilt(item, 0);

	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);

	CreatureAnimation(itemNumber, angle, 0);
}
