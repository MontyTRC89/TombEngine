#include "framework.h"
#include "tr4_goon.h"
#include "Game/items.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/control/lot.h"
#include "Game/itemdata/creature_info.h"
#include "Game/misc.h"

/*
ID_GOON1
1 - Rolls to the right 1 pow
2 - Jumps to the left 1 pow
3 - ducks when triggered
4 - Climbs up 4 clicks when triggered
101-104 – Slides to the left while crouching when triggered (eg. train level – just doesn’t work in trainmode)
1004 - Climbs up 6 clicks when triggered
1000 – N x 1000 – Is activated once the goon with the previous thousand is dead and needs no trigger (have tested up to 20.000). Must be placed in room 2 of a level.
This means that:
2000 - Attacks Lara after she kills 1st goon triggered
3000 - Same as above but after she kills 2nd goon triggered
4000 - Same as above but after she kills 3rd goon triggered
6000 - Same as above but after she kills 5th goon triggered
etc.

ID_GOON2
1 - Jumps to right when triggered
2 - Rolls to left when triggered
3 - Crouches when triggered
4 - Climbs up 4 clicks when triggered
10 - Draws uzi when triggered
11 - Jumps to the right when triggered and draws uzi
12 - Rolls to the left when triggered and draws uzi
13 - Crouches when triggered and draws uzi
14 - Climbs up 4 clicks when triggered and draws uzi
101 - Slides to the left while crouching when triggered (eg. Train level)
101-104 - Slides to the left while crouching when triggered. The setup requires an enemy jeep and an AI_X1 nullmesh with the same OCB as the jeep and the goon. It works only in trainmode. When triggered, the goon will ride the roof of the enemy jeep parallel to the railtracks, until they reach the AI_X1 nullmesh. The goon will jump off in the direction he’s placed in the map, while the jeep will fall back.
*/

namespace TEN::Entities::TR4
{
	enum GoonState
	{
		GOON_STATE_IDLE = 0,
		GOON_STATE_WALK = 1,
		GOON_STATE_RUN = 2,
		// 3
		GOON_STATE_DODGE_START = 4,
		// 5
		// 6
		// 7
		GOON_STATE_UNKNOWN_8 = 8,
		GOON_STATE_UNKNOWN_9 = 9,
		GOON_STATE_DRAW_GUN = 10,
		GOON_STATE_HOLSTER_GUN = 11,
		GOON_STATE_DRAW_SWORD = 12,
		GOON_STATE_HOLSTER_SWORD = 13,
		GOON_STATE_FIRE = 14,
		GOON_STATE_SWORD_HIT_FRONT = 15,
		GOON_STATE_SWORD_HIT_RIGHT = 16,
		GOON_STATE_SWORD_HIT_LEFT = 17,
		GOON_STATE_MONKEY_GRAB = 18,
		GOON_STATE_MONKEY_IDLE = 19,
		GOON_STATE_MONKEY_FORWARD = 20,
		GOON_STATE_MONKEY_PUSH_OFF = 21,
		GOON_STATE_MONKEY_FALL_LAND = 22,
		GOON_STATE_ROLL_LEFT = 23,
		GOON_STATE_JUMP_RIGHT = 24,
		GOON_STATE_STAND_TO_CROUCH = 25,
		GOON_STATE_CROUCH = 26,
		GOON_STATE_CROUCH_PICKUP = 27,
		GOON_STATE_CROUCH_TO_STAND = 28,
		GOON_STATE_WALK_SWORD_HIT_RIGHT = 29,
		GOON_STATE_SOMERSAULT = 30,
		GOON_STATE_AIM = 31,
		GOON_STATE_DEATH = 32,
		GOON_STATE_JUMP_FORWARD_1_BLOCK = 33,
		GOON_STATE_JUMP_FORWARD_FALL = 34,
		GOON_STATE_MONKEY_TO_FREEFALL = 35,
		GOON_STATE_FREEFALL = 36,
		GOON_STATE_FREEFALL_LAND_DEATH = 37,
		GOON_STATE_JUMP_FORWARD_2_BLOCKS = 38,
		GOON_STATE_CLIMB_4_STEPS = 39,
		GOON_STATE_CLIMB_3_STEPS = 40,
		GOON_STATE_CLIMB_2_STEPS = 41,
		GOON_STATE_JUMP_OFF_4_STEPS = 42,
		GOON_STATE_JUMP_OFF_3_STEPS = 43,
		GOON_STATE_BLIND = 44
	};

