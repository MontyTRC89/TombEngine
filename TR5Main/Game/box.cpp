#include "Box.h"
#include "..\Global\global.h"
#include "items.h"
#include "tomb4fx.h"
#include "lot.h"
#include "deltapak.h"
#include "items.h"
#include "lara.h"
#include "draw.h"
#include "sphere.h"

extern LaraExtraInfo g_LaraExtra;

void __cdecl DropBaddyPickups(ITEM_INFO* item)
{
	ITEM_INFO* pickup = NULL;

	for (__int16 pickupNumber = item->carriedItem; pickupNumber != NO_ITEM; pickupNumber = pickup->carriedItem)
	{
		pickup = &Items[pickupNumber];
		pickup->pos.xPos = (item->pos.xPos & 0xFFFFFC00) | 0x200;
		pickup->pos.zPos = (item->pos.zPos & 0xFFFFFC00) | 0x200;

		__int16 roomNumber = item->roomNumber;
		pickup->pos.yPos = GetFloorHeight(GetFloor(pickup->pos.xPos, item->pos.yPos, pickup->pos.zPos, &roomNumber),
			pickup->pos.xPos, item->pos.yPos, pickup->pos.zPos);
		pickup->pos.yPos -= GetBoundsAccurate(pickup)[3];

		ItemNewRoom(pickupNumber, item->roomNumber);
		pickup->flags |= 0x20;
	}
}

__int32 __cdecl MoveCreature3DPos(PHD_3DPOS* srcpos, PHD_3DPOS* destpos, __int32 velocity, __int16 angdif, __int32 angadd)
{
	__int32 x = destpos->xPos - srcpos->xPos;
	__int32 y = destpos->yPos - srcpos->yPos;
	__int32 z = destpos->zPos - srcpos->zPos;
	__int32 dist = SQRT_ASM(SQUARE(x) + SQUARE(y) + SQUARE(z));

	if (velocity < dist)
	{
		srcpos->xPos += velocity * x / dist;
		srcpos->yPos += velocity * y / dist;
		srcpos->zPos += velocity * z / dist;
	}
	else
	{
		srcpos->xPos = destpos->xPos;
		srcpos->yPos = destpos->yPos;
		srcpos->zPos = destpos->zPos;
	}

	if (angdif <= angadd)
	{
		if (angdif >= -angadd)
			srcpos->yRot = destpos->yRot;
		else
			srcpos->yRot -= angadd;
	}
	else
	{
		srcpos->yRot += angadd;
	}

	return srcpos->xPos == destpos->xPos
		&& srcpos->yPos == destpos->yPos
		&& srcpos->zPos == destpos->zPos
		&& srcpos->yRot == destpos->yRot;
}

void __cdecl CreatureYRot2(PHD_3DPOS* srcpos, __int16 angle, __int16 angadd) 
{
	if (angadd < angle)
	{
		srcpos->yRot += angadd;
		return;
	} 

	if (angle < -angadd)
	{
		srcpos->yRot -= angadd;
		return;
	} 

	srcpos->yRot += angle;

	return;
}

__int16 __cdecl SameZone(CREATURE_INFO* creature, ITEM_INFO* targetItem) 
{
	ITEM_INFO* item = &Items[creature->itemNum];

	__int16* zone = GroundZones[creature->LOT.zone * 2 + FlipStatus];

	ROOM_INFO* r = &Rooms[item->roomNumber];
	item->boxNumber = XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z).box;

	r = &Rooms[targetItem->roomNumber];
	targetItem->boxNumber = XZ_GET_SECTOR(r, targetItem->pos.xPos - r->x, targetItem->pos.zPos - r->z).box;

	return (zone[item->boxNumber] == zone[targetItem->boxNumber]);
}

__int16 __cdecl AIGuard(CREATURE_INFO* creature) 
{
	__int32 random;

	if (Items[creature->itemNum].aiBits & (GUARD | PATROL1))
		return 0;

	random = GetRandomControl();

	if (random < 0x100)
	{
		creature->headRight = true;
		creature->headLeft = true;
	}
	else if (random < 0x180)
	{
		creature->headRight = false;
		creature->headLeft = true;
	}
	else if (random < 0x200)
	{
		creature->headRight = true;
		creature->headLeft = false;
	}

	if (!creature->headLeft)
		return (creature->headRight) << 12;

	if (creature->headRight)
		return 0;

	return -0x4000;
}

void __cdecl AlertNearbyGuards(ITEM_INFO* item) 
{
	for (__int32 i = 0; i < NUM_SLOTS; i++)
	{
		CREATURE_INFO* creature = &BaddieSlots[i];

		if (creature->itemNum == NO_ITEM)
			continue;

		ITEM_INFO* target = &Items[creature->itemNum + i];
		if (item->roomNumber == target->roomNumber)
		{
			creature->alerted = true;
			continue;
		}

		__int32 x = (target->pos.xPos - item->pos.xPos) / 64;
		__int32 y = (target->pos.yPos - item->pos.yPos) / 64;
		__int32 z = (target->pos.zPos - item->pos.zPos) / 64;

		__int32 distance = SQUARE(x) + SQUARE(y) + SQUARE(z);

		if (distance < 8000)
			creature->alerted = true;
	}
}

void __cdecl AlertAllGuards(__int16 itemNumber) 
{
	__int16 objNumber = Items[itemNumber].objectNumber;

	for (__int32 i = 0; i < NUM_SLOTS; i++)
	{
		CREATURE_INFO* creature = &BaddieSlots[i];
		if (creature->itemNum == NO_ITEM)
			continue;

		ITEM_INFO* target = &Items[creature->itemNum];
		if (objNumber == target->objectNumber)
		{
			if (target->status == ITEM_ACTIVE)
			{
				creature->alerted = true;
			}
		}
	}
}

void __cdecl CreatureKill(ITEM_INFO* item, __int32 killAnim, __int32 killState, __int16 laraKillState)
{
	item->animNumber = Objects[item->objectNumber].animIndex + killAnim;
	item->frameNumber = Anims[item->animNumber].frameBase;
	item->currentAnimState = killState;

	LaraItem->animNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex;
	LaraItem->frameNumber = Anims[LaraItem->animNumber].frameBase;
	LaraItem->currentAnimState = 0;
	LaraItem->goalAnimState = laraKillState;

	LaraItem->pos.xPos = item->pos.xPos;
	LaraItem->pos.yPos = item->pos.yPos;
	LaraItem->pos.zPos = item->pos.zPos;
	LaraItem->pos.yRot = item->pos.yRot;
	LaraItem->pos.xRot = item->pos.xRot;
	LaraItem->pos.zRot = item->pos.zRot;
	LaraItem->fallspeed = 0;
	LaraItem->gravityStatus = false;
	LaraItem->speed = 0;

	if (item->roomNumber != LaraItem->roomNumber)
		ItemNewRoom(Lara.itemNumber, item->roomNumber);

	AnimateItem(LaraItem);

	g_LaraExtra.ExtraAnim = 1;
	Lara.gunStatus = LG_HANDS_BUSY;
	Lara.gunType = WEAPON_NONE;
	Lara.hitDirection = -1;
	Lara.air = -1;

	Camera.pos.roomNumber = LaraItem->roomNumber; 
	Camera.type = CHASE_CAMERA;
	Camera.flags = FOLLOW_CENTRE;
	Camera.targetAngle = ANGLE(170);
	Camera.targetElevation = -ANGLE(25);
}

