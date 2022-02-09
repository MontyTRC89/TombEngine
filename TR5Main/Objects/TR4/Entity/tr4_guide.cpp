#include "framework.h"
#include "tr4_guide.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

#define	STATE_GUIDE_STOP				1
#define	STATE_GUIDE_WALK				2
#define	STATE_GUIDE_RUN					3
#define	STATE_GUIDE_IGNITE_TORCH		11
#define STATE_GUIDE_LOOK_BACK			22
#define	STATE_GUIDE_TORCH_ATTACK		31
#define STATE_GUIDE_PICKUP_TORCH		37

BITE_INFO guideBiteInfo1 = { 0, 20, 200, 18 };
BITE_INFO guideBiteInfo2 = { 30, 80, 50, 15 };

void InitialiseGuide(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 4;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->TargetState = STATE_GUIDE_STOP;
	item->ActiveState = STATE_GUIDE_STOP;
	
	if (Objects[ID_WRAITH1].loaded)
	{
		item->SwapMeshFlags = 0;
		item->ItemFlags[1] = 2;
	}
	else
	{
		item->SwapMeshFlags = 0x40000;
	}
}

void GuideControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
	OBJECT_INFO* obj = &Objects[item->ObjectNumber];

	short angle = 0;
	short tilt = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;

	// Ignite torch
	if (item->ItemFlags[1] == 2)
	{
		PHD_VECTOR pos;

		pos.x = guideBiteInfo1.x;
		pos.y = guideBiteInfo1.y;
		pos.z = guideBiteInfo1.z;

		GetJointAbsPosition(item, &pos, guideBiteInfo1.meshNum);

		AddFire(pos.x, pos.y, pos.z, 0, item->RoomNumber, 0);
		SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Position, 0);
		TriggerFireFlame(pos.x, pos.y - 40, pos.z, -1, 7);

		short random = GetRandomControl();
		TriggerDynamicLight(
			pos.x, 
			pos.y,
			pos.z, 
			15, 
			255 - ((random >> 4) & 0x1F), 
			192 - ((random >> 6) & 0x1F), 
			random & 0x3F);

		if (item->AnimNumber == obj->animIndex + 61)
		{
			if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 32 &&
				item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 42)
			{
				TriggerFireFlame(
					(random & 0x3F) + pos.x - 32,
					((random / 8) & 0x3F) + pos.y - 128,
					pos.z + ((random / 64) & 0x3F) - 32,
					-1,
					1);
			}
		}
	}

	item->AIBits = FOLLOW;
	item->HitPoints = NOT_TARGETABLE;

	GetAITarget(creature);

	AI_INFO info;
	AI_INFO laraInfo;

	int dx = LaraItem->Position.xPos - item->Position.xPos;
	int dz = LaraItem->Position.zPos - item->Position.zPos;

	laraInfo.angle = phd_atan(dz, dx) - item->Position.yRot;

	laraInfo.ahead = true;
	if (laraInfo.angle <= -ANGLE(90) || laraInfo.angle >= ANGLE(90))
		laraInfo.ahead = false;

	int distance = 0;
	if (dz > 32000 || dz < -32000 || dx > 32000 || dx < -32000)
		laraInfo.distance = 0x7FFFFFFF;
	else
		laraInfo.distance = SQUARE(dx) + SQUARE(dz);

	dx = abs(dx);
	dz = abs(dz);

	int dy = item->Position.yPos - LaraItem->Position.yPos;
	short rot2 = 0;

	if (dx <= dz)
		laraInfo.xAngle = phd_atan(dz + (dx / 2), dy);
	else
		laraInfo.xAngle = phd_atan(dx + (dz / 2), dy);

	ITEM_INFO* foundEnemy = NULL;

	if (!Objects[ID_WRAITH1].loaded)
	{
		if (item->ActiveState < 4
			|| item->ActiveState == STATE_GUIDE_TORCH_ATTACK)
		{
			int minDistance = 0x7FFFFFFF;

			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				CREATURE_INFO* baddie = ActiveCreatures[i];

				if (baddie->itemNum == NO_ITEM || baddie->itemNum == itemNumber)
					continue;

				ITEM_INFO* currentItem = &g_Level.Items[baddie->itemNum];
				if (currentItem->ObjectNumber != ID_GUIDE &&
					abs(currentItem->Position.yPos - item->Position.yPos) <= 512)
				{
					dx = currentItem->Position.xPos - item->Position.xPos;
					dy = currentItem->Position.yPos - item->Position.yPos;
					dz = currentItem->Position.zPos - item->Position.zPos;

					if (dx > 32000 || dx < -32000 || dz > 32000 || dz < -32000)
						distance = 0x7FFFFFFF;
					else
						distance = SQUARE(dx) + SQUARE(dz);

					if (distance < minDistance 
						&& distance < SQUARE(2048) 
						&& (abs(dy) < 256 
							|| laraInfo.distance < SQUARE(2048) 
							|| currentItem->ObjectNumber == ID_DOG))
					{
						foundEnemy = currentItem;
						minDistance = distance;
					}
				}
			}
		}
	}

	ITEM_INFO* enemy = creature->enemy;
	if (foundEnemy)
		creature->enemy = foundEnemy;

	CreatureAIInfo(item, &info);

	GetCreatureMood(item, &info, VIOLENT);
	CreatureMood(item, &info, VIOLENT);

	angle = CreatureTurn(item, creature->maximumTurn);

	if (foundEnemy)
	{
		creature->enemy = enemy;
		enemy = foundEnemy;
	}

	bool someFlag = false;
	FLOOR_INFO* floor;
	PHD_VECTOR pos1;
	int frameNumber;
	short random;

	TENLog("Guide state:" + std::to_string(item->ActiveState), LogLevel::Info);

	switch (item->ActiveState)
	{
	case STATE_GUIDE_STOP:
		creature->LOT.isJumping = false;
		creature->flags = 0;
		creature->maximumTurn = 0;
		joint2 = info.angle / 2;

		if (laraInfo.ahead)
		{
			joint0 = laraInfo.angle / 2;
			joint1 = laraInfo.xAngle / 2;
			joint2 = laraInfo.angle / 2;
		}
		else if (info.ahead)
		{
			joint0 = info.angle / 2;
			joint1 = info.xAngle / 2;
			joint2 = info.angle / 2;
		}

		if (Objects[ID_WRAITH1].loaded)
		{
			if (item->ItemFlags[3] == 5)
				item->TargetState = STATE_GUIDE_WALK;

			if (item->ItemFlags[3] == 5 || item->ItemFlags[3] == 6)
			{
				break;
			}
		}

		if (item->RequiredState)
		{
			item->TargetState = item->RequiredState;
		}
		else if (Lara.location >= item->ItemFlags[3] 
			|| item->ItemFlags[1] != 2)
		{
			if (!creature->reachedGoal || foundEnemy)
			{
				if (item->SwapMeshFlags == 0x40000)
				{
					item->TargetState = 40;
				}
				else if (foundEnemy && info.distance < SQUARE(1024))
				{
					if (info.bite)
					{
						item->TargetState = STATE_GUIDE_TORCH_ATTACK;
					}
				}
				else if (enemy != LaraItem || info.distance > SQUARE(2048))
				{
					item->TargetState = STATE_GUIDE_WALK;
				}
			}
			else
			{
				if (!enemy->Flags)
				{
					creature->reachedGoal = false;
					creature->enemy = NULL;
					item->AIBits = FOLLOW;
					item->ItemFlags[3]++;

					break;
				}

				if (info.distance <= SQUARE(128))
				{
					switch (enemy->Flags)
					{
					case 0x02:
						item->TargetState = 38;
						item->RequiredState = 38;

						break;

					case 0x20:
						item->TargetState = STATE_GUIDE_PICKUP_TORCH;
						item->RequiredState = STATE_GUIDE_PICKUP_TORCH;

						break;

					case 0x28:
						if (laraInfo.distance < SQUARE(2048))
						{
							item->TargetState = 39;
							item->RequiredState = 39;
						}

						break;

					case 0x10:
						if (laraInfo.distance < SQUARE(2048))
						{
							// Ignite torch
							item->TargetState = 36;
							item->RequiredState = 36;
						}

						break;

					case 0x04:
						if (laraInfo.distance < SQUARE(2048))
						{
							item->TargetState = 36;
							item->RequiredState = 43;
						}

						break;

					case 0x3E:
						item->Status = ITEM_INVISIBLE;
						RemoveActiveItem(itemNumber);
						DisableBaddieAI(itemNumber);

						break;

					}
				}
				else
				{
					creature->maximumTurn = 0;
					item->RequiredState = 42 - (info.ahead != 0);
				}
			}
		}
		else
		{
			item->TargetState = STATE_GUIDE_STOP;
		}

		break;

	case STATE_GUIDE_WALK:
		creature->LOT.isJumping = false;

		creature->maximumTurn = ANGLE(7);

		if (laraInfo.ahead)
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

		if (Objects[ID_WRAITH1].loaded && item->ItemFlags[3] == 5)
		{
			item->ItemFlags[3] = 6;
			item->TargetState = STATE_GUIDE_STOP;
		}
		else if (item->ItemFlags[1] == 1)
		{
			item->TargetState = STATE_GUIDE_STOP;
			item->RequiredState = STATE_GUIDE_IGNITE_TORCH; 
		}
		else if (creature->reachedGoal)
		{
			if (!enemy->Flags)
			{
				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->AIBits = FOLLOW;
				item->ItemFlags[3]++;

				break;
			}
			item->TargetState = STATE_GUIDE_STOP;
		}
		else
		{
			if (Lara.location >= item->ItemFlags[3])
			{
				if (!foundEnemy 
					|| info.distance >= 0x200000
					&& (item->SwapMeshFlags & 0x40000
						|| info.distance >= SQUARE(3072)))
				{
					if (creature->enemy == LaraItem)
					{
						if (info.distance >= SQUARE(2048))
						{
							if (info.distance > SQUARE(4096))
							{
								item->TargetState = STATE_GUIDE_RUN;
							}
						}
						else
						{
							item->TargetState = STATE_GUIDE_STOP;
						}
					}
					else if (Lara.location > item->ItemFlags[3]
						&& laraInfo.distance > SQUARE(2048))
					{
						item->TargetState = STATE_GUIDE_RUN;
					}
				}
				else
				{
					item->TargetState = STATE_GUIDE_STOP;
				}
			}
			else
			{
				item->TargetState = STATE_GUIDE_STOP;
			}
		}

		break;

	case STATE_GUIDE_RUN:
		if (info.ahead)
		{
			joint2 = info.angle;
		}

		creature->maximumTurn = ANGLE(11);
		tilt = angle / 2;

		if (info.distance < SQUARE(2048) 
			|| Lara.location < item->ItemFlags[3])
		{
			item->TargetState = STATE_GUIDE_STOP;
			break;
		}
		if (creature->reachedGoal)
		{
			if (!enemy->Flags)
			{
				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->AIBits = FOLLOW;
				item->ItemFlags[3]++;

				break;
			}
			item->TargetState = STATE_GUIDE_STOP;
		}
		else if (foundEnemy && 
			(info.distance < 0x200000 
				|| !(item->SwapMeshFlags & 0x40000) 
				&& info.distance < SQUARE(3072)))
		{
			item->TargetState = STATE_GUIDE_STOP;
			break;
		}

		break;

	case STATE_GUIDE_IGNITE_TORCH:
		// Ignite torch
		pos1.x = guideBiteInfo2.x;
		pos1.y = guideBiteInfo2.y;
		pos1.z = guideBiteInfo2.z;

		GetJointAbsPosition(item, &pos1, guideBiteInfo2.meshNum);

		frameNumber = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;
		random = GetRandomControl();

		if (frameNumber == 32)
		{
			item->SwapMeshFlags |= 0x8000;
		}
		else if (frameNumber == 216)
		{
			item->SwapMeshFlags &= 0x7FFF;
		}
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
							7);

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
					7);
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

	case STATE_GUIDE_LOOK_BACK:
		creature->maximumTurn = 0;

		if (laraInfo.angle < -256)
		{
			item->Position.yRot -= 399;
		}

		break;

	case STATE_GUIDE_TORCH_ATTACK:
		if (info.ahead)
		{
			joint0 = info.angle / 2;
			joint2 = info.angle / 2;
			joint1 = info.xAngle / 2;
		}

		creature->maximumTurn = 0;

		if (abs(info.angle) >= ANGLE(7))
		{
			if (info.angle < 0)
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

		if (!creature->flags)
		{
			if (enemy)
			{
				if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 15 &&
					item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 26)
				{
					dx = abs(enemy->Position.xPos - item->Position.xPos);
					dy = abs(enemy->Position.yPos - item->Position.yPos);
					dz = abs(enemy->Position.zPos - item->Position.zPos);

					if (dx < 512 && dy < 512 && dz < 512)
					{
						enemy->HitPoints -= 20;

						if (enemy->HitPoints <= 0)
						{
							item->AIBits = FOLLOW;
						}

						enemy->HitStatus = true;
						creature->flags = 1;

						CreatureEffect2(
							item,
							&guideBiteInfo1,
							8,
							-1,
							DoBloodSplat);
					}
				}
			}
		}

		break;

	case 35:
		creature->maximumTurn = 0;

		if (laraInfo.angle > 256)
		{
			item->Position.yRot += 399;
		}

		break;

	case 36:
	case 43:
		if (enemy)
		{
			short deltaAngle = enemy->Position.yRot - item->Position.yRot;
			if (deltaAngle < -ANGLE(2))
				item->Position.yRot -= ANGLE(2);
			else if (deltaAngle > ANGLE(2))
				item->Position.yRot = ANGLE(2);
		}

		if (item->RequiredState == 43)
		{
			item->TargetState = 43;
		}
		else
		{
			if (item->AnimNumber != obj->animIndex + 57
				&& item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd - 20)
			{
				TestTriggers(item, true);

				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->AIBits = FOLLOW;
				item->ItemFlags[3]++;
				item->TargetState = STATE_GUIDE_STOP;

				break;
			}
		}

		break;

	case STATE_GUIDE_PICKUP_TORCH:
		if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
		{
			someFlag = true;

			item->Position.xPos = enemy->Position.xPos;
			item->Position.yPos = enemy->Position.yPos;
			item->Position.zPos = enemy->Position.zPos;
			item->Position.xRot = enemy->Position.xRot;
			item->Position.yRot = enemy->Position.yRot;
			item->Position.zRot = enemy->Position.zRot;
		}
		else if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 35)
		{
			item->SwapMeshFlags &= 0xFFFBFFFF;

			ROOM_INFO* room = &g_Level.Rooms[item->RoomNumber];
			ITEM_INFO* currentItem = NULL;

			short currentitemNumber = room->itemNumber;
			while (currentitemNumber != NO_ITEM)
			{
				currentItem = &g_Level.Items[currentitemNumber];

				if (currentItem->ObjectNumber >= ID_ANIMATING1
					&& currentItem->ObjectNumber <= ID_ANIMATING15
					&& trunc(item->Position.xPos) == trunc(currentItem->Position.xPos)
					&& trunc(item->Position.zPos) == trunc(currentItem->Position.zPos))
				{
					break;
				}

				currentitemNumber = currentItem->NextItem;
			}

			if (currentItem != NULL)
				currentItem->MeshBits = 0xFFFFFFFD;
		}

		item->ItemFlags[1] = 1;
		if (someFlag)
		{
			creature->reachedGoal = false;
			creature->enemy = NULL;
			item->AIBits = FOLLOW;
			item->ItemFlags[3]++;
		}

		break;

	case 38:
		if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
		{
			item->Position.xPos = enemy->Position.xPos;
			item->Position.yPos = enemy->Position.yPos;
			item->Position.zPos = enemy->Position.zPos;
		}
		else
		{
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 42)
			{

				TestTriggers(item, true);
				item->Position.yRot = enemy->Position.yRot;
				
				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->AIBits = FOLLOW;
				item->ItemFlags[3]++;

				break;
			}
			else if (item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 42)
			{
				if (enemy->Position.yRot - item->Position.yRot <= ANGLE(2))
				{
					if (enemy->Position.yRot - item->Position.yRot < -ANGLE(2))
					{
						item->Position.yRot -= ANGLE(2);
					}
				}
				else
				{
					item->Position.yRot += ANGLE(2);
				}
			}
		}

		break;

	case 39:
		if (item->FrameNumber >= g_Level.Anims[item->AnimNumber].frameBase + 20)
		{
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 20)
			{
				item->TargetState = STATE_GUIDE_STOP;

				TestTriggers(item, true);

				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->AIBits = FOLLOW;
				item->ItemFlags[3]++;

				break;
			}

			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 70 && item->RoomNumber == 70)
			{
				item->RequiredState = STATE_GUIDE_RUN;
				item->SwapMeshFlags |= 0x200000;
				SoundEffect(SFX_TR4_GUIDE_SCARE, &item->Position, 0);
			}
		}
		else if (enemy->Position.yRot - item->Position.yRot <= ANGLE(2))
		{
			if (enemy->Position.yRot - item->Position.yRot < -ANGLE(2))
			{
				item->Position.yRot -= ANGLE(2);
			}
		}
		else
		{
			item->Position.yRot += ANGLE(2);
		}

		break;

	case 40:
		creature->LOT.isJumping;
		creature->maximumTurn = ANGLE(7);

		if (laraInfo.ahead)
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
		if (!(creature->reachedGoal))
		{
			break;
		}

		if (!enemy->Flags)
		{
			creature->reachedGoal = false;
			creature->enemy = NULL;
			item->AIBits = FOLLOW;
			item->ItemFlags[3]++;

			break;
		}
		if (enemy->Flags == 42)
		{
			TestTriggers(item, true);
			
			creature->reachedGoal = false;
			creature->enemy = NULL;
			item->AIBits = FOLLOW;
			item->ItemFlags[3]++;
		}
		else if (item->TriggerFlags <= 999)
		{
			item->TargetState = STATE_GUIDE_STOP;
		}
		else
		{
			KillItem(itemNumber);
			DisableBaddieAI(itemNumber);
			item->Flags |= 1;
		}

		break;

	case 41:
	case 42:
		creature->maximumTurn = 0;
		MoveCreature3DPos(&item->Position, &enemy->Position, 15, enemy->Position.yRot - item->Position.yRot, ANGLE(10));

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