	enum GoonAnim
	{
		GOON_ANIM_RUN = 0,
		GOON_ANIM_RUN_STOP_START = 1,
		GOON_ANIM_RUN_STOP_END = 2,
		GOON_ANIM_SOMERSAULT_START = 3,
		GOON_ANIM_SOMERSAULT_END = 4,
		GOON_ANIM_DODGE_START = 5,
		// 6
		// 7
		// 8
		GOON_ANIM_MONKEY_GRAB = 9,
		GOON_ANIM_MONKEY_IDLE = 10,
		GOON_ANIM_MONKEY_FORWARD = 11,
		GOON_ANIM_MONKEY_IDLE_TO_FORWARD = 12,
		GOON_ANIM_MONKEY_STOP_LEFT = 13,
		GOON_ANIM_MONKEY_STOP_RIGHT = 14,
		GOON_ANIM_MONKEY_FALL_LAND = 15,
		GOON_ANIM_MONKEY_PUSH_OFF = 16,
		GOON_ANIM_DODGE_END = 17,
		GOON_ANIM_STAND_IDLE = 18,
		GOON_ANIM_DODGE_END_TO_STAND = 19,
		GOON_ANIM_DRAW_GUN = 20,
		GOON_ANIM_HOLSTER_GUN = 21,
		GOON_ANIM_DRAW_SWORD = 22,
		GOON_ANIM_HOLSTER_SWORD = 23,
		GOON_ANIM_STAND_TO_ROLL_LEFT = 24,
		GOON_ANIM_ROLL_LEFT_START = 25,
		GOON_ANIM_ROLL_LEFT_CONTINUE = 26,
		GOON_ANIM_ROLL_LEFT_END = 27,
		GOON_ANIM_ROLL_LEFT_TO_CROUCH = 28,
		GOON_ANIM_CROUCH = 29,
		GOON_ANIM_CROUCH_TO_STAND = 30,
		GOON_ANIM_STAND_TO_WALK = 31,
		GOON_ANIM_WALK = 32,
		GOON_ANIM_WALK_TO_RUN = 33,
		GOON_ANIM_STAND_TO_AIM = 34,
		GOON_ANIM_AIM = 35,
		GOON_ANIM_FIRE = 36,
		GOON_ANIM_AIM_TO_STAND = 37,
		GOON_ANIM_SWORD_HIT_FRONT = 38,
		GOON_ANIM_CROUCH_PICKUP = 39,
		GOON_ANIM_STAND_TO_CROUCH = 40,
		GOON_ANIM_SWORD_HIT_RIGHT = 41,
		GOON_ANIM_SWORD_HIT_RIGHT_TO_LEFT = 42,
		GOON_ANIM_SWORD_HIT_RIGHT_TO_STAND = 43,
		GOON_ANIM_SWORD_HIT_LEFT = 44,
		GOON_ANIM_STAND_DEATH = 45,
		GOON_ANIM_WALK_SWORD_HIT_RIGHT = 46,
		GOON_ANIM_STAND_TO_JUMP_RIGHT = 47,
		GOON_ANIM_JUMP_RIGHT_START = 48,
		GOON_ANIM_JUMP_RIGHT_CONTINUE = 49,
		GOON_ANIM_JUMP_RIGHT_END = 50,
		GOON_ANIM_RUN_TO_WALK = 51,
		// 52
		// 53
		GOON_ANIM_WALK_STOP_RIGHT = 54,
		GOON_ANIM_STAND_TO_JUMP_FORWARD = 55,
		GOON_ANIM_JUMP_FORWARD_1_BLOCK = 56,
		GOON_ANIM_JUMP_FORWARD_FALL = 57,
		GOON_ANIM_JUMP_FORWARD_LAND = 58,
		GOON_ANIM_MONKEY_TO_FREEFALL = 59,
		GOON_ANIM_FREEFALL = 60,
		GOON_ANIM_FREEFALL_LAND_DEATH = 61,
		GOON_ANIM_CLIMB_4_STEPS = 62,
		GOON_ANIM_CLIMB_3_STEPS = 63,
		GOON_ANIM_CLIMB_2_STEPS = 64,
		GOON_ANIM_JUMP_OFF_4_STEPS = 65,
		GOON_ANIM_JUMP_OFF_3_STEPS = 66,
		GOON_ANIM_JUMP_FORWARD_2_BLOCKS = 67,
		GOON_ANIM_BLIND = 68,
		GOON_ANIM_BLIND_TO_STAND = 69,
		GOON_ANIM_DEAD = 70,
	};

	enum GoonFrame
	{
		FRAME_GOON_HOLSTER_GUN = 20,
		FRAME_GOON_DRAW_GUN = 21,
		FRAME_GOON_HOLSTER_SWORD = 22,
		FRAME_GOON_DRAW_SWORD = 12,
		FRAME_GOON_RUN_TO_SOMERSAULT = 11,
		FRAME_GOON_SWORD_HIT_NO_DAMAGE_MAX = 12,
		FRAME_GOON_SWORD_HIT_DAMAGE_MIN = 13,
		FRAME_GOON_SWORD_HIT_DAMAGE_MAX = 21,
		FRAME_GOON_CROUCH_PICKUP = 9,
		FRAME_GOON_FIRE_MIN = 1,
		FRAME_GOON_FIRE_MAX = 13,
		FRAME_GOON_SOMERSAULT_START_TAKE_OFF = 18,
	};

	enum GoonMeshSwapFlags
	{
		MESHSWAPFLAGS_GOON_EMPTY = 0x7FC800,
		MESHSWAPFLAGS_GOON_SWORD_SIMPLE = 0x7E0880,
		MESHSWAPFLAGS_GOON_SWORD_NINJA = 0x000880,
		MESHSWAPFLAGS_GOON_GUN = 0x7FC010,
	};

	#define GOON_USE_UZI	24

	BITE_INFO GoonGunBite = { 0, -16, 200, 11 };
	BITE_INFO GoonSwordBite = { 0, 0, 0, 15 };

