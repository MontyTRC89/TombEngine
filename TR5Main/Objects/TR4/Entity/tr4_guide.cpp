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

	item->animNumber = Objects[item->objectNumber].animIndex + 4;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->goalAnimState = STATE_GUIDE_STOP;
	item->currentAnimState = STATE_GUIDE_STOP;
	
	if (Objects[ID_WRAITH1].loaded)
	{
		item->swapMeshFlags = 0;
		item->itemFlags[1] = 2;
	}
	else
	{
		item->swapMeshFlags = 0x40000;
	}
}

void GuideControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	short angle = 0;
	short tilt = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;

	// Ignite torch
	if (item->itemFlags[1] == 2)
	{
		PHD_VECTOR pos;

		pos.x = guideBiteInfo1.x;
		pos.y = guideBiteInfo1.y;
		pos.z = guideBiteInfo1.z;

		GetJointAbsPosition(item, &pos, guideBiteInfo1.meshNum);

		AddFire(pos.x, pos.y, pos.z, 0, item->roomNumber, 0);
		SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
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

		if (item->animNumber == obj->animIndex + 61)
		{
			if (item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 32 &&
				item->frameNumber < g_Level.Anims[item->animNumber].frameBase + 42)
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

	item->aiBits = FOLLOW;
	item->hitPoints = NOT_TARGETABLE;

	GetAITarget(creature);

	AI_INFO info;
	AI_INFO laraInfo;

	int dx = LaraItem->pos.xPos - item->pos.xPos;
	int dz = LaraItem->pos.zPos - item->pos.zPos;

	laraInfo.angle = phd_atan(dz, dx) - item->pos.yRot;

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

	int dy = item->pos.yPos - LaraItem->pos.yPos;
	short rot2 = 0;

	if (dx <= dz)
		laraInfo.xAngle = phd_atan(dz + (dx / 2), dy);
	else
		laraInfo.xAngle = phd_atan(dx + (dz / 2), dy);

	ITEM_INFO* foundEnemy = NULL;

	if (!Objects[ID_WRAITH1].loaded)
	{
		if (item->currentAnimState < 4
			|| item->currentAnimState == STATE_GUIDE_TORCH_ATTACK)
		{
			int minDistance = 0x7FFFFFFF;

			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				CREATURE_INFO* baddie = ActiveCreatures[i];

				if (baddie->itemNum == NO_ITEM || baddie->itemNum == itemNumber)
					continue;

				ITEM_INFO* currentItem = &g_Level.Items[baddie->itemNum];
				if (currentItem->objectNumber != ID_GUIDE &&
					abs(currentItem->pos.yPos - item->pos.yPos) <= 512)
				{
					dx = currentItem->pos.xPos - item->pos.xPos;
					dy = currentItem->pos.yPos - item->pos.yPos;
					dz = currentItem->pos.zPos - item->pos.zPos;

					if (dx > 32000 || dx < -32000 || dz > 32000 || dz < -32000)
						distance = 0x7FFFFFFF;
					else
						distance = SQUARE(dx) + SQUARE(dz);

					if (distance < minDistance 
						&& distance < SQUARE(2048) 
						&& (abs(dy) < 256 
							|| laraInfo.distance < SQUARE(2048) 
							|| currentItem->objectNumber == ID_DOG))
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

	switch (item->currentAnimState)
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
			if (item->itemFlags[3] == 5)
				item->goalAnimState = STATE_GUIDE_WALK;

			if (item->itemFlags[3] == 5 || item->itemFlags[3] == 6)
			{
				break;
			}
		}

		if (item->requiredAnimState)
		{
			item->goalAnimState = item->requiredAnimState;
		}
		else if (Lara.location >= item->itemFlags[3] 
			|| item->itemFlags[1] != 2)
		{
			if (!creature->reachedGoal || foundEnemy)
			{
				if (item->swapMeshFlags == 0x40000)
				{
					item->goalAnimState = 40;
				}
				else if (foundEnemy && info.distance < SQUARE(1024))
				{
					if (info.bite)
					{
						item->goalAnimState = STATE_GUIDE_TORCH_ATTACK;
					}
				}
				else if (enemy != LaraItem || info.distance > SQUARE(2048))
				{
					item->goalAnimState = STATE_GUIDE_WALK;
				}
			}
			else
			{
				if (!enemy->flags)
				{
					creature->reachedGoal = false;
					creature->enemy = NULL;
					item->aiBits = FOLLOW;
					item->itemFlags[3]++;

					break;
				}

				if (info.distance <= SQUARE(128))
				{
					switch (enemy->flags)
					{
					case 0x02:
						item->goalAnimState = 38;
						item->requiredAnimState = 38;

						break;

					case 0x20:
						item->goalAnimState = STATE_GUIDE_PICKUP_TORCH;
						item->requiredAnimState = STATE_GUIDE_PICKUP_TORCH;

						break;

					case 0x28:
						if (laraInfo.distance < SQUARE(2048))
						{
							item->goalAnimState = 39;
							item->requiredAnimState = 39;
						}

						break;

					case 0x10:
						if (laraInfo.distance < SQUARE(2048))
						{
							// Ignite torch
							item->goalAnimState = 36;
							item->requiredAnimState = 36;
						}

						break;

					case 0x04:
						if (laraInfo.distance < SQUARE(2048))
						{
							item->goalAnimState = 36;
							item->requiredAnimState = 43;
						}

						break;

					case 0x3E:
						item->status = ITEM_INVISIBLE;
						RemoveActiveItem(itemNumber);
						DisableBaddieAI(itemNumber);

						break;

					}
				}
				else
				{
					creature->maximumTurn = 0;
					item->requiredAnimState = 42 - (info.ahead != 0);
				}
			}
		}
		else
		{
			item->goalAnimState = STATE_GUIDE_STOP;
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

		if (Objects[ID_WRAITH1].loaded && item->itemFlags[3] == 5)
		{
			item->itemFlags[3] = 6;
			item->goalAnimState = STATE_GUIDE_STOP;
		}
		else if (item->itemFlags[1] == 1)
		{
			item->goalAnimState = STATE_GUIDE_STOP;
			item->requiredAnimState = STATE_GUIDE_IGNITE_TORCH; 
		}
		else if (creature->reachedGoal)
		{
			if (!enemy->flags)
			{
				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->aiBits = FOLLOW;
				item->itemFlags[3]++;

				break;
			}
			item->goalAnimState = STATE_GUIDE_STOP;
		}
		else
		{
			if (Lara.location >= item->itemFlags[3])
			{
				if (!foundEnemy 
					|| info.distance >= 0x200000
					&& (item->swapMeshFlags & 0x40000
						|| info.distance >= SQUARE(3072)))
				{
					if (creature->enemy == LaraItem)
					{
						if (info.distance >= SQUARE(2048))
						{
							if (info.distance > SQUARE(4096))
							{
								item->goalAnimState = STATE_GUIDE_RUN;
							}
						}
						else
						{
							item->goalAnimState = STATE_GUIDE_STOP;
						}
					}
					else if (Lara.location > item->itemFlags[3]
						&& laraInfo.distance > SQUARE(2048))
					{
						item->goalAnimState = STATE_GUIDE_RUN;
					}
				}
				else
				{
					item->goalAnimState = STATE_GUIDE_STOP;
				}
			}
			else
			{
				item->goalAnimState = STATE_GUIDE_STOP;
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
			|| Lara.location < item->itemFlags[3])
		{
			item->goalAnimState = STATE_GUIDE_STOP;
			break;
		}
		if (creature->reachedGoal)
		{
			if (!enemy->flags)
			{
				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->aiBits = FOLLOW;
				item->itemFlags[3]++;

				break;
			}
			item->goalAnimState = STATE_GUIDE_STOP;
		}
		else if (foundEnemy && 
			(info.distance < 0x200000 
				|| !(item->swapMeshFlags & 0x40000) 
				&& info.distance < SQUARE(3072)))
		{
			item->goalAnimState = STATE_GUIDE_STOP;
			break;
		}

		break;

	case STATE_GUIDE_IGNITE_TORCH:
		// Ignite torch
		pos1.x = guideBiteInfo2.x;
		pos1.y = guideBiteInfo2.y;
		pos1.z = guideBiteInfo2.z;

		GetJointAbsPosition(item, &pos1, guideBiteInfo2.meshNum);

		frameNumber = item->frameNumber - g_Level.Anims[item->animNumber].frameBase;
		random = GetRandomControl();

		if (frameNumber == 32)
		{
			item->swapMeshFlags |= 0x8000;
		}
		else if (frameNumber == 216)
		{
			item->swapMeshFlags &= 0x7FFF;
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

						item->itemFlags[1] = 2;
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
			item->pos.yRot -= 399;
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

		if (!creature->flags)
		{
			if (enemy)
			{
				if (item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 15 &&
					item->frameNumber < g_Level.Anims[item->animNumber].frameBase + 26)
				{
					dx = abs(enemy->pos.xPos - item->pos.xPos);
					dy = abs(enemy->pos.yPos - item->pos.yPos);
					dz = abs(enemy->pos.zPos - item->pos.zPos);

					if (dx < 512 && dy < 512 && dz < 512)
					{
						enemy->hitPoints -= 20;

						if (enemy->hitPoints <= 0)
						{
							item->aiBits = FOLLOW;
						}

						enemy->hitStatus = true;
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
			item->pos.yRot += 399;
		}

		break;

	case 36:
	case 43:
		if (enemy)
		{
			short deltaAngle = enemy->pos.yRot - item->pos.yRot;
			if (deltaAngle < -ANGLE(2))
				item->pos.yRot -= ANGLE(2);
			else if (deltaAngle > ANGLE(2))
				item->pos.yRot = ANGLE(2);
		}

		if (item->requiredAnimState == 43)
		{
			item->goalAnimState = 43;
		}
		else
		{
			if (item->animNumber != obj->animIndex + 57
				&& item->frameNumber == g_Level.Anims[item->animNumber].frameEnd - 20)
			{
				TestTriggers(item, true);

				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->aiBits = FOLLOW;
				item->itemFlags[3]++;
				item->goalAnimState = STATE_GUIDE_STOP;

				break;
			}
		}

		break;

	case STATE_GUIDE_PICKUP_TORCH:
		if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
		{
			someFlag = true;

			item->pos.xPos = enemy->pos.xPos;
			item->pos.yPos = enemy->pos.yPos;
			item->pos.zPos = enemy->pos.zPos;
			item->pos.xRot = enemy->pos.xRot;
			item->pos.yRot = enemy->pos.yRot;
			item->pos.zRot = enemy->pos.zRot;
		}
		else if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 35)
		{
			item->swapMeshFlags &= 0xFFFBFFFF;

			ROOM_INFO* room = &g_Level.Rooms[item->roomNumber];
			ITEM_INFO* currentItem = NULL;

			short currentitemNumber = room->itemNumber;
			while (currentitemNumber != NO_ITEM)
			{
				currentItem = &g_Level.Items[currentitemNumber];

				if (currentItem->objectNumber >= ID_ANIMATING1
					&& currentItem->objectNumber <= ID_ANIMATING15
					&& trunc(item->pos.xPos) == trunc(currentItem->pos.xPos)
					&& trunc(item->pos.zPos) == trunc(currentItem->pos.zPos))
				{
					break;
				}

				currentitemNumber = currentItem->nextItem;
			}

			if (currentItem != NULL)
				currentItem->meshBits = 0xFFFFFFFD;
		}

		item->itemFlags[1] = 1;
		if (someFlag)
		{
			creature->reachedGoal = false;
			creature->enemy = NULL;
			item->aiBits = FOLLOW;
			item->itemFlags[3]++;
		}

		break;

	case 38:
		if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
		{
			item->pos.xPos = enemy->pos.xPos;
			item->pos.yPos = enemy->pos.yPos;
			item->pos.zPos = enemy->pos.zPos;
		}
		else
		{
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 42)
			{

				TestTriggers(item, true);
				item->pos.yRot = enemy->pos.yRot;
				
				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->aiBits = FOLLOW;
				item->itemFlags[3]++;

				break;
			}
			else if (item->frameNumber < g_Level.Anims[item->animNumber].frameBase + 42)
			{
				if (enemy->pos.yRot - item->pos.yRot <= ANGLE(2))
				{
					if (enemy->pos.yRot - item->pos.yRot < -ANGLE(2))
					{
						item->pos.yRot -= ANGLE(2);
					}
				}
				else
				{
					item->pos.yRot += ANGLE(2);
				}
			}
		}

		break;

	case 39:
		if (item->frameNumber >= g_Level.Anims[item->animNumber].frameBase + 20)
		{
			if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 20)
			{
				item->goalAnimState = STATE_GUIDE_STOP;

				TestTriggers(item, true);

				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->aiBits = FOLLOW;
				item->itemFlags[3]++;

				break;
			}

			if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 70 && item->roomNumber == 70)
			{
				item->requiredAnimState = STATE_GUIDE_RUN;
				item->swapMeshFlags |= 0x200000;
				SoundEffect(SFX_TR4_GUIDE_SCARE, &item->pos, 0);
			}
		}
		else if (enemy->pos.yRot - item->pos.yRot <= ANGLE(2))
		{
			if (enemy->pos.yRot - item->pos.yRot < -ANGLE(2))
			{
				item->pos.yRot -= ANGLE(2);
			}
		}
		else
		{
			item->pos.yRot += ANGLE(2);
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

		if (!enemy->flags)
		{
			creature->reachedGoal = false;
			creature->enemy = NULL;
			item->aiBits = FOLLOW;
			item->itemFlags[3]++;

			break;
		}
		if (enemy->flags == 42)
		{
			TestTriggers(item, true);
			
			creature->reachedGoal = false;
			creature->enemy = NULL;
			item->aiBits = FOLLOW;
			item->itemFlags[3]++;
		}
		else if (item->triggerFlags <= 999)
		{
			item->goalAnimState = STATE_GUIDE_STOP;
		}
		else
		{
			KillItem(itemNumber);
			DisableBaddieAI(itemNumber);
			item->flags |= 1;
		}

		break;

	case 41:
	case 42:
		creature->maximumTurn = 0;
		MoveCreature3DPos(&item->pos, &enemy->pos, 15, enemy->pos.yRot - item->pos.yRot, ANGLE(10));

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