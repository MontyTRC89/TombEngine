#include "framework.h"
#include "Objects/TR4/Entity/tr4_guide.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using std::vector;

namespace TEN::Entities::TR4
{
	constexpr auto GUIDE_ATTACK_DAMAGE = 20;

	const auto GuideBite1 = BiteInfo(Vector3(0.0f, 20.0f, 180.0f), 18);
	const auto GuideBite2 = BiteInfo(Vector3(30.0f, 80.0f, 50.0f), 15);
	const vector<uint> GuideLeftFingerSwapJoints = { 15 };
	const vector<uint> GuideRightHandSwapJoints	= { 18 };
	const vector<uint> GuideHeadSwapJoints		= { 21 };

	enum GuideState
	{
		GUIDE_STATE_NONE = 0,
		GUIDE_STATE_IDLE = 1,
		GUIDE_STATE_WALK_FORWARD = 2,
		GUIDE_STATE_RUN_FORWARD = 3,
		// No states 4-5.
		GUIDE_STATE_CHECK_GROUND = 7,
		// No states 8-10.
		GUIDE_STATE_IGNITE_TORCH = 11,
		// No states 12-21.
		GUIDE_STATE_TURN_LEFT = 22,
		// No states 23-30.
		GUIDE_STATE_ATTACK_LOW = 31,
		GUIDE_STATE_ACTION_CANDLES = 32,
		// No states 33-34.
		GUIDE_STATE_TURN_RIGHT = 35,
		GUIDE_STATE_CROUCH = 36,
		GUIDE_STATE_PICK_UP_TORCH = 37,
		GUIDE_STATE_LIGHT_TORCHES = 38,
		GUIDE_STATE_READ_INSCRIPTION = 39,
		GUIDE_STATE_WALK_FORWARD_NO_TORCH = 40,
		GUIDE_STATE_ADJUST_POSITION_FRONT = 41,
		GUIDE_STATE_ADJUST_POSITION_BACK = 42,
		GUIDE_STATE_ACTIVATE_TRAP_CROUCHING = 43
	};

	enum GuideAnim
	{
		GUIDE_ANIM_WALK_FORWARD = 0,
		GUIDE_ANIM_RUN = 1,
		// No anims 2-3.
		GUIDE_ANIM_IDLE = 4,
		// No anims 5-11.
		GUIDE_ANIM_CHECK_GROUND = 12,
		GUIDE_ANIM_WALK_FORWARD_TO_IDLE_RIGHT = 13,
		GUIDE_ANIM_IDLE_TO_RUN = 14,
		GUIDE_ANIM_RUN_TO_IDLE = 15,
		GUIDE_ANIM_WALK_FORWARD_TO_RUN_FORWARD = 16,
		GUIDE_ANIM_RUN_FORWARD_TO_WALK_FORWARD = 17,
		// No anims 18-25.
		GUIDE_ANIM_TURN_LEFT = 26,
		// No anims 27-29.
		GUIDE_ANIM_USE_LIGHTER  = 30,
		GUIDE_ANIM_COME_SIGNAL = 31,
		// No anims 32-43.
		GUIDE_ANIM_ATTACK = 44,
		// No anims 45-46.
		GUIDE_ANIM_IDLE_LIGHTING_CANDLE = 47,
		GUIDE_ANIM_LIGHTING_CANDLE = 48,
		GUIDE_ANIM_LIGHTING_TORCH_CANDLE = 49,
		// No anims 55-55.
		GUIDE_ANIM_TURN_RIGHT = 56,
		GUIDE_ANIM_IDLE_CROUCH = 57,
		GUIDE_ANIM_IDLE_TO_CROUCH = 58,
		GUIDE_ANIM_IDLE_TO_CROUCH_IDLE = 59,
		GUIDE_ANIM_GRAB_TORCH = 60,
		GUIDE_ANIM_LIGHTING_TORCH = 61,
		GUIDE_ANIM_READ_INSCRIPTION = 62,
		GUIDE_ANIM_WALK_FORWARD_NO_TORCH = 63,
		GUIDE_ANIM_IDLE_WALK_NO_TORCH = 64,
		GUIDE_ANIM_WALK_FORWARD_NO_TORCH_IDLE = 65,
		GUIDE_ANIM_CORRECT_POSITION_FRONT = 66,
		GUIDE_ANIM_CORRECT_POSITION_BACK = 67,
		GUIDE_ANIM_WALK_FORWARD_IDLE_LEFT = 68,
		GUIDE_ANIM_ACTIVATE_TRAP_CROUCHING = 69
	};