	void InitialiseGoon(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
	
		ClearItem(itemNumber);

		short objectNumber = (Objects[ID_GOON2].loaded ? ID_GOON2 : ID_GOON1);

		if (item->ObjectNumber == ID_GOON1)
		{
			item->SwapMeshFlags = MESHSWAPFLAGS_GOON_GUN;
			item->MeshBits = 0xFF81FFFF;
			item->ItemFlags[2] = GOON_USE_UZI;
		}
		else
		{
			item->SwapMeshFlags = MESHSWAPFLAGS_GOON_SWORD_NINJA;
			item->MeshBits = -1;
			item->ItemFlags[2] = 0;
		}
	
		item->ItemFlags[1] = -1;

		short ocb = item->TriggerFlags % 1000;

		// To the same things of OCB 1, 2, 3, 4 but also drawing uzis
		if (ocb > 9 && ocb < 20)
		{
			item->ItemFlags[2] += GOON_USE_UZI;
			item->TriggerFlags -= 10;
			ocb -= 10;
		}
	
		if (!ocb || ocb > 4 && ocb < 7)
		{
			item->Animation.AnimNumber = Objects[objectNumber].animIndex + GOON_ANIM_STAND_IDLE;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.TargetState = GOON_STATE_IDLE;
			item->Animation.ActiveState = GOON_STATE_IDLE;
			return;
		}

		// OCB: jump right
		if (ocb == 1)
		{
			item->Animation.AnimNumber = Objects[objectNumber].animIndex + GOON_ANIM_STAND_TO_JUMP_RIGHT;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.TargetState = GOON_STATE_JUMP_RIGHT;
			item->Animation.ActiveState = GOON_STATE_JUMP_RIGHT;
			return;
		}

		// OCB: jump left
		if (ocb == 2)
		{
			item->Animation.AnimNumber = Objects[objectNumber].animIndex + GOON_ANIM_STAND_TO_ROLL_LEFT;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.TargetState = GOON_STATE_ROLL_LEFT;
			item->Animation.ActiveState = GOON_STATE_ROLL_LEFT;
			return;
		}
	
		// OCB: crouch
		if (ocb == 3)
		{
			item->Animation.AnimNumber = Objects[objectNumber].animIndex + GOON_ANIM_CROUCH;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.TargetState = GOON_STATE_CROUCH;
			item->Animation.ActiveState = GOON_STATE_CROUCH;
			return;
		}

		// OCB: climb up 4 or 6 clicks 
		if (ocb == 4)
		{
			item->Animation.AnimNumber = Objects[objectNumber].animIndex + GOON_ANIM_CLIMB_4_STEPS;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.TargetState = GOON_STATE_CLIMB_4_STEPS;
			item->Animation.ActiveState = GOON_STATE_CLIMB_4_STEPS;
			item->Pose.Position.x += phd_sin(item->Pose.Orientation.y) * CLICK(4);
			item->Pose.Position.z += phd_cos(item->Pose.Orientation.y) * CLICK(4);
			return;
		}

		// OCB: crouch and jump in train levels?
		if (ocb > 100)
		{
			item->Animation.AnimNumber = Objects[objectNumber].animIndex + GOON_ANIM_CROUCH;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.TargetState = GOON_STATE_CROUCH;
			item->Animation.ActiveState = GOON_STATE_CROUCH;
			item->Pose.Position.x += phd_sin(item->Pose.Orientation.y) * CLICK(4);
			item->Pose.Position.z += phd_cos(item->Pose.Orientation.y) * CLICK(4);
			item->ItemFlags[3] = ocb;
			return;
		}
	
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	}

	void GoonControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		CreatureInfo* creature = GetCreatureInfo(item);
		auto* enemyItem = creature->Enemy;
		auto* object = &Objects[ID_GOON1];

		short tilt = 0;
		short angle = 0;
		short joint1 = 0;
		short joint2 = 0;
		short joint3 = 0;

		// TODO: better add a second control routine for goon 2 instead of mixing them?
		short objectNumber = (Objects[ID_GOON2].loaded ? ID_GOON2 : ID_GOON1);

		bool roll = false;
		bool jump = false;
		bool someFlag3 = false;

		if (item->TriggerFlags % 1000)
		{
			creature->MaxTurn = 0;
			creature->LOT.IsJumping = true;

			if (item->TriggerFlags % 1000 > 100)
			{
				item->ItemFlags[0] = -80;
				FindAITargetObject(creature, ID_AI_X1);
			}

			item->TriggerFlags = 1000 * (item->TriggerFlags / 1000);
		}

		// Can goon jump? Check for a distance of 1 and 2 sectors
		int x = item->Pose.Position.x;
		int y = item->Pose.Position.y;
		int z = item->Pose.Position.z;

		int dx = 942 * phd_sin(item->Pose.Orientation.y);
		int dz = 942 * phd_cos(item->Pose.Orientation.y);

		x += dx;
		z += dz;
		int height1 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

		x += dx;
		z += dz;
		int height2 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

		x += dx;
		z += dz;
		int height3 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

		int height = 0;
		bool canJump1Sector = true;
		if (enemyItem && item->BoxNumber == enemyItem->BoxNumber ||
			y >= (height1 - CLICK(1.5f)) ||
			y >= (height2 + CLICK(1)) ||
			y <= (height2 - CLICK(1)))
		{
			height = height2;
			canJump1Sector = false;
		}

		bool canJump2Sectors = true;
		if (enemyItem && item->BoxNumber == enemyItem->BoxNumber ||
			y >= (height1 - CLICK(1.5f)) ||
			y >= (height - CLICK(1.5f)) ||
			y >= (height3 + CLICK(1)) ||
			y <= (height3 - CLICK(1)))
		{
			canJump2Sectors = false;
		}

		auto* currentCreature = creature;

		if (item->ItemFlags[1] == item->RoomNumber ||
			g_Level.Rooms[item->RoomNumber].itemNumber == NO_ITEM)
		{
			currentCreature = creature;
		}
		else
		{
			currentCreature = creature;
			creature->Enemy = LaraItem;
			ITEM_INFO* currentItem = NULL;
			for (short itemNum = g_Level.Rooms[item->RoomNumber].itemNumber; itemNum != NO_ITEM; itemNum = currentItem->NextItem)
			{
				currentItem = &g_Level.Items[itemNum];
				if ((currentItem->ObjectNumber == ID_SMALLMEDI_ITEM ||
					currentItem->ObjectNumber == ID_BIGMEDI_ITEM ||
					currentItem->ObjectNumber == ID_UZI_AMMO_ITEM) &&
					SameZone(creature, currentItem))
				{
					if (item->Status != ITEM_INVISIBLE)
					{
						creature->Enemy = currentItem;
						break;
					}
				}
			}
		}

		item->ItemFlags[1] = item->RoomNumber;

		// Handle goon firing
		if (creature->FiredWeapon)
		{
			PHD_VECTOR pos = { GoonGunBite.x, GoonGunBite.y, GoonGunBite.z };
			GetJointAbsPosition(item, &pos, GoonGunBite.meshNum);

			TriggerDynamicLight(pos.x, pos.y, pos.z, 4 * creature->FiredWeapon + 8, 24, 16, 4);
			creature->FiredWeapon--;
		}

		CollisionResult probe;

