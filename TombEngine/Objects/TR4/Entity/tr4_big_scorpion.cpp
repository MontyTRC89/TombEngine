#include "framework.h"
#include "tr4_big_scorpion.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Specific/setup.h"
#include "Game/control/lot.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"

using std::vector;

namespace TEN::Entities::TR4
{
	int CutSeqNum;

	BITE_INFO BigScorpionBite1 = { 0, 0, 0, 8 };
	BITE_INFO BigScorpionBite2 = { 0, 0, 0, 23 };
	const vector<int> BigScorpionAttackJoints = { 8, 20, 21, 23, 24 };

	constexpr auto BIG_SCORPION_ATTACK_DAMAGE = 120;
	constexpr auto BIG_SCORPION_TROOP_ATTACK_DAMAGE = 15;
	constexpr auto BIG_SCORPION_STINGER_POISON_POTENCY = 8;

	constexpr auto BIG_SCORPION_ATTACK_RANGE = SECTOR(1.35);

	enum BigScorpionState
	{
		BSCORPION_STATE_NONE = 0,
		BSCORPION_STATE_IDLE = 1,
		BSCORPION_STATE_WALK_FORWARD = 2,
		BSCORPION_STATE_RUN_FORWARD = 3,
		BSCORPION_STATE_PINCER_ATTACK = 4,
		BSCORPION_STATE_STINGER_ATTACK = 5,
		BSCORPION_STATE_DEATH = 6,
		BSCORPION_STATE_KILL = 7,
		BSCORPION_STATE_KILL_TROOP = 8
	};

	enum BigScorpionAnim
	{
		BSCORPION_ANIM_WALK_FORWARD = 0,
		BSCORPION_ANIM_RUN_FORWARD = 1,
		BSCORPION_ANIM_IDLE = 2,
		BSCORPION_ANIM_PINCER_ATTACK = 3,
		BSCORPION_ANIM_STINGER_ATTACK = 4,
		BSCORPION_ANIM_DEATH = 5,
		BSCORPION_ANIM_KILL = 6,
		BSCORPION_ANIM_KILL_TROOP = 7
	};

	void InitialiseScorpion(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		if (item->TriggerFlags == 1)
		{
			item->Animation.TargetState = BSCORPION_STATE_KILL_TROOP;
			item->Animation.ActiveState = BSCORPION_STATE_KILL_TROOP;
			item->Animation.AnimNumber = Objects[ID_BIG_SCORPION].animIndex + BSCORPION_ANIM_KILL_TROOP;
		}
		else
		{
			item->Animation.TargetState = BSCORPION_STATE_IDLE;
			item->Animation.ActiveState = BSCORPION_STATE_IDLE;
			item->Animation.AnimNumber = Objects[ID_BIG_SCORPION].animIndex + BSCORPION_ANIM_IDLE;
		}

		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	}

	void ScorpionControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short angle = 0;
		short head = 0;
		short neck = 0;
		short tilt = 0;
		short joint0 = 0;
		short joint1 = 0;
		short joint2 = 0;
		short joint3 = 0;

		int x = item->Pose.Position.x + SECTOR(0.68f) * phd_sin(item->Pose.Orientation.y);
		int z = item->Pose.Position.z + SECTOR(0.68f) * phd_cos(item->Pose.Orientation.y);

		auto probe = GetCollision(x, item->Pose.Position.y, z, item->RoomNumber);
		int height1 = probe.Position.Floor;
		if (abs(item->Pose.Position.y - height1) > CLICK(2))
			probe.Position.Floor = item->Pose.Position.y;

		x = item->Pose.Position.x - SECTOR(0.68f) * phd_sin(item->Pose.Orientation.y);
		z = item->Pose.Position.z - SECTOR(0.68f) * phd_cos(item->Pose.Orientation.y);

		probe = GetCollision(x, item->Pose.Position.y, z, probe.RoomNumber);
		int height2 = probe.Position.Floor;
		if (abs(item->Pose.Position.y - height2) > CLICK(2))
			height2 = item->Pose.Position.y;

		short angle1 = phd_atan(SECTOR(1.34), height2 - height1);

		x = item->Pose.Position.x - SECTOR(0.68f) * phd_sin(item->Pose.Orientation.y);
		z = item->Pose.Position.z + SECTOR(0.68f) * phd_cos(item->Pose.Orientation.y);

		probe = GetCollision(x, item->Pose.Position.y, z, probe.RoomNumber);
		int height3 = probe.Position.Floor;
		if (abs(item->Pose.Position.y - height3) > CLICK(2))
			height3 = item->Pose.Position.y;