__int16 __cdecl CreatureEffect2(ITEM_INFO* item, BITE_INFO* bite, __int16 damage, __int16 angle, __int16 (*generate)(__int32 x, __int32 y, __int32 z, __int16 speed, __int16 yrot, __int16 roomNumber))
{
	PHD_VECTOR pos;

	pos.x = bite->x;
	pos.y = bite->y;
	pos.z = bite->z;

	GetJointAbsPosition(item, &pos, bite->meshNum);

	return generate(pos.x, pos.y, pos.z, damage, angle, item->roomNumber);
}

__int16 __cdecl CreatureEffect(ITEM_INFO* item, BITE_INFO* bite, __int16(*generate)(__int32 x, __int32 y, __int32 z, __int16 speed, __int16 yrot, __int16 roomNumber))
{
	PHD_VECTOR pos;

	pos.x = bite->x;
	pos.y = bite->y;
	pos.z = bite->z;

	GetJointAbsPosition(item, &pos, bite->meshNum);

	return generate(pos.x, pos.y, pos.z, item->speed, item->pos.yRot, item->roomNumber);
}

void __cdecl CreatureUnderwater(ITEM_INFO* item, __int32 depth)
{
	__int32 waterLevel;
	if (depth < 0)
	{
		waterLevel = -depth;
		depth = 0;
	}
	else
	{
		waterLevel = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
	}

	if (item->pos.yPos < depth)
	{
		__int16 roomNumber = item->roomNumber;
		__int32 floorHeight = GetFloorHeight(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber), item->pos.xPos, item->pos.yPos, item->pos.zPos);

		if (floorHeight < depth)
		{
			item->pos.yPos = floorHeight;
		}
		else
		{
			item->pos.yPos = depth;
		}

		if (item->pos.xRot > ANGLE(2))
		{
			item->pos.xRot -= ANGLE(2);
		}
		else if (item->pos.xRot > 0)
		{
			item->pos.xRot = 0;
		}
	}
}