		if (item->HitPoints <= 0)
		{
			item->Floor = GetCollision(item).Position.Floor;
			currentCreature->LOT.IsMonkeying = false;

			switch (item->Animation.ActiveState)
			{
			case GOON_STATE_DEATH:
				item->Animation.Airborne = true;
				currentCreature->LOT.IsMonkeying = false;

				if (item->Pose.Position.y >= item->Floor)
				{
					item->Pose.Position.y = item->Floor;
					item->Animation.VerticalVelocity = 0;
					item->Animation.Airborne = false;
				}

				break;

			case GOON_STATE_MONKEY_TO_FREEFALL:
				item->Animation.TargetState = GOON_STATE_FREEFALL;
				item->Animation.Airborne = false;
				break;

			case GOON_STATE_FREEFALL:
				item->Animation.Airborne = true;

				if (item->Pose.Position.y >= item->Floor)
				{
					item->Pose.Position.y = item->Floor;
					item->Animation.VerticalVelocity = 0;
					item->Animation.Airborne = false;
					item->Animation.TargetState = GOON_STATE_FREEFALL_LAND_DEATH;
				}

				break;

			case GOON_STATE_FREEFALL_LAND_DEATH:
				item->Pose.Position.y = item->Floor;
				break;

			case GOON_STATE_MONKEY_GRAB:
			case GOON_STATE_MONKEY_IDLE:
			case GOON_STATE_MONKEY_FORWARD:
				item->Animation.AnimNumber = Objects[objectNumber].animIndex + GOON_ANIM_MONKEY_TO_FREEFALL;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = GOON_STATE_MONKEY_TO_FREEFALL;
				item->Animation.Velocity = 0;
				break;

			default:
				item->Animation.AnimNumber = Objects[objectNumber].animIndex + GOON_ANIM_STAND_DEATH;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = GOON_STATE_DEATH;
				currentCreature->LOT.IsJumping = true;

				// OCB: respawn code for GOON_1
				if (item->TriggerFlags > 999)
				{
					for (int i = 0; i < g_Level.NumItems; i++)
					{
						auto* possibleEnemy = &g_Level.Items[i];

						if (possibleEnemy->ObjectNumber == ID_GOON1 || possibleEnemy->ObjectNumber == ID_GOON2 &&
							(item->TriggerFlags / 1000) == (possibleEnemy->TriggerFlags / 1000) - 1 &&
							!(possibleEnemy->Flags & IFLAG_KILLED))
						{
							if (EnableBaddieAI(i, 0))
								possibleEnemy->Status = ITEM_ACTIVE;
							else
								possibleEnemy->Status = ITEM_INVISIBLE;
							
							AddActiveItem(i);
							break;
						}
					}
				}

				break;
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(currentCreature);
			else if (!currentCreature->Enemy)
				currentCreature->Enemy = LaraItem;

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			AI_INFO laraAI;
			if (currentCreature->Enemy == LaraItem)
			{
				laraAI.angle = AI.angle;
				laraAI.ahead = AI.ahead;
				laraAI.distance = AI.distance;
			}
			else
			{
				dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
				laraAI.angle = phd_atan(dz, dx) - item->Pose.Orientation.y;
				laraAI.ahead = true;

				if (laraAI.angle <= -ANGLE(90.0f) || laraAI.angle >= ANGLE(90.0f))
					laraAI.ahead = false;

				laraAI.distance = dx * dx + dz * dz;
			}

			GetCreatureMood(item, &AI, VIOLENT);

			// Vehicle handling
			if (Lara.Vehicle != NO_ITEM && AI.bite)
				currentCreature->Mood = MoodType::Escape;

			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, currentCreature->MaxTurn);

			//currentCreature->enemy = LaraItem;

			//ITEM_INFO* oldEnemy = creature->enemy;
			//creature->enemy = LaraItem;

			// Is goon alerted?
			if (item->HitStatus ||
				laraAI.distance < pow(SECTOR(1), 2) ||
				TargetVisible(item, &laraAI) &&
				abs(LaraItem->Pose.Position.y - item->Pose.Position.y) < CLICK(4))
			{
				currentCreature->Alerted = true;
			}

			if (item != Lara.TargetEntity ||
				laraAI.distance <= 942 ||
				laraAI.angle <= -ANGLE(56.25f) ||
				laraAI.angle >= ANGLE(56.25f))
			{
				roll = false;
				jump = false;
			}
			else
			{
				dx = 942 * phd_sin(item->Pose.Orientation.y + ANGLE(45.0f));
				dz = 942 * phd_cos(item->Pose.Orientation.y + ANGLE(45.0f));

				x = item->Pose.Position.x + dx;
				y = item->Pose.Position.y;
				z = item->Pose.Position.z + dz;
				int height4 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

				dx = 942 * phd_sin(item->Pose.Orientation.y + ANGLE(78.75f));
				dz = 942 * phd_cos(item->Pose.Orientation.y + ANGLE(78.75f));

				x = item->Pose.Position.x + dx;
				y = item->Pose.Position.y;
				z = item->Pose.Position.z + dz;
				int height5 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

				if (abs(height5 - item->Pose.Position.y) > CLICK(1))
					jump = false;
				else
				{
					jump = true;
					if ((height4 + CLICK(2)) >= item->Pose.Position.y)
						jump = false;
				}

				dx = 942 * phd_sin(item->Pose.Orientation.y - ANGLE(45.0f));
				dz = 942 * phd_cos(item->Pose.Orientation.y - ANGLE(45.0f));

				x = item->Pose.Position.x + dx;
				y = item->Pose.Position.y;
				z = item->Pose.Position.z + dz;
				int height6 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

				dx = 942 * phd_sin(item->Pose.Orientation.y - ANGLE(78.75f));
				dz = 942 * phd_cos(item->Pose.Orientation.y - ANGLE(78.75f));

				x = item->Pose.Position.x + dx;
				y = item->Pose.Position.y;
				z = item->Pose.Position.z + dz;
				int height7 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

				if (abs(height7 - item->Pose.Position.y) > CLICK(1) ||
					(height6 + CLICK(2)) >= item->Pose.Position.y)
				{
					roll = false;
					someFlag3 = false;
				}
				else
					roll = true;
			}

			switch (item->Animation.ActiveState)
			{
			case GOON_STATE_IDLE:
				currentCreature->MaxTurn = 0;
				currentCreature->Flags = 0;
				currentCreature->LOT.IsMonkeying = false;
				currentCreature->LOT.IsJumping = false;
				joint3 = AI.angle / 2;

				if (AI.ahead && item->AIBits != GUARD)
				{
					joint1 = AI.angle / 2;
					joint2 = AI.xAngle;
				}

				if (item->AIBits & GUARD)
				{
					joint3 = AIGuard(currentCreature);
					item->Animation.TargetState = 0;
					break;
				}

				if (item->SwapMeshFlags == MESHSWAPFLAGS_GOON_SWORD_NINJA &&
					item == Lara.TargetEntity &&
					laraAI.ahead &&
					laraAI.distance > pow(682, 2))
				{
					item->Animation.TargetState = GOON_STATE_DODGE_START;
					break;
				}

				if (Targetable(item, &AI) && item->ItemFlags[2] > 0)
				{
					if (item->SwapMeshFlags == MESHSWAPFLAGS_GOON_GUN)
					{
						item->Animation.TargetState = GOON_STATE_AIM;
						break;
					}

					if (item->SwapMeshFlags != MESHSWAPFLAGS_GOON_SWORD_SIMPLE && item->SwapMeshFlags != MESHSWAPFLAGS_GOON_SWORD_NINJA)
					{
						item->Animation.TargetState = GOON_STATE_DRAW_GUN;
						break;
					}

					item->Animation.TargetState = GOON_STATE_HOLSTER_SWORD;
					break;
				}

				if (item->AIBits & MODIFY)
				{
					item->Animation.TargetState = GOON_STATE_IDLE;

					if (item->Floor > item->Pose.Position.y + CLICK(3))
						item->AIBits &= ~MODIFY;

					break;
				}

				if (canJump1Sector || canJump2Sectors)
				{
					currentCreature->MaxTurn = 0;
					currentCreature->LOT.IsJumping = true;
					item->Animation.AnimNumber = Objects[objectNumber].animIndex + GOON_ANIM_STAND_TO_JUMP_FORWARD;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					item->Animation.ActiveState = GOON_STATE_JUMP_FORWARD_1_BLOCK;

					if (!canJump2Sectors)
						item->Animation.TargetState = GOON_STATE_JUMP_FORWARD_1_BLOCK;
					else
						item->Animation.TargetState = GOON_STATE_JUMP_FORWARD_2_BLOCKS;

					break;
				}

				if (currentCreature->Enemy)
				{
					short objectNumber = currentCreature->Enemy->ObjectNumber;
					if ((objectNumber == ID_SMALLMEDI_ITEM || objectNumber == ID_UZI_AMMO_ITEM || objectNumber == ID_BIGMEDI_ITEM) &&
						AI.distance < pow(SECTOR(0.5f), 2))
					{
						item->Animation.TargetState = GOON_STATE_STAND_TO_CROUCH;
						item->Animation.RequiredState = GOON_STATE_CROUCH_PICKUP;
						break;
					}
				}

				if (item->SwapMeshFlags == MESHSWAPFLAGS_GOON_GUN && item->ItemFlags[2] < 1)
				{
					item->Animation.TargetState = GOON_STATE_HOLSTER_GUN;
					break;
				}

				if (currentCreature->MonkeySwingAhead)
				{
					probe = GetCollision(item);
					if (probe.Position.Ceiling == probe.Position.Floor - CLICK(6))
					{
						if (item->SwapMeshFlags == MESHSWAPFLAGS_GOON_EMPTY)
						{
							item->Animation.TargetState = GOON_STATE_MONKEY_GRAB;
							break;
						}

						if (item->SwapMeshFlags == MESHSWAPFLAGS_GOON_GUN)
						{
							item->Animation.TargetState = GOON_STATE_HOLSTER_GUN;
							break;
						}

						item->Animation.TargetState = GOON_STATE_HOLSTER_SWORD;
						break;
					}
				}
				else
				{
					if (roll)
					{
						currentCreature->MaxTurn = 0;
						item->Animation.TargetState = GOON_STATE_ROLL_LEFT;
						break;
					}

					if (jump)
					{
						currentCreature->MaxTurn = 0;
						item->Animation.TargetState = GOON_STATE_JUMP_RIGHT;
						break;
					}

					if (item->SwapMeshFlags == MESHSWAPFLAGS_GOON_EMPTY)
					{
						item->Animation.TargetState = GOON_STATE_DRAW_SWORD;
						break;
					}

					if (currentCreature->Enemy && currentCreature->Enemy->HitPoints > 0 && AI.distance < pow(682, 2))
					{
						if (item->SwapMeshFlags == MESHSWAPFLAGS_GOON_GUN)
							item->Animation.TargetState = GOON_STATE_HOLSTER_GUN;
						else if (AI.distance >= pow(SECTOR(0.5f), 2))
							item->Animation.TargetState = GOON_STATE_SWORD_HIT_FRONT;
						else if (GetRandomControl() & 1)
							item->Animation.TargetState = GOON_STATE_SWORD_HIT_LEFT;
						else
							item->Animation.TargetState = GOON_STATE_SWORD_HIT_RIGHT;
						
						break;
					}
				}

				item->Animation.TargetState = GOON_STATE_WALK;
				break;

			case GOON_STATE_WALK:
				currentCreature->MaxTurn = ANGLE(7.0f);
				currentCreature->Flags = 0;
				currentCreature->LOT.IsMonkeying = false;
				currentCreature->LOT.IsJumping = false;

				if (laraAI.ahead)
					joint3 = laraAI.angle;
				else if (laraAI.ahead)
					joint3 = laraAI.angle;

				if (Targetable(item, &AI) && item->ItemFlags[2] > 0)
				{
					item->Animation.TargetState = GOON_STATE_IDLE;
					break;
				}

				if (canJump1Sector || canJump2Sectors)
				{
					currentCreature->MaxTurn = 0;
					item->Animation.TargetState = GOON_STATE_IDLE;
					break;
				}

				if (currentCreature->ReachedGoal && currentCreature->MonkeySwingAhead)
				{
					item->Animation.TargetState = GOON_STATE_IDLE;
					break;
				}

				if (item->ItemFlags[2] < 1)
				{
					if (item->SwapMeshFlags != MESHSWAPFLAGS_GOON_SWORD_SIMPLE && item->SwapMeshFlags != MESHSWAPFLAGS_GOON_SWORD_NINJA)
					{
						item->Animation.TargetState = GOON_STATE_IDLE;
						break;
					}
				}

				if (AI.ahead && AI.distance < SECTOR(256))
				{
					item->Animation.TargetState = GOON_STATE_IDLE;
					break;
				}

				if (AI.bite)
				{
					if (AI.distance < pow(482, 2))
					{
						item->Animation.TargetState = GOON_STATE_IDLE;
						break;
					}

					if (AI.distance < pow(SECTOR(1), 2))
					{
						item->Animation.TargetState = GOON_STATE_WALK_SWORD_HIT_RIGHT;
						break;
					}
				}

				if (roll || jump)
				{
					item->Animation.ActiveState = GOON_STATE_IDLE;
					break;
				}

				if (currentCreature->Mood == MoodType::Attack &&
					!(currentCreature->JumpAhead) &&
					AI.distance > pow(SECTOR(1), 2))
				{
					item->Animation.TargetState = GOON_STATE_RUN;
				}

				break;

			case GOON_STATE_RUN:
				currentCreature->MaxTurn = ANGLE(11.0f);
				tilt = abs(angle) / 2;

				if (AI.ahead)
					joint3 = AI.angle;
				
				if (objectNumber == ID_GOON2 &&
					item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + FRAME_GOON_RUN_TO_SOMERSAULT &&
					height3 == height1 &&
					abs(height1 - item->Pose.Position.y) < CLICK(1.5f) &&
					(AI.angle > -ANGLE(22.5f) && AI.angle < ANGLE(22.5f) &&
						AI.distance < pow(SECTOR(3), 2) ||
						height2 >= (height1 + CLICK(2))))
				{
					item->Animation.TargetState = GOON_STATE_SOMERSAULT;
					currentCreature->MaxTurn = 0;
					break;
				}

				if (Targetable(item, &AI) &&
					item->ItemFlags[2] > 0 ||
					canJump1Sector ||
					canJump2Sectors ||
					currentCreature->MonkeySwingAhead ||
					item->AIBits & FOLLOW ||
					AI.distance < pow(614, 2) ||
					currentCreature->JumpAhead)
				{
					item->Animation.TargetState = GOON_STATE_IDLE;
					break;
				}

				if (AI.distance < pow(SECTOR(1), 2))
				{
					item->Animation.TargetState = GOON_STATE_WALK;
					break;
				}

				break;

			case GOON_STATE_SWORD_HIT_RIGHT:
			case GOON_STATE_SWORD_HIT_FRONT:
			case GOON_STATE_SWORD_HIT_LEFT:
			case GOON_STATE_WALK_SWORD_HIT_RIGHT:
				currentCreature->MaxTurn = 0;

				if (item->Animation.ActiveState == GOON_STATE_SWORD_HIT_RIGHT &&
					AI.distance < SECTOR(254))
				{
					item->Animation.TargetState = GOON_STATE_SWORD_HIT_LEFT;
				}

				if (AI.ahead)
				{
					joint1 = AI.angle;
					joint2 = AI.xAngle;
				}

				if (item->Animation.ActiveState != GOON_STATE_SWORD_HIT_FRONT ||
					item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + FRAME_GOON_SWORD_HIT_NO_DAMAGE_MAX)
				{
					if (abs(AI.angle) >= ANGLE(7.0f))
					{
						if (AI.angle >= 0)
							item->Pose.Orientation.y += ANGLE(7.0f);
						else
							item->Pose.Orientation.y -= ANGLE(7.0f);
					}
					else
						item->Pose.Orientation.y += AI.angle;
				}

				if (!currentCreature->Flags)
				{
					if (item->TouchBits & 0x1C000)
					{
						if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + FRAME_GOON_SWORD_HIT_DAMAGE_MIN &&
							item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + FRAME_GOON_SWORD_HIT_DAMAGE_MAX)
						{
							CreatureEffect2(
								item,
								&GoonSwordBite,
								10,
								item->Pose.Orientation.y,
								DoBloodSplat);

							currentCreature->Flags = 1;

							LaraItem->HitPoints -= 120;
							LaraItem->HitStatus = true;
						}
					}
				}

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd - 1)
					currentCreature->Flags = 0;

