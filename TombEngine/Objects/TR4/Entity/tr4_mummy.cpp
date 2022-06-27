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

namespace TEN::Entities::TR4
{
	BITE_INFO MummyBite1 = { 0, 0, 0, 11 };
	BITE_INFO MummyBite2 = { 0, 0, 0, 14 };
	const std::vector<int> MummyAttackJoints { 11, 14 };

	constexpr auto MUMMY_IDLE_SWIPE_ATTACK_RANGE = SECTOR(0.5f);
	constexpr auto MUMMY_WALK_FORWARD_SWIPE_ATTACK_RANGE = SECTOR(0.67f);
	constexpr auto MUMMY_ACTIVATE_RANGE = SECTOR(1);
	constexpr auto MUMMY_RECOIL_RANGE = SECTOR(3);
	constexpr auto MUMMY_ARMS_UP_RANGE = SECTOR(3);
	constexpr auto MUMMY_AWARE_RANGE = SECTOR(7);

	#define MUMMY_WALK_TURN_ANGLE ANGLE(7.0f)

	enum MymmyState
	{
		MUMMY_STATE_INACTIVE_ARMS_CROSSED = 0,
		MUMMY_STATE_IDLE = 1,
		MUMMY_STATE_WALK_FORWARD = 2,
		MUMMY_STATE_WALK_FORWARD_ARMS_UP = 3,
		MUMMY_STATE_WALK_FORWARD_SWIPE_ATTACK = 4,
		MUMMY_STATE_RECOIL = 5,
		MUMMY_STATE_ARMS_UP_RECOIL = 6,
		MUMMY_STATE_COLLAPSE = 7,
		MUMMY_INACTIVE_STATE_LYING_DOWN = 8,
		MUMMY_STATE_COLLAPSED_TO_IDLE = 9,
		MUMMY_STATE_IDLE_SWIPE_ATTACK = 10
	};

	enum MummyAnim
	{
		MUMMY_ANIM_STAND = 0,
		MUMMY_ANIM_WALK_FORWARD = 1,
		MUMMY_ANIM_WALK_FORWARD_ARMS_UP = 2,
		MUMMY_ANIM_RECOIL = 3,
		MUMMY_ANIM_WALK_FORWARD_TO_WALK_FORWARD_ARMS_UP_RIGHT = 4,
		MUMMY_ANIM_WALK_FORWARD_ARMS_UP_TO_WALK_FORWARD_LEFT = 5,
		MUMMY_ANIM_WALK_FORWARD_ARMS_UP_TO_IDLE = 6,
		MUMMY_ANIM_IDLE_TO_WALK_FORWARD_ARMS_UP = 7,
		MUMMY_ANIM_IDLE_TO_WALK_FORWARD = 8,
		MUMMY_ANIM_WALK_FORWARD_TO_IDLE = 9,
		MUMMY_ANIM_COLLAPSE_START = 10,
		MUMMY_ANIM_COLLAPSE_CONTINUE = 11,
		MUMMY_ANIM_COLLAPSE_END = 12,
		MUMMY_ANIM_COLLAPSED_TO_IDLE = 13,
		MUMMY_ANIM_IDLE_SWIPE_ATTACK_RIGHT = 14,
		MUMMY_ANIM_IDLE_SWIPE_ATTACK_LEFT = 15,
		MUMMY_ANIM_WALK_FORWARD_SWIPE_ATTACK = 16,
		MUMMY_ANIM_ARMS_CROSSED_TO_WALK_FORWARD_START = 17,
		MUMMY_ANIM_ARMS_CROSSED_TO_WALK_FORWARD_END = 18,
		MUMMY_ANIM_ARMS_CROSSED = 19,
		MUMMY_ANIM_ARMS_UP_RECOIL = 20
	};

