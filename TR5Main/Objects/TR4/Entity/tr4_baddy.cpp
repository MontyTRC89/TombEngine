#include "framework.h"
#include "tr4_baddy.h"
#include "Game/items.h"
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

/*
ID_BADDY_1
1 - Rolls to the right 1 square
2 - Jumps to the left 1 square
3 - ducks when triggered
4 - Climbs up 4 clicks when triggered
101-104 – Slides to the left while crouching when triggered (eg. train level – just doesn’t work in trainmode)
1004 - Climbs up 6 clicks when triggered
1000 – N x 1000 – Is activated once the baddie with the previous thousand is dead and needs no trigger (have tested up to 20.000). Must be placed in room 2 of a level.
This means that:
2000 - Attacks Lara after she kills 1st baddie triggered
3000 - Same as above but after she kills 2nd baddie triggered
4000 - Same as above but after she kills 3rd baddie triggered
6000 - Same as above but after she kills 5th baddie triggered
etc.

ID_BADDY_2
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
101-104 - Slides to the left while crouching when triggered. The setup requires an enemy jeep and an AI_X1 nullmesh with the same OCB as the jeep and the baddy. It works only in trainmode. When triggered, the baddy will ride the roof of the enemy jeep parallel to the railtracks, until they reach the AI_X1 nullmesh. The baddy will jump off in the direction he’s placed in the map, while the jeep will fall back.
*/

namespace TEN::Entities::TR4
{
	enum BADDY_STATES 
	{
		STATE_BADDY_STOP = 0,
		STATE_BADDY_WALK = 1,
		STATE_BADDY_RUN = 2,
		// 3
		STATE_BADDY_DODGE_START = 4,
		// 5
		// 6
		// 7
		STATE_BADDY_UNKNOWN_8 = 8,
		STATE_BADDY_UNKNOWN_9 = 9,
		STATE_BADDY_DRAW_GUN = 10,
		STATE_BADDY_HOLSTER_GUN = 11,
		STATE_BADDY_DRAW_SWORD = 12,
		STATE_BADDY_HOLSTER_SWORD = 13,
		STATE_BADDY_FIRE = 14,
		STATE_BADDY_SWORD_HIT_FRONT = 15,
		STATE_BADDY_SWORD_HIT_RIGHT = 16,
		STATE_BADDY_SWORD_HIT_LEFT = 17,
		STATE_BADDY_MONKEY_GRAB = 18,
		STATE_BADDY_MONKEY_IDLE = 19,
		STATE_BADDY_MONKEY_FORWARD = 20,
		STATE_BADDY_MONKEY_PUSH_OFF = 21,
		STATE_BADDY_MONKEY_FALL_LAND = 22,
		STATE_BADDY_ROLL_LEFT = 23,
		STATE_BADDY_JUMP_RIGHT = 24,
		STATE_BADDY_STAND_TO_CROUCH = 25,
		STATE_BADDY_CROUCH = 26,
		STATE_BADDY_CROUCH_PICKUP = 27,
		STATE_BADDY_CROUCH_TO_STAND = 28,
		STATE_BADDY_WALK_SWORD_HIT_RIGHT = 29,
		STATE_BADDY_SOMERSAULT = 30,
		STATE_BADDY_AIM = 31,
		STATE_BADDY_DEATH = 32,
		STATE_BADDY_JUMP_FORWARD_1_BLOCK = 33,
		STATE_BADDY_JUMP_FORWARD_FALL = 34,
		STATE_BADDY_MONKEY_TO_FREEFALL = 35,
		STATE_BADDY_FREEFALL = 36,
		STATE_BADDY_FREEFALL_LAND_DEATH = 37,
		STATE_BADDY_JUMP_FORWARD_2_BLOCKS = 38,
		STATE_BADDY_CLIMB_4_CLICKS = 39,
		STATE_BADDY_CLIMB_3_CLICKS = 40,
		STATE_BADDY_CLIMB_2_CLICKS = 41,
		STATE_BADDY_JUMP_OFF_4_CLICKS = 42,
		STATE_BADDY_JUMP_OFF_3_CLICKS = 43,
		STATE_BADDY_BLIND = 44
	};

