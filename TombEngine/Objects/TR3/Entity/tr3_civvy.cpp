#include "framework.h"
#include "Objects/TR3/Entity/tr3_civvy.h"

#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Math/Random.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;
using std::vector;

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto CIVVY_ATTACK_DAMAGE  = 50;

	constexpr auto CIVVY_ATTACK_WALKING_PUNCH_RANGE = SQUARE(BLOCK(0.75f));
	constexpr auto CIVVY_ATTACK_CLOSE_PUNCH_RANGE = SQUARE(BLOCK(0.45f));
	constexpr auto CIVVY_ATTACK_FAR_PUNCH_RANGE = SQUARE(BLOCK(0.70f));
	
	constexpr auto CIVVY_WALK_RANGE	   = SQUARE(BLOCK(2));
	constexpr auto CIVVY_ESCAPE_RANGE  = SQUARE(BLOCK(5));
	constexpr auto CIVVY_AWARE_RANGE   = SQUARE(BLOCK(1));

	constexpr auto CIVVY_WAIT_CHANCE	   = 0.008f;
	constexpr auto CIVVY_STATE_WALK_CHANCE = 0.008f; // Unused.

	constexpr auto CIVVY_VAULT_SHIFT = 260;

	#define CIVVY_WALK_TURN_RATE_MAX ANGLE(5.0f)
	#define CIVVY_RUN_TURN_RATE_MAX	 ANGLE(6.0f)
	#define CIVVY_AIM_TURN_RATE_MAX	 ANGLE(10.0f)

	constexpr auto CIVVY_TARGET_ALERT_VELOCITY = 10.0f;

	const auto CivvyBiteRight = BiteInfo(Vector3::Zero, 13);
	const auto CivvyBiteLeft = BiteInfo(Vector3::Zero, 10);
	const vector<unsigned int> CivvyAttackJoints = { 10, 13 };

	std::vector<GAME_OBJECT_ID> CivvyExcludedTargets =
	{
		ID_CIVVIE,
		ID_VON_CROY,
		ID_GUIDE,
		ID_MONK1,
		ID_MONK2
	};

	enum CivvyState
	{
		// No state 0.
		CIVVY_STATE_IDLE = 1,
		CIVVY_STATE_WALK = 2,
		CIVVY_STATE_ATTACK_WALKING_PUNCH = 3, //Punch while walking
		CIVVY_STATE_AIM_WALKING_PUNCH = 4,
		CIVVY_STATE_WAIT = 5,
		CIVVY_STATE_AIM_FAR_PUNCH = 6, //Punch from Idle to walk
		CIVVY_STATE_AIM_CLOSE_PUNCH = 7, //Punch while Idle
		CIVVY_STATE_ATTACK_FAR_PUNCH = 8,
		CIVVY_STATE_ATTACK_CLOSE_PUNCH = 9,
		CIVVY_STATE_RUN = 10,
		CIVVY_STATE_DEATH = 11,
		CIVVY_STATE_CLIMB_2CLICKS = 12,
		CIVVY_STATE_CLIMB_3CLICKS = 13,
		CIVVY_STATE_CLIMB_4CLICKS = 14,
		CIVVY_STATE_FALL_4CLICKS = 15
	};

	enum CivvyAnim
	{
		CIVVY_ANIM_WALK = 0,
		CIVVY_ANIM_WALKING_PUNCH_PREPARE = 1,
		CIVVY_ANIM_WALKING_PUNCH_TO_IDLE = 2,
		CIVVY_ANIM_WALKING_PUNCH_TO_WALK = 3,
		CIVVY_ANIM_WALKING_PUNCH_ATTACK = 4,
		CIVVY_ANIM_IDLE_TO_WALK = 5,
		CIVVY_ANIM_IDLE = 6,
		CIVVY_ANIM_WALKING_PUNCH_CANCEL = 7,
		CIVVY_ANIM_WALK_TO_IDLE_LEFT = 8,
		CIVVY_ANIM_CLOSE_PUNCH_CANCEL = 9,
		CIVVY_ANIM_CLOSE_PUNCH_PREPARE = 10,
		CIVVY_ANIM_FAR_PUNCH_CANCEL = 11,
		CIVVY_ANIM_FAR_PUNCH_PREPARE = 12,
		CIVVY_ANIM_CLOSE_PUNCH_TO_IDLE = 13,
		CIVVY_ANIM_FAR_PUNCH_TO_IDLE = 14,
		CIVVY_ANIM_FAR_PUNCH_TO_WALK = 15,
		CIVVY_ANIM_CLOSE_PUNCH_ATTACK = 16,
		CIVVY_ANIM_FAR_PUNCH_ATTACK = 17,
		CIVVY_ANIM_IDLE_TO_WAIT = 18,
		CIVVY_ANIM_WAIT_TO_IDLE = 19,
		CIVVY_ANIM_WAIT = 20,
		CIVVY_ANIM_RUN = 21,
		CIVVY_ANIM_WALK_TO_RUN = 22,
		CIVVY_ANIM_RUN_TO_WALK_LEFT = 23,
		CIVVY_ANIM_RUN_TO_IDLE = 24,
		CIVVY_ANIM_IDLE_TO_RUN = 25,
		CIVVY_ANIM_DEATH = 26,
		CIVVY_ANIM_CLIMB_4CLICKS = 27,
		CIVVY_ANIM_CLIMB_2CLICKS = 28,
		CIVVY_ANIM_CLIMB_3CLICKS = 29,
		CIVVY_ANIM_FALL_4CLICKS = 30,
		CIVVY_ANIM_RUN_TO_WALK_RIGHT = 31,
	};

	void InitialiseCivvy(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);
		SetAnimation(item, CIVVY_ANIM_IDLE);
	}

	ItemInfo* CivvyFindNearTarget(short itemNumber, int RangeDetection)
	{
		auto& item = g_Level.Items[itemNumber];
		auto& creature = *GetCreatureInfo(&item);

		ItemInfo* result = nullptr;

		auto closetsDistance = FLT_MAX;
		Vector3 distanceVector;
		float distanceValue;
		auto* targetCreature = ActiveCreatures[0];
		float MaxRange = RangeDetection <= 0 ? FLT_MAX : RangeDetection;

		for (int i = 0; i < ActiveCreatures.size(); i++)
		{
			targetCreature = ActiveCreatures[i];

			//Ignore if it's itself, or a non valid Item.
			if (targetCreature->ItemNumber == NO_ITEM || targetCreature->ItemNumber == itemNumber)
				continue;

			//Ignore if it's Lara, but was not hurt by her.
			if (g_Level.Items[targetCreature->ItemNumber].IsLara() && !creature.HurtByLara)
				continue;

			//Ignore if it's an entity from the Excluded Targets lists.
			bool forbiddenTarget = false;
			for (std::vector<GAME_OBJECT_ID>::iterator it = CivvyExcludedTargets.begin(); it != CivvyExcludedTargets.end(); ++it)
			{
				if (g_Level.Items[targetCreature->ItemNumber].ObjectNumber == *it)
					forbiddenTarget = true;
			}
			if (forbiddenTarget)
				continue;

			//If it's closer than other entity, choose this one.
			auto& currentItem = g_Level.Items[targetCreature->ItemNumber];

			distanceValue = Vector3i::Distance(item.Pose.Position, currentItem.Pose.Position);

			if (distanceValue < closetsDistance && distanceValue < MaxRange)
			{
				closetsDistance = distanceValue;
				result = &currentItem;
			}
		}
		return result;
	}

	void CivvyControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto& item = g_Level.Items[itemNumber];
		auto& object = Objects[item.ObjectNumber];
		auto& creature = *GetCreatureInfo(&item);

		short angle = 0;
		short tilt = 0;
		bool doDamageFlag = false;

		int targetAngle = 0;
		int targetDistance = 0;
		auto jointHeadRot = EulerAngles::Zero;
		auto jointTorsoRot = EulerAngles::Zero;

		if (item.BoxNumber != NO_BOX && (g_Level.Boxes[item.BoxNumber].flags & BLOCKED))
		{
			DoDamage(&item, 20);
			DoLotsOfBlood(item.Pose.Position.x, item.Pose.Position.y - (GetRandomControl() & 255) - 32, item.Pose.Position.z, (GetRandomControl() & 127) + 128, GetRandomControl() << 1, item.RoomNumber, 3);
		}

		if (item.HitPoints <= 0)
		{
			if (item.Animation.ActiveState != CIVVY_STATE_DEATH)
				SetAnimation(&item, CIVVY_ANIM_DEATH);
				//creature.LOT.Step = CLICK(1); //I don't know what is this for.
		}
		else
		{
			if (item.AIBits)
				GetAITarget(&creature);
			else
				creature.Enemy = CivvyFindNearTarget(itemNumber, 0);

			AI_INFO AI;
			CreatureAIInfo(&item, &AI);

			float distance2D = 0;
			int angle = 0;
			if (creature.Enemy->IsLara())
			{
				targetAngle = AI.angle;
				targetDistance = AI.distance;
			}
			else
			{
				int targetDx = creature.Enemy->Pose.Position.x - item.Pose.Position.x;
				int targetDz = creature.Enemy->Pose.Position.z - item.Pose.Position.z;
				targetAngle = phd_atan(targetDz, targetDx) - item.Pose.Orientation.y;
				targetDistance = SQUARE(targetDx) + SQUARE(targetDz);
			}

			//If Lara was placed by system, not because she were a real target. Then delete the target.
			if (creature.Enemy->IsLara() && !creature.HurtByLara)
				creature.Enemy = nullptr;

			GetCreatureMood(&item, &AI, false);

			if (creature.Enemy == LaraItem &&
				AI.distance > CIVVY_ESCAPE_RANGE &&
				AI.enemyFacing < ANGLE(67.0f) &&
				AI.enemyFacing > -ANGLE(67.0f))
			{
				creature.Mood = MoodType::Escape;
			}

			CreatureMood(&item, &AI, false);

			angle = CreatureTurn(&item, creature.MaxTurn);

			auto* realEnemy = creature.Enemy;
			creature.Enemy = LaraItem;

			if ((targetDistance < CIVVY_AWARE_RANGE || 
				 item.HitStatus || 
				 TargetVisible(&item, &AI)) &&
				 !(item.AIBits & FOLLOW))
			{
				if (!creature.Alerted)
					SoundEffect(SFX_TR3_AMERCAN_HOY, &item.Pose);

				AlertAllGuards(itemNumber);
			}
			creature.Enemy = realEnemy;

			switch (item.Animation.ActiveState)
			{
				case CIVVY_STATE_IDLE:
					creature.MaxTurn = 0;
					creature.Flags = 0;
					jointHeadRot.y = targetAngle;

					if (item.AIBits & GUARD)
					{
						jointHeadRot.y = AIGuard(&creature);

						if (!(GetRandomControl() & 0xFF))
						{
							if (item.Animation.ActiveState == CIVVY_STATE_IDLE)
								item.Animation.TargetState = CIVVY_STATE_WAIT;
							else
								item.Animation.TargetState = CIVVY_STATE_IDLE;
						}

						break;
					}

					else if (item.AIBits & PATROL1)
						item.Animation.TargetState = CIVVY_STATE_WALK;

					else if (creature.Mood == MoodType::Escape)
					{
						if (Lara.TargetEntity != &item && AI.ahead)
							item.Animation.TargetState = CIVVY_STATE_IDLE;
						else
							item.Animation.TargetState = CIVVY_STATE_RUN;
					}
					else if (creature.Mood == MoodType::Bored ||
						(item.AIBits & FOLLOW && (creature.ReachedGoal || targetDistance > SQUARE(BLOCK(2)))))
					{
						if (AI.ahead)
							item.Animation.TargetState = CIVVY_STATE_IDLE;
						else
							item.Animation.TargetState = CIVVY_STATE_WAIT;
					}
					else if (AI.distance < CIVVY_ATTACK_CLOSE_PUNCH_RANGE)
						item.Animation.TargetState = CIVVY_STATE_AIM_CLOSE_PUNCH;
					else if (AI.bite && AI.distance < CIVVY_ATTACK_FAR_PUNCH_RANGE)
						item.Animation.TargetState = CIVVY_STATE_AIM_FAR_PUNCH;
					else if (AI.bite && AI.distance < CIVVY_WALK_RANGE)
						item.Animation.TargetState = CIVVY_STATE_WALK;
					else
						item.Animation.TargetState = CIVVY_STATE_RUN;

					break;

				case CIVVY_STATE_WALK:
					creature.MaxTurn = CIVVY_WALK_TURN_RATE_MAX;
					jointHeadRot.y = targetAngle;

					if (item.AIBits & PATROL1)
					{
						jointHeadRot.y = 0;
						item.Animation.TargetState = CIVVY_STATE_WALK;
					}
					else if (creature.Mood == MoodType::Escape)
						item.Animation.TargetState = CIVVY_STATE_RUN;
					else if (creature.Mood == MoodType::Bored)
					{
						if (TestProbability(CIVVY_WAIT_CHANCE))
							item.Animation.TargetState = CIVVY_STATE_WAIT;
					}
					else if (AI.distance < CIVVY_ATTACK_CLOSE_PUNCH_RANGE && creature.Enemy->Animation.Velocity.z < CIVVY_TARGET_ALERT_VELOCITY)
						item.Animation.TargetState = CIVVY_STATE_IDLE;
					else if (AI.bite && AI.distance < CIVVY_ATTACK_WALKING_PUNCH_RANGE)
						item.Animation.TargetState = CIVVY_STATE_AIM_WALKING_PUNCH;
					else if (AI.distance > CIVVY_WALK_RANGE)
						item.Animation.TargetState = CIVVY_STATE_RUN;
					else
						item.Animation.TargetState = CIVVY_STATE_WALK;

					break;

				case CIVVY_STATE_WAIT:
					if (creature.Alerted)
						item.Animation.TargetState = CIVVY_STATE_IDLE;
					break;

				case CIVVY_STATE_RUN:
					creature.MaxTurn = CIVVY_RUN_TURN_RATE_MAX;
					tilt = angle / 2;

					if (AI.ahead)
						jointHeadRot.y = AI.angle;

					if (item.AIBits & GUARD)
						item.Animation.TargetState = CIVVY_STATE_WAIT;
					//else if (creature.Mood == MoodType::Escape && Lara.TargetEntity != &item && AI.ahead)
						//item.Animation.TargetState = CIVVY_STATE_IDLE;
					else if ((item.AIBits & FOLLOW) && (creature.ReachedGoal || targetDistance > SQUARE(SECTOR(2))))
						item.Animation.TargetState = CIVVY_STATE_IDLE;
					else if (creature.Mood == MoodType::Bored)
						item.Animation.TargetState = CIVVY_STATE_WALK;
					else if (AI.ahead && AI.distance <= CIVVY_WALK_RANGE)
						item.Animation.TargetState = CIVVY_STATE_WALK;

					break;

				case CIVVY_STATE_AIM_CLOSE_PUNCH:
					creature.MaxTurn = CIVVY_AIM_TURN_RATE_MAX;
					creature.Flags = 0;

					if (AI.ahead)
					{
						jointTorsoRot.x = AI.xAngle;
						jointTorsoRot.y = AI.angle;
					}

					if (AI.bite && AI.distance < CIVVY_ATTACK_CLOSE_PUNCH_RANGE)
						item.Animation.TargetState = CIVVY_STATE_ATTACK_CLOSE_PUNCH;
					else
						item.Animation.TargetState = CIVVY_STATE_IDLE;

					break;

				case CIVVY_STATE_AIM_FAR_PUNCH:
					creature.MaxTurn = CIVVY_AIM_TURN_RATE_MAX;
					creature.Flags = 0;

					if (AI.ahead)
					{
						jointTorsoRot.x = AI.xAngle;
						jointTorsoRot.y = AI.angle;
					}

					if (AI.ahead && AI.distance < CIVVY_ATTACK_FAR_PUNCH_RANGE)
						item.Animation.TargetState = CIVVY_STATE_ATTACK_FAR_PUNCH;
					else
						item.Animation.TargetState = CIVVY_STATE_IDLE;

					break;

				case CIVVY_STATE_AIM_WALKING_PUNCH:
					creature.MaxTurn = CIVVY_AIM_TURN_RATE_MAX;
					creature.Flags = 0;

					if (AI.ahead)
					{
						jointTorsoRot.x = AI.xAngle;
						jointTorsoRot.y = AI.angle;
					}

					if (AI.bite && AI.distance < CIVVY_ATTACK_WALKING_PUNCH_RANGE)
						item.Animation.TargetState = CIVVY_STATE_ATTACK_WALKING_PUNCH;
					else
						item.Animation.TargetState = CIVVY_STATE_WALK;

					break;

				case CIVVY_STATE_ATTACK_CLOSE_PUNCH:
					creature.MaxTurn = CIVVY_WALK_TURN_RATE_MAX;

					if (AI.ahead)
					{
						jointTorsoRot.x = AI.xAngle;
						jointTorsoRot.y = AI.angle;
					}

					doDamageFlag = false;

					if (creature.Flags == 0 &&
						item.Animation.AnimNumber == GetAnimNumber(item, CIVVY_ANIM_CLOSE_PUNCH_ATTACK))
					{
						if (creature.Enemy->IsLara())
						{
							if (item.TouchBits.Test(CivvyAttackJoints))
								doDamageFlag = true;
						}
						else
						{
							float distance = Vector3i::Distance(item.Pose.Position, creature.Enemy->Pose.Position);
							if (distance <= CLICK(2))
								doDamageFlag = true;
						}

						if (doDamageFlag)
						{
							DoDamage(creature.Enemy, CIVVY_ATTACK_DAMAGE);
							CreatureEffect(&item, CivvyBiteLeft, DoBloodSplat);
							SoundEffect(SFX_TR4_LARA_THUD, &item.Pose);
							creature.Flags = 1;
							TENLog("CLOSE PUNCH", LogLevel::Info, LogConfig::All, true);
						}
					}

					break;

				case CIVVY_STATE_ATTACK_FAR_PUNCH:
					creature.MaxTurn = CIVVY_WALK_TURN_RATE_MAX;

					if (AI.ahead)
					{
						jointTorsoRot.x = AI.xAngle;
						jointTorsoRot.y = AI.angle;
					}

					doDamageFlag = false;

					if (creature.Flags == 0 &&
						item.Animation.AnimNumber == GetAnimNumber(item, CIVVY_ANIM_FAR_PUNCH_ATTACK))
					{
						if (creature.Enemy->IsLara())
						{
							if (item.TouchBits.Test(CivvyAttackJoints))
								doDamageFlag = true;
						}
						else
						{
							float distance = Vector3i::Distance(item.Pose.Position, creature.Enemy->Pose.Position);
							if (distance <= CLICK(2))
								doDamageFlag = true;
						}

						if (doDamageFlag)
						{
							DoDamage(creature.Enemy, CIVVY_ATTACK_DAMAGE);
							CreatureEffect(&item, CivvyBiteLeft, DoBloodSplat);
							SoundEffect(SFX_TR4_LARA_THUD, &item.Pose);
							creature.Flags = 1;
							TENLog("FAR PUNCH", LogLevel::Info, LogConfig::All, true);
						}
					}

					if (AI.ahead && AI.distance > CIVVY_ATTACK_FAR_PUNCH_RANGE)
						item.Animation.TargetState = CIVVY_STATE_WALK;

					break;

				case CIVVY_STATE_ATTACK_WALKING_PUNCH:
					creature.MaxTurn = CIVVY_WALK_TURN_RATE_MAX;

					if (AI.ahead)
					{
						jointTorsoRot.x = AI.xAngle;
						jointTorsoRot.y = AI.angle;
					}

					doDamageFlag = false;

					if (creature.Flags == 0 &&
						item.Animation.AnimNumber == GetAnimNumber(item, CIVVY_ANIM_WALKING_PUNCH_ATTACK))
					{
						if (creature.Enemy->IsLara())
						{
							if (item.TouchBits.Test(CivvyAttackJoints))
								doDamageFlag = true;
						}
						else
						{
							float distance = Vector3i::Distance(item.Pose.Position, creature.Enemy->Pose.Position);
							if (distance <= CLICK(2))
								doDamageFlag = true;
						}

						if (doDamageFlag)
						{
							DoDamage(creature.Enemy, CIVVY_ATTACK_DAMAGE);
							CreatureEffect(&item, CivvyBiteLeft, DoBloodSplat);
							SoundEffect(SFX_TR4_LARA_THUD, &item.Pose);
							creature.Flags = 1;
							TENLog("WALKING PUNCH", LogLevel::Info, LogConfig::All, true);
						}
					}

					break;
			}
		}

		CreatureTilt(&item, tilt);
		CreatureJoint(&item, 0, jointTorsoRot.y);
		CreatureJoint(&item, 1, jointTorsoRot.x);
		CreatureJoint(&item, 2, jointHeadRot.y);

		if (item.Animation.ActiveState < CIVVY_STATE_DEATH)
		{
			switch (CreatureVault(itemNumber, angle, 2, CIVVY_VAULT_SHIFT))
			{
			case 2:
				creature.MaxTurn = 0;
				SetAnimation(&item, CIVVY_ANIM_CLIMB_2CLICKS);
				break;

			case 3:
				creature.MaxTurn = 0;
				SetAnimation(&item, CIVVY_ANIM_CLIMB_3CLICKS);
				break;

			case 4:
				creature.MaxTurn = 0;
				SetAnimation(&item, CIVVY_ANIM_CLIMB_4CLICKS);
				break;

			case -2:
				creature.MaxTurn = 0;
				SetAnimation(&item, CIVVY_ANIM_FALL_4CLICKS);
				break;

			case -3:
				creature.MaxTurn = 0;
				SetAnimation(&item, CIVVY_ANIM_FALL_4CLICKS);
				break;

			case -4:
				creature.MaxTurn = 0;
				SetAnimation(&item, CIVVY_ANIM_FALL_4CLICKS);
				break;
			}
		}
		else
		{
			creature.MaxTurn = 0;
			CreatureAnimation(itemNumber, angle, 0);
		}
	}
}
