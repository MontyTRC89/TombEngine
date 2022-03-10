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

BITE_INFO GuideBite1 = { 0, 20, 200, 18 };
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

	item->AnimNumber = Objects[item->ObjectNumber].animIndex + 4;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->TargetState = GUIDE_STATE_IDLE;
	item->ActiveState = GUIDE_STATE_IDLE;
	
	if (Objects[ID_WRAITH1].loaded)
	{
		item->SwapMeshFlags = 0;
		item->ItemFlags[1] = 2;
	}
	else
		item->SwapMeshFlags = 0x40000;
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
		PHD_VECTOR pos;
		pos.x = GuideBite1.x;
		pos.y = GuideBite1.y;
		pos.z = GuideBite1.z;
		GetJointAbsPosition(item, &pos, GuideBite1.meshNum);

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

		if (item->AnimNumber == object->animIndex + 61)
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

	AI_INFO AI;
	AI_INFO laraAI;

	int dx = LaraItem->Position.xPos - item->Position.xPos;
	int dz = LaraItem->Position.zPos - item->Position.zPos;

	laraAI.angle = phd_atan(dz, dx) - item->Position.yRot;

	laraAI.ahead = true;
	if (laraAI.angle <= -ANGLE(90.0f) || laraAI.angle >= ANGLE(90.0f))
		laraAI.ahead = false;

	int distance = 0;
	if (dz > 32000 || dz < -32000 || dx > 32000 || dx < -32000)
		laraAI.distance = 0x7FFFFFFF;
	else
		laraAI.distance = pow(dx, 2) + pow(dz, 2);

	dx = abs(dx);
	dz = abs(dz);

	int dy = item->Position.yPos - LaraItem->Position.yPos;
	short rot2 = 0;

	if (dx <= dz)
		laraAI.xAngle = phd_atan(dz + (dx / 2), dy);
	else
		laraAI.xAngle = phd_atan(dx + (dz / 2), dy);

	ITEM_INFO* foundEnemy = NULL;

	if (!Objects[ID_WRAITH1].loaded)
	{
		if (item->ActiveState < 4 ||
			item->ActiveState == GUIDE_STATE_TORCH_ATTACK)
		{
			int minDistance = 0x7FFFFFFF;

			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				auto* currentCreatureInfo = ActiveCreatures[i];

				if (currentCreatureInfo->ItemNumber == NO_ITEM || currentCreatureInfo->ItemNumber == itemNumber)
					continue;

				auto* currentItem = &g_Level.Items[currentCreatureInfo->ItemNumber];

				if (currentItem->ObjectNumber != ID_GUIDE &&
					abs(currentItem->Position.yPos - item->Position.yPos) <= 512)
				{
					dx = currentItem->Position.xPos - item->Position.xPos;
					dy = currentItem->Position.yPos - item->Position.yPos;
					dz = currentItem->Position.zPos - item->Position.zPos;

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
	FLOOR_INFO* floor;
	PHD_VECTOR pos1;
	int frameNumber;
	short random;

	TENLog("Guide state:" + std::to_string(item->ActiveState), LogLevel::Info);

	switch (item->ActiveState)
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
				item->TargetState = GUIDE_STATE_WALK;

			if (item->ItemFlags[3] == 5 || item->ItemFlags[3] == 6)
				break;
		}

		if (item->RequiredState)
			item->TargetState = item->RequiredState;
		else if (Lara.Location >= item->ItemFlags[3] ||
			item->ItemFlags[1] != 2)
		{
			if (!creature->ReachedGoal || foundEnemy)
			{
				if (item->SwapMeshFlags == 0x40000)
					item->TargetState = 40;
				else if (foundEnemy && AI.distance < pow(SECTOR(1), 2))
				{
					if (AI.bite)
						item->TargetState = GUIDE_STATE_TORCH_ATTACK;
				}
				else if (enemy != LaraItem || AI.distance > pow(SECTOR(2), 2))
					item->TargetState = GUIDE_STATE_WALK;
			}
			else
			{
				if (!enemy->Flags)
				{
					creature->ReachedGoal = false;
					creature->Enemy = NULL;
					item->AIBits = FOLLOW;
					item->ItemFlags[3]++;
					break;
				}

				if (AI.distance <= pow(CLICK(0.5f), 2))
				{
					switch (enemy->Flags)
					{
					case 0x02:
						item->TargetState = 38;
						item->RequiredState = 38;
						break;

					case 0x20:
						item->TargetState = GUIDE_STATE_PICKUP_TORCH;
						item->RequiredState = GUIDE_STATE_PICKUP_TORCH;
						break;

					case 0x28:
						if (laraAI.distance < pow(SECTOR(2), 2))
						{
							item->TargetState = 39;
							item->RequiredState = 39;
						}

						break;

					case 0x10:
						if (laraAI.distance < pow(SECTOR(2), 2))
						{
							// Ignite torch
							item->TargetState = 36;
							item->RequiredState = 36;
						}

						break;

					case 0x04:
						if (laraAI.distance < pow(SECTOR(2), 2))
						{
							item->TargetState = 36;
							item->RequiredState = 43;
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
					item->RequiredState = 42 - (AI.ahead != 0);
				}
			}
		}
		else
			item->TargetState = GUIDE_STATE_IDLE;

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
			item->TargetState = GUIDE_STATE_IDLE;
		}
		else if (item->ItemFlags[1] == 1)
		{
			item->TargetState = GUIDE_STATE_IDLE;
			item->RequiredState = GUIDE_STATE_IGNITE_TORCH; 
		}
		else if (creature->ReachedGoal)
		{
			if (!enemy->Flags)
			{
				creature->ReachedGoal = false;
				creature->Enemy = NULL;
				item->AIBits = FOLLOW;
				item->ItemFlags[3]++;
				break;
			}

			item->TargetState = GUIDE_STATE_IDLE;
		}
		else
		{
			if (Lara.Location >= item->ItemFlags[3])
			{
				if (!foundEnemy ||
					AI.distance >= 0x200000 &&
					(item->SwapMeshFlags & 0x40000 || AI.distance >= pow(SECTOR(3), 2)))
				{
					if (creature->Enemy == LaraItem)
					{
						if (AI.distance >= pow(SECTOR(2), 2))
						{
							if (AI.distance > pow(SECTOR(4), 2))
								item->TargetState = GUIDE_STATE_RUN;
						}
						else
							item->TargetState = GUIDE_STATE_IDLE;
					}
					else if (Lara.Location > item->ItemFlags[3] &&
						laraAI.distance > pow(SECTOR(2), 2))
					{
						item->TargetState = GUIDE_STATE_RUN;
					}
				}
				else
					item->TargetState = GUIDE_STATE_IDLE;
			}
			else
				item->TargetState = GUIDE_STATE_IDLE;
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
			item->TargetState = GUIDE_STATE_IDLE;
			break;
		}

		if (creature->ReachedGoal)
		{
			if (!enemy->Flags)
			{
				creature->ReachedGoal = false;
				creature->Enemy = NULL;
				item->AIBits = FOLLOW;
				item->ItemFlags[3]++;
				break;
			}

			item->TargetState = GUIDE_STATE_IDLE;
		}
		else if (foundEnemy && 
			(AI.distance < 0x200000 ||
				!(item->SwapMeshFlags & 0x40000) &&
				AI.distance < pow(SECTOR(3), 2)))
		{
			item->TargetState = GUIDE_STATE_IDLE;
			break;
		}

		break;

	case GUIDE_STATE_IGNITE_TORCH:
		// Ignite torch
		pos1.x = GuideBite2.x;
		pos1.y = GuideBite2.y;
		pos1.z = GuideBite2.z;

		GetJointAbsPosition(item, &pos1, GuideBite2.meshNum);

		frameNumber = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;
		random = GetRandomControl();

		if (frameNumber == 32)
			item->SwapMeshFlags |= 0x8000;
		else if (frameNumber == 216)
			item->SwapMeshFlags &= 0x7FFF;
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

	case GUIDE_STATE_LOOK_BACK:
		creature->MaxTurn = 0;

		if (laraAI.angle < -256)
			item->Position.yRot -= 399;

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
				item->Position.yRot += ANGLE(7.0f);
			else
				item->Position.yRot -= ANGLE(7.0f);
		}
		else
			item->Position.yRot += AI.angle;

		if (!creature->Flags)
		{
			if (enemy)
			{
				if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 15 &&
					item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 26)
				{
					dx = abs(enemy->Position.xPos - item->Position.xPos);
					dy = abs(enemy->Position.yPos - item->Position.yPos);
					dz = abs(enemy->Position.zPos - item->Position.zPos);

					if (dx < CLICK(2) &&
						dy < CLICK(2) &&
						dz < CLICK(2))
					{
						enemy->HitPoints -= 20;

						if (enemy->HitPoints <= 0)
							item->AIBits = FOLLOW;

						enemy->HitStatus = true;
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
			item->Position.yRot += 399;

		break;

	case 36:
	case 43:
		if (enemy)
		{
			short deltaAngle = enemy->Position.yRot - item->Position.yRot;
			if (deltaAngle < -ANGLE(2.0f))
				item->Position.yRot -= ANGLE(2.0f);
			else if (deltaAngle > ANGLE(2.0f))
				item->Position.yRot = ANGLE(2.0f);
		}

		if (item->RequiredState == 43)
			item->TargetState = 43;
		else
		{
			if (item->AnimNumber != object->animIndex + 57 &&
				item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd - 20)
			{
				TestTriggers(item, true);

				creature->ReachedGoal = false;
				creature->Enemy = NULL;
				item->AIBits = FOLLOW;
				item->ItemFlags[3]++;
				item->TargetState = GUIDE_STATE_IDLE;
				break;
			}
		}

		break;

	case GUIDE_STATE_PICKUP_TORCH:
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

			auto* room = &g_Level.Rooms[item->RoomNumber];
			ITEM_INFO* currentItem = NULL;

			short currentitemNumber = room->itemNumber;
			while (currentitemNumber != NO_ITEM)
			{
				currentItem = &g_Level.Items[currentitemNumber];

				if (currentItem->ObjectNumber >= ID_ANIMATING1 &&
					currentItem->ObjectNumber <= ID_ANIMATING15 &&
					trunc(item->Position.xPos) == trunc(currentItem->Position.xPos) &&
					trunc(item->Position.zPos) == trunc(currentItem->Position.zPos))
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
			item->AIBits = FOLLOW;
			item->ItemFlags[3]++;
			creature->ReachedGoal = false;
			creature->Enemy = NULL;
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
				item->AIBits = FOLLOW;
				item->ItemFlags[3]++;
				creature->ReachedGoal = false;
				creature->Enemy = NULL;
				break;
			}
			else if (item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 42)
			{
				if (enemy->Position.yRot - item->Position.yRot <= ANGLE(2.0f))
				{
					if (enemy->Position.yRot - item->Position.yRot < -ANGLE(2.0f))
						item->Position.yRot -= ANGLE(2.0f);
				}
				else
					item->Position.yRot += ANGLE(2.0f);
			}
		}

		break;

	case 39:
		if (item->FrameNumber >= g_Level.Anims[item->AnimNumber].frameBase + 20)
		{
			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 20)
			{
				item->TargetState = GUIDE_STATE_IDLE;

				TestTriggers(item, true);

				item->AIBits = FOLLOW;
				item->ItemFlags[3]++;
				creature->ReachedGoal = false;
				creature->Enemy = NULL;
				break;
			}

			if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 70 && item->RoomNumber == 70)
			{
				item->RequiredState = GUIDE_STATE_RUN;
				item->SwapMeshFlags |= 0x200000;
				SoundEffect(SFX_TR4_GUIDE_SCARE, &item->Position, 0);
			}
		}
		else if (enemy->Position.yRot - item->Position.yRot <= ANGLE(2.0f))
		{
			if (enemy->Position.yRot - item->Position.yRot < -ANGLE(2.0f))
				item->Position.yRot -= ANGLE(2.0f);
		}
		else
			item->Position.yRot += ANGLE(2.0f);

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
			creature->Enemy = NULL;
			break;
		}
		if (enemy->Flags == 42)
		{
			TestTriggers(item, true);

			item->AIBits = FOLLOW;
			item->ItemFlags[3]++;
			creature->ReachedGoal = false;
			creature->Enemy = NULL;
		}
		else if (item->TriggerFlags <= 999)
			item->TargetState = GUIDE_STATE_IDLE;
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
		MoveCreature3DPos(&item->Position, &enemy->Position, 15, enemy->Position.yRot - item->Position.yRot, ANGLE(10.0f));

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
