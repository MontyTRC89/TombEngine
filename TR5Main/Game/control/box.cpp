#include "framework.h"
#include "Game/control/box.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/sphere.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Specific/setup.h"
#include "Specific/trmath.h"
#include "Objects/objectslist.h"
#include "Game/itemdata/creature_info.h"
#include "Objects/TR5/Object/tr5_pushableblock.h"

#define CHECK_CLICK(x) CLICK(x) / 2
#define ESCAPE_DIST SECTOR(5)
#define STALK_DIST SECTOR(3)
#define REACHED_GOAL_RADIUS 640
#define ATTACK_RANGE pow(SECTOR(3), 2)
#define ESCAPE_CHANCE  0x800
#define RECOVER_CHANCE 0x100
#define BIFF_AVOID_TURN 1536
#define FEELER_DISTANCE CLICK(2)
#define FEELER_ANGLE ANGLE(45.0f)

#ifdef CREATURE_AI_PRIORITY_OPTIMIZATION
constexpr int HIGH_PRIO_RANGE = 8;
constexpr int MEDIUM_PRIO_RANGE = HIGH_PRIO_RANGE + HIGH_PRIO_RANGE * (HIGH_PRIO_RANGE / 6.0f);
constexpr int LOW_PRIO_RANGE = MEDIUM_PRIO_RANGE + MEDIUM_PRIO_RANGE * (MEDIUM_PRIO_RANGE / 24.0f);
constexpr int NONE_PRIO_RANGE = LOW_PRIO_RANGE + LOW_PRIO_RANGE * (LOW_PRIO_RANGE / 32.0f);
constexpr auto FRAME_PRIO_BASE = 4;
constexpr auto FRAME_PRIO_EXP = 1.5;
#endif // CREATURE_AI_PRIORITY_OPTIMIZATION