void __cdecl CreatureFloat(__int16 itemNumber) 
{
	ITEM_INFO* item = &Items[itemNumber];
	item->hitPoints = -16384;
	item->pos.xRot = 0;

	__int32 waterLevel = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
	__int32 y = item->pos.yPos;

	if (y > waterLevel)
		item->pos.yPos = y - 32;
	if (item->pos.yPos < waterLevel)
		item->pos.yPos = waterLevel;

	AnimateItem(item);

	printf("Crocodile Y: %d\n", item->pos.yPos);

	__int16 roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	
	if (roomNumber != item->roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	if (item->pos.yPos <= waterLevel)
	{
		if (item->frameNumber == Anims[item->animNumber].frameBase)
		{
			item->pos.yPos = waterLevel;
			item->collidable = false;
			item->status = ITEM_DEACTIVATED;
			DisableBaddieAI(itemNumber);
			RemoveActiveItem(itemNumber);
			item->afterDeath = 1;
		}
	}
}

void __cdecl CreatureJoint(ITEM_INFO* item, __int16 joint, __int16 required) 
{
	if (item->data == NULL)
		return;

	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	__int16 change = required - creature->jointRotation[joint];
	if (change > ANGLE(3))
		change = ANGLE(3);
	else if (change < -ANGLE(3))
		change = -ANGLE(3);

	creature->jointRotation[joint] += change;

	if (creature->jointRotation[joint] > 12288)
		creature->jointRotation[joint] = 12288;
	else if (creature->jointRotation[joint] < -12288)
		creature->jointRotation[joint] = -12288;
}

void __cdecl CreatureTilt(ITEM_INFO* item, __int16 angle) 
{
	angle = (angle << 2) - item->pos.zRot;

	if (angle < -ANGLE(3))
		angle = -ANGLE(3);
	else if (angle > ANGLE(3))
		angle = ANGLE(3);

	__int16 theAngle = -ANGLE(3);

	__int16 absRot = abs(item->pos.zRot);
	if (absRot < ANGLE(15) || absRot > ANGLE(30))
		angle >>= 1;

	item->pos.zRot += angle;  
}

__int16 __cdecl CreatureTurn2(ITEM_INFO* item, __int16 maximumTurn)
{
	if (item->data == NULL || maximumTurn == 0)
		return 0;

	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;



	return 0;
}

__int32 __cdecl CreatureAnimation(__int16 itemNumber, __int16 angle, __int16 tilt)
{
	ITEM_INFO* item = &Items[itemNumber];
	if (item->data == NULL)
		return (0);

	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	LOT_INFO* LOT = &creature->LOT;

	__int32 boxHeight = Boxes[item->boxNumber].height;

	PHD_VECTOR old;
	old.x = item->pos.xPos;
	old.y = item->pos.yPos;
	old.z = item->pos.zPos;

	__int16* zone = GroundZones[FlipStatus + LOT->zone];

	AnimateItem(item);

	if (item->status == ITEM_DEACTIVATED)
	{
		CreatureDie(itemNumber, 0);
		return 0;
	}

	__int16* bounds = GetBoundsAccurate(item);
	__int32 y = item->pos.yPos + bounds[2];

	__int16 roomNumber = item->roomNumber;
	GetFloor(old.x, y, old.z, &roomNumber);  
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, y, item->pos.zPos, &roomNumber);

	__int32 boxIndex = floor->box;
	__int32 height = Boxes[item->boxNumber].height;
	__int32 nextHeight = 0;

	if (LOT->node[boxIndex].exitBox == NO_BOX)
		nextHeight = height;
	else
		nextHeight = Boxes[LOT->node[boxIndex].exitBox].height;

	if (floor->box == NO_BOX
		|| !LOT->isJumping
		&& (zone[item->boxNumber] != zone[boxIndex]
			|| boxHeight - height > LOT->step
			|| boxHeight - height < LOT->drop))
	{
		__int32 xPos = item->pos.xPos >> WALL_SHIFT;
		__int32 zPos = item->pos.zPos >> WALL_SHIFT;

		__int32 shiftX = old.x >> WALL_SHIFT;
		__int32 shiftZ = old.z >> WALL_SHIFT;

		if (xPos < shiftX)
			item->pos.xPos = old.x & (~(WALL_SIZE - 1));
		else if (xPos > shiftX)
			item->pos.xPos = old.x | (WALL_SIZE - 1);

		if (zPos < shiftZ)
			item->pos.zPos = old.z & (~(WALL_SIZE - 1));
		else if (zPos > shiftZ)
			item->pos.zPos = old.z | (WALL_SIZE - 1);

		floor = GetFloor(item->pos.xPos, y, item->pos.zPos, &roomNumber);

		height = Boxes[floor->box].height;
		if (LOT->node[floor->box].exitBox == NO_BOX)
			nextHeight = height;
		else
			nextHeight = Boxes[LOT->node[boxIndex].exitBox].height;
	}

	__int32 x = item->pos.xPos;
	__int32 z = item->pos.zPos;

	__int32 xPos = x & (WALL_SIZE - 1);
	__int32 zPos = z & (WALL_SIZE - 1);
	__int32 radius = Objects[item->objectNumber].radius;

	__int32 shiftX = 0;
	__int32 shiftZ = 0;

	if (zPos < radius)
	{
		if (BadFloor(x, y, z - radius, height, nextHeight, roomNumber, LOT))
			shiftZ = radius - zPos;

		if (xPos < radius)
		{
			if (BadFloor(x - radius, y, z, height, nextHeight, roomNumber, LOT))
				shiftX = radius - xPos;
			else if (!shiftZ && BadFloor(x - radius, y, z - radius, height, nextHeight, roomNumber, LOT))
			{
				if (item->pos.yRot > -0x6000 && item->pos.yRot < 0x2000)
					shiftZ = radius - zPos;
				else
					shiftX = radius - xPos;
			}
		}
		else if (xPos > WALL_SIZE - radius)
		{
			if (BadFloor(x + radius, y, z, height, nextHeight, roomNumber, LOT))
				shiftX = WALL_SIZE - radius - xPos;
			else if (!shiftZ && BadFloor(x + radius, y, z - radius, height, nextHeight, roomNumber, LOT))
			{
				if (item->pos.yRot > -0x2000 && item->pos.yRot < 0x6000)
					shiftZ = radius - zPos;
				else
					shiftX = WALL_SIZE - radius - xPos;
			}
		}
	}
	else if (zPos > WALL_SIZE - radius)
	{
		if (BadFloor(x, y, z + radius, height, nextHeight, roomNumber, LOT))
			shiftZ = WALL_SIZE - radius - zPos;

		if (xPos < radius)
		{
			if (BadFloor(x - radius, y, z, height, nextHeight, roomNumber, LOT))
				shiftX = radius - xPos;
			else if (!shiftZ && BadFloor(x - radius, y, z + radius, height, nextHeight, roomNumber, LOT))
			{
				if (item->pos.yRot > -0x2000 && item->pos.yRot < 0x6000)
					shiftX = radius - xPos;
				else
					shiftZ = WALL_SIZE - radius - zPos;
			}
		}
		else if (xPos > WALL_SIZE - radius)
		{
			if (BadFloor(x + radius, y, z, height, nextHeight, roomNumber, LOT))
				shiftX = WALL_SIZE - radius - xPos;
			else if (!shiftZ && BadFloor(x + radius, y, z + radius, height, nextHeight, roomNumber, LOT))
			{
				if (item->pos.yRot > -0x6000 && item->pos.yRot < 0x2000)
					shiftX = WALL_SIZE - radius - xPos;
				else
					shiftZ = WALL_SIZE - radius - zPos;
			}
		}
	}
	else if (xPos < radius)
	{
		if (BadFloor(x - radius, y, z, height, nextHeight, roomNumber, LOT))
			shiftX = radius - xPos;
	}
	else if (xPos > WALL_SIZE - radius)
	{
		if (BadFloor(x + radius, y, z, height, nextHeight, roomNumber, LOT))
			shiftX = WALL_SIZE - radius - xPos;
	}

	item->pos.xPos += shiftX;
	item->pos.zPos += shiftZ;

	if (shiftX || shiftZ)
	{
		floor = GetFloor(item->pos.xPos, y, item->pos.zPos, &roomNumber);

		item->pos.yRot += angle;
		if (tilt)
			CreatureTilt(item, (tilt * 2));
	}

	__int16 biffAngle = 0;
	if (item->speed)
	{
		if (item->hitPoints > 0)
		{
			biffAngle = CreatureCreature(itemNumber);
			if (biffAngle)
			{
				if (abs(biffAngle) >= 1536)
				{
					if (biffAngle <= 0)
						item->pos.yRot += 1536;
					else
						item->pos.yRot -= 1536;
					return 1;
				}
				else
				{
					item->pos.yRot -= biffAngle;
					return 1;
				}
			}
		}
	}

	if (LOT->fly && item->hitPoints > 0)
	{
		__int32 dy = creature->target.y - item->pos.yPos;
		if (dy > LOT->fly)
			dy = LOT->fly;
		else if (dy < -LOT->fly)
			dy = -LOT->fly;

		height = GetFloorHeight(floor, item->pos.xPos, y, item->pos.zPos);
		if (item->pos.yPos + dy <= height)
		{
			if (Objects[item->objectNumber].waterCreature)
			{
				__int32 ceiling = GetCeiling(floor, item->pos.xPos, y, item->pos.zPos);

				if (item->pos.yPos + bounds[2] + dy < ceiling)
				{
					if (item->pos.yPos + bounds[2] < ceiling)
					{
						item->pos.xPos = old.x;
						item->pos.zPos = old.z;
						dy = LOT->fly;
					}
					else
						dy = 0;
				}
			}
			else
			{
				floor = GetFloor(item->pos.xPos, y + 256, item->pos.zPos, &roomNumber);
				if (Rooms[roomNumber].flags & ENV_FLAG_WATER)  // (UNDERWATER | SWAMP)
				{
					dy = -LOT->fly;
				}
			}
		}
		else if (item->pos.yPos <= height)
		{
			dy = 0;
			item->pos.yPos = height;
		}
		else
		{
			item->pos.xPos = old.x;
			item->pos.zPos = old.z;
			dy = -LOT->fly;
		}

		item->pos.yPos += dy;
		floor = GetFloor(item->pos.xPos, y, item->pos.zPos, &roomNumber);
		item->floor = GetFloorHeight(floor, item->pos.xPos, y, item->pos.zPos);
 
		angle = (item->speed) ? ATAN(item->speed, -dy) : 0;
		if (angle < -ANGLE(20))
			angle = -ANGLE(20);
		else if (angle > ANGLE(20))
			angle = ANGLE(20);

		if (angle < item->pos.xRot - ONE_DEGREE)
			item->pos.xRot -= ONE_DEGREE;
		else if (angle > item->pos.xRot + ONE_DEGREE)
			item->pos.xRot += ONE_DEGREE;
		else
			item->pos.xRot = angle;
	}
	else if (LOT->isJumping)
	{
		floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		
		if (LOT->isMonkeying)
		{
			item->pos.yPos = GetCeiling(floor, item->pos.xPos, y, item->pos.zPos) - bounds[2];
		}
		else
		{
			if (item->pos.yPos > height + 256)
			{
				item->pos.xPos = old.x;
				item->pos.yPos = old.y;
				item->pos.zPos = old.z;
			}
			else
			{
				item->pos.yPos = height;
			}
		}
	} 
	else
	{
		floor = GetFloor(item->pos.xPos, y, item->pos.zPos, &roomNumber);

		// Uncomment these lines if there are baddies whose height is not represented correctly by their bounds */
		/*if (item->object_number == TREX || item->object_number == SHIVA || item->object_number == MUTANT2)
			top = STEP_L * 3;
		else
			top = bounds[2];*/

		if (item->pos.yPos + bounds[2] < GetCeiling(floor, item->pos.xPos, y, item->pos.zPos))
		{
			item->pos.xPos = old.x;
			item->pos.zPos = old.z;
			item->pos.yPos = old.y;
		}

		floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

		if (item->pos.yPos > item->floor)
			item->pos.yPos = item->floor;
		else if (item->floor - item->pos.yPos > 64)
			item->pos.yPos += 64;
		else if (item->pos.yPos < item->floor)
			item->pos.yPos = item->floor;

		item->pos.xRot = 0;
	}

	roomNumber = item->roomNumber;
	GetFloor(item->pos.xPos, item->pos.yPos - 32, item->pos.zPos, &roomNumber);
	if (item->roomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	return 1;
}

void __cdecl CreatureDie(__int16 itemNumber, __int32 explode)
{
	ITEM_INFO* item = &Items[itemNumber];
	item->hitPoints = -16384;
	item->collidable = false;

	if (explode)
	{
		if (Objects[item->objectNumber].hitEffect)
			ExplodingDeath2(itemNumber, -1, 258);
		else
			ExplodingDeath2(itemNumber, -1, 256);

		KillItem(itemNumber);
	}
	else
	{
		RemoveActiveItem(itemNumber);
	}

	DisableBaddieAI(itemNumber);
	item->flags |= IFLAG_KILLED | IFLAG_INVISIBLE;
	DropBaddyPickups(item);
}

__int32 __cdecl BadFloor(__int32 x, __int32 y, __int32 z, __int32 boxHeight, __int32 nextHeight, __int16 roomNumber, LOT_INFO* LOT)
{
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	
	if (floor->box == 0x7FF)
		return 1;

	if (LOT->isJumping)
		return 0;

	if (Boxes[floor->box].overlapIndex & LOT->blockMask)
		return 1;

	__int32 height = Boxes[floor->box].height;

	if (boxHeight - height > LOT->step)
		return 1;

	if (boxHeight - height < LOT->drop)
		return 1;

	if (boxHeight - height < -LOT->step && height > nextHeight)
		return 1;

	if (LOT->fly && y > height + LOT->fly)
		return 1;

	return 0;
}

__int32 __cdecl CreatureCreature(__int16 itemNumber)  
{
	__int32 x = Items[itemNumber].pos.xPos;
	__int32 z = Items[itemNumber].pos.zPos;

	ITEM_INFO* item;

	for (__int16 link = Rooms[Items[itemNumber].roomNumber].itemNumber; link != NO_ITEM; link = item->nextItem)
	{
		item = &Items[link];

		if (link != itemNumber && item != LaraItem && item->status == ITEM_ACTIVE && item->hitPoints > 0)
		{
			__int32 xdistance = abs(item->pos.xPos - x);
			__int32 zdistance = abs(item->pos.zPos - z);
			__int32 radius = xdistance <= zdistance ? zdistance + (xdistance >> 1) : xdistance + (zdistance >> 1);
			if (radius < Objects[Items[itemNumber].objectNumber].radius + Objects[item->objectNumber].radius)
			{
				__int32 yRot = Items[itemNumber].pos.yRot;
				return ATAN(item->pos.zPos - z, item->pos.xPos - x) - yRot;
			}
		}
	}

	return 0;
}

/*TARGET_TYPE CalculateTarget(PHD_VECTOR* target, ITEM_INFO* item, LOT_INFO* LOT)
{
	UNIMPLEMENTED();
	return NO_TARGET;
}*/

__int32 __cdecl ValidBox(ITEM_INFO* item, __int16 zoneNumber, __int16 boxNumber) 
{
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	__int16* zone = GroundZones[FlipStatus + 2 * creature->LOT.zone];

	if (creature->LOT.fly == 0 && zone[boxNumber] != zoneNumber)
		return 0;

	BOX_INFO* box = &Boxes[boxNumber];

	if (box->overlapIndex & creature->LOT.blockMask)
		return 0;

	if ((item->pos.zPos > (box->left * 1024)) || ((box->right * 1024) > item->pos.zPos) ||
		(item->pos.xPos > (box->top * 1024)) || ((box->bottom * 1024) > item->pos.xPos))
		return 1;

	return 0;
}

__int32 __cdecl EscapeBox(ITEM_INFO* item, ITEM_INFO* enemy, __int16 boxNumber) 
{
	BOX_INFO* box = &Boxes[boxNumber];
	__int32 x = ((box->top + box->bottom) << (WALL_SHIFT - 1)) - enemy->pos.xPos;
	__int32 z = ((box->left + box->right) << (WALL_SHIFT - 1)) - enemy->pos.zPos;
	
	if (x > -5120 && x < 5120 && z > -5120 && z < 5120)
		return 0;

	if (((x > 0) ^ (item->pos.xPos > enemy->pos.xPos)) && 
		((z > 0) ^ (item->pos.zPos > enemy->pos.zPos)))
		return 0;

	return 1;
}

void __cdecl TargetBox(LOT_INFO* LOT, __int16 boxNumber) 
{
	boxNumber &= 0x7FF;
	BOX_INFO* box = &Boxes[boxNumber];

	LOT->requiredBox = boxNumber;

	LOT->target.x = (((((box->bottom - box->top) - 1) * GetRandomControl()) / 32) + (box->top * 1024)) + 512;
	LOT->target.z = (((((box->right - box->left) - 1) * GetRandomControl()) / 32) + (box->left * 1024)) + 512;
	
	if (LOT->fly == 0)
		LOT->target.y = box->height;
	else
		LOT->target.y = box->height - 384;

	return;
}

__int32 __cdecl UpdateLOT(LOT_INFO* LOT, __int32 depth)
{
	BOX_NODE* node;

	if (LOT->requiredBox != NO_BOX && LOT->requiredBox != LOT->targetBox)
	{
		LOT->targetBox = LOT->requiredBox;

		node = &LOT->node[LOT->targetBox];
		if (node->nextExpansion == NO_BOX && LOT->tail != LOT->targetBox)
		{
			node->nextExpansion = LOT->head;

			if (LOT->head == NO_BOX)
				LOT->tail = LOT->targetBox;

			LOT->head = LOT->targetBox;
		}

		node->searchNumber = LOT->searchNumber++;
		node->exitBox = NO_BOX;
	}

	return SearchLOT(LOT, depth);
}

__int32 __cdecl SearchLOT(LOT_INFO* LOT, __int32 depth)
{
	__int16* zone = GroundZones[FlipStatus + 2 * LOT->zone];
	__int16 searchZone = zone[LOT->head];

	for (__int32 i = 0; i < depth; i++)
	{
		if (LOT->head == NO_BOX)
		{
			LOT->tail = NO_BOX; 
			return false;
		}

		BOX_NODE* node = &LOT->node[LOT->head];
		BOX_INFO* box = &Boxes[LOT->head];

		__int32 index = box->overlapIndex & OVERLAP_INDEX;
		bool done = false;
		do
		{
			__int16 boxNumber = Overlaps[index++];
			if (boxNumber & BOX_END_BIT)
			{
				done = true;
				boxNumber &= boxNumber;
			}

			if (LOT->fly == NO_FLYING && searchZone != zone[boxNumber])
				continue;

			__int32 delta = Boxes[boxNumber].height - box->height;
			if (delta > LOT->step || delta < LOT->drop)
				continue;

			BOX_NODE* expand = &LOT->node[boxNumber];
			if ((node->searchNumber & SEARCH_NUMBER) < (expand->searchNumber & SEARCH_NUMBER))
				continue;

			if (node->searchNumber & BLOCKED_SEARCH)
			{
				if ((node->searchNumber & SEARCH_NUMBER) == (expand->searchNumber & SEARCH_NUMBER))
					continue;

				expand->searchNumber = node->searchNumber;
			}
			else
			{
				if ((node->searchNumber & SEARCH_NUMBER) == (expand->searchNumber & SEARCH_NUMBER) &&
					!(expand->searchNumber & BLOCKED_SEARCH))
					continue;

				if (Boxes[boxNumber].overlapIndex & LOT->blockMask)
				{
					expand->searchNumber = node->searchNumber | BLOCKED_SEARCH;
				}
				else
				{
					expand->searchNumber = node->searchNumber;
					expand->exitBox = LOT->head;
				}
			}

			if (expand->nextExpansion == NO_BOX && boxNumber != LOT->tail)
			{
				LOT->node[LOT->tail].nextExpansion = boxNumber;
				LOT->tail = boxNumber;
			}
		} while (!done);

		LOT->head = node->nextExpansion;
		node->nextExpansion = NO_BOX;
	}

	return true;
}

__int32 __cdecl CreatureActive(__int16 itemNumber) 
{
	ITEM_INFO* item = &Items[itemNumber];

	if (!(item->flags & IFLAG_KILLED) && (item->status & ITEM_INVISIBLE) == ITEM_INVISIBLE)
	{
		if (!EnableBaddieAI(itemNumber, 0))
			return false;
		item->status = ITEM_ACTIVE;
	}

	return true;
}

void __cdecl InitialiseCreature(__int16 itemNumber) 
{
	ITEM_INFO* item = &Items[itemNumber];
	ROOM_INFO* room = &Rooms[item->roomNumber];

	item->collidable = true;
	item->data = NULL;
	item->drawRoom = (((item->pos.zPos - room->z) / 1024) & 0xFF) | (((item->pos.xPos - room->mesh->x) / 4) & 0xFF00);
	item->TOSSPAD = item->pos.yRot & 0xE000;
	item->itemFlags[2] = item->roomNumber | (item->pos.yPos - room->minfloor);
}

__int32 __cdecl StalkBox(ITEM_INFO* item, ITEM_INFO* enemy, __int16 boxNumber) 
{
	if (enemy == NULL)
		return 0;

	BOX_INFO* box = &Boxes[boxNumber];

	__int32 xrange = ((box->bottom - box->top + 3) << WALL_SHIFT);
	__int32 zrange = ((box->right - box->left + 3) << WALL_SHIFT);

	__int32 x = ((box->top + box->bottom) << (WALL_SHIFT - 1)) - enemy->pos.xPos;
	__int32 z = ((box->left + box->right) << (WALL_SHIFT - 1)) - enemy->pos.zPos;
	
	if (x > xrange || x < -xrange || z > zrange || z < -zrange)
		return 0;

	__int32 enemyQuad = (enemy->pos.yRot >> W2V_SHIFT) + 2;
	__int32 boxQuad = (z <= 0 ? (x <= 0 ? 0 : 3) : (x > 0) + 1);

	if (enemyQuad == boxQuad)
		return 0;

	__int32 baddieQuad = 0;
	if (item->pos.zPos > enemy->pos.zPos)
		baddieQuad = (item->pos.xPos > enemy->pos.xPos) + 1;
	else
		baddieQuad = item->pos.xPos <= enemy->pos.xPos ? 0 : 3;

	if (enemyQuad == baddieQuad && abs(enemyQuad - boxQuad) == 2)
		return 1;

	return 0;
}

__int32 CreatureVault(__int16 itemNum, __int16 angle, __int32 vault, __int32 shift)
{
	ITEM_INFO* item = &Items[itemNum];

	__int32 xBlock = item->pos.xPos >> WALL_SHIFT;
	__int32 y = item->pos.yPos;
	__int32 zBlock = item->pos.zPos >> WALL_SHIFT;
	__int32 roomNumber = item->roomNumber;

	CreatureAnimation(itemNum, angle, 0);

	if (item->floor > y + STEP_SIZE * 7 / 2)
		vault = -4;
	else if (item->floor > y + STEP_SIZE * 5)
		vault = -3;
	else if (item->floor > y + STEP_SIZE * 3 / 2 && item->objectNumber != ID_BADDY1 && item->objectNumber != ID_BADDY2) // Baddy 1&2 don't have some climb down animations
		vault = -2;
	else if (item->pos.yPos > y - STEP_SIZE * 3 / 2)
		return 0;
	else if (item->pos.yPos > y - STEP_SIZE * 5 / 2)
		vault = 2;
	else if (item->pos.yPos > y - STEP_SIZE * 7 / 2)
		vault = 3;
	else
		vault = 4;

	__int32 newXblock = item->pos.xPos >> WALL_SHIFT;
	__int32 newZblock = item->pos.zPos >> WALL_SHIFT;
	
	if (zBlock == newZblock)
	{
		if (xBlock == newXblock)
			return 0;

		if (xBlock < newXblock)
		{
			item->pos.xPos = (newXblock << WALL_SHIFT) - shift;
			item->pos.yRot = ANGLE(90);
		}
		else
		{
			item->pos.xPos = (xBlock << WALL_SHIFT) + shift;
			item->pos.yRot = -ANGLE(90);
		}
	}
	else if (xBlock == newXblock)
	{
		if (zBlock < newZblock)
		{
			item->pos.zPos = (newZblock << WALL_SHIFT) - shift;
			item->pos.yRot = 0;
		}
		else
		{
			item->pos.zPos = (zBlock << WALL_SHIFT) + shift;
			item->pos.yRot = -ANGLE(180);
		}
	}

	item->pos.yPos = item->floor = y;
	if (roomNumber != item->roomNumber)
		ItemNewRoom(itemNum, roomNumber);

	return vault;
}

void __cdecl GetAITarget(CREATURE_INFO* creature)
{
	ITEM_INFO* enemy = creature->enemy;
	__int16 enemyObjectNumber;
	if (enemy)
		enemyObjectNumber = enemy->objectNumber;
	else
		enemyObjectNumber = -1;

	ITEM_INFO* item = &Items[creature->itemNum];
	
	if (item->aiBits & GUARD)
	{
		if (creature->alerted)
		{
			item->aiBits = ~GUARD;
			if (item->aiBits & AMBUSH)
				item->aiBits |= MODIFY;
		}
	} 
	else if (item->aiBits & PATROL1)
	{
		if (creature->alerted || creature->hurtByLara)
		{
			item->aiBits &= ~PATROL1;  
			if (item->aiBits & AMBUSH)
			{
				item->aiBits |= MODIFY;
				item->itemFlags[3] = item->pad2[6];
			}
		}
		else if (enemyObjectNumber == ID_AI_PATROL1)
		{
			if (abs(enemy->pos.xPos - item->pos.xPos) < 640 &&
				abs(enemy->pos.zPos - item->pos.zPos) < 640 &&
				abs(enemy->pos.yPos - item->pos.yPos) < 640 || Objects[item->objectNumber].waterCreature)
				creature->reachedGoal = true;
		}
		else
		{
			FindAITargetObject(creature, ID_AI_PATROL1);
		}
	}
	else  if (item->aiBits & AMBUSH)
	{
		if (enemyObjectNumber == ID_AI_AMBUSH)
		{
			if (abs(enemy->pos.xPos - item->pos.xPos) < 640 &&
				abs(enemy->pos.zPos - item->pos.zPos) < 640 &&
				abs(enemy->pos.yPos - item->pos.yPos) < 640 || Objects[item->objectNumber].waterCreature)
			{
				FLOOR_INFO* floor = GetFloor(enemy->pos.xPos, enemy->pos.yPos, enemy->pos.zPos, &(enemy->roomNumber));
				GetFloorHeight(floor, enemy->pos.xPos, enemy->pos.yPos, enemy->pos.zPos);
				TestTriggers(TriggerIndex, 1, 0);

				creature->reachedGoal = true; 
				creature->enemy = LaraItem;
				item->aiBits &= ~AMBUSH; 
				if (!(item->aiBits & MODIFY))
				{
					item->aiBits |= GUARD;
					creature->alerted = false;
				}
			}
		}
		else
		{
			FindAITargetObject(creature, ID_AI_AMBUSH);
		}
	} 
	else if (item->aiBits & FOLLOW)
	{
		if (creature->hurtByLara & 0x10)
		{
			creature->enemy = LaraItem;
			creature->alerted = true;
			item->aiBits &= ~FOLLOW;
		}
		else if (item->hitStatus)
			item->aiBits &= ~FOLLOW;
		else if (enemyObjectNumber != ID_AI_FOLLOW)
			FindAITargetObject(creature, ID_AI_FOLLOW);
		else if (abs(enemy->pos.xPos - item->pos.xPos) < 640 &&
			abs(enemy->pos.zPos - item->pos.zPos) < 640 &&
			abs(enemy->pos.yPos - item->pos.yPos) < 1280)
		{
			creature->reachedGoal = true;
			item->aiBits &= ~FOLLOW;
		}
	}
}

void __cdecl FindAITargetObject(CREATURE_INFO* creature, __int16 objectNumber)
{
	ITEM_INFO* item = &Items[creature->itemNum];

	if (nAIObjects > 0)
	{
		AIOBJECT* foundObject = NULL;

		for (__int32 i = 0; i < nAIObjects; i++)
		{
			AIOBJECT* aiObject = &AIObjects[i];

			if (aiObject->objectNumber == objectNumber && aiObject->triggerFlags == item->itemFlags[3] && aiObject->roomNumber != 255)
			{
				__int16* zone = GroundZones[FlipStatus + 2 * creature->LOT.zone];

				ROOM_INFO* r = &Rooms[item->roomNumber];
				item->boxNumber = XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z).box & 0x7FF;

				r = &Rooms[aiObject->roomNumber];
				aiObject->boxNumber = XZ_GET_SECTOR(r, aiObject->x - r->x, aiObject->z - r->z).box & 0x7FF;

				if (zone[item->boxNumber] == zone[aiObject->boxNumber])
				{
					foundObject = aiObject;
					break;
				}
			}
		}

		if (foundObject != NULL)
		{
			ITEM_INFO* aiItem = &creature->aiTarget;

			creature->enemy = aiItem;

			aiItem->objectNumber = foundObject->objectNumber;
			aiItem->roomNumber = foundObject->roomNumber;
			aiItem->pos.xPos = foundObject->x;
			aiItem->pos.yPos = foundObject->y;
			aiItem->pos.zPos = foundObject->z;
			aiItem->pos.yRot = foundObject->yRot;
			aiItem->flags = foundObject->flags;
			aiItem->triggerFlags = foundObject->triggerFlags;
			aiItem->boxNumber = foundObject->boxNumber;

			if (!(creature->aiTarget.flags & 0x20))
			{
				creature->aiTarget.pos.xPos += SIN(creature->aiTarget.pos.yRot) >> 4;   
				creature->aiTarget.pos.zPos += COS(creature->aiTarget.pos.yRot) >> 4;
			}
		}
	}
}