				break;

			case GOON_STATE_MONKEY_IDLE:
				currentCreature->MaxTurn = 0;
				currentCreature->Flags = 0;
				joint1 = 0;
				joint2 = 0;

				probe = GetCollision(item);

				if (laraAI.ahead && laraAI.distance < pow(682, 2) &&
					(LaraItem->Animation.ActiveState == LS_MONKEY_IDLE ||
						LaraItem->Animation.ActiveState == LS_MONKEY_FORWARD ||
						LaraItem->Animation.ActiveState == LS_MONKEY_SHIMMY_LEFT ||
						LaraItem->Animation.ActiveState == LS_MONKEY_SHIMMY_RIGHT ||
						LaraItem->Animation.ActiveState == LS_MONKEY_TURN_180 ||
						LaraItem->Animation.ActiveState == LS_MONKEY_TURN_LEFT ||
						LaraItem->Animation.ActiveState == LS_MONKEY_TURN_RIGHT))
				{
					item->Animation.TargetState = GOON_STATE_MONKEY_PUSH_OFF;
				}
				else if (item->BoxNumber != currentCreature->LOT.TargetBox &&
					currentCreature->MonkeySwingAhead ||
					probe.Position.Ceiling != (probe.Position.Floor - CLICK(6)))
				{
					item->Animation.TargetState = GOON_STATE_MONKEY_FORWARD;
				}
				else
				{
					item->Animation.TargetState = GOON_STATE_MONKEY_FALL_LAND;
					currentCreature->LOT.IsMonkeying = false;
					currentCreature->LOT.IsJumping = false;
				}

