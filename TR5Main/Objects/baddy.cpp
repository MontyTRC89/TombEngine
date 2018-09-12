#include "objects.h"
#include "..\Global\global.h"
#include "..\Game\Box.h"
#include "..\Game\items.h"
#include "..\Game\lot.h"
#include "..\Game\control.h"
#include "..\Game\effects.h"
#include "..\Game\draw.h"
#include "..\Game\sphere.h"
#include "..\Game\effect2.h"
#include "..\Game\people.h"

BITE_INFO baddyBite = { 0, -16, 200, 11 };
BITE_INFO baddyBite2 = { 0, 0, 0, 15 };

void __cdecl ClampRotation(PHD_3DPOS *pos, __int16 angle, __int16 rot)
{
	if (angle <= rot)
	{
		if (angle >= -rot)
		{
			pos->yRot += angle;
		}
		else
		{
			pos->yRot -= rot;
		}
	}
	else
	{
		pos->yRot += rot;
	}
}

void __cdecl InitialiseBaddy(__int16 itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	
	ClearItem(itemNum);

	__int16 objectNumber = (Objects[ID_BADDY2].loaded ? ID_BADDY2 : ID_BADDY1);
	if (item->objectNumber == ID_BADDY1)
	{
		item->swapMeshFlags = 0x7FC010;
		item->meshBits = 0xFF81FFFF;
		item->itemFlags[2] = 24;
	}
	else
	{
		item->swapMeshFlags = 0x880;
		item->meshBits = -1;
		item->itemFlags[2] = 0;
	}
	
	item->itemFlags[1] = -1;

	__int16 ocb = item->triggerFlags;

	if (ocb > 9 && ocb < 20)
	{
		item->itemFlags[2] += 24;
		item->triggerFlags = item->triggerFlags % 1000 - 10;
		ocb -= 10;
	}
	
	if (!ocb || ocb > 4 && ocb < 7)
	{
		item->animNumber = Objects[objectNumber].animIndex + 18;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->goalAnimState = 0;
		item->currentAnimState = 0;

		return;
	}

	if (ocb == 1)
	{
		item->animNumber = Objects[objectNumber].animIndex + 47;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->goalAnimState = 24;
		item->currentAnimState = 24;

		return;
	}

	if (ocb == 2)
	{
		item->animNumber = Objects[objectNumber].animIndex + 24;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->goalAnimState = 23;
		item->currentAnimState = 23;

		return;
	}
	
	if (ocb == 3)
	{
		item->animNumber = Objects[objectNumber].animIndex + 29;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->goalAnimState = 26;
		item->currentAnimState = 26;

		return;
	}

	if (ocb == 4)
	{
		item->animNumber = Objects[objectNumber].animIndex + 62;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->goalAnimState = 39;
		item->currentAnimState = 39;
		item->pos.xPos += SIN(item->pos.yRot) * 1024 >> 14;;
		item->pos.zPos += COS(item->pos.yRot) * 1024 >> 14;;

		return;
	}

	if (ocb > 100)
	{
		item->animNumber = Objects[objectNumber].animIndex + 29;
		item->frameNumber = Anims[item->animNumber].frameBase;
		item->goalAnimState = 26;
		item->currentAnimState = 26;
		item->pos.xPos += SIN(item->pos.yRot) * 1024 >> 14;;
		item->pos.zPos += COS(item->pos.yRot) * 1024 >> 14;;
		item->itemFlags[3] = ocb;

		return;
	}
	
	item->frameNumber = Anims[item->animNumber].frameBase;
}