void DropBaddyPickups(ITEM_INFO* item)
{
	ITEM_INFO* pickup = NULL;
	FLOOR_INFO* floor;
	short roomNumber;
	BOUNDING_BOX* bounds;

	for (short pickupNumber = item->CarriedItem; pickupNumber != NO_ITEM; pickupNumber = pickup->CarriedItem)
	{
		pickup = &g_Level.Items[pickupNumber];
		pickup->Position.xPos = (item->Position.xPos & -CLICK(1)) | CLICK(1);
		pickup->Position.zPos = (item->Position.zPos & -CLICK(1)) | CLICK(1);

		roomNumber = item->RoomNumber;
		floor = GetFloor(pickup->Position.xPos, item->Position.yPos, pickup->Position.zPos, &roomNumber);
		pickup->Position.yPos = GetFloorHeight(floor, pickup->Position.xPos, item->Position.yPos, pickup->Position.zPos);
		bounds = GetBoundsAccurate(pickup);
		pickup->Position.yPos -= bounds->Y2;

		ItemNewRoom(pickupNumber, item->RoomNumber);
		pickup->Flags |= 32;
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

bool SameZone(CREATURE_INFO* creature, ITEM_INFO* target)
{
	int* zone = g_Level.Zones[creature->LOT.zone][FlipStatus].data();
	ITEM_INFO* item = &g_Level.Items[creature->itemNum];
	ROOM_INFO* room = &g_Level.Rooms[item->RoomNumber];
	FLOOR_INFO* floor = GetSector(room, item->Position.xPos - room->x, item->Position.zPos - room->z);
	if (floor->Box == NO_BOX)
		return false;
	item->BoxNumber = floor->Box;

	room = &g_Level.Rooms[target->RoomNumber];
	floor = GetSector(room, target->Position.xPos - room->x, target->Position.zPos - room->z);
	if (floor->Box == NO_BOX)
		return false;
	target->BoxNumber = floor->Box;

	return (zone[item->BoxNumber] == zone[target->BoxNumber]);
}

short AIGuard(CREATURE_INFO* creature) 
{
	ITEM_INFO* item;
	int random;

	item = &g_Level.Items[creature->itemNum];
	if (item->AIBits & (GUARD | PATROL1))
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
		return (creature->headRight) << 12;

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

	for (int i = 0; i < ActiveCreatures.size(); i++)
	{
		creature = ActiveCreatures[i];
		if (creature->itemNum == NO_ITEM)
			continue;

		target = &g_Level.Items[creature->itemNum + i];
		if (item->RoomNumber == target->RoomNumber)
		{
			creature->alerted = true;
			continue;
		}

		x = (target->Position.xPos - item->Position.xPos) / 64;
		y = (target->Position.yPos - item->Position.yPos) / 64;
		z = (target->Position.zPos - item->Position.zPos) / 64;

		distance = (SQUARE(x) + SQUARE(y) + SQUARE(z));
		if (distance < SECTOR(8))
			creature->alerted = true;
	}
}

void AlertAllGuards(short itemNumber) 
{
	for (int i = 0; i < ActiveCreatures.size(); i++)
	{
		auto* creature = ActiveCreatures[i];
		if (creature->itemNum == NO_ITEM)
			continue;

		auto* target = &g_Level.Items[creature->itemNum];
		short objNumber = g_Level.Items[itemNumber].ObjectNumber;
		if (objNumber == target->ObjectNumber)
		{
			if (target->Status == ITEM_ACTIVE)
				creature->alerted = true;
		}
	}
}

void CreatureKill(ITEM_INFO* item, int killAnim, int killState, short laraKillState)
{
	item->AnimNumber = Objects[item->ObjectNumber].animIndex + killAnim;
	item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	item->ActiveState = killState;

	LaraItem->AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex;
	LaraItem->FrameNumber = g_Level.Anims[LaraItem->AnimNumber].frameBase;
	LaraItem->ActiveState = 0;
	LaraItem->TargetState = laraKillState;

	LaraItem->Position.xPos = item->Position.xPos;
	LaraItem->Position.yPos = item->Position.yPos;
	LaraItem->Position.zPos = item->Position.zPos;
	LaraItem->Position.yRot = item->Position.yRot;
	LaraItem->Position.xRot = item->Position.xRot;
	LaraItem->Position.zRot = item->Position.zRot;
	LaraItem->VerticalVelocity = 0;
	LaraItem->Airborne = false;
	LaraItem->VerticalVelocity = 0;

	if (item->RoomNumber != LaraItem->RoomNumber)
		ItemNewRoom(Lara.ItemNumber, item->RoomNumber);

	AnimateItem(LaraItem);

	Lara.ExtraAnim = 1;
	Lara.Control.HandStatus = HandStatus::Busy;
	Lara.Control.WeaponControl.GunType = WEAPON_NONE;
	Lara.hitDirection = -1;
	Lara.Air = -1;

	Camera.pos.roomNumber = LaraItem->RoomNumber; 
	Camera.type = CameraType::Chase;
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
	return func(pos.x, pos.y, pos.z, damage, angle, item->RoomNumber);
}

short CreatureEffect(ITEM_INFO* item, BITE_INFO* bite, std::function<CreatureEffectFunction> func)
{
	PHD_VECTOR pos;
	pos.x = bite->x;
	pos.y = bite->y;
	pos.z = bite->z;
	GetJointAbsPosition(item, &pos, bite->meshNum);
	return func(pos.x, pos.y, pos.z, item->VerticalVelocity, item->Position.yRot, item->RoomNumber);
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
		wh = GetWaterHeight(item);
	}

	int y = wh + waterLevel;

	if (item->Position.yPos < y)
	{
		roomNumber = item->RoomNumber;
		floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
		height = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);

		item->Position.yPos = y;
		if (y > height)
			item->Position.yPos = height;

		if (item->Position.xRot > ANGLE(2.0f))
			item->Position.xRot -= ANGLE(2.0f);
		else if (item->Position.xRot > 0)
			item->Position.xRot = 0;
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
	item->HitPoints = NOT_TARGETABLE;
	item->Position.xRot = 0;

	waterLevel = GetWaterHeight(item);

	y = item->Position.yPos;
	if (y > waterLevel)
		item->Position.yPos = y - 32;
	if (item->Position.yPos < waterLevel)
		item->Position.yPos = waterLevel;

	AnimateItem(item);

	roomNumber = item->RoomNumber;
	floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
	item->Floor = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
	
	if (roomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	if (item->Position.yPos <= waterLevel)
	{
		if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
		{
			item->Position.yPos = waterLevel;
			item->Collidable = false;
			item->Status = ITEM_DEACTIVATED;
			DisableEntityAI(itemNumber);
			RemoveActiveItem(itemNumber);
			item->AfterDeath = 1;
		}
	}
}

void CreatureJoint(ITEM_INFO* item, short joint, short required) 
{
	if (!item->Data)
		return;

	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;

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
	angle = (angle << 2) - item->Position.zRot;

	if (angle < -ANGLE(3))
		angle = -ANGLE(3);
	else if (angle > ANGLE(3))
		angle = ANGLE(3);

	short theAngle = -ANGLE(3);

	short absRot = abs(item->Position.zRot);
	if (absRot < ANGLE(15) || absRot > ANGLE(30))
		angle >>= 1;
	
	item->Position.zRot += angle;
}

short CreatureTurn(ITEM_INFO* item, short maximumTurn)
{
	if (!item->Data || maximumTurn == 0)
		return 0;

	CREATURE_INFO* creature;
	int x, z, range, distance;
	short angle;

	creature = (CREATURE_INFO*)item->Data;
	angle = 0;

	x = creature->target.x - item->Position.xPos;
	z = creature->target.z - item->Position.zPos;
	angle = phd_atan(z, x) - item->Position.yRot;
	range = item->VerticalVelocity * 16384 / maximumTurn;
	distance = SQUARE(x) + SQUARE(z);

	if (angle > FRONT_ARC || angle < -FRONT_ARC && distance < SQUARE(range))
		maximumTurn >>= 1;

	if (angle > maximumTurn)
		angle = maximumTurn;
	else if (angle < -maximumTurn)
		angle = -maximumTurn;

	item->Position.yRot += angle;

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
	if (!item->Data)
		return false;

	creature = (CREATURE_INFO*)item->Data;
	LOT = &creature->LOT;
	zone = g_Level.Zones[LOT->zone][FlipStatus].data();
	if (item->BoxNumber != NO_BOX)
		boxHeight = g_Level.Boxes[item->BoxNumber].height;
	else
		boxHeight = item->Floor;
	old.x = item->Position.xPos;
	old.y = item->Position.yPos;
	old.z = item->Position.zPos;
	
	/*if (!Objects[item->objectNumber].waterCreature)
	{
		roomNumber = item->roomNumber;
		GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		if (roomNumber != item->roomNumber)
			ItemNewRoom(itemNumber, roomNumber);
	}*/

	AnimateItem(item);
	if (item->Status == ITEM_DEACTIVATED)
	{
		CreatureDie(itemNumber, FALSE);
		return false;
	}

	bounds = GetBoundsAccurate(item);
	y = item->Position.yPos + bounds->Y1;

	roomNumber = item->RoomNumber;
	GetFloor(old.x, y, old.z, &roomNumber);  
	floor = GetFloor(item->Position.xPos, y, item->Position.zPos, &roomNumber);

	// TODO: Check why some blocks have box = -1 assigned to them -- Lwmte, 10.11.21
	if (floor->Box == NO_BOX)
		return false;

	height = g_Level.Boxes[floor->Box].height;
	nextHeight = 0;

	if (!Objects[item->ObjectNumber].nonLot)
	{
		nextBox = LOT->node[floor->Box].exitBox;
	}
	else
	{
		floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
		height = g_Level.Boxes[floor->Box].height;
		nextBox = floor->Box;
	}

	if (nextBox == NO_BOX)
		nextHeight = height;
	else
		nextHeight = g_Level.Boxes[nextBox].height;

	if (floor->Box == NO_BOX || !LOT->isJumping && (LOT->fly == NO_FLYING && item->BoxNumber != NO_BOX && zone[item->BoxNumber] != zone[floor->Box] ||  boxHeight - height > LOT->step ||  boxHeight - height < LOT->drop))
	{
		xPos = item->Position.xPos / SECTOR(1);
		zPos = item->Position.zPos / SECTOR(1);
		shiftX = old.x / SECTOR(1);
		shiftZ = old.z / SECTOR(1);

		if (xPos < shiftX)
			item->Position.xPos = old.x & (~(WALL_SIZE - 1));
		else if (xPos > shiftX)
			item->Position.xPos = old.x | (WALL_SIZE - 1);

		if (zPos < shiftZ)
			item->Position.zPos = old.z & (~(WALL_SIZE - 1));
		else if (zPos > shiftZ)
			item->Position.zPos = old.z | (WALL_SIZE - 1);

		floor = GetFloor(item->Position.xPos, y, item->Position.zPos, &roomNumber);
		height = g_Level.Boxes[floor->Box].height;
		if (!Objects[item->ObjectNumber].nonLot)
		{
			nextHeight = LOT->node[floor->Box].exitBox;
		}
		else
		{
			floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
			height = g_Level.Boxes[floor->Box].height;
			nextBox = floor->Box;
		}

		if (nextBox == NO_BOX)
			nextHeight = height;
		else
			nextHeight = g_Level.Boxes[nextBox].height;
	}

	x = item->Position.xPos;
	z = item->Position.zPos;
	xPos = x & (WALL_SIZE - 1);
	zPos = z & (WALL_SIZE - 1);
	radius = Objects[item->ObjectNumber].radius;
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
				if (item->Position.yRot > -ANGLE(135) && item->Position.yRot < ANGLE(45))
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
				if (item->Position.yRot > -ANGLE(45) && item->Position.yRot < ANGLE(135))
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
				if (item->Position.yRot > -ANGLE(45) && item->Position.yRot < ANGLE(135))
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
				if (item->Position.yRot > -ANGLE(135) && item->Position.yRot < ANGLE(45))
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

	item->Position.xPos += shiftX;
	item->Position.zPos += shiftZ;

	if (shiftX || shiftZ)
	{
		floor = GetFloor(item->Position.xPos, y, item->Position.zPos, &roomNumber);

		item->Position.yRot += angle;
		if (tilt)
			CreatureTilt(item, (tilt * 2));
	}

	if (item->ObjectNumber != ID_TYRANNOSAUR && item->VerticalVelocity && item->HitPoints > 0)
		biffAngle = CreatureCreature(itemNumber);
	else
		biffAngle = 0;

	if (biffAngle)
	{
		if (abs(biffAngle) < BIFF_AVOID_TURN)
			item->Position.yRot -= BIFF_AVOID_TURN;
		else if (biffAngle > 0)
			item->Position.yRot -= BIFF_AVOID_TURN;
		else
			item->Position.yRot += BIFF_AVOID_TURN;
		return true;
	}

	if (LOT->fly != NO_FLYING && item->HitPoints > 0)
	{
		dy = creature->target.y - item->Position.yPos;
		if (dy > LOT->fly)
			dy = LOT->fly;
		else if (dy < -LOT->fly)
			dy = -LOT->fly;

		height = GetFloorHeight(floor, item->Position.xPos, y, item->Position.zPos);
		if (item->Position.yPos + dy <= height)
		{
			if (Objects[item->ObjectNumber].waterCreature)
			{
				ceiling = GetCeiling(floor, item->Position.xPos, y, item->Position.zPos);

				if (item->ObjectNumber == ID_WHALE)
					top = STEP_SIZE / 2;
				else
					top = bounds->Y1;

				if (item->Position.yPos + top + dy < ceiling)
				{
					if (item->Position.yPos + top < ceiling)
					{
						item->Position.xPos = old.x;
						item->Position.zPos = old.z;
						dy = LOT->fly;
					}
					else
						dy = 0;
				}
			}
			else
			{
				floor = GetFloor(item->Position.xPos, y + STEP_SIZE, item->Position.zPos, &roomNumber);
				if (TestEnvironment(ENV_FLAG_WATER, roomNumber) ||
					TestEnvironment(ENV_FLAG_SWAMP, roomNumber))
				{
					dy = -LOT->fly;
				}
			}
		}
		else if (item->Position.yPos <= height)
		{
			dy = 0;
			item->Position.yPos = height;
		}
		else
		{
			item->Position.xPos = old.x;
			item->Position.zPos = old.z;
			dy = -LOT->fly;
		}

		item->Position.yPos += dy;
		floor = GetFloor(item->Position.xPos, y, item->Position.zPos, &roomNumber);
		item->Floor = GetFloorHeight(floor, item->Position.xPos, y, item->Position.zPos);
 
		angle = (item->VerticalVelocity) ? phd_atan(item->VerticalVelocity, -dy) : 0;
		if (angle < -ANGLE(20))
			angle = -ANGLE(20);
		else if (angle > ANGLE(20))
			angle = ANGLE(20);

		if (angle < item->Position.xRot - ANGLE(1))
			item->Position.xRot -= ANGLE(1);
		else if (angle > item->Position.xRot + ANGLE(1))
			item->Position.xRot += ANGLE(1);
		else
			item->Position.xRot = angle;
	}
	else if (LOT->isJumping)
	{
		floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
		int height2 = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
		item->Floor = height2;

		if (LOT->isMonkeying)
		{
			ceiling = GetCeiling(floor, item->Position.xPos, y, item->Position.zPos);
			item->Position.yPos = ceiling - bounds->Y1;
		}
		else
		{
			if (item->Position.yPos > item->Floor)
			{
				if (item->Position.yPos > item->Floor + STEP_SIZE)
				{
					item->Position.xPos = old.x;
					item->Position.yPos = old.y;
					item->Position.zPos = old.z;
				}
				else
				{
					item->Position.yPos = item->Floor;
				}
			}
		}
	} 
	else
	{
		floor = GetFloor(item->Position.xPos, y, item->Position.zPos, &roomNumber);
		ceiling = GetCeiling(floor, item->Position.xPos, y, item->Position.zPos);

		if (item->ObjectNumber == ID_TYRANNOSAUR || item->ObjectNumber == ID_SHIVA || item->ObjectNumber == ID_MUTANT2)
			top = STEP_SIZE*3;
		else
			top = bounds->Y1; // TODO: check if Y1 or Y2

		if (item->Position.yPos + top < ceiling)
		{
			item->Position.xPos = old.x;
			item->Position.zPos = old.z;
			item->Position.yPos = old.y;
		}

		floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
		item->Floor = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);

		if (item->Position.yPos > item->Floor)
			item->Position.yPos = item->Floor;
		else if (item->Floor - item->Position.yPos > STEP_SIZE/4)
			item->Position.yPos += STEP_SIZE/4;
		else if (item->Position.yPos < item->Floor)
			item->Position.yPos = item->Floor;

		item->Position.xRot = 0;
	}

	/*roomNumber = item->roomNumber;
	if (!Objects[item->objectNumber].waterCreature)
	{
		GetFloor(item->pos.xPos, item->pos.yPos - STEP_SIZE*2, item->pos.zPos, &roomNumber);

		if (g_Level.Rooms[roomNumber].flags & ENV_FLAG_WATER)
			item->HitPoints = 0;
	}*/

	roomNumber = item->RoomNumber;
	GetFloor(item->Position.xPos, item->Position.yPos - STEP_SIZE * 2, item->Position.zPos, &roomNumber);
	if (item->RoomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	return true;
}

void CreatureDie(short itemNumber, int explode)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	item->HitPoints = -16384;
	item->Collidable = false;

	if (explode)
	{
		if (Objects[item->ObjectNumber].hitEffect)
			ExplodingDeath(itemNumber, ALL_MESHBITS, EXPLODE_HIT_EFFECT);
		else
			ExplodingDeath(itemNumber, ALL_MESHBITS, EXPLODE_NORMAL);

		KillItem(itemNumber);
	}
	else
	{
		RemoveActiveItem(itemNumber);
	}

	DisableEntityAI(itemNumber);
	item->Flags |= IFLAG_KILLED | IFLAG_INVISIBLE;
	DropBaddyPickups(item);
}

int BadFloor(int x, int y, int z, int boxHeight, int nextHeight, short roomNumber, LOT_INFO* LOT)
{
	FLOOR_INFO* floor;
	int height;

	floor = GetFloor(x, y, z, &roomNumber);
	if (floor->Box == NO_BOX)
		return true;

	if (LOT->isJumping)
		return false;

	if (g_Level.Boxes[floor->Box].flags & LOT->blockMask)
		return true;

	height = g_Level.Boxes[floor->Box].height;
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
	obj = &Objects[item->ObjectNumber];
	x = item->Position.xPos;
	z = item->Position.zPos;
	radius = obj->radius;

	r = &g_Level.Rooms[item->RoomNumber];
	link = r->itemNumber;
	do
	{
		linked = &g_Level.Items[link];
		
		if (link != itemNumber && linked != LaraItem && linked->Status == ITEM_ACTIVE && linked->HitPoints > 0)
		{
			xDistance = abs(linked->Position.xPos - x);
			zDistance = abs(linked->Position.zPos - z);
			
			if (xDistance > zDistance)
				distance = xDistance + (zDistance >> 1);
			else
				distance = xDistance + (zDistance >> 1);

			if (distance < radius + Objects[linked->ObjectNumber].radius)
				return phd_atan(linked->Position.zPos - z, linked->Position.xPos - x) - item->Position.yRot;
		}

		link = linked->NextItem;
	} while (link != NO_ITEM);

	return 0;
}

int ValidBox(ITEM_INFO* item, short zoneNumber, short boxNumber) 
{
	if (boxNumber == NO_BOX)
		return false;

	CREATURE_INFO* creature;
	BOX_INFO* box;
	int* zone;

	creature = (CREATURE_INFO*)item->Data;
	zone = g_Level.Zones[creature->LOT.zone][FlipStatus].data();
	if (creature->LOT.fly == NO_FLYING && zone[boxNumber] != zoneNumber)
		return false;

	box = &g_Level.Boxes[boxNumber];
	if (box->flags & creature->LOT.blockMask)
		return false;

	if ((item->Position.zPos > (box->left * SECTOR(1))) && item->Position.zPos < ((box->right * SECTOR(1))) &&
		(item->Position.xPos > (box->top * SECTOR(1))) && item->Position.xPos < ((box->bottom * SECTOR(1))))
		return false;

	return true;
}

int EscapeBox(ITEM_INFO* item, ITEM_INFO* enemy, int boxNumber) 
{
	if (boxNumber == NO_BOX)
		return false;

	BOX_INFO* box = &g_Level.Boxes[boxNumber];
	int x, z;

	x = (box->top + box->bottom) * SECTOR(1) / 2 - enemy->Position.xPos;
	z = (box->left + box->right) * SECTOR(1) / 2 - enemy->Position.zPos;
	
	if (x > -ESCAPE_DIST && x < ESCAPE_DIST && z > -ESCAPE_DIST && z < ESCAPE_DIST)
		return false;

	if (((x > 0) ^ (item->Position.xPos > enemy->Position.xPos)) && ((z > 0) ^ (item->Position.zPos > enemy->Position.zPos)))
		return false;

	return true;
}

void TargetBox(LOT_INFO* LOT, int boxNumber)
{
	if (boxNumber == NO_BOX)
		return;

	BOX_INFO* box;

	boxNumber &= NO_BOX;
	box = &g_Level.Boxes[boxNumber];

	LOT->target.x = ((box->top * SECTOR(1)) + GetRandomControl() * ((box->bottom - box->top) - 1) >> 5) + WALL_SIZE / 2;
	LOT->target.z = ((box->left * SECTOR(1)) + GetRandomControl() * ((box->right - box->left) - 1) >> 5) + WALL_SIZE / 2;
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
				if ((delta > LOT->step || delta < LOT->drop) && (!(flags & BOX_MONKEY) || !LOT->canMonkey))
					continue;

				if ((flags & BOX_JUMP) && !LOT->canJump)
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


#if CREATURE_AI_PRIORITY_OPTIMIZATION
CREATURE_AI_PRIORITY GetCreatureLOTPriority(ITEM_INFO* item) {
	Vector3 itemPos = Vector3(item->Position.xPos, item->Position.yPos, item->Position.zPos);
	Vector3 cameraPos = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);
	float distance = Vector3::Distance(itemPos, cameraPos);
	distance /= SECTOR(1);
	if(distance <= HIGH_PRIO_RANGE)
		return CREATURE_AI_PRIORITY::HIGH;
	if(distance <= MEDIUM_PRIO_RANGE)
		return CREATURE_AI_PRIORITY::MEDIUM;
	if(distance <= LOW_PRIO_RANGE)
		return CREATURE_AI_PRIORITY::LOW;
	return CREATURE_AI_PRIORITY::NONE;
}
#endif
int CreatureActive(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (!Objects[item->ObjectNumber].intelligent)
		return false; // Object is not a creature

	if (item->Flags & IFLAG_KILLED)
		return false; // Object is already dead

	if (item->Status == ITEM_INVISIBLE || !item->Data.is<CREATURE_INFO>())
	{
		if (!EnableBaddieAI(itemNumber, 0))
			return false; // AI couldn't be activated

		item->Status = ITEM_ACTIVE;
	}

#ifdef CREATURE_AI_PRIORITY_OPTIMIZATION
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
	creature->priority = GetCreatureLOTPriority(item);
#endif // CREATURE_AI_PRIORITY_OPTIMIZATION

	return true;
}

void InitialiseCreature(short itemNumber) 
{
	ClearItem(itemNumber);
}

int StalkBox(ITEM_INFO* item, ITEM_INFO* enemy, int boxNumber)
{
	if (boxNumber == NO_BOX)
		return false;

	if (enemy == NULL)
		return false;

	BOX_INFO* box;
	int x, z, xrange, zrange;
	int enemyQuad, boxQuad, baddieQuad;

	box = &g_Level.Boxes[boxNumber];

	xrange = STALK_DIST + ((box->bottom - box->top + 3) * SECTOR(1));
	zrange = STALK_DIST + ((box->right - box->left + 3) * SECTOR(1));
	x = (box->top + box->bottom) * SECTOR(1) / 2 - enemy->Position.xPos;
	z = (box->left + box->right) * SECTOR(1) / 2 - enemy->Position.zPos;
	
	if (x > xrange || x < -xrange || z > zrange || z < -zrange)
		return false;

	enemyQuad = enemy->Position.yRot / ANGLE(90) + 2;
	
	if (z > 0)
		boxQuad = (x > 0) ? 2 : 1;
	else
		boxQuad = (x > 0) ? 3 : 0;

	if (enemyQuad == boxQuad)
		return false;

	baddieQuad = 0;
	if (item->Position.zPos > enemy->Position.zPos)
		baddieQuad = (item->Position.xPos > enemy->Position.xPos) ? 2 : 1;
	else
		baddieQuad = (item->Position.xPos > enemy->Position.xPos) ? 3 : 0;

	if (enemyQuad == baddieQuad && abs(enemyQuad - boxQuad) == 2)
		return false;

	return true;
}

int CreatureVault(short itemNum, short angle, int vault, int shift)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;

	int xBlock, zBlock, y, newXblock, newZblock;
	short roomNumber;

	xBlock = item->Position.xPos / SECTOR(1);
	zBlock = item->Position.zPos / SECTOR(1);
	y = item->Position.yPos;
	roomNumber = item->RoomNumber;

	CreatureAnimation(itemNum, angle, 0);

	if (item->Floor > y + CHECK_CLICK(9))
		vault = 0;
	else if (item->Floor > y + CHECK_CLICK(7))
		vault = -4;
	// FIXME: edit assets adding climb down animations for Von Croy and baddies?
	else if (item->Floor > y + CHECK_CLICK(5)
		&& item->ObjectNumber != ID_VON_CROY
		&& item->ObjectNumber != ID_BADDY1
		&& item->ObjectNumber != ID_BADDY2)
		vault = -3;
	else if (item->Floor > y + CHECK_CLICK(3) 
		&& item->ObjectNumber != ID_VON_CROY
		&& item->ObjectNumber != ID_BADDY1 
		&& item->ObjectNumber != ID_BADDY2) 
		vault = -2;
	else if (item->Position.yPos > y - CHECK_CLICK(3))
		return 0;
	else if (item->Position.yPos > y - CHECK_CLICK(5))
		vault = 2;
	else if (item->Position.yPos > y - CHECK_CLICK(7))
		vault = 3;
	else
		vault = 4;

	// Jump
	newXblock = item->Position.xPos / SECTOR(1);
	newZblock = item->Position.zPos / SECTOR(1);

	if (zBlock == newZblock)
	{
		if (xBlock == newXblock)
			return 0;

		if (xBlock < newXblock)
		{
			item->Position.xPos = (newXblock * SECTOR(1)) - shift;
			item->Position.yRot = ANGLE(90);
		}
		else
		{
			item->Position.xPos = (xBlock * SECTOR(1)) + shift;
			item->Position.yRot = -ANGLE(90);
		}
	}
	else if (xBlock == newXblock)
	{
		if (zBlock < newZblock)
		{
			item->Position.zPos = (newZblock * SECTOR(1)) - shift;
			item->Position.yRot = 0;
		}
		else
		{
			item->Position.zPos = (zBlock * SECTOR(1)) + shift;
			item->Position.yRot = -ANGLE(180);
		}
	}

	item->Position.yPos = item->Floor = y;
	if (roomNumber != item->RoomNumber)
		ItemNewRoom(itemNum, roomNumber);

	return vault;
}