		x = item->Pose.Position.x + SECTOR(0.68f) * phd_sin(item->Pose.Orientation.y);
		z = item->Pose.Position.z - SECTOR(0.68f) * phd_cos(item->Pose.Orientation.y);

		probe = GetCollision(x, item->Pose.Position.y, z, probe.RoomNumber);
		int height4 = probe.Position.Floor;
		if (abs(item->Pose.Position.y - height4) > CLICK(2))
			height4 = item->Pose.Position.y;

		short angle2 = phd_atan(SECTOR(1.34), height4 - height3);

		if (item->HitPoints <= 0)
		{
			item->HitPoints = 0;
			if (item->Animation.ActiveState != BSCORPION_STATE_DEATH)
			{
				if (item->TriggerFlags > 0 &&
					item->TriggerFlags < 7)
				{
					CutSeqNum = 4;

					item->Animation.AnimNumber = Objects[item->Animation.AnimNumber].animIndex + BSCORPION_ANIM_DEATH;
					item->Animation.ActiveState = BSCORPION_STATE_DEATH;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					item->Status = ITEM_INVISIBLE;
					creature->MaxTurn = 0;

					short linkNumber = g_Level.Rooms[item->RoomNumber].itemNumber;
					if (linkNumber != NO_ITEM)
					{
						for (linkNumber = g_Level.Rooms[item->RoomNumber].itemNumber; linkNumber != NO_ITEM; linkNumber = g_Level.Items[linkNumber].NextItem)
						{
							auto* currentItem = &g_Level.Items[linkNumber];

							if (currentItem->ObjectNumber == ID_TROOPS && currentItem->TriggerFlags == 1)
							{
								DisableEntityAI(linkNumber);
								KillItem(linkNumber);
								currentItem->Flags |= IFLAG_KILLED;
								break;
							}
						}
					}
				}
				else if (item->Animation.ActiveState != BSCORPION_STATE_DEATH && item->Animation.ActiveState != BSCORPION_STATE_KILL)
				{
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + BSCORPION_ANIM_DEATH;
					item->Animation.ActiveState = BSCORPION_STATE_DEATH;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				}
			}
			else if (CutSeqNum == 4)
			{
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameEnd - 1;
				item->Status = ITEM_INVISIBLE;
			}
			else if (item->Animation.ActiveState == BSCORPION_STATE_DEATH)
			{
				if (item->Status == ITEM_INVISIBLE)
					item->Status = ITEM_ACTIVE;
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);
			else
			{
				if (creature->HurtByLara && item->Animation.ActiveState != BSCORPION_STATE_KILL_TROOP)
					creature->Enemy = LaraItem;
				else
				{
					creature->Enemy = nullptr;
					int minDistance = INT_MAX;

					for (int i = 0; i < ActiveCreatures.size(); i++)
					{
						auto* currentCreatureInfo = ActiveCreatures[i];

						if (currentCreatureInfo->ItemNumber != NO_ITEM && currentCreatureInfo->ItemNumber != itemNumber)
						{
							auto* currentItem = &g_Level.Items[currentCreatureInfo->ItemNumber];

							if (currentItem->ObjectNumber != ID_LARA)
							{
								if (currentItem->ObjectNumber != ID_BIG_SCORPION &&
									(currentItem != LaraItem || creature->HurtByLara))
								{
									int dx = currentItem->Pose.Position.x - item->Pose.Position.x;
									int dy = currentItem->Pose.Position.y - item->Pose.Position.y;
									int dz = currentItem->Pose.Position.z - item->Pose.Position.z;

									int distance = pow(dx, 2) + pow(dy, 2) + pow(dz, 2);

									if (distance < minDistance)
									{
										minDistance = distance;
										creature->Enemy = currentItem;
									}
								}
							}
						}
					}
				}
			}

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, VIOLENT);
			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case BSCORPION_STATE_IDLE:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (AI.distance > pow(BIG_SCORPION_ATTACK_RANGE, 2))
				{
					item->Animation.TargetState = BSCORPION_STATE_WALK_FORWARD;
					break;
				}

