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
#include "Game/misc.h"
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

void DropEntityPickups(ITEM_INFO* item)
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

int MoveCreature3DPos(PHD_3DPOS* srcPos, PHD_3DPOS* destPos, int velocity, short angleDif, int angleAdd)
{
	int x = destPos->xPos - srcPos->xPos;
	int y = destPos->yPos - srcPos->yPos;
	int z = destPos->zPos - srcPos->zPos;
	int distance = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));

	if (velocity < distance)
	{
		srcPos->xPos += x * velocity / distance;
		srcPos->yPos += y * velocity / distance;
		srcPos->zPos += z * velocity / distance;
	}
	else
	{
		srcPos->xPos = destPos->xPos;
		srcPos->yPos = destPos->yPos;
		srcPos->zPos = destPos->zPos;
	}

	if (angleDif <= angleAdd)
	{
		if (angleDif >= -angleAdd)
			srcPos->yRot = destPos->yRot;
		else
			srcPos->yRot -= angleAdd;
	}
	else
		srcPos->yRot += angleAdd;

	return (srcPos->xPos == destPos->xPos &&
		srcPos->yPos == destPos->yPos &&
		srcPos->zPos == destPos->zPos &&
		srcPos->yRot == destPos->yRot);
}

void CreatureYRot2(PHD_3DPOS* srcPos, short angle, short angadd) 
{
	if (angadd < angle)
	{
		srcPos->yRot += angadd;
		return;
	} 

	if (angle < -angadd)
	{
		srcPos->yRot -= angadd;
		return;
	} 

	srcPos->yRot += angle;
}