void __cdecl BaddyControl(__int16 itemNum)
{
	
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	ITEM_INFO* enemyItem = creature->enemy;

	printf("Anim: %d Frame: %d\n", item->animNumber, item->frameNumber);

	__int16 tilt = 0;
	__int16 angle = 0;
	__int16 joint1 = 0;
	__int16 joint2 = 0;
	__int16 joint3 = 0;
	__int16 objectNumber = (Objects[ID_BADDY2].loaded ? ID_BADDY2 : ID_BADDY1);

	__int32 roll = false;
	__int32 jump = false;
	__int32 someFlag3 = false;

	/*if (item->triggerFlags % 1000)
	{
		creature->LOT.isJumping = true;
		creature->maximumTurn = 0;
		if (item->triggerFlags % 1000 > 100)
		{
			item->itemFlags[0] = -80;
			SameZoneAIObject(creature, ID_AI_X1);
		}
		item->triggerFlags = 1000 * (item->triggerFlags / 1000);
	}*/

	// Test if baddy can jump?
	__int32 x = item->pos.xPos;
	__int32 y = item->pos.yPos;
	__int32 z = item->pos.zPos;

	__int32 dx = 942 * SIN(item->pos.yRot) >> 14;
	__int32 dz = 942 * COS(item->pos.yRot) >> 14;

	x += dx;
	z += dz;

	__int16 roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	__int32 height1 = TrGetHeight(floor, x, y, z);

	x += dx;
	z += dz;

	roomNumber = item->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	__int32 height2 = TrGetHeight(floor, x, y, z);

	x += dx;
	z += dz;

	roomNumber = item->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	__int32 height3 = TrGetHeight(floor, x, y, z);

	__int32 height = 0;
	bool canJump2sectors = true;
	if (enemyItem && item->boxNumber == enemyItem->boxNumber
		|| y >= height1 - 384
		|| y >= height2 + 256
		|| y <= height2 - 256)
	{
		height = height2;
		canJump2sectors = false;
	}

	bool canJump3sectors = true;
	if (enemyItem && item->boxNumber == enemyItem->boxNumber
		|| y >= height1 - 384
		|| y >= height - 384
		|| y >= height3 + 256
		|| y <= height3 - 256)
	{
		canJump3sectors = false;
	}

	CREATURE_INFO* currentCreature;

	if (item->itemFlags[1] == item->roomNumber ||
		Rooms[item->roomNumber].flippedRoom == -1)
	{
		currentCreature = creature;
	}
	else
	{
		currentCreature = creature;

		// TODO: picking
	}

	item->itemFlags[1] = item->roomNumber;

	// Handle baddy firing
	if (item->firedWeapon)
	{
		PHD_VECTOR pos;

		GetJointAbsPosition(item, &pos, baddyBite.meshNum);
		TriggerDynamics(pos.x, pos.y, pos.z, 4 * item->firedWeapon + 8, 24, 16, 4);
		item->firedWeapon--;
	}

	if (item->hitPoints <= 0)
	{
		currentCreature->LOT.isMonkeying = false;

		roomNumber = item->roomNumber;
		floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		height = TrGetHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		item->floor = height;

		switch (item->currentAnimState)
		{
		case 32:
			item->gravityStatus = true;
			currentCreature->LOT.isMonkeying = false;
			if (item->pos.yPos >= item->floor)
			{
				item->pos.yPos = item->floor;
				item->fallspeed = 0;
				item->gravityStatus = false;
			}
			break;

		case 35:
			item->goalAnimState = 36;
			item->gravityStatus = false;
			break;

		case 36:
			item->gravityStatus = true;
			if (item->pos.yPos >= item->floor)
			{
				item->pos.yPos = item->floor;
				item->fallspeed = 0;
				item->gravityStatus = false;
				item->goalAnimState = 37;
			}
			break;

		case 37:
			item->pos.yPos = item->floor;
			break;

		case 18:
		case 19:
		case 20:
			item->animNumber = Objects[objectNumber].animIndex + 59;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 35;
			item->speed = 0;
			break;

		default:
			currentCreature->LOT.isJumping = true;
			item->animNumber = Objects[objectNumber].animIndex + 45;
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 32;

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
			laraInfo.angle = ATAN(dz, dx);
			laraInfo.ahead = true;

			if (laraInfo.angle <= -16384 || laraInfo.angle >= 16384)
				laraInfo.ahead = false;

			laraInfo.distance = dx * dx + dz * dz;
		}

		GetCreatureMood(item, &info, VIOLENT);

		// Bike handling
		//if (Lara.bike != -1 && info.bite)
		//	currentCreature->mood == MOOD_TYPE::ESCAPE_MOOD;

		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, currentCreature->maximumTurn);

		//currentCreature->enemy = LaraItem;

		// Is baddy alerted?
		if (item->hitStatus || laraInfo.distance < 0x100000 ||
			TargetVisible(item, &laraInfo) &&
			abs(LaraItem->pos.yPos - item->pos.yPos) < 1024)
		{
			currentCreature->alerted = true;
		}

		if (item != Lara.target || laraInfo.distance <= 942 ||
			laraInfo.angle <= -10240 || laraInfo.angle >= 10240)
		{
			roll = false;
			jump = false;
		}
		
			dx = 942 * SIN(item->pos.yRot + 8192) >> 14;
			dz = 942 * COS(item->pos.yRot + 8192) >> 14;

			x = item->pos.xPos + dx;
			y = item->pos.yPos;
			z = item->pos.zPos + dz;

			roomNumber = item->roomNumber;
			floor = GetFloor(x, y, z, &roomNumber);
			__int32 height4 = TrGetHeight(floor, x, y, z);

			dx = 942 * SIN(item->pos.yRot + 14336) >> 14;
			dz = 942 * COS(item->pos.yRot + 14336) >> 14;

			x = item->pos.xPos + dx;
			y = item->pos.yPos;
			z = item->pos.zPos + dz;

			roomNumber = item->roomNumber;
			floor = GetFloor(x, y, z, &roomNumber);
			__int32 height5 = TrGetHeight(floor, x, y, z);

			if (abs(height5 - item->pos.yPos) > 256)
				jump = false;
			else
			{
				roll = true;
				if (height4 + 512 >= item->pos.yPos)
					jump = false;
			}

			dx = 942 * SIN(item->pos.yRot - 8192) >> 14;
			dz = 942 * COS(item->pos.yRot - 8192) >> 14;

			x = item->pos.xPos + dx;
			y = item->pos.yPos;
			z = item->pos.zPos + dz;

			roomNumber = item->roomNumber;
			floor = GetFloor(x, y, z, &roomNumber);
			__int32 height6 = TrGetHeight(floor, x, y, z);

			dx = 942 * SIN(item->pos.yRot - 14336) >> 14;
			dz = 942 * COS(item->pos.yRot - 14336) >> 14;

			x = item->pos.xPos + dx;
			y = item->pos.yPos;
			z = item->pos.zPos + dz;

			roomNumber = item->roomNumber;
			floor = GetFloor(x, y, z, &roomNumber);
			__int32 height7 = TrGetHeight(floor, x, y, z);

			if (abs(height7 - item->pos.yPos) > 256 || height6 + 512 >= item->pos.yPos)
			{
				roll = false;
				someFlag3 = false;

				switch (item->currentAnimState)
				{
				case 0:
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

					if (item->swapMeshFlags == 2176
						&& item == Lara.target
						&& laraInfo.ahead
						&& laraInfo.distance > 465124)
					{
						item->goalAnimState = 4;
						break;
					}

					if (Targetable(item, &info) && item->itemFlags[2] > 0)
					{
						if (item->swapMeshFlags == 0x7FC010)
						{
							item->goalAnimState = 31;
							break;
						}

						if (item->swapMeshFlags != 0x7E0880 && item->swapMeshFlags != 2176)
						{
							item->goalAnimState = 10;
							break;
						}

						item->goalAnimState = 0;
						break;
					}

					if (item->aiBits & MODIFY)
					{
						item->goalAnimState = 0;
						if (item->floor > item->pos.yPos + 768)
							item->aiBits &= ~MODIFY;
						break;
					}

					if (canJump2sectors || canJump3sectors)
					{
						currentCreature->maximumTurn = 0;
						item->animNumber = Objects[objectNumber].animIndex + 55;
						item->frameNumber = Anims[item->animNumber].frameBase;
						item->currentAnimState = 33;
						currentCreature->LOT.isJumping = true;

						if (!canJump3sectors)
							item->goalAnimState = 33;
						else
							item->goalAnimState = 38;
						break;
					}

					if (currentCreature->enemy)
					{
						__int16 objNum = currentCreature->enemy->objectNumber;
						if ((objNum == ID_SMALLMEDI_ITEM || objNum == ID_UZI_AMMO_ITEM) && info.distance < 0x40000)
						{
							item->goalAnimState = 25;
							item->requiredAnimState = 27;
							break;
						}
					}

					if (item->swapMeshFlags == 0x7FC010 && item->itemFlags[2] < 1)
					{
						item->goalAnimState = 11;
						break;
					}

					if (currentCreature->monkeyAhead)
					{
						floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
						height = TrGetHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
						if (GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) == height - 1536)
						{
							if (item->swapMeshFlags == 0x7FC800)
							{
								item->goalAnimState = 18;
								break;
							}
							if (item->swapMeshFlags == 0x7FC010)
							{
								item->goalAnimState = 11;
								break;
							}

							item->goalAnimState = 13;
							break;
						}
					}
					else
					{
						if (roll)
						{
							currentCreature->maximumTurn = 0;
							item->goalAnimState = 23;
							break;
						}
						if (jump)
						{
							currentCreature->maximumTurn = 0;
							item->goalAnimState = 24;
							break;
						}
						if (item->swapMeshFlags == 0x7FC800)
						{
							item->goalAnimState = 12;
							break;
						}
						if (currentCreature->enemy && currentCreature->enemy->hitPoints > 0 && info.distance < 465124)
						{
							if (item->swapMeshFlags == 0x7FC010)
							{
								item->goalAnimState = 11;
							}
							else if (info.distance >= 0x40000)
							{
								item->goalAnimState = 15;
							}
							else if (GetRandomControl() & 1)
							{
								item->goalAnimState = 17;
							}
							else
							{
								item->goalAnimState = 16;
							}
							break;
						}
					}
					item->goalAnimState = 1;
					break;

				case 1:
					currentCreature->LOT.isMonkeying = false;
					currentCreature->LOT.isJumping = false;
					currentCreature->maximumTurn = 1274;
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
						item->goalAnimState = 0;
						break;
					}
					if (canJump2sectors || canJump3sectors)
					{
						currentCreature->maximumTurn = 0;
						item->goalAnimState = 0;
						break;
					}
					if (currentCreature->reachedGoal && currentCreature->monkeyAhead)
					{
						item->goalAnimState = 0;
						break;
					}
					if (item->itemFlags[2] < 1)
					{
						if (item->swapMeshFlags != 0x7E0880 && item->swapMeshFlags != 2176)
						{
							item->goalAnimState = 0;
							break;
						}
					}
					if (info.ahead && info.distance < 0x40000)
					{
						item->goalAnimState = 0;
						break;
					}
					if (info.bite)
					{
						if (info.distance < 465124)
						{
							item->goalAnimState = 0;
							break;
						}
						if (info.distance < 0x100000)
						{
							item->goalAnimState = 29;
							break;
						}
					}
					if (roll || jump)
					{
						item->currentAnimState = 0;
						break;
					}
					if (currentCreature->mood == MOOD_TYPE::ATTACK_MOOD &&
						!(currentCreature->jumpAhead) &&
						info.distance > 0x100000)
					{
						item->goalAnimState = 2;
					}
					break;

				case 2:
					if (info.ahead)
					{
						joint3 = info.angle;
					}
					currentCreature->maximumTurn = 2002;
					tilt = abs(angle) / 2;
					if (objectNumber == ID_BADDY2
						&& item->frameNumber == Anims[item->animNumber].frameBase + 11
						&& height3 == height1
						&& abs(height1 - item->pos.yPos) < 384
						&& (info.angle > -4096 && info.angle < 4096 &&
							info.distance < 9437184
							|| height2 >= height1 + 512))
					{
						item->goalAnimState = 30;
						currentCreature->maximumTurn = 0;
						break;
					}
					if (Targetable(item, &info) && item->itemFlags[2] > 0
						|| canJump2sectors
						|| canJump3sectors
						|| currentCreature->monkeyAhead
						|| item->aiBits & FOLLOW
						|| info.distance < 376996
						|| currentCreature->jumpAhead)
					{
						item->goalAnimState = 0;
						break;
					}
					if (info.distance < 0x100000)
					{
						item->goalAnimState = 1;
						break;
					}
					break;

				case 16:
				case 15:
				case 17:
				case 29:
					if (item->currentAnimState == 16 &&
						info.distance < 0x40000)
					{
						item->goalAnimState = 17;
					}
					if (info.ahead)
					{
						joint1 = info.angle;
						joint2 = info.xAngle;
					}
					currentCreature->maximumTurn = 0;
					if (item->currentAnimState != 15 ||
						item->frameNumber < Anims[item->animNumber].frameBase + 12)
					{
						if (abs(info.angle) >= 1274)
						{
							if (info.angle >= 0)
							{
								item->pos.yRot += 1274;
							}
							else
							{
								item->pos.yRot -= 1274;
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
							if (item->frameNumber > Anims[item->animNumber].frameBase + 13 &&
								item->frameNumber < Anims[item->animNumber].frameBase + 21)
							{
								LaraItem->hitPoints -= 120;
								LaraItem->hitStatus = true;
								CreatureEffect2(
									item,
									&baddyBite2,
									10,
									item->pos.yRot,
									DoBloodSplat);
								currentCreature->flags = 1;
							}
						}
					}
					if (item->frameNumber == Anims[item->animNumber].frameEnd - 1)
					{
						currentCreature->flags = 0;
					}
					break;

				case 19:
					joint2 = 0;
					joint1 = 0;
					currentCreature->maximumTurn = 0;
					currentCreature->flags = 0;

					floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
					height = TrGetHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

					if (laraInfo.ahead
						&& laraInfo.distance < 465124
						&& (LaraItem->currentAnimState > 74
							&& LaraItem->currentAnimState < 80
							|| LaraItem->currentAnimState == 82
							|| LaraItem->currentAnimState == 83))
					{
						item->goalAnimState = 21;
					}
					else if (item->boxNumber != currentCreature->LOT.targetBox
						&& currentCreature->monkeyAhead
						|| GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) != height - 1536)
					{
						item->goalAnimState = 20;
					}
					else
					{
						item->goalAnimState = 22;
						currentCreature->LOT.isMonkeying = false;
						currentCreature->LOT.isJumping = false;
					}
					break;

				case 20:
					joint2 = 0;
					joint1 = 0;
					currentCreature->LOT.isJumping = true;
					currentCreature->LOT.isMonkeying = true;
					currentCreature->flags = 0;
					currentCreature->maximumTurn = 1274;
					if (item->boxNumber == currentCreature->LOT.targetBox ||
						!currentCreature->monkeyAhead)
					{
						floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
						height = TrGetHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
						if (GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) == height - 1536)
						{
							item->goalAnimState = 19;
						}
					}
					if (laraInfo.ahead)
					{
						if (laraInfo.distance < 465124)
						{
							 
							if (LaraItem->currentAnimState > 74 &&
								LaraItem->currentAnimState < 80 ||
								LaraItem->currentAnimState == 82 ||
								LaraItem->currentAnimState == 83)
							{
								item->goalAnimState = 19;
							}
						}
					}
					break;

				case 21:
					currentCreature->maximumTurn = 1274;
					if (currentCreature->flags == someFlag3)
					{
						if (item->touchBits)
						{
							LaraItem->currentAnimState = 28;
							LaraItem->goalAnimState = 28;
							LaraItem->animNumber = 28;
							LaraItem->frameNumber = Anims[LaraItem->frameNumber].frameBase + 9;
							LaraItem->gravityStatus = true;
							LaraItem->speed = 2;
							LaraItem->fallspeed = 1;
							LaraItem->pos.yPos += 192;
							Lara.gunStatus = 0;
							currentCreature->flags = 1;
						}
					}
					break;

				case 23:
				case 24:
					currentCreature->alerted = false;
					currentCreature->maximumTurn = someFlag3;
					item->status = ITEM_ACTIVE;
					break;

				case 26:
					if (item->itemFlags[0] == someFlag3)
					{
						if (currentCreature->enemy)
						{
							if ((currentCreature->enemy->objectNumber == ID_SMALLMEDI_ITEM ||
								currentCreature->enemy->objectNumber == ID_UZI_AMMO_ITEM) &&
								info.distance < 0x40000)
							{
								item->goalAnimState = 27;
								break;
							}
						}
						if (currentCreature->alerted)
						{
							item->goalAnimState = 28;
						}
					}
					else
					{
						if (info.distance >= 465124)
						{
							break;
						}
						item->goalAnimState = 28;
						currentCreature->enemy = NULL;
					}
					break;

				case 27:
					ClampRotation(&item->pos, info.angle, 2002);
					if (item->frameNumber != Anims[item->animNumber].frameBase + 9)
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
					if (currentCreature->enemy->roomNumber == 255 ||
						currentCreature->enemy->status == ITEM_INVISIBLE ||
						currentCreature->enemy->InDrawRoom)
					{
						currentCreature->enemy = NULL;
						break;
					}
					if (currentCreature->enemy->objectNumber == ID_SMALLMEDI_ITEM)
					{
						item->hitPoints += Objects[item->objectNumber].hitPoints >> 1;
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

				case 31:
					currentCreature->maximumTurn = 0;
					if (info.ahead)
					{
						joint1 = info.angle;
						joint2 = info.xAngle;
					}
					ClampRotation(&item->pos, info.angle, 1274);
					if (!Targetable(item, &info) ||
						item->itemFlags[2] < 1)
					{
						item->goalAnimState = 0;
						break;
					}
					item->goalAnimState = 14;
					break;

				case 14:
					if (info.ahead)
					{
						joint1 = info.angle;
						joint2 = info.xAngle;
					}
					ClampRotation(&item->pos, info.angle, 1274);
					if (item->frameNumber >= Anims[item->animNumber].frameBase + 13 ||
						item->frameNumber == Anims[item->animNumber].frameBase + 1)
					{
						break;
					}
					item->firedWeapon = true;
					if (!item->hitStatus)
					{
						item->itemFlags[2]--;
					}
					if (!ShotLara(item, &info, &baddyBite, joint1, 15));
					item->goalAnimState = 0;
					break;

				default:
					break;

				case 11:
					//printf("%d %d\n", item->frameNumber, Anims[item->animNumber].frameBase + 20);
					if (item->frameNumber == Anims[item->animNumber].frameBase + 20)
					{
						item->swapMeshFlags = 0x7FC800;
					}
					break;

				case 10:
					if (item->frameNumber == Anims[item->animNumber].frameBase + 21)
					{
						item->swapMeshFlags = 0x7FC010;
					}
					break;

				case 13:
					if (item->frameNumber == Anims[item->animNumber].frameBase + 22)
					{
						item->swapMeshFlags = 0x7FC800;
					}
					break;

				case 12:
					if (item->frameNumber != Anims[item->animNumber].frameBase + 12)
					{
						break;
					}
					if (item->objectNumber == ID_BADDY1)
					{
						item->swapMeshFlags = 0x7E0880;
					}
					else
					{
						item->swapMeshFlags = 2176;
					}
					break;

				case 8:
					currentCreature->maximumTurn = 0;
					ClampRotation(&item->pos, info.angle, 2002);
					if (laraInfo.distance < 465124 ||
						item != Lara.target)
					{
						item->goalAnimState = 9;
					}
					break;

				case 44:
					if (!WeaponEnemyTimer)
					{
						if ((GetRandomControl() & 0x7F) == 0)
						{
							item->goalAnimState = 0;
						}
					}
					break;

				case 30:
					if (item->animNumber == Objects[objectNumber].animIndex + 4)
					{
						ClampRotation(&item->pos, info.angle, 1274);
						break;
					}
					if (item->frameNumber != Anims[item->animNumber].frameBase + 18)
					{
						break;
					}
					currentCreature->LOT.isJumping = true;
					break;

				case 33:
				case 38:
					if (item->itemFlags[0] >= someFlag3)
					{
						break;
					}
					if (item->animNumber != Objects[objectNumber].animIndex + 55)
					{
						item->itemFlags[0] += 2;
					}
					break;
				}
			}
			roll = true;
		}
		someFlag3 = false;

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint1);
	CreatureJoint(item, 1, joint2);
	CreatureJoint(item, 2, joint3);

	if (item->currentAnimState >= 38 ||
		item->currentAnimState == 33 ||
		item->currentAnimState == 20 ||
		item->currentAnimState == 32 ||
		item->currentAnimState == 30 ||
		item->currentAnimState == 44)
	{
		CreatureAnimation(itemNum, angle, 0);
	}
	else  if (WeaponEnemyTimer <= 100)
	{
		__int32 vault = CreatureVault(itemNum, angle, 2, 260) + 4;
		switch (vault)
		{
		case 6:
			creature->maximumTurn = 0;
			item->animNumber = Objects[objectNumber].animIndex + 64;
			item->currentAnimState = 41;
			item->frameNumber = Anims[item->animNumber].frameBase;
			break;

		case 7:
			creature->maximumTurn = 0;
			item->animNumber = Objects[objectNumber].animIndex + 63;
			item->currentAnimState = 40;
			item->frameNumber = Anims[item->animNumber].frameBase;
			break;

		case 8:
			creature->maximumTurn = 0;
			item->animNumber = Objects[objectNumber].animIndex + 62;
			item->currentAnimState = 39;
			item->frameNumber = Anims[item->animNumber].frameBase;
			break;

		case 1:
			creature->maximumTurn = 0;
			item->animNumber = Objects[objectNumber].animIndex + 66;
			item->currentAnimState = 43;
			item->frameNumber = Anims[item->animNumber].frameBase;
			break;

		case 0:
			creature->maximumTurn = 0;
			item->animNumber = Objects[objectNumber].animIndex + 65;
			item->currentAnimState = 42;
			item->frameNumber = Anims[item->animNumber].frameBase;
			break;

		default:
			return;
		}
	}
	else
	{
		creature->maximumTurn = 0;
		item->animNumber = Objects[objectNumber].animIndex + 68;
		item->frameNumber = Anims[item->animNumber].frameBase + (GetRandomControl() & 7);
		item->currentAnimState = 44;
	}

	return;
}