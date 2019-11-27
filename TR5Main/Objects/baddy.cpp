#include "newobjects.h"
#include "../Game/Box.h"
#include "../Game/items.h"
#include "../Game/lot.h"
#include "../Game/control.h"
#include "../Game/effects.h"
#include "../Game/draw.h"
#include "../Game/sphere.h"
#include "../Game/effect2.h"
#include "../Game/people.h"
#include "../Game/lara.h"

BITE_INFO baddyGun = { 0, -16, 200, 11 };
BITE_INFO baddySword = { 0, 0, 0, 15 };
BITE_INFO silencerGun = { 3, 331, 56, 10 };

void __cdecl ClampRotation(PHD_3DPOS *pos, short angle, short rot)
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

void __cdecl InitialiseBaddy(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	
	ClearItem(itemNum);

	short objectNumber = (Objects[ID_BADDY2].loaded ? ID_BADDY2 : ID_BADDY1);
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

	short ocb = item->triggerFlags;

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

void __cdecl BaddyControl(short itemNum)
{

	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
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

	int dx = 942 * SIN(item->pos.yRot) >> 14;
	int dz = 942 * COS(item->pos.yRot) >> 14;

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
		|| y >= height1 - 384
		|| y >= height2 + 256
		|| y <= height2 - 256)
	{
		height = height2;
		canJump1sector = false;
	}

	bool canJump2sectors = true;
	if (enemyItem && item->boxNumber == enemyItem->boxNumber
		|| y >= height1 - 384
		|| y >= height - 384
		|| y >= height3 + 256
		|| y <= height3 - 256)
	{
		canJump2sectors = false;
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

		pos.x = baddyGun.x;
		pos.y = baddyGun.y;
		pos.z = baddyGun.z;

		GetJointAbsPosition(item, &pos, baddyGun.meshNum);
		TriggerDynamics(pos.x, pos.y, pos.z, 4 * item->firedWeapon + 8, 24, 16, 4);
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
			laraInfo.angle = ATAN(dz, dx) - item->pos.yRot;
			laraInfo.ahead = true;

			if (laraInfo.angle <= -16384 || laraInfo.angle >= 16384)
				laraInfo.ahead = false;

			laraInfo.distance = dx * dx + dz * dz;
		}

		GetCreatureMood(item, &info, VIOLENT);

		// Vehicle handling
		if (g_LaraExtra.Vehicle != NO_ITEM && info.bite)
			currentCreature->mood == ESCAPE_MOOD;

		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, currentCreature->maximumTurn);

		//currentCreature->enemy = LaraItem;

		// Is baddy alerted?
		if (item->hitStatus || laraInfo.distance < SQUARE(1024) ||
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

		dx = 942 * SIN(item->pos.yRot + ANGLE(45)) >> 14;
		dz = 942 * COS(item->pos.yRot + ANGLE(45)) >> 14;

		x = item->pos.xPos + dx;
		y = item->pos.yPos;
		z = item->pos.zPos + dz;

		roomNumber = item->roomNumber;
		floor = GetFloor(x, y, z, &roomNumber);
		int height4 = GetFloorHeight(floor, x, y, z);

		dx = 942 * SIN(item->pos.yRot + 14336) >> 14;
		dz = 942 * COS(item->pos.yRot + 14336) >> 14;

		x = item->pos.xPos + dx;
		y = item->pos.yPos;
		z = item->pos.zPos + dz;

		roomNumber = item->roomNumber;
		floor = GetFloor(x, y, z, &roomNumber);
		int height5 = GetFloorHeight(floor, x, y, z);

		if (abs(height5 - item->pos.yPos) > 256)
			jump = false;
		else
		{
			jump = true;
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
		int height6 = GetFloorHeight(floor, x, y, z);

		dx = 942 * SIN(item->pos.yRot - 14336) >> 14;
		dz = 942 * COS(item->pos.yRot - 14336) >> 14;

		x = item->pos.xPos + dx;
		y = item->pos.yPos;
		z = item->pos.zPos + dz;

		roomNumber = item->roomNumber;
		floor = GetFloor(x, y, z, &roomNumber);
		int height7 = GetFloorHeight(floor, x, y, z);

		if (abs(height7 - item->pos.yPos) > 256 || height6 + 512 >= item->pos.yPos)
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
				&& laraInfo.distance > SQUARE(682))
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

			if (canJump1sector || canJump2sectors)
			{
				currentCreature->maximumTurn = 0;
				item->animNumber = Objects[objectNumber].animIndex + 55;
				item->frameNumber = Anims[item->animNumber].frameBase;
				item->currentAnimState = 33;
				currentCreature->LOT.isJumping = true;

				if (!canJump2sectors)
					item->goalAnimState = 33;
				else
					item->goalAnimState = 38;
				break;
			}

			if (currentCreature->enemy)
			{
				short objNum = currentCreature->enemy->objectNumber;
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
				height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
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
				if (currentCreature->enemy && currentCreature->enemy->hitPoints > 0 && info.distance < SQUARE(682))
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
				item->goalAnimState = 0;
				break;
			}
			if (canJump1sector || canJump2sectors)
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
				if (info.distance < SQUARE(482))
				{
					item->goalAnimState = 0;
					break;
				}
				if (info.distance < SQUARE(1024))
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
			if (currentCreature->mood == ATTACK_MOOD &&
				!(currentCreature->jumpAhead) &&
				info.distance > SQUARE(1024))
			{
				item->goalAnimState = 2;
			}
			break;

		case 2:
			if (info.ahead)
			{
				joint3 = info.angle;
			}
			currentCreature->maximumTurn = ANGLE(11);
			tilt = abs(angle) / 2;
			if (objectNumber == ID_BADDY2
				&& item->frameNumber == Anims[item->animNumber].frameBase + 11
				&& height3 == height1
				&& abs(height1 - item->pos.yPos) < 384
				&& (info.angle > -4096 && info.angle < 4096 &&
					info.distance < SQUARE(3072)
					|| height2 >= height1 + 512))
			{
				item->goalAnimState = 30;
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
				item->goalAnimState = 0;
				break;
			}
			if (info.distance < SQUARE(1024))
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
					if (item->frameNumber > Anims[item->animNumber].frameBase + 13 &&
						item->frameNumber < Anims[item->animNumber].frameBase + 21)
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
			height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

			if (laraInfo.ahead
				&& laraInfo.distance < SQUARE(682)
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
			currentCreature->maximumTurn = ANGLE(7);
			if (item->boxNumber == currentCreature->LOT.targetBox ||
				!currentCreature->monkeyAhead)
			{
				floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
				height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
				if (GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) == height - 1536)
				{
					item->goalAnimState = 19;
				}
			}
			if (laraInfo.ahead)
			{
				if (laraInfo.distance < SQUARE(682))
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
			currentCreature->maximumTurn = ANGLE(7);
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
				if (info.distance >= SQUARE(682))
				{
					break;
				}
				item->goalAnimState = 28;
				currentCreature->enemy = NULL;
			}
			break;

		case 27:
			ClampRotation(&item->pos, info.angle, ANGLE(11));
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
			ClampRotation(&item->pos, info.angle, ANGLE(7));
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
			ClampRotation(&item->pos, info.angle, ANGLE(7));
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
			if (!ShotLara(item, &info, &baddyGun, joint1, 15));
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
			ClampRotation(&item->pos, info.angle, ANGLE(11));
			if (laraInfo.distance < SQUARE(682) ||
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
				ClampRotation(&item->pos, info.angle, ANGLE(7));
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
		int vault = CreatureVault(itemNum, angle, 2, 260);

		switch (vault)
		{
		case 2:
			creature->maximumTurn = 0;
			item->animNumber = Objects[objectNumber].animIndex + 64;
			item->currentAnimState = 41;
			item->frameNumber = Anims[item->animNumber].frameBase;
			break;

		case 3:
			creature->maximumTurn = 0;
			item->animNumber = Objects[objectNumber].animIndex + 63;
			item->currentAnimState = 40;
			item->frameNumber = Anims[item->animNumber].frameBase;
			break;

		case 4:
			creature->maximumTurn = 0;
			item->animNumber = Objects[objectNumber].animIndex + 62;
			item->currentAnimState = 39;
			item->frameNumber = Anims[item->animNumber].frameBase;
			break;

		case -3:
			creature->maximumTurn = 0;
			item->animNumber = Objects[objectNumber].animIndex + 66;
			item->currentAnimState = 43;
			item->frameNumber = Anims[item->animNumber].frameBase;
			break;

		case -4:
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

void __cdecl SilencerControl(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* silencer = (CREATURE_INFO*)item->data;
	short angle = 0, torso_y = 0, torso_x = 0, head_y = 0, tilt = 0;

	if (item->hitPoints <= 0)
	{
		if (item->currentAnimState != 12 && item->currentAnimState != 13)
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 20; // die 21 is for heavy weapon.
			item->frameNumber = Anims[item->animNumber].frameBase;
			item->currentAnimState = 13;
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);
		angle = CreatureTurn(item, silencer->maximumTurn);

		switch (item->currentAnimState)
		{
		case 3:
			if (info.ahead)
				head_y = info.angle;
			silencer->maximumTurn = 0;
			if (item->requiredAnimState)
				item->goalAnimState = item->requiredAnimState;
			break;

		case 4:
			if (info.ahead)
				head_y = info.angle;
			silencer->maximumTurn = 0;

			if (silencer->mood == ESCAPE_MOOD)
			{
				item->requiredAnimState = 2;
				item->goalAnimState = 3;
			}
			else
			{
				if (Targetable(item, &info))
				{
					item->requiredAnimState = (GetRandomControl() >= 0x4000 ? 10 : 6);
					item->goalAnimState = 3;
				}

				if (silencer->mood == ATTACK_MOOD || !info.ahead)
				{
					if (info.distance >= 0x400000)
					{
						item->requiredAnimState = 2;
						item->goalAnimState = 3;
					}
					else
					{
						item->requiredAnimState = 1;
						item->goalAnimState = 3;
					}
				}
				else
				{
					if (GetRandomControl() >= 1280)
					{
						if (GetRandomControl() < 2560)
						{
							item->requiredAnimState = 1;
							item->goalAnimState = 3;
						}
					}
					else
					{
						item->requiredAnimState = 5;
						item->goalAnimState = 3;
					}
				}
			}
			break;
		case 1:
			if (info.ahead)
				head_y = info.angle;
			silencer->maximumTurn = 910;

			if (silencer->mood == ESCAPE_MOOD)
			{
				item->goalAnimState = 2;
			}
			else if (Targetable(item, &info))
			{
				item->requiredAnimState = (GetRandomControl() >= 0x4000 ? 10 : 6);
				item->goalAnimState = 3;
			}
			else
			{

				if (info.distance > 0x400000 || !info.ahead)
					item->goalAnimState = 2;
				if (silencer->mood == BORED_MOOD && GetRandomControl() < 0x300)
					item->goalAnimState = 3;
			}
			break;
		case 2:
			if (info.ahead)
				head_y = info.angle;
			silencer->maximumTurn = 910;
			silencer->flags = 0;
			tilt = (angle / 4);

			if (silencer->mood == ESCAPE_MOOD)
			{
				if (Targetable(item, &info))
					item->goalAnimState = 9;
				break;
			}

			if (Targetable(item, &info))
			{
				if (info.distance >= 0x400000 && info.zoneNumber == info.enemyZone)
					item->goalAnimState = 9;
				break;
			}
			else if (silencer->mood == ATTACK_MOOD)
				item->goalAnimState = (GetRandomControl() >= 0x4000) ? 3 : 2;
			else
				item->goalAnimState = 3;
			break;

		case 5:
			if (info.ahead)
				head_y = info.angle;
			silencer->maximumTurn = 0;

			if (Targetable(item, &info))
			{
				item->requiredAnimState = 6;
				item->goalAnimState = 3;
			}
			else
			{
				if (silencer->mood == ATTACK_MOOD || GetRandomControl() < 0x100)
					item->goalAnimState = 3;
				if (!info.ahead)
					item->goalAnimState = 3;
			}
			break;

		case 6:
		case 10:
			silencer->maximumTurn = 0;
			silencer->flags = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}
			else
			{
				head_y = info.angle;
			}

			if (silencer->mood == ESCAPE_MOOD)
				item->goalAnimState = 3;
			else if (Targetable(item, &info))
				item->goalAnimState = item->currentAnimState != 6 ? 11 : 7;
			else
				item->goalAnimState = 3;
			break;
		case 7:
		case 11:
			silencer->maximumTurn = 0;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}
			else
			{
				head_y = info.angle;
			}

			if (!silencer->flags)
			{
				ShotLara(item, &info, &silencerGun, torso_y, 50);
				silencer->flags = 1;
			}
			break;
		case 9:
			silencer->maximumTurn = 910;

			if (info.ahead)
			{
				torso_y = info.angle;
				torso_x = info.xAngle;
			}
			else
			{
				head_y = info.angle;
			}

			if (!item->requiredAnimState)
			{
				if (!ShotLara(item, &info, &silencerGun, torso_y, 50))
					item->goalAnimState = 2;

				item->requiredAnimState = 9;
			}
			break;
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, torso_y);
	CreatureJoint(item, 1, torso_x);
	CreatureJoint(item, 2, head_y);
	CreatureAnimation(itemNum, angle, tilt);
}