				if (AI.bite)
				{
					creature->MaxTurn = ANGLE(2.0f);

					if (GetRandomControl() & 1 || //If random conditional, OR, troop is almost dying... choose the pincers attack.
						creature->Enemy->HitPoints <= 15 &&
						creature->Enemy->ObjectNumber == ID_TROOPS)
					{
						item->Animation.TargetState = BSCORPION_STATE_PINCER_ATTACK;
					}
					else
						item->Animation.TargetState = BSCORPION_STATE_STINGER_ATTACK;
				}
				else if (!AI.ahead)
					item->Animation.TargetState = BSCORPION_STATE_WALK_FORWARD;

				break;

			case BSCORPION_STATE_WALK_FORWARD:
				creature->MaxTurn = ANGLE(2.0f);

				if (AI.distance < pow(BIG_SCORPION_ATTACK_RANGE, 2))
					item->Animation.TargetState = BSCORPION_STATE_IDLE;
				else if (AI.distance > pow(853, 2))
					item->Animation.TargetState = BSCORPION_STATE_RUN_FORWARD;

				break;

			case BSCORPION_STATE_RUN_FORWARD:
				creature->MaxTurn = ANGLE(3.0f);

				if (AI.distance < pow(BIG_SCORPION_ATTACK_RANGE, 2))
					item->Animation.TargetState = BSCORPION_STATE_IDLE;

				break;

			case BSCORPION_STATE_PINCER_ATTACK:
			case BSCORPION_STATE_STINGER_ATTACK:
				creature->MaxTurn = 0;

				if (abs(AI.angle) >= ANGLE(2.0f))
				{
					if (AI.angle >= 0)
						item->Pose.Orientation.y += ANGLE(2.0f);
					else
						item->Pose.Orientation.y -= ANGLE(2.0f);
				}
				else
					item->Pose.Orientation.y += AI.angle;

				if (creature->Flags)
					break;

				if (creature->Enemy && !creature->Enemy->IsLara() &&
					AI.distance < pow(1365, 2))
				{
					DoDamage(creature->Enemy, BIG_SCORPION_TROOP_ATTACK_DAMAGE);

					if (creature->Enemy->HitPoints <= 0)
					{
						item->Animation.TargetState = BSCORPION_STATE_KILL;
						creature->MaxTurn = 0;
					}

					creature->Flags = 1;

					CreatureEffect2(
						item,
						&BigScorpionBite1,
						10,
						item->Pose.Orientation.y - ANGLE(180.0f),
						DoBloodSplat);
				}
				else if (item->TestBits(JointBitType::Touch, BigScorpionAttackJoints))
				{
					DoDamage(creature->Enemy, BIG_SCORPION_ATTACK_DAMAGE);

					if (item->Animation.ActiveState == BSCORPION_STATE_STINGER_ATTACK)
					{
						Lara.PoisonPotency += BIG_SCORPION_STINGER_POISON_POTENCY;

						CreatureEffect2(
							item,
							&BigScorpionBite1,
							10,
							item->Pose.Orientation.y - ANGLE(180.0f),
							DoBloodSplat);
					}
					else
					{
						CreatureEffect2(
							item,
							&BigScorpionBite2,
							10,
							item->Pose.Orientation.y - ANGLE(180.0f),
							DoBloodSplat);
					}

					creature->Flags = 1;
					if (LaraItem->HitPoints <= 0)
					{
						CreatureKill(item, 6, 7, 442);
						creature->MaxTurn = 0;
						return;
					}
				}

				break;

			case BSCORPION_STATE_KILL_TROOP:
				creature->MaxTurn = 0;

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
					item->TriggerFlags++;

				if (creature->Enemy &&
					creature->Enemy->HitPoints <= 0 ||
					item->TriggerFlags > 6)
				{
					item->Animation.TargetState = BSCORPION_STATE_KILL;
					creature->Enemy->HitPoints = 0;
				}

				break;

			default:
				break;
			}
		}

		if ((angle1 - item->Pose.Orientation.x) < ANGLE(1.4f))
			item->Pose.Orientation.x = ANGLE(1.4f);
		else
		{
			if (angle1 <= item->Pose.Orientation.x)
				item->Pose.Orientation.x -= ANGLE(1.4f);
			else
				item->Pose.Orientation.x += ANGLE(1.4f);
		}

		if ((angle2 - item->Pose.Orientation.z) < ANGLE(1.4f))
			item->Pose.Orientation.z = ANGLE(1.4f);
		else
		{
			if (angle2 <= item->Pose.Orientation.z)
				item->Pose.Orientation.z -= ANGLE(1.4f);
			else
				item->Pose.Orientation.z += ANGLE(1.4f);
		}

		if (!CutSeqNum)
			CreatureAnimation(itemNumber, angle, 0);
	}
}