void __cdecl CreatureAIInfo(ITEM_INFO* item, AI_INFO* info)
{
	if (item->data == NULL)
		return;

	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	ITEM_INFO* enemy = creature->enemy;

	if (!enemy)
	{
		enemy = LaraItem;  
		creature->enemy = LaraItem;
	}

	__int16* zone = GroundZones[FlipStatus + 2 * creature->LOT.zone];
	ROOM_INFO* r = &Rooms[item->roomNumber];
	item->boxNumber = XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z).box & 0x7FF;
	info->zoneNumber = zone[item->boxNumber];
	
	r = &Rooms[enemy->roomNumber];
	enemy->boxNumber = XZ_GET_SECTOR(r, enemy->pos.xPos - r->x, enemy->pos.zPos - r->z).box & 0x7FF;
	info->enemyZone = zone[enemy->boxNumber];

	if ((Boxes[enemy->boxNumber].overlapIndex & creature->LOT.blockMask) ||
		creature->LOT.node[item->boxNumber].searchNumber == (creature->LOT.searchNumber | BLOCKED_SEARCH))
		info->enemyZone |= BLOCKED;

	OBJECT_INFO* object = &Objects[item->objectNumber];
	__int32 x;
	__int32 z;

	if (enemy == LaraItem)
	{
		x = (enemy->pos.xPos + (enemy->speed * 14 * SIN(Lara.moveAngle) >> W2V_SHIFT)) - (item->pos.xPos + (object->pivotLength * SIN(item->pos.yRot) >> W2V_SHIFT));
		z = (enemy->pos.zPos + (enemy->speed * 14 * COS(Lara.moveAngle) >> W2V_SHIFT)) - (item->pos.zPos + (object->pivotLength * COS(item->pos.yRot) >> W2V_SHIFT));
	}
	else
	{
		x = (enemy->pos.xPos + (enemy->speed * 14 * SIN(enemy->pos.yRot) >> W2V_SHIFT)) - (item->pos.xPos + (object->pivotLength * SIN(item->pos.yRot) >> W2V_SHIFT));
		z = (enemy->pos.zPos + (enemy->speed * 14 * COS(enemy->pos.yRot) >> W2V_SHIFT)) - (item->pos.zPos + (object->pivotLength * COS(item->pos.yRot) >> W2V_SHIFT));
	}

	__int32 y = item->pos.yPos - enemy->pos.yPos;
	__int32 angle = ATAN(z, x);

	if (x > 32000 || x < -32000 || z > 32000 || z < -32000)
		info->distance = 0x7FFFFFFF;
	else if (creature->enemy)
		info->distance = SQUARE(x) + SQUARE(z);
	else
		info->distance = 0x7FFFFFFF;

	info->angle = angle - item->pos.yRot;
	info->enemyFacing = -ANGLE(180) + angle - enemy->pos.yRot;

	x = abs(x);
	z = abs(z);
	
	if (!(enemy != LaraItem
		|| LaraItem->currentAnimState != STATE_LARA_CROUCH_IDLE
		&& LaraItem->currentAnimState != STATE_LARA_CROUCH_ROLL
		&& (LaraItem->currentAnimState <= STATE_LARA_MONKEYSWING_TURNAROUND || 
			LaraItem->currentAnimState >= STATE_LARA_CLIMB_TO_CRAWL)
		&& LaraItem->currentAnimState != STATE_LARA_CROUCH_TURN_LEFT
		&& LaraItem->currentAnimState != STATE_LARA_CROUCH_TURN_RIGHT))
	{
		y -= 384;
	}

	if (x > z)
		info->xAngle = ATAN(x + (z >> 1), y);
	else
		info->xAngle = ATAN(z + (x >> 1), y);

	info->ahead = (info->angle > -ANGLE(90) && info->angle < ANGLE(90));
	info->bite = (info->ahead && enemy->hitPoints > 0 && abs(enemy->pos.yPos - item->pos.yPos) <= 512);
}