				break;

			case GOON_STATE_MONKEY_FORWARD:
				currentCreature->MaxTurn = ANGLE(7.0f);
				currentCreature->Flags = 0;
				currentCreature->LOT.IsJumping = true;
				currentCreature->LOT.IsMonkeying = true;
				joint1 = 0;
				joint2 = 0;

				if (item->BoxNumber == currentCreature->LOT.TargetBox ||
					!currentCreature->MonkeySwingAhead)
				{
					probe = GetCollision(item);

					if (probe.Position.Ceiling == probe.Position.Floor - CLICK(6))
						item->Animation.TargetState = GOON_STATE_MONKEY_IDLE;
				}

				if (laraAI.ahead)
				{
					if (laraAI.distance < pow(682, 2))
					{

						if (LaraItem->Animation.ActiveState == LS_MONKEY_IDLE ||
							LaraItem->Animation.ActiveState == LS_MONKEY_FORWARD ||
							LaraItem->Animation.ActiveState == LS_MONKEY_SHIMMY_LEFT ||
							LaraItem->Animation.ActiveState == LS_MONKEY_SHIMMY_RIGHT ||
							LaraItem->Animation.ActiveState == LS_MONKEY_TURN_180 ||
							LaraItem->Animation.ActiveState == LS_MONKEY_TURN_LEFT ||
							LaraItem->Animation.ActiveState == LS_MONKEY_TURN_RIGHT)
						{
							item->Animation.TargetState = GOON_STATE_MONKEY_IDLE;
						}
					}
				}