void GetAITarget(CREATURE_INFO* creature)
{
	ITEM_INFO* enemy;
	ITEM_INFO* item;
	ITEM_INFO* targetItem;
	FLOOR_INFO* floor;
	int i;
	short enemyObjectNumber;

	enemy = creature->enemy;

	if (enemy)
		enemyObjectNumber = enemy->ObjectNumber;
	else
		enemyObjectNumber = NO_ITEM;

	item = &g_Level.Items[creature->itemNum];

	if (item->AIBits & GUARD)
	{
		creature->enemy = LaraItem;
		if (creature->alerted)
		{
			item->AIBits = ~GUARD;
			if (item->AIBits & AMBUSH)
				item->AIBits |= MODIFY;
		}
	}
	else if (item->AIBits & PATROL1)
	{
		if (creature->alerted || creature->hurtByLara)
		{
			item->AIBits &= ~PATROL1;
			if (item->AIBits & AMBUSH)
			{
				item->AIBits |= MODIFY;
				// NOTE: added in TR5
				//item->itemFlags[3] = (item->TOSSPAD & 0xFF);
			}
		}
		else if (!creature->patrol2)
		{
			if (enemyObjectNumber != ID_AI_PATROL1)
			{
				FindAITargetObject(creature, ID_AI_PATROL1);
			}
		}
		else if (enemyObjectNumber != ID_AI_PATROL2)
		{
			FindAITargetObject(creature, ID_AI_PATROL2);
		}
		else if (abs(enemy->Position.xPos - item->Position.xPos) < REACHED_GOAL_RADIUS &&
			abs(enemy->Position.yPos - item->Position.yPos) < REACHED_GOAL_RADIUS &&
			abs(enemy->Position.zPos - item->Position.zPos) < REACHED_GOAL_RADIUS
			|| Objects[item->ObjectNumber].waterCreature)
		{
			TestTriggers(enemy, true);
			creature->patrol2 = !creature->patrol2;
		}
	}
	else if (item->AIBits & AMBUSH)
	{
		// First if was removed probably after TR3 and was it used by monkeys?
		/*if (!(item->aiBits & MODIFY) && !creature->hurtByLara)
		{
			creature->enemy = LaraItem;
		}
		else*/ if (enemyObjectNumber != ID_AI_AMBUSH)
		{
			FindAITargetObject(creature, ID_AI_AMBUSH);
		}
		/*else if (item->objectNumber == ID_MONKEY)
		{
			return;
		}*/
		else if (abs(enemy->Position.xPos - item->Position.xPos) < REACHED_GOAL_RADIUS &&
			abs(enemy->Position.yPos - item->Position.yPos) < REACHED_GOAL_RADIUS &&
			abs(enemy->Position.zPos - item->Position.zPos) < REACHED_GOAL_RADIUS)
		{
			TestTriggers(enemy, true);

			creature->reachedGoal = true;
			creature->enemy = LaraItem;
			item->AIBits &= ~(AMBUSH /* | MODIFY*/);
			if (item->AIBits != MODIFY)
			{
				item->AIBits |= GUARD;
				creature->alerted = false;
			}
		}
	}
	else if (item->AIBits & FOLLOW)
	{
		if (creature->hurtByLara)
		{
			creature->enemy = LaraItem;
			creature->alerted = true;
			//item->aiBits &= ~FOLLOW;
		}
		else if (item->HitStatus)
		{
			item->AIBits &= ~FOLLOW;
		}
		else if (enemyObjectNumber != ID_AI_FOLLOW)
		{
			FindAITargetObject(creature, ID_AI_FOLLOW);
		}
		else if (abs(enemy->Position.xPos - item->Position.xPos) < REACHED_GOAL_RADIUS &&
			abs(enemy->Position.yPos - item->Position.yPos) < REACHED_GOAL_RADIUS &&
			abs(enemy->Position.zPos - item->Position.zPos) < REACHED_GOAL_RADIUS)
		{
			creature->reachedGoal = true;
			item->AIBits &= ~FOLLOW;
		}
	}
	/*else if (item->objectNumber == ID_MONKEY && item->carriedItem == NO_ITEM)
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
	}*/
}

// TR3 old way..
void FindAITarget(CREATURE_INFO* creature, short objectNumber)
{
	ITEM_INFO* item = &g_Level.Items[creature->itemNum];
	ITEM_INFO* targetItem;
	int i;

	for (i = 0, targetItem = &g_Level.Items[0]; i < g_Level.NumItems; i++, targetItem++)
	{
		if (targetItem->ObjectNumber == objectNumber && targetItem->RoomNumber != NO_ROOM)
		{
			if (SameZone(creature, targetItem) && targetItem->Position.yRot == item->ItemFlags[3])
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
		AI_OBJECT* foundObject = NULL;

		for (int i = 0; i < g_Level.AIObjects.size(); i++)
		{
			AI_OBJECT* aiObject = &g_Level.AIObjects[i];

			if (aiObject->objectNumber == objectNumber && aiObject->triggerFlags == item->ItemFlags[3] && aiObject->roomNumber != NO_ROOM)
			{
				int* zone = g_Level.Zones[creature->LOT.zone][FlipStatus].data();

				ROOM_INFO* r = &g_Level.Rooms[item->RoomNumber];
				item->BoxNumber = GetSector(r, item->Position.xPos - r->x, item->Position.zPos - r->z)->Box;

				r = &g_Level.Rooms[aiObject->roomNumber];
				aiObject->boxNumber = GetSector(r, aiObject->x - r->x, aiObject->z - r->z)->Box;

				if (item->BoxNumber == NO_BOX || aiObject->boxNumber == NO_BOX)
				{
					return;
				}

				if (zone[item->BoxNumber] == zone[aiObject->boxNumber])
				{
					foundObject = aiObject;
					break;
				}
			}
		}

		if (foundObject != NULL)
		{
			ITEM_INFO* aiItem = creature->aiTarget;

			creature->enemy = aiItem;

			aiItem->ObjectNumber = foundObject->objectNumber;
			aiItem->RoomNumber = foundObject->roomNumber;
			aiItem->Position.xPos = foundObject->x;
			aiItem->Position.yPos = foundObject->y;
			aiItem->Position.zPos = foundObject->z;
			aiItem->Position.yRot = foundObject->yRot;
			aiItem->Flags = foundObject->flags;
			aiItem->TriggerFlags = foundObject->triggerFlags;
			aiItem->BoxNumber = foundObject->boxNumber;

			if (!(creature->aiTarget->Flags & 32))
			{
				creature->aiTarget->Position.xPos += phd_sin(creature->aiTarget->Position.yRot) * 256;
				creature->aiTarget->Position.zPos += phd_cos(creature->aiTarget->Position.yRot) * 256;
			}
		}
	}
}

void CreatureAIInfo(ITEM_INFO* item, AI_INFO* info)
{
	if (!item->Data)
		return;

	CREATURE_INFO * creature;
	ITEM_INFO * enemy;
	OBJECT_INFO * obj;
	ROOM_INFO * r;
	short angle;
	int* zone;
	int x, y, z;

	creature = (CREATURE_INFO*)item->Data;
	obj = &Objects[item->ObjectNumber];

	enemy = creature->enemy;
	if (!enemy)
	{
		enemy = LaraItem;
		creature->enemy = LaraItem;
	}

	zone = g_Level.Zones[creature->LOT.zone][FlipStatus].data();

	r = &g_Level.Rooms[item->RoomNumber];
	item->BoxNumber = NO_BOX;
	FLOOR_INFO* floor = GetSector(r, item->Position.xPos - r->x, item->Position.zPos - r->z);
	if(floor)
		item->BoxNumber = floor->Box;
	if (item->BoxNumber != NO_BOX)
		info->zoneNumber = zone[item->BoxNumber];
	else
		info->zoneNumber = NO_ZONE;

	r = &g_Level.Rooms[enemy->RoomNumber];
	enemy->BoxNumber = NO_BOX;
	floor = GetSector(r, enemy->Position.xPos - r->x, enemy->Position.zPos - r->z);
	if(floor)
		enemy->BoxNumber = floor->Box;
	if (enemy->BoxNumber != NO_BOX)
		info->enemyZone = zone[enemy->BoxNumber];
	else
		info->enemyZone = NO_ZONE;

	if (!obj->nonLot)
	{
		if (enemy->BoxNumber != NO_BOX && g_Level.Boxes[enemy->BoxNumber].flags & creature->LOT.blockMask)
			info->enemyZone |= BLOCKED;
		else if (item->BoxNumber != NO_BOX && creature->LOT.node[item->BoxNumber].searchNumber == (creature->LOT.searchNumber | BLOCKED_SEARCH))
			info->enemyZone |= BLOCKED;
	}

	if (enemy == LaraItem)
	{
		x = enemy->Position.xPos + enemy->VerticalVelocity * PREDICTIVE_SCALE_FACTOR * phd_sin(Lara.Control.MoveAngle) - item->Position.xPos - obj->pivotLength * phd_sin(item->Position.yRot);
		z = enemy->Position.zPos + enemy->VerticalVelocity * PREDICTIVE_SCALE_FACTOR * phd_cos(Lara.Control.MoveAngle) - item->Position.zPos - obj->pivotLength * phd_cos(item->Position.yRot);
	}
	else
	{
		x = enemy->Position.xPos + enemy->VerticalVelocity * PREDICTIVE_SCALE_FACTOR * phd_sin(enemy->Position.yRot) - item->Position.xPos - obj->pivotLength * phd_sin(item->Position.yRot);
		z = enemy->Position.zPos + enemy->VerticalVelocity * PREDICTIVE_SCALE_FACTOR * phd_cos(enemy->Position.yRot) - item->Position.zPos - obj->pivotLength * phd_cos(item->Position.yRot);
	}

	y = item->Position.yPos - enemy->Position.yPos;
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

	info->angle = angle - item->Position.yRot;
	info->enemyFacing = 0x8000 + angle - enemy->Position.yRot;

	x = abs(x);
	z = abs(z);

	// Makes Lara smaller
	if (enemy == LaraItem && ((LaraInfo*)enemy)->Control.IsLow)
		y -= STEPUP_HEIGHT;

	if (x > z)
		info->xAngle = phd_atan(x + (z >> 1), y);
	else
		info->xAngle = phd_atan(z + (x >> 1), y);

	info->ahead = (info->angle > -FRONT_ARC && info->angle < FRONT_ARC);
	info->bite = (info->ahead && enemy->HitPoints > 0 && abs(enemy->Position.yPos - item->Position.yPos) <= (STEP_SIZE * 2));
}

void CreatureMood(ITEM_INFO* item, AI_INFO* info, int violent)
{
	if (!item->Data)
		return;

	int boxNumber;

	auto creature = (CREATURE_INFO*)item->Data;
	auto enemy = creature->enemy;
	auto LOT = &creature->LOT;

	if (enemy != nullptr)
	{
		switch (creature->mood)
		{
		case BORED_MOOD:
			boxNumber = LOT->node[GetRandomControl() * LOT->zoneCount >> 15].boxNumber;
			if (ValidBox(item, info->zoneNumber, boxNumber)
				&& !(GetRandomControl() & 0x0F))
			{
				if (StalkBox(item, enemy, boxNumber) && enemy->HitPoints > 0 && creature->enemy)
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
			LOT->target.x = enemy->Position.xPos;
			LOT->target.y = enemy->Position.yPos;
			LOT->target.z = enemy->Position.zPos;
			LOT->requiredBox = enemy->BoxNumber;

			if (LOT->fly != NO_FLYING && Lara.Control.WaterStatus == WaterStatus::Dry)
			{
				auto bounds = (BOUNDING_BOX*)GetBestFrame(enemy);
				LOT->target.y += bounds->Y1;
			}
			break;

		case ESCAPE_MOOD:
			boxNumber = LOT->node[GetRandomControl() * LOT->zoneCount >> 15].boxNumber;
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
				boxNumber = LOT->node[GetRandomControl() * LOT->zoneCount >> 15].boxNumber;
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
	}

	if (LOT->targetBox == NO_BOX)
		TargetBox(LOT, item->BoxNumber);

#ifdef CREATURE_AI_PRIORITY_OPTIMIZATION
	bool shouldUpdateTarget = false;
	switch(creature->priority) {
	case CREATURE_AI_PRIORITY::HIGH:
		shouldUpdateTarget = true;
	break;
	case CREATURE_AI_PRIORITY::MEDIUM:
	{
		if(creature->framesSinceLOTUpdate > std::pow(FRAME_PRIO_BASE, FRAME_PRIO_EXP))
			shouldUpdateTarget = true;
	}
	break;
	case CREATURE_AI_PRIORITY::LOW:
	{
		if(creature->framesSinceLOTUpdate > std::pow(FRAME_PRIO_BASE,FRAME_PRIO_EXP*2))
			shouldUpdateTarget = true;
	}
	break;
	default:
		break;
	}
	if(shouldUpdateTarget) {
		CalculateTarget(&creature->target, item, &creature->LOT);
		creature->framesSinceLOTUpdate = 0;
	} else {
		creature->framesSinceLOTUpdate++;
	}
#else
	CalculateTarget(&creature->target, item, &creature->LOT);
#endif // CREATURE_AI_PRIORITY_OPTIMIZATION

	creature->jumpAhead = false;
	creature->monkeyAhead = false;

	if (item->BoxNumber != NO_BOX)
	{
		auto startBox = LOT->node[item->BoxNumber].exitBox;
		if (startBox != NO_BOX)
		{
			int overlapIndex = g_Level.Boxes[item->BoxNumber].overlapIndex;
			int nextBox = 0;
			int flags = 0;

			if (overlapIndex >= 0)
			{
				do
				{
					nextBox = g_Level.Overlaps[overlapIndex].box;
					flags = g_Level.Overlaps[overlapIndex++].flags;
				} while (nextBox != NO_BOX && ((flags & BOX_END_BIT) == false) && (nextBox != startBox));
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
}

void GetCreatureMood(ITEM_INFO* item, AI_INFO* info, int isViolent)
{
	if (!item->Data)
		return;

	CREATURE_INFO* creature;
	LOT_INFO* LOT;
	ITEM_INFO* enemy;
	MOOD_TYPE mood;

	creature = (CREATURE_INFO*)item->Data;
	enemy = creature->enemy;
	LOT = &creature->LOT;

	if (item->BoxNumber == NO_BOX || creature->LOT.node[item->BoxNumber].searchNumber == (creature->LOT.searchNumber | BLOCKED_SEARCH))
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
	else if (enemy->HitPoints <= 0 && enemy == LaraItem)
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
				else if (item->HitStatus)
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
				if (item->HitStatus 
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

	target->x = item->Position.xPos;
	target->y = item->Position.yPos;
	target->z = item->Position.zPos;

	boxNumber = item->BoxNumber;
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

		if (item->Position.zPos >= boxLeft && item->Position.zPos <= boxRight &&
			item->Position.xPos >= boxTop && item->Position.xPos <= boxBottom)
		{
			left = boxLeft;
			right = boxRight;
			top = boxTop;
			bottom = boxBottom;
		}
		else
		{
			if (item->Position.zPos < boxLeft)
			{
				if ((direction & CLIP_LEFT) && item->Position.xPos >= boxTop && item->Position.xPos <= boxBottom)
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
			else if (item->Position.zPos > boxRight)
			{
				if ((direction & CLIP_RIGHT) && item->Position.xPos >= boxTop && item->Position.xPos <= boxBottom)
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

			if (item->Position.xPos < boxTop)
			{
				if ((direction & CLIP_TOP) && item->Position.zPos >= boxLeft && item->Position.zPos <= boxRight)
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
			else if (item->Position.xPos > boxBottom)
			{
				if ((direction & CLIP_BOTTOM) && item->Position.zPos >= boxLeft && item->Position.zPos <= boxRight)
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
		target->z = boxLeft + WALL_SIZE / 2 + (GetRandomControl() * (boxRight - boxLeft - WALL_SIZE) >> 15);
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
		target->x = boxTop + WALL_SIZE / 2 + (GetRandomControl() * (boxBottom - boxTop - WALL_SIZE) >> 15);
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

void AdjustStopperFlag(ITEM_INFO* item, int direction, bool set)
{
	int x = item->Position.xPos;
	int z = item->Position.zPos;

	auto* room = &g_Level.Rooms[item->RoomNumber];

	FLOOR_INFO* floor = GetSector(room, x - room->x, z - room->z);
	floor->Stopper = set;

	x = item->Position.xPos + 1024 * phd_sin(direction);
	z = item->Position.zPos + 1024 * phd_cos(direction);

	short roomNumber = item->RoomNumber;
	GetFloor(x, item->Position.yPos, z, &roomNumber);
	room = &g_Level.Rooms[roomNumber];

	floor = GetSector(room, x - room->x, z - room->z);
	floor->Stopper = set;
}

void InitialiseItemBoxData()
{
	for (int i = 0; i < g_Level.Items.size(); i++)
	{
		auto* item = &g_Level.Items[i];

		if (item->Active && item->Data.is<PUSHABLE_INFO>())
			ClearMovableBlockSplitters(item->Position.xPos, item->Position.yPos, item->Position.zPos, item->RoomNumber);
	}

	for (auto& r : g_Level.Rooms)
	{
		for (const auto& mesh : r.mesh)
		{
			long index = ((mesh.pos.zPos - r.z) / 1024) + r.zSize * ((mesh.pos.xPos - r.x) / 1024);

			if (index > r.floor.size())
				continue;

			FLOOR_INFO* floor = &r.floor[index];

			if (floor->Box == NO_BOX)
				continue;

			if (!(g_Level.Boxes[floor->Box].flags & BLOCKED))
			{
				int floorHeight = floor->FloorHeight(mesh.pos.xPos, mesh.pos.zPos);
				auto* staticInfo = &StaticObjects[mesh.staticNumber];

				if (floorHeight <= mesh.pos.yPos - staticInfo->collisionBox.Y2 + 512 &&
					floorHeight < mesh.pos.yPos - staticInfo->collisionBox.Y1)
				{
					if (staticInfo->collisionBox.X1 == 0 || staticInfo->collisionBox.X2 == 0 ||
						staticInfo->collisionBox.Z1 == 0 || staticInfo->collisionBox.Z2 == 0 ||
						((staticInfo->collisionBox.X1 < 0) ^ (staticInfo->collisionBox.X2 < 0)) &&
						((staticInfo->collisionBox.Z1 < 0) ^ (staticInfo->collisionBox.Z2 < 0)))
					{
						floor->Stopper = true;
					}
				}
			}
		}
	}
}
