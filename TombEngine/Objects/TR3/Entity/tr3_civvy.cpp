#include "framework.h"
#include "Objects/TR3/Entity/tr3_civvy.h"

#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using std::vector;

namespace TEN::Entities::TR3
{
	BITE_INFO CivvyBite = { 0, 0, 0, 13 };
	const vector<int> CivvyAttackJoints = { 10, 13 };

	constexpr auto CIVVY_ATTACK_DAMAGE = 40;
	constexpr auto CIVVY_SWIPE_DAMAGE = 50;

	constexpr auto CIVVY_ATTACK0_RANGE = SECTOR(3);
	constexpr auto CIVVY_ATTACK1_RANGE = SECTOR(0.67f);
	constexpr auto CIVVY_ATTACK2_RANGE = SECTOR(1);
	constexpr auto CIVVY_WALK_RANGE = SECTOR(1);
	constexpr auto CIVVY_ESCAPE_RANGE = SECTOR(3);
	constexpr auto CIVVY_AWARE_RANGE = SECTOR(1);

	constexpr auto CIVVY_STATE_WALK_FORWARD_CHANCE = 0x100;
	constexpr auto CIVVY_WAIT_CHANCE = 0x100;

	constexpr auto CIVVY_VAULT_SHIFT = 260;

	#define CIVVY_STATE_WALK_FORWARD_TURN_ANGLE ANGLE(5.0f)
	#define CIVVY_STATE_RUN_FORWARD_TURN_ANGLE ANGLE(6.0f)

	#define CIVVY_CLIMB1_ANIM 28
	#define CIVVY_CLIMB2_ANIM 29
	#define CIVVY_CLIMB3_ANIM 27
	#define CIVVY_FALL3_ANIM  30

	// TODO
	enum CivvyState
	{
		CIVVY_STATE_NONE,
		CIVVY_STATE_IDLE,
		CIVVY_STATE_WALK_FORWARD,
		CIVVY_PUNCH2,
		CIVVY_AIM2,
		CIVVY_WAIT,
		CIVVY_AIM1,
		CIVVY_AIM0,
		CIVVY_PUNCH1,
		CIVVY_PUNCH0,
		CIVVY_STATE_RUN_FORWARD,
		CIVVY_DEATH,
		CIVVY_CLIMB3,
		CIVVY_CLIMB1,
		CIVVY_CLIMB2,
		CIVVY_FALL3
	};

	// TODO
	enum CivvyAnim
	{

		CIVVY_ANIM_IDLE = 6,

		CIVVY_ANIM_DEATH = 26,
	};

	void InitialiseCivvy(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_ANIM_IDLE;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = item->Animation.TargetState = CIVVY_STATE_IDLE;
	}

	void CivvyControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short torsoX = 0;
		short torsoY = 0;
		short head = 0;
		short angle = 0;
		short tilt = 0;