				break;

			case GOON_STATE_MONKEY_PUSH_OFF:
				currentCreature->MaxTurn = ANGLE(7.0f);

				if (currentCreature->Flags == someFlag3)
				{
					if (item->TouchBits)
					{
						LaraItem->Animation.ActiveState = LS_JUMP_UP;
						LaraItem->Animation.TargetState = LS_JUMP_UP;
						LaraItem->Animation.AnimNumber = LA_JUMP_UP;
						LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.FrameNumber].frameBase + 9;
						LaraItem->Animation.Airborne = true;
						LaraItem->Animation.VerticalVelocity = 2;
						LaraItem->Animation.VerticalVelocity = 1;
						LaraItem->Pose.Position.y += CLICK(0.75f);
						Lara.Control.HandStatus = HandStatus::Free;
						currentCreature->Flags = 1;
					}
				}

				break;

			case GOON_STATE_ROLL_LEFT:
			case GOON_STATE_JUMP_RIGHT:
				item->Status = ITEM_ACTIVE;
				currentCreature->MaxTurn = someFlag3;
				currentCreature->Alerted = false;
				break;

			case GOON_STATE_CROUCH:
				if (item->ItemFlags[0] == someFlag3)
				{
					if (currentCreature->Enemy)
					{
						if ((currentCreature->Enemy->ObjectNumber == ID_SMALLMEDI_ITEM ||
							currentCreature->Enemy->ObjectNumber == ID_BIGMEDI_ITEM ||
							currentCreature->Enemy->ObjectNumber == ID_UZI_AMMO_ITEM) &&
							AI.distance < pow(SECTOR(0.5f), 2))
						{
							item->Animation.TargetState = GOON_STATE_CROUCH_PICKUP;
							break;
						}
					}

					if (currentCreature->Alerted)
						item->Animation.TargetState = GOON_STATE_CROUCH_TO_STAND;
				}
				else
				{
					if (AI.distance >= pow(682, 2))
						break;
					
					item->Animation.TargetState = GOON_STATE_CROUCH_TO_STAND;
					currentCreature->Enemy = NULL;
				}

				break;

			case GOON_STATE_CROUCH_PICKUP:
				ClampRotation(&item->Pose, AI.angle, ANGLE(11.0f));

				if (item->Animation.FrameNumber != g_Level.Anims[item->Animation.AnimNumber].frameBase + FRAME_GOON_CROUCH_PICKUP)
					break;

				if (!currentCreature->Enemy)
					break;

				if (currentCreature->Enemy->ObjectNumber != ID_SMALLMEDI_ITEM &&
					currentCreature->Enemy->ObjectNumber != ID_BIGMEDI_ITEM &&
					currentCreature->Enemy->ObjectNumber != ID_UZI_AMMO_ITEM)
				{
					break;
				}

				if (currentCreature->Enemy->RoomNumber == NO_ROOM ||
					currentCreature->Enemy->Status == ITEM_INVISIBLE ||
					currentCreature->Enemy->InDrawRoom)
				{
					currentCreature->Enemy = NULL;
					break;
				}

				if (currentCreature->Enemy->ObjectNumber == ID_SMALLMEDI_ITEM)
					item->HitPoints += Objects[item->ObjectNumber].HitPoints / 2;
				else if (currentCreature->Enemy->ObjectNumber == ID_BIGMEDI_ITEM)
					item->HitPoints = Objects[item->ObjectNumber].HitPoints;
				else if (currentCreature->Enemy->ObjectNumber == ID_UZI_AMMO_ITEM)
					item->ItemFlags[2] += GOON_USE_UZI;
				else
				{
					currentCreature->Enemy = NULL;
					break;
				}
			
				KillItem(currentCreature->Enemy - g_Level.Items.data());

				// cancel enemy pointer for other active goons
				for (int i = 0; i < ActiveCreatures.size(); i++)
				{
					if (ActiveCreatures[i]->ItemNumber != NO_ITEM && ActiveCreatures[i]->ItemNumber != itemNumber && ActiveCreatures[i]->Enemy == creature->Enemy)
						ActiveCreatures[i]->Enemy = NULL;
				}

				creature->Enemy = NULL;
				break;

			case GOON_STATE_AIM:
				currentCreature->MaxTurn = 0;

				if (AI.ahead)
				{
					joint1 = AI.angle;
					joint2 = AI.xAngle;
				}
				ClampRotation(&item->Pose, AI.angle, ANGLE(7));

				if (!Targetable(item, &AI) ||
					item->ItemFlags[2] < 1)
				{
					item->Animation.TargetState = GOON_STATE_IDLE;
					break;
				}

				item->Animation.TargetState = GOON_STATE_FIRE;
				break;

			case GOON_STATE_FIRE:
				creature->FiredWeapon = true;

				if (AI.ahead)
				{
					joint1 = AI.angle;
					joint2 = AI.xAngle;
				}
				ClampRotation(&item->Pose, AI.angle, ANGLE(7.0f));

				if (item->Animation.FrameNumber >= g_Level.Anims[item->Animation.AnimNumber].frameBase + FRAME_GOON_FIRE_MAX ||
					item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + FRAME_GOON_FIRE_MIN)
				{
					break;
				}

				if (!item->HitStatus)
					item->ItemFlags[2]--;
				
				if (!ShotLara(item, &AI, &GoonGunBite, joint1, 15));
					item->Animation.TargetState = GOON_STATE_IDLE;

				break;

			default:
				break;

			case GOON_STATE_HOLSTER_GUN:
				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + FRAME_GOON_HOLSTER_GUN)
					item->SwapMeshFlags = MESHSWAPFLAGS_GOON_EMPTY;

				break;

			case GOON_STATE_DRAW_GUN:
				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + FRAME_GOON_DRAW_GUN)
					item->SwapMeshFlags = MESHSWAPFLAGS_GOON_GUN;

				break;

			case GOON_STATE_HOLSTER_SWORD:
				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + FRAME_GOON_HOLSTER_SWORD)
					item->SwapMeshFlags = MESHSWAPFLAGS_GOON_EMPTY;
				
				break;

			case GOON_STATE_DRAW_SWORD:
				if (item->Animation.FrameNumber != g_Level.Anims[item->Animation.AnimNumber].frameBase + FRAME_GOON_DRAW_SWORD)
					break;

				if (item->ObjectNumber == ID_GOON1)
					item->SwapMeshFlags = MESHSWAPFLAGS_GOON_SWORD_SIMPLE;
				else
					item->SwapMeshFlags = MESHSWAPFLAGS_GOON_SWORD_NINJA;

				break;

			case GOON_STATE_UNKNOWN_8:
				currentCreature->MaxTurn = 0;

				ClampRotation(&item->Pose, AI.angle, ANGLE(11.0f));

				if (laraAI.distance < pow(682, 2) ||
					item != Lara.TargetEntity)
				{
					item->Animation.TargetState = GOON_STATE_UNKNOWN_9;
				}

				break;

			case GOON_STATE_BLIND:
				if (!WeaponEnemyTimer)
				{
					if ((GetRandomControl() & 0x7F) == 0)
						item->Animation.TargetState = GOON_STATE_IDLE;
				}

				break;

			case GOON_STATE_SOMERSAULT:
				if (item->Animation.AnimNumber == Objects[objectNumber].animIndex + GOON_ANIM_SOMERSAULT_END)
				{
					ClampRotation(&item->Pose, AI.angle, ANGLE(7.0f));
					break;
				}

				if (item->Animation.FrameNumber != g_Level.Anims[item->Animation.AnimNumber].frameBase + FRAME_GOON_SOMERSAULT_START_TAKE_OFF)
					break;

				currentCreature->LOT.IsJumping = true;
				break;

			case GOON_STATE_JUMP_FORWARD_1_BLOCK:
			case GOON_STATE_JUMP_FORWARD_2_BLOCKS:
				if (item->ItemFlags[0] >= someFlag3)
					break;

				if (item->Animation.AnimNumber != Objects[objectNumber].animIndex + GOON_ANIM_STAND_TO_JUMP_FORWARD)
					item->ItemFlags[0] += 2;
				
				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, joint1);
		CreatureJoint(item, 1, joint2);
		CreatureJoint(item, 2, joint3);

		if (item->Animation.ActiveState >= GOON_STATE_JUMP_FORWARD_2_BLOCKS ||
			item->Animation.ActiveState == GOON_STATE_JUMP_FORWARD_1_BLOCK ||
			item->Animation.ActiveState == GOON_STATE_MONKEY_FORWARD ||
			item->Animation.ActiveState == GOON_STATE_DEATH ||
			item->Animation.ActiveState == GOON_STATE_SOMERSAULT ||
			item->Animation.ActiveState == GOON_STATE_BLIND)
		{
			CreatureAnimation(itemNumber, angle, 0);
		}
		else  if (WeaponEnemyTimer <= 100)
		{
			int vault = CreatureVault(itemNumber, angle, 2, 260);

			switch (vault)
			{
			case 2:
				item->Animation.AnimNumber = Objects[objectNumber].animIndex + GOON_ANIM_CLIMB_2_STEPS;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = GOON_STATE_CLIMB_2_STEPS;
				creature->MaxTurn = 0;
				break;

			case 3:
				item->Animation.AnimNumber = Objects[objectNumber].animIndex + GOON_ANIM_CLIMB_3_STEPS;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = GOON_STATE_CLIMB_3_STEPS;
				creature->MaxTurn = 0;
				break;

			case 4:
				item->Animation.AnimNumber = Objects[objectNumber].animIndex + GOON_ANIM_CLIMB_4_STEPS;
				item->Animation.ActiveState = GOON_STATE_CLIMB_4_STEPS;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				creature->MaxTurn = 0;
				break;

			case -3:
				item->Animation.AnimNumber = Objects[objectNumber].animIndex + GOON_ANIM_JUMP_OFF_3_STEPS;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = GOON_STATE_JUMP_OFF_3_STEPS;
				creature->MaxTurn = 0;
				break;

			case -4:
				item->Animation.AnimNumber = Objects[objectNumber].animIndex + GOON_ANIM_JUMP_OFF_4_STEPS;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = GOON_STATE_JUMP_OFF_4_STEPS;
				creature->MaxTurn = 0;
				break;

			default:
				return;
			}
		}
		else
		{
			item->Animation.AnimNumber = Objects[objectNumber].animIndex + GOON_ANIM_BLIND;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase + (GetRandomControl() & 7);
			item->Animation.ActiveState = GOON_STATE_BLIND;
			creature->MaxTurn = 0;
		}

		return;
	}
}