void __cdecl CreatureMood(ITEM_INFO* item, AI_INFO* info, __int32 violent)
{
	__int32 boxNumber;
	__int16* bounds;

	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	if (creature)
	{
		ITEM_INFO* enemy = creature->enemy;
		LOT_INFO* LOT = &creature->LOT;

		switch (creature->mood)
		{
		case BORED_MOOD:
			boxNumber = LOT->node[GetRandomControl() * LOT->zoneCount >> 15].boxNumber;
			if (ValidBox(item, info->zoneNumber, boxNumber) && !(GetRandomControl() & 0xF))
			{
				if (StalkBox(item, enemy, boxNumber) && enemy->hitPoints > 0 && creature->enemy)
				{
					TargetBox(LOT, boxNumber);
					creature->mood = BORED_MOOD;
				}
				else if (LOT->requiredBox == NO_BOX)
					TargetBox(LOT, boxNumber);
			}
			break;

		case ATTACK_MOOD:
			LOT->target.x = enemy->pos.xPos;
			LOT->target.y = enemy->pos.yPos;
			LOT->target.z = enemy->pos.zPos;
			LOT->requiredBox = enemy->boxNumber;

			if (LOT->fly != NO_FLYING && !Lara.waterStatus)
			{
				bounds = GetBestFrame(enemy);
				LOT->target.y += bounds[2];
			}
			break;

		case ESCAPE_MOOD:
			boxNumber = LOT->node[GetRandomControl() * LOT->zoneCount >> 15].boxNumber;
			if (ValidBox(item, info->zoneNumber, boxNumber) && LOT->requiredBox == NO_BOX)
			{
				if (EscapeBox(item, enemy, boxNumber))
					TargetBox(LOT, boxNumber);
				else if (info->zoneNumber == info->enemyZone && StalkBox(item, enemy, boxNumber) && !violent)
				{
					TargetBox(LOT, boxNumber);
					creature->mood = STALK_MOOD;
				}
			}
			break;

		case STALK_MOOD:
			if (LOT->requiredBox == NO_BOX || !StalkBox(item, enemy, LOT->requiredBox))
			{
				boxNumber = LOT->node[GetRandomControl() * LOT->zoneCount >> 15].boxNumber;
				if (ValidBox(item, info->zoneNumber, boxNumber))
				{
					if (StalkBox(item, enemy, boxNumber))
						TargetBox(LOT, boxNumber);
					else if (LOT->requiredBox == NO_BOX)
					{
						TargetBox(LOT, boxNumber);
						if (info->zoneNumber != info->enemyZone)
							creature->mood = BORED_MOOD;
					}
				}
			}
			break;

		}

		if (LOT->targetBox == NO_BOX)
			TargetBox(LOT, item->boxNumber);

		Unk_00EEFB6C = CalculateTarget(&creature->target, item, &creature->LOT);
		
		creature->jumpAhead = false;
		creature->monkeyAhead = false;
		
		__int32 startBox = LOT->node[item->boxNumber].exitBox;
		if (startBox != NO_BOX)
		{
			__int32 overlapIndex = Boxes[startBox].overlapIndex & OVERLAP_INDEX;
			__int32 nextBox = 0;
			do
			{
				overlapIndex++;
				nextBox = Overlaps[overlapIndex];
			} while (nextBox != NO_BOX && ((nextBox & 0x8000) == 0) && ((nextBox & 0x7FF) != startBox));
			if ((nextBox & 0x7FF) == startBox)
			{
				if (nextBox & 0x800)
					creature->jumpAhead = true;
				if (nextBox & 0x2000)
					creature->monkeyAhead = true;
			}
		}
	}
}

