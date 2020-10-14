#include "framework.h"
#include "box.h"
#include "tomb4fx.h"
#include "lot.h"
#include "Lara.h"
#include "draw.h"
#include "sphere.h"
#include "misc.h"
#include "camera.h"
#include "control.h"
#include "setup.h"
#include "trmath.h"
#include "objectslist.h"

#define CHECK_CLICK(x) CLICK(x) / 2
#define ESCAPE_DIST SECTOR(5)
#define STALK_DIST SECTOR(3)
#define REACHED_GOAL_RADIUS 640
#define ATTACK_RANGE SQUARE(SECTOR(3))
#define ESCAPE_CHANCE  0x800
#define RECOVER_CHANCE 0x100
#define BIFF_AVOID_TURN 1536
#define FEELER_DISTANCE 512
#define FEELER_ANGLE ANGLE(45.0f)

void DropBaddyPickups(ITEM_INFO* item)
{
	ITEM_INFO* pickup = NULL;
	FLOOR_INFO* floor;
	short roomNumber;
	BOUNDING_BOX* bounds;

	for (short pickupNumber = item->carriedItem; pickupNumber != NO_ITEM; pickupNumber = pickup->carriedItem)
	{
		pickup = &g_Level.Items[pickupNumber];
		pickup->pos.xPos = (item->pos.xPos & -CLICK(2)) | CLICK(2);
		pickup->pos.zPos = (item->pos.zPos & -CLICK(2)) | CLICK(2);

		roomNumber = item->roomNumber;
		floor = GetFloor(pickup->pos.xPos, item->pos.yPos, pickup->pos.zPos, &roomNumber);
		pickup->pos.yPos = GetFloorHeight(floor, pickup->pos.xPos, item->pos.yPos, pickup->pos.zPos);
		bounds = GetBoundsAccurate(pickup);
		pickup->pos.yPos -= bounds->Y2;

		ItemNewRoom(pickupNumber, item->roomNumber);
		pickup->flags |= 32;
	}
}

int MoveCreature3DPos(PHD_3DPOS* srcpos, PHD_3DPOS* destpos, int velocity, short angdif, int angadd)
{
	int x, y, z, distance;

	x = destpos->xPos - srcpos->xPos;
	y = destpos->yPos - srcpos->yPos;
	z = destpos->zPos - srcpos->zPos;
	distance = sqrt(SQUARE(x) + SQUARE(y) + SQUARE(z));

	if (velocity < distance)
	{
		srcpos->xPos += x * velocity / distance;
		srcpos->yPos += y * velocity / distance;
		srcpos->zPos += z * velocity / distance;
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

	return (srcpos->xPos == destpos->xPos
		&&  srcpos->yPos == destpos->yPos
		&&  srcpos->zPos == destpos->zPos
		&&  srcpos->yRot == destpos->yRot);
}

void CreatureYRot2(PHD_3DPOS* srcpos, short angle, short angadd) 
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
}

short SameZone(CREATURE_INFO* creature, ITEM_INFO* targetItem) 
{
	ITEM_INFO* item = &g_Level.Items[creature->itemNum];
	ROOM_INFO* r;
	int* zone;

	r = &g_Level.Rooms[item->roomNumber];
	item->boxNumber = XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z).box;

	r = &g_Level.Rooms[targetItem->roomNumber];
	targetItem->boxNumber = XZ_GET_SECTOR(r, targetItem->pos.xPos - r->x, targetItem->pos.zPos - r->z).box;

	zone = g_Level.Zones[creature->LOT.zone][FlipStatus].data();
	return zone[item->boxNumber] == zone[targetItem->boxNumber];
}

short AIGuard(CREATURE_INFO* creature) 
{
	ITEM_INFO* item;
	int random;

	item = &g_Level.Items[creature->itemNum];
	if (item->aiBits & (GUARD | PATROL1))
		return 0;

	random = GetRandomControl();

	if (random < 256)
	{
		creature->headRight = true;
		creature->headLeft = true;
	}
	else if (random < 384)
	{
		creature->headRight = false;
		creature->headLeft = true;
	}
	else if (random < 512)
	{
		creature->headRight = true;
		creature->headLeft = false;
	}

	if (!creature->headLeft)
		return (creature->headRight) * 4096;

	if (creature->headRight)
		return 0;

	return -ANGLE(90.0f);
}

void AlertNearbyGuards(ITEM_INFO* item) 
{
	ITEM_INFO* target;
	CREATURE_INFO* creature;
	int x, y, z;
	int distance;

	for (int i = 0; i < NUM_SLOTS; i++)
	{
		creature = &BaddieSlots[i];
		if (creature->itemNum == NO_ITEM)
			continue;

		target = &g_Level.Items[creature->itemNum + i];
		if (item->roomNumber == target->roomNumber)
		{
			creature->alerted = true;
			continue;
		}

		x = (target->pos.xPos - item->pos.xPos) / 64;
		y = (target->pos.yPos - item->pos.yPos) / 64;
		z = (target->pos.zPos - item->pos.zPos) / 64;

		distance = (SQUARE(x) + SQUARE(y) + SQUARE(z));
		if (distance < SECTOR(8))
			creature->alerted = true;
	}
}

void AlertAllGuards(short itemNumber) 
{
	ITEM_INFO* target;
	CREATURE_INFO* creature;
	short objNumber;

	for (int i = 0; i < NUM_SLOTS; i++)
	{
		creature = &BaddieSlots[i];
		if (creature->itemNum == NO_ITEM)
			continue;

		target = &g_Level.Items[creature->itemNum];
		objNumber = g_Level.Items[itemNumber].objectNumber;
		if (objNumber == target->objectNumber)
		{
			if (target->status == ITEM_ACTIVE)
				creature->alerted = true;
		}
	}
}

void CreatureKill(ITEM_INFO* item, int killAnim, int killState, short laraKillState)
{
	item->animNumber = Objects[item->objectNumber].animIndex + killAnim;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->currentAnimState = killState;

	LaraItem->animNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex;
	LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
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

	Lara.ExtraAnim = 1;
	Lara.gunStatus = LG_HANDS_BUSY;
	Lara.gunType = WEAPON_NONE;
	Lara.hitDirection = -1;
	Lara.air = -1;

	Camera.pos.roomNumber = LaraItem->roomNumber; 
	Camera.type = CHASE_CAMERA;
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(170.0f);
	Camera.targetElevation = -ANGLE(25.0f);

	// TODO: exist in TR5 but just commented in case.
	/*
	ForcedFixedCamera.x = item->pos.xPos + (phd_sin(item->pos.yRot) << 13) >> W2V_SHIFT;
	ForcedFixedCamera.y = item->pos.yPos - WALL_SIZE;
	ForcedFixedCamera.z = item->pos.zPos + (phd_cos(item->pos.yRot) << 13) >> W2V_SHIFT;
	ForcedFixedCamera.roomNumber = item->roomNumber;
	UseForcedFixedCamera = true;
	*/
}

short CreatureEffect2(ITEM_INFO* item, BITE_INFO* bite, short damage, short angle, std::function<CreatureEffectFunction> func)
{
	PHD_VECTOR pos;
	pos.x = bite->x;
	pos.y = bite->y;
	pos.z = bite->z;
	GetJointAbsPosition(item, &pos, bite->meshNum);
	return func(pos.x, pos.y, pos.z, damage, angle, item->roomNumber);
}

short CreatureEffect(ITEM_INFO* item, BITE_INFO* bite, std::function<CreatureEffectFunction> func)
{
	PHD_VECTOR pos;
	pos.x = bite->x;
	pos.y = bite->y;
	pos.z = bite->z;
	GetJointAbsPosition(item, &pos, bite->meshNum);
	return func(pos.x, pos.y, pos.z, item->speed, item->pos.yRot, item->roomNumber);
}

void CreatureUnderwater(ITEM_INFO* item, int depth)
{
	FLOOR_INFO* floor;
	short roomNumber;
	int height;
	int waterLevel = depth;
	int wh = 0;

	waterLevel = depth;
	wh = 0;

	if (depth < 0)
	{
		wh = abs(depth);
		waterLevel = 0;
	}
	else
	{
		wh = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);
	}

	int y = wh + waterLevel;

	if (item->pos.yPos < y)
	{
		roomNumber = item->roomNumber;
		floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

		item->pos.yPos = y;
		if (y > height)
			item->pos.yPos = height;

		if (item->pos.xRot > ANGLE(2.0f))
			item->pos.xRot -= ANGLE(2.0f);
		else if (item->pos.xRot > 0)
			item->pos.xRot = 0;
	}
}

