#include "framework.h"
#include "Objects/TR1/Entity/tr1_giant_mutant.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/prng.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;
using std::vector;

namespace TEN::Entities::Creatures::TR1
{
	constexpr auto MUTANT_ATTACK_DAMAGE	 = 500;
	constexpr auto MUTANT_CONTACT_DAMAGE = 6;

	constexpr auto MUTANT_ATTACK_RANGE = SQUARE(SECTOR(2.5f));
	constexpr auto MUTANT_CLOSE_RANGE  = SQUARE(SECTOR(2.2f));

	// TODO: Unused.
	constexpr auto MUTANT_ATTACK_1_CHANCE = 0.33f;
	constexpr auto MUTANT_ATTACK_2_CHANCE = 0.67f;

	#define MUTANT_NEED_TURN ANGLE(45.0f)
	#define MUTANT_TURN ANGLE(3.0f)

	#define LARA_GIANT_MUTANT_DEATH 6 // TODO: Not 13? Check this.

	const vector<int> MutantAttackJoints	  = { 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 };
	const vector<int> MutantAttackLeftJoint	  = { 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
	const vector<int> MutantAttackRightJoints = { 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 };

	enum GiantMutantState
	{
		MUTANT_STATE_NONE = 0,
		MUTANT_STATE_IDLE = 1,
		MUTANT_STATE_TURN_LEFT = 2,
		MUTANT_STATE_TURN_RIGHT = 3,
		MUTANT_STATE_ATTACK_1 = 4,
		MUTANT_STATE_ATTACK_2 = 5,
		MUTANT_STATE_ATTACK_3 = 6,
		MUTANT_STATE_FORWARD = 7,
		MUTANT_STATE_SET = 8,
		MUTANT_STATE_FALL = 9,
		MUTANT_STATE_DEATH = 10,
		MUTANT_STATE_KILL = 11
	};

	// TODO
	enum GiantMutantAnim
	{
		MUTANT_ANIM_DEATH = 13,
	};

	void GiantMutantControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short head = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != MUTANT_STATE_DEATH)
				SetAnimation(item, MUTANT_ANIM_DEATH);
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = (short)phd_atan(creature->Target.z - item->Pose.Position.z, creature->Target.x - item->Pose.Position.x) - item->Pose.Orientation.y;

			if (item->TouchBits)
				DoDamage(creature->Enemy, MUTANT_CONTACT_DAMAGE);