	void InitialiseMummy(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		if (item->TriggerFlags == 2)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + MUMMY_ANIM_COLLAPSE_END;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.TargetState = MUMMY_INACTIVE_STATE_LYING_DOWN;
			item->Animation.ActiveState = MUMMY_INACTIVE_STATE_LYING_DOWN;
			item->Status -= ITEM_INVISIBLE;
		}
		else
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + MUMMY_ANIM_ARMS_CROSSED;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.TargetState = MUMMY_STATE_INACTIVE_ARMS_CROSSED;
			item->Animation.ActiveState = MUMMY_STATE_INACTIVE_ARMS_CROSSED;
		}
	}

	void MummyControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short tilt = 0;
		short angle = 0;
		short joint0 = 0;
		short joint1 = 0;
		short joint2 = 0;

		if (item->AIBits)
			GetAITarget(creature);
		else if (creature->HurtByLara)
			creature->Enemy = LaraItem;

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		if (item->HitStatus)
		{
			if (AI.distance < pow(MUMMY_RECOIL_RANGE, 2))
			{
				if (item->Animation.ActiveState != MUMMY_ANIM_IDLE_TO_WALK_FORWARD_ARMS_UP &&
					item->Animation.ActiveState != MUMMY_ANIM_WALK_FORWARD_ARMS_UP_TO_WALK_FORWARD_LEFT &&
					item->Animation.ActiveState != MUMMY_ANIM_IDLE_TO_WALK_FORWARD)
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
							if (item->Animation.ActiveState == MUMMY_STATE_WALK_FORWARD_ARMS_UP ||
								item->Animation.ActiveState == MUMMY_STATE_WALK_FORWARD_SWIPE_ATTACK)
							{
								item->Animation.ActiveState = MUMMY_STATE_ARMS_UP_RECOIL;
								item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + MUMMY_ANIM_ARMS_UP_RECOIL;
							}
							else
							{
								item->Animation.ActiveState = MUMMY_STATE_RECOIL;
								item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + MUMMY_ANIM_RECOIL;
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

				if (AI.distance <= pow(MUMMY_IDLE_SWIPE_ATTACK_RANGE, 2) ||
					AI.distance >= pow(MUMMY_AWARE_RANGE, 2))
				{
					if (AI.distance - pow(MUMMY_IDLE_SWIPE_ATTACK_RANGE, 2) <= 0)
						item->Animation.TargetState = MUMMY_STATE_IDLE_SWIPE_ATTACK;
					else
					{
						item->Animation.TargetState = MUMMY_STATE_IDLE;
						joint0 = 0;
						joint1 = 0;
						joint2 = 0;

						if (item->TriggerFlags > -100 && item->TriggerFlags < 0)
							item->TriggerFlags++;
					}
				}
				else
					item->Animation.TargetState = MUMMY_STATE_WALK_FORWARD;

				break;

			case MUMMY_STATE_WALK_FORWARD:
				if (item->TriggerFlags == 1)
				{
					creature->MaxTurn = 0;

					if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
						item->TriggerFlags = 0;
				}
				else
				{
					creature->MaxTurn = MUMMY_WALK_TURN_ANGLE;

					if (AI.distance >= pow(MUMMY_ARMS_UP_RANGE, 2))
					{
						if (AI.distance > pow(MUMMY_AWARE_RANGE, 2))
							item->Animation.TargetState = MUMMY_STATE_IDLE;
					}
					else
						item->Animation.TargetState = MUMMY_STATE_WALK_FORWARD_ARMS_UP;
				}

				break;

			case MUMMY_STATE_WALK_FORWARD_ARMS_UP:
				creature->MaxTurn = MUMMY_WALK_TURN_ANGLE;
				creature->Flags = 0;

				if (AI.distance < pow(MUMMY_IDLE_SWIPE_ATTACK_RANGE, 2))
				{
					item->Animation.TargetState = MUMMY_STATE_IDLE;
					break;
				}

				if (AI.distance > pow(MUMMY_ARMS_UP_RANGE, 2) && AI.distance < pow(MUMMY_AWARE_RANGE, 2))
				{
					item->Animation.TargetState = MUMMY_STATE_WALK_FORWARD;
					break;
				}

				if (AI.distance <= pow(MUMMY_WALK_FORWARD_SWIPE_ATTACK_RANGE, 2))
					item->Animation.TargetState = MUMMY_STATE_WALK_FORWARD_SWIPE_ATTACK;
				else if (AI.distance > pow(MUMMY_AWARE_RANGE, 2))
					item->Animation.TargetState = MUMMY_STATE_IDLE;

				break;

			case MUMMY_STATE_INACTIVE_ARMS_CROSSED:
				creature->MaxTurn = 0;

				if (AI.distance < pow(MUMMY_ACTIVATE_RANGE, 2) || item->TriggerFlags > -1)
					item->Animation.TargetState = MUMMY_STATE_WALK_FORWARD;

				break;

			case MUMMY_INACTIVE_STATE_LYING_DOWN:
				item->HitPoints = 0;
				creature->MaxTurn = 0;
				joint0 = 0;
				joint1 = 0;
				joint2 = 0;

				if (AI.distance < pow(MUMMY_ACTIVATE_RANGE, 2) || !(GetRandomControl() & 0x7F))
				{
					item->Animation.TargetState = MUMMY_STATE_COLLAPSED_TO_IDLE;
					item->HitPoints = Objects[item->ObjectNumber].HitPoints;
				}

				break;

			case MUMMY_STATE_WALK_FORWARD_SWIPE_ATTACK:
			case MUMMY_STATE_IDLE_SWIPE_ATTACK:
				creature->MaxTurn = 0;

				if (abs(AI.angle) >= ANGLE(7.0f))
				{
					if (AI.angle >= 0)
						item->Pose.Orientation.y += ANGLE(7.0f);
					else
						item->Pose.Orientation.y -= ANGLE(7.0f);
				}
				else
					item->Pose.Orientation.y += AI.angle;

				if (!creature->Flags)
				{
					if (item->TestBits(JointBitType::Touch, MummyAttackJoints))
					{
						if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase &&
							item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameEnd)
						{
							DoDamage(creature->Enemy, 100);

							if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + MUMMY_ANIM_IDLE_SWIPE_ATTACK_LEFT)
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
}