	void InitialiseGuide(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);
		SetAnimation(item, GUIDE_ANIM_IDLE);
		item->MeshSwapBits.Set(GuideRightHandSwapJoints);

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

		// Ignite torch.
		if (item->ItemFlags[1] == 2)
		{
			auto pos = Vector3Int(GuideBite1.Position);
			GetJointAbsPosition(item, &pos, GuideBite1.meshNum);

			SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Pose);
			TriggerFireFlame(pos.x, pos.y - 20, pos.z, -1, 3);

			short random = GetRandomControl();
			TriggerDynamicLight(
				pos.x, pos.y, pos.z,
				15,
				255 - ((random >> 4) & 0x1F),
				192 - ((random >> 6) & 0x1F),
				random & 0x3F);

			if (item->Animation.AnimNumber == (object->animIndex + GUIDE_ANIM_LIGHTING_TORCH))
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
		if (dx > SECTOR(31.25f) || dx < -SECTOR(31.25f) ||
			dz > SECTOR(31.25f) || dz < -SECTOR(31.25f))
		{
			laraAI.distance = INT_MAX;
		}
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
			int minDistance = INT_MAX;

			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				auto* currentCreatureInfo = ActiveCreatures[i];

				if (currentCreatureInfo->ItemNumber == NO_ITEM ||
					currentCreatureInfo->ItemNumber == itemNumber)
				{
					continue;
				}

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
							currentItem->ObjectNumber == ID_DOG)) // Here to add more entities as target.
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

		GetCreatureMood(item, &AI, true);
		CreatureMood(item, &AI, true);

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
				
		bool flagNewBehaviour		= ((item->ItemFlags[2] & (1 << 0)) != 0);
		bool flagIgnoreLaraDistance = ((item->ItemFlags[2] & (1 << 1)) != 0);
		bool flagRunDefault			= ((item->ItemFlags[2] & (1 << 2)) != 0);
		bool flagRetryNodeSearch	= ((item->ItemFlags[2] & (1 << 3)) != 0);
		bool flagScaryInscription	= ((item->ItemFlags[2] & (1 << 4)) != 0);

		short goalNode = (flagNewBehaviour) ? item->ItemFlags[4] : Lara.Location;

		if (flagRetryNodeSearch)
		{
			item->ItemFlags[2] &= ~(1 << 3); // Turn off 3rd for flagRetryNodeSearch.
			creature->Enemy = nullptr;
		}

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
			else if (goalNode >= item->ItemFlags[3] ||
				item->ItemFlags[1] != 2)
			{
				if (!creature->ReachedGoal || foundEnemy)
				{
					if (item->MeshSwapBits == 0x40000)
						item->Animation.TargetState = GUIDE_STATE_WALK_FORWARD_NO_TORCH;
					else if (foundEnemy && AI.distance < pow(SECTOR(1), 2))
					{
						if (AI.bite)
							item->Animation.TargetState = GUIDE_STATE_ATTACK_LOW;
					}
					else if (!enemy->IsLara() || AI.distance > pow(SECTOR(2), 2))
						if (flagRunDefault && AI.distance > pow(SECTOR(3), 2))
							item->Animation.TargetState = GUIDE_STATE_RUN_FORWARD;
						else
							item->Animation.TargetState = GUIDE_STATE_WALK_FORWARD;
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
						// Light flames.
						case 0x02:
							item->Animation.TargetState = GUIDE_STATE_LIGHT_TORCHES;
							item->Animation.RequiredState = GUIDE_STATE_LIGHT_TORCHES;
							break;

						// Pick up torch.
						case 0x20:
							item->Animation.TargetState = GUIDE_STATE_PICK_UP_TORCH;
							item->Animation.RequiredState = GUIDE_STATE_PICK_UP_TORCH;
							break;

						// Read inscription.
						case 0x28:
							if (laraAI.distance < pow(SECTOR(2), 2) || flagIgnoreLaraDistance)
							{
								item->Animation.TargetState = GUIDE_STATE_READ_INSCRIPTION;
								item->Animation.RequiredState = GUIDE_STATE_READ_INSCRIPTION;
							}

							break;

						// Ignite pool.
						case 0x10:
							if (laraAI.distance < pow(SECTOR(2), 2) || flagIgnoreLaraDistance)
							{
								item->Animation.TargetState = GUIDE_STATE_CROUCH;
								item->Animation.RequiredState = GUIDE_STATE_CROUCH;
							}

							break;

						// Activate trap.
						case 0x04:
							if (laraAI.distance < pow(SECTOR(2), 2) || flagIgnoreLaraDistance)
							{
								item->Animation.TargetState = GUIDE_STATE_CROUCH;
								item->Animation.RequiredState = GUIDE_STATE_ACTIVATE_TRAP_CROUCHING;
							}

							break;

						// Disappear.
						case 0x3E:
							item->Status = ITEM_INVISIBLE;
							RemoveActiveItem(itemNumber);
							DisableEntityAI(itemNumber);
							break;
						}
					}
					else
					{
						item->Animation.RequiredState = GUIDE_STATE_ADJUST_POSITION_BACK - (AI.ahead != 0);
						creature->MaxTurn = 0;
					}
				}
			}
			else
				item->Animation.TargetState = GUIDE_STATE_IDLE;

			break;

		case GUIDE_STATE_WALK_FORWARD:
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
				if (goalNode >= item->ItemFlags[3])
				{
					if (!foundEnemy ||
						AI.distance >= pow(SECTOR(1.5f), 2) &&
						(item->MeshSwapBits.Test(GuideRightHandSwapJoints) || AI.distance >= pow(SECTOR(3), 2)))
					{
						if (creature->Enemy->IsLara())
						{
							if (AI.distance >= pow(SECTOR(2), 2))
							{
								if (AI.distance > pow(SECTOR(4), 2))
									item->Animation.TargetState = GUIDE_STATE_RUN_FORWARD;
							}
							else
								item->Animation.TargetState = GUIDE_STATE_IDLE;
						}
						else if (goalNode > item->ItemFlags[3] &&
							laraAI.distance > pow(SECTOR(2), 2))
						{
							item->Animation.TargetState = GUIDE_STATE_RUN_FORWARD;
						}
					}
					else
						item->Animation.TargetState = GUIDE_STATE_IDLE;
				}
				else
					item->Animation.TargetState = GUIDE_STATE_IDLE;
			}

			break;

		case GUIDE_STATE_RUN_FORWARD:
			creature->MaxTurn = ANGLE(11.0f);
			tilt = angle / 2;

			if (AI.ahead)
				joint2 = AI.angle;

			if (AI.distance < pow(SECTOR(2), 2) ||
				goalNode < item->ItemFlags[3])
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
					!(item->MeshSwapBits.Test(GuideRightHandSwapJoints)) &&
					AI.distance < pow(SECTOR(3), 2)))
			{
				item->Animation.TargetState = GUIDE_STATE_IDLE;
				break;
			}

			break;

		case GUIDE_STATE_IGNITE_TORCH:
			pos1 = Vector3Int(GuideBite2.Position);
			GetJointAbsPosition(item, &pos1, GuideBite2.meshNum);

			frameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;
			random = GetRandomControl();

			if (frameNumber == 32)
				item->MeshSwapBits.Set(GuideLeftFingerSwapJoints);
			else if (frameNumber == 216)
				item->MeshSwapBits.Clear(GuideLeftFingerSwapJoints);
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
							pos1.x, pos1.y, pos1.z,
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
					pos1.x, pos1.y, pos1.z,
					10,
					random & 0x1F,
					96 - ((random >> 6) & 0x1F),
					128 - ((random >> 4) & 0x1F));

				TriggerMetalSparks(pos1.x, pos1.y, pos1.z, -1, -1, 0, 1);
			}

			break;

		case GUIDE_STATE_TURN_LEFT:
			creature->MaxTurn = 0;

			if (laraAI.angle < ANGLE(-1.4f))
				item->Pose.Orientation.y -= ANGLE(2.2f);

			break;

		case GUIDE_STATE_ATTACK_LOW:
			creature->MaxTurn = 0;

			if (AI.ahead)
			{
				joint0 = AI.angle / 2;
				joint1 = AI.xAngle / 2;
				joint2 = AI.angle / 2;
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
							DoDamage(enemy, GUIDE_ATTACK_DAMAGE);
							CreatureEffect2(item, GuideBite1, 8, -1, DoBloodSplat);
							creature->Flags = 1;

							if (enemy->HitPoints <= 0)
								item->AIBits = FOLLOW;
						}
					}
				}
			}

			break;

		case GUIDE_STATE_TURN_RIGHT:
			creature->MaxTurn = 0;

			if (laraAI.angle > ANGLE(1.4f))
				item->Pose.Orientation.y += ANGLE(2.2f);

			break;

		case GUIDE_STATE_CROUCH:
		case GUIDE_STATE_ACTIVATE_TRAP_CROUCHING:
			if (enemy != nullptr)
			{
				short deltaAngle = enemy->Pose.Orientation.y - item->Pose.Orientation.y;
				if (deltaAngle < -ANGLE(2.0f))
					item->Pose.Orientation.y -= ANGLE(2.0f);
				else if (deltaAngle > ANGLE(2.0f))
					item->Pose.Orientation.y = ANGLE(2.0f);
			}

			if (item->Animation.RequiredState == GUIDE_STATE_ACTIVATE_TRAP_CROUCHING)
				item->Animation.TargetState = GUIDE_STATE_ACTIVATE_TRAP_CROUCHING;
			else
			{
				if (item->Animation.AnimNumber != (object->animIndex + GUIDE_ANIM_IDLE_CROUCH) &&
					item->Animation.FrameNumber == (g_Level.Anims[item->Animation.AnimNumber].frameEnd - 20))
				{
					TestTriggers(item, true);

					item->Animation.TargetState = GUIDE_STATE_IDLE;
					item->AIBits = FOLLOW;
					item->ItemFlags[3]++;
					creature->ReachedGoal = false;
					creature->Enemy = nullptr;
					break;
				}
			}

			break;

		case GUIDE_STATE_PICK_UP_TORCH:
			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
			{
				someFlag = true;
				item->Pose = enemy->Pose;
			}
			else if (item->Animation.FrameNumber == (g_Level.Anims[item->Animation.AnimNumber].frameBase + 35))
			{
				item->MeshSwapBits.Clear(GuideRightHandSwapJoints);

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

		case GUIDE_STATE_LIGHT_TORCHES:
			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
				item->Pose.Position = enemy->Pose.Position;
			else
			{
				if (item->Animation.FrameNumber == (g_Level.Anims[item->Animation.AnimNumber].frameBase + 42))
				{
					TestTriggers(item, true);

					item->Pose.Orientation.y = enemy->Pose.Orientation.y;
					item->AIBits = FOLLOW;
					item->ItemFlags[3]++;
					creature->ReachedGoal = false;
					creature->Enemy = nullptr;
					break;
				}
				else if (item->Animation.FrameNumber < (g_Level.Anims[item->Animation.AnimNumber].frameBase + 42))
				{
					if ((enemy->Pose.Orientation.y - item->Pose.Orientation.y) <= ANGLE(2.0f))
					{
						if ((enemy->Pose.Orientation.y - item->Pose.Orientation.y) < -ANGLE(2.0f))
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
				if (item->Animation.FrameNumber == (g_Level.Anims[item->Animation.AnimNumber].frameBase + 20))
				{
					item->Animation.TargetState = GUIDE_STATE_IDLE;

					TestTriggers(item, true);

					item->AIBits = FOLLOW;
					item->ItemFlags[3]++;
					creature->ReachedGoal = false;
					creature->Enemy = nullptr;
					break;
				}

				if (item->Animation.FrameNumber == (g_Level.Anims[item->Animation.AnimNumber].frameBase + 70) &&
					flagScaryInscription)
				{
					item->Animation.RequiredState = GUIDE_STATE_RUN_FORWARD;
					item->MeshSwapBits.Set(GuideHeadSwapJoints);
					SoundEffect(SFX_TR4_GUIDE_SCARE, &item->Pose);
				}
				if (item->Animation.FrameNumber == (g_Level.Anims[item->Animation.AnimNumber].frameBase + 185) &&
					flagScaryInscription)
				{
					item->ItemFlags[2] &= ~(1 << 4); // Turn off 4th bit for flagScaryInscription.
					item->MeshSwapBits.Clear(GuideHeadSwapJoints);
				}
			}
			else if ((enemy->Pose.Orientation.y - item->Pose.Orientation.y) <= ANGLE(2.0f))
			{
				if ((enemy->Pose.Orientation.y - item->Pose.Orientation.y) < -ANGLE(2.0f))
					item->Pose.Orientation.y -= ANGLE(2.0f);
			}
			else
				item->Pose.Orientation.y += ANGLE(2.0f);

			break;

		case GUIDE_STATE_WALK_FORWARD_NO_TORCH:
			creature->MaxTurn = ANGLE(7.0f);
			creature->LOT.IsJumping;

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

		case GUIDE_STATE_ADJUST_POSITION_FRONT:
		case GUIDE_STATE_ADJUST_POSITION_BACK:
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