		if (item->BoxNumber != NO_BOX && (g_Level.Boxes[item->BoxNumber].flags & BLOCKED))
		{
			DoLotsOfBlood(item->Pose.Position.x, item->Pose.Position.y - (GetRandomControl() & 255) - 32, item->Pose.Position.z, (GetRandomControl() & 127) + 128, GetRandomControl() << 1, item->RoomNumber, 3);
			DoDamage(item, 20);
		}

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != CIVVY_DEATH)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_ANIM_DEATH;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = CIVVY_DEATH;
				creature->LOT.Step = CLICK(1);
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);
			else
				creature->Enemy = LaraItem;

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			AI_INFO laraAiInfo;
			if (creature->Enemy == LaraItem)
			{
				laraAiInfo.angle = AI.angle;
				laraAiInfo.distance = AI.distance;
			}
			else
			{
				int laraDz = LaraItem->Pose.Position.z - item->Pose.Position.z;
				int laraDx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				laraAiInfo.angle = phd_atan(laraDz, laraDx) - item->Pose.Orientation.y;
				laraAiInfo.distance = pow(laraDx, 2) + pow(laraDz, 2);
			}

			GetCreatureMood(item, &AI, VIOLENT);

			if (creature->Enemy == LaraItem &&
				AI.distance > pow(CIVVY_ESCAPE_RANGE, 2) &&
				AI.enemyFacing < ANGLE(67.0f) &&
				AI.enemyFacing > -ANGLE(67.0f))
			{
				creature->Mood = MoodType::Escape;
			}

			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, creature->MaxTurn);

			auto* realEnemy = creature->Enemy;
			creature->Enemy = LaraItem;

			if ((laraAiInfo.distance < pow(CIVVY_AWARE_RANGE, 2) || item->HitStatus || TargetVisible(item, &laraAiInfo)) &&
				!(item->AIBits & FOLLOW))
			{
				if (!creature->Alerted)
					SoundEffect(SFX_TR3_AMERCAN_HOY, &item->Pose);

				AlertAllGuards(itemNumber);
			}
			creature->Enemy = realEnemy;

			switch (item->Animation.ActiveState)
			{
			case CIVVY_WAIT:
				if (creature->Alerted || item->Animation.TargetState == CIVVY_STATE_RUN_FORWARD)
				{
					item->Animation.TargetState = CIVVY_STATE_IDLE;
					break;
				}

			case CIVVY_STATE_IDLE:
				creature->MaxTurn = 0;
				creature->Flags = 0;
				head = laraAiInfo.angle;

				if (item->AIBits & GUARD)
				{
					head = AIGuard(creature);
					if (!(GetRandomControl() & 0xFF))
					{
						if (item->Animation.ActiveState == CIVVY_STATE_IDLE)
							item->Animation.TargetState = CIVVY_WAIT;
						else
							item->Animation.TargetState = CIVVY_STATE_IDLE;
					}

					break;
				}

				else if (item->AIBits & PATROL1)
					item->Animation.TargetState = CIVVY_STATE_WALK_FORWARD;

				else if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead)
						item->Animation.TargetState = CIVVY_STATE_IDLE;
					else
						item->Animation.TargetState = CIVVY_STATE_RUN_FORWARD;
				}
				else if (creature->Mood == MoodType::Bored ||
					(item->AIBits & FOLLOW && (creature->ReachedGoal || laraAiInfo.distance > pow(SECTOR(2), 2))))
				{
					if (item->Animation.RequiredState)
						item->Animation.TargetState = item->Animation.RequiredState;
					else if (AI.ahead)
						item->Animation.TargetState = CIVVY_STATE_IDLE;
					else
						item->Animation.TargetState = CIVVY_STATE_RUN_FORWARD;
				}
				else if (AI.bite && AI.distance < pow(CIVVY_ATTACK0_RANGE, 2))
					item->Animation.TargetState = CIVVY_AIM0;
				else if (AI.bite && AI.distance < pow(CIVVY_ATTACK1_RANGE, 2))
					item->Animation.TargetState = CIVVY_AIM1;
				else if (AI.bite && AI.distance < pow(CIVVY_WALK_RANGE, 2))
					item->Animation.TargetState = CIVVY_STATE_WALK_FORWARD;
				else
					item->Animation.TargetState = CIVVY_STATE_RUN_FORWARD;

				break;

			case CIVVY_STATE_WALK_FORWARD:
				creature->MaxTurn = CIVVY_STATE_WALK_FORWARD_TURN_ANGLE;
				head = laraAiInfo.angle;

				if (item->AIBits & PATROL1)
				{
					item->Animation.TargetState = CIVVY_STATE_WALK_FORWARD;
					head = 0;
				}
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = CIVVY_STATE_RUN_FORWARD;
				else if (creature->Mood == MoodType::Bored)
				{
					if (GetRandomControl() < CIVVY_WAIT_CHANCE)
					{
						item->Animation.RequiredState = CIVVY_WAIT;
						item->Animation.TargetState = CIVVY_STATE_IDLE;
					}
				}
				else if (AI.bite && AI.distance < pow(CIVVY_ATTACK0_RANGE, 2))
					item->Animation.TargetState = CIVVY_STATE_IDLE;
				else if (AI.bite && AI.distance < pow(CIVVY_ATTACK2_RANGE, 2))
					item->Animation.TargetState = CIVVY_AIM2;
				else
					item->Animation.TargetState = CIVVY_STATE_RUN_FORWARD;

				break;

			case CIVVY_STATE_RUN_FORWARD:
				creature->MaxTurn = CIVVY_STATE_RUN_FORWARD_TURN_ANGLE;
				tilt = angle / 2;

				if (AI.ahead)
					head = AI.angle;

				if (item->AIBits & GUARD)
					item->Animation.TargetState = CIVVY_WAIT;
				else if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead)
						item->Animation.TargetState = CIVVY_STATE_IDLE;
					break;
				}
				else if ((item->AIBits & FOLLOW) && (creature->ReachedGoal || laraAiInfo.distance > pow(SECTOR(2), 2)))
					item->Animation.TargetState = CIVVY_STATE_IDLE;
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = CIVVY_STATE_WALK_FORWARD;
				else if (AI.ahead && AI.distance < pow(CIVVY_WALK_RANGE, 2))
					item->Animation.TargetState = CIVVY_STATE_WALK_FORWARD;

				break;

			case CIVVY_AIM0:
				creature->MaxTurn = CIVVY_STATE_WALK_FORWARD_TURN_ANGLE;

				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (AI.bite && AI.distance < pow(CIVVY_ATTACK0_RANGE, 2))
					item->Animation.TargetState = CIVVY_PUNCH0;
				else
					item->Animation.TargetState = CIVVY_STATE_IDLE;

				creature->Flags = 0;
				break;

			case CIVVY_AIM1:
				creature->MaxTurn = CIVVY_STATE_WALK_FORWARD_TURN_ANGLE;

				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (AI.ahead && AI.distance < pow(CIVVY_ATTACK1_RANGE, 2))
					item->Animation.TargetState = CIVVY_PUNCH1;
				else
					item->Animation.TargetState = CIVVY_STATE_IDLE;

				creature->Flags = 0;
				break;

			case CIVVY_AIM2:
				creature->MaxTurn = CIVVY_STATE_WALK_FORWARD_TURN_ANGLE;
				creature->Flags = 0;

				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (AI.bite && AI.distance < pow(CIVVY_ATTACK2_RANGE, 2))
					item->Animation.TargetState = CIVVY_PUNCH2;
				else
					item->Animation.TargetState = CIVVY_STATE_WALK_FORWARD;

				break;

			case CIVVY_PUNCH0:
				creature->MaxTurn = CIVVY_STATE_WALK_FORWARD_TURN_ANGLE;

				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (!creature->Flags && item->TestBits(JointBitType::Touch, CivvyAttackJoints))
				{
					CreatureEffect(item, &CivvyBite, DoBloodSplat);
					DoDamage(creature->Enemy, CIVVY_ATTACK_DAMAGE);
					SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
					creature->Flags = 1;
				}

				break;

			case CIVVY_PUNCH1:
				creature->MaxTurn = CIVVY_STATE_WALK_FORWARD_TURN_ANGLE;

				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (!creature->Flags && item->TestBits(JointBitType::Touch, CivvyAttackJoints))
				{
					CreatureEffect(item, &CivvyBite, DoBloodSplat);
					DoDamage(creature->Enemy, CIVVY_ATTACK_DAMAGE);
					SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
					creature->Flags = 1;
				}

				if (AI.ahead && AI.distance > pow(CIVVY_ATTACK1_RANGE, 2) && AI.distance < pow(CIVVY_ATTACK2_RANGE, 2))
					item->Animation.TargetState = CIVVY_PUNCH2;

				break;

			case CIVVY_PUNCH2:
				creature->MaxTurn = CIVVY_STATE_WALK_FORWARD_TURN_ANGLE;

				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (creature->Flags != 2 && item->TestBits(JointBitType::Touch, CivvyAttackJoints))
				{
					CreatureEffect(item, &CivvyBite, DoBloodSplat);
					DoDamage(creature->Enemy, CIVVY_SWIPE_DAMAGE);
					SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
					creature->Flags = 2;
				}

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, torsoY);
		CreatureJoint(item, 1, torsoX);
		CreatureJoint(item, 2, head);

		if (item->Animation.ActiveState < CIVVY_DEATH)
		{
			switch (CreatureVault(itemNumber, angle, 2, CIVVY_VAULT_SHIFT))
			{
			case 2:
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_CLIMB1_ANIM;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = CIVVY_CLIMB1;
				break;

			case 3:
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_CLIMB2_ANIM;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = CIVVY_CLIMB2;
				break;

			case 4:
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_CLIMB3_ANIM;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = CIVVY_CLIMB3;
				break;

			case -4:
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + CIVVY_FALL3_ANIM;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = CIVVY_FALL3;
				break;
			}
		}
		else
		{
			creature->MaxTurn = 0;
			CreatureAnimation(itemNumber, angle, 0);
		}
	}
}