	enum BADDY_ANIM {
		ANIMATION_BADDY_RUN = 0,
		ANIMATION_BADDY_RUN_STOP_START = 1,
		ANIMATION_BADDY_RUN_STOP_END = 2,
		ANIMATION_BADDY_SOMERSAULT_START = 3,
		ANIMATION_BADDY_SOMERSAULT_END = 4,
		ANIMATION_BADDY_DODGE_START = 5,
		// 6
		// 7
		// 8
		ANIMATION_BADDY_MONKEY_GRAB = 9,
		ANIMATION_BADDY_MONKEY_IDLE = 10,
		ANIMATION_BADDY_MONKEY_FORWARD = 11,
		ANIMATION_BADDY_MONKEY_IDLE_TO_FORWARD = 12,
		ANIMATION_BADDY_MONKEY_STOP_LEFT = 13,
		ANIMATION_BADDY_MONKEY_STOP_RIGHT = 14,
		ANIMATION_BADDY_MONKEY_FALL_LAND = 15,
		ANIMATION_BADDY_MONKEY_PUSH_OFF = 16,
		ANIMATION_BADDY_DODGE_END = 17,
		ANIMATION_BADDY_STAND_IDLE = 18,
		ANIMATION_BADDY_DODGE_END_TO_STAND = 19,
		ANIMATION_BADDY_DRAW_GUN = 20,
		ANIMATION_BADDY_HOLSTER_GUN = 21,
		ANIMATION_BADDY_DRAW_SWORD = 22,
		ANIMATION_BADDY_HOLSTER_SWORD = 23,
		ANIMATION_BADDY_STAND_TO_ROLL_LEFT = 24,
		ANIMATION_BADDY_ROLL_LEFT_START = 25,
		ANIMATION_BADDY_ROLL_LEFT_CONTINUE = 26,
		ANIMATION_BADDY_ROLL_LEFT_END = 27,
		ANIMATION_BADDY_ROLL_LEFT_TO_CROUCH = 28,
		ANIMATION_BADDY_CROUCH = 29,
		ANIMATION_BADDY_CROUCH_TO_STAND = 30,
		ANIMATION_BADDY_STAND_TO_WALK = 31,
		ANIMATION_BADDY_WALK = 32,
		ANIMATION_BADDY_WALK_TO_RUN = 33,
		ANIMATION_BADDY_STAND_TO_AIM = 34,
		ANIMATION_BADDY_AIM = 35,
		ANIMATION_BADDY_FIRE = 36,
		ANIMATION_BADDY_AIM_TO_STAND = 37,
		ANIMATION_BADDY_SWORD_HIT_FRONT = 38,
		ANIMATION_BADDY_CROUCH_PICKUP = 39,
		ANIMATION_BADDY_STAND_TO_CROUCH = 40,
		ANIMATION_BADDY_SWORD_HIT_RIGHT = 41,
		ANIMATION_BADDY_SWORD_HIT_RIGHT_TO_LEFT = 42,
		ANIMATION_BADDY_SWORD_HIT_RIGHT_TO_STAND = 43,
		ANIMATION_BADDY_SWORD_HIT_LEFT = 44,
		ANIMATION_BADDY_STAND_DEATH = 45,
		ANIMATION_BADDY_WALK_SWORD_HIT_RIGHT = 46,
		ANIMATION_BADDY_STAND_TO_JUMP_RIGHT = 47,
		ANIMATION_BADDY_JUMP_RIGHT_START = 48,
		ANIMATION_BADDY_JUMP_RIGHT_CONTINUE = 49,
		ANIMATION_BADDY_JUMP_RIGHT_END = 50,
		ANIMATION_BADDY_RUN_TO_WALK = 51,
		// 52
		// 53
		ANIMATION_BADDY_WALK_STOP_RIGHT = 54,
		ANIMATION_BADDY_STAND_TO_JUMP_FORWARD = 55,
		ANIMATION_BADDY_JUMP_FORWARD_1_BLOCK = 56,
		ANIMATION_BADDY_JUMP_FORWARD_FALL = 57,
		ANIMATION_BADDY_JUMP_FORWARD_LAND = 58,
		ANIMATION_BADDY_MONKEY_TO_FREEFALL = 59,
		ANIMATION_BADDY_FREEFALL = 60,
		ANIMATION_BADDY_FREEFALL_LAND_DEATH = 61,
		ANIMATION_BADDY_CLIMB_4_CLICKS = 62,
		ANIMATION_BADDY_CLIMB_3_CLICKS = 63,
		ANIMATION_BADDY_CLIMB_2_CLICKS = 64,
		ANIMATION_BADDY_JUMP_OFF_4_CLICKS = 65,
		ANIMATION_BADDY_JUMP_OFF_3_CLICKS = 66,
		ANIMATION_BADDY_JUMP_FORWARD_2_BLOCKS = 67,
		ANIMATION_BADDY_BLIND = 68,
		ANIMATION_BADDY_BLIND_TO_STAND = 69,
		ANIMATION_BADDY_DEAD = 70,
	};

	enum BADDY_FRAMES {
		FRAME_BADDY_HOLSTER_GUN = 20,
		FRAME_BADDY_DRAW_GUN = 21,
		FRAME_BADDY_HOLSTER_SWORD = 22,
		FRAME_BADDY_DRAW_SWORD = 12,
		FRAME_BADDY_RUN_TO_SOMERSAULT = 11,
		FRAME_BADDY_SWORD_HIT_NO_DAMAGE_MAX = 12,
		FRAME_BADDY_SWORD_HIT_DAMAGE_MIN = 13,
		FRAME_BADDY_SWORD_HIT_DAMAGE_MAX = 21,
		FRAME_BADDY_CROUCH_PICKUP = 9,
		FRAME_BADDY_FIRE_MIN = 1,
		FRAME_BADDY_FIRE_MAX = 13,
		FRAME_BADDY_SOMERSAULT_START_TAKE_OFF = 18,
	};

	enum BADDY_SWAP_MESH_FLAGS {
		SWAPMESHFLAGS_BADDY_EMPTY = 0x7FC800,
		SWAPMESHFLAGS_BADDY_SWORD_SIMPLE = 0x7E0880,
		SWAPMESHFLAGS_BADDY_SWORD_NINJA = 0x000880,
		SWAPMESHFLAGS_BADDY_GUN = 0x7FC010,
	};

	#define BADDY_USE_UZI	24

	BITE_INFO baddyGun = { 0, -16, 200, 11 };
	BITE_INFO baddySword = { 0, 0, 0, 15 };

