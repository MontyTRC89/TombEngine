#include "framework.h"
#include "tr4_guide.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

namespace TEN::Entities::TR4
{
	BITE_INFO GuideBite1 = { 0, 20, 180, 18 };
	BITE_INFO GuideBite2 = { 30, 80, 50, 15 };

	enum GuideState
	{
		GUIDE_STATE_IDLE = 1,
		GUIDE_STATE_WALK = 2,
		GUIDE_STATE_RUN = 3,
		GUIDE_STATE_IGNITE_TORCH = 11,
		GUIDE_STATE_LOOK_BACK = 22,
		GUIDE_STATE_TORCH_ATTACK = 31,
		GUIDE_STATE_PICKUP_TORCH = 37
	};

	// TODO
	enum GuideAnim
	{

	};

	void InitialiseGuide(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 4;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.TargetState = GUIDE_STATE_IDLE;
		item->Animation.ActiveState = GUIDE_STATE_IDLE;

		if (Objects[ID_WRAITH1].loaded)
		{
			item->MeshSwapBits = NO_JOINT_BITS;
			item->ItemFlags[1] = 2;
		}
		else
			item->MeshSwapBits = 0x40000;
	}

	void GuideControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);
		auto* object = &Objects[item->ObjectNumber];

		short angle = 0;
		short tilt = 0;
		short joint0 = 0;
		short joint1 = 0;
		short joint2 = 0;

		// Ignite torch
		if (item->ItemFlags[1] == 2)
		{
			auto pos = Vector3Int(GuideBite1.x, GuideBite1.y, GuideBite1.z);
			GetJointAbsPosition(item, &pos, GuideBite1.meshNum);

			SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Pose);
			TriggerFireFlame(pos.x, pos.y - 20, pos.z, -1, 3);

			short random = GetRandomControl();
			TriggerDynamicLight(
				pos.x,
				pos.y,
				pos.z,
				15,
				255 - ((random >> 4) & 0x1F),
				192 - ((random >> 6) & 0x1F),
				random & 0x3F);

			if (item->Animation.AnimNumber == object->animIndex + 61)
			{
				if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 32 &&
					item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 42)
				{
					TriggerFireFlame(
						(random & 0x3F) + pos.x - 32,
						((random / 8) & 0x3F) + pos.y - 128,
						pos.z + ((random / 64) & 0x3F) - 32,
						-1,
						3);
				}
			}
		}

		item->AIBits = FOLLOW;
		item->HitPoints = NOT_TARGETABLE;

		GetAITarget(creature);

		AI_INFO AI;
		AI_INFO laraAI;

		int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
		int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;

		laraAI.angle = phd_atan(dz, dx) - item->Pose.Orientation.y;

		laraAI.ahead = true;
		if (laraAI.angle <= -ANGLE(90.0f) || laraAI.angle >= ANGLE(90.0f))
			laraAI.ahead = false;

		int distance = 0;
		if (dz > 32000 || dz < -32000 || dx > 32000 || dx < -32000)
			laraAI.distance = INT_MAX;
		else
			laraAI.distance = pow(dx, 2) + pow(dz, 2);

		dx = abs(dx);
		dz = abs(dz);

		int dy = item->Pose.Position.y - LaraItem->Pose.Position.y;
		short rot2 = 0;

		if (dx <= dz)
			laraAI.xAngle = phd_atan(dz + (dx / 2), dy);
		else
			laraAI.xAngle = phd_atan(dx + (dz / 2), dy);

		ItemInfo* foundEnemy = nullptr;

		if (!Objects[ID_WRAITH1].loaded)
		{
			if (item->Animation.ActiveState < 4 ||
				item->Animation.ActiveState == GUIDE_STATE_TORCH_ATTACK)
			{
				int minDistance = 0x7FFFFFFF;

				for (int i = 0; i < ActiveCreatures.size(); i++)
				{
					auto* currentCreatureInfo = ActiveCreatures[i];

					if (currentCreatureInfo->ItemNumber == NO_ITEM || currentCreatureInfo->ItemNumber == itemNumber)
						continue;

					auto* currentItem = &g_Level.Items[currentCreatureInfo->ItemNumber];

					if (currentItem->ObjectNumber != ID_GUIDE &&
						abs(currentItem->Pose.Position.y - item->Pose.Position.y) <= 512)
					{
						dx = currentItem->Pose.Position.x - item->Pose.Position.x;
						dy = currentItem->Pose.Position.y - item->Pose.Position.y;
						dz = currentItem->Pose.Position.z - item->Pose.Position.z;

						if (dx > 32000 || dx < -32000 || dz > 32000 || dz < -32000)
							distance = 0x7FFFFFFF;
						else
							distance = pow(dx, 2) + pow(dz, 2);

						if (distance < minDistance &&
							distance < pow(SECTOR(2), 2) &&
							(abs(dy) < CLICK(1) ||
								laraAI.distance < pow(SECTOR(2), 2) ||
								currentItem->ObjectNumber == ID_DOG))
						{
							foundEnemy = currentItem;
							minDistance = distance;
						}
					}
				}
			}
		}

		auto* enemy = creature->Enemy;
		if (foundEnemy)
			creature->Enemy = foundEnemy;

		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);

		if (foundEnemy)
		{
			creature->Enemy = enemy;
			enemy = foundEnemy;
		}

		bool someFlag = false;
		Vector3Int pos1;
		int frameNumber;
		short random;

		TENLog("Guide state:" + std::to_string(item->Animation.ActiveState), LogLevel::Info);

		switch (item->Animation.ActiveState)
		{
		case GUIDE_STATE_IDLE:
			creature->MaxTurn = 0;
			creature->Flags = 0;
			creature->LOT.IsJumping = false;
			joint2 = AI.angle / 2;

			if (laraAI.ahead)
			{
				joint0 = laraAI.angle / 2;
				joint1 = laraAI.xAngle / 2;
				joint2 = laraAI.angle / 2;
			}
			else if (AI.ahead)
			{
				joint0 = AI.angle / 2;
				joint1 = AI.xAngle / 2;
				joint2 = AI.angle / 2;
			}

			if (Objects[ID_WRAITH1].loaded)
			{
				if (item->ItemFlags[3] == 5)
					item->Animation.TargetState = GUIDE_STATE_WALK;

				if (item->ItemFlags[3] == 5 || item->ItemFlags[3] == 6)
					break;
			}

			if (item->Animation.RequiredState)
				item->Animation.TargetState = item->Animation.RequiredState;
			else if (Lara.Location >= item->ItemFlags[3] ||
				item->ItemFlags[1] != 2)
			{
				if (!creature->ReachedGoal || foundEnemy)
				{
					if (item->MeshSwapBits == 0x40000)
						item->Animation.TargetState = 40;
					else if (foundEnemy && AI.distance < pow(SECTOR(1), 2))
					{
						if (AI.bite)
							item->Animation.TargetState = GUIDE_STATE_TORCH_ATTACK;
					}
					else if (enemy != LaraItem || AI.distance > pow(SECTOR(2), 2))
						item->Animation.TargetState = GUIDE_STATE_WALK;
				}
				else
				{
					if (!enemy->Flags)
					{
						creature->ReachedGoal = false;
						creature->Enemy = nullptr;
						item->AIBits = FOLLOW;
						item->ItemFlags[3]++;
						break;
					}

					if (AI.distance <= pow(CLICK(0.5f), 2))
					{
						switch (enemy->Flags)
						{
						case 0x02:
							item->Animation.TargetState = 38;
							item->Animation.RequiredState = 38;
							break;

						case 0x20:
							item->Animation.TargetState = GUIDE_STATE_PICKUP_TORCH;
							item->Animation.RequiredState = GUIDE_STATE_PICKUP_TORCH;
							break;

						case 0x28:
							if (laraAI.distance < pow(SECTOR(2), 2))
							{
								item->Animation.TargetState = 39;
								item->Animation.RequiredState = 39;
							}

							break;

						case 0x10:
							if (laraAI.distance < pow(SECTOR(2), 2))
							{
								// Ignite torch
								item->Animation.TargetState = 36;
								item->Animation.RequiredState = 36;
							}

							break;

						case 0x04:
							if (laraAI.distance < pow(SECTOR(2), 2))
							{
								item->Animation.TargetState = 36;
								item->Animation.RequiredState = 43;
							}

							break;

						case 0x3E:
							item->Status = ITEM_INVISIBLE;
							RemoveActiveItem(itemNumber);
							DisableEntityAI(itemNumber);
							break;
						}
					}
					else
					{
						creature->MaxTurn = 0;
						item->Animation.RequiredState = 42 - (AI.ahead != 0);
					}
				}
			}
			else
				item->Animation.TargetState = GUIDE_STATE_IDLE;

			break;

		case GUIDE_STATE_WALK:
			creature->MaxTurn = ANGLE(7.0f);
			creature->LOT.IsJumping = false;

			if (laraAI.ahead)
			{
				if (AI.ahead)
					joint2 = AI.angle;
			}
			else
				joint2 = laraAI.angle;

			if (Objects[ID_WRAITH1].loaded && item->ItemFlags[3] == 5)
			{
				item->ItemFlags[3] = 6;
				item->Animation.TargetState = GUIDE_STATE_IDLE;
			}
			else if (item->ItemFlags[1] == 1)
			{
				item->Animation.TargetState = GUIDE_STATE_IDLE;
				item->Animation.RequiredState = GUIDE_STATE_IGNITE_TORCH;
			}
			else if (creature->ReachedGoal)
			{
				if (!enemy->Flags)
				{
					creature->ReachedGoal = false;
					creature->Enemy = nullptr;
					item->AIBits = FOLLOW;
					item->ItemFlags[3]++;
					break;
				}

				item->Animation.TargetState = GUIDE_STATE_IDLE;
			}
			else
			{
				if (Lara.Location >= item->ItemFlags[3])
				{
					if (!foundEnemy ||
						AI.distance >= pow(SECTOR(1.5f), 2) &&
						(item->MeshSwapBits & 0x40000 || AI.distance >= pow(SECTOR(3), 2)))
					{
						if (creature->Enemy == LaraItem)
						{
							if (AI.distance >= pow(SECTOR(2), 2))
							{
								if (AI.distance > pow(SECTOR(4), 2))
									item->Animation.TargetState = GUIDE_STATE_RUN;
							}
							else
								item->Animation.TargetState = GUIDE_STATE_IDLE;
						}
						else if (Lara.Location > item->ItemFlags[3] &&
							laraAI.distance > pow(SECTOR(2), 2))
						{
							item->Animation.TargetState = GUIDE_STATE_RUN;
						}
					}
					else
						item->Animation.TargetState = GUIDE_STATE_IDLE;
				}
				else
					item->Animation.TargetState = GUIDE_STATE_IDLE;
			}

			break;

		case GUIDE_STATE_RUN:
			creature->MaxTurn = ANGLE(11.0f);
			tilt = angle / 2;

			if (AI.ahead)
				joint2 = AI.angle;

			if (AI.distance < pow(SECTOR(2), 2) ||
				Lara.Location < item->ItemFlags[3])
			{
				item->Animation.TargetState = GUIDE_STATE_IDLE;
				break;
			}

			if (creature->ReachedGoal)
			{
				if (!enemy->Flags)
				{
					creature->ReachedGoal = false;
					creature->Enemy = nullptr;
					item->AIBits = FOLLOW;
					item->ItemFlags[3]++;
					break;
				}

				item->Animation.TargetState = GUIDE_STATE_IDLE;
			}
			else if (foundEnemy &&
				(AI.distance < pow(SECTOR(1.5f), 2) ||
					!(item->MeshSwapBits & 0x40000) &&
					AI.distance < pow(SECTOR(3), 2)))
			{
				item->Animation.TargetState = GUIDE_STATE_IDLE;
				break;
			}

			break;

		case GUIDE_STATE_IGNITE_TORCH:
			// Ignite torch
			pos1.x = GuideBite2.x;
			pos1.y = GuideBite2.y;
			pos1.z = GuideBite2.z;

			GetJointAbsPosition(item, &pos1, GuideBite2.meshNum);

			frameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;
			random = GetRandomControl();

			if (frameNumber == 32)
				item->MeshSwapBits |= 0x8000;
			else if (frameNumber == 216)
				item->MeshSwapBits &= 0x7FFF;
			else if (frameNumber <= 79 || frameNumber >= 84)
			{
				if (frameNumber <= 83 || frameNumber >= 94)
				{
					if (frameNumber <= 159 || frameNumber >= 164)
					{
						if (frameNumber > 163 && frameNumber < 181)
						{
							TriggerFireFlame(
								(random & 0x3F) + pos1.x - 64,
								((random >> 5) & 0x3F) + pos1.y - 96,
								((random >> 10) & 0x3F) + pos1.z - 64,
								-1,
								3);

							TriggerDynamicLight(
								pos1.x - 32,
								pos1.y - 64,
								pos1.z - 32,
								10,
								-64 - ((random >> 4) & 0x1F),
								-128 - ((random >> 6) & 0x1F),
								random & 0x1F);

							item->ItemFlags[1] = 2;
						}
					}
					else
					{
						TriggerMetalSparks(pos1.x, pos1.y, pos1.z, -1, -1, 0, 1);
						TriggerDynamicLight(
							pos1.x,
							pos1.y,
							pos1.z,
							10,
							random & 0x1F,
							96 - ((random >> 6) & 0x1F),
							128 - ((random >> 4) & 0x1F));
					}
				}
				else
				{
					TriggerDynamicLight(
						pos1.x - 32,
						pos1.y - 64,
						pos1.z - 32,
						10,
						-64 - ((random >> 4) & 0x1F),
						-128 - ((random >> 6) & 0x1F),
						random & 0x1F);

					TriggerFireFlame(
						(random & 0x3F) + pos1.x - 64,
						((random >> 5) & 0x3F) + pos1.y - 96,
						((random >> 10) & 0x3F) + pos1.z - 64,
						-1,
						3);
				}
			}
			else
			{
				TriggerDynamicLight(
					pos1.x,
					pos1.y,
					pos1.z,
					10,
					random & 0x1F,
					96 - ((random >> 6) & 0x1F),
					128 - ((random >> 4) & 0x1F));

				TriggerMetalSparks(pos1.x, pos1.y, pos1.z, -1, -1, 0, 1);
			}

			break;

		case GUIDE_STATE_LOOK_BACK:
			creature->MaxTurn = 0;

			if (laraAI.angle < -256)
				item->Pose.Orientation.y -= 399;

			break;

		case GUIDE_STATE_TORCH_ATTACK:
			creature->MaxTurn = 0;

			if (AI.ahead)
			{
				joint0 = AI.angle / 2;
				joint2 = AI.angle / 2;
				joint1 = AI.xAngle / 2;
			}

			if (abs(AI.angle) >= ANGLE(7.0f))
			{
				if (AI.angle < 0)
					item->Pose.Orientation.y += ANGLE(7.0f);
				else
					item->Pose.Orientation.y -= ANGLE(7.0f);
			}
			else
				item->Pose.Orientation.y += AI.angle;

			if (!creature->Flags)
			{
				if (enemy)
				{
					if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 15 &&
						item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 26)
					{
						dx = abs(enemy->Pose.Position.x - item->Pose.Position.x);
						dy = abs(enemy->Pose.Position.y - item->Pose.Position.y);
						dz = abs(enemy->Pose.Position.z - item->Pose.Position.z);

						if (dx < CLICK(2) &&
							dy < CLICK(2) &&
							dz < CLICK(2))
						{
							DoDamage(enemy, 20);

							if (enemy->HitPoints <= 0)
								item->AIBits = FOLLOW;

							creature->Flags = 1;

							CreatureEffect2(
								item,
								&GuideBite1,
								8,
								-1,
								DoBloodSplat);
						}
					}
				}
			}

			break;

		case 35:
			creature->MaxTurn = 0;

			if (laraAI.angle > 256)
				item->Pose.Orientation.y += 399;

			break;

		case 36:
		case 43:
			if (enemy)
			{
				short deltaAngle = enemy->Pose.Orientation.y - item->Pose.Orientation.y;
				if (deltaAngle < -ANGLE(2.0f))
					item->Pose.Orientation.y -= ANGLE(2.0f);
				else if (deltaAngle > ANGLE(2.0f))
					item->Pose.Orientation.y = ANGLE(2.0f);
			}

			if (item->Animation.RequiredState == 43)
				item->Animation.TargetState = 43;
			else
			{
				if (item->Animation.AnimNumber != object->animIndex + 57 &&
					item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd - 20)
				{
					TestTriggers(item, true);

					creature->ReachedGoal = false;
					creature->Enemy = nullptr;
					item->AIBits = FOLLOW;
					item->ItemFlags[3]++;
					item->Animation.TargetState = GUIDE_STATE_IDLE;
					break;
				}
			}

			break;

		case GUIDE_STATE_PICKUP_TORCH:
			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
			{
				someFlag = true;

				item->Pose = enemy->Pose;
			}
			else if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 35)
			{
				item->MeshSwapBits &= 0xFFFBFFFF;

				auto* room = &g_Level.Rooms[item->RoomNumber];
				ItemInfo* currentItem = nullptr;

				short currentitemNumber = room->itemNumber;
				while (currentitemNumber != NO_ITEM)
				{
					currentItem = &g_Level.Items[currentitemNumber];

					if (currentItem->ObjectNumber >= ID_ANIMATING1 &&
						currentItem->ObjectNumber <= ID_ANIMATING15 &&
						trunc(item->Pose.Position.x) == trunc(currentItem->Pose.Position.x) &&
						trunc(item->Pose.Position.z) == trunc(currentItem->Pose.Position.z))
					{
						break;
					}

					currentitemNumber = currentItem->NextItem;
				}

				if (currentItem != nullptr)
					currentItem->MeshBits = 0xFFFFFFFD;
			}

			item->ItemFlags[1] = 1;

			if (someFlag)
			{
				item->AIBits = FOLLOW;
				item->ItemFlags[3]++;
				creature->ReachedGoal = false;
				creature->Enemy = nullptr;
			}

			break;

		case 38:
			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
				item->Pose.Position = enemy->Pose.Position;
			else
			{
				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 42)
				{
					TestTriggers(item, true);

					item->Pose.Orientation.y = enemy->Pose.Orientation.y;
					item->AIBits = FOLLOW;
					item->ItemFlags[3]++;
					creature->ReachedGoal = false;
					creature->Enemy = nullptr;
					break;
				}
				else if (item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 42)
				{
					if (enemy->Pose.Orientation.y - item->Pose.Orientation.y <= ANGLE(2.0f))
					{
						if (enemy->Pose.Orientation.y - item->Pose.Orientation.y < -ANGLE(2.0f))
							item->Pose.Orientation.y -= ANGLE(2.0f);
					}
					else
						item->Pose.Orientation.y += ANGLE(2.0f);
				}
			}

			break;

		case 39:
			if (item->Animation.FrameNumber >= g_Level.Anims[item->Animation.AnimNumber].frameBase + 20)
			{
				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 20)
				{
					item->Animation.TargetState = GUIDE_STATE_IDLE;

					TestTriggers(item, true);

					item->AIBits = FOLLOW;
					item->ItemFlags[3]++;
					creature->ReachedGoal = false;
					creature->Enemy = nullptr;
					break;
				}

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 70 && item->RoomNumber == 70)
				{
					item->Animation.RequiredState = GUIDE_STATE_RUN;
					item->MeshSwapBits |= 0x200000;
					SoundEffect(SFX_TR4_GUIDE_SCARE, &item->Pose);
				}
			}
			else if (enemy->Pose.Orientation.y - item->Pose.Orientation.y <= ANGLE(2.0f))
			{
				if (enemy->Pose.Orientation.y - item->Pose.Orientation.y < -ANGLE(2.0f))
					item->Pose.Orientation.y -= ANGLE(2.0f);
			}
			else
				item->Pose.Orientation.y += ANGLE(2.0f);

			break;

		case 40:
			creature->LOT.IsJumping;
			creature->MaxTurn = ANGLE(7.0f);

			if (laraAI.ahead)
			{
				if (AI.ahead)
					joint2 = AI.angle;
			}
			else
				joint2 = laraAI.angle;

			if (!(creature->ReachedGoal))
				break;

			if (!enemy->Flags)
			{
				item->AIBits = FOLLOW;
				item->ItemFlags[3]++;
				creature->ReachedGoal = false;
				creature->Enemy = nullptr;
				break;
			}
			if (enemy->Flags == 42)
			{
				TestTriggers(item, true);

				item->AIBits = FOLLOW;
				item->ItemFlags[3]++;
				creature->ReachedGoal = false;
				creature->Enemy = nullptr;
			}
			else if (item->TriggerFlags <= 999)
				item->Animation.TargetState = GUIDE_STATE_IDLE;
			else
			{
				KillItem(itemNumber);
				DisableEntityAI(itemNumber);
				item->Flags |= 1;
			}

			break;

		case 41:
		case 42:
			creature->MaxTurn = 0;
			MoveCreature3DPos(&item->Pose, &enemy->Pose, 15, enemy->Pose.Orientation.y - item->Pose.Orientation.y, ANGLE(10.0f));

		default:
			break;
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);
		CreatureJoint(item, 3, joint1);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
