#include "newobjects.h"
#include "items.h"
#include "box.h"
#include "effect2.h"
#include "sphere.h"
#include "lot.h"
#include "effects.h"
#include "tomb4fx.h"
#include "setup.h"
#include "level.h"
#include "lara.h"
#include "sound.h"

BITE_INFO guideBiteInfo1 = { 0, 20, 200, 18 };
BITE_INFO guideBiteInfo2 = { 30, 80, 50, 15 };

void InitialiseGuide(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	ClearItem(itemNumber);

	item->animNumber = Objects[item->objectNumber].animIndex + 4;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
	item->swapMeshFlags = 0x40000;
}

void GuideControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	ObjectInfo* obj = &Objects[item->objectNumber];

	short angle = 0;
	short tilt = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;

	if (item->itemFlags[1] == 2)
	{
		PHD_VECTOR pos;

		pos.x = guideBiteInfo1.x;
		pos.y = guideBiteInfo1.y;
		pos.z = guideBiteInfo1.z;

		GetJointAbsPosition(item, &pos, guideBiteInfo1.meshNum);

		AddFire(pos.x, pos.y, pos.z, 0, item->roomNumber, 0);
		SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
		GrenadeLauncherSpecialEffect1(pos.x, pos.y - 40, pos.z, -1, 7);

		short random = GetRandomControl();
		TriggerDynamicLight(pos.x, pos.y, pos.z, 15, 255 - ((random >> 4) & 0x1F), 192 - ((random >> 6) & 0x1F), random & 0x3F);

		if (item->animNumber == obj->animIndex + 61)
		{
			if (item->frameNumber > Anims[item->animNumber].frameBase + 32 &&
				item->frameNumber < Anims[item->animNumber].frameBase + 42)
			{
				GrenadeLauncherSpecialEffect1(
					(random & 0x3F) + pos.x - 32,
					((random >> 3) & 0x3F) + pos.y - 128,
					pos.z + ((random >> 6) & 0x3F) - 32,
					-1,
					1);
			}
		}
	}

	item->aiBits = FOLLOW;

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
		laraInfo.distance = dx * dx + dz * dz;

	dx = abs(dx);
	dz = abs(dz);

	int dy = item->pos.yPos - LaraItem->pos.yPos;
	short rot2 = 0;

	if (dx <= dz)
		laraInfo.xAngle = phd_atan(dz + (dx >> 1), dy);
	else
		laraInfo.xAngle = phd_atan(dx + (dz >> 1), dy);

	ITEM_INFO* foundEnemy = NULL;

	if (item->currentAnimState < 4 || item->currentAnimState == 31)
	{
		int minDistance = 0x7FFFFFFF;
		CREATURE_INFO* baddie = &BaddieSlots[0];

		for (int i = 0; i < NUM_SLOTS; i++)
		{
			baddie = &BaddieSlots[i];

			if (baddie->itemNum == NO_ITEM || baddie->itemNum == itemNumber)
				continue;

			ITEM_INFO* currentItem = &Items[baddie->itemNum];
			if (currentItem->objectNumber != ID_GUIDE &&
				abs(currentItem->pos.yPos - item->pos.yPos) <= 512)
			{
				dx = currentItem->pos.xPos - item->pos.xPos;
				dy = currentItem->pos.yPos - item->pos.yPos;
				dz = currentItem->pos.zPos - item->pos.zPos;

				if (dx > 32000 || dx < -32000 || dz > 32000 || dz < -32000)
					distance = 0x7FFFFFFF;
				else
					distance = dx * dx + dz * dz;

				if (distance < minDistance && distance < SQUARE(2048) &&
					(dy < 256 || laraInfo.distance < SQUARE(2048) ||
						currentItem->objectNumber == ID_DOG))
				{
					foundEnemy = currentItem;
					minDistance = distance;
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
	short frameNumber;
	short random;

	printf("Guide state: %d\n", item->currentAnimState);

	switch (item->currentAnimState)
	{
	case 1:
		creature->LOT.isJumping = false;
		creature->flags = 0;
		creature->maximumTurn = 0;
		joint2 = info.angle >> 1;

		if (laraInfo.ahead)
		{
			joint0 = laraInfo.angle >> 1;
			joint1 = laraInfo.xAngle >> 1;
			joint2 = laraInfo.angle >> 1;
		}
		else if (info.ahead)
		{
			joint0 = info.angle >> 1;
			joint1 = info.xAngle >> 1;
			joint2 = info.angle >> 1;
		}

		/*if (Objects[ID_WRAITH1].loaded & 0x10000)
		{
			if (item->itemFlags[3] == 5)
				item->goalAnimState = 2;

			if (item->itemFlags[3] == 5 || item->itemFlags[3] == 6)
			{
				CreatureTilt((int)item, tilt);
				CreatureJoint((int)item, 0, joint0);
				CreatureJoint((int)item, 1, joint1);
				CreatureJoint((int)item, 2, joint2);
				CreatureJoint((int)item, 3, joint1);

				CreatureAnimation(itemNumber, angle, 0);

				return;
			}
		}*/

		if (item->requiredAnimState)
		{
			item->goalAnimState = item->requiredAnimState;
		}
		else if (Lara.location >= item->itemFlags[3] || item->itemFlags[1] != 2)
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
						item->goalAnimState = 31;
					}
				}
				else if (/*true ||*/ enemy != LaraItem || info.distance > SQUARE(2048))
				{
					item->goalAnimState = 2;
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
						item->goalAnimState = 37;
						item->requiredAnimState = 37;

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

					case 0x3Eu:
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
			item->goalAnimState = 1;
		}

		break;

	case 2:
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

		if (item->itemFlags[1] == 1)
		{
			item->goalAnimState = 1;
			item->requiredAnimState = 11; // Ignite torch
		}
		else if (creature->reachedGoal)
		{
			if (!creature->enemy->flags)
			{
				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->aiBits = FOLLOW;
				item->itemFlags[3]++;

				break;
			}
			item->goalAnimState = 1;
		}
		else
		{
			if (Lara.location >= item->itemFlags[3])
			{
				if (!foundEnemy || info.distance >= 0x200000 && (item->swapMeshFlags & 0x40000 || info.distance >= 9437184))
				{
					if (creature->enemy == LaraItem)
					{
						if (info.distance >= 0x400000)
						{
							if (info.distance > 0x1000000)
							{
								item->goalAnimState = 3;
							}
						}
						else
						{
							item->goalAnimState = 1;
						}
					}
					else if (Lara.location > item->itemFlags[3] && laraInfo.distance > 0x400000)
					{
						item->goalAnimState = 3;
					}
				}
				else
				{
					item->goalAnimState = 1;
				}
			}
			else
			{
				item->goalAnimState = 1;
			}
		}

		break;

	case 3:
		if (info.ahead)
		{
			joint2 = info.angle;
		}

		creature->maximumTurn = ANGLE(11);
		tilt = angle / 2;

		if (info.distance < SQUARE(2048) || Lara.location < item->itemFlags[3])
		{
			item->goalAnimState = 1;
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
			item->goalAnimState = 1;
		}
		else if (foundEnemy && (info.distance < 0x200000 || !(item->swapMeshFlags & 0x40000) && info.distance < SQUARE(3072)))
		{
			item->goalAnimState = 1;
			break;
		}

		break;

	case 11:
		// Ignite torch
		pos1.x = guideBiteInfo2.x;
		pos1.y = guideBiteInfo2.y;
		pos1.z = guideBiteInfo2.z;

		GetJointAbsPosition(item, &pos1, guideBiteInfo2.meshNum);

		frameNumber = item->frameNumber - Anims[item->animNumber].frameBase;
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
						GrenadeLauncherSpecialEffect1(
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
							192 - ((random >> 4) & 0x1F),
							128 - ((random >> 6) & 0x1F),
							random & 0x1F);

						item->itemFlags[1] = 2;
					}
				}
				else
				{
					TriggerMetalSparks(pos1.x, pos1.y, pos1.z, -1, -1, 0, 1);
					TriggerDynamicLight(pos1.x, pos1.y, pos1.z, 10, random & 0x1F, 96 - ((random >> 6) & 0x1F), 128 - ((random >> 4) & 0x1F));
				}
			}
			else
			{
				TriggerDynamicLight(pos1.x - 32, pos1.y - 64, pos1.z - 32, 10, 192 - ((random >> 4) & 0x1F), 128 - ((random >> 6) & 0x1F), random & 0x1F);

				GrenadeLauncherSpecialEffect1(
					(random & 0x3F) + pos1.x - 64,
					((random >> 5) & 0x3F) + pos1.y - 96,
					((random >> 10) & 0x3F) + pos1.z - 64,
					-1,
					7);
			}
		}
		else
		{
			TriggerDynamicLight(pos1.x, pos1.y, pos1.z, 10, random & 0x1F, 96 - ((random >> 6) & 0x1F), 128 - ((random >> 4) & 0x1F));
			TriggerMetalSparks(pos1.x, pos1.y, pos1.z, -1, -1, 0, 1);
		}

		break;

	case 22:
		creature->maximumTurn = 0;

		if (laraInfo.angle < -256)
		{
			item->pos.yRot -= 399;
		}

		break;

	case 31:
		if (info.ahead)
		{
			joint0 = info.angle >> 1;
			joint2 = info.angle >> 1;
			joint1 = info.xAngle >> 1;
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
				if (item->frameNumber > Anims[item->animNumber].frameBase + 15 &&
					item->frameNumber < Anims[item->animNumber].frameBase + 26)
				{
					dx = abs(enemy->pos.xPos - item->pos.xPos);
					dy = abs(enemy->pos.yPos - item->pos.yPos);
					dz = abs(enemy->pos.zPos - item->pos.zPos);

					if (dx <= 512 && dy <= 512 && dz >= 512)
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
			if (deltaAngle <= 364)
			{
				if (deltaAngle < -364)
					item->pos.yRot -= 364;
			}
			else
			{
				item->pos.yRot += 364;
			}
		}

		if (item->requiredAnimState == 43)
		{
			item->goalAnimState = 43;
		}
		else
		{
			if (item->animNumber != obj->animIndex + 57
				&& item->frameNumber == Anims[item->animNumber].frameEnd - 20)
			{
				floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &item->roomNumber);
				GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
				TestTriggers(TriggerIndex, 1, 0);

				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->aiBits = FOLLOW;
				item->itemFlags[3]++;

				break;
			}
		}

		break;

	case 37:
		if (item->frameNumber == Anims[item->animNumber].frameBase)
		{
			someFlag = true;

			item->pos.xPos = enemy->pos.xPos;
			item->pos.yPos = enemy->pos.yPos;
			item->pos.zPos = enemy->pos.zPos;
			item->pos.xRot = enemy->pos.xRot;
			item->pos.yRot = enemy->pos.yRot;
			item->pos.zRot = enemy->pos.zRot;
		}
		else if (item->frameNumber == Anims[item->animNumber].frameBase + 35)
		{
			item->swapMeshFlags &= 0xFFFBFFFF;

			ROOM_INFO* room = &Rooms[item->roomNumber];
			ITEM_INFO* currentItem = NULL;

			short currentitemNumber = room->itemNumber;
			while (currentitemNumber != NO_ITEM)
			{
				currentItem = &Items[currentitemNumber];

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
		if (item->frameNumber == Anims[item->animNumber].frameBase)
		{
			item->pos.xPos = enemy->pos.xPos;
			item->pos.yPos = enemy->pos.yPos;
			item->pos.zPos = enemy->pos.zPos;
		}
		else
		{
			if (item->frameNumber == Anims[item->animNumber].frameBase + 42)
			{

				floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &item->roomNumber);
				GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
				TestTriggers(TriggerIndex, 1, 0);

				item->pos.yRot = enemy->pos.yRot;
				//goto LABEL_222;

				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->aiBits = FOLLOW;
				item->itemFlags[3]++;

				break;
			}
			if (item->frameNumber < Anims[item->animNumber].frameBase + 42)
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
		if (item->frameNumber >= Anims[item->animNumber].frameBase + 20)
		{
			if (item->frameNumber == Anims[item->animNumber].frameBase + 20)
			{
				item->goalAnimState = 1;

				floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &item->roomNumber);
				GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
				TestTriggers(TriggerIndex, 1, 0);

				// LABEL_222
				creature->reachedGoal = false;
				creature->enemy = NULL;
				item->aiBits = FOLLOW;
				item->itemFlags[3]++;

				break;
			}

			if (item->frameNumber == Anims[item->animNumber].frameBase + 70 && item->roomNumber == 70)
			{
				item->requiredAnimState = 3;
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
			floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &item->roomNumber);
			GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
			TestTriggers(TriggerIndex, 1, 0);

			//LABEL_222:
			creature->reachedGoal = false;
			creature->enemy = NULL;
			item->aiBits = FOLLOW;
			item->itemFlags[3]++;
		}
		else if (item->triggerFlags <= 999)
		{
			item->goalAnimState = 1;
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