void __cdecl GetCreatureMood(ITEM_INFO* item, AI_INFO* info, __int32 violent)
{
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	if (creature)
	{
		LOT_INFO* LOT = &creature->LOT;
		ITEM_INFO* enemy = creature->enemy;
		if (creature->LOT.node[item->boxNumber].searchNumber == (creature->LOT.searchNumber | 0x8000))
			creature->LOT.requiredBox = NO_BOX;
		
		if (creature->mood != ATTACK_MOOD && creature->LOT.requiredBox != NO_BOX)
		{
			if (!ValidBox(item, info->zoneNumber, creature->LOT.targetBox))
			{
				if (info->zoneNumber == info->enemyZone)
					creature->mood = BORED_MOOD;
				creature->LOT.requiredBox = NO_BOX;
			}
		}
		
		__int32 mood = creature->mood;

		if (enemy)
		{
			if (enemy->hitPoints > 0 || enemy != LaraItem)
			{
				if (violent)
				{
					switch (creature->mood)
					{
					case BORED_MOOD:
					case STALK_MOOD:
						if (info->zoneNumber == info->enemyZone)
							creature->mood = ATTACK_MOOD;
						else if (item->hitStatus)
							creature->mood = ESCAPE_MOOD;
						break;

					case ATTACK_MOOD:
						if (info->zoneNumber != info->enemyZone)
							creature->mood = BORED_MOOD;
						break;

					case ESCAPE_MOOD:
						if (info->zoneNumber == info->enemyZone)
							creature->mood = ATTACK_MOOD;
						break;
					}
				}
				else
				{
					switch (creature->mood)
					{
					case BORED_MOOD:
					case STALK_MOOD:
						if (creature->alerted && info->zoneNumber != info->enemyZone)
							creature->mood = info->distance <= 3072 ? BORED_MOOD : STALK_MOOD;
						else if (info->zoneNumber == info->enemyZone)
						{
							if (info->distance < SQUARE(3072) || creature->mood == STALK_MOOD && LOT->requiredBox == NO_BOX)
								creature->mood = ATTACK_MOOD;
							else
								creature->mood = STALK_MOOD;
						}
						break;

					case ATTACK_MOOD:
						if (item->hitStatus && (GetRandomControl() < 2048 || info->zoneNumber != info->enemyZone))
							creature->mood = STALK_MOOD;
						else if (info->zoneNumber != info->enemyZone && info->distance > 6144)
							creature->mood = BORED_MOOD;
						break;

					case ESCAPE_MOOD:
						if (info->zoneNumber == info->enemyZone)
						{
							if (GetRandomControl() < 256)
								creature->mood = STALK_MOOD;
						}
						break;

					}
				}
			}
			else
			{
				creature->mood = BORED_MOOD;
			}
		}
		else
		{
			creature->mood = BORED_MOOD;
		}

		if (mood != creature->mood)
		{
			if (mood == ATTACK_MOOD)
				TargetBox(LOT, LOT->targetBox);
			LOT->requiredBox = NO_BOX;
		}
	}
}