bool SameZone(CreatureInfo* creature, ITEM_INFO* target)
{
	int* zone = g_Level.Zones[creature->LOT.Zone][FlipStatus].data();
	auto* item = &g_Level.Items[creature->ItemNumber];

	auto* room = &g_Level.Rooms[item->RoomNumber];
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

short AIGuard(CreatureInfo* creature) 
{
	auto* item = &g_Level.Items[creature->ItemNumber];
	if (item->AIBits & (GUARD | PATROL1))
		return 0;

	int random = GetRandomControl();

	if (random < 256)
	{
		creature->HeadRight = true;
		creature->HeadLeft = true;
	}
	else if (random < 384)
	{
		creature->HeadRight = false;
		creature->HeadLeft = true;
	}
	else if (random < 512)
	{
		creature->HeadRight = true;
		creature->HeadLeft = false;
	}

	if (!creature->HeadLeft)
		return (creature->HeadRight) << 12;

	if (creature->HeadRight)
		return 0;

	return -ANGLE(90.0f);
}

void AlertNearbyGuards(ITEM_INFO* item) 
{
	for (int i = 0; i < ActiveCreatures.size(); i++)
	{
		auto* currentCreature = ActiveCreatures[i];
		if (currentCreature->ItemNumber == NO_ITEM)
			continue;

		auto* currentTarget = &g_Level.Items[currentCreature->ItemNumber + i];
		if (item->RoomNumber == currentTarget->RoomNumber)
		{
			currentCreature->Alerted = true;
			continue;
		}

		int x = (currentTarget->Position.xPos - item->Position.xPos) / 64;
		int y = (currentTarget->Position.yPos - item->Position.yPos) / 64;
		int z = (currentTarget->Position.zPos - item->Position.zPos) / 64;

		int distance = (pow(x, 2) + pow(y, 2) + pow(z, 2));
		if (distance < SECTOR(8))
			currentCreature->Alerted = true;
	}
}

void AlertAllGuards(short itemNumber) 
{
	for (int i = 0; i < ActiveCreatures.size(); i++)
	{
		auto* creature = ActiveCreatures[i];
		if (creature->ItemNumber == NO_ITEM)
			continue;

		auto* target = &g_Level.Items[creature->ItemNumber];
		short objNumber = g_Level.Items[itemNumber].ObjectNumber;
		if (objNumber == target->ObjectNumber)
		{
			if (target->Status == ITEM_ACTIVE)
				creature->Alerted = true;
		}
	}
}

void CreatureKill(ITEM_INFO* item, int killAnim, int killState, int laraKillState)
{
	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + killAnim;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.ActiveState = killState;

	LaraItem->Animation.AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex;
	LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
	LaraItem->Animation.ActiveState = 0;
	LaraItem->Animation.TargetState = laraKillState;

	LaraItem->Position.xPos = item->Position.xPos;
	LaraItem->Position.yPos = item->Position.yPos;
	LaraItem->Position.zPos = item->Position.zPos;
	LaraItem->Position.yRot = item->Position.yRot;
	LaraItem->Position.xRot = item->Position.xRot;
	LaraItem->Position.zRot = item->Position.zRot;
	LaraItem->Animation.Velocity = 0;
	LaraItem->Animation.VerticalVelocity = 0;
	LaraItem->Animation.Airborne = false;

	if (item->RoomNumber != LaraItem->RoomNumber)
		ItemNewRoom(Lara.ItemNumber, item->RoomNumber);

	AnimateItem(LaraItem);

	Lara.ExtraAnim = 1;
	Lara.Control.HandStatus = HandStatus::Busy;
	Lara.Control.Weapon.GunType = LaraWeaponType::None;
	Lara.HitDirection = -1;
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
	PHD_VECTOR pos = { bite->x, bite->y, bite->z };
	GetJointAbsPosition(item, &pos, bite->meshNum);

	return func(pos.x, pos.y, pos.z, damage, angle, item->RoomNumber);
}

short CreatureEffect(ITEM_INFO* item, BITE_INFO* bite, std::function<CreatureEffectFunction> func)
{
	PHD_VECTOR pos = { bite->x, bite->y, bite->z };
	GetJointAbsPosition(item, &pos, bite->meshNum);

	return func(pos.x, pos.y, pos.z, item->Animation.Velocity, item->Position.yRot, item->RoomNumber);
}

void CreatureUnderwater(ITEM_INFO* item, int depth)
{
	int waterLevel = depth;
	int wh = 0;

	if (depth < 0)
	{
		wh = abs(depth);
		waterLevel = 0;
	}
	else
		wh = GetWaterHeight(item);

	int y = wh + waterLevel;

	if (item->Position.yPos < y)
	{
		short roomNumber = item->RoomNumber;
		FLOOR_INFO* floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
		int height = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);

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
	auto* item = &g_Level.Items[itemNumber];
	item->HitPoints = NOT_TARGETABLE;
	item->Position.xRot = 0;

	int waterLevel = GetWaterHeight(item);

	int y = item->Position.yPos;
	if (y > waterLevel)
		item->Position.yPos = y - 32;
	if (item->Position.yPos < waterLevel)
		item->Position.yPos = waterLevel;

	AnimateItem(item);

	short roomNumber = item->RoomNumber;
	FLOOR_INFO* floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
	item->Floor = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
	
	if (roomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	if (item->Position.yPos <= waterLevel)
	{
		if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
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

	auto* creature = GetCreatureInfo(item);

	short change = required - creature->JointRotation[joint];
	if (change > ANGLE(3.0f))
		change = ANGLE(3.0f);
	else if (change < -ANGLE(3.0f))
		change = -ANGLE(3.0f);

	creature->JointRotation[joint] += change;

	if (creature->JointRotation[joint] > ANGLE(70.0f))
		creature->JointRotation[joint] = ANGLE(70.0f);
	else if (creature->JointRotation[joint] < -ANGLE(70.0f))
		creature->JointRotation[joint] = -ANGLE(70.0f);
}

void CreatureTilt(ITEM_INFO* item, short angle) 
{
	angle = (angle << 2) - item->Position.zRot;

	if (angle < -ANGLE(3.0f))
		angle = -ANGLE(3.0f);
	else if (angle > ANGLE(3.0f))
		angle = ANGLE(3.0f);

	short theAngle = -ANGLE(3.0f);

	short absRot = abs(item->Position.zRot);
	if (absRot < ANGLE(15.0f) || absRot > ANGLE(30.0f))
		angle >>= 1;
	
	item->Position.zRot += angle;
}

short CreatureTurn(ITEM_INFO* item, short maxTurn)
{
	if (!item->Data || maxTurn == 0)
		return 0;

	auto* creature = GetCreatureInfo(item);
	short angle = 0;

	int x = creature->Target.x - item->Position.xPos;
	int z = creature->Target.z - item->Position.zPos;
	angle = phd_atan(z, x) - item->Position.yRot;
	int range = item->Animation.Velocity * (16384 / maxTurn);
	int distance = pow(x, 2) + pow(z, 2);

	if (angle > FRONT_ARC || angle < -FRONT_ARC && distance < pow(range, 2))
		maxTurn >>= 1;

	if (angle > maxTurn)
		angle = maxTurn;
	else if (angle < -maxTurn)
		angle = -maxTurn;

	item->Position.yRot += angle;

	return angle;
}

int CreatureAnimation(short itemNumber, short angle, short tilt)
{
	int xPos, zPos, ceiling, shiftX, shiftZ;
	short top;

	auto* item = &g_Level.Items[itemNumber];
	if (!item->Data)
		return false;

	auto* creature = GetCreatureInfo(item);
	auto* LOT = &creature->LOT;
	int* zone = g_Level.Zones[LOT->Zone][FlipStatus].data();

	int boxHeight;
	if (item->BoxNumber != NO_BOX)
		boxHeight = g_Level.Boxes[item->BoxNumber].height;
	else
		boxHeight = item->Floor;

	PHD_VECTOR old = { item->Position.xPos, item->Position.yPos, item->Position.zPos };
	
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

	auto* bounds = GetBoundsAccurate(item);
	int y = item->Position.yPos + bounds->Y1;

	short roomNumber = item->RoomNumber;
	GetFloor(old.x, y, old.z, &roomNumber);  
	FLOOR_INFO* floor = GetFloor(item->Position.xPos, y, item->Position.zPos, &roomNumber);

	// TODO: Check why some blocks have box = -1 assigned to them -- Lwmte, 10.11.21
	if (floor->Box == NO_BOX)
		return false;

	int height = g_Level.Boxes[floor->Box].height;
	int nextHeight = 0;

	int nextBox;
	if (!Objects[item->ObjectNumber].nonLot)
		nextBox = LOT->Node[floor->Box].exitBox;
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

	if (floor->Box == NO_BOX || !LOT->IsJumping && (LOT->Fly == NO_FLYING && item->BoxNumber != NO_BOX && zone[item->BoxNumber] != zone[floor->Box] ||  boxHeight - height > LOT->Step ||  boxHeight - height < LOT->Drop))
	{
		xPos = item->Position.xPos / SECTOR(1);
		zPos = item->Position.zPos / SECTOR(1);
		shiftX = old.x / SECTOR(1);
		shiftZ = old.z / SECTOR(1);

		if (xPos < shiftX)
			item->Position.xPos = old.x & (~(SECTOR(1) - 1));
		else if (xPos > shiftX)
			item->Position.xPos = old.x | (SECTOR(1) - 1);

		if (zPos < shiftZ)
			item->Position.zPos = old.z & (~(SECTOR(1) - 1));
		else if (zPos > shiftZ)
			item->Position.zPos = old.z | (SECTOR(1) - 1);

		floor = GetFloor(item->Position.xPos, y, item->Position.zPos, &roomNumber);
		height = g_Level.Boxes[floor->Box].height;
		if (!Objects[item->ObjectNumber].nonLot)
			nextHeight = LOT->Node[floor->Box].exitBox;
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

	int x = item->Position.xPos;
	int z = item->Position.zPos;
	xPos = x & (SECTOR(1) - 1);
	zPos = z & (SECTOR(1) - 1);
	short radius = Objects[item->ObjectNumber].radius;
	shiftX = 0;
	shiftZ = 0;

	if (zPos < radius)
	{
		if (BadFloor(x, y, z - radius, height, nextHeight, roomNumber, LOT))
			shiftZ = radius - zPos;

		if (xPos < radius)
		{
			if (BadFloor(x-radius, y, z, height, nextHeight, roomNumber, LOT))
				shiftX = radius - xPos;
			else if (!shiftZ && BadFloor(x - radius, y, z - radius, height, nextHeight, roomNumber, LOT))
			{
				if (item->Position.yRot > -ANGLE(135.0f) && item->Position.yRot < ANGLE(45.0f))
					shiftZ = radius - zPos;
				else
					shiftX = radius - xPos;
			}
		}
		else if (xPos > SECTOR(1) - radius)
		{
			if (BadFloor(x + radius, y, z, height, nextHeight, roomNumber, LOT))
				shiftX = SECTOR(1) - radius - xPos;
			else if (!shiftZ && BadFloor(x + radius, y, z - radius, height, nextHeight, roomNumber, LOT))
			{
				if (item->Position.yRot > -ANGLE(45.0f) && item->Position.yRot < ANGLE(135.0f))
					shiftZ = radius - zPos;
				else
					shiftX = SECTOR(1) - radius - xPos;
			}
		}
	}
	else if (zPos > SECTOR(1) - radius)
	{
		if (BadFloor(x, y, z + radius, height, nextHeight, roomNumber, LOT))
			shiftZ = SECTOR(1) - radius - zPos;

		if (xPos < radius)
		{
			if (BadFloor(x - radius, y, z, height, nextHeight, roomNumber, LOT))
				shiftX = radius - xPos;
			else if (!shiftZ && BadFloor(x - radius, y, z + radius, height, nextHeight, roomNumber, LOT))
			{
				if (item->Position.yRot > -ANGLE(45.0f) && item->Position.yRot < ANGLE(135.0f))
					shiftX = radius - xPos;
				else
					shiftZ = SECTOR(1) - radius - zPos;
			}
		}
		else if (xPos > SECTOR(1) - radius)
		{
			if (BadFloor(x + radius, y, z, height, nextHeight, roomNumber, LOT))
				shiftX = SECTOR(1) - radius - xPos;
			else if (!shiftZ && BadFloor(x + radius, y, z + radius, height, nextHeight, roomNumber, LOT))
			{
				if (item->Position.yRot > -ANGLE(135.0f) && item->Position.yRot < ANGLE(45.0f))
					shiftX = SECTOR(1) - radius - xPos;
				else
					shiftZ = SECTOR(1) - radius - zPos;
			}
		}
	}
	else if (xPos < radius)
	{
		if (BadFloor(x - radius, y, z, height, nextHeight, roomNumber, LOT))
			shiftX = radius - xPos;
	}
	else if (xPos > SECTOR(1) - radius)
	{
		if (BadFloor(x + radius, y, z, height, nextHeight, roomNumber, LOT))
			shiftX = SECTOR(1) - radius - xPos;
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

	short biffAngle;
	if (item->ObjectNumber != ID_TYRANNOSAUR && item->Animation.Velocity && item->HitPoints > 0)
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

	if (LOT->Fly != NO_FLYING && item->HitPoints > 0)
	{
		int dy = creature->Target.y - item->Position.yPos;
		if (dy > LOT->Fly)
			dy = LOT->Fly;
		else if (dy < -LOT->Fly)
			dy = -LOT->Fly;

		height = GetFloorHeight(floor, item->Position.xPos, y, item->Position.zPos);
		if (item->Position.yPos + dy <= height)
		{
			if (Objects[item->ObjectNumber].waterCreature)
			{
				ceiling = GetCeiling(floor, item->Position.xPos, y, item->Position.zPos);

				if (item->ObjectNumber == ID_WHALE)
					top = CLICK(0.5f);
				else
					top = bounds->Y1;

				if (item->Position.yPos + top + dy < ceiling)
				{
					if (item->Position.yPos + top < ceiling)
					{
						item->Position.xPos = old.x;
						item->Position.zPos = old.z;
						dy = LOT->Fly;
					}
					else
						dy = 0;
				}
			}
			else
			{
				floor = GetFloor(item->Position.xPos, y + CLICK(1), item->Position.zPos, &roomNumber);
				if (TestEnvironment(ENV_FLAG_WATER, roomNumber) ||
					TestEnvironment(ENV_FLAG_SWAMP, roomNumber))
				{
					dy = -LOT->Fly;
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
			dy = -LOT->Fly;
		}

		item->Position.yPos += dy;
		floor = GetFloor(item->Position.xPos, y, item->Position.zPos, &roomNumber);
		item->Floor = GetFloorHeight(floor, item->Position.xPos, y, item->Position.zPos);
 
		angle = (item->Animation.Velocity) ? phd_atan(item->Animation.Velocity, -dy) : 0;
		if (angle < -ANGLE(20.0f))
			angle = -ANGLE(20.0f);
		else if (angle > ANGLE(20.0f))
			angle = ANGLE(20.0f);

		if (angle < item->Position.xRot - ANGLE(1.0f))
			item->Position.xRot -= ANGLE(1.0f);
		else if (angle > item->Position.xRot + ANGLE(1.0f))
			item->Position.xRot += ANGLE(1.0f);
		else
			item->Position.xRot = angle;
	}
	else if (LOT->IsJumping)
	{
		floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
		int height2 = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
		item->Floor = height2;

		if (LOT->IsMonkeying)
		{
			ceiling = GetCeiling(floor, item->Position.xPos, y, item->Position.zPos);
			item->Position.yPos = ceiling - bounds->Y1;
		}
		else
		{
			if (item->Position.yPos > item->Floor)
			{
				if (item->Position.yPos > (item->Floor + CLICK(1)))
				{
					item->Position.xPos = old.x;
					item->Position.yPos = old.y;
					item->Position.zPos = old.z;
				}
				else
					item->Position.yPos = item->Floor;
			}
		}
	} 
	else
	{
		floor = GetFloor(item->Position.xPos, y, item->Position.zPos, &roomNumber);
		ceiling = GetCeiling(floor, item->Position.xPos, y, item->Position.zPos);

		if (item->ObjectNumber == ID_TYRANNOSAUR || item->ObjectNumber == ID_SHIVA || item->ObjectNumber == ID_MUTANT2)
			top = CLICK(3);
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
	GetFloor(item->Position.xPos, item->Position.yPos - CLICK(2), item->Position.zPos, &roomNumber);
	if (item->RoomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	return true;
}

void CreatureDie(short itemNumber, int explode)
{
	auto* item = &g_Level.Items[itemNumber];

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
		RemoveActiveItem(itemNumber);

	DisableEntityAI(itemNumber);
	item->Flags |= IFLAG_KILLED | IFLAG_INVISIBLE;
	DropEntityPickups(item);
}

int BadFloor(int x, int y, int z, int boxHeight, int nextHeight, short roomNumber, LOTInfo* LOT)
{
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	if (floor->Box == NO_BOX)
		return true;

	if (LOT->IsJumping)
		return false;

	if (g_Level.Boxes[floor->Box].flags & LOT->BlockMask)
		return true;

	int height = g_Level.Boxes[floor->Box].height;
	if ((boxHeight - height) > LOT->Step || (boxHeight - height) < LOT->Drop)
		return true;

	if (boxHeight - height < -LOT->Step && height > nextHeight)
		return true;

	if (LOT->Fly != NO_FLYING && y > (height + LOT->Fly))
		return true;

	return false;
}

int CreatureCreature(short itemNumber)  
{
	auto* item = &g_Level.Items[itemNumber];
	auto* object = &Objects[item->ObjectNumber];
	int x = item->Position.xPos;
	int z = item->Position.zPos;
	short radius = object->radius;

	auto* room = &g_Level.Rooms[item->RoomNumber];
	short link = room->itemNumber;
	int distance = 0;
	do
	{
		auto* linked = &g_Level.Items[link];
		
		if (link != itemNumber && linked != LaraItem && linked->Status == ITEM_ACTIVE && linked->HitPoints > 0)
		{
			int xDistance = abs(linked->Position.xPos - x);
			int zDistance = abs(linked->Position.zPos - z);
			
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

	auto* creature = GetCreatureInfo(item);
	int* zone = g_Level.Zones[creature->LOT.Zone][FlipStatus].data();
	if (creature->LOT.Fly == NO_FLYING && zone[boxNumber] != zoneNumber)
		return false;

	auto* box = &g_Level.Boxes[boxNumber];
	if (box->flags & creature->LOT.BlockMask)
		return false;

	if (item->Position.zPos > (box->left * SECTOR(1)) &&
		item->Position.zPos < (box->right * SECTOR(1)) &&
		item->Position.xPos > (box->top * SECTOR(1)) &&
		item->Position.xPos < (box->bottom * SECTOR(1)))
	{
		return false;
	}

	return true;
}

int EscapeBox(ITEM_INFO* item, ITEM_INFO* enemy, int boxNumber) 
{
	if (boxNumber == NO_BOX)
		return false;

	auto* box = &g_Level.Boxes[boxNumber];

	int x = (box->top + box->bottom) * SECTOR(1) / 2 - enemy->Position.xPos;
	int z = (box->left + box->right) * SECTOR(1) / 2 - enemy->Position.zPos;
	
	if (x > -ESCAPE_DIST && x < ESCAPE_DIST && z > -ESCAPE_DIST && z < ESCAPE_DIST)
		return false;

	if (((x > 0) ^ (item->Position.xPos > enemy->Position.xPos)) && ((z > 0) ^ (item->Position.zPos > enemy->Position.zPos)))
		return false;

	return true;
}

void TargetBox(LOTInfo* LOT, int boxNumber)
{
	if (boxNumber == NO_BOX)
		return;

	boxNumber &= NO_BOX;
	auto* box = &g_Level.Boxes[boxNumber];

	LOT->Target.x = ((box->top * SECTOR(1)) + GetRandomControl() * ((box->bottom - box->top) - 1) >> 5) + SECTOR(0.5f);
	LOT->Target.z = ((box->left * SECTOR(1)) + GetRandomControl() * ((box->right - box->left) - 1) >> 5) + SECTOR(0.5f);
	LOT->RequiredBox = boxNumber;

	if (LOT->Fly == NO_FLYING)
		LOT->Target.y = box->height;
	else
		LOT->Target.y = box->height - STEPUP_HEIGHT;
}

int UpdateLOT(LOTInfo* LOT, int depth)
{
	//printf("LOT->head: %d, LOT->tail: %d\n", LOT->head, LOT->tail);

	if (LOT->RequiredBox != NO_BOX && LOT->RequiredBox != LOT->TargetBox)
	{
		LOT->TargetBox = LOT->RequiredBox;

		auto* node = &LOT->Node[LOT->TargetBox];
		if (node->nextExpansion == NO_BOX && LOT->Tail != LOT->TargetBox)
		{
			node->nextExpansion = LOT->Head;

			if (LOT->Head == NO_BOX)
				LOT->Tail = LOT->TargetBox;

			LOT->Head = LOT->TargetBox;
		}

		node->searchNumber = ++LOT->SearchNumber;
		node->exitBox = NO_BOX;
	}

	return SearchLOT(LOT, depth);
}

int SearchLOT(LOTInfo* LOT, int depth)
{
	int* zone = g_Level.Zones[LOT->Zone][FlipStatus].data();
	int searchZone = zone[LOT->Head];

	if (depth <= 0)
		return true;

	for (int i = 0; i < depth; i++)
	{
		if (LOT->Head == NO_BOX)
		{
			LOT->Tail = NO_BOX; 
			return false;
		}

		auto* node = &LOT->Node[LOT->Head];
		auto* box = &g_Level.Boxes[LOT->Head];

		int index = box->overlapIndex;
		bool done = false;
		if (index >= 0)
		{
			do
			{
				int boxNumber = g_Level.Overlaps[index].box;
				int flags = g_Level.Overlaps[index++].flags;

				if (flags & BOX_END_BIT)
					done = true;

				if (LOT->Fly == NO_FLYING && searchZone != zone[boxNumber])
					continue;

				int delta = g_Level.Boxes[boxNumber].height - box->height;
				if ((delta > LOT->Step || delta < LOT->Drop) && (!(flags & BOX_MONKEY) || !LOT->CanMonkey))
					continue;

				if ((flags & BOX_JUMP) && !LOT->CanJump)
					continue;

				auto* expand = &LOT->Node[boxNumber];
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
					{
						continue;
					}

					if (g_Level.Boxes[boxNumber].flags & LOT->BlockMask)
						expand->searchNumber = node->searchNumber | BLOCKED_SEARCH;
					else
					{
						expand->searchNumber = node->searchNumber;
						expand->exitBox = LOT->Head;
					}
				}

				if (expand->nextExpansion == NO_BOX && boxNumber != LOT->Tail)
				{
					LOT->Node[LOT->Tail].nextExpansion = boxNumber;
					LOT->Tail = boxNumber;
				}
			} while (!done);
		}

		LOT->Head = node->nextExpansion;
		node->nextExpansion = NO_BOX;
	}

	return true;
}


#if CREATURE_AI_PRIORITY_OPTIMIZATION
CreatureAIPriority GetCreatureLOTPriority(ITEM_INFO* item)
{
	Vector3 itemPos = Vector3(item->Position.xPos, item->Position.yPos, item->Position.zPos);
	Vector3 cameraPos = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);
	float distance = Vector3::Distance(itemPos, cameraPos);

	distance /= SECTOR(1);
	if (distance <= HIGH_PRIO_RANGE)
		return CreatureAIPriority::High;

	if (distance <= MEDIUM_PRIO_RANGE)
		return CreatureAIPriority::Medium;

	if (distance <= LOW_PRIO_RANGE)
		return CreatureAIPriority::Low;

	return CreatureAIPriority::None;
}
#endif

int CreatureActive(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (!Objects[item->ObjectNumber].intelligent)
		return false; // Object is not a creature

	if (item->Flags & IFLAG_KILLED)
		return false; // Object is already dead

	if (item->Status == ITEM_INVISIBLE || !item->Data.is<CreatureInfo>())
	{
		if (!EnableBaddieAI(itemNumber, 0))
			return false; // AI couldn't be activated

		item->Status = ITEM_ACTIVE;
	}

#ifdef CREATURE_AI_PRIORITY_OPTIMIZATION
	auto* creature = GetCreatureInfo(item);
	creature->Priority = GetCreatureLOTPriority(item);
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

	auto* box = &g_Level.Boxes[boxNumber];

	int xRange = STALK_DIST + ((box->bottom - box->top + 3) * SECTOR(1));
	int zRange = STALK_DIST + ((box->right - box->left + 3) * SECTOR(1));
	int x = (box->top + box->bottom) * SECTOR(1) / 2 - enemy->Position.xPos;
	int z = (box->left + box->right) * SECTOR(1) / 2 - enemy->Position.zPos;
	
	if (x > xRange || x < -xRange || z > zRange || z < -zRange)
		return false;

	int enemyQuad = (enemy->Position.yRot / ANGLE(90.0f)) + 2;
	
	int boxQuad;
	if (z > 0)
		boxQuad = (x > 0) ? 2 : 1;
	else
		boxQuad = (x > 0) ? 3 : 0;

	if (enemyQuad == boxQuad)
		return false;

	int baddieQuad = 0;
	if (item->Position.zPos > enemy->Position.zPos)
		baddieQuad = (item->Position.xPos > enemy->Position.xPos) ? 2 : 1;
	else
		baddieQuad = (item->Position.xPos > enemy->Position.xPos) ? 3 : 0;

	if (enemyQuad == baddieQuad && abs(enemyQuad - boxQuad) == 2)
		return false;

	return true;
}

int CreatureVault(short itemNumber, short angle, int vault, int shift)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	int xBlock = item->Position.xPos / SECTOR(1);
	int zBlock = item->Position.zPos / SECTOR(1);
	int y = item->Position.yPos;
	short roomNumber = item->RoomNumber;

	CreatureAnimation(itemNumber, angle, 0);

	if (item->Floor > y + CHECK_CLICK(9))
		vault = 0;
	else if (item->Floor > y + CHECK_CLICK(7))
		vault = -4;
	// FIXME: edit assets adding climb down animations for Von Croy and baddies?
	else if (item->Floor > y + CHECK_CLICK(5) &&
		item->ObjectNumber != ID_VON_CROY &&
		item->ObjectNumber != ID_GOON1 &&
		item->ObjectNumber != ID_GOON2)
	{
		vault = -3;
	}
	else if (item->Floor > y + CHECK_CLICK(3) &&
		item->ObjectNumber != ID_VON_CROY &&
		item->ObjectNumber != ID_GOON1 &&
		item->ObjectNumber != ID_GOON2)
	{
		vault = -2;
	}
	else if (item->Position.yPos > y - CHECK_CLICK(3))
		return 0;
	else if (item->Position.yPos > y - CHECK_CLICK(5))
		vault = 2;
	else if (item->Position.yPos > y - CHECK_CLICK(7))
		vault = 3;
	else
		vault = 4;

	// Jump
	int newXblock = item->Position.xPos / SECTOR(1);
	int newZblock = item->Position.zPos / SECTOR(1);

	if (zBlock == newZblock)
	{
		if (xBlock == newXblock)
			return 0;

		if (xBlock < newXblock)
		{
			item->Position.xPos = (newXblock * SECTOR(1)) - shift;
			item->Position.yRot = ANGLE(90.0f);
		}
		else
		{
			item->Position.xPos = (xBlock * SECTOR(1)) + shift;
			item->Position.yRot = -ANGLE(90.0f);
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
			item->Position.yRot = -ANGLE(180.0f);
		}
	}

	item->Position.yPos = y;
	item->Floor = y;

	if (roomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	return vault;
}

void GetAITarget(CreatureInfo* creature)
{
	auto* enemy = creature->Enemy;

	short enemyObjectNumber;
	if (enemy)
		enemyObjectNumber = enemy->ObjectNumber;
	else
		enemyObjectNumber = NO_ITEM;

	auto* item = &g_Level.Items[creature->ItemNumber];

	if (item->AIBits & GUARD)
	{
		creature->Enemy = LaraItem;
		if (creature->Alerted)
		{
			item->AIBits = ~GUARD;
			if (item->AIBits & AMBUSH)
				item->AIBits |= MODIFY;
		}
	}
	else if (item->AIBits & PATROL1)
	{
		if (creature->Alerted || creature->HurtByLara)
		{
			item->AIBits &= ~PATROL1;
			if (item->AIBits & AMBUSH)
			{
				item->AIBits |= MODIFY;
				// NOTE: added in TR5
				//item->itemFlags[3] = (creature->Tosspad & 0xFF);
			}
		}
		else if (!creature->Patrol)
		{
			if (enemyObjectNumber != ID_AI_PATROL1)
				FindAITargetObject(creature, ID_AI_PATROL1);
		}
		else if (enemyObjectNumber != ID_AI_PATROL2)
			FindAITargetObject(creature, ID_AI_PATROL2);
		else if (abs(enemy->Position.xPos - item->Position.xPos) < REACHED_GOAL_RADIUS &&
			abs(enemy->Position.yPos - item->Position.yPos) < REACHED_GOAL_RADIUS &&
			abs(enemy->Position.zPos - item->Position.zPos) < REACHED_GOAL_RADIUS ||
			Objects[item->ObjectNumber].waterCreature)
		{
			TestTriggers(enemy, true);
			creature->Patrol = !creature->Patrol;
		}
	}
	else if (item->AIBits & AMBUSH)
	{
		// First if was removed probably after TR3 and was it used by monkeys?
		/*if (!(item->aiBits & MODIFY) && !creature->hurtByLara)
			creature->enemy = LaraItem;
		else*/ if (enemyObjectNumber != ID_AI_AMBUSH)
			FindAITargetObject(creature, ID_AI_AMBUSH);
		/*else if (item->objectNumber == ID_MONKEY)
			return;*/
		else if (abs(enemy->Position.xPos - item->Position.xPos) < REACHED_GOAL_RADIUS &&
			abs(enemy->Position.yPos - item->Position.yPos) < REACHED_GOAL_RADIUS &&
			abs(enemy->Position.zPos - item->Position.zPos) < REACHED_GOAL_RADIUS)
		{
			TestTriggers(enemy, true);

			creature->ReachedGoal = true;
			creature->Enemy = LaraItem;
			item->AIBits &= ~(AMBUSH /* | MODIFY*/);
			if (item->AIBits != MODIFY)
			{
				item->AIBits |= GUARD;
				creature->Alerted = false;
			}
		}
	}
	else if (item->AIBits & FOLLOW)
	{
		if (creature->HurtByLara)
		{
			creature->Enemy = LaraItem;
			creature->Alerted = true;
			//item->aiBits &= ~FOLLOW;
		}
		else if (item->HitStatus)
			item->AIBits &= ~FOLLOW;
		else if (enemyObjectNumber != ID_AI_FOLLOW)
			FindAITargetObject(creature, ID_AI_FOLLOW);
		else if (abs(enemy->Position.xPos - item->Position.xPos) < REACHED_GOAL_RADIUS &&
			abs(enemy->Position.yPos - item->Position.yPos) < REACHED_GOAL_RADIUS &&
			abs(enemy->Position.zPos - item->Position.zPos) < REACHED_GOAL_RADIUS)
		{
			creature->ReachedGoal = true;
			item->AIBits &= ~FOLLOW;
		}
	}
	/*else if (item->objectNumber == ID_MONKEY && item->carriedItem == NO_ITEM)
	{
		if (item->aiBits != MODIFY)
		{
			if (enemyObjectNumber != ID_SMALLMEDI_ITEM)
				FindAITargetObject(creature, ID_SMALLMEDI_ITEM);
		}
		else
		{
			if (enemyObjectNumber != ID_KEY_ITEM4)
				FindAITargetObject(creature, ID_KEY_ITEM4);
		}
	}*/
}

// TR3 old way..
void FindAITarget(CreatureInfo* creature, short objectNumber)
{
	auto* item = &g_Level.Items[creature->ItemNumber];
	ITEM_INFO* targetItem;

	int i;
	for (i = 0, targetItem = &g_Level.Items[0]; i < g_Level.NumItems; i++, targetItem++)
	{
		if (targetItem->ObjectNumber == objectNumber && targetItem->RoomNumber != NO_ROOM)
		{
			if (SameZone(creature, targetItem) && targetItem->Position.yRot == item->ItemFlags[3])
			{
				creature->Enemy = targetItem;
				break;
			}
		}
	}
}

void FindAITargetObject(CreatureInfo* creature, short objectNumber)
{
	auto* item = &g_Level.Items[creature->ItemNumber];

	if (g_Level.AIObjects.size() > 0)
	{
		AI_OBJECT* foundObject = NULL;

		for (int i = 0; i < g_Level.AIObjects.size(); i++)
		{
			auto* aiObject = &g_Level.AIObjects[i];

			if (aiObject->objectNumber == objectNumber && aiObject->triggerFlags == item->ItemFlags[3] && aiObject->roomNumber != NO_ROOM)
			{
				int* zone = g_Level.Zones[creature->LOT.Zone][FlipStatus].data();

				auto* room = &g_Level.Rooms[item->RoomNumber];
				item->BoxNumber = GetSector(room, item->Position.xPos - room->x, item->Position.zPos - room->z)->Box;

				room = &g_Level.Rooms[aiObject->roomNumber];
				aiObject->boxNumber = GetSector(room, aiObject->x - room->x, aiObject->z - room->z)->Box;

				if (item->BoxNumber == NO_BOX || aiObject->boxNumber == NO_BOX)
					return;

				if (zone[item->BoxNumber] == zone[aiObject->boxNumber])
				{
					foundObject = aiObject;
					break;
				}
			}
		}

		if (foundObject != NULL)
		{
			auto* aiItem = creature->AITarget;

			creature->Enemy = aiItem;

			aiItem->ObjectNumber = foundObject->objectNumber;
			aiItem->RoomNumber = foundObject->roomNumber;
			aiItem->Position.xPos = foundObject->x;
			aiItem->Position.yPos = foundObject->y;
			aiItem->Position.zPos = foundObject->z;
			aiItem->Position.yRot = foundObject->yRot;
			aiItem->Flags = foundObject->flags;
			aiItem->TriggerFlags = foundObject->triggerFlags;
			aiItem->BoxNumber = foundObject->boxNumber;

			if (!(creature->AITarget->Flags & 32))
			{
				creature->AITarget->Position.xPos += phd_sin(creature->AITarget->Position.yRot) * 256;
				creature->AITarget->Position.zPos += phd_cos(creature->AITarget->Position.yRot) * 256;
			}
		}
	}
}

void CreatureAIInfo(ITEM_INFO* item, AI_INFO* info)
{
	if (!item->Data)
		return;

	auto* creature = GetCreatureInfo(item);
	auto* object = &Objects[item->ObjectNumber];

	auto* enemy = creature->Enemy;
	if (!enemy)
	{
		enemy = LaraItem;
		creature->Enemy = LaraItem;
	}

	int* zone = g_Level.Zones[creature->LOT.Zone][FlipStatus].data();

	auto* room = &g_Level.Rooms[item->RoomNumber];
	item->BoxNumber = NO_BOX;
	FLOOR_INFO* floor = GetSector(room, item->Position.xPos - room->x, item->Position.zPos - room->z);
	if(floor)
		item->BoxNumber = floor->Box;

	if (item->BoxNumber != NO_BOX)
		info->zoneNumber = zone[item->BoxNumber];
	else
		info->zoneNumber = NO_ZONE;

	room = &g_Level.Rooms[enemy->RoomNumber];
	enemy->BoxNumber = NO_BOX;
	floor = GetSector(room, enemy->Position.xPos - room->x, enemy->Position.zPos - room->z);
	if(floor)
		enemy->BoxNumber = floor->Box;

	if (enemy->BoxNumber != NO_BOX)
		info->enemyZone = zone[enemy->BoxNumber];
	else
		info->enemyZone = NO_ZONE;

	if (!object->nonLot)
	{
		if (enemy->BoxNumber != NO_BOX && g_Level.Boxes[enemy->BoxNumber].flags & creature->LOT.BlockMask)
			info->enemyZone |= BLOCKED;
		else if (item->BoxNumber != NO_BOX && creature->LOT.Node[item->BoxNumber].searchNumber == (creature->LOT.SearchNumber | BLOCKED_SEARCH))
			info->enemyZone |= BLOCKED;
	}

	int x, y, z;

	if (enemy == LaraItem)
	{
		x = enemy->Position.xPos + enemy->Animation.Velocity * PREDICTIVE_SCALE_FACTOR * phd_sin(Lara.Control.MoveAngle) - item->Position.xPos - object->pivotLength * phd_sin(item->Position.yRot);
		z = enemy->Position.zPos + enemy->Animation.Velocity * PREDICTIVE_SCALE_FACTOR * phd_cos(Lara.Control.MoveAngle) - item->Position.zPos - object->pivotLength * phd_cos(item->Position.yRot);
	}
	else
	{
		x = enemy->Position.xPos + enemy->Animation.Velocity * PREDICTIVE_SCALE_FACTOR * phd_sin(enemy->Position.yRot) - item->Position.xPos - object->pivotLength * phd_sin(item->Position.yRot);
		z = enemy->Position.zPos + enemy->Animation.Velocity * PREDICTIVE_SCALE_FACTOR * phd_cos(enemy->Position.yRot) - item->Position.zPos - object->pivotLength * phd_cos(item->Position.yRot);
	}

	y = item->Position.yPos - enemy->Position.yPos;
	short angle = phd_atan(z, x);

	if (x > 32000 || x < -32000 || z > 32000 || z < -32000)
		info->distance = 0x7FFFFFFF;
	else
	{
		if (creature->Enemy)
			info->distance = pow(x, 2) + pow(z, 2);
		else
			info->distance = 0x7FFFFFFF;
	}

	info->angle = angle - item->Position.yRot;
	info->enemyFacing = 0x8000 + angle - enemy->Position.yRot;

	x = abs(x);
	z = abs(z);

	// Makes Lara smaller.
	if (enemy == LaraItem && ((LaraInfo*)enemy)->Control.IsLow)
		y -= STEPUP_HEIGHT;

	if (x > z)
		info->xAngle = phd_atan(x + (z >> 1), y);
	else
		info->xAngle = phd_atan(z + (x >> 1), y);

	info->ahead = (info->angle > -FRONT_ARC && info->angle < FRONT_ARC);
	info->bite = (info->ahead && enemy->HitPoints > 0 && abs(enemy->Position.yPos - item->Position.yPos) <= CLICK(2));
}

void CreatureMood(ITEM_INFO* item, AI_INFO* AI, int violent)
{
	if (!item->Data)
		return;

	auto* creature = GetCreatureInfo(item);
	auto* enemy = creature->Enemy;
	auto* LOT = &creature->LOT;

	int boxNumber;

	if (enemy != nullptr)
	{
		switch (creature->Mood)
		{
		case MoodType::Bored:
			boxNumber = LOT->Node[GetRandomControl() * LOT->ZoneCount >> 15].boxNumber;
			if (ValidBox(item, AI->zoneNumber, boxNumber) &&
				!(GetRandomControl() & 0x0F))
			{
				if (StalkBox(item, enemy, boxNumber) && enemy->HitPoints > 0 && creature->Enemy)
				{
					TargetBox(LOT, boxNumber);
					creature->Mood = MoodType::Bored;
				}
				else if (LOT->RequiredBox == NO_BOX)
					TargetBox(LOT, boxNumber);
			}

			break;

		case MoodType::Attack:
			LOT->Target.x = enemy->Position.xPos;
			LOT->Target.y = enemy->Position.yPos;
			LOT->Target.z = enemy->Position.zPos;
			LOT->RequiredBox = enemy->BoxNumber;

			if (LOT->Fly != NO_FLYING && Lara.Control.WaterStatus == WaterStatus::Dry)
			{
				auto* bounds = (BOUNDING_BOX*)GetBestFrame(enemy);
				LOT->Target.y += bounds->Y1;
			}

			break;

		case MoodType::Escape:
			boxNumber = LOT->Node[GetRandomControl() * LOT->ZoneCount >> 15].boxNumber;
			if (ValidBox(item, AI->zoneNumber, boxNumber) && LOT->RequiredBox == NO_BOX)
			{
				if (EscapeBox(item, enemy, boxNumber))
					TargetBox(LOT, boxNumber);
				else if (AI->zoneNumber == AI->enemyZone && StalkBox(item, enemy, boxNumber) && !violent)
				{
					TargetBox(LOT, boxNumber);
					creature->Mood = MoodType::Stalk;
				}
			}

			break;

		case MoodType::Stalk:
			if (LOT->RequiredBox == NO_BOX || !StalkBox(item, enemy, LOT->RequiredBox))
			{
				boxNumber = LOT->Node[GetRandomControl() * LOT->ZoneCount >> 15].boxNumber;
				if (ValidBox(item, AI->zoneNumber, boxNumber))
				{
					if (StalkBox(item, enemy, boxNumber))
						TargetBox(LOT, boxNumber);
					else if (LOT->RequiredBox == NO_BOX)
					{
						TargetBox(LOT, boxNumber);
						if (AI->zoneNumber != AI->enemyZone)
							creature->Mood = MoodType::Bored;
					}
				}
			}

			break;
		}
	}

	if (LOT->TargetBox == NO_BOX)
		TargetBox(LOT, item->BoxNumber);

#ifdef CREATURE_AI_PRIORITY_OPTIMIZATION
	bool shouldUpdateTarget = false;

	switch(creature->Priority)
	{
		case CreatureAIPriority::High:
			shouldUpdateTarget = true;
			break;

		case CreatureAIPriority::Medium:
			if (creature->FramesSinceLOTUpdate > std::pow(FRAME_PRIO_BASE, FRAME_PRIO_EXP))
				shouldUpdateTarget = true;

			break;

		case CreatureAIPriority::Low:
			if (creature->FramesSinceLOTUpdate > std::pow(FRAME_PRIO_BASE, FRAME_PRIO_EXP * 2))
				shouldUpdateTarget = true;

			break;

		default:
			break;
	}

	if (shouldUpdateTarget)
	{
		CalculateTarget(&creature->Target, item, &creature->LOT);
		creature->FramesSinceLOTUpdate = 0;
	}
	else
		creature->FramesSinceLOTUpdate++;
#else
	CalculateTarget(&creature->Target, item, &creature->LOT);
#endif // CREATURE_AI_PRIORITY_OPTIMIZATION

	creature->JumpAhead = false;
	creature->MonkeySwingAhead = false;

	if (item->BoxNumber != NO_BOX)
	{
		int startBox = LOT->Node[item->BoxNumber].exitBox;
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
					creature->JumpAhead = true;

				if (flags & BOX_MONKEY)
					creature->MonkeySwingAhead = true;
			}
		}
	}
}

void GetCreatureMood(ITEM_INFO* item, AI_INFO* AI, int isViolent)
{
	if (!item->Data)
		return;

	auto* creature = GetCreatureInfo(item);
	auto* enemy = creature->Enemy;
	auto* LOT = &creature->LOT;

	if (item->BoxNumber == NO_BOX || creature->LOT.Node[item->BoxNumber].searchNumber == (creature->LOT.SearchNumber | BLOCKED_SEARCH))
		creature->LOT.RequiredBox = NO_BOX;

	if (creature->Mood != MoodType::Attack &&
		creature->LOT.RequiredBox != NO_BOX)
	{
		if (!ValidBox(item, AI->zoneNumber, creature->LOT.TargetBox))
		{
			if (AI->zoneNumber == AI->enemyZone)
				creature->Mood = MoodType::Bored;

			creature->LOT.RequiredBox = NO_BOX;
		}
	}

	auto mood = creature->Mood;
	if (!enemy)
	{
		creature->Mood = MoodType::Bored;
		enemy = LaraItem;
	}
	else if (enemy->HitPoints <= 0 && enemy == LaraItem)
		creature->Mood = MoodType::Bored;
	else if (isViolent)
	{
		switch (creature->Mood)
		{
			case MoodType::Bored:
			case MoodType::Stalk:
				if (AI->zoneNumber == AI->enemyZone)
					creature->Mood = MoodType::Attack;
				else if (item->HitStatus)
					creature->Mood = MoodType::Escape;

				break;

			case MoodType::Attack:
				if (AI->zoneNumber != AI->enemyZone)
					creature->Mood = MoodType::Bored;

				break;

			case MoodType::Escape:
				if (AI->zoneNumber == AI->enemyZone)
					creature->Mood = MoodType::Attack;

				break;
		}
	}
	else
	{
		switch (creature->Mood)
		{
			case MoodType::Bored:
			case MoodType::Stalk:
				if (creature->Alerted &&
					AI->zoneNumber != AI->enemyZone)
				{
					if (AI->distance > SECTOR(3))
						creature->Mood = MoodType::Stalk;
					else
						creature->Mood = MoodType::Bored;
				}
				else if (AI->zoneNumber == AI->enemyZone)
				{
					if (AI->distance < ATTACK_RANGE ||
						(creature->Mood == MoodType::Stalk &&
							LOT->RequiredBox == NO_BOX))
						creature->Mood = MoodType::Attack;
					else
						creature->Mood = MoodType::Stalk;
				}

				break;

			case MoodType::Attack:
				if (item->HitStatus &&
					(GetRandomControl() < ESCAPE_CHANCE ||
						AI->zoneNumber != AI->enemyZone))
					creature->Mood = MoodType::Stalk;
				else if (AI->zoneNumber != AI->enemyZone && AI->distance > SECTOR(6))
					creature->Mood = MoodType::Bored;

				break;

			case MoodType::Escape:
				if (AI->zoneNumber == AI->enemyZone &&
					GetRandomControl() < RECOVER_CHANCE)
					creature->Mood = MoodType::Stalk;

				break;
		}
	}

	if (mood != creature->Mood)
	{
		if (mood == MoodType::Attack)
			TargetBox(LOT, LOT->TargetBox);

		LOT->RequiredBox = NO_BOX;
	}
}

TARGET_TYPE CalculateTarget(PHD_VECTOR* target, ITEM_INFO* item, LOTInfo* LOT)
{
	UpdateLOT(LOT, 5);

	target->x = item->Position.xPos;
	target->y = item->Position.yPos;
	target->z = item->Position.zPos;

	int boxNumber = item->BoxNumber;
	if (boxNumber == NO_BOX)
		return TARGET_TYPE::NO_TARGET;

	auto* box = &g_Level.Boxes[boxNumber];
	int boxLeft = ((int)box->left * SECTOR(1));
	int boxRight = ((int)box->right * SECTOR(1)) - 1;
	int boxTop = ((int)box->top * SECTOR(1));
	int boxBottom = ((int)box->bottom * SECTOR(1)) - 1;
	int left = boxLeft;
	int right = boxRight;
	int top = boxTop;
	int bottom = boxBottom;
	int direction = ALL_CLIP;

	do
	{
		box = &g_Level.Boxes[boxNumber];

		if (LOT->Fly == NO_FLYING)
		{
			if (target->y > box->height)
				target->y = box->height;
		}
		else
		{
			if (target->y > box->height - SECTOR(1))
				target->y = box->height - SECTOR(1);
		}

		boxLeft = ((int)box->left * SECTOR(1));
		boxRight = ((int)box->right * SECTOR(1)) - 1;
		boxTop = ((int)box->top * SECTOR(1));
		boxBottom = ((int)box->bottom * SECTOR(1)) - 1;

		if (item->Position.zPos >= boxLeft &&
			item->Position.zPos <= boxRight &&
			item->Position.xPos >= boxTop &&
			item->Position.xPos <= boxBottom)
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
				if (direction & CLIP_LEFT &&
					item->Position.xPos >= boxTop &&
					item->Position.xPos <= boxBottom)
				{
					if (target->z < (boxLeft + CLICK(2)))
						target->z = boxLeft + CLICK(2);

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
					target->z = (right - CLICK(2));
					if (direction != ALL_CLIP)
						return TARGET_TYPE::SECONDARY_TARGET;

					direction |= SECONDARY_CLIP;
				}
			}
			else if (item->Position.zPos > boxRight)
			{
				if (direction & CLIP_RIGHT &&
					item->Position.xPos >= boxTop &&
					item->Position.xPos <= boxBottom)
				{
					if (target->z > boxRight - CLICK(2))
						target->z = boxRight - CLICK(2);

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
					target->z = left + CLICK(2);
					if (direction != ALL_CLIP)
						return TARGET_TYPE::SECONDARY_TARGET;

					direction |= SECONDARY_CLIP;
				}
			}

			if (item->Position.xPos < boxTop)
			{
				if (direction & CLIP_TOP &&
					item->Position.zPos >= boxLeft &&
					item->Position.zPos <= boxRight)
				{
					if (target->x < boxTop + CLICK(2))
						target->x = boxTop + CLICK(2);

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
					target->x = bottom - CLICK(2);
					if (direction != ALL_CLIP)
						return TARGET_TYPE::SECONDARY_TARGET;

					direction |= SECONDARY_CLIP;
				}
			}
			else if (item->Position.xPos > boxBottom)
			{
				if (direction & CLIP_BOTTOM &&
					item->Position.zPos >= boxLeft &&
					item->Position.zPos <= boxRight)
				{
					if (target->x > (boxBottom - CLICK(2)))
						target->x = boxBottom - CLICK(2);

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
					target->x = top + CLICK(2);
					if (direction != ALL_CLIP)
						return TARGET_TYPE::SECONDARY_TARGET;

					direction |= SECONDARY_CLIP;
				}
			}
		}

		if (boxNumber == LOT->TargetBox)
		{
			if (direction & (CLIP_LEFT | CLIP_RIGHT))
				target->z = LOT->Target.z;
			else if (!(direction & SECONDARY_CLIP))
			{
				if (target->z < (boxLeft + CLICK(2)))
					target->z = boxLeft + CLICK(2);
				else if (target->z > (boxRight - CLICK(2)))
					target->z = boxRight - CLICK(2);
			}

			if (direction & (CLIP_TOP | CLIP_BOTTOM))
				target->x = LOT->Target.x;
			else if (!(direction & SECONDARY_CLIP))
			{
				if (target->x < (boxTop + CLICK(2)))
					target->x = boxTop + CLICK(2);
				else if (target->x > (boxBottom - CLICK(2)))
					target->x = boxBottom - CLICK(2);
			}

			target->y = LOT->Target.y;
			return TARGET_TYPE::PRIME_TARGET;
		}

		boxNumber = LOT->Node[boxNumber].exitBox;
		if (boxNumber != NO_BOX && (g_Level.Boxes[boxNumber].flags & LOT->BlockMask))
			break;
	} while (boxNumber != NO_BOX);

	if (direction & (CLIP_LEFT | CLIP_RIGHT))
		target->z = boxLeft + SECTOR(0.5f) + (GetRandomControl() * (boxRight - boxLeft - SECTOR(1)) >> 15);
	else if (!(direction & SECONDARY_CLIP))
	{
		if (target->z < (boxLeft + CLICK(2)))
			target->z = boxLeft + CLICK(2);
		else if (target->z > (boxRight - CLICK(2)))
			target->z = boxRight - CLICK(2);
	}

	if (direction & (CLIP_TOP | CLIP_BOTTOM))
		target->x = boxTop + SECTOR(0.5f) + (GetRandomControl() * (boxBottom - boxTop - SECTOR(1)) >> 15);
	else if (!(direction & SECONDARY_CLIP))
	{
		if (target->x < (boxTop + CLICK(2)))
			target->x = boxTop + CLICK(2);
		else if (target->x > (boxBottom - CLICK(2)))
			target->x = boxBottom - CLICK(2);
	}

	if (LOT->Fly == NO_FLYING)
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

	x = item->Position.xPos + SECTOR(1) * phd_sin(direction);
	z = item->Position.zPos + SECTOR(1) * phd_cos(direction);

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
		auto* currentItem = &g_Level.Items[i];

		if (currentItem->Active && currentItem->Data.is<PushableInfo>())
			ClearMovableBlockSplitters(currentItem->Position.xPos, currentItem->Position.yPos, currentItem->Position.zPos, currentItem->RoomNumber);
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
