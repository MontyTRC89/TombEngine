#include "framework.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Game/animation.h"
#include "Game/itemdata/creature_info.h"

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

#define SWAPMESHFLAGS_VON_CROY					0x40080

#define VON_CROY_FLAG_JUMP						6

bool VonCroyPassedWaypoints[128];
BITE_INFO VonCroyBite = { 0, 35, 130, 18 };

void InitialiseVonCroy(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	
	ClearItem(itemNumber);
	
	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 11;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = STATE_VON_CROY_TOGGLE_KNIFE;
	item->Animation.ActiveState = STATE_VON_CROY_TOGGLE_KNIFE;
	item->SwapMeshFlags = SWAPMESHFLAGS_VON_CROY;

	memset(VonCroyPassedWaypoints, 0, 128);
}

void VonCroyControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!CreatureActive(itemNumber))
		return;

	CreatureInfo* creature = (CreatureInfo*)item->Data;
	OBJECT_INFO* obj = &Objects[item->ObjectNumber];

	short angle = 0;
	short joint3 = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;
	short tilt = 0;

	// check if Von Croy can jump 1 or 2 blocks

	int x = item->Position.xPos;
	int z = item->Position.zPos;

	int dx = 808 * phd_sin(item->Position.yRot);
	int dz = 808 * phd_cos(item->Position.yRot);

	x += dx;
	z += dz;

	short roomNumber = item->RoomNumber;
	FLOOR_INFO* floor = GetFloor(x, item->Position.yPos, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, item->Position.yPos, z);

	x += dx;
	z += dz;

	roomNumber = item->RoomNumber;
	floor = GetFloor(x, item->Position.yPos, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, item->Position.yPos, z);

	x += dx;
	z += dz;

	roomNumber = item->RoomNumber;
	floor = GetFloor(x, item->Position.yPos, z, &roomNumber);
	int height3 = GetFloorHeight(floor, x, item->Position.yPos, z);

	x += dx ;
	z += dz ;

	roomNumber = item->RoomNumber;
	floor = GetFloor(x, item->Position.yPos, z, &roomNumber);
	int height4 = GetFloorHeight(floor, x, item->Position.yPos, z);

	bool canJump1block;
	if (item->BoxNumber == LaraItem->BoxNumber
		|| item->Position.yPos >= height1 - 384
		|| item->Position.yPos >= height2 + 256
		|| item->Position.yPos <= height2 - 256)
		canJump1block = false;
	else
		canJump1block = true;

	bool canJump2blocks;
	if (item->BoxNumber == LaraItem->BoxNumber
		|| item->Position.yPos >= height1 - 384
		|| item->Position.yPos >= height2 - 384
		|| item->Position.yPos >= height3 + 256
		|| item->Position.yPos <= height3 - 256)
		canJump2blocks = false;
	else
		canJump2blocks = true;

	bool canJump3blocks;
	if (item->BoxNumber == LaraItem->BoxNumber
		|| item->Position.yPos >= height1 - 384
		|| item->Position.yPos >= height2 - 384
		|| item->Position.yPos >= height3 - 384
		|| item->Position.yPos >= height4 + 256
		|| item->Position.yPos <= height4 - 256)
		canJump3blocks = false;
	else
		canJump3blocks = true;

	// Von Croy must follow Lara and navigate with ID_AI_FOLLOW objects
	item->AIBits = FOLLOW;
	GetAITarget(creature);

	AI_INFO info;
	AI_INFO laraInfo;

	// Try to find a possible enemy or target
	ITEM_INFO* foundTarget = NULL;

	if (Lara.Location <= creature->LocationAI)
	{
		int minDistance = 0x7FFFFFFF;
		int distance;
		CreatureInfo* targetCreature = ActiveCreatures[0];

		for (int i = 0; i < ActiveCreatures.size(); i++)
		{
			targetCreature = ActiveCreatures[i];

			if (targetCreature->ItemNumber == NO_ITEM
				|| targetCreature->ItemNumber == itemNumber
				|| g_Level.Items[targetCreature->ItemNumber].ObjectNumber == ID_VON_CROY
				|| g_Level.Items[targetCreature->ItemNumber].ObjectNumber == ID_GUIDE)
				continue;

			ITEM_INFO* currentItem = &g_Level.Items[targetCreature->ItemNumber];
			if (abs(currentItem->Position.yPos - item->Position.yPos) <= 512)
			{
				dx = currentItem->Position.xPos - item->Position.xPos;
				dz = currentItem->Position.zPos - item->Position.zPos;

				if (abs(dx) < 5120 && abs(dz) < 5120)
				{
					distance = SQUARE(dx) + SQUARE(dz);
					if (distance < minDistance)
					{
						creature->ReachedGoal = false;
						foundTarget = currentItem;
						minDistance = distance;
					}
				}
			}
		}
	}

	// If a target is found, that it becomes the enemy
	ITEM_INFO* enemy = creature->Enemy;
	if (foundTarget != 0)
	{
		creature->Enemy = foundTarget;
	}

	// HACK: even the most advanced zone in TR must have a step height of 1024, so we need to recreate zones when step difference is higher
	if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + ANIMATION_VON_CROY_STEP_DOWN_HIGH
		|| item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + ANIMATION_VON_CROY_CLIMB_UP_AFTER_JUMP)
	{
		short oldRoom = item->RoomNumber;
		item->Position.xPos += dx;
		item->Position.zPos += dz;

		GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &item->RoomNumber);

		if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 1)
		{
			CreateZone(item);
		}
		CreatureAIInfo(item, &info);

		item->RoomNumber = oldRoom;
		item->Position.xPos -= dx;
		item->Position.zPos -= dz;
	}
	else
	{
		CreatureAIInfo(item, &info);
	}

	GetCreatureMood(item, &info, VIOLENT);
	CreatureMood(item, &info, VIOLENT);

	if (creature->Enemy == LaraItem)
	{
		memcpy(&laraInfo, &info, sizeof(AI_INFO));
	}
	else
	{
		dx = LaraItem->Position.xPos - item->Position.xPos;
		dz = LaraItem->Position.zPos - item->Position.zPos;
		laraInfo.angle = phd_atan(dz, dx) - item->Position.yRot;

		laraInfo.ahead = true;
		if (laraInfo.angle <= -ANGLE(90) || laraInfo.angle >= ANGLE(90))
			laraInfo.ahead = false;

		laraInfo.enemyFacing = laraInfo.angle - LaraItem->Position.xPos + -ANGLE(180);
		int distance = 0;
		if (dz > 32000 || dz < -32000 || dx > 32000 || dx < -32000)
			laraInfo.distance = 0x7FFFFFFF;
		else
			laraInfo.distance = dx * dx + dz * dz;

		dx = abs(dx);
		dz = abs(dz);

		int dy = item->Position.yPos - LaraItem->Position.yPos;
		short rot2 = 0;

		if (dx <= dz)
			laraInfo.xAngle = phd_atan(dz + (dx / 2), dy);
		else
			laraInfo.xAngle = phd_atan(dx + (dz / 2), dy);
	}

	if (abs(laraInfo.angle) < 6144 && laraInfo.distance < SQUARE(1024))
		laraInfo.bite = true;
	else
		laraInfo.bite = false;

	angle = CreatureTurn(item, creature->MaxTurn);

	if (foundTarget != NULL)
	{
		creature->Enemy = enemy;
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

	TENLog("State:" + std::to_string(item->Animation.ActiveState), LogLevel::Info);
	
	switch (item->Animation.ActiveState)
	{
	case STATE_VON_CROY_STOP:
		creature->LOT.IsMonkeying = false;
		creature->LOT.IsJumping = false;
		creature->Flags = 0;
		creature->MaxTurn = 0;
		joint3 = info.angle / 2;
		if (info.ahead && item->AIBits & FOLLOW)
		{
			joint1 = info.angle / 2;
			joint2 = info.xAngle;
		}

		if (item->AIBits & GUARD)
		{
			joint3 = AIGuard(creature);
			item->Animation.TargetState = 0;
			break;
		}

		if (item->AIBits & MODIFY)
		{
			item->Animation.TargetState = STATE_VON_CROY_STOP;
			if (item->Floor > item->Position.yPos + (STEP_SIZE * 3))
				item->AIBits &= ~MODIFY;
			break;
		}

		if (canJump3blocks || item->ItemFlags[2] == VON_CROY_FLAG_JUMP)
		{
			if (item->ItemFlags[2] != VON_CROY_FLAG_JUMP && !canJump2blocks)
			{
				item->Animation.TargetState = STATE_VON_CROY_JUMP_BACK;
			}
			else
			{
				item->Animation.TargetState = STATE_VON_CROY_RUN;
			}
			break;
		}
		else if (canJump1block || canJump2blocks)
		{
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 22;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = STATE_VON_CROY_JUMP;
			creature->LOT.IsJumping = true;

			if (!canJump2blocks && !canJump3blocks)
				item->Animation.TargetState = STATE_VON_CROY_JUMP;
			else
			{
				item->Animation.TargetState = STATE_VON_CROY_JUMP_2BLOCKS;
			}
			break;
		}

		if (creature->MonkeySwingAhead)
		{
			floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
			height = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
			if (GetCeiling(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos) == height - 1536)
			{
				if (item->SwapMeshFlags == SWAPMESHFLAGS_VON_CROY)
					item->Animation.TargetState = STATE_VON_CROY_TOGGLE_KNIFE;
				else
					item->Animation.TargetState = STATE_VON_CROY_START_MONKEY;
				break;
			}
		}
		else
		{
			if (creature->Enemy && creature->Enemy->HitPoints > 0 && info.distance < SQUARE(1024) && creature->Enemy != LaraItem 
				&& creature->Enemy->ObjectNumber != ID_AI_FOLLOW)
			{
				if (info.bite)
				{
					if (enemy->HitPoints > 0 && info.ahead)
					{
						if (abs(enemy->Position.yPos - item->Position.yPos + 512) < 512)
							item->Animation.TargetState = STATE_VON_CROY_KNIFE_ATTACK_HIGH;
						else
							item->Animation.TargetState = STATE_VON_CROY_KNIFE_ATTACK_LOW;
						break;
					}
				}
			}
		}

		item->Animation.TargetState = STATE_VON_CROY_WALK;
		break;

	case STATE_VON_CROY_WALK:
		creature->LOT.IsMonkeying = false;
		creature->LOT.IsJumping = false;
		creature->MaxTurn = ANGLE(7);
		creature->Flags = 0;

		if (laraInfo.ahead)
		{
			joint3 = laraInfo.angle;
		}
		else if (info.ahead)
		{
			joint3 = info.angle;
		}
		
		if (canJump1block || canJump2blocks || canJump3blocks)
		{
			creature->MaxTurn = 0;
			item->Animation.TargetState = STATE_VON_CROY_STOP;
			break;
		}

		if (creature->ReachedGoal && creature->MonkeySwingAhead)
		{
			item->Animation.TargetState = STATE_VON_CROY_STOP;
			break;
		}

		if (creature->ReachedGoal)
		{
			if (!creature->Enemy->Flags)
			{
				creature->ReachedGoal = false;
				creature->Enemy = NULL;
				item->AIBits = FOLLOW;
				item->ItemFlags[3]++;

				break;
			}
			item->Animation.TargetState = STATE_VON_CROY_STOP;
			break;
		}
		else
		{
			if (Lara.Location >= item->ItemFlags[3])
			{
				if (!foundTarget || info.distance >= 0x200000 && (item->SwapMeshFlags & 0x40000 || info.distance >= 9437184))
				{
					if (creature->Enemy == LaraItem)
					{
						if (info.distance >= 0x400000)
						{
							if (info.distance > 0x1000000)
							{
								item->Animation.TargetState = STATE_VON_CROY_RUN;
							}
						}
						else
						{
							item->Animation.TargetState = STATE_VON_CROY_STOP;
						}
					}
					else if (Lara.Location > item->ItemFlags[3] && laraInfo.distance > 0x400000)
					{
						item->Animation.TargetState = STATE_VON_CROY_RUN;
					}
				}
				else
				{
					item->Animation.TargetState = STATE_VON_CROY_STOP;
				}
			}
			else
			{
				item->Animation.TargetState = STATE_VON_CROY_STOP;
			}
		}

		if (info.bite)
		{
			if (info.distance < SQUARE(1024))
			{
				item->Animation.TargetState = STATE_VON_CROY_STOP;
				break;
			}
		}

		if (creature->Mood == MoodType::Attack &&
			!(creature->JumpAhead) &&
			info.distance > SQUARE(1024))
		{
			item->Animation.TargetState = STATE_VON_CROY_RUN;
		}
		break;

	case STATE_VON_CROY_RUN:
		if (info.ahead)
		{
			joint3 = info.angle;
		}

		if (item->ItemFlags[2] == VON_CROY_FLAG_JUMP)
		{
			creature->MaxTurn = 0;
			item->Animation.TargetState = STATE_VON_CROY_JUMP_2BLOCKS;
			break;
		}

		creature->MaxTurn = ANGLE(11);
		tilt = abs(angle) / 2;

		if (info.distance < SQUARE(2048) || Lara.Location < creature->LocationAI)
		{
			item->Animation.TargetState = STATE_VON_CROY_STOP;
			break;
		}

		if (creature->ReachedGoal)
		{
			if (!enemy->Flags)
			{
				creature->ReachedGoal = false;
				creature->Enemy = NULL;
				item->AIBits = FOLLOW;

				break;
			}

			item->Animation.TargetState = STATE_VON_CROY_STOP;
			break;
		}

		if (canJump1block
			|| canJump2blocks
			|| canJump3blocks
			|| creature->MonkeySwingAhead
			|| item->AIBits & FOLLOW
			|| info.distance < SQUARE(1024)
			|| creature->JumpAhead)
		{
			item->Animation.TargetState = STATE_VON_CROY_STOP;
			break;
		}

		if (info.distance < SQUARE(1024))
		{
			item->Animation.TargetState = STATE_VON_CROY_WALK;
			break;
		}

		break;

	case STATE_VON_CROY_START_MONKEY:
		creature->MaxTurn = 0;
		if (item->BoxNumber != creature->LOT.TargetBox && creature->MonkeySwingAhead) 
		{
			item->Animation.TargetState = STATE_VON_CROY_MONKEY;
		}
		else
		{
			floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
			height = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
			ceiling = GetCeiling(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
			if (ceiling == height - 1536)
				item->Animation.TargetState = STATE_VON_CROY_STOP;
		}

		break;

	case STATE_VON_CROY_MONKEY:
		creature->LOT.IsMonkeying = true;
		creature->LOT.IsJumping = true;
		creature->MaxTurn = ANGLE(6);

		if (item->BoxNumber == creature->LOT.TargetBox || !creature->MonkeySwingAhead)
		{
			floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
			height = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
			if (GetCeiling(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos) == height - 1536)
				item->Animation.TargetState = STATE_VON_CROY_START_MONKEY;
		}

		break;

	case STATE_VON_CROY_TOGGLE_KNIFE:
		if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase) 
		{
			if (!(item->SwapMeshFlags & SWAPMESHFLAGS_VON_CROY))
			{
				item->SwapMeshFlags |= SWAPMESHFLAGS_VON_CROY;
			}
			else 
			{
				item->SwapMeshFlags &= ~SWAPMESHFLAGS_VON_CROY;
			}
		}
		break;

	case STATE_VON_CROY_LOOK_BEFORE_JUMP:
		if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
		{
			item->Position.xPos = enemy->Position.xPos;
			item->Position.yPos = enemy->Position.yPos;
			item->Position.zPos = enemy->Position.zPos;
			item->Position.xRot = enemy->Position.xRot;
			item->Position.yRot = enemy->Position.yRot;
			item->Position.zRot = enemy->Position.zRot;

			if (item->ItemFlags[2] == VON_CROY_FLAG_JUMP)
			{
				creature->MaxTurn = 0;
				item->Animation.ActiveState = STATE_VON_CROY_JUMP;
				item->Animation.TargetState = STATE_VON_CROY_JUMP_2BLOCKS;
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 22;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				creature->LOT.IsJumping = true;
			}

			creature->ReachedGoal = false;
			creature->Enemy = NULL;
			item->AIBits = FOLLOW;
			creature->LocationAI++;
		}

		break;

	case STATE_VON_CROY_JUMP_2BLOCKS:
		if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + 25
			|| item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 5)
		{
			creature->LOT.IsJumping = true;
			//if (canJump3blocks)
			//	item->itemFlags[2] = VON_CROY_FLAG_JUMP;
		}
		else if (canJump1block)
		{
			item->Animation.TargetState = STATE_VON_CROY_JUMP;
		}

		if (item->ItemFlags[2] == VON_CROY_FLAG_JUMP)
		{
			item->Animation.TargetState = 33;
		}

		break;

	case STATE_VON_CROY_ENABLE_TRAP:
		if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
		{
			item->Position.xPos = enemy->Position.xPos;
			item->Position.yPos = enemy->Position.yPos;
			item->Position.zPos = enemy->Position.zPos;
			item->Position.xRot = enemy->Position.xRot;
			item->Position.yRot = enemy->Position.yRot;
			item->Position.zRot = enemy->Position.zRot;
		}
		else if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 120)
		{
			TestTriggers(
				creature->AITarget->Position.xPos,
				creature->AITarget->Position.yPos,
				creature->AITarget->Position.zPos,
				creature->AITarget->RoomNumber,
				true);

			creature->ReachedGoal = false;
			creature->Enemy = NULL;
			item->AIBits = FOLLOW;
			creature->LocationAI++;
		}

		break;

	case STATE_VON_CROY_KNIFE_ATTACK_HIGH:
		if (info.ahead) 
		{
			joint2 = info.angle / 2;
			joint1 = info.xAngle / 2;
			joint0 = joint2;
		}

		creature->MaxTurn = 0;
		ClampRotation(&item->Position, info.angle, ANGLE(6));

		if (!creature->Flags && enemy != NULL) 
		{
			if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 20 
				&& item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 45) 
			{
				if (abs(item->Position.xPos - enemy->Position.xPos) < 512
					&& abs(item->Position.yPos - enemy->Position.yPos) < 512
					&& abs(item->Position.zPos - enemy->Position.zPos) < 512)
				{
					enemy->HitPoints -= 40;
					if (enemy->HitPoints <= 0)
					{
						item->AIBits = FOLLOW;
					}
					enemy->HitStatus = true;
					creature->Flags = 1;
					CreatureEffect2(item, &VonCroyBite, 2, -1, DoBloodSplat);
				}
			}
		}

		break;

	case STATE_VON_CROY_LOOK_BACK_LEFT:
	case STATE_VON_CROY_LOOK_BACK_RIGHT:
		creature->MaxTurn = 0;
		if (item->ItemFlags[2] == 0)
		{
			ClampRotation(&item->Position, laraInfo.angle, 512);
		}
		else
		{
			ClampRotation(&item->Position, enemy->Position.yRot - item->Position.yRot, 512);
		}
		break;

	case STATE_VON_CROY_GRAB_LADDER:
		creature->LOT.IsJumping = true;
		creature->MaxTurn = 0;

		/*if (!creature->reachedGoal) 
		{
			item->TargetState = STATE_VON_CROY_CLIMB_LADDER_RIGHT;
		}
		else
		{*/
			item->Animation.TargetState = STATE_VON_CROY_LADDER_CLIMB_UP;
			creature->ReachedGoal = false;
			creature->Enemy = NULL;
			item->AIBits = FOLLOW;
			creature->LocationAI++;
		//}
		break;

	case STATE_VON_CROY_CLIMB_LADDER_RIGHT:
		creature->LOT.IsJumping = true;
		creature->MaxTurn = 0;
		break;

	case STATE_VON_CROY_KNIFE_ATTACK_LOW:
		if (info.ahead)
		{
			joint2 = info.angle / 2;
			joint1 = info.xAngle / 2;
			joint0 = joint2;
		}

		creature->MaxTurn = 0;
		ClampRotation(&item->Position, info.angle, ANGLE(6));

		if ((enemy == NULL
			|| enemy->Flags != 0) 
			|| item->Animation.FrameNumber <= g_Level.Anims[item->Animation.AnimNumber].frameBase + 21)
		{
			if (creature->Flags == 0 && enemy != NULL) 
			{
				if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 15 
					&& item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 26)
				{
					if (abs(item->Position.xPos - enemy->Position.xPos) < 512
						&& abs(item->Position.yPos - enemy->Position.yPos) < 512
						&& abs(item->Position.zPos - enemy->Position.zPos) < 512)
					{
						enemy->HitPoints -= 20;
						if (enemy->HitPoints <= 0)
						{
							item->AIBits = FOLLOW;
						}
						enemy->HitStatus = true;
						creature->Flags = 1;
						CreatureEffect2(item, &VonCroyBite, 2, -1, DoBloodSplat);
					}
				}
			}
			break;
		}

		TestTriggers(
			creature->AITarget->Position.xPos,
			creature->AITarget->Position.yPos,
			creature->AITarget->Position.zPos,
			creature->AITarget->RoomNumber,
			true);

		creature->ReachedGoal = false;
		creature->Enemy = NULL;
		item->AIBits = FOLLOW;
		creature->LocationAI++;

		break;

	case 32:
		if (info.ahead) 
		{
			joint2 = info.angle / 2;
			joint1 = info.xAngle;
			joint0 = joint2;
		}
		
		creature->MaxTurn = 0;
		ClampRotation(&item->Position, info.angle / 2, ANGLE(6));

		if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + 47) 
		{
			if (item->Animation.FrameNumber != g_Level.Anims[item->Animation.AnimNumber].frameBase) 
				break;
		}
		else 
		{
			if (GetRandomControl() & 0x1F)
				break;
			item->Animation.TargetState = STATE_VON_CROY_STOP;
		}

		creature->ReachedGoal = false;
		creature->Enemy = NULL;
		item->AIBits = FOLLOW;
		creature->LocationAI++;

		break;

	case 33:
		flags = 1;
		if (item->Animation.AnimNumber != Objects[item->ObjectNumber].animIndex + 52
			|| item->Animation.FrameNumber != g_Level.Anims[item->Animation.AnimNumber].frameBase) 
		{
			flags = 0;
		}
		
		item->Animation.TargetState = STATE_VON_CROY_WALK;
		item->Animation.RequiredState = STATE_VON_CROY_RUN;
		item->ItemFlags[2] = 0;
		//if (sVar3 == -1) goto LAB_0041a991;
		if (!flags)
		{
			creature->ReachedGoal = false;
			creature->Enemy = NULL;
			item->AIBits = FOLLOW;
			creature->LocationAI++;
		}

		break;
	
	case STATE_VON_CROY_JUMP_BACK:
		item->ItemFlags[2] = VON_CROY_FLAG_JUMP;
		break;

	case 36:
	case 37:
		creature->MaxTurn = 0;
		MoveCreature3DPos(&item->Position, &enemy->Position, 15, enemy->Position.yRot-item->Position.yRot, 512);
		break;

	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureJoint(item, 3, joint3);
	
	if ((item->Animation.ActiveState < 15) && (item->Animation.ActiveState != 5)) 
	{
		switch (CreatureVault(itemNumber, angle, 2, 260)) 
		{
		case 2:
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex+29;
			item->Animation.ActiveState = 19;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			break;

		case 3:
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 28;
			item->Animation.ActiveState = 18;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			break;

		case 4:
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 27;
			item->Animation.ActiveState = 17;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			break;

		case 7:
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 37;
			item->Animation.ActiveState = 27;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			break;

		case -7:
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 36;
			item->Animation.ActiveState = 26;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			break;

		case -4:
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 35;
			item->Animation.ActiveState = 25;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			break;

		case -3:
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 41;
			item->Animation.ActiveState = 24;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			break;

		case -2:
			creature->MaxTurn = 0;
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 42;
			item->Animation.ActiveState = 23;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			break;

		}
	}
	else
	{
		CreatureAnimation(itemNumber, angle, 0);
	}
}