TARGET_TYPE __cdecl CalculateTarget(PHD_VECTOR* target, ITEM_INFO* item, LOT_INFO* LOT)
{
	UpdateLOT(LOT, 5);

	target->x = item->pos.xPos;
	target->y = item->pos.yPos;
	target->z = item->pos.zPos;

	__int32 boxNumber = item->boxNumber;
	if (boxNumber == NO_BOX)
		return (NO_TARGET);

	BOX_INFO* box = &Boxes[boxNumber];

	__int32 boxLeft = (__int32)box->left << WALL_SHIFT;
	__int32 boxRight = ((__int32)box->right << WALL_SHIFT) - 1;
	__int32 boxTop = (__int32)box->top << WALL_SHIFT;
	__int32 boxBottom = ((__int32)box->bottom << WALL_SHIFT) - 1;

	__int32 left = boxLeft;
	__int32 right = boxRight;
	__int32 top = boxTop;
	__int32 bottom = boxBottom;

	__int32 direction = ALL_CLIP;

	do {
		box = &Boxes[boxNumber];

		if (LOT->fly == NO_FLYING)
		{
			if (target->y > box->height)
				target->y = box->height;
		}
		else
		{
			if (target->y > box->height - WALL_SIZE)
				target->y = box->height - WALL_SIZE;
		}

		boxLeft = (__int32)box->left << WALL_SHIFT;
		boxRight = ((__int32)box->right << WALL_SHIFT) - 1;
		boxTop = (__int32)box->top << WALL_SHIFT;
		boxBottom = ((__int32)box->bottom << WALL_SHIFT) - 1;

		if (item->pos.zPos >= boxLeft && item->pos.zPos <= boxRight &&
			item->pos.xPos >= boxTop && item->pos.xPos <= boxBottom)
		{
			left = boxLeft;
			right = boxRight;
			top = boxTop;
			bottom = boxBottom;
		}
		else
		{
			if (item->pos.zPos < boxLeft)
			{
				if ((direction & CLIP_LEFT) && item->pos.xPos >= boxTop && item->pos.xPos <= boxBottom)
				{
					if (target->z < boxLeft + 512)
						target->z = boxLeft + 512;

					if (direction & SECONDARY_CLIP)
						return (SECONDARY_TARGET);

					if (boxTop > top)
						top = boxTop;
					if (boxBottom < bottom)
						bottom = boxBottom;

					direction = CLIP_LEFT;
				}
				else if (direction != CLIP_LEFT)
				{
					target->z = right - 512;
					if (direction != ALL_CLIP)
						return (SECONDARY_TARGET);

					direction |= SECONDARY_CLIP;
				}
			}
			else if (item->pos.zPos > boxRight)
			{
				if ((direction & CLIP_RIGHT) && item->pos.xPos >= boxTop && item->pos.xPos <= boxBottom)
				{
					if (target->z > boxRight - 512)
						target->z = boxRight - 512;

					if (direction & SECONDARY_CLIP)
						return (SECONDARY_TARGET);

					if (boxTop > top)
						top = boxTop;
					if (boxBottom < bottom)
						bottom = boxBottom;

					direction = CLIP_RIGHT;
				}
				else if (direction != CLIP_RIGHT)
				{
					target->z = left + 512;
					if (direction != ALL_CLIP)
						return (SECONDARY_TARGET);

					direction |= SECONDARY_CLIP;
				}
			}

			if (item->pos.xPos < boxTop)
			{
				if ((direction & CLIP_TOP) && item->pos.zPos >= boxLeft && item->pos.zPos <= boxRight)
				{
					if (target->x < boxTop + 512)
						target->x = boxTop + 512;

					if (direction & SECONDARY_CLIP)
						return (SECONDARY_TARGET);

					if (boxLeft > left)
						left = boxLeft;
					if (boxRight < right)
						right = boxRight;

					direction = CLIP_TOP;
				}
				else if (direction != CLIP_TOP)
				{
					target->x = bottom - 512;
					if (direction != ALL_CLIP)
						return (SECONDARY_TARGET);

					direction |= SECONDARY_CLIP;
				}
			}
			else if (item->pos.xPos > boxBottom)
			{
				if ((direction & CLIP_BOTTOM) && item->pos.zPos >= boxLeft && item->pos.zPos <= boxRight)
				{
					if (target->x > boxBottom - 512)
						target->x = boxBottom - 512;

					if (direction & SECONDARY_CLIP)
						return (SECONDARY_TARGET);

					if (boxLeft > left)
						left = boxLeft;
					if (boxRight < right)
						right = boxRight;

					direction = CLIP_BOTTOM;
				}
				else if (direction != CLIP_BOTTOM)
				{
					target->x = top + 512;
					if (direction != ALL_CLIP)
						return (SECONDARY_TARGET);

					direction |= SECONDARY_CLIP;
				}
			}
		}

		if (boxNumber == LOT->targetBox)
		{
			if (direction & (CLIP_LEFT | CLIP_RIGHT))
				target->z = LOT->target.z;
			else if (!(direction & SECONDARY_CLIP))
			{
				if (target->z < boxLeft + 512)
					target->z = boxLeft + 512;
				else if (target->z > boxRight - 512)
					target->z = boxRight - 512;
			}

			if (direction & (CLIP_TOP | CLIP_BOTTOM))
				target->x = LOT->target.x;
			else if (!(direction & SECONDARY_CLIP))
			{
				if (target->x < boxTop + 512)
					target->x = boxTop + 512;
				else if (target->x > boxBottom - 512)
					target->x = boxBottom - 512;
			}

			target->y = LOT->target.y;

			return (PRIME_TARGET);
		}

		boxNumber = LOT->node[boxNumber].exitBox;
		if (boxNumber != NO_BOX && (Boxes[boxNumber].overlapIndex & LOT->blockMask))
			break;
	} while (boxNumber != NO_BOX);

	if (direction & (CLIP_LEFT | CLIP_RIGHT))
		target->z = boxLeft + WALL_SIZE / 2 + (GetRandomControl() * (boxRight - boxLeft - WALL_SIZE) >> 15);
	else if (!(direction & SECONDARY_CLIP))
	{
		if (target->z < boxLeft + 512)
			target->z = boxLeft + 512;
		else if (target->z > boxRight - 512)
			target->z = boxRight - 512;
	}

	if (direction & (CLIP_TOP | CLIP_BOTTOM))
		target->x = boxTop + WALL_SIZE / 2 + (GetRandomControl() * (boxBottom - boxTop - WALL_SIZE) >> 15);
	else if (!(direction & SECONDARY_CLIP))
	{
		if (target->x < boxTop + 512)
			target->x = boxTop + 512;
		else if (target->x > boxBottom - 512)
			target->x = boxBottom - 512;
	}

	if (LOT->fly == NO_FLYING)
		target->y = box->height;
	else
		target->y = box->height - 320;

	return NO_TARGET;
}