			switch (item->Animation.ActiveState)
			{
			case MUTANT_STATE_SET:
				item->Animation.TargetState = MUTANT_STATE_FALL;
				item->Animation.IsAirborne = true;
				break;

			case MUTANT_STATE_IDLE:
				if (LaraItem->HitPoints <= 0)
					break;

				creature->Flags = 0;

				if (angle > MUTANT_NEED_TURN)
					item->Animation.TargetState = MUTANT_STATE_TURN_RIGHT;
				else if (angle < -MUTANT_NEED_TURN)
					item->Animation.TargetState = MUTANT_STATE_TURN_LEFT;
				else if (AI.distance < MUTANT_ATTACK_RANGE)
				{
					if (LaraItem->HitPoints <= MUTANT_ATTACK_DAMAGE)
					{
						if (AI.distance < MUTANT_CLOSE_RANGE)
							item->Animation.TargetState = MUTANT_STATE_ATTACK_3;
						else
							item->Animation.TargetState = MUTANT_STATE_FORWARD;
					}
					else if (TestProbability(0.5f))
						item->Animation.TargetState = MUTANT_STATE_ATTACK_1;
					else
						item->Animation.TargetState = MUTANT_STATE_ATTACK_2;
				}
				else
					item->Animation.TargetState = MUTANT_STATE_FORWARD;

				break;

			case MUTANT_STATE_FORWARD:
				if (angle < -MUTANT_TURN)
					item->Animation.TargetState -= MUTANT_TURN;
				else if (angle > MUTANT_TURN)
					item->Animation.TargetState += MUTANT_TURN;
				else
					item->Animation.TargetState += angle;

				if (angle > MUTANT_NEED_TURN || angle < -MUTANT_NEED_TURN)
					item->Animation.TargetState = MUTANT_STATE_IDLE;
				else if (AI.distance < MUTANT_ATTACK_RANGE)
					item->Animation.TargetState = MUTANT_STATE_IDLE;

				break;

			case MUTANT_STATE_TURN_RIGHT:
				if (!creature->Flags)
					creature->Flags = item->Animation.FrameNumber;
				else if (item->Animation.FrameNumber - creature->Flags > 16 &&
					item->Animation.FrameNumber - creature->Flags < 23)
				{
					item->Pose.Orientation.y += ANGLE(14.0f);
				}

				if (angle < MUTANT_NEED_TURN)
					item->Animation.TargetState = MUTANT_STATE_IDLE;

				break;

			case MUTANT_STATE_TURN_LEFT:
				if (!creature->Flags)
					creature->Flags = item->Animation.FrameNumber;
				else if (item->Animation.FrameNumber - creature->Flags > 13 &&
					item->Animation.FrameNumber - creature->Flags < 23)
				{
					item->Pose.Orientation.y -= ANGLE(9.0f);
				}

				if (angle > -MUTANT_NEED_TURN)
					item->Animation.TargetState = MUTANT_STATE_IDLE;

				break;

			case MUTANT_STATE_ATTACK_1:
				if (!creature->Flags && item->TestBits(JointBitType::Touch, MutantAttackRightJoints))
				{
					DoDamage(creature->Enemy, MUTANT_ATTACK_DAMAGE);
					creature->Flags = 1;
				}

				break;

			case MUTANT_STATE_ATTACK_2:
				if (!creature->Flags && item->TestBits(JointBitType::Touch, MutantAttackJoints))
				{
					DoDamage(creature->Enemy, MUTANT_ATTACK_DAMAGE);
					creature->Flags = 1;
				}

				break;

			case MUTANT_STATE_ATTACK_3:
				if (item->TestBits(JointBitType::Touch, MutantAttackRightJoints) || LaraItem->HitPoints <= 0)
				{
					item->Animation.TargetState = MUTANT_STATE_KILL;
					Camera.targetDistance = SECTOR(2);
					Camera.flags = CF_FOLLOW_CENTER;

					LaraItem->Animation.AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex + LARA_GIANT_MUTANT_DEATH;
					LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
					LaraItem->Animation.ActiveState = 46;
					LaraItem->Animation.TargetState = 46;
					LaraItem->Animation.IsAirborne = false;
					LaraItem->Pose = PHD_3DPOS(item->Pose.Position, 0, item->Pose.Orientation.y, 0);
					LaraItem->RoomNumber = item->RoomNumber;
					LaraItem->HitPoints = -1;
					Lara.Air = -1;
					Lara.Control.HandStatus = HandStatus::Busy;
					Lara.Control.Weapon.GunType = LaraWeaponType::None;
				}

				break;

			case MUTANT_STATE_KILL:
				Camera.targetDistance = SECTOR(2);
				Camera.flags = CF_FOLLOW_CENTER;
				break;
			}
		}

		CreatureJoint(item, 0, head);

		if (item->Animation.ActiveState == MUTANT_STATE_FALL)
		{
			AnimateItem(item);

			if (item->Pose.Position.y > item->Floor)
			{
				item->Animation.TargetState = MUTANT_STATE_IDLE;
				item->Animation.IsAirborne = false;
				item->Pose.Position.y = item->Floor;
				Camera.bounce = 500;
			}
		}
		else
			CreatureAnimation(itemNumber, 0, 0);

		if (item->Status == ITEM_DEACTIVATED)
		{
			SoundEffect(SFX_TR1_ATLANTEAN_DEATH, &item->Pose);
			ExplodingDeath(itemNumber, BODY_EXPLODE);
		
			TestTriggers(item, true);

			KillItem(itemNumber);
			item->Status = ITEM_DEACTIVATED;
		}
	}
}