	void InitialiseBaddy(short itemNum)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];
	
		ClearItem(itemNum);

		short objectNumber = (Objects[ID_BADDY2].loaded ? ID_BADDY2 : ID_BADDY1);

		if (item->ObjectNumber == ID_BADDY1)
		{
			item->SwapMeshFlags = SWAPMESHFLAGS_BADDY_GUN;
			item->MeshBits = 0xFF81FFFF;
			item->ItemFlags[2] = BADDY_USE_UZI;
		}
		else
		{
			item->SwapMeshFlags = SWAPMESHFLAGS_BADDY_SWORD_NINJA;
			item->MeshBits = -1;
			item->ItemFlags[2] = 0;
		}
	
		item->ItemFlags[1] = -1;

		short ocb = item->TriggerFlags % 1000;

		// To the same things of OCB 1, 2, 3, 4 but also drawing uzis
		if (ocb > 9 && ocb < 20)
		{
			item->ItemFlags[2] += BADDY_USE_UZI;
			item->TriggerFlags -= 10;
			ocb -= 10;
		}
	
		if (!ocb || ocb > 4 && ocb < 7)
		{
			item->AnimNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_STAND_IDLE;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->TargetState = STATE_BADDY_STOP;
			item->ActiveState = STATE_BADDY_STOP;

			return;
		}

		// OCB: jump right
		if (ocb == 1)
		{
			item->AnimNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_STAND_TO_JUMP_RIGHT;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->TargetState = STATE_BADDY_JUMP_RIGHT;
			item->ActiveState = STATE_BADDY_JUMP_RIGHT;

			return;
		}

		// OCB: jump left
		if (ocb == 2)
		{
			item->AnimNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_STAND_TO_ROLL_LEFT;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->TargetState = STATE_BADDY_ROLL_LEFT;
			item->ActiveState = STATE_BADDY_ROLL_LEFT;

			return;
		}
	
		// OCB: crouch
		if (ocb == 3)
		{
			item->AnimNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_CROUCH;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->TargetState = STATE_BADDY_CROUCH;
			item->ActiveState = STATE_BADDY_CROUCH;

			return;
		}

		// OCB: climb up 4 or 6 clicks 
		if (ocb == 4)
		{
			item->AnimNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_CLIMB_4_CLICKS;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->TargetState = STATE_BADDY_CLIMB_4_CLICKS;
			item->ActiveState = STATE_BADDY_CLIMB_4_CLICKS;
			item->Position.xPos += phd_sin(item->Position.yRot) * (STEP_SIZE * 4);
			item->Position.zPos += phd_cos(item->Position.yRot) * (STEP_SIZE * 4);

			return;
		}

		// OCB: crouch and jump in train levels?
		if (ocb > 100)
		{
			item->AnimNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_CROUCH;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->TargetState = STATE_BADDY_CROUCH;
			item->ActiveState = STATE_BADDY_CROUCH;
			item->Position.xPos += phd_sin(item->Position.yRot) * (STEP_SIZE * 4);
			item->Position.zPos += phd_cos(item->Position.yRot) * (STEP_SIZE * 4);
			item->ItemFlags[3] = ocb;

			return;
		}
	
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	}

	void BaddyControl(short itemNum)
	{
		if (!CreatureActive(itemNum))
			return;

		ITEM_INFO* item = &g_Level.Items[itemNum];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
		ITEM_INFO* enemyItem = creature->enemy;
		OBJECT_INFO* obj = &Objects[ID_BADDY1];

		short tilt = 0;
		short angle = 0;
		short joint1 = 0;
		short joint2 = 0;
		short joint3 = 0;

		// TODO: better add a second control routine for baddy 2 instead of mixing them?
		short objectNumber = (Objects[ID_BADDY2].loaded ? ID_BADDY2 : ID_BADDY1);

		int roll = false;
		int jump = false;
		int someFlag3 = false;

		if (item->TriggerFlags % 1000)
		{
			creature->LOT.isJumping = true;
			creature->maximumTurn = 0;
			if (item->TriggerFlags % 1000 > 100)
			{
				item->ItemFlags[0] = -80;
				FindAITargetObject(creature, ID_AI_X1);
			}
			item->TriggerFlags = 1000 * (item->TriggerFlags / 1000);
		}

		// Can baddy jump? Check for a distance of 1 and 2 sectors
		int x = item->Position.xPos;
		int y = item->Position.yPos;
		int z = item->Position.zPos;

		int dx = 942 * phd_sin(item->Position.yRot);
		int dz = 942 * phd_cos(item->Position.yRot);

		x += dx;
		z += dz;

		short roomNumber = item->RoomNumber;
		FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
		int height1 = GetFloorHeight(floor, x, y, z);

		x += dx;
		z += dz;

		roomNumber = item->RoomNumber;
		floor = GetFloor(x, y, z, &roomNumber);
		int height2 = GetFloorHeight(floor, x, y, z);

		x += dx;
		z += dz;

		roomNumber = item->RoomNumber;
		floor = GetFloor(x, y, z, &roomNumber);
		int height3 = GetFloorHeight(floor, x, y, z);

		int height = 0;
		bool canJump1sector = true;
		if (enemyItem && item->BoxNumber == enemyItem->BoxNumber
			|| y >= height1 - (STEP_SIZE * 1.5f)
			|| y >= height2 + STEP_SIZE
			|| y <= height2 - STEP_SIZE)
		{
			height = height2;
			canJump1sector = false;
		}

		bool canJump2sectors = true;
		if (enemyItem && item->BoxNumber == enemyItem->BoxNumber
			|| y >= height1 - (STEP_SIZE * 1.5f)
			|| y >= height - (STEP_SIZE * 1.5f)
			|| y >= height3 + STEP_SIZE
			|| y <= height3 - STEP_SIZE)
		{
			canJump2sectors = false;
		}

		CREATURE_INFO* currentCreature = creature;

		if (item->ItemFlags[1] == item->RoomNumber
			|| g_Level.Rooms[item->RoomNumber].itemNumber == NO_ITEM)
		{
			currentCreature = creature;
		}
		else
		{
			currentCreature = creature;
			creature->enemy = LaraItem;
			ITEM_INFO* currentItem = NULL;
			for (short itemNum = g_Level.Rooms[item->RoomNumber].itemNumber; itemNum != NO_ITEM; itemNum = currentItem->NextItem)
			{
				currentItem = &g_Level.Items[itemNum];
				if ((currentItem->ObjectNumber == ID_SMALLMEDI_ITEM 
					|| currentItem->ObjectNumber == ID_BIGMEDI_ITEM
					|| currentItem->ObjectNumber == ID_UZI_AMMO_ITEM) 
					&& SameZone(creature, currentItem))
				{
					if (item->Status != ITEM_INVISIBLE)
					{
						creature->enemy = currentItem;
						break;
					}
				}
			}
		}

		item->ItemFlags[1] = item->RoomNumber;

		// Handle baddy firing
		if (item->FiredWeapon)
		{
			PHD_VECTOR pos;

			pos.x = baddyGun.x;
			pos.y = baddyGun.y;
			pos.z = baddyGun.z;

			GetJointAbsPosition(item, &pos, baddyGun.meshNum);
			TriggerDynamicLight(pos.x, pos.y, pos.z, 4 * item->FiredWeapon + 8, 24, 16, 4);
			item->FiredWeapon--;
		}

		if (item->HitPoints <= 0)
		{
			currentCreature->LOT.isMonkeying = false;

			roomNumber = item->RoomNumber;
			floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
			height = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
			item->Floor = height;

			switch (item->ActiveState)
			{
			case STATE_BADDY_DEATH:
				item->Airborne = true;
				currentCreature->LOT.isMonkeying = false;
				if (item->Position.yPos >= item->Floor)
				{
					item->Position.yPos = item->Floor;
					item->VerticalVelocity = 0;
					item->Airborne = false;
				}
				break;

			case STATE_BADDY_MONKEY_TO_FREEFALL:
				item->TargetState = STATE_BADDY_FREEFALL;
				item->Airborne = false;
				break;

			case STATE_BADDY_FREEFALL:
				item->Airborne = true;
				if (item->Position.yPos >= item->Floor)
				{
					item->Position.yPos = item->Floor;
					item->VerticalVelocity = 0;
					item->Airborne = false;
					item->TargetState = STATE_BADDY_FREEFALL_LAND_DEATH;
				}
				break;

			case STATE_BADDY_FREEFALL_LAND_DEATH:
				item->Position.yPos = item->Floor;
				break;

			case STATE_BADDY_MONKEY_GRAB:
			case STATE_BADDY_MONKEY_IDLE:
			case STATE_BADDY_MONKEY_FORWARD:
				item->AnimNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_MONKEY_TO_FREEFALL;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				item->ActiveState = STATE_BADDY_MONKEY_TO_FREEFALL;
				item->Velocity = 0;
				break;

			default:
				currentCreature->LOT.isJumping = true;
				item->AnimNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_STAND_DEATH;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				item->ActiveState = STATE_BADDY_DEATH;

				// OCB: respawn code for BADDY_1
				if (item->TriggerFlags > 999)
				{
					for (int i = 0; i < g_Level.NumItems; i++)
					{
						ITEM_INFO* possibleEnemy = &g_Level.Items[i];
						if (possibleEnemy->ObjectNumber == ID_BADDY1 || possibleEnemy->ObjectNumber == ID_BADDY2
							&& (item->TriggerFlags / 1000) == (possibleEnemy->TriggerFlags / 1000) - 1 
							&& !(possibleEnemy->Flags & IFLAG_KILLED))
						{
							if (EnableBaddieAI(i, 0))
							{
								possibleEnemy->Status = ITEM_ACTIVE;
							}
							else
							{
								possibleEnemy->Status = ITEM_INVISIBLE;
							}
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
			else if (!currentCreature->enemy)
				currentCreature->enemy = LaraItem;

			AI_INFO info;
			AI_INFO laraInfo;

			CreatureAIInfo(item, &info);

			if (currentCreature->enemy == LaraItem)
			{
				laraInfo.angle = info.angle;
				laraInfo.ahead = info.ahead;
				laraInfo.distance = info.distance;
			}
			else
			{
				dx = LaraItem->Position.xPos - item->Position.xPos;
				dz = LaraItem->Position.zPos - item->Position.zPos;
				laraInfo.angle = phd_atan(dz, dx) - item->Position.yRot;
				laraInfo.ahead = true;

				if (laraInfo.angle <= -ANGLE(90) || laraInfo.angle >= ANGLE(90))
					laraInfo.ahead = false;

				laraInfo.distance = dx * dx + dz * dz;
			}

			GetCreatureMood(item, &info, VIOLENT);

			// Vehicle handling
			if (Lara.Vehicle != NO_ITEM && info.bite)
				currentCreature->mood = ESCAPE_MOOD;

			CreatureMood(item, &info, VIOLENT);

			angle = CreatureTurn(item, currentCreature->maximumTurn);

			//currentCreature->enemy = LaraItem;

			//ITEM_INFO* oldEnemy = creature->enemy;
			//creature->enemy = LaraItem;

			// Is baddy alerted?
			if (item->HitStatus
				|| laraInfo.distance < SQUARE(1024)
				|| TargetVisible(item, &laraInfo) 
				&& abs(LaraItem->Position.yPos - item->Position.yPos) < STEP_SIZE * 4)
			{
				currentCreature->alerted = true;
			}

			if (item != Lara.target 
				|| laraInfo.distance <= 942 
				|| laraInfo.angle <= -ANGLE(56.25f) 
				|| laraInfo.angle >= ANGLE(56.25f))
			{
				roll = false;
				jump = false;
			}
			else
			{
				dx = 942 * phd_sin(item->Position.yRot + ANGLE(45));
				dz = 942 * phd_cos(item->Position.yRot + ANGLE(45));

				x = item->Position.xPos + dx;
				y = item->Position.yPos;
				z = item->Position.zPos + dz;

				roomNumber = item->RoomNumber;
				floor = GetFloor(x, y, z, &roomNumber);
				int height4 = GetFloorHeight(floor, x, y, z);

				dx = 942 * phd_sin(item->Position.yRot + ANGLE(78.75f));
				dz = 942 * phd_cos(item->Position.yRot + ANGLE(78.75f));

				x = item->Position.xPos + dx;
				y = item->Position.yPos;
				z = item->Position.zPos + dz;

				roomNumber = item->RoomNumber;
				floor = GetFloor(x, y, z, &roomNumber);
				int height5 = GetFloorHeight(floor, x, y, z);

				if (abs(height5 - item->Position.yPos) > STEP_SIZE)
					jump = false;
				else
				{
					jump = true;
					if (height4 + 512 >= item->Position.yPos)
						jump = false;
				}

				dx = 942 * phd_sin(item->Position.yRot - ANGLE(45));
				dz = 942 * phd_cos(item->Position.yRot - ANGLE(45));

				x = item->Position.xPos + dx;
				y = item->Position.yPos;
				z = item->Position.zPos + dz;

				roomNumber = item->RoomNumber;
				floor = GetFloor(x, y, z, &roomNumber);
				int height6 = GetFloorHeight(floor, x, y, z);

				dx = 942 * phd_sin(item->Position.yRot - ANGLE(78.75f));
				dz = 942 * phd_cos(item->Position.yRot - ANGLE(78.75f));

				x = item->Position.xPos + dx;
				y = item->Position.yPos;
				z = item->Position.zPos + dz;

				roomNumber = item->RoomNumber;
				floor = GetFloor(x, y, z, &roomNumber);
				int height7 = GetFloorHeight(floor, x, y, z);

				if (abs(height7 - item->Position.yPos) > STEP_SIZE || height6 + (STEP_SIZE * 2) >= item->Position.yPos)
				{
					roll = false;
					someFlag3 = false;
				}
				else
				{
					roll = true;
				}
			}

			switch (item->ActiveState)
			{
			case STATE_BADDY_STOP:
				currentCreature->LOT.isMonkeying = false;
				currentCreature->LOT.isJumping = false;
				currentCreature->flags = 0;
				currentCreature->maximumTurn = 0;
				joint3 = info.angle / 2;
				if (info.ahead && item->AIBits != GUARD)
				{
					joint1 = info.angle / 2;
					joint2 = info.xAngle;
				}

				if (item->AIBits & GUARD)
				{
					joint3 = AIGuard(currentCreature);
					item->TargetState = 0;
					break;
				}

				if (item->SwapMeshFlags == SWAPMESHFLAGS_BADDY_SWORD_NINJA
					&& item == Lara.target
					&& laraInfo.ahead
					&& laraInfo.distance > SQUARE(682))
				{
					item->TargetState = STATE_BADDY_DODGE_START;
					break;
				}

				if (Targetable(item, &info) && item->ItemFlags[2] > 0)
				{
					if (item->SwapMeshFlags == SWAPMESHFLAGS_BADDY_GUN)
					{
						item->TargetState = STATE_BADDY_AIM;
						break;
					}

					if (item->SwapMeshFlags != SWAPMESHFLAGS_BADDY_SWORD_SIMPLE && item->SwapMeshFlags != SWAPMESHFLAGS_BADDY_SWORD_NINJA)
					{
						item->TargetState = STATE_BADDY_DRAW_GUN;
						break;
					}

					item->TargetState = STATE_BADDY_HOLSTER_SWORD;
					break;
				}

				if (item->AIBits & MODIFY)
				{
					item->TargetState = STATE_BADDY_STOP;
					if (item->Floor > item->Position.yPos + (STEP_SIZE * 3))
						item->AIBits &= ~MODIFY;
					break;
				}

				if (canJump1sector || canJump2sectors)
				{
					currentCreature->maximumTurn = 0;
					item->AnimNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_STAND_TO_JUMP_FORWARD;
					item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
					item->ActiveState = STATE_BADDY_JUMP_FORWARD_1_BLOCK;
					currentCreature->LOT.isJumping = true;

					if (!canJump2sectors)
						item->TargetState = STATE_BADDY_JUMP_FORWARD_1_BLOCK;
					else
						item->TargetState = STATE_BADDY_JUMP_FORWARD_2_BLOCKS;
					break;
				}

				if (currentCreature->enemy)
				{
					short objNum = currentCreature->enemy->ObjectNumber;
					if ((objNum == ID_SMALLMEDI_ITEM || objNum == ID_UZI_AMMO_ITEM || objNum == ID_BIGMEDI_ITEM) 
						&& info.distance < SQUARE(512))
					{
						item->TargetState = STATE_BADDY_STAND_TO_CROUCH;
						item->RequiredState = STATE_BADDY_CROUCH_PICKUP;
						break;
					}
				}

				if (item->SwapMeshFlags == SWAPMESHFLAGS_BADDY_GUN && item->ItemFlags[2] < 1)
				{
					item->TargetState = STATE_BADDY_HOLSTER_GUN;
					break;
				}

				if (currentCreature->monkeyAhead)
				{
					floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
					height = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
					if (GetCeiling(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos) == height - (STEP_SIZE * 6))
					{
						if (item->SwapMeshFlags == SWAPMESHFLAGS_BADDY_EMPTY)
						{
							item->TargetState = STATE_BADDY_MONKEY_GRAB;
							break;
						}
						if (item->SwapMeshFlags == SWAPMESHFLAGS_BADDY_GUN)
						{
							item->TargetState = STATE_BADDY_HOLSTER_GUN;
							break;
						}

						item->TargetState = STATE_BADDY_HOLSTER_SWORD;
						break;
					}
				}
				else
				{
					if (roll)
					{
						currentCreature->maximumTurn = 0;
						item->TargetState = STATE_BADDY_ROLL_LEFT;
						break;
					}
					if (jump)
					{
						currentCreature->maximumTurn = 0;
						item->TargetState = STATE_BADDY_JUMP_RIGHT;
						break;
					}
					if (item->SwapMeshFlags == SWAPMESHFLAGS_BADDY_EMPTY)
					{
						item->TargetState = STATE_BADDY_DRAW_SWORD;
						break;
					}
					if (currentCreature->enemy && currentCreature->enemy->HitPoints > 0 && info.distance < SQUARE(682))
					{
						if (item->SwapMeshFlags == SWAPMESHFLAGS_BADDY_GUN)
						{
							item->TargetState = STATE_BADDY_HOLSTER_GUN;
						}
						else if (info.distance >= SQUARE(512))
						{
							item->TargetState = STATE_BADDY_SWORD_HIT_FRONT;
						}
						else if (GetRandomControl() & 1)
						{
							item->TargetState = STATE_BADDY_SWORD_HIT_LEFT;
						}
						else
						{
							item->TargetState = STATE_BADDY_SWORD_HIT_RIGHT;
						}
						break;
					}
				}

				item->TargetState = STATE_BADDY_WALK;
				break;

			case STATE_BADDY_WALK:
				currentCreature->LOT.isMonkeying = false;
				currentCreature->LOT.isJumping = false;
				currentCreature->maximumTurn = ANGLE(7);
				currentCreature->flags = 0;

				if (laraInfo.ahead)
				{
					joint3 = laraInfo.angle;
				}
				else if (laraInfo.ahead)
				{
					joint3 = laraInfo.angle;
				}
				if (Targetable(item, &info) && item->ItemFlags[2] > 0)
				{
					item->TargetState = STATE_BADDY_STOP;
					break;
				}
				if (canJump1sector || canJump2sectors)
				{
					currentCreature->maximumTurn = 0;
					item->TargetState = STATE_BADDY_STOP;
					break;
				}
				if (currentCreature->reachedGoal && currentCreature->monkeyAhead)
				{
					item->TargetState = STATE_BADDY_STOP;
					break;
				}

				if (item->ItemFlags[2] < 1)
				{
					if (item->SwapMeshFlags != SWAPMESHFLAGS_BADDY_SWORD_SIMPLE && item->SwapMeshFlags != SWAPMESHFLAGS_BADDY_SWORD_NINJA)
					{
						item->TargetState = STATE_BADDY_STOP;
						break;
					}
				}
				if (info.ahead && info.distance < 0x40000)
				{
					item->TargetState = STATE_BADDY_STOP;
					break;
				}
				if (info.bite)
				{
					if (info.distance < SQUARE(482))
					{
						item->TargetState = STATE_BADDY_STOP;
						break;
					}
					if (info.distance < SQUARE(1024))
					{
						item->TargetState = STATE_BADDY_WALK_SWORD_HIT_RIGHT;
						break;
					}
				}
				if (roll || jump)
				{
					item->ActiveState = STATE_BADDY_STOP;
					break;
				}
				if (currentCreature->mood == ATTACK_MOOD &&
					!(currentCreature->jumpAhead) &&
					info.distance > SQUARE(1024))
				{
					item->TargetState = STATE_BADDY_RUN;
				}
				break;

			case STATE_BADDY_RUN:
				if (info.ahead)
				{
					joint3 = info.angle;
				}
				currentCreature->maximumTurn = ANGLE(11);
				tilt = abs(angle) / 2;
				if (objectNumber == ID_BADDY2
					&& item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + FRAME_BADDY_RUN_TO_SOMERSAULT
					&& height3 == height1
					&& abs(height1 - item->Position.yPos) < STEP_SIZE * 1.5f
					&& (info.angle > -ANGLE(22.5f) && info.angle < ANGLE(22.5f) &&
						info.distance < SQUARE(3072)
						|| height2 >= height1 + 512))
				{
					item->TargetState = STATE_BADDY_SOMERSAULT;
					currentCreature->maximumTurn = 0;
					break;
				}
				if (Targetable(item, &info)
					&& item->ItemFlags[2] > 0
					|| canJump1sector
					|| canJump2sectors
					|| currentCreature->monkeyAhead
					|| item->AIBits & FOLLOW
					|| info.distance < SQUARE(614)
					|| currentCreature->jumpAhead)
				{
					item->TargetState = STATE_BADDY_STOP;
					break;
				}
				if (info.distance < SQUARE(1024))
				{
					item->TargetState = STATE_BADDY_WALK;
					break;
				}
				break;

			case STATE_BADDY_SWORD_HIT_RIGHT:
			case STATE_BADDY_SWORD_HIT_FRONT:
			case STATE_BADDY_SWORD_HIT_LEFT:
			case STATE_BADDY_WALK_SWORD_HIT_RIGHT:
				if (item->ActiveState == STATE_BADDY_SWORD_HIT_RIGHT &&
					info.distance < 0x40000)
				{
					item->TargetState = STATE_BADDY_SWORD_HIT_LEFT;
				}
				if (info.ahead)
				{
					joint1 = info.angle;
					joint2 = info.xAngle;
				}
				currentCreature->maximumTurn = 0;
				if (item->ActiveState != STATE_BADDY_SWORD_HIT_FRONT ||
					item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + FRAME_BADDY_SWORD_HIT_NO_DAMAGE_MAX)
				{
					if (abs(info.angle) >= ANGLE(7))
					{
						if (info.angle >= 0)
						{
							item->Position.yRot += ANGLE(7);
						}
						else
						{
							item->Position.yRot -= ANGLE(7);
						}
					}
					else
					{
						item->Position.yRot += info.angle;
					}
				}
				if (!currentCreature->flags)
				{
					if (item->TouchBits & 0x1C000)
					{
						if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + FRAME_BADDY_SWORD_HIT_DAMAGE_MIN &&
							item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + FRAME_BADDY_SWORD_HIT_DAMAGE_MAX)
						{
							LaraItem->HitPoints -= 120;
							LaraItem->HitStatus = true;
							CreatureEffect2(
								item,
								&baddySword,
								10,
								item->Position.yRot,
								DoBloodSplat);
							currentCreature->flags = 1;
						}
					}
				}
				if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd - 1)
				{
					currentCreature->flags = 0;
				}
				break;

			case STATE_BADDY_MONKEY_IDLE:
				joint2 = 0;
				joint1 = 0;
				currentCreature->maximumTurn = 0;
				currentCreature->flags = 0;

				floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
				height = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);

				if (laraInfo.ahead
					&& laraInfo.distance < SQUARE(682)
					&& (LaraItem->ActiveState == LS_MONKEY_IDLE
						|| LaraItem->ActiveState == LS_MONKEY_FORWARD
						|| LaraItem->ActiveState == LS_MONKEY_SHIMMY_LEFT
						|| LaraItem->ActiveState == LS_MONKEY_SHIMMY_RIGHT
						|| LaraItem->ActiveState == LS_MONKEY_TURN_180
						|| LaraItem->ActiveState == LS_MONKEY_TURN_LEFT
						|| LaraItem->ActiveState == LS_MONKEY_TURN_RIGHT))
				{
					item->TargetState = STATE_BADDY_MONKEY_PUSH_OFF;
				}
				else if (item->BoxNumber != currentCreature->LOT.targetBox
					&& currentCreature->monkeyAhead
					|| GetCeiling(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos) != height - (STEP_SIZE * 6))
				{
					item->TargetState = STATE_BADDY_MONKEY_FORWARD;
				}
				else
				{
					item->TargetState = STATE_BADDY_MONKEY_FALL_LAND;
					currentCreature->LOT.isMonkeying = false;
					currentCreature->LOT.isJumping = false;
				}
				break;

			case STATE_BADDY_MONKEY_FORWARD:
				joint2 = 0;
				joint1 = 0;
				currentCreature->LOT.isJumping = true;
				currentCreature->LOT.isMonkeying = true;
				currentCreature->flags = 0;
				currentCreature->maximumTurn = ANGLE(7);
				if (item->BoxNumber == currentCreature->LOT.targetBox ||
					!currentCreature->monkeyAhead)
				{
					floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
					height = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
					if (GetCeiling(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos) == height - (STEP_SIZE * 6))
					{
						item->TargetState = STATE_BADDY_MONKEY_IDLE;
					}
				}
				if (laraInfo.ahead)
				{
					if (laraInfo.distance < SQUARE(682))
					{

						if (LaraItem->ActiveState == LS_MONKEY_IDLE
							|| LaraItem->ActiveState == LS_MONKEY_FORWARD
							|| LaraItem->ActiveState == LS_MONKEY_SHIMMY_LEFT
							|| LaraItem->ActiveState == LS_MONKEY_SHIMMY_RIGHT
							|| LaraItem->ActiveState == LS_MONKEY_TURN_180
							|| LaraItem->ActiveState == LS_MONKEY_TURN_LEFT
							|| LaraItem->ActiveState == LS_MONKEY_TURN_RIGHT)
						{
							item->TargetState = STATE_BADDY_MONKEY_IDLE;
						}
					}
				}
				break;

			case STATE_BADDY_MONKEY_PUSH_OFF:
				currentCreature->maximumTurn = ANGLE(7);
				if (currentCreature->flags == someFlag3)
				{
					if (item->TouchBits)
					{
						LaraItem->ActiveState = LS_JUMP_UP;
						LaraItem->TargetState = LS_JUMP_UP;
						LaraItem->AnimNumber = LA_JUMP_UP;
						LaraItem->FrameNumber = g_Level.Anims[LaraItem->FrameNumber].frameBase + 9;
						LaraItem->Airborne = true;
						LaraItem->VerticalVelocity = 2;
						LaraItem->VerticalVelocity = 1;
						LaraItem->Position.yPos += (STEP_SIZE * 0.75f);
						Lara.Control.HandStatus = HandStatus::Free;
						currentCreature->flags = 1;
					}
				}
				break;

			case STATE_BADDY_ROLL_LEFT:
			case STATE_BADDY_JUMP_RIGHT:
				currentCreature->alerted = false;
				currentCreature->maximumTurn = someFlag3;
				item->Status = ITEM_ACTIVE;
				break;

			case STATE_BADDY_CROUCH:
				if (item->ItemFlags[0] == someFlag3)
				{
					if (currentCreature->enemy)
					{
						if ((currentCreature->enemy->ObjectNumber == ID_SMALLMEDI_ITEM
							|| currentCreature->enemy->ObjectNumber == ID_BIGMEDI_ITEM
							|| currentCreature->enemy->ObjectNumber == ID_UZI_AMMO_ITEM) 
							&& info.distance < SQUARE(512))
						{
							item->TargetState = STATE_BADDY_CROUCH_PICKUP;
							break;
						}
					}
					if (currentCreature->alerted)
					{
						item->TargetState = STATE_BADDY_CROUCH_TO_STAND;
					}
				}
				else
				{
					if (info.distance >= SQUARE(682))
					{
						break;
					}
					item->TargetState = STATE_BADDY_CROUCH_TO_STAND;
					currentCreature->enemy = NULL;
				}
				break;

			case STATE_BADDY_CROUCH_PICKUP:
				ClampRotation(&item->Position, info.angle, ANGLE(11));
				if (item->FrameNumber != g_Level.Anims[item->AnimNumber].frameBase + FRAME_BADDY_CROUCH_PICKUP)
				{
					break;
				}
				if (!currentCreature->enemy)
				{
					break;
				}
				if (currentCreature->enemy->ObjectNumber != ID_SMALLMEDI_ITEM &&
					currentCreature->enemy->ObjectNumber != ID_BIGMEDI_ITEM &&
					currentCreature->enemy->ObjectNumber != ID_UZI_AMMO_ITEM)
				{
					break;
				}
				if (currentCreature->enemy->RoomNumber == NO_ROOM ||
					currentCreature->enemy->Status == ITEM_INVISIBLE ||
					currentCreature->enemy->InDrawRoom)
				{
					currentCreature->enemy = NULL;
					break;
				}
				if (currentCreature->enemy->ObjectNumber == ID_SMALLMEDI_ITEM)
				{
					item->HitPoints += Objects[item->ObjectNumber].HitPoints / 2;
				}
				else if (currentCreature->enemy->ObjectNumber == ID_BIGMEDI_ITEM)
				{
					item->HitPoints = Objects[item->ObjectNumber].HitPoints;
				}
				else if (currentCreature->enemy->ObjectNumber == ID_UZI_AMMO_ITEM)
				{
					item->ItemFlags[2] += BADDY_USE_UZI;
				}
				else
				{
					currentCreature->enemy = NULL;
					break;
				}
			
				KillItem(currentCreature->enemy - g_Level.Items.data());

				// cancel enemy pointer for other active baddies
				for (int i = 0; i < ActiveCreatures.size(); i++)
				{
					if (ActiveCreatures[i]->itemNum != NO_ITEM && ActiveCreatures[i]->itemNum != itemNum && ActiveCreatures[i]->enemy == creature->enemy)
					{
						ActiveCreatures[i]->enemy = NULL;
					}
				}
				creature->enemy = NULL;

				break;

			case STATE_BADDY_AIM:
				currentCreature->maximumTurn = 0;
				if (info.ahead)
				{
					joint1 = info.angle;
					joint2 = info.xAngle;
				}
				ClampRotation(&item->Position, info.angle, ANGLE(7));
				if (!Targetable(item, &info)
					|| item->ItemFlags[2] < 1)
				{
					item->TargetState = STATE_BADDY_STOP;
					break;
				}
				item->TargetState = STATE_BADDY_FIRE;
				break;

			case STATE_BADDY_FIRE:
				if (info.ahead)
				{
					joint1 = info.angle;
					joint2 = info.xAngle;
				}
				ClampRotation(&item->Position, info.angle, ANGLE(7));
				if (item->FrameNumber >= g_Level.Anims[item->AnimNumber].frameBase + FRAME_BADDY_FIRE_MAX ||
					item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + FRAME_BADDY_FIRE_MIN)
				{
					break;
				}
				item->FiredWeapon = true;
				if (!item->HitStatus)
				{
					item->ItemFlags[2]--;
				}
				if (!ShotLara(item, &info, &baddyGun, joint1, 15));
				item->TargetState = STATE_BADDY_STOP;
				break;

			default:
				break;

			case STATE_BADDY_HOLSTER_GUN:
				if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + FRAME_BADDY_HOLSTER_GUN)
				{
					item->SwapMeshFlags = SWAPMESHFLAGS_BADDY_EMPTY;
				}
				break;

			case STATE_BADDY_DRAW_GUN:
				if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + FRAME_BADDY_DRAW_GUN)
				{
					item->SwapMeshFlags = SWAPMESHFLAGS_BADDY_GUN;
				}
				break;

			case STATE_BADDY_HOLSTER_SWORD:
				if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + FRAME_BADDY_HOLSTER_SWORD)
				{
					item->SwapMeshFlags = SWAPMESHFLAGS_BADDY_EMPTY;
				}
				break;

			case STATE_BADDY_DRAW_SWORD:
				if (item->FrameNumber != g_Level.Anims[item->AnimNumber].frameBase + FRAME_BADDY_DRAW_SWORD)
				{
					break;
				}
				if (item->ObjectNumber == ID_BADDY1)
				{
					item->SwapMeshFlags = SWAPMESHFLAGS_BADDY_SWORD_SIMPLE;
				}
				else
				{
					item->SwapMeshFlags = SWAPMESHFLAGS_BADDY_SWORD_NINJA;
				}
				break;

			case STATE_BADDY_UNKNOWN_8:
				currentCreature->maximumTurn = 0;
				ClampRotation(&item->Position, info.angle, ANGLE(11));
				if (laraInfo.distance < SQUARE(682) ||
					item != Lara.target)
				{
					item->TargetState = STATE_BADDY_UNKNOWN_9;
				}
				break;

			case STATE_BADDY_BLIND:
				if (!WeaponEnemyTimer)
				{
					if ((GetRandomControl() & 0x7F) == 0)
					{
						item->TargetState = STATE_BADDY_STOP;
					}
				}
				break;

			case STATE_BADDY_SOMERSAULT:
				if (item->AnimNumber == Objects[objectNumber].animIndex + ANIMATION_BADDY_SOMERSAULT_END)
				{
					ClampRotation(&item->Position, info.angle, ANGLE(7));
					break;
				}
				if (item->FrameNumber != g_Level.Anims[item->AnimNumber].frameBase + FRAME_BADDY_SOMERSAULT_START_TAKE_OFF)
				{
					break;
				}
				currentCreature->LOT.isJumping = true;
				break;

			case STATE_BADDY_JUMP_FORWARD_1_BLOCK:
			case STATE_BADDY_JUMP_FORWARD_2_BLOCKS:
				if (item->ItemFlags[0] >= someFlag3)
				{
					break;
				}
				if (item->AnimNumber != Objects[objectNumber].animIndex + ANIMATION_BADDY_STAND_TO_JUMP_FORWARD)
				{
					item->ItemFlags[0] += 2;
				}
				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, joint1);
		CreatureJoint(item, 1, joint2);
		CreatureJoint(item, 2, joint3);

		if (item->ActiveState >= STATE_BADDY_JUMP_FORWARD_2_BLOCKS ||
			item->ActiveState == STATE_BADDY_JUMP_FORWARD_1_BLOCK ||
			item->ActiveState == STATE_BADDY_MONKEY_FORWARD ||
			item->ActiveState == STATE_BADDY_DEATH ||
			item->ActiveState == STATE_BADDY_SOMERSAULT ||
			item->ActiveState == STATE_BADDY_BLIND)
		{
			CreatureAnimation(itemNum, angle, 0);
		}
		else  if (WeaponEnemyTimer <= 100)
		{
			int vault = CreatureVault(itemNum, angle, 2, 260);

			switch (vault)
			{
			case 2:
				creature->maximumTurn = 0;
				item->AnimNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_CLIMB_2_CLICKS;
				item->ActiveState = STATE_BADDY_CLIMB_2_CLICKS;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				break;

			case 3:
				creature->maximumTurn = 0;
				item->AnimNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_CLIMB_3_CLICKS;
				item->ActiveState = STATE_BADDY_CLIMB_3_CLICKS;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				break;

			case 4:
				creature->maximumTurn = 0;
				item->AnimNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_CLIMB_4_CLICKS;
				item->ActiveState = STATE_BADDY_CLIMB_4_CLICKS;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				break;

			case -3:
				creature->maximumTurn = 0;
				item->AnimNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_JUMP_OFF_3_CLICKS;
				item->ActiveState = STATE_BADDY_JUMP_OFF_3_CLICKS;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				break;

			case -4:
				creature->maximumTurn = 0;
				item->AnimNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_JUMP_OFF_4_CLICKS;
				item->ActiveState = STATE_BADDY_JUMP_OFF_4_CLICKS;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				break;

			default:
				return;
			}
		}
		else
		{
			creature->maximumTurn = 0;
			item->AnimNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_BLIND;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase + (GetRandomControl() & 7);
			item->ActiveState = STATE_BADDY_BLIND;
		}

		return;
	}
}