void Inject_Box()
{
	INJECT(0x0040B5D0, CreatureVault);
	INJECT(0x00408FD0, ValidBox);
	INJECT(0x00408E20, TargetBox);
	INJECT(0x00409770, StalkBox);
	INJECT(0x00408EF0, EscapeBox);
	INJECT(0x0040C2D0, SameZone);
	INJECT(0x0040B820, CreatureKill);
	INJECT(0x00409FB0, BadFloor);
	INJECT(0x0040B550, CreatureEffect2);
	INJECT(0x0040B4D0, CreatureEffect);
	INJECT(0x0040C5A0, DropBaddyPickups);
	INJECT(0x0040A090, CreatureDie);
	INJECT(0x0040B240, CreatureJoint);
	INJECT(0x0040B1B0, CreatureTilt);
	INJECT(0x00409E20, CreatureCreature);
	INJECT(0x00408630, CreatureActive);
	INJECT(0x0040C460, MoveCreature3DPos);
	INJECT(0x004086C0, CreatureAIInfo);
	INJECT(0x00409370, CreatureMood);
	INJECT(0x004090A0, GetCreatureMood);
	INJECT(0x0040BB10, AlertNearbyGuards);
	INJECT(0x0040BA70, AlertAllGuards);
	INJECT(0x00408B00, UpdateLOT);
	INJECT(0x0040B400, CreatureUnderwater);
	INJECT(0x0040B2C0, CreatureFloat);
	INJECT(0x004098B0, CalculateTarget);
	INJECT(0x0040A1D0, CreatureAnimation);
	INJECT(0x00408550, InitialiseCreature);
	INJECT(0x0040C070, FindAITargetObject);
	INJECT(0x0040BBE0, AIGuard);
	INJECT(0x0040BCC0, GetAITarget);

	/*+
	//INJECT(0x0040AE90, CreatureTurn);+*/
}