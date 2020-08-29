#include "framework.h"
#include "tr4_guide.h"
#include "items.h"
#include "box.h"
#include "effect2.h"
#include "sphere.h"
#include "lot.h"
#include "effect.h"
#include "tomb4fx.h"
#include "setup.h"
#include "level.h"
#include "lara.h"
#include "sound.h"
#include <Game\draw.h>

#define STATE_VON_CROY_STOP						1
#define STATE_VON_CROY_WALK						2
#define STATE_VON_CROY_RUN						3
#define STATE_VON_CROY_START_MONKEY				4
#define STATE_VON_CROY_MONKEY					5
#define STATE_VON_CROY_TOGGLE_KNIFE				6
#define STATE_VON_CROY_LOOK_BEFORE_JUMP			7
#define STATE_VON_CROY_CALL_LARA1				13
#define STATE_VON_CROY_CALL_LARA2				14
#define STATE_VON_CROY_JUMP						15
#define STATE_VON_CROY_JUMP_2BLOCKS				16
#define STATE_VON_CROY_ENABLE_TRAP				20
#define STATE_VON_CROY_KNIFE_ATTACK_HIGH		21
#define STATE_VON_CROY_LOOK_BACK_LEFT			22
#define STATE_VON_CROY_STEP_DOWN_HIGH			26
#define STATE_VON_CROY_GRAB_LADDER				27
#define STATE_VON_CROY_CLIMB_LADDER_RIGHT		28
#define STATE_VON_CROY_LADDER_CLIMB_UP			30
#define STATE_VON_CROY_KNIFE_ATTACK_LOW			31
#define STATE_VON_CROY_JUMP_BACK				34
#define STATE_VON_CROY_LOOK_BACK_RIGHT			35

#define ANIMATION_VON_CROY_STEP_DOWN_HIGH		36
#define ANIMATION_VON_CROY_CLIMB_UP_AFTER_JUMP	52

#define SWAP_MESH_BITS_VON_CROY					0x40080

#define VON_CROY_FLAG_JUMP						6

bool VonCroyPassedWaypoints[128];
BITE_INFO VonCroyBite = { 0, 35, 130, 18 };

void InitialiseVonCroy(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	
	ClearItem(itemNumber);
	
	item->animNumber = Objects[item->objectNumber].animIndex + 11;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->goalAnimState = STATE_VON_CROY_TOGGLE_KNIFE;
	item->currentAnimState = STATE_VON_CROY_TOGGLE_KNIFE;
	item->swapMeshFlags = SWAP_MESH_BITS_VON_CROY;

	memset(VonCroyPassedWaypoints, 0, 128);
}

void VonCroyControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!CreatureActive(itemNumber))
		return;

	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	short angle = 0;
	short joint3 = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;
	short tilt = 0;

	// check if Von Croy can jump 1 or 2 blocks

	int x = item->pos.xPos;
	int z = item->pos.zPos;

	int dx = 808 * phd_sin(item->pos.yRot) >> W2V_SHIFT;
	int dz = 808 * phd_cos(item->pos.yRot) >> W2V_SHIFT;

	x += dx;
	z += dz;

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, item->pos.yPos, z);

	x += dx;
	z += dz;

	roomNumber = item->roomNumber;
	floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, item->pos.yPos, z);

	x += dx;
	z += dz;

	roomNumber = item->roomNumber;
	floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
	int height3 = GetFloorHeight(floor, x, item->pos.yPos, z);

	bool canJump1block;
	if (item->boxNumber == LaraItem->boxNumber
		|| item->pos.yPos >= height1 - 384
		|| item->pos.yPos >= height2 + 256
		|| item->pos.yPos <= height2 - 256)
		canJump1block = false;
	else
		canJump1block = true;

	bool canJump2blocks;
	if (item->boxNumber == LaraItem->boxNumber
		|| item->pos.yPos >= height1 - 384
		|| item->pos.yPos >= height2 - 384
		|| item->pos.yPos >= height3 + 256
		|| item->pos.yPos <= height3 - 256)
		canJump2blocks = false;
	else
		canJump2blocks = true;

	// Von Croy must follow Lara and navigate with ID_AI_FOLLOW objects
	item->aiBits = FOLLOW;
	GetAITarget(creature);

	AI_INFO info;
	AI_INFO laraInfo;

	// Try to find a possible enemy or target
	ITEM_INFO* foundTarget = NULL;

	if (Lara.location <= item->location)
	{
		int minDistance = 0x7FFFFFFF;
		int distance;
		CREATURE_INFO* baddie = &BaddieSlots[0];

		for (int i = 0; i < NUM_SLOTS; i++)
		{
			baddie = &BaddieSlots[i];

			if (baddie->itemNum == NO_ITEM
				|| baddie->itemNum == itemNumber
				|| g_Level.Items[baddie->itemNum].objectNumber == ID_VON_CROY
				|| g_Level.Items[baddie->itemNum].objectNumber == ID_GUIDE)
				continue;

			ITEM_INFO* currentItem = &g_Level.Items[baddie->itemNum];
			if (abs(currentItem->pos.yPos - item->pos.yPos) <= 512)
			{
				dx = currentItem->pos.xPos - item->pos.xPos;
				dz = currentItem->pos.zPos - item->pos.zPos;

				if (abs(dx) < 5120 && abs(dz) < 5120)
				{
					distance = SQUARE(dx) + SQUARE(dz);
					if (distance < minDistance)
					{
						creature->reachedGoal = false;
						foundTarget = currentItem;
						minDistance = distance;
					}
				}
			}
		}
	}

	// If a target is found, that it becomes the enemy
	ITEM_INFO* enemy = creature->enemy;
	if (foundTarget != 0)
	{
		creature->enemy = foundTarget;
	}

	// HACK: even the most advanced zone in TR must have a step height of 1024, so we need to recreate zones when step difference is higher
	if (item->animNumber == Objects[item->objectNumber].animIndex + ANIMATION_VON_CROY_STEP_DOWN_HIGH
		|| item->animNumber == Objects[item->objectNumber].animIndex + ANIMATION_VON_CROY_CLIMB_UP_AFTER_JUMP)
	{
		short oldRoom = item->roomNumber;
		item->pos.xPos += dx;
		item->pos.zPos += dz;

		GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &item->roomNumber);

		if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 1)
		{
			CreateZone(item);
		}
		CreatureAIInfo(item, &info);

		item->roomNumber = oldRoom;
		item->pos.xPos -= dx;
		item->pos.zPos -= dz;
	}
	else
	{
		CreatureAIInfo(item, &info);
	}

	GetCreatureMood(item, &info, VIOLENT);
	CreatureMood(item, &info, VIOLENT);

	if (creature->enemy == LaraItem)
	{
		memcpy(&laraInfo, &info, sizeof(AI_INFO));
	}
	else
	{
		dx = LaraItem->pos.xPos - item->pos.xPos;
		dz = LaraItem->pos.zPos - item->pos.zPos;
		laraInfo.angle = phd_atan(dz, dx) - item->pos.yRot;

		laraInfo.ahead = true;
		if (laraInfo.angle <= -ANGLE(90) || laraInfo.angle >= ANGLE(90))
			laraInfo.ahead = false;

		laraInfo.enemyFacing = laraInfo.angle - LaraItem->pos.xPos + -ANGLE(180);
		int distance = 0;
		if (dz > 32000 || dz < -32000 || dx > 32000 || dx < -32000)
			laraInfo.distance = 0x7FFFFFFF;
		else
			laraInfo.distance = dx * dx + dz * dz;

		dx = abs(dx);
		dz = abs(dz);

		int dy = item->pos.yPos - LaraItem->pos.yPos;
		short rot2 = 0;

		if (dx <= dz)
			laraInfo.xAngle = phd_atan(dz + (dx >> 1), dy);
		else
			laraInfo.xAngle = phd_atan(dx + (dz >> 1), dy);
	}

	if (abs(laraInfo.angle) < 6144 && laraInfo.distance < SQUARE(1024))
		laraInfo.bite = true;
	else
		laraInfo.bite = false;

	angle = CreatureTurn(item, creature->maximumTurn);

	if (foundTarget != NULL)
	{
		creature->enemy = enemy;
		enemy = foundTarget;
	}

	// NOTE: I've removed here a bunch of if (Lara.location == X) 
	// made for making Von Croy wait for Lara in tutorial area

	/*if (!VonCroyPassedWaypoints[item->location] &&
		(((creature->reachedGoal 
			&& item->location == Lara.locationPad)
			|| item->triggerFlags > 0)
			|| (VonCroyPassedWaypoints[item->location] <= Lara.locationPad
				&& !VonCroyPassedWaypoints[Lara.locationPad])))
	{
		CreatureJoint(item, 0, laraInfo.angle >> 1);
		CreatureJoint(item, 1, laraInfo.angle >> 1);
		CreatureJoint(item, 2, laraInfo.angle >> 1);
		CreatureJoint(item, 3, laraInfo.angle >> 1);
		VonCroyAnimation(item, creature);
		return;
	}*/

	short rot = 0;
	int dy, height, ceiling, flags;
	
	switch (item->currentAnimState)
	{
	case STATE_VON_CROY_STOP:
		creature->LOT.isJumping = false;
		joint2 = laraInfo.angle;
		creature->flags = 0;
		creature->maximumTurn = 0;

		joint3 = 0;
		joint2 = info.angle >> 1;
		if (info.ahead) 
		{
			joint1 = info.xAngle >> 1;
			joint0 = info.angle >> 1;
		}

		if (item->requiredAnimState) 
		{
			item->goalAnimState = item->requiredAnimState;
			break;
		}

		/*if (item->itemFlags[2] == 2)
		{
			rot = enemy->pos.yRot - item->pos.yRot;
			if (rot < -1024)
			{
				item->goalAnimState = STATE_VON_CROY_LOOK_BACK_RIGHT;
				break;
			}
			else if (rot > 1024)
			{
				item->goalAnimState = STATE_VON_CROY_LOOK_BACK_LEFT;
				break;
			}
			else
			{
				item->itemFlags[2] = 0;
				if (!item->flags)
				{
					creature->reachedGoal = false;
					creature->enemy = NULL;
					item->aiBits = FOLLOW;
					item->location++;
					break;
				}
			}
		}*/

		// Wait for Lara 
		if (Lara.location < item->location && creature->reachedGoal) 
		{
			item->goalAnimState = STATE_VON_CROY_STOP;
			break;
		}

		if (foundTarget != 0) 
		{
			if (info.distance < SQUARE(3072) && (item->swapMeshFlags & SWAP_MESH_BITS_VON_CROY)) 
			{
				item->goalAnimState = STATE_VON_CROY_TOGGLE_KNIFE;
				break;
			}

			if (info.distance >= SQUARE(1024))
			{
				if (enemy != LaraItem && info.distance > SQUARE(640)) 
				{
					item->goalAnimState = STATE_VON_CROY_WALK;
					break;
				}
			}
			else if (!info.bite)
			{
				if (enemy->hitPoints > 0 && info.ahead)
				{
					if (abs(enemy->pos.yPos - item->pos.yPos + 512) < 512)
						item->goalAnimState = STATE_VON_CROY_KNIFE_ATTACK_HIGH;
					else
						item->goalAnimState = STATE_VON_CROY_KNIFE_ATTACK_LOW;
				}
			}

			break;
		}

		if (!creature->reachedGoal) 
		{
			if (laraInfo.bite) 
			{
				item->goalAnimState = STATE_VON_CROY_STOP;
				break;
			}

			if (canJump1block || canJump2blocks)
			{
				creature->maximumTurn = 0;
				item->animNumber = obj->animIndex + 22;
				item->currentAnimState = STATE_VON_CROY_JUMP;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				if (canJump2blocks)
					item->goalAnimState = STATE_VON_CROY_JUMP_2BLOCKS;
				creature->LOT.isJumping = true;
				break;
			}
			else if (creature->monkeyAhead) 
			{
				floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
				height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
				if (GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) == height - 1536)
				{
					/*if (!(item->swapMeshFlags & SWAP_MESH_BITS_VON_CROY))
					{
						item->goalAnimState = STATE_VON_CROY_TOGGLE_KNIFE;
						break;
					}
					else
					{*/
						item->goalAnimState = STATE_VON_CROY_START_MONKEY;
						break;
					//}
				}
			}
			else 
			{
				if (foundTarget != 0) 
				{
					if (info.distance < SQUARE(3072) 
						&& (item->swapMeshFlags & SWAP_MESH_BITS_VON_CROY)) 
					{
						item->goalAnimState = STATE_VON_CROY_TOGGLE_KNIFE;
						break;
					}
					if (info.distance < SQUARE(1024))
					{
						if (!info.bite)
						{
							if (enemy->hitPoints > 0 && info.ahead)
							{
								if (abs(enemy->pos.yPos - item->pos.yPos + 512) < 512)
									item->goalAnimState = STATE_VON_CROY_KNIFE_ATTACK_HIGH;
							}
						}
						else
						{
							item->goalAnimState = STATE_VON_CROY_KNIFE_ATTACK_LOW;
						}
						break;
					}
				}
				if ((info.distance < SQUARE(640)
					|| laraInfo.distance > SQUARE(5120)) 
					&& Lara.location < item->location)
					break;
			}

			item->goalAnimState = STATE_VON_CROY_WALK;
			break;
		}

		if (Lara.location <= item->location) 
		{
			if (enemy && enemy->flags) 
			{
				/*if (Lara.locationPad == item->location)
					goto LAB_00419db3;*/
				if (!enemy->flags && laraInfo.distance > 3072) 
				{
					item->goalAnimState = STATE_VON_CROY_STOP;
					break;
				}
			}

			/*if (item->itemFlags[2] == 0)
			{
				if (laraInfo.angle < 1024) 
				{
					if (laraInfo.angle < -1024) 
					{
						item->goalAnimState = STATE_VON_CROY_LOOK_BACK_LEFT;
					}
					else 
					{
						item->itemFlags[2] = 1;
					}
				}
				else 
				{
					item->goalAnimState = STATE_VON_CROY_LOOK_BACK_RIGHT;
				}
				break;
			}*/

			/*if (item->itemFlags[2] == 1)
			{
				if (!(GetRandomControl() & 0xF)) 
				{
					if (laraInfo.distance < SQUARE(3072)) 
						item->goalAnimState = STATE_VON_CROY_CALL_LARA2;
					else 
						item->goalAnimState = STATE_VON_CROY_CALL_LARA1;
				}
				else 
				{
					item->itemFlags[2] = 0;
				}
				break;
			}*/
			
			creature->reachedGoal = false;
			creature->enemy = NULL;
			item->aiBits = FOLLOW;
			item->location++;
			break;
		}

		/*if (enemy->flags > 32)
		{
			if (enemy->flags == 34)
			{
				if (Lara.location <= item->location)
				{
					item->goalAnimState = 32;
					break;
				}
				else
				{
					creature->reachedGoal = false;
					creature->enemy = NULL;
					item->aiBits = FOLLOW;
					item->location += 2;
					break;
				}
			}
			else if (enemy->flags == 36)
			{
				if (Lara.location <= item->location)
				{
					item->goalAnimState = STATE_VON_CROY_STOP;
				}
				break;
			}
			else if (enemy->flags == 40)
			{
				if (item->itemFlags[2] != VON_CROY_FLAG_JUMP)
				{
					item->goalAnimState = STATE_VON_CROY_JUMP_BACK;
					item->pos.xPos = enemy->pos.xPos;
					item->pos.yPos = enemy->pos.yPos;
					item->pos.zPos = enemy->pos.zPos;
					item->pos.xRot = enemy->pos.xRot;
					item->pos.yRot = enemy->pos.yRot;
					item->pos.zRot = enemy->pos.zRot;
					break;
				}
				else
				{
					item->goalAnimState = STATE_VON_CROY_RUN;
					break;
				}
			}
			else if (enemy->flags == 48)
			{
				GetFloorAndTestTriggers(
					creature->aiTarget.pos.xPos,
					creature->aiTarget.pos.yPos,
					creature->aiTarget.pos.zPos,
					creature->aiTarget.roomNumber,
					1, 0);
				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->aiBits = FOLLOW;
				item->location++;
				break;
			}
			else if (enemy->flags == -1)
			{
				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->aiBits = FOLLOW;
				item->location++;
				break;
			}
			else
			{
				break;
			}
		}

		if (enemy->flags == 2) 
		{
			item->currentAnimState = STATE_VON_CROY_GRAB_LADDER;
			item->animNumber = Objects[item->objectNumber].animIndex + 37;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->pos.xPos = enemy->pos.xPos;
			item->pos.yPos = enemy->pos.yPos;
			item->pos.zPos = enemy->pos.zPos;
			item->pos.xRot = enemy->pos.xRot;
			item->pos.yRot = enemy->pos.yRot;
			item->pos.zRot = enemy->pos.zRot;
		}
		else if (enemy->flags == 4)
		{
			item->currentAnimState = STATE_VON_CROY_STEP_DOWN_HIGH;
			item->animNumber = Objects[item->objectNumber].animIndex + 36;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

			item->pos.xPos = enemy->pos.xPos;
			item->pos.yPos = enemy->pos.yPos;
			item->pos.zPos = enemy->pos.zPos;
			item->pos.xRot = enemy->pos.xRot;
			item->pos.yRot = enemy->pos.yRot;
			item->pos.zRot = enemy->pos.zRot;

			creature->LOT.isJumping = true;
		}
		else if (enemy->flags == 8)
		{
			item->goalAnimState = STATE_VON_CROY_ENABLE_TRAP;
			break;
		}
		else if (enemy->flags == 10)
		{
			item->goalAnimState = STATE_VON_CROY_LOOK_BEFORE_JUMP;
			break;
		}
		else if (enemy->flags == 12)
		{
			creature->maximumTurn = 0;

			item->animNumber = Objects[item->objectNumber].animIndex + 22;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = STATE_VON_CROY_JUMP;
			if (canJump2blocks == 0)
			{
				item->goalAnimState = STATE_VON_CROY_JUMP;
			}
			else
			{
				item->goalAnimState = STATE_VON_CROY_JUMP_2BLOCKS;
			}

			item->pos.xPos = enemy->pos.xPos;
			item->pos.yPos = enemy->pos.yPos;
			item->pos.zPos = enemy->pos.zPos;
			item->pos.xRot = enemy->pos.xRot;
			item->pos.yRot = enemy->pos.yRot;
			item->pos.zRot = enemy->pos.zRot;

			creature->LOT.isJumping = true;
		}
		else if (enemy->flags == 1 || enemy->flags == 3
			|| enemy->flags == 5 || enemy->flags == 6
			|| enemy->flags == 7 || enemy->flags == 9
			|| enemy->flags == 11)
		{
			break;
		}
		else
		{
			GetFloorAndTestTriggers(
				creature->aiTarget.pos.xPos,
				creature->aiTarget.pos.yPos,
				creature->aiTarget.pos.zPos,
				creature->aiTarget.roomNumber,
				1, 0);
		}*/

		creature->reachedGoal = false;
		creature->enemy = NULL;
		item->aiBits = FOLLOW;
		item->location++;
		break;

	case STATE_VON_CROY_WALK:
		creature->LOT.isJumping = false;
		creature->LOT.isMonkeying = false;
		creature->maximumTurn = ANGLE(6);
		
		if (!laraInfo.ahead)
		{
			if (info.ahead)
			{
				joint2 = info.angle;
			}
		}
		else 
		{
			joint2 = laraInfo.angle;
		}

		if (item->requiredAnimState != 0)
		{
			item->goalAnimState = item->requiredAnimState;
			break;
		}

		if ((Lara.location < item->location
			&& laraInfo.distance >SQUARE(5120)) 
			|| laraInfo.bite)
		{
			item->goalAnimState = STATE_VON_CROY_STOP;
			break;
		}

		if (creature->monkeyAhead) 
		{
			item->goalAnimState = STATE_VON_CROY_STOP;
			break;
		}

		if (creature->reachedGoal)
		{
			if (enemy->flags != 32) 
			{
				item->goalAnimState = STATE_VON_CROY_STOP;
				break;
			}
			else
			{
				GetFloorAndTestTriggers(
					creature->aiTarget.pos.xPos,
					creature->aiTarget.pos.yPos,
					creature->aiTarget.pos.zPos,
					creature->aiTarget.roomNumber,
					1, 0);
				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->aiBits = FOLLOW;
				item->location++;
				break;
			}
		}

		if (foundTarget != NULL 
			&& (info.distance < SQUARE(1448) 
				|| ((item->swapMeshFlags & SWAP_MESH_BITS_VON_CROY) == 0 
					&& (info.distance < SQUARE(3072)))))
		{
			item->goalAnimState = STATE_VON_CROY_STOP;
			break;
		}

		if (info.distance < SQUARE(640) 
			&& enemy->flags != 32) 
		{
			item->goalAnimState = STATE_VON_CROY_STOP;
			break;
		}

		if (info.distance < SQUARE(3072) 
			|| Lara.location < item->location) 
			break;

		item->goalAnimState = STATE_VON_CROY_RUN;

		break;

	case STATE_VON_CROY_RUN:
		if (info.ahead) 
		{
			joint2 = info.angle;
		}

		if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
		{
			creature->LOT.isJumping = false;
			creature->maximumTurn = ANGLE(8);
		}

		tilt = angle >> 1;

		if (item->itemFlags[2] == VON_CROY_FLAG_JUMP)
		{
			creature->maximumTurn = 0;
			item->goalAnimState = STATE_VON_CROY_JUMP_2BLOCKS;
			break;
		}

		if (item->location <= Lara.location 
			&& !canJump1block == 0 
			&& !laraInfo.bite) 
		{
			if (creature->monkeyAhead) 
			{
				item->goalAnimState = STATE_VON_CROY_STOP;
				break;
			}

			if (!creature->reachedGoal) 
			{
				if (info.distance < SQUARE(640) 
					&& enemy->flags != 32
					&& enemy->flags != 40) 
				{
					item->goalAnimState = STATE_VON_CROY_STOP;
				}
				break;
			}

			if (enemy->flags == 32)
			{
				GetFloorAndTestTriggers(
					creature->aiTarget.pos.xPos,
					creature->aiTarget.pos.yPos,
					creature->aiTarget.pos.zPos,
					creature->aiTarget.roomNumber,
					1, 0);
				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->aiBits = FOLLOW;
				item->location++;
				break;
			}

			if (info.distance < 512) 
			{
				if (enemy->flags == 40) 
				{
					creature->maximumTurn = 0;
					item->pos.yRot = enemy->pos.yRot;
					item->goalAnimState = STATE_VON_CROY_JUMP_2BLOCKS;
					item->itemFlags[2] = VON_CROY_FLAG_JUMP;
				}
				break;
			}
		}
		
		item->goalAnimState = STATE_VON_CROY_STOP;
		break;

	case STATE_VON_CROY_START_MONKEY:
		creature->maximumTurn = 0;
		if (item->boxNumber != creature->LOT.targetBox && creature->monkeyAhead) 
		{
			item->goalAnimState = STATE_VON_CROY_MONKEY;
		}
		else
		{
			floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
			height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
			ceiling = GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
			if (ceiling == height - 1536)
				item->goalAnimState = STATE_VON_CROY_STOP;
		}

		break;

	case STATE_VON_CROY_MONKEY:
		creature->LOT.isMonkeying = true;
		creature->LOT.isJumping = true;
		creature->maximumTurn = ANGLE(6);

		if (item->boxNumber == creature->LOT.targetBox || !creature->monkeyAhead)
		{
			floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
			height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
			if (GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) == height - 1536)
				item->goalAnimState = STATE_VON_CROY_START_MONKEY;
		}

		break;

	case STATE_VON_CROY_TOGGLE_KNIFE:
		if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase) 
		{
			if (!(item->swapMeshFlags & SWAP_MESH_BITS_VON_CROY))
			{
				item->swapMeshFlags |= SWAP_MESH_BITS_VON_CROY;
			}
			else 
			{
				item->swapMeshFlags &= ~SWAP_MESH_BITS_VON_CROY;
			}
		}
		break;

	case STATE_VON_CROY_LOOK_BEFORE_JUMP:
		if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
		{
			item->pos.xPos = enemy->pos.xPos;
			item->pos.yPos = enemy->pos.yPos;
			item->pos.zPos = enemy->pos.zPos;
			item->pos.xRot = enemy->pos.xRot;
			item->pos.yRot = enemy->pos.yRot;
			item->pos.zRot = enemy->pos.zRot;

			if (item->itemFlags[2] == VON_CROY_FLAG_JUMP)
			{
				creature->maximumTurn = 0;
				item->currentAnimState = STATE_VON_CROY_JUMP;
				item->goalAnimState = STATE_VON_CROY_JUMP_2BLOCKS;
				item->animNumber = Objects[item->objectNumber].animIndex + 22;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				creature->LOT.isJumping = true;
			}

			creature->reachedGoal = false;
			creature->enemy = NULL;
			item->aiBits = FOLLOW;
			item->location++;
		}

		break;

	case STATE_VON_CROY_JUMP_2BLOCKS:
		if (item->animNumber == Objects[item->objectNumber].animIndex + 25
			|| item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 5)
		{
			creature->LOT.isJumping = true;
		}
		else if (canJump1block) 
		{
				item->goalAnimState = STATE_VON_CROY_JUMP;
		}

		if (item->itemFlags[2] == VON_CROY_FLAG_JUMP)
		{
			item->goalAnimState = 33;
		}

		break;

	case STATE_VON_CROY_ENABLE_TRAP:
		if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
		{
			item->pos.xPos = enemy->pos.xPos;
			item->pos.yPos = enemy->pos.yPos;
			item->pos.zPos = enemy->pos.zPos;
			item->pos.xRot = enemy->pos.xRot;
			item->pos.yRot = enemy->pos.yRot;
			item->pos.zRot = enemy->pos.zRot;
		}
		else if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 120)
		{
			GetFloorAndTestTriggers(
				creature->aiTarget.pos.xPos,
				creature->aiTarget.pos.yPos,
				creature->aiTarget.pos.zPos,
				creature->aiTarget.roomNumber,
				1, 0);

			creature->reachedGoal = false;
			creature->enemy = NULL;
			item->aiBits = FOLLOW;
			item->location++;
		}

		break;

	case STATE_VON_CROY_KNIFE_ATTACK_HIGH:
		if (info.ahead) 
		{
			joint2 = info.angle >> 1;
			joint1 = info.xAngle >> 1;
			joint0 = joint2;
		}

		creature->maximumTurn = 0;
		ClampRotation(&item->pos, info.angle, ANGLE(6));

		if (!creature->flags && enemy != NULL) 
		{
			if (item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 20 
				&& item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 45) 
			{
				if (abs(item->pos.xPos - enemy->pos.xPos) < 512
					&& abs(item->pos.yPos - enemy->pos.yPos) < 512
					&& abs(item->pos.zPos - enemy->pos.zPos) < 512)
				{
					enemy->hitPoints -= 40;
					if (enemy->hitPoints <= 0)
					{
						item->aiBits = FOLLOW;
					}
					enemy->hitStatus = true;
					creature->flags = 1;
					CreatureEffect2(item, &VonCroyBite, 2, -1, DoBloodSplat);
				}
			}
		}

		break;

	case STATE_VON_CROY_LOOK_BACK_LEFT:
	case STATE_VON_CROY_LOOK_BACK_RIGHT:
		creature->maximumTurn = 0;
		if (item->itemFlags[2] == 0)
		{
			ClampRotation(&item->pos, laraInfo.angle, 512);
		}
		else
		{
			ClampRotation(&item->pos, enemy->pos.yRot - item->pos.yRot, 512);
		}
		break;

	case STATE_VON_CROY_GRAB_LADDER:
		creature->LOT.isJumping = true;
		creature->maximumTurn = 0;

		if (!creature->reachedGoal) 
		{
			item->goalAnimState = STATE_VON_CROY_CLIMB_LADDER_RIGHT;
		}
		else
		{
			item->goalAnimState = STATE_VON_CROY_LADDER_CLIMB_UP;
			creature->reachedGoal = false;
			creature->enemy = NULL;
			item->aiBits = FOLLOW;
			item->location++;
		}
		break;

	case STATE_VON_CROY_CLIMB_LADDER_RIGHT:
		creature->LOT.isJumping = true;
		creature->maximumTurn = 0;
		break;

	case STATE_VON_CROY_KNIFE_ATTACK_LOW:
		if (info.ahead)
		{
			joint2 = info.angle >> 1;
			joint1 = info.xAngle >> 1;
			joint0 = joint2;
		}

		creature->maximumTurn = 0;
		ClampRotation(&item->pos, info.angle, ANGLE(6));

		if ((enemy == NULL
			|| enemy->flags != 0) 
			|| item->frameNumber <= g_Level.Anims[item->animNumber].frameBase + 21)
		{
			if (creature->flags == 0 && enemy != NULL) 
			{
				if (item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 15 
					&& item->frameNumber < g_Level.Anims[item->animNumber].frameBase + 26)
				{
					if (abs(item->pos.xPos - enemy->pos.xPos) < 512
						&& abs(item->pos.yPos - enemy->pos.yPos) < 512
						&& abs(item->pos.zPos - enemy->pos.zPos) < 512)
					{
						enemy->hitPoints -= 20;
						if (enemy->hitPoints <= 0)
						{
							item->aiBits = FOLLOW;
						}
						enemy->hitStatus = true;
						creature->flags = 1;
						CreatureEffect2(item, &VonCroyBite, 2, -1, DoBloodSplat);
					}
				}
			}
			break;
		}

		GetFloorAndTestTriggers(
			creature->aiTarget.pos.xPos,
			creature->aiTarget.pos.yPos,
			creature->aiTarget.pos.zPos,
			creature->aiTarget.roomNumber,
			1, 0);

		creature->reachedGoal = false;
		creature->enemy = NULL;
		item->aiBits = FOLLOW;
		item->location++;

		break;

	case 32:
		if (info.ahead) 
		{
			joint2 = info.angle >> 1;
			joint1 = info.xAngle;
			joint0 = joint2;
		}
		
		creature->maximumTurn = 0;
		ClampRotation(&item->pos, info.angle >> 1, ANGLE(6));

		if (item->animNumber == Objects[item->objectNumber].animIndex + 47) 
		{
			if (item->frameNumber != g_Level.Anims[item->animNumber].frameBase) 
				break;
		}
		else 
		{
			if (GetRandomControl() & 0x1F)
				break;
			item->goalAnimState = STATE_VON_CROY_STOP;
		}

		creature->reachedGoal = false;
		creature->enemy = NULL;
		item->aiBits = FOLLOW;
		item->location++;

		break;

	case 33:
		flags = 1;
		if (item->animNumber != Objects[item->objectNumber].animIndex + 52
			|| item->frameNumber != g_Level.Anims[item->animNumber].frameBase) 
		{
			flags = 0;
		}
		
		item->goalAnimState = STATE_VON_CROY_WALK;
		item->requiredAnimState = STATE_VON_CROY_RUN;
		item->itemFlags[2] = 0;
		//if (sVar3 == -1) goto LAB_0041a991;
		if (!flags)
		{
			creature->reachedGoal = false;
			creature->enemy = NULL;
			item->aiBits = FOLLOW;
			item->location++;
		}

		break;
	
	case STATE_VON_CROY_JUMP_BACK:
		item->itemFlags[2] = 6;
		break;

	case 36:
	case 37:
		creature->maximumTurn = 0;
		MoveCreature3DPos(&item->pos, &enemy->pos, 15, enemy->pos.yRot-item->pos.yRot, 512);
		break;

	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureJoint(item, 3, joint3);
	
	if ((item->currentAnimState < 15) && (item->currentAnimState != 5)) 
	{
		switch (CreatureVault(itemNumber, angle, 2, 260)) 
		{
		case 2:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex+29;
			item->currentAnimState = 19;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;

		case 3:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 28;
			item->currentAnimState = 18;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;

		case 4:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 27;
			item->currentAnimState = 17;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;

		case -4:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 35;
			item->currentAnimState = 25;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;

		case -3:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 41;
			item->currentAnimState = 24;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;

		case -2:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 42;
			item->currentAnimState = 23;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			break;

		}
	}
	else
	{
		CreatureAnimation(itemNumber, angle, 0);
	}
}