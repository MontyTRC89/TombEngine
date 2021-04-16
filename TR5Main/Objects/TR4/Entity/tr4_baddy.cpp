#include "framework.h"
#include "tr4_baddy.h"
#include "items.h"
#include "box.h"
#include "sphere.h"
#include "effect2.h"
#include "lara.h"
#include "people.h"
#include "effect.h"
#include "setup.h"
#include "level.h"
#include <draw.h>

enum BADDY_STATES {
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

BITE_INFO baddyGun = { 0, -16, 200, 11 };
BITE_INFO baddySword = { 0, 0, 0, 15 };

void InitialiseBaddy(short itemNum)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	
	ClearItem(itemNum);

	short objectNumber = (Objects[ID_BADDY2].loaded ? ID_BADDY2 : ID_BADDY1);

	if (item->objectNumber == ID_BADDY1)
	{
		item->swapMeshFlags = SWAPMESHFLAGS_BADDY_GUN;
		item->meshBits = 0xFF81FFFF;
		item->itemFlags[2] = 24;
	}
	else
	{
		item->swapMeshFlags = SWAPMESHFLAGS_BADDY_SWORD_NINJA;
		item->meshBits = -1;
		item->itemFlags[2] = 0;
	}
	
	item->itemFlags[1] = -1;

	short ocb = item->triggerFlags;

	if (ocb > 9 && ocb < 20)
	{
		item->itemFlags[2] += 24;
		item->triggerFlags = item->triggerFlags % 1000 - 10;
		ocb -= 10;
	}
	
	if (!ocb || ocb > 4 && ocb < 7)
	{
		item->animNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_STAND_IDLE;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = STATE_BADDY_STOP;
		item->currentAnimState = STATE_BADDY_STOP;

		return;
	}

	if (ocb == 1)
	{
		item->animNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_STAND_TO_JUMP_RIGHT;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = STATE_BADDY_JUMP_RIGHT;
		item->currentAnimState = STATE_BADDY_JUMP_RIGHT;

		return;
	}

	if (ocb == 2)
	{
		item->animNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_STAND_TO_ROLL_LEFT;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = STATE_BADDY_ROLL_LEFT;
		item->currentAnimState = STATE_BADDY_ROLL_LEFT;

		return;
	}
	
	if (ocb == 3)
	{
		item->animNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_CROUCH;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = STATE_BADDY_CROUCH;
		item->currentAnimState = STATE_BADDY_CROUCH;

		return;
	}

	if (ocb == 4)
	{
		item->animNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_CLIMB_4_CLICKS;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = STATE_BADDY_CLIMB_4_CLICKS;
		item->currentAnimState = STATE_BADDY_CLIMB_4_CLICKS;
		item->pos.xPos += phd_sin(item->pos.yRot) * (STEP_SIZE * 4);
		item->pos.zPos += phd_cos(item->pos.yRot) * (STEP_SIZE * 4);

		return;
	}

	if (ocb > 100)
	{
		item->animNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_CROUCH;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = STATE_BADDY_CROUCH;
		item->currentAnimState = STATE_BADDY_CROUCH;
		item->pos.xPos += phd_sin(item->pos.yRot) * (STEP_SIZE * 4);
		item->pos.zPos += phd_cos(item->pos.yRot) * (STEP_SIZE * 4);
		item->itemFlags[3] = ocb;

		return;
	}
	
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
}

void BaddyControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
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

	if (item->triggerFlags % 1000)
	{
		creature->LOT.isJumping = true;
		creature->maximumTurn = 0;
		if (item->triggerFlags % 1000 > 100)
		{
			item->itemFlags[0] = -80;
			FindAITargetObject(creature, ID_AI_X1);
		}
		item->triggerFlags = 1000 * (item->triggerFlags / 1000);
	}

	// Can baddy jump? Check for a distance of 1 and 2 sectors
	int x = item->pos.xPos;
	int y = item->pos.yPos;
	int z = item->pos.zPos;

	int dx = 942 * phd_sin(item->pos.yRot);
	int dz = 942 * phd_cos(item->pos.yRot);

	x += dx;
	z += dz;

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, y, z);

	x += dx;
	z += dz;

	roomNumber = item->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, y, z);

	x += dx;
	z += dz;

	roomNumber = item->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	int height3 = GetFloorHeight(floor, x, y, z);

	int height = 0;
	bool canJump1sector = true;
	if (enemyItem && item->boxNumber == enemyItem->boxNumber
		|| y >= height1 - (STEP_SIZE * 1.5f)
		|| y >= height2 + STEP_SIZE
		|| y <= height2 - STEP_SIZE)
	{
		height = height2;
		canJump1sector = false;
	}

	bool canJump2sectors = true;
	if (enemyItem && item->boxNumber == enemyItem->boxNumber
		|| y >= height1 - (STEP_SIZE * 1.5f)
		|| y >= height - (STEP_SIZE * 1.5f)
		|| y >= height3 + STEP_SIZE
		|| y <= height3 - STEP_SIZE)
	{
		canJump2sectors = false;
	}

	CREATURE_INFO* currentCreature = creature;

	if (item->itemFlags[1] == item->roomNumber
		|| g_Level.Rooms[item->roomNumber].itemNumber == NO_ITEM)
	{
		currentCreature = creature;
	}
	else
	{
		currentCreature = creature;
		ITEM_INFO* currentItem = &g_Level.Items[g_Level.Rooms[item->roomNumber].itemNumber];
		for (short itemNum = g_Level.Rooms[item->roomNumber].itemNumber; itemNum != NO_ITEM; itemNum = currentItem->nextItem)
		{
			currentItem = &g_Level.Items[itemNum];
			if ((currentItem->objectNumber == ID_SMALLMEDI_ITEM || currentItem->objectNumber == ID_UZI_AMMO_ITEM) 
				&& SameZone(creature, currentItem))
			{
				if (item->status != ITEM_INVISIBLE)
					break;
			}
		}
		creature->enemy = currentItem;
	}

	item->itemFlags[1] = item->roomNumber;

	// Handle baddy firing
	if (item->firedWeapon)
	{
		PHD_VECTOR pos;

		pos.x = baddyGun.x;
		pos.y = baddyGun.y;
		pos.z = baddyGun.z;

		GetJointAbsPosition(item, &pos, baddyGun.meshNum);
		TriggerDynamicLight(pos.x, pos.y, pos.z, 4 * item->firedWeapon + 8, 24, 16, 4);
		item->firedWeapon--;
	}

	if (item->hitPoints <= 0)
	{
		currentCreature->LOT.isMonkeying = false;

		roomNumber = item->roomNumber;
		floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		item->floor = height;

		switch (item->currentAnimState)
		{
		case STATE_BADDY_DEATH:
			item->gravityStatus = true;
			currentCreature->LOT.isMonkeying = false;
			if (item->pos.yPos >= item->floor)
			{
				item->pos.yPos = item->floor;
				item->fallspeed = 0;
				item->gravityStatus = false;
			}
			break;

		case STATE_BADDY_MONKEY_TO_FREEFALL:
			item->goalAnimState = STATE_BADDY_FREEFALL;
			item->gravityStatus = false;
			break;

		case STATE_BADDY_FREEFALL:
			item->gravityStatus = true;
			if (item->pos.yPos >= item->floor)
			{
				item->pos.yPos = item->floor;
				item->fallspeed = 0;
				item->gravityStatus = false;
				item->goalAnimState = STATE_BADDY_FREEFALL_LAND_DEATH;
			}
			break;

		case STATE_BADDY_FREEFALL_LAND_DEATH:
			item->pos.yPos = item->floor;
			break;

		case STATE_BADDY_MONKEY_GRAB:
		case STATE_BADDY_MONKEY_IDLE:
		case STATE_BADDY_MONKEY_FORWARD:
			item->animNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_MONKEY_TO_FREEFALL;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = STATE_BADDY_MONKEY_TO_FREEFALL;
			item->speed = 0;
			break;

		default:
			currentCreature->LOT.isJumping = true;
			item->animNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_STAND_DEATH;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = STATE_BADDY_DEATH;

			// TODO: baddy respawn setup with OCB
			break;
		}
	}
	else
	{
		if (item->aiBits)
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
			dx = LaraItem->pos.xPos - item->pos.xPos;
			dz = LaraItem->pos.zPos - item->pos.zPos;
			laraInfo.angle = phd_atan(dz, dx) - item->pos.yRot;
			laraInfo.ahead = true;

			if (laraInfo.angle <= -ANGLE(90) || laraInfo.angle >= ANGLE(90))
				laraInfo.ahead = false;

			laraInfo.distance = dx * dx + dz * dz;
		}

		GetCreatureMood(item, &info, VIOLENT);

		// Vehicle handling
		if (Lara.Vehicle != NO_ITEM && info.bite)
			currentCreature->mood == ESCAPE_MOOD;

		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, currentCreature->maximumTurn);

		//currentCreature->enemy = LaraItem;

		// Is baddy alerted?
		if (item->hitStatus
			|| laraInfo.distance < SQUARE(1024)
			|| TargetVisible(item, &laraInfo) && abs(LaraItem->pos.yPos - item->pos.yPos) < STEP_SIZE * 4)
		{
			currentCreature->alerted = true;
		}

		if (item != Lara.target || laraInfo.distance <= 942 ||
			laraInfo.angle <= -ANGLE(56.25f) || laraInfo.angle >= ANGLE(56.25f))
		{
			roll = false;
			jump = false;
		}

		dx = 942 * phd_sin(item->pos.yRot + ANGLE(45));
		dz = 942 * phd_cos(item->pos.yRot + ANGLE(45));

		x = item->pos.xPos + dx;
		y = item->pos.yPos;
		z = item->pos.zPos + dz;

		roomNumber = item->roomNumber;
		floor = GetFloor(x, y, z, &roomNumber);
		int height4 = GetFloorHeight(floor, x, y, z);

		dx = 942 * phd_sin(item->pos.yRot + ANGLE(78.75f));
		dz = 942 * phd_cos(item->pos.yRot + ANGLE(78.75f));

		x = item->pos.xPos + dx;
		y = item->pos.yPos;
		z = item->pos.zPos + dz;

		roomNumber = item->roomNumber;
		floor = GetFloor(x, y, z, &roomNumber);
		int height5 = GetFloorHeight(floor, x, y, z);

		if (abs(height5 - item->pos.yPos) > STEP_SIZE)
			jump = false;
		else
		{
			jump = true;
			if (height4 + 512 >= item->pos.yPos)
				jump = false;
		}

		dx = 942 * phd_sin(item->pos.yRot - ANGLE(45));
		dz = 942 * phd_cos(item->pos.yRot - ANGLE(45));

		x = item->pos.xPos + dx;
		y = item->pos.yPos;
		z = item->pos.zPos + dz;

		roomNumber = item->roomNumber;
		floor = GetFloor(x, y, z, &roomNumber);
		int height6 = GetFloorHeight(floor, x, y, z);

		dx = 942 * phd_sin(item->pos.yRot - ANGLE(78.75f));
		dz = 942 * phd_cos(item->pos.yRot - ANGLE(78.75f));

		x = item->pos.xPos + dx;
		y = item->pos.yPos;
		z = item->pos.zPos + dz;

		roomNumber = item->roomNumber;
		floor = GetFloor(x, y, z, &roomNumber);
		int height7 = GetFloorHeight(floor, x, y, z);

		if (abs(height7 - item->pos.yPos) > STEP_SIZE || height6 + (STEP_SIZE * 2) >= item->pos.yPos)
		{
			roll = false;
			someFlag3 = false;
		}
		else
		{
			roll = true;
		}

		switch (item->currentAnimState)
		{
		case STATE_BADDY_STOP:
			currentCreature->LOT.isMonkeying = false;
			currentCreature->LOT.isJumping = false;
			currentCreature->flags = 0;
			currentCreature->maximumTurn = 0;
			joint3 = info.angle / 2;
			if (info.ahead && item->aiBits & FOLLOW)
			{
				joint1 = info.angle / 2;
				joint2 = info.xAngle;
			}

			if (item->aiBits & GUARD)
			{
				joint3 = AIGuard(currentCreature);
				item->goalAnimState = 0;
				break;
			}

			if (item->swapMeshFlags == SWAPMESHFLAGS_BADDY_SWORD_NINJA
				&& item == Lara.target
				&& laraInfo.ahead
				&& laraInfo.distance > SQUARE(682))
			{
				item->goalAnimState = STATE_BADDY_DODGE_START;
				break;
			}

			if (Targetable(item, &info) && item->itemFlags[2] > 0)
			{
				if (item->swapMeshFlags == SWAPMESHFLAGS_BADDY_GUN)
				{
					item->goalAnimState = STATE_BADDY_AIM;
					break;
				}

				if (item->swapMeshFlags != SWAPMESHFLAGS_BADDY_SWORD_SIMPLE && item->swapMeshFlags != SWAPMESHFLAGS_BADDY_SWORD_NINJA)
				{
					item->goalAnimState = STATE_BADDY_DRAW_GUN;
					break;
				}

				item->goalAnimState = STATE_BADDY_STOP;
				break;
			}

			if (item->aiBits & MODIFY)
			{
				item->goalAnimState = STATE_BADDY_STOP;
				if (item->floor > item->pos.yPos + (STEP_SIZE * 3))
					item->aiBits &= ~MODIFY;
				break;
			}

			if (canJump1sector || canJump2sectors)
			{
				currentCreature->maximumTurn = 0;
				item->animNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_STAND_TO_JUMP_FORWARD;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = STATE_BADDY_JUMP_FORWARD_1_BLOCK;
				currentCreature->LOT.isJumping = true;

				if (!canJump2sectors)
					item->goalAnimState = STATE_BADDY_JUMP_FORWARD_1_BLOCK;
				else
					item->goalAnimState = STATE_BADDY_JUMP_FORWARD_2_BLOCKS;
				break;
			}

			if (currentCreature->enemy)
			{
				short objNum = currentCreature->enemy->objectNumber;
				if ((objNum == ID_SMALLMEDI_ITEM || objNum == ID_UZI_AMMO_ITEM) && info.distance < 0x40000)
				{
					item->goalAnimState = STATE_BADDY_STAND_TO_CROUCH;
					item->requiredAnimState = STATE_BADDY_CROUCH_PICKUP;
					break;
				}
			}

			if (item->swapMeshFlags == SWAPMESHFLAGS_BADDY_GUN && item->itemFlags[2] < 1)
			{
				item->goalAnimState = STATE_BADDY_HOLSTER_GUN;
				break;
			}

			if (currentCreature->monkeyAhead)
			{
				floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
				height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
				if (GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) == height - (STEP_SIZE * 6))
				{
					if (item->swapMeshFlags == SWAPMESHFLAGS_BADDY_EMPTY)
					{
						item->goalAnimState = STATE_BADDY_MONKEY_GRAB;
						break;
					}
					if (item->swapMeshFlags == SWAPMESHFLAGS_BADDY_GUN)
					{
						item->goalAnimState = STATE_BADDY_HOLSTER_GUN;
						break;
					}

					item->goalAnimState = STATE_BADDY_HOLSTER_SWORD;
					break;
				}
			}
			else
			{
				if (roll)
				{
					currentCreature->maximumTurn = 0;
					item->goalAnimState = STATE_BADDY_ROLL_LEFT;
					break;
				}
				if (jump)
				{
					currentCreature->maximumTurn = 0;
					item->goalAnimState = STATE_BADDY_JUMP_RIGHT;
					break;
				}
				if (item->swapMeshFlags == SWAPMESHFLAGS_BADDY_EMPTY)
				{
					item->goalAnimState = STATE_BADDY_DRAW_SWORD;
					break;
				}
				if (currentCreature->enemy && currentCreature->enemy->hitPoints > 0 && info.distance < SQUARE(682))
				{
					if (item->swapMeshFlags == SWAPMESHFLAGS_BADDY_GUN)
					{
						item->goalAnimState = STATE_BADDY_HOLSTER_GUN;
					}
					else if (info.distance >= 0x40000)
					{
						item->goalAnimState = STATE_BADDY_SWORD_HIT_FRONT;
					}
					else if (GetRandomControl() & 1)
					{
						item->goalAnimState = STATE_BADDY_SWORD_HIT_LEFT;
					}
					else
					{
						item->goalAnimState = STATE_BADDY_SWORD_HIT_RIGHT;
					}
					break;
				}
			}
			item->goalAnimState = STATE_BADDY_WALK;
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
			if (Targetable(item, &info) && item->itemFlags[2] > 0)
			{
				item->goalAnimState = STATE_BADDY_STOP;
				break;
			}
			if (canJump1sector || canJump2sectors)
			{
				currentCreature->maximumTurn = 0;
				item->goalAnimState = STATE_BADDY_STOP;
				break;
			}
			if (currentCreature->reachedGoal && currentCreature->monkeyAhead)
			{
				item->goalAnimState = STATE_BADDY_STOP;
				break;
			}

			if (item->itemFlags[2] < 1)
			{
				if (item->swapMeshFlags != SWAPMESHFLAGS_BADDY_SWORD_SIMPLE && item->swapMeshFlags != SWAPMESHFLAGS_BADDY_SWORD_NINJA)
				{
					item->goalAnimState = STATE_BADDY_STOP;
					break;
				}
			}
			if (info.ahead && info.distance < 0x40000)
			{
				item->goalAnimState = STATE_BADDY_STOP;
				break;
			}
			if (info.bite)
			{
				if (info.distance < SQUARE(482))
				{
					item->goalAnimState = STATE_BADDY_STOP;
					break;
				}
				if (info.distance < SQUARE(1024))
				{
					item->goalAnimState = STATE_BADDY_WALK_SWORD_HIT_RIGHT;
					break;
				}
			}
			if (roll || jump)
			{
				item->currentAnimState = STATE_BADDY_STOP;
				break;
			}
			if (currentCreature->mood == ATTACK_MOOD &&
				!(currentCreature->jumpAhead) &&
				info.distance > SQUARE(1024))
			{
				item->goalAnimState = STATE_BADDY_RUN;
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
				&& item->frameNumber == g_Level.Anims[item->animNumber].frameBase + FRAME_BADDY_RUN_TO_SOMERSAULT
				&& height3 == height1
				&& abs(height1 - item->pos.yPos) < STEP_SIZE * 1.5f
				&& (info.angle > -ANGLE(22.5f) && info.angle < ANGLE(22.5f) &&
					info.distance < SQUARE(3072)
					|| height2 >= height1 + 512))
			{
				item->goalAnimState = STATE_BADDY_SOMERSAULT;
				currentCreature->maximumTurn = 0;
				break;
			}
			if (Targetable(item, &info)
				&& item->itemFlags[2] > 0
				|| canJump1sector
				|| canJump2sectors
				|| currentCreature->monkeyAhead
				|| item->aiBits & FOLLOW
				|| info.distance < SQUARE(614)
				|| currentCreature->jumpAhead)
			{
				item->goalAnimState = STATE_BADDY_STOP;
				break;
			}
			if (info.distance < SQUARE(1024))
			{
				item->goalAnimState = STATE_BADDY_WALK;
				break;
			}
			break;

		case STATE_BADDY_SWORD_HIT_RIGHT:
		case STATE_BADDY_SWORD_HIT_FRONT:
		case STATE_BADDY_SWORD_HIT_LEFT:
		case STATE_BADDY_WALK_SWORD_HIT_RIGHT:
			if (item->currentAnimState == STATE_BADDY_SWORD_HIT_RIGHT &&
				info.distance < 0x40000)
			{
				item->goalAnimState = STATE_BADDY_SWORD_HIT_LEFT;
			}
			if (info.ahead)
			{
				joint1 = info.angle;
				joint2 = info.xAngle;
			}
			currentCreature->maximumTurn = 0;
			if (item->currentAnimState != STATE_BADDY_SWORD_HIT_FRONT ||
				item->frameNumber < g_Level.Anims[item->animNumber].frameBase + FRAME_BADDY_SWORD_HIT_NO_DAMAGE_MAX)
			{
				if (abs(info.angle) >= ANGLE(7))
				{
					if (info.angle >= 0)
					{
						item->pos.yRot += ANGLE(7);
					}
					else
					{
						item->pos.yRot -= ANGLE(7);
					}
				}
				else
				{
					item->pos.yRot += info.angle;
				}
			}
			if (!currentCreature->flags)
			{
				if (item->touchBits & 0x1C000)
				{
					if (item->frameNumber > g_Level.Anims[item->animNumber].frameBase + FRAME_BADDY_SWORD_HIT_DAMAGE_MIN &&
						item->frameNumber < g_Level.Anims[item->animNumber].frameBase + FRAME_BADDY_SWORD_HIT_DAMAGE_MAX)
					{
						LaraItem->hitPoints -= 120;
						LaraItem->hitStatus = true;
						CreatureEffect2(
							item,
							&baddySword,
							10,
							item->pos.yRot,
							DoBloodSplat);
						currentCreature->flags = 1;
					}
				}
			}
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd - 1)
			{
				currentCreature->flags = 0;
			}
			break;

		case STATE_BADDY_MONKEY_IDLE:
			joint2 = 0;
			joint1 = 0;
			currentCreature->maximumTurn = 0;
			currentCreature->flags = 0;

			floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
			height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

			if (laraInfo.ahead
				&& laraInfo.distance < SQUARE(682)
				&& (LaraItem->currentAnimState == LS_MONKEYSWING_IDLE
					|| LaraItem->currentAnimState == LS_MONKEYSWING_FORWARD
					|| LaraItem->currentAnimState == LS_MONKEYSWING_LEFT
					|| LaraItem->currentAnimState == LS_MONKEYSWING_RIGHT
					|| LaraItem->currentAnimState == LS_MONKEYSWING_TURN_180
					|| LaraItem->currentAnimState == LS_MONKEYSWING_TURN_LEFT
					|| LaraItem->currentAnimState == LS_MONKEYSWING_TURN_RIGHT))
			{
				item->goalAnimState = STATE_BADDY_MONKEY_PUSH_OFF;
			}
			else if (item->boxNumber != currentCreature->LOT.targetBox
				&& currentCreature->monkeyAhead
				|| GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) != height - (STEP_SIZE * 6))
			{
				item->goalAnimState = STATE_BADDY_MONKEY_FORWARD;
			}
			else
			{
				item->goalAnimState = STATE_BADDY_MONKEY_FALL_LAND;
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
			if (item->boxNumber == currentCreature->LOT.targetBox ||
				!currentCreature->monkeyAhead)
			{
				floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
				height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
				if (GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) == height - (STEP_SIZE * 6))
				{
					item->goalAnimState = STATE_BADDY_MONKEY_IDLE;
				}
			}
			if (laraInfo.ahead)
			{
				if (laraInfo.distance < SQUARE(682))
				{

					if (LaraItem->currentAnimState == LS_MONKEYSWING_IDLE
						|| LaraItem->currentAnimState == LS_MONKEYSWING_FORWARD
						|| LaraItem->currentAnimState == LS_MONKEYSWING_LEFT
						|| LaraItem->currentAnimState == LS_MONKEYSWING_RIGHT
						|| LaraItem->currentAnimState == LS_MONKEYSWING_TURN_180
						|| LaraItem->currentAnimState == LS_MONKEYSWING_TURN_LEFT
						|| LaraItem->currentAnimState == LS_MONKEYSWING_TURN_RIGHT)
					{
						item->goalAnimState = STATE_BADDY_MONKEY_IDLE;
					}
				}
			}
			break;

		case STATE_BADDY_MONKEY_PUSH_OFF:
			currentCreature->maximumTurn = ANGLE(7);
			if (currentCreature->flags == someFlag3)
			{
				if (item->touchBits)
				{
					LaraItem->currentAnimState = LS_JUMP_UP;
					LaraItem->goalAnimState = LS_JUMP_UP;
					LaraItem->animNumber = LA_JUMP_UP;
					LaraItem->frameNumber = g_Level.Anims[LaraItem->frameNumber].frameBase + 9;
					LaraItem->gravityStatus = true;
					LaraItem->speed = 2;
					LaraItem->fallspeed = 1;
					LaraItem->pos.yPos += (STEP_SIZE * 0.75f);
					Lara.gunStatus = LG_NO_ARMS;
					currentCreature->flags = 1;
				}
			}
			break;

		case STATE_BADDY_ROLL_LEFT:
		case STATE_BADDY_JUMP_RIGHT:
			currentCreature->alerted = false;
			currentCreature->maximumTurn = someFlag3;
			item->status = ITEM_ACTIVE;
			break;

		case STATE_BADDY_CROUCH:
			if (item->itemFlags[0] == someFlag3)
			{
				if (currentCreature->enemy)
				{
					if ((currentCreature->enemy->objectNumber == ID_SMALLMEDI_ITEM
						|| currentCreature->enemy->objectNumber == ID_UZI_AMMO_ITEM) 
						&& info.distance < 0x40000)
					{
						item->goalAnimState = STATE_BADDY_CROUCH_PICKUP;
						break;
					}
				}
				if (currentCreature->alerted)
				{
					item->goalAnimState = STATE_BADDY_CROUCH_TO_STAND;
				}
			}
			else
			{
				if (info.distance >= SQUARE(682))
				{
					break;
				}
				item->goalAnimState = STATE_BADDY_CROUCH_TO_STAND;
				currentCreature->enemy = NULL;
			}
			break;

		case STATE_BADDY_CROUCH_PICKUP:
			ClampRotation(&item->pos, info.angle, ANGLE(11));
			if (item->frameNumber != g_Level.Anims[item->animNumber].frameBase + FRAME_BADDY_CROUCH_PICKUP)
			{
				break;
			}
			if (!currentCreature->enemy)
			{
				break;
			}
			if (currentCreature->enemy->objectNumber != ID_SMALLMEDI_ITEM &&
				currentCreature->enemy->objectNumber != ID_UZI_AMMO_ITEM)
			{
				break;
			}
			if (currentCreature->enemy->roomNumber == NO_ROOM ||
				currentCreature->enemy->status == ITEM_INVISIBLE ||
				currentCreature->enemy->inDrawRoom)
			{
				currentCreature->enemy = NULL;
				break;
			}
			if (currentCreature->enemy->objectNumber == ID_SMALLMEDI_ITEM)
			{
				item->hitPoints += Objects[item->objectNumber].hitPoints / 2;
			}
			else
			{
				if (currentCreature->enemy->objectNumber != ID_UZI_AMMO_ITEM)
				{
					currentCreature->enemy = NULL;
					break;
				}
				item->itemFlags[2] += 24;
			}
			//KillItem(currentCreature->enemy->);

			// Search for the next enemy
			/*v82 = creature2;
			v113 = BaddieSlots + 18;
			v114 = 5;
			do
			{
				v115 = *(_WORD *)(v113 + 5628);
				if (v115 != -1 && v115 != (_WORD)itemNum && *(ITEM_INFO_OK **)v113 == creature2->enemy)
				{
					*(_DWORD *)v113 = 0;
				}
				v113 += 5702;
				--v114;
			} while (v114);
			creature2->enemy = 0;*/
			break;

		case STATE_BADDY_AIM:
			currentCreature->maximumTurn = 0;
			if (info.ahead)
			{
				joint1 = info.angle;
				joint2 = info.xAngle;
			}
			ClampRotation(&item->pos, info.angle, ANGLE(7));
			if (!Targetable(item, &info)
				|| item->itemFlags[2] < 1)
			{
				item->goalAnimState = STATE_BADDY_STOP;
				break;
			}
			item->goalAnimState = STATE_BADDY_FIRE;
			break;

		case STATE_BADDY_FIRE:
			if (info.ahead)
			{
				joint1 = info.angle;
				joint2 = info.xAngle;
			}
			ClampRotation(&item->pos, info.angle, ANGLE(7));
			if (item->frameNumber >= g_Level.Anims[item->animNumber].frameBase + FRAME_BADDY_FIRE_MAX ||
				item->frameNumber == g_Level.Anims[item->animNumber].frameBase + FRAME_BADDY_FIRE_MIN)
			{
				break;
			}
			item->firedWeapon = true;
			if (!item->hitStatus)
			{
				item->itemFlags[2]--;
			}
			if (!ShotLara(item, &info, &baddyGun, joint1, 15));
			item->goalAnimState = STATE_BADDY_STOP;
			break;

		default:
			break;

		case STATE_BADDY_HOLSTER_GUN:
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + FRAME_BADDY_HOLSTER_GUN)
			{
				item->swapMeshFlags = SWAPMESHFLAGS_BADDY_EMPTY;
			}
			break;

		case STATE_BADDY_DRAW_GUN:
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + FRAME_BADDY_DRAW_GUN)
			{
				item->swapMeshFlags = SWAPMESHFLAGS_BADDY_GUN;
			}
			break;

		case STATE_BADDY_HOLSTER_SWORD:
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + FRAME_BADDY_HOLSTER_SWORD)
			{
				item->swapMeshFlags = SWAPMESHFLAGS_BADDY_EMPTY;
			}
			break;

		case STATE_BADDY_DRAW_SWORD:
			if (item->frameNumber != g_Level.Anims[item->animNumber].frameBase + FRAME_BADDY_DRAW_SWORD)
			{
				break;
			}
			if (item->objectNumber == ID_BADDY1)
			{
				item->swapMeshFlags = SWAPMESHFLAGS_BADDY_SWORD_SIMPLE;
			}
			else
			{
				item->swapMeshFlags = SWAPMESHFLAGS_BADDY_SWORD_NINJA;
			}
			break;

		case STATE_BADDY_UNKNOWN_8:
			currentCreature->maximumTurn = 0;
			ClampRotation(&item->pos, info.angle, ANGLE(11));
			if (laraInfo.distance < SQUARE(682) ||
				item != Lara.target)
			{
				item->goalAnimState = STATE_BADDY_UNKNOWN_9;
			}
			break;

		case STATE_BADDY_BLIND:
			if (!WeaponEnemyTimer)
			{
				if ((GetRandomControl() & 0x7F) == 0)
				{
					item->goalAnimState = STATE_BADDY_STOP;
				}
			}
			break;

		case STATE_BADDY_SOMERSAULT:
			if (item->animNumber == Objects[objectNumber].animIndex + ANIMATION_BADDY_SOMERSAULT_END)
			{
				ClampRotation(&item->pos, info.angle, ANGLE(7));
				break;
			}
			if (item->frameNumber != g_Level.Anims[item->animNumber].frameBase + FRAME_BADDY_SOMERSAULT_START_TAKE_OFF)
			{
				break;
			}
			currentCreature->LOT.isJumping = true;
			break;

		case STATE_BADDY_JUMP_FORWARD_1_BLOCK:
		case STATE_BADDY_JUMP_FORWARD_2_BLOCKS:
			if (item->itemFlags[0] >= someFlag3)
			{
				break;
			}
			if (item->animNumber != Objects[objectNumber].animIndex + ANIMATION_BADDY_STAND_TO_JUMP_FORWARD)
			{
				item->itemFlags[0] += 2;
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint1);
	CreatureJoint(item, 1, joint2);
	CreatureJoint(item, 2, joint3);

	if (item->currentAnimState >= STATE_BADDY_JUMP_FORWARD_2_BLOCKS ||
		item->currentAnimState == STATE_BADDY_JUMP_FORWARD_1_BLOCK ||
		item->currentAnimState == STATE_BADDY_MONKEY_FORWARD ||
		item->currentAnimState == STATE_BADDY_DEATH ||
		item->currentAnimState == STATE_BADDY_SOMERSAULT ||
		item->currentAnimState == STATE_BADDY_BLIND)
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
			item->animNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_CLIMB_2_CLICKS;
			item->currentAnimState = STATE_BADDY_CLIMB_2_CLICKS;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;

		case 3:
			creature->maximumTurn = 0;
			item->animNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_CLIMB_3_CLICKS;
			item->currentAnimState = STATE_BADDY_CLIMB_3_CLICKS;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;

		case 4:
			creature->maximumTurn = 0;
			item->animNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_CLIMB_4_CLICKS;
			item->currentAnimState = STATE_BADDY_CLIMB_4_CLICKS;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;

		case -3:
			creature->maximumTurn = 0;
			item->animNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_JUMP_OFF_3_CLICKS;
			item->currentAnimState = STATE_BADDY_JUMP_OFF_3_CLICKS;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;

		case -4:
			creature->maximumTurn = 0;
			item->animNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_JUMP_OFF_4_CLICKS;
			item->currentAnimState = STATE_BADDY_JUMP_OFF_4_CLICKS;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;

		default:
			return;
		}
	}
	else
	{
		creature->maximumTurn = 0;
		item->animNumber = Objects[objectNumber].animIndex + ANIMATION_BADDY_BLIND;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase + (GetRandomControl() & 7);
		item->currentAnimState = STATE_BADDY_BLIND;
	}

	return;
}