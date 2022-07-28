#include "framework.h"
#include "Objects/TR3/Entity/tr3_mp_stick.h"

#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using std::vector;

namespace TEN::Entities::TR3
{
	BITE_INFO MPStickBite1 = { 247, 10, 11, 13 };
	BITE_INFO MPStickBite2 = { 0, 0, 100, 6 };
	const vector<int> MPStickPunchAttackJoints = { 10, 13 };
	const vector<int> MPStickKickAttackJoints = { 5, 6 };

	enum MPStickState
	{
		MPSTICK_STATE_NONE,
		MPSTICK_STATE_STOP,
		MPSTICK_STATE_WALK,
		MPSTICK_STATE_PUNCH2,
		MPSTICK_STATE_AIM2,
		MPSTICK_STATE_WAIT,
		MPSTICK_STATE_AIM1,
		MPSTICK_STATE_AIM0,
		MPSTICK_STATE_PUNCH1,
		MPSTICK_STATE_PUNCH0,
		MPSTICK_STATE_RUN,
		MPSTICK_STATE_DEATH,
		MPSTICK_STATE_KICK,
		MPSTICK_STATE_CLIMB3,
		MPSTICK_STATE_CLIMB1,
		MPSTICK_STATE_CLIMB2,
		MPSTICK_STATE_FALL3
	};

	// TODO
	enum MPStickAnim
	{

	};

	void InitialiseMPStick(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->Animation.AnimNumber = Objects[ID_MP_WITH_STICK].animIndex + 6;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = item->Animation.TargetState = MPSTICK_STATE_STOP;
	}

	void MPStickControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short head = 0;
		short angle = 0;
		short tilt = 0;
		short torsoX = 0;
		short torsoY = 0;

		if (item->BoxNumber != NO_BOX && (g_Level.Boxes[item->BoxNumber].flags & BLOCKED))
		{
			DoDamage(item, 20);
			DoLotsOfBlood(item->Pose.Position.x, item->Pose.Position.y - (GetRandomControl() & 255) - 32, item->Pose.Position.z, (GetRandomControl() & 127) + 128, GetRandomControl() * 2, item->RoomNumber, 3);
		}

		AI_INFO laraAI;
		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != MPSTICK_STATE_DEATH)
			{
				item->Animation.AnimNumber = Objects[ID_MP_WITH_STICK].animIndex + 26;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = MPSTICK_STATE_DEATH;
				creature->LOT.Step = 256;
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);
			else
			{
				creature->Enemy = LaraItem;

				int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
				laraAI.distance = pow(dx, 2) + pow(dx, 2);

				int bestDistance = 0x7fffffff;
				for (int slot = 0; slot < ActiveCreatures.size(); slot++)
				{
					auto* currentCreature = ActiveCreatures[slot];
					if (currentCreature->ItemNumber == NO_ITEM || currentCreature->ItemNumber == itemNumber)
						continue;

					auto* target = &g_Level.Items[currentCreature->ItemNumber];
					if (target->ObjectNumber != ID_LARA)
						continue;

					dx = target->Pose.Position.x - item->Pose.Position.x;
					dz = target->Pose.Position.z - item->Pose.Position.z;

					if (dz > SECTOR(31.25f) || dz < -SECTOR(31.25f) ||
						dx > SECTOR(31.25f) || dx < -SECTOR(31.25f))
					{
						continue;
					}

					int distance = pow(dx, 2) + pow(dz, 2);
					if (distance < bestDistance && distance < laraAI.distance)
					{
						bestDistance = distance;
						creature->Enemy = target;
					}
				}
			}

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (creature->Enemy == LaraItem)
			{
				laraAI.angle = AI.angle;
				laraAI.distance = AI.distance;
			}
			else
			{
				int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
				laraAI.angle = phd_atan(dz, dx) - item->Pose.Orientation.y;
				laraAI.distance = pow(dx, 2) + pow(dz, 2);
			}

			GetCreatureMood(item, &AI, VIOLENT);
			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, creature->MaxTurn);

			auto* enemy = creature->Enemy;
			creature->Enemy = LaraItem;

			if (item->HitStatus || ((laraAI.distance < pow(SECTOR(1), 2) ||
				TargetVisible(item, &laraAI)) && abs(LaraItem->Pose.Position.y - item->Pose.Position.y) < SECTOR(1)))
			{
				if (!creature->Alerted)
					SoundEffect(SFX_TR3_AMERCAN_HOY, &item->Pose);

				AlertAllGuards(itemNumber);
			}

			creature->Enemy = enemy;

			switch (item->Animation.ActiveState)
			{
			case MPSTICK_STATE_WAIT:
				if (creature->Alerted || item->Animation.TargetState == MPSTICK_STATE_RUN)
				{
					item->Animation.TargetState = MPSTICK_STATE_STOP;
					break;
				}

			case MPSTICK_STATE_STOP:
				creature->MaxTurn = 0;
				creature->Flags = 0;
				head = laraAI.angle;

				if (item->AIBits & GUARD)
				{
					head = AIGuard(creature);
					if (!(GetRandomControl() & 0xFF))
					{
						if (item->Animation.ActiveState == MPSTICK_STATE_STOP)
							item->Animation.TargetState = MPSTICK_STATE_WAIT;
						else
							item->Animation.TargetState = MPSTICK_STATE_STOP;
					}

					break;
				}

				else if (item->AIBits & PATROL1)
					item->Animation.TargetState = MPSTICK_STATE_WALK;

				else if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead && !item->HitStatus)
						item->Animation.TargetState = MPSTICK_STATE_STOP;
					else
						item->Animation.TargetState = MPSTICK_STATE_RUN;
				}
				else if (creature->Mood == MoodType::Bored ||
					(item->AIBits & FOLLOW && (creature->ReachedGoal || laraAI.distance > pow(SECTOR(2), 2))))
				{
					if (item->Animation.RequiredState)
						item->Animation.TargetState = item->Animation.RequiredState;
					else if (AI.ahead)
						item->Animation.TargetState = MPSTICK_STATE_STOP;
					else
						item->Animation.TargetState = MPSTICK_STATE_RUN;
				}
				else if (AI.bite && AI.distance < pow(SECTOR(0.5f), 2))
					item->Animation.TargetState = MPSTICK_STATE_AIM0;
				else if (AI.bite && AI.distance < pow(SECTOR(1), 2))
					item->Animation.TargetState = MPSTICK_STATE_AIM1;
				else if (AI.bite && AI.distance < pow(SECTOR(1), 2))
					item->Animation.TargetState = MPSTICK_STATE_WALK;
				else
					item->Animation.TargetState = MPSTICK_STATE_RUN;

				break;

			case MPSTICK_STATE_WALK:
				creature->MaxTurn = ANGLE(6.0f);
				creature->Flags = 0;
				head = laraAI.angle;

				if (item->AIBits & PATROL1)
				{
					item->Animation.TargetState = MPSTICK_STATE_WALK;
					head = 0;
				}
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = MPSTICK_STATE_RUN;
				else if (creature->Mood == MoodType::Bored)
				{
					if (GetRandomControl() < 0x100)
					{
						item->Animation.RequiredState = MPSTICK_STATE_WAIT;
						item->Animation.TargetState = MPSTICK_STATE_STOP;
					}
				}
				else if (AI.bite && AI.distance < pow(SECTOR(1.5f), 2) && AI.xAngle < 0)
					item->Animation.TargetState = MPSTICK_STATE_KICK;
				else if (AI.bite && AI.distance < pow(SECTOR(0.5f), 2))
					item->Animation.TargetState = MPSTICK_STATE_STOP;
				else if (AI.bite && AI.distance < pow(SECTOR(1.25f), 2))
					item->Animation.TargetState = MPSTICK_STATE_AIM2;
				else
					item->Animation.TargetState = MPSTICK_STATE_RUN;

				break;

			case MPSTICK_STATE_RUN:
				creature->MaxTurn = ANGLE(7.0f);
				tilt = angle / 2;

				if (AI.ahead)
					head = AI.angle;

				if (item->AIBits & GUARD)
					item->Animation.TargetState = MPSTICK_STATE_WAIT;
				else if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead)
						item->Animation.TargetState = MPSTICK_STATE_STOP;

					break;
				}
				else if (item->AIBits & FOLLOW && (creature->ReachedGoal || laraAI.distance > pow(SECTOR(2), 2)))
					item->Animation.TargetState = MPSTICK_STATE_STOP;
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = MPSTICK_STATE_WALK;
				else if (AI.ahead && AI.distance < pow(SECTOR(1), 2))
					item->Animation.TargetState = MPSTICK_STATE_WALK;

				break;

			case MPSTICK_STATE_AIM0:
				creature->MaxTurn = ANGLE(6.0f);
				creature->Flags = 0;

				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (AI.bite && AI.distance < pow(SECTOR(0.5f), 2))
					item->Animation.TargetState = MPSTICK_STATE_PUNCH0;
				else
					item->Animation.TargetState = MPSTICK_STATE_STOP;

				break;

			case MPSTICK_STATE_AIM1:
				creature->MaxTurn = ANGLE(6.0f);
				creature->Flags = 0;

				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (AI.ahead && AI.distance < pow(SECTOR(1), 2))
					item->Animation.TargetState = MPSTICK_STATE_PUNCH1;
				else
					item->Animation.TargetState = MPSTICK_STATE_STOP;

				break;

			case MPSTICK_STATE_AIM2:
				creature->MaxTurn = ANGLE(6.0f);
				creature->Flags = 0;

				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (AI.bite && AI.distance < pow(SECTOR(1.25f), 2))
					item->Animation.TargetState = MPSTICK_STATE_PUNCH2;
				else
					item->Animation.TargetState = MPSTICK_STATE_WALK;

				break;

			case MPSTICK_STATE_PUNCH0:
				creature->MaxTurn = ANGLE(6.0f);

				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (enemy->IsLara())
				{
					if (!creature->Flags && item->TestBits(JointBitType::Touch, MPStickPunchAttackJoints))
					{
						CreatureEffect(item, &MPStickBite1, DoBloodSplat);
						DoDamage(enemy, 80);
						SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
						creature->Flags = 1;
					}
				}
				else
				{
					if (!creature->Flags && enemy)
					{
						if (abs(enemy->Pose.Position.x - item->Pose.Position.x) < SECTOR(0.25f) &&
							abs(enemy->Pose.Position.y - item->Pose.Position.y) <= SECTOR(0.25f) &&
							abs(enemy->Pose.Position.z - item->Pose.Position.z) < SECTOR(0.25f))
						{
							creature->Flags = 1;
							CreatureEffect(item, &MPStickBite1, DoBloodSplat);
							DoDamage(enemy, 5);
							SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
						}
					}
				}

				break;

			case MPSTICK_STATE_PUNCH1:
				creature->MaxTurn = ANGLE(6.0f);

				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (enemy->IsLara())
				{
					if (!creature->Flags && item->TestBits(JointBitType::Touch, MPStickPunchAttackJoints))
					{
						CreatureEffect(item, &MPStickBite1, DoBloodSplat);
						DoDamage(enemy, 80);
						SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
						creature->Flags = 1;
					}
				}
				else
				{
					if (!creature->Flags && enemy)
					{
						if (abs(enemy->Pose.Position.x - item->Pose.Position.x) < SECTOR(0.25f) &&
							abs(enemy->Pose.Position.y - item->Pose.Position.y) <= SECTOR(0.25f) &&
							abs(enemy->Pose.Position.z - item->Pose.Position.z) < SECTOR(0.25f))
						{
							creature->Flags = 1;
							CreatureEffect(item, &MPStickBite1, DoBloodSplat);
							DoDamage(enemy, 5);
							SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
						}
					}
				}

				if (AI.ahead && AI.distance > pow(SECTOR(1), 2) && AI.distance < pow(SECTOR(1.25f), 2))
					item->Animation.TargetState = MPSTICK_STATE_PUNCH2;

				break;

			case MPSTICK_STATE_PUNCH2:
				creature->MaxTurn = ANGLE(6.0f);

				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (enemy->IsLara())
				{
					if (creature->Flags != 2 && item->TestBits(JointBitType::Touch, MPStickPunchAttackJoints))
					{
						CreatureEffect(item, &MPStickBite1, DoBloodSplat);
						DoDamage(enemy, 100);
						creature->Flags = 2;
						SoundEffect(70, &item->Pose);
					}
				}
				else
				{
					if (creature->Flags != 2 && enemy)
					{
						if (abs(enemy->Pose.Position.x - item->Pose.Position.x) < SECTOR(0.25f) &&
							abs(enemy->Pose.Position.y - item->Pose.Position.y) <= SECTOR(0.25f) &&
							abs(enemy->Pose.Position.z - item->Pose.Position.z) < SECTOR(0.25f))
						{
							creature->Flags = 2;
							CreatureEffect(item, &MPStickBite1, DoBloodSplat);
							DoDamage(enemy, 6);
							SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
						}
					}
				}

				break;

			case MPSTICK_STATE_KICK:
				creature->MaxTurn = ANGLE(6.0f);

				if (AI.ahead)
					torsoY = AI.angle;

				if (enemy->IsLara())
				{
					if (creature->Flags != 1 && item->TestBits(JointBitType::Touch, MPStickKickAttackJoints) &&
						item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 8)
					{
						CreatureEffect(item, &MPStickBite2, DoBloodSplat);
						DoDamage(enemy, 150);
						SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
						creature->Flags = 1;
					}
				}
				else
				{
					if (!creature->Flags != 1 && enemy &&
						item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 8)
					{
						if (abs(enemy->Pose.Position.x - item->Pose.Position.x) < SECTOR(0.25f) &&
							abs(enemy->Pose.Position.y - item->Pose.Position.y) <= SECTOR(0.25f) &&
							abs(enemy->Pose.Position.z - item->Pose.Position.z) < SECTOR(0.25f))
						{
							creature->Flags = 1;
							CreatureEffect(item, &MPStickBite2, DoBloodSplat);
							DoDamage(enemy, 9);
							SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
						}
					}
				}

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, torsoY);
		CreatureJoint(item, 1, torsoX);
		CreatureJoint(item, 2, head);

		if (item->Animation.ActiveState < MPSTICK_STATE_DEATH)
		{
			switch (CreatureVault(itemNumber, angle, 2, 260))
			{
			case 2:
				item->Animation.AnimNumber = Objects[ID_MP_WITH_STICK].animIndex + 28;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = MPSTICK_STATE_CLIMB1;
				creature->MaxTurn = 0;
				break;

			case 3:
				item->Animation.AnimNumber = Objects[ID_MP_WITH_STICK].animIndex + 29;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = MPSTICK_STATE_CLIMB2;
				creature->MaxTurn = 0;
				break;

			case 4:
				item->Animation.AnimNumber = Objects[ID_MP_WITH_STICK].animIndex + 27;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = MPSTICK_STATE_CLIMB3;
				creature->MaxTurn = 0;
				break;

			case -4:
				item->Animation.AnimNumber = Objects[ID_MP_WITH_STICK].animIndex + 30;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = MPSTICK_STATE_FALL3;
				creature->MaxTurn = 0;
				break;
			}
		}
		else
		{
			creature->MaxTurn = 0;
			CreatureAnimation(itemNumber, angle, tilt);
		}
	}
}
