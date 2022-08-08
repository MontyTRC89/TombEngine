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
	int GuideSwapJoint_Left_Fingers = 15;
	int GuideSwapJoint_Right_Hand = 18 ;
	int GuideSwapJoint_Right_Head = 21;

	enum GuideState
	{
		GUIDE_STATE_IDLE = 1,
		GUIDE_STATE_WALK = 2,
		GUIDE_STATE_RUN = 3,
		GUIDE_STATE_CHECKING_GROUND = 7,
		GUIDE_STATE_IGNITE_TORCH = 11,
		GUIDE_STATE_TURNING_LEFT = 22,
		GUIDE_STATE_ATTACK_LOW = 31,
		GUIDE_STATE_ACTION_CANDLES = 32,
		GUIDE_STATE_TURNING_RIGHT = 35,
		GUIDE_STATE_CROUCH = 36,
		GUIDE_STATE_PICKUP_TORCH = 37,
		GUIDE_STATE_LIGHTING_TORCHES = 38,
		GUIDE_STATE_READ_INSCRIPTION = 39,
		GUIDE_STATE_WALK_NO_TORCH = 40,
		GUIDE_STATE_CORRECT_POSITION_FRONT = 41,
		GUIDE_STATE_CORRECT_POSITION_BACK = 42,
		GUIDE_STATE_CROUCH_ACTIVATE_TRAP = 43
	};

	enum GuideAnim
	{
		GUIDE_ANIM_WALK = 0,
		GUIDE_ANIM_RUN = 1,
		//2,
		//3,
		GUIDE_ANIM_IDLE = 4,
		//5,
		//6,
		//7,
		//8,
		//9,
		//10,
		//11,
		GUIDE_ANIM_CHECK_GROUND = 12,
		GUIDE_ANIM_WALK_IDLE_RIGHT = 13,
		GUIDE_ANIM_IDLE_RUN = 14,
		GUIDE_ANIM_RUN_IDLE = 15,
		GUIDE_ANIM_WALK_RUN = 16,
		GUIDE_ANIM_RUN_WALK = 17,
		//18,
		//19,
		//20,
		//21,
		//22,
		//23,
		//24,
		//25,
		GUIDE_ANIM_TURN_LEFT = 26,
		//27,
		//28,
		//29,
		GUIDE_ANIM_USE_LIGHTER  = 30,
		GUIDE_ANIM_COME_SIGNAL = 31,
		//32,
		//33,
		//34,
		//35,
		//36,
		//37,
		//38,
		//39,
		//40,
		//41,
		//42,
		//43,
		GUIDE_ANIM_ATTACK = 44,
		//45,
		//46,
		GUIDE_ANIM_IDLE_LIGHTING_CANDLE = 47,
		GUIDE_ANIM_LIGHTING_CANDLE = 48,
		GUIDE_ANIM_LIGHTING_TORCH_CANDLE = 49,
		//50,
		//51,
		//52,
		//53,
		//54,
		//55,
		GUIDE_ANIM_TURN_RIGHT = 56,
		GUIDE_ANIM_IDLE_CROUCH = 57,
		GUIDE_ANIM_CROUCH = 58,
		GUIDE_ANIM_CROUCH_IDLE = 59,
		GUIDE_ANIM_GRAB_TORCH = 60,
		GUIDE_ANIM_LIGHTING_TORCH = 61,
		GUIDE_ANIM_READ_INSCRIPTION = 62,
		GUIDE_ANIM_WALK_NO_TORCH = 63,
		GUIDE_ANIM_IDLE_WALK_NO_TORCH = 64,
		GUIDE_ANIM_WALK_NO_TORCH_IDLE = 65,
		GUIDE_ANIM_CORRECT_POSITION_FRONT = 66,
		GUIDE_ANIM_CORRECT_POSITION_BACK = 67,
		GUIDE_ANIM_WALK_IDLE_LEFT = 68,
		GUIDE_ANIM_CROUCH_ACTIVATING_TRAP = 69
	};

	void InitialiseGuide(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + GUIDE_ANIM_IDLE;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.TargetState = GUIDE_STATE_IDLE;
		item->Animation.ActiveState = GUIDE_STATE_IDLE;

		item->SetBits(JointBitType::MeshSwap, GuideSwapJoint_Right_Hand);

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

			if (item->Animation.AnimNumber == object->animIndex + GUIDE_ANIM_LIGHTING_TORCH)
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


		if (item->Animation.ActiveState < 4 ||
			item->Animation.ActiveState == GUIDE_STATE_ATTACK_LOW)
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
							currentItem->ObjectNumber == ID_DOG)) // <- Here to add more enemies as his target.
					{
						foundEnemy = currentItem;
						minDistance = distance;
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
				
		bool flag_NewBehaviour			= ((item->ItemFlags[2] & (1 << 0)) != 0) ? true : false;
		bool flag_IgnoreLaraDistance	= ((item->ItemFlags[2] & (1 << 1)) != 0) ? true : false;
		bool flag_RunDefault			= ((item->ItemFlags[2] & (1 << 2)) != 0) ? true : false;
		bool flag_RetryNodeSearch		= ((item->ItemFlags[2] & (1 << 3)) != 0) ? true : false;
		bool flag_ScaryInscription		= ((item->ItemFlags[2] & (1 << 4)) != 0) ? true : false;

		short GoalNode = (flag_NewBehaviour) ? item->ItemFlags[4] : Lara.Location;

		if (flag_RetryNodeSearch)
		{
			item->ItemFlags[2] &= ~(1 << 3);	//turn off bit 3 for flag_RetryNodeSearch
			creature->Enemy = nullptr;
		}

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

			if (item->Animation.RequiredState)
				item->Animation.TargetState = item->Animation.RequiredState;
			else if (GoalNode >= item->ItemFlags[3] ||
				item->ItemFlags[1] != 2)
			{
				if (!creature->ReachedGoal || foundEnemy)
				{
					if (item->MeshSwapBits == 0x40000)
						item->Animation.TargetState = GUIDE_STATE_WALK_NO_TORCH;
					else if (foundEnemy && AI.distance < pow(SECTOR(1), 2))
					{
						if (AI.bite)
							item->Animation.TargetState = GUIDE_STATE_ATTACK_LOW;
					}
					else if (enemy != LaraItem || AI.distance > pow(SECTOR(2), 2))
						if ((flag_RunDefault) && AI.distance > pow(SECTOR(3), 2))
						{
							item->Animation.TargetState = GUIDE_STATE_RUN;
						}
						else
						{
							item->Animation.TargetState = GUIDE_STATE_WALK;
						}
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
						case 0x02: //Action Lit Flames
							item->Animation.TargetState = GUIDE_STATE_LIGHTING_TORCHES;
							item->Animation.RequiredState = GUIDE_STATE_LIGHTING_TORCHES;
							break;

						case 0x20: //Action Pickup Torch
							item->Animation.TargetState = GUIDE_STATE_PICKUP_TORCH;
							item->Animation.RequiredState = GUIDE_STATE_PICKUP_TORCH;
							break;

						case 0x28: //Action Read Inscription
							if (laraAI.distance < pow(SECTOR(2), 2) || flag_IgnoreLaraDistance)
							{
								item->Animation.TargetState = GUIDE_STATE_READ_INSCRIPTION;
								item->Animation.RequiredState = GUIDE_STATE_READ_INSCRIPTION;
							}

							break;

						case 0x10: //Action Ignite Pool
							if (laraAI.distance < pow(SECTOR(2), 2) || flag_IgnoreLaraDistance)
							{
								item->Animation.TargetState = GUIDE_STATE_CROUCH;
								item->Animation.RequiredState = GUIDE_STATE_CROUCH;
							}

							break;

						case 0x04: //Action Trap activation
							if (laraAI.distance < pow(SECTOR(2), 2) || flag_IgnoreLaraDistance)
							{
								item->Animation.TargetState = GUIDE_STATE_CROUCH;
								item->Animation.RequiredState = GUIDE_STATE_CROUCH_ACTIVATE_TRAP;
							}

							break;

						case 0x3E: //Action Dissapear
							item->Status = ITEM_INVISIBLE;
							RemoveActiveItem(itemNumber);
							DisableEntityAI(itemNumber);
							break;
						}
					}
					else
					{
						creature->MaxTurn = 0;
						item->Animation.RequiredState = GUIDE_STATE_CORRECT_POSITION_BACK - (AI.ahead != 0);
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

			if (item->ItemFlags[1] == 1)
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
				if (GoalNode >= item->ItemFlags[3])
				{
					if (!foundEnemy ||
						AI.distance >= pow(SECTOR(1.5f), 2) &&
						(item->TestBits(JointBitType::MeshSwap, GuideSwapJoint_Right_Hand) || AI.distance >= pow(SECTOR(3), 2)))
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
						else if (GoalNode > item->ItemFlags[3] &&
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
				GoalNode < item->ItemFlags[3])
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
					!(item->TestBits(JointBitType::MeshSwap, GuideSwapJoint_Right_Hand)) &&
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
				item->SetBits(JointBitType::MeshSwap, GuideSwapJoint_Left_Fingers);
			else if (frameNumber == 216)
				item->ClearBits(JointBitType::MeshSwap, GuideSwapJoint_Left_Fingers);
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

		case GUIDE_STATE_TURNING_LEFT:
			creature->MaxTurn = 0;

			if (laraAI.angle < -256)
				item->Pose.Orientation.y -= 399;

			break;

		case GUIDE_STATE_ATTACK_LOW:
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

		case GUIDE_STATE_TURNING_RIGHT:
			creature->MaxTurn = 0;

			if (laraAI.angle > 256)
				item->Pose.Orientation.y += 399;

			break;

		case GUIDE_STATE_CROUCH:
		case GUIDE_STATE_CROUCH_ACTIVATE_TRAP:
			if (enemy)
			{
				short deltaAngle = enemy->Pose.Orientation.y - item->Pose.Orientation.y;
				if (deltaAngle < -ANGLE(2.0f))
					item->Pose.Orientation.y -= ANGLE(2.0f);
				else if (deltaAngle > ANGLE(2.0f))
					item->Pose.Orientation.y = ANGLE(2.0f);
			}

			if (item->Animation.RequiredState == GUIDE_STATE_CROUCH_ACTIVATE_TRAP)
				item->Animation.TargetState = GUIDE_STATE_CROUCH_ACTIVATE_TRAP;
			else
			{
				if (item->Animation.AnimNumber != object->animIndex + GUIDE_ANIM_IDLE_CROUCH &&
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
				item->ClearBits(JointBitType::MeshSwap, GuideSwapJoint_Right_Hand);

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

		case GUIDE_STATE_LIGHTING_TORCHES:
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

		case GUIDE_STATE_READ_INSCRIPTION:
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

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 70 && flag_ScaryInscription)
				{
					item->Animation.RequiredState = GUIDE_STATE_RUN;
					item->SetBits(JointBitType::MeshSwap, GuideSwapJoint_Right_Head);
					SoundEffect(SFX_TR4_GUIDE_SCARE, &item->Pose);
				}
				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 185 && flag_ScaryInscription)
				{
					item->ItemFlags[2] &= ~(1 << 4);	//turn off bit 4 for flag_ScaryInscription
					item->ClearBits(JointBitType::MeshSwap, GuideSwapJoint_Right_Head);
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

		case GUIDE_STATE_WALK_NO_TORCH:
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

		case GUIDE_STATE_CORRECT_POSITION_FRONT:
		case GUIDE_STATE_CORRECT_POSITION_BACK:
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