void CreatureFloat(short itemNumber) 
{
	ITEM_INFO* item;
	FLOOR_INFO* floor;
	int waterLevel;
	int y;
	short roomNumber;

	item = &g_Level.Items[itemNumber];
	item->hitPoints = NOT_TARGETABLE;
	item->pos.xRot = 0;

	waterLevel = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber);

	y = item->pos.yPos;
	if (y > waterLevel)
		item->pos.yPos = y - 32;
	if (item->pos.yPos < waterLevel)
		item->pos.yPos = waterLevel;

	AnimateItem(item);

	roomNumber = item->roomNumber;
	floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	
	if (roomNumber != item->roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	if (item->pos.yPos <= waterLevel)
	{
		if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
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

void CreatureJoint(ITEM_INFO* item, short joint, short required) 
{
	if (item->data == NULL)
		return;

	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	short change = required - creature->jointRotation[joint];
	if (change > ANGLE(3))
		change = ANGLE(3);
	else if (change < -ANGLE(3))
		change = -ANGLE(3);

	creature->jointRotation[joint] += change;

	if (creature->jointRotation[joint] > ANGLE(70))
		creature->jointRotation[joint] = ANGLE(70);
	else if (creature->jointRotation[joint] < -ANGLE(70))
		creature->jointRotation[joint] = -ANGLE(70);
}

void CreatureTilt(ITEM_INFO* item, short angle) 
{
	angle = (angle * 4) - item->pos.zRot;

	if (angle < -ANGLE(3))
		angle = -ANGLE(3);
	else if (angle > ANGLE(3))
		angle = ANGLE(3);

	short theAngle = -ANGLE(3);

	short absRot = abs(item->pos.zRot);
	if (absRot < ANGLE(15) || absRot > ANGLE(30))
		angle /= 2;
	
	item->pos.zRot += angle;
}

short CreatureTurn(ITEM_INFO* item, short maximumTurn)
{
	if (item->data == NULL || maximumTurn == 0)
		return 0;

	CREATURE_INFO* creature;
	int x, z, range, distance;
	short angle;

	creature = (CREATURE_INFO*)item->data;
	angle = 0;

	x = creature->target.x - item->pos.xPos;
	z = creature->target.z - item->pos.zPos;
	angle = phd_atan(z, x) - item->pos.yRot;
	range = item->speed * 16384 / maximumTurn;
	distance = SQUARE(x) + SQUARE(z);

	if (angle > FRONT_ARC || angle < -FRONT_ARC && distance < SQUARE(range))
		maximumTurn /= 2;

	if (angle > maximumTurn)
		angle = maximumTurn;
	else if (angle < -maximumTurn)
		angle = -maximumTurn;

	item->pos.yRot += angle;

	return angle;
}

int CreatureAnimation(short itemNumber, short angle, short tilt)
{
	ITEM_INFO* item;
	CREATURE_INFO* creature;
	LOT_INFO* LOT;
	FLOOR_INFO* floor;
	PHD_VECTOR old;
	int xPos, zPos, x, y, z, ceiling, shiftX, shiftZ, dy;
	BOUNDING_BOX* bounds;
	int* zone;
	short roomNumber, radius, biffAngle, top;
	int boxHeight, height, nextHeight, nextBox;

	item = &g_Level.Items[itemNumber];
	if (item->data == NULL)
		return false;

	creature = (CREATURE_INFO*)item->data;
	LOT = &creature->LOT;
	zone = g_Level.Zones[LOT->zone][FlipStatus].data();
	boxHeight = g_Level.Boxes[item->boxNumber].height;
	old.x = item->pos.xPos;
	old.y = item->pos.yPos;
	old.z = item->pos.zPos;
	
	if (!Objects[item->objectNumber].waterCreature)
	{
		roomNumber = item->roomNumber;
		GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		if (roomNumber != item->roomNumber)
			ItemNewRoom(itemNumber, roomNumber);
	}

	AnimateItem(item);
	if (item->status == ITEM_DEACTIVATED)
	{
		CreatureDie(itemNumber, FALSE);
		return false;
	}

	bounds = GetBoundsAccurate(item);
	y = item->pos.yPos + bounds->Y1;

	roomNumber = item->roomNumber;
	GetFloor(old.x, y, old.z, &roomNumber);  
	floor = GetFloor(item->pos.xPos, y, item->pos.zPos, &roomNumber);
	height = g_Level.Boxes[floor->box].height;
	nextHeight = 0;

	if (!Objects[item->objectNumber].nonLot)
	{
		nextBox = LOT->node[floor->box].exitBox;
	}
	else
	{
		floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		height = g_Level.Boxes[floor->box].height;
		nextBox = floor->box;
	}

	if (nextBox == NO_BOX)
		nextHeight = height;
	else
		nextHeight = g_Level.Boxes[nextBox].height;

	if (floor->box == NO_BOX || !LOT->isJumping && (LOT->fly == NO_FLYING && zone[item->boxNumber] != zone[floor->box] ||  boxHeight - height > LOT->step ||  boxHeight - height < LOT->drop))
	{
		xPos = item->pos.xPos / SECTOR(1);
		zPos = item->pos.zPos / SECTOR(1);
		shiftX = old.x / SECTOR(1);
		shiftZ = old.z / SECTOR(1);

		if (xPos < shiftX)
			item->pos.xPos = old.x & (~(WALL_SIZE - 1));
		else if (xPos > shiftX)
			item->pos.xPos = old.x | (WALL_SIZE - 1);

		if (zPos < shiftZ)
			item->pos.zPos = old.z & (~(WALL_SIZE - 1));
		else if (zPos > shiftZ)
			item->pos.zPos = old.z | (WALL_SIZE - 1);

		floor = GetFloor(item->pos.xPos, y, item->pos.zPos, &roomNumber);
		height = g_Level.Boxes[floor->box].height;
		if (!Objects[item->objectNumber].nonLot)
		{
			nextHeight = LOT->node[floor->box].exitBox;
		}
		else
		{
			floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
			height = g_Level.Boxes[floor->box].height;
			nextBox = floor->box;
		}

		if (nextBox == NO_BOX)
			nextHeight = height;
		else
			nextHeight = g_Level.Boxes[nextBox].height;
	}

	x = item->pos.xPos;
	z = item->pos.zPos;
	xPos = x & (WALL_SIZE - 1);
	zPos = z & (WALL_SIZE - 1);
	radius = Objects[item->objectNumber].radius;
	shiftX = 0;
	shiftZ = 0;

	if (zPos < radius)
	{
		if (BadFloor(x, y, z - radius, height, nextHeight, roomNumber, LOT))
		{
			shiftZ = radius - zPos;
		}

		if (xPos < radius)
		{
			if (BadFloor(x-radius, y, z, height, nextHeight, roomNumber, LOT))
			{
				shiftX = radius - xPos;
			}
			else if (!shiftZ && BadFloor(x - radius, y, z - radius, height, nextHeight, roomNumber, LOT))
			{
				if (item->pos.yRot > -ANGLE(135) && item->pos.yRot < ANGLE(45))
					shiftZ = radius - zPos;
				else
					shiftX = radius - xPos;
			}
		}
		else if (xPos > WALL_SIZE - radius)
		{
			if (BadFloor(x + radius, y, z, height, nextHeight, roomNumber, LOT))
			{
				shiftX = WALL_SIZE - radius - xPos;
			}
			else if (!shiftZ && BadFloor(x + radius, y, z - radius, height, nextHeight, roomNumber, LOT))
			{
				if (item->pos.yRot > -ANGLE(45) && item->pos.yRot < ANGLE(135))
					shiftZ = radius - zPos;
				else
					shiftX = WALL_SIZE-radius - xPos;
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
			{
				shiftX = radius - xPos;
			}
			else if (!shiftZ && BadFloor(x - radius, y, z + radius, height, nextHeight, roomNumber, LOT))
			{
				if (item->pos.yRot > -ANGLE(45) && item->pos.yRot < ANGLE(135))
					shiftX = radius - xPos;
				else
					shiftZ = WALL_SIZE - radius - zPos;
			}
		}
		else if (xPos > WALL_SIZE - radius)
		{
			if (BadFloor(x + radius, y, z, height, nextHeight, roomNumber, LOT))
			{
				shiftX = WALL_SIZE - radius - xPos;
			}
			else if (!shiftZ && BadFloor(x + radius, y, z + radius, height, nextHeight, roomNumber, LOT))
			{
				if (item->pos.yRot > -ANGLE(135) && item->pos.yRot < ANGLE(45))
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

	if (item->objectNumber != ID_TYRANNOSAUR && item->speed && item->hitPoints > 0)
		biffAngle = CreatureCreature(itemNumber);
	else
		biffAngle = 0;

	if (biffAngle)
	{
		if (abs(biffAngle) < BIFF_AVOID_TURN)
			item->pos.yRot -= BIFF_AVOID_TURN;
		else if (biffAngle > 0)
			item->pos.yRot -= BIFF_AVOID_TURN;
		else
			item->pos.yRot += BIFF_AVOID_TURN;
		return true;
	}

	if (LOT->fly != NO_FLYING && item->hitPoints > 0)
	{
		dy = creature->target.y - item->pos.yPos;
		if (dy > LOT->fly)
			dy = LOT->fly;
		else if (dy < -LOT->fly)
			dy = -LOT->fly;

		height = GetFloorHeight(floor, item->pos.xPos, y, item->pos.zPos);
		if (item->pos.yPos + dy <= height)
		{
			if (Objects[item->objectNumber].waterCreature)
			{
				ceiling = GetCeiling(floor, item->pos.xPos, y, item->pos.zPos);

				if (item->objectNumber == ID_WHALE)
					top = STEP_SIZE / 2;
				else
					top = bounds->Y1;

				if (item->pos.yPos + top + dy < ceiling)
				{
					if (item->pos.yPos + top < ceiling)
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
				floor = GetFloor(item->pos.xPos, y + STEP_SIZE, item->pos.zPos, &roomNumber);
				if (g_Level.Rooms[roomNumber].flags & (ENV_FLAG_WATER | ENV_FLAG_SWAMP))
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
 
		angle = (item->speed) ? phd_atan(item->speed, -dy) : 0;
		if (angle < -ANGLE(20))
			angle = -ANGLE(20);
		else if (angle > ANGLE(20))
			angle = ANGLE(20);

		if (angle < item->pos.xRot - ANGLE(1))
			item->pos.xRot -= ANGLE(1);
		else if (angle > item->pos.xRot + ANGLE(1))
			item->pos.xRot += ANGLE(1);
		else
			item->pos.xRot = angle;
	}
	else if (LOT->isJumping)
	{
		floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		int height2 = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		item->floor = height2;

		if (LOT->isMonkeying)
		{
			ceiling = GetCeiling(floor, item->pos.xPos, y, item->pos.zPos);
			item->pos.yPos = ceiling - bounds->Y1;
		}
		else
		{
			if (item->pos.yPos > item->floor)
			{
				if (item->pos.yPos > item->floor + STEP_SIZE)
				{
					item->pos.xPos = old.x;
					item->pos.yPos = old.y;
					item->pos.zPos = old.z;
				}
				else
				{
					item->pos.yPos = item->floor;
				}
			}
		}
	} 
	else
	{
		floor = GetFloor(item->pos.xPos, y, item->pos.zPos, &roomNumber);
		ceiling = GetCeiling(floor, item->pos.xPos, y, item->pos.zPos);

		if (item->objectNumber == ID_TYRANNOSAUR || item->objectNumber == ID_SHIVA || item->objectNumber == ID_MUTANT2)
			top = STEP_SIZE*3;
		else
			top = bounds->Y1; // TODO: check if Y1 or Y2

		if (item->pos.yPos + top < ceiling)
		{
			item->pos.xPos = old.x;
			item->pos.zPos = old.z;
			item->pos.yPos = old.y;
		}

		floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

		if (item->pos.yPos > item->floor)
			item->pos.yPos = item->floor;
		else if (item->floor - item->pos.yPos > STEP_SIZE/4)
			item->pos.yPos += STEP_SIZE/4;
		else if (item->pos.yPos < item->floor)
			item->pos.yPos = item->floor;

		item->pos.xRot = 0;
	}

	roomNumber = item->roomNumber;
	if (!Objects[item->objectNumber].waterCreature)
	{
		GetFloor(item->pos.xPos, item->pos.yPos - STEP_SIZE*2, item->pos.zPos, &roomNumber);

		if (g_Level.Rooms[roomNumber].flags & ENV_FLAG_WATER)
			item->hitPoints = 0;
	}

	if (item->roomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	return true;
}

void CreatureDie(short itemNumber, int explode)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	item->hitPoints = -16384;
	item->collidable = false;

	if (explode)
	{
		if (Objects[item->objectNumber].hitEffect)
			ExplodingDeath(itemNumber, ALL_MESHBITS, EXPLODE_HIT_EFFECT);
		else
			ExplodingDeath(itemNumber, ALL_MESHBITS, EXPLODE_NORMAL);

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

int BadFloor(int x, int y, int z, int boxHeight, int nextHeight, short roomNumber, LOT_INFO* LOT)
{
	FLOOR_INFO* floor;
	int height;

	floor = GetFloor(x, y, z, &roomNumber);
	if (floor->box == NO_BOX)
		return true;

	if (LOT->isJumping)
		return false;

	if (g_Level.Boxes[floor->box].flags & LOT->blockMask)
		return true;

	height = g_Level.Boxes[floor->box].height;
	if (boxHeight - height > LOT->step || boxHeight - height < LOT->drop)
		return true;

	if (boxHeight - height < -LOT->step && height > nextHeight)
		return true;

	if ((LOT->fly != NO_FLYING) && y > height + LOT->fly)
		return true;

	return false;
}

int CreatureCreature(short itemNumber)  
{
	ITEM_INFO* item, *linked;
	OBJECT_INFO* obj;
	ROOM_INFO* r;
	int x, z, xDistance, zDistance, distance = 0;
	short link, radius;

	item = &g_Level.Items[itemNumber];
	obj = &Objects[item->objectNumber];
	x = item->pos.xPos;
	z = item->pos.zPos;
	radius = obj->radius;

	r = &g_Level.Rooms[item->roomNumber];
	link = r->itemNumber;
	do
	{
		linked = &g_Level.Items[link];
		
		if (link != itemNumber && linked != LaraItem && linked->status == ITEM_ACTIVE && linked->hitPoints > 0)
		{
			xDistance = abs(linked->pos.xPos - x);
			zDistance = abs(linked->pos.zPos - z);
			
			if (xDistance > zDistance)
				distance = xDistance + (zDistance / 2);
			else
				distance = zDistance + (xDistance / 2);

			if (distance < radius + Objects[linked->objectNumber].radius)
				return phd_atan(linked->pos.zPos - z, linked->pos.xPos - x) - item->pos.yRot;
		}

		link = linked->nextItem;
	} while (link != NO_ITEM);

	return 0;
}

int ValidBox(ITEM_INFO* item, short zoneNumber, short boxNumber) 
{
	CREATURE_INFO* creature;
	BOX_INFO* box;
	int* zone;

	creature = (CREATURE_INFO*)item->data;
	zone = g_Level.Zones[creature->LOT.zone][FlipStatus].data();
	if (creature->LOT.fly == NO_FLYING && zone[boxNumber] != zoneNumber)
		return false;

	box = &g_Level.Boxes[boxNumber];
	if (box->flags & creature->LOT.blockMask)
		return false;

	if ((item->pos.zPos > (box->left * SECTOR(1))) && item->pos.zPos < ((box->right  * SECTOR(1))) &&
		(item->pos.xPos > (box->top  * SECTOR(1))) && item->pos.xPos < ((box->bottom * SECTOR(1))))
		return false;

	return true;
}

int EscapeBox(ITEM_INFO* item, ITEM_INFO* enemy, int boxNumber) 
{
	BOX_INFO* box = &g_Level.Boxes[boxNumber];
	int x, z;

	x = (box->top + box->bottom) * SECTOR(1) / 2 - enemy->pos.xPos;
	z = (box->left + box->right) * SECTOR(1) / 2 - enemy->pos.zPos;
	
	if (x > -ESCAPE_DIST && x < ESCAPE_DIST && z > -ESCAPE_DIST && z < ESCAPE_DIST)
		return false;

	if (((x > 0) ^ (item->pos.xPos > enemy->pos.xPos)) && ((z > 0) ^ (item->pos.zPos > enemy->pos.zPos)))
		return false;

	return true;
}

void TargetBox(LOT_INFO* LOT, int boxNumber)
{
	BOX_INFO* box;

	boxNumber &= NO_BOX;
	box = &g_Level.Boxes[boxNumber];

	LOT->target.x = (((box->top * SECTOR(1)) + GetRandomControl() * ((box->bottom - box->top) - 1)) / 32) + WALL_SIZE / 2;
	LOT->target.z = (((box->left * SECTOR(1)) + GetRandomControl() * ((box->right - box->left) - 1)) / 32) + WALL_SIZE / 2;
	LOT->requiredBox = boxNumber;

	if (LOT->fly == NO_FLYING)
		LOT->target.y = box->height;
	else
		LOT->target.y = box->height - STEPUP_HEIGHT;
}

int UpdateLOT(LOT_INFO* LOT, int depth)
{
	BOX_NODE* node;

	//printf("LOT->head: %d, LOT->tail: %d\n", LOT->head, LOT->tail);

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

		node->searchNumber = ++LOT->searchNumber;
		node->exitBox = NO_BOX;
	}

	return SearchLOT(LOT, depth);
}

int SearchLOT(LOT_INFO* LOT, int depth)
{
	BOX_NODE* node, *expand;
	BOX_INFO* box;
	int* zone, index, searchZone, boxNumber, delta, flags;
	bool done;
	
	zone = g_Level.Zones[LOT->zone][FlipStatus].data();
	searchZone = zone[LOT->head];

	if (depth <= 0)
		return true;

	for (int i = 0; i < depth; i++)
	{
		if (LOT->head == NO_BOX)
		{
			LOT->tail = NO_BOX; 
			return false;
		}

		node = &LOT->node[LOT->head];
		box = &g_Level.Boxes[LOT->head];

		index = box->overlapIndex;
		done = false;
		if (index >= 0)
		{
			do
			{
				boxNumber = g_Level.Overlaps[index].box;
				flags = g_Level.Overlaps[index++].flags;

				if (flags & BOX_END_BIT)
				{
					done = true;
				}

				if (LOT->fly == NO_FLYING && searchZone != zone[boxNumber])
					continue;

				delta = g_Level.Boxes[boxNumber].height - box->height;
				if ((delta > LOT->step || delta < LOT->drop) && !((flags & BOX_MONKEY) && LOT->canMonkey))
					continue;

				expand = &LOT->node[boxNumber];
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
					if ((node->searchNumber & SEARCH_NUMBER) == (expand->searchNumber & SEARCH_NUMBER) && !(expand->searchNumber & BLOCKED_SEARCH))
						continue;

					if (g_Level.Boxes[boxNumber].flags & LOT->blockMask)
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
		}

		LOT->head = node->nextExpansion;
		node->nextExpansion = NO_BOX;
	}

	return true;
}

int CreatureActive(short itemNumber) 
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!(item->flags & IFLAG_KILLED) 
		&& item->status & ITEM_INVISIBLE)
	{
		if (!EnableBaddieAI(itemNumber, false))
			return false;
		item->status = ITEM_ACTIVE;
	}

	return true;
}

void InitialiseCreature(short itemNumber) 
{
	ClearItem(itemNumber);
}

int StalkBox(ITEM_INFO* item, ITEM_INFO* enemy, int boxNumber)
{
	if (enemy == NULL)
		return false;

	BOX_INFO* box;
	int x, z, xrange, zrange;
	int enemyQuad, boxQuad, baddieQuad;

	box = &g_Level.Boxes[boxNumber];

	xrange	= STALK_DIST + ((box->bottom - box->top + 3) * SECTOR(1));
	zrange	= STALK_DIST + ((box->right - box->left + 3) * SECTOR(1));
	x		= (box->top + box->bottom) * SECTOR(1) / 2 - enemy->pos.xPos;
	z		= (box->left + box->right) * SECTOR(1) / 2 - enemy->pos.zPos;
	
	if (x > xrange || x < -xrange || z > zrange || z < -zrange)
		return false;

	enemyQuad = enemy->pos.yRot / 16384 + 2;
	
	// boxQuad = (z <= 0 ? (x <= 0 ? 0 : 3) : (x > 0) + 1);
	if (z > 0)
		boxQuad = (x > 0) ? 2 : 1;
	else
		boxQuad = (x > 0) ? 3 : 0;

	if (enemyQuad == boxQuad)
		return false;

	baddieQuad = 0;
	if (item->pos.zPos > enemy->pos.zPos)
		baddieQuad = (item->pos.xPos > enemy->pos.xPos) ? 2 : 1;
	else
		baddieQuad = (item->pos.xPos > enemy->pos.xPos) ? 3 : 0;

	if (enemyQuad == baddieQuad && abs(enemyQuad - boxQuad) == 2)
		return false;

	return true;
}

int CreatureVault(short itemNum, short angle, int vault, int shift)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	int xBlock, zBlock, y, newXblock, newZblock;
	short roomNumber;

	xBlock = item->pos.xPos / SECTOR(1);
	zBlock = item->pos.zPos / SECTOR(1);
	y = item->pos.yPos;
	roomNumber = item->roomNumber;

	CreatureAnimation(itemNum, angle, 0);

	if (item->floor > y + CHECK_CLICK(13) && creature->LOT.drop == -1792)
		vault = -7;
	else if (item->floor > y + CHECK_CLICK(7))
		vault = -4;
	else if (item->floor > y + CHECK_CLICK(5))
		vault = -3;
	else if (item->floor > y + CHECK_CLICK(3) && item->objectNumber != ID_BADDY1 && item->objectNumber != ID_BADDY2) // Baddy 1&2 don't have some climb down animations
		vault = -2;
	else if (item->pos.yPos > y - CHECK_CLICK(3))
		return 0;
	else if (item->pos.yPos > y - CHECK_CLICK(5))
		vault = 2;
	else if (item->pos.yPos > y - CHECK_CLICK(7))
		vault = 3;
	else if (item->pos.yPos > y - CHECK_CLICK(15) && creature->LOT.step == 1792)
		vault = 7;
	else
		vault = 4;

	newXblock = item->pos.xPos / SECTOR(1);
	newZblock = item->pos.zPos / SECTOR(1);
	
	if (zBlock == newZblock)
	{
		if (xBlock == newXblock)
			return 0;

		if (xBlock < newXblock)
		{
			item->pos.xPos = (newXblock * SECTOR(1)) - shift;
			item->pos.yRot = ANGLE(90);
		}
		else
		{
			item->pos.xPos = (xBlock * SECTOR(1)) + shift;
			item->pos.yRot = -ANGLE(90);
		}
	}
	else if (xBlock == newXblock)
	{
		if (zBlock < newZblock)
		{
			item->pos.zPos = (newZblock * SECTOR(1)) - shift;
			item->pos.yRot = 0;
		}
		else
		{
			item->pos.zPos = (zBlock * SECTOR(1)) + shift;
			item->pos.yRot = -ANGLE(180);
		}
	}

	item->pos.yPos = item->floor = y;
	if (roomNumber != item->roomNumber)
		ItemNewRoom(itemNum, roomNumber);

	return vault;
}

void GetAITarget(CREATURE_INFO* creature)
{
#define GET_REACHED_GOAL	abs(enemy->pos.xPos - item->pos.xPos) < REACHED_GOAL_RADIUS &&\
							abs(enemy->pos.zPos - item->pos.zPos) < REACHED_GOAL_RADIUS &&\
							abs(enemy->pos.yPos - item->pos.yPos) < REACHED_GOAL_RADIUS

	ITEM_INFO* enemy;
	ITEM_INFO* item;
	ITEM_INFO* targetItem;
	FLOOR_INFO* floor;
	int i;
	short enemyObjectNumber;

	enemy = creature->enemy;

	if (enemy)
		enemyObjectNumber = enemy->objectNumber;
	else
		enemyObjectNumber = NO_ITEM;

	item = &g_Level.Items[creature->itemNum];
	
	if (item->aiBits & GUARD)
	{
		creature->enemy = LaraItem;
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
				item->itemFlags[3] = (item->TOSSPAD & 0xFF);
			}
		}
		else if (!creature->patrol2 && enemyObjectNumber != ID_AI_PATROL1)
		{
			FindAITargetObject(creature, ID_AI_PATROL1);
		}
		else if (creature->patrol2 && enemyObjectNumber != ID_AI_PATROL2)
		{
			FindAITargetObject(creature, ID_AI_PATROL2);
		}
		else if (GET_REACHED_GOAL || Objects[item->objectNumber].waterCreature)
		{
			floor = GetFloor(enemy->pos.xPos, enemy->pos.yPos, enemy->pos.zPos, &(enemy->roomNumber));
			GetFloorHeight(floor, enemy->pos.xPos, enemy->pos.yPos, enemy->pos.zPos);
			TestTriggers(TriggerIndex, TRUE, 0);
			creature->patrol2 = ~creature->patrol2;
		}
	}
	else if (item->aiBits & AMBUSH)
	{
		if (!(item->aiBits & MODIFY) && !creature->hurtByLara)
		{
			creature->enemy = LaraItem;
		}
		else if (enemyObjectNumber != ID_AI_AMBUSH)
		{
			FindAITargetObject(creature, ID_AI_AMBUSH);
		}
		else if (item->objectNumber == ID_MONKEY)
		{
			return;
		}
		else if (GET_REACHED_GOAL)
		{
			floor = GetFloor(enemy->pos.xPos, enemy->pos.yPos, enemy->pos.zPos, &(enemy->roomNumber));
			GetFloorHeight(floor, enemy->pos.xPos, enemy->pos.yPos, enemy->pos.zPos);
			TestTriggers(TriggerIndex, TRUE, 0);

			creature->reachedGoal = true;
			creature->enemy = LaraItem;
			item->aiBits &= ~(AMBUSH|MODIFY);
			item->aiBits |= GUARD;
			creature->alerted = false;
		}
	}
	else if (item->aiBits & FOLLOW)
	{
		if (creature->hurtByLara)
		{
			creature->enemy = LaraItem;
			creature->alerted = true;
			item->aiBits &= ~FOLLOW;
		}
		else if (item->hitStatus)
		{
			item->aiBits &= ~FOLLOW;
		}
		else if (enemyObjectNumber != ID_AI_FOLLOW)
		{
			FindAITargetObject(creature, ID_AI_FOLLOW);
		}
		else if (GET_REACHED_GOAL)
		{
			creature->reachedGoal = true;
			item->aiBits &= ~FOLLOW;
		}
	}
	else if (item->objectNumber == ID_MONKEY && item->carriedItem == NO_ITEM)
	{
		if (item->aiBits != MODIFY)
		{
			if (enemyObjectNumber != ID_SMALLMEDI_ITEM)
			{
				FindAITargetObject(creature, ID_SMALLMEDI_ITEM);
			}
		}
		else
		{
			if (enemyObjectNumber != ID_KEY_ITEM4)
			{
				FindAITargetObject(creature, ID_KEY_ITEM4);
			}
		}
	}

#undef GET_REACHED_GOAL
}

// tr3 old way..
void FindAITarget(CREATURE_INFO* creature, short objectNumber)
{
	ITEM_INFO* item = &g_Level.Items[creature->itemNum];
	ITEM_INFO* targetItem;
	int i;

	for (i = 0, targetItem = &g_Level.Items[0]; i < g_Level.NumItems; i++, targetItem++)
	{
		if (targetItem->objectNumber == objectNumber && targetItem->roomNumber != NO_ROOM)
		{
			if (SameZone(creature, targetItem) && targetItem->pos.yRot == item->itemFlags[3])
			{
				creature->enemy = targetItem;
				break;
			}
		}
	}
}

void FindAITargetObject(CREATURE_INFO* creature, short objectNumber)
{
	ITEM_INFO* item = &g_Level.Items[creature->itemNum];

	if (g_Level.AIObjects.size() > 0)
	{
		AIOBJECT* foundObject = NULL;

		for (int i = 0; i < g_Level.AIObjects.size(); i++)
		{
			AIOBJECT* aiObject = &g_Level.AIObjects[i];

			if (aiObject->objectNumber == objectNumber && aiObject->triggerFlags == item->itemFlags[3] && aiObject->roomNumber != NO_ROOM)
			{
				int* zone = g_Level.Zones[creature->LOT.zone][FlipStatus].data();

				ROOM_INFO* r = &g_Level.Rooms[item->roomNumber];
				item->boxNumber = XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z).box;

				r = &g_Level.Rooms[aiObject->roomNumber];
				aiObject->boxNumber = XZ_GET_SECTOR(r, aiObject->x - r->x, aiObject->z - r->z).box;

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
				creature->aiTarget.pos.xPos += phd_sin(creature->aiTarget.pos.yRot) * 1024;
				creature->aiTarget.pos.zPos += phd_cos(creature->aiTarget.pos.yRot) * 1024;
			}
		}
	}
}

void CreatureAIInfo(ITEM_INFO* item, AI_INFO* info)
{
	if (item->data == NULL)
		return;

	CREATURE_INFO * creature;
	ITEM_INFO * enemy;
	OBJECT_INFO * obj;
	ROOM_INFO * r;
	short angle;
	int* zone;
	int x, y, z;

	creature = (CREATURE_INFO*)item->data;
	obj = &Objects[item->objectNumber];

	enemy = creature->enemy;
	if (!enemy)
	{
		enemy = LaraItem;
		creature->enemy = LaraItem;
	}

	zone = g_Level.Zones[creature->LOT.zone][FlipStatus].data();

	r = &g_Level.Rooms[item->roomNumber];
	item->boxNumber = XZ_GET_SECTOR(r, item->pos.xPos - r->x, item->pos.zPos - r->z).box;
	info->zoneNumber = zone[item->boxNumber];

	r = &g_Level.Rooms[enemy->roomNumber];
	enemy->boxNumber = XZ_GET_SECTOR(r, enemy->pos.xPos - r->x, enemy->pos.zPos - r->z).box;
	info->enemyZone = zone[enemy->boxNumber];

	if (!obj->nonLot)
	{
		if (g_Level.Boxes[enemy->boxNumber].flags & creature->LOT.blockMask)
			info->enemyZone |= BLOCKED;
		else if (creature->LOT.node[item->boxNumber].searchNumber == (creature->LOT.searchNumber | BLOCKED_SEARCH))
			info->enemyZone |= BLOCKED;
	}

	if (enemy == LaraItem)
	{
		x = enemy->pos.xPos + enemy->speed * PREDICTIVE_SCALE_FACTOR * phd_sin(LaraItem->pos.yRot + Lara.moveAngle) - item->pos.xPos + obj->pivotLength * phd_sin(item->pos.yRot);
		z = enemy->pos.zPos + enemy->speed * PREDICTIVE_SCALE_FACTOR * phd_cos(LaraItem->pos.yRot + Lara.moveAngle) - item->pos.zPos + obj->pivotLength * phd_cos(item->pos.yRot);
	}
	else
	{
		x = enemy->pos.xPos + enemy->speed * PREDICTIVE_SCALE_FACTOR * phd_sin(enemy->pos.yRot) - item->pos.xPos + obj->pivotLength * phd_sin(item->pos.yRot);
		z = enemy->pos.zPos + enemy->speed * PREDICTIVE_SCALE_FACTOR * phd_cos(enemy->pos.yRot) - item->pos.zPos + obj->pivotLength * phd_cos(item->pos.yRot);
	}

	y = item->pos.yPos - enemy->pos.yPos;
	angle = phd_atan(z, x);

	if (x > 32000 || x < -32000 || z > 32000 || z < -32000)
	{
		info->distance = 0x7FFFFFFF;
	}
	else
	{
		if (creature->enemy)
			info->distance = SQUARE(x) + SQUARE(z);
		else
			info->distance = 0x7FFFFFFF;
	}

	info->angle = angle - item->pos.yRot;
	info->enemyFacing = -ANGLE(180) + angle - enemy->pos.yRot;

	x = abs(x);
	z = abs(z);

	if (enemy == LaraItem)
	{
		short laraState = LaraItem->currentAnimState;
		if (laraState == LS_CROUCH_IDLE ||
			laraState == LS_CROUCH_TURN_LEFT ||
			laraState == LS_CROUCH_TURN_RIGHT ||
			laraState == LS_CROUCH_ROLL ||
			laraState <= LS_MONKEYSWING_TURN_180 ||
			laraState >= LS_HANG_TO_CRAWL)
		{
			y -= STEPUP_HEIGHT;
		}
	}

	if (x > z)
		info->xAngle = phd_atan(x + (z / 2), y);
	else
		info->xAngle = phd_atan(z + (x / 2), y);

	info->ahead = (info->angle > -FRONT_ARC && info->angle < FRONT_ARC);
	info->bite = (info->ahead && enemy->hitPoints > 0 && abs(enemy->pos.yPos - item->pos.yPos) <= (STEP_SIZE * 2));
}

void CreatureMood(ITEM_INFO* item, AI_INFO* info, int violent)
{
	if (item->data == NULL)
		return;

	CREATURE_INFO* creature;
	ITEM_INFO* enemy;
	LOT_INFO* LOT;
	int boxNumber, startBox, overlapIndex, nextBox, flags;
	BOUNDING_BOX* bounds;

	creature = (CREATURE_INFO*)item->data;
	enemy = creature->enemy;
	LOT = &creature->LOT;

	switch (creature->mood)
	{
		case BORED_MOOD:
			boxNumber = LOT->node[GetRandomControl() * LOT->zoneCount / 32768].boxNumber;
			if (ValidBox(item, info->zoneNumber, boxNumber)
				&& !(GetRandomControl() & 0x0F))
			{
				if (StalkBox(item, enemy, boxNumber) && enemy->hitPoints > 0 && creature->enemy)
				{
					TargetBox(LOT, boxNumber);
					creature->mood = BORED_MOOD;
				}
				else if (LOT->requiredBox == NO_BOX)
				{
					TargetBox(LOT, boxNumber);
				}
			}
			break;

		case ATTACK_MOOD:
			LOT->target.x = enemy->pos.xPos;
			LOT->target.y = enemy->pos.yPos;
			LOT->target.z = enemy->pos.zPos;
			LOT->requiredBox = enemy->boxNumber;

			if (LOT->fly != NO_FLYING && Lara.waterStatus == LW_ABOVE_WATER)
			{
				bounds = (BOUNDING_BOX*)GetBestFrame(enemy);
				LOT->target.y += bounds->Y1;
			}
			break;

		case ESCAPE_MOOD:
			boxNumber = LOT->node[GetRandomControl() * LOT->zoneCount / 32768].boxNumber;
			if (ValidBox(item, info->zoneNumber, boxNumber) && LOT->requiredBox == NO_BOX)
			{
				if (EscapeBox(item, enemy, boxNumber))
				{
					TargetBox(LOT, boxNumber);
				}
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
				boxNumber = LOT->node[GetRandomControl() * LOT->zoneCount / 32768].boxNumber;
				if (ValidBox(item, info->zoneNumber, boxNumber))
				{
					if (StalkBox(item, enemy, boxNumber))
					{
						TargetBox(LOT, boxNumber);
					}
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

	CalculateTarget(&creature->target, item, &creature->LOT);

	creature->jumpAhead = false;
	creature->monkeyAhead = false;

	startBox = LOT->node[item->boxNumber].exitBox;
	if (startBox != NO_BOX)
	{
		overlapIndex = g_Level.Boxes[item->boxNumber].overlapIndex;
		nextBox = 0;
		flags = 0;
		if (overlapIndex >= 0)
		{
			do
			{
				//overlapIndex++;
				//if (g_Level.Overlaps[overlapIndex++].flags & (BOX_JUMP|BOX_MONKEY))
				//{
				nextBox = g_Level.Overlaps[overlapIndex].box;
				flags = g_Level.Overlaps[overlapIndex++].flags;
			} while (nextBox != NO_BOX && ((flags & BOX_END_BIT) == FALSE) && (nextBox != startBox));
		}

		if (nextBox == startBox)
		{
			if (flags & BOX_JUMP)
				creature->jumpAhead = true;
			if (flags & BOX_MONKEY)
				creature->monkeyAhead = true;
		}
	}
}

void GetCreatureMood(ITEM_INFO* item, AI_INFO* info, int isViolent)
{
	if (item->data == NULL)
		return;

	CREATURE_INFO* creature;
	LOT_INFO* LOT;
	ITEM_INFO* enemy;
	MOOD_TYPE mood;

	creature = (CREATURE_INFO*)item->data;
	enemy = creature->enemy;
	LOT = &creature->LOT;

	if (creature->LOT.node[item->boxNumber].searchNumber == (creature->LOT.searchNumber | BLOCKED_SEARCH))
		creature->LOT.requiredBox = NO_BOX;

	if (creature->mood != ATTACK_MOOD 
		&& creature->LOT.requiredBox != NO_BOX)
	{
		if (!ValidBox(item, info->zoneNumber, creature->LOT.targetBox))
		{
			if (info->zoneNumber == info->enemyZone)
				creature->mood = BORED_MOOD;
			creature->LOT.requiredBox = NO_BOX;
		}
	}

	mood = creature->mood;
	if (!enemy)
	{
		creature->mood = BORED_MOOD;
		enemy = LaraItem;
	}
	else if (enemy->hitPoints <= 0 && enemy == LaraItem)
	{
		creature->mood = BORED_MOOD;
	}
	else if (isViolent)
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
				if (creature->alerted 
					&& info->zoneNumber != info->enemyZone)
				{
					if (info->distance > 3072)
						creature->mood = STALK_MOOD;
					else
						creature->mood = BORED_MOOD;
				}
				else if (info->zoneNumber == info->enemyZone)
				{
					if (info->distance < ATTACK_RANGE 
						|| (creature->mood == STALK_MOOD 
							&& LOT->requiredBox == NO_BOX))
						creature->mood = ATTACK_MOOD;
					else
						creature->mood = STALK_MOOD;
				}
				break;

			case ATTACK_MOOD:
				if (item->hitStatus 
					&& (GetRandomControl() < ESCAPE_CHANCE 
						|| info->zoneNumber != info->enemyZone))
					creature->mood = STALK_MOOD;
				else if (info->zoneNumber != info->enemyZone && info->distance > (WALL_SIZE*6))
					creature->mood = BORED_MOOD;
				break;

			case ESCAPE_MOOD:
				if (info->zoneNumber == info->enemyZone 
					&& GetRandomControl() < RECOVER_CHANCE)
					creature->mood = STALK_MOOD;
				break;
		}
	}

	if (mood != creature->mood)
	{
		if (mood == ATTACK_MOOD)
			TargetBox(LOT, LOT->targetBox);
		LOT->requiredBox = NO_BOX;
	}
}

TARGET_TYPE CalculateTarget(PHD_VECTOR* target, ITEM_INFO* item, LOT_INFO* LOT)
{
	BOX_INFO* box;
	int boxLeft, boxRight, boxTop, boxBottom;
	int left, top, right, bottom;
	int direction;
	int boxNumber;

	UpdateLOT(LOT, 5);

	target->x = item->pos.xPos;
	target->y = item->pos.yPos;
	target->z = item->pos.zPos;

	boxNumber = item->boxNumber;
	if (boxNumber == NO_BOX)
		return TARGET_TYPE::NO_TARGET;

	box = &g_Level.Boxes[boxNumber];
	boxLeft = ((int)box->left * SECTOR(1));
	boxRight = ((int)box->right * SECTOR(1)) - 1;
	boxTop = ((int)box->top * SECTOR(1));
	boxBottom = ((int)box->bottom * SECTOR(1)) - 1;
	left = boxLeft;
	right = boxRight;
	top = boxTop;
	bottom = boxBottom;
	direction = ALL_CLIP;

	do
	{
		box = &g_Level.Boxes[boxNumber];

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

		boxLeft = ((int)box->left * SECTOR(1));
		boxRight = ((int)box->right * SECTOR(1)) - 1;
		boxTop = ((int)box->top * SECTOR(1));
		boxBottom = ((int)box->bottom * SECTOR(1)) - 1;

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
						return TARGET_TYPE::SECONDARY_TARGET;

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
						return TARGET_TYPE::SECONDARY_TARGET;

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
						return TARGET_TYPE::SECONDARY_TARGET;

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
						return TARGET_TYPE::SECONDARY_TARGET;

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
						return TARGET_TYPE::SECONDARY_TARGET;

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
						return TARGET_TYPE::SECONDARY_TARGET;

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
						return TARGET_TYPE::SECONDARY_TARGET;

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
						return TARGET_TYPE::SECONDARY_TARGET;

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

			return TARGET_TYPE::PRIME_TARGET;
		}

		boxNumber = LOT->node[boxNumber].exitBox;
		if (boxNumber != NO_BOX && (g_Level.Boxes[boxNumber].flags & LOT->blockMask))
			break;
	} while (boxNumber != NO_BOX);

	if (direction & (CLIP_LEFT | CLIP_RIGHT))
	{
		target->z = boxLeft + WALL_SIZE / 2 + ((GetRandomControl() * (boxRight - boxLeft - WALL_SIZE)) / 32768);
	}
	else if (!(direction & SECONDARY_CLIP))
	{
		if (target->z < boxLeft + 512)
			target->z = boxLeft + 512;
		else if (target->z > boxRight - 512)
			target->z = boxRight - 512;
	}

	if (direction & (CLIP_TOP | CLIP_BOTTOM))
	{
		target->x = boxTop + WALL_SIZE / 2 + ((GetRandomControl() * (boxBottom - boxTop - WALL_SIZE)) / 32768);
	}
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
		target->y = box->height - STEPUP_HEIGHT;

	return TARGET_TYPE::NO_TARGET;
}

void AdjustStopperFlag(ITEM_INFO* item, int dir, int set)
{
	int x = item->pos.xPos;
	int z = item->pos.zPos;

	ROOM_INFO* r = &g_Level.Rooms[item->roomNumber];

	FLOOR_INFO* floor = &XZ_GET_SECTOR(r, x - r->x, z - r->z);
	floor->stopper = set;

	x = item->pos.xPos + 1024 * phd_sin(dir);
	z = item->pos.zPos + 1024 * phd_cos(dir);

	short roomNumber = item->roomNumber;
	GetFloor(x, item->pos.yPos, z, &roomNumber);
	r = &g_Level.Rooms[roomNumber];

	floor = &XZ_GET_SECTOR(r, x - r->x, z - r->z);
	floor->stopper = set;
}