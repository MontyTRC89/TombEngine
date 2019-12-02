#include "mafia.h"

#include "lion.h"
#include "..\Global\global.h"
#include "items.h"
#include "effect2.h"
#include "draw.h"
#include "sphere.h"
#include "effects.h"
#include "people.h"
#include "lot.h"
#include "box.h"

BITE_INFO ArmedBaddy2Gun = { 0x0FFFFFFCE, 0xDC, 0x3C, 0x0D };

void __cdecl InitialiseArmedBaddy2(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	
	ClearItem(itemNum);

	item->animNumber = Objects[item->objectNumber].animIndex;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->goalAnimState = 1;
	item->currentAnimState = 1;
	*item->pad2 = 9216;
}

void __cdecl ArmedBaddy2Control(short itemNum)
{
	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	
	short angle = 0;
	short joint2 = 0;
	short joint1 = 0;
	short joint0 = 0;

	// Can baddy jump? Check for a distance of 1 and 2 sectors
	int x = item->pos.xPos;
	int y = item->pos.yPos;
	int z = item->pos.zPos;

	int dx = 870 * SIN(item->pos.yRot) >> 14;
	int dz = 870 * COS(item->pos.yRot) >> 14;

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
	if (item->boxNumber == LaraItem->boxNumber
		|| y >= height1 - 384
		|| y >= height2 + 256
		|| y <= height2 - 256)
	{
		height = height2;
		canJump1sector = false;
	}

	bool canJump2sectors = true;
	if (item->boxNumber == LaraItem->boxNumber
		|| y >= height1 - 384
		|| y >= height - 384
		|| y >= height3 + 256
		|| y <= height3 - 256)
	{
		canJump2sectors = false;
	}

	if (item->firedWeapon)
	{
		PHD_VECTOR pos;

		pos.x = ArmedBaddy2Gun.x;
		pos.y = ArmedBaddy2Gun.y;
		pos.z = ArmedBaddy2Gun.z;

		GetJointAbsPosition(item, &pos, ArmedBaddy2Gun.meshNum);
		TriggerDynamicLight(pos.x, pos.y, pos.z, 4 * item->firedWeapon + 8, 24, 16, 4);
		item->firedWeapon--;
	}

	AI_INFO info;
	AI_INFO laraInfo;

	ZeroMemory(&info, sizeof(AI_INFO));

	if (item->hitPoints > 0)
	{
		if (item->aiBits)
			GetAITarget(creature);
		else
			creature->enemy = LaraItem;

		CreatureAIInfo(item, &info);

		if (creature->enemy == LaraItem)
		{
			laraInfo.angle = info.angle;
			laraInfo.distance = info.distance;
		}
		else
		{
			dx = LaraItem->pos.xPos - item->pos.xPos;
			dz = LaraItem->pos.zPos - item->pos.zPos;
			laraInfo.angle = ATAN(dz, dx) - item->pos.yRot;
			laraInfo.distance = SQUARE(dx) + SQUARE(dz);
		}

		GetCreatureMood(item, &info, creature->enemy != LaraItem);
		CreatureMood(item, &info, creature->enemy != LaraItem);

		angle = CreatureTurn(item, creature->maximumTurn);
		//creature->enemy = LaraItem;

		if (laraInfo.distance < SQUARE(2048) && LaraItem->speed > 20 || item->hitStatus || TargetVisible(item, &laraInfo))
		{
			if (!(item->aiBits & FOLLOW))
			{
				creature->enemy = LaraItem;
				AlertAllGuards(itemNum);
			}
		}
		
		switch (item->currentAnimState)
		{
		case 0:
			creature->LOT.isJumping = false;
			joint2 = laraInfo.angle;
			creature->flags = 0;
			creature->maximumTurn = 0;

			if (info.ahead && !(item->aiBits & GUARD) )
			{
				joint0 = info.angle >> 1;
				joint1 = info.xAngle;
			}
			
			if (item->aiBits & GUARD)
			{
				joint2 = AIGuard(creature);
				break;
			}

			if (laraInfo.angle <= 20480 && laraInfo.angle >= -20480)
			{
				if (*item->pad2 == 9216)
				{
					item->goalAnimState = 37;
					break;
				}
			}
			else if (*item->pad2 == 9216)
			{
				item->goalAnimState = 2;
				break;
			}

			if (Targetable(item, &info))
			{
				if (info.distance < SQUARE(1024) || info.zoneNumber != info.enemyZone)
				{
					item->goalAnimState = 4;
				}
				else if (!(item->aiBits & MODIFY))
				{
					item->goalAnimState = 5;
				}
			}
			else
			{
				if (item->aiBits & PATROL1)
				{
					item->goalAnimState = 5;
				}
				else
				{
					if (canJump1sector || canJump2sectors)
					{
						creature->maximumTurn = 0;
						item->animNumber = Objects[item->objectNumber].animIndex + 41;
						item->currentAnimState = 26;
						item->frameNumber = Anims[item->animNumber].frameBase;
						if (canJump2sectors)
							item->goalAnimState = 28;
						else
							item->goalAnimState = 27;
						creature->LOT.isJumping = true;
						break;
					}
					if (creature->mood)
					{
						if (info.distance >= SQUARE(3072))
							goto LABEL_82;
						item->goalAnimState = 5;
					}
					else
					{
						item->goalAnimState = 1;
					}
				}
			}
			break;

		case 1:
		case 31:
			creature->maximumTurn = 0;
			if (info.angle >= 0)
				item->pos.yRot -= ANGLE(2);
			else
				item->pos.yRot += ANGLE(2);

			if (item->frameNumber != Anims[item->animNumber].frameBase + 16 || *item->pad2 != 9216)
			{
				if (item->frameNumber == Anims[item->animNumber].frameEnd)
					item->pos.yRot += -ANGLE(180);
			}
			else
			{
				*item->pad2 = 128;
			}
			break;

		case 2:
			joint0 = laraInfo.angle >> 1;
			joint2 = laraInfo.angle >> 1;
			if (info.ahead)
				joint1 = info.xAngle;
			creature->maximumTurn = 0;
			if (abs(info.angle) >= ANGLE(2))
			{
				if (info.angle >= 0)
					item->pos.yRot += ANGLE(2);
				else
					item->pos.yRot -= ANGLE(2);
			}
			else
			{
				item->pos.yRot += info.angle;
			}
			if (!creature->flags)
			{
				ShotLara(item, &info, &ArmedBaddy2Gun, laraInfo.angle >> 1, 35);
				creature->flags = 1;
				item->firedWeapon = 2;
			}
			break;

		case 3:
			joint0 = laraInfo.angle >> 1;
			joint2 = laraInfo.angle >> 1;
			creature->flags = 0;
			creature->maximumTurn = 0;
			if (info.ahead)
				joint1 = info.xAngle;
			if (abs(info.angle) >= ANGLE(2))
			{
				if (info.angle >= 0)
					item->pos.yRot += ANGLE(2);
				else
					item->pos.yRot -= ANGLE(2);
			}
			else
			{
				item->pos.yRot += info.angle;
			}
			if (Targetable(item, &info))
			{
				item->goalAnimState = 3;
			}
			else if (laraInfo.angle > 20480 || laraInfo.angle < -20480)
			{
				item->goalAnimState = 32;
			}
			else
			{
				item->goalAnimState = 1;
			}
			break;

		case 4:
			creature->LOT.isJumping = false;
			creature->maximumTurn = ANGLE(5);
			if (Targetable(item, &info) && (info.distance < SQUARE(1024) || info.zoneNumber != info.enemyZone))
			{
				item->goalAnimState = 4;
			}
			else
			{
				if (canJump1sector || canJump2sectors)
				{
					creature->maximumTurn = 0;
					creature->maximumTurn = 0;
					item->animNumber = Objects[item->objectNumber].animIndex + 41;
					item->currentAnimState = 26;
					item->frameNumber = Anims[item->animNumber].frameBase;
					if (canJump2sectors)
						item->goalAnimState = 28;
					else
						item->goalAnimState = 27;
					creature->LOT.isJumping = true;
					break;
				}
				if (info.distance >= SQUARE(1024))
				{
					if (info.distance > SQUARE(3072))
						LABEL_82:
					item->goalAnimState = 7;
				}
				else
				{
					item->goalAnimState = 1;
				}
			}
			break;

		case 6:
			creature->LOT.isJumping = false;
			creature->maximumTurn = ANGLE(10);
			if (Targetable(item, &info) && (info.distance < SQUARE(1024) || info.zoneNumber != info.enemyZone))
			{
				item->goalAnimState = 4;
			}
			else if (canJump1sector || canJump2sectors)
			{
				creature->maximumTurn = 0;
				item->animNumber = Objects[item->objectNumber].animIndex + 50;
				item->currentAnimState = 26;
				item->frameNumber = Anims[item->animNumber].frameBase;
				if (canJump2sectors)
					item->goalAnimState = 28;
				else
					item->goalAnimState = 27;
				creature->LOT.isJumping = true;
			}
			else if (info.distance < SQUARE(3072))
			{
				item->goalAnimState = 5;
			}
			break;

		case 36:
			creature->maximumTurn = 0;
			if (info.angle >= 0)
				item->pos.yRot += ANGLE(2);
			else
				item->pos.yRot -= ANGLE(2);
			if (item->frameNumber == Anims[item->animNumber].frameBase + 16 && *item->pad2 == 9216)
				* item->pad2 = 128;
			break;
		default:
			break;
		}
	}
	else
	{
		if (item->currentAnimState != 8 && item->currentAnimState != 6)
		{
			if (info.angle >= 12288 || info.angle <= -12288)
			{
				item->currentAnimState = 8;
				item->animNumber = Objects[item->objectNumber].animIndex + 16;
				item->pos.yRot += info.angle - ANGLE(180);
			}
			else
			{
				item->currentAnimState = 6;
				item->animNumber = Objects[item->objectNumber].animIndex + 11;
				item->pos.yRot += info.angle;
			}
			item->frameNumber = Anims[item->animNumber].frameBase;
		}
	}

	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);

	if (item->currentAnimState >= 20 || item->currentAnimState == 6 || item->currentAnimState == 8)
	{
		CreatureAnimation(itemNum, angle, 0);
	}
	else
	{
		switch (CreatureVault(itemNum, angle, 2, 256) + 4)
		{
		case 0:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 38;
			item->currentAnimState = 23;
			item->frameNumber = Anims[item->animNumber].frameBase;
			break;
		case 1:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 39;
			item->currentAnimState = 24;
			item->frameNumber = Anims[item->animNumber].frameBase;
			break;
		case 2:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 40;
			item->currentAnimState = 25;
			item->frameNumber = Anims[item->animNumber].frameBase;
			break;
		case 6:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 35;
			item->currentAnimState = 20;
			item->frameNumber = Anims[item->animNumber].frameBase;
			break;
		case 7:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 36;
			item->currentAnimState = 21;
			item->frameNumber = Anims[item->animNumber].frameBase;
			break;
		case 8:
			creature->maximumTurn = 0;
			item->animNumber = Objects[item->objectNumber].animIndex + 37;
			item->currentAnimState = 22;
			item->frameNumber = Anims[item->animNumber].frameBase;
			break;
		default:
			return;
		}
	}
}

void Inject_Mafia()
{
	INJECT(0x0045B7B0, InitialiseArmedBaddy2);
	INJECT(0x0045B840, ArmedBaddy2Control);
}