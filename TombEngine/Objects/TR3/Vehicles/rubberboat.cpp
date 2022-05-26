#include "framework.h"
#include "Objects/TR3/Vehicles/rubberboat.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/effects/bubble.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/TR3/Vehicles/rubberboat_info.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#define RBOAT_SLIP		10
#define RBOAT_SIDE_SLIP	30

#define RBOAT_FRONT		750
#define RBOAT_SIDE		300
#define RBOAT_RADIUS	500
#define RBOAT_SNOW		500

#define RBOAT_MAX_VELOCITY	110
#define RBOAT_SLOW_VELOCITY	(RBOAT_MAX_VELOCITY / 3)
#define RBOAT_FAST_VELOCITY	(RBOAT_MAX_VELOCITY + 75)
#define RBOAT_MIN_VELOCITY	20
#define RBOAT_MAX_BACK		-20
#define RBOAT_MAX_KICK		-80

#define RBOAT_ACCELERATION	5
#define RBOAT_BRAKE			5
#define RBOAT_SLOW_DOWN		1
#define RBOAT_UNDO_TURN		ANGLE(0.25f)
#define RBOAT_TURN			(ANGLE(0.25f) / 2)

#define RBOAT_IN_SPEED		(IN_SPRINT | IN_CROUCH)
#define RBOAT_IN_SLOW		IN_WALK
#define RBOAT_IN_DISMOUNT	IN_ROLL
#define RBOAT_IN_FORWARD	IN_ACTION
#define RBOAT_IN_BACK		IN_JUMP
#define RBOAT_IN_LEFT		IN_LEFT
#define RBOAT_IN_RIGHT		IN_RIGHT

enum RubberBoatState
{
	RBOAT_STATE_MOUNT = 0,
	RBOAT_STATE_IDLE = 1,
	RBOAT_STATE_MOVING = 2,
	RBOAT_STATE_JUMP_RIGHT = 3,
	RBOAT_STATE_JUMP_LEFT = 4,
	RBOAT_STATE_HIT = 5,
	RBOAT_STATE_FALL = 6,
	RBOAT_STATE_TURN_RIGHT = 7,
	RBOAT_STATE_DEATH = 8,
	RBOAT_STATE_TURN_LEFT = 9
};

enum RubberBoatAnim
{
	RBOAT_ANIM_MOUNT_LEFT = 0,
	RBOAT_ANIM_IDLE = 1,
	RBOAT_ANIM_UNK_1 = 2,
	RBOAT_ANIM_UNK_2 = 3,
	RBOAT_ANIM_UNK_3 = 4,
	RBOAT_ANIM_DISMOUNT_LEFT = 5,
	RBOAT_ANIM_MOUNT_JUMP = 6,
	RBOAT_ANIM_DISMOUNT_RIGHT = 7,
	RBOAT_ANIM_MOUNT_RIGHT = 8,
	RBOAT_ANIM_TURN_LEFT_CONTINUE = 9,
	RBOAT_ANIM_TURN_LEFT_END = 10,
	RBOAT_ANIM_HIT_RIGHT = 11,
	RBOAT_ANIM_HIT_LEFT = 12,
	RBOAT_ANIM_HIT_FRONT = 13,
	RBOAT_ANIM_HIT_BACK = 14,
	RBOAT_ANIM_LEAP_START = 15,
	RBOAT_ANIM_LEAP_CONTINUE = 16,
	RBOAT_ANIM_LEAP_END = 17,
	RBOAT_ANIM_IDLE_DEATH = 18,
	RBOAT_ANIM_TURN_RIGHT_CONTINUE = 19,
	RBOAT_ANIM_TURN_RIGHT_END = 20,
	RBOAT_ANIM_TURN_LEFT_START = 21,
	RBOAT_ANIM_TURN_RIGHT_START = 22
};

enum RubberBoatMountType
{
	RBOAT_MOUNT_NONE = 0,
	RBOAT_MOUNT_LEFT = 1,
	RBOAT_MOUNT_RIGHT = 2,
	RBOAT_MOUNT_JUMP = 3,
	RBOAT_MOUNT_LEVEL_START = 4
};

void InitialiseRubberBoat(short itemNumber)
{
	auto* rBoatItem = &g_Level.Items[itemNumber];
	rBoatItem->Data = RubberBoatInfo();
	auto* rBoat = (RubberBoatInfo*)rBoatItem->Data;

	rBoat->TurnRate = 0;
	rBoat->LeanAngle = 0;
	rBoat->ExtraRotation = 0;
	rBoat->LeftVerticalVelocity = 0;
	rBoat->RightVerticalVelocity = 0;
	rBoat->Water = 0;
	rBoat->Pitch = 0;
}

void DrawRubberBoat(ItemInfo* rBoatItem)
{
	/* TODO: WTF?
	RUBBER_BOAT_INFO *b;

	b = item->data;
	item->data = &b->propRot;
	DrawAnimatingItem(item);
	item->data = b;
	*/
}

RubberBoatMountType GetRubberBoatMountType(ItemInfo* laraItem, short itemNumber, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* rBoat = &g_Level.Items[itemNumber];

	RubberBoatMountType mountType = RBOAT_MOUNT_NONE;

	if (!(TrInput & IN_ACTION) ||
		lara->Control.HandStatus != HandStatus::Free ||
		laraItem->Animation.Airborne ||
		rBoat->Animation.Velocity)
	{
		return mountType;
	}

	short x = laraItem->Pose.Position.x - rBoat->Pose.Position.x;
	short z = laraItem->Pose.Position.z - rBoat->Pose.Position.z;

	int distance = z * phd_cos(-rBoat->Pose.Orientation.y) - x * phd_sin(-rBoat->Pose.Orientation.y);
	if (distance > CLICK(2))
		return mountType;

	short deltaAngle = rBoat->Pose.Orientation.y - laraItem->Pose.Orientation.y;
	if (lara->Control.WaterStatus == WaterStatus::TreadWater || lara->Control.WaterStatus == WaterStatus::Wade)
	{
		if (deltaAngle > ANGLE(45.0f) && deltaAngle < ANGLE(135.0f))
			mountType = RBOAT_MOUNT_LEFT;
		if (deltaAngle < -ANGLE(135.0f) && deltaAngle > -ANGLE(45.0f))
			mountType = RBOAT_MOUNT_RIGHT;
	}
	else if (lara->Control.WaterStatus == WaterStatus::Dry)
	{
		if (laraItem->Animation.VerticalVelocity > 0)
		{
			if ((laraItem->Pose.Position.y + CLICK(2)) > rBoat->Pose.Position.y)
				mountType = RBOAT_MOUNT_JUMP;
		}
		else if (laraItem->Animation.VerticalVelocity == 0)
		{
			if (deltaAngle > -ANGLE(135.0f) && deltaAngle < ANGLE(135.0f))
			{
				if (laraItem->Pose.Position.x == rBoat->Pose.Position.x &&
					laraItem->Pose.Position.y == rBoat->Pose.Position.x &&
					laraItem->Pose.Position.z == rBoat->Pose.Position.z)
				{
					mountType = RBOAT_MOUNT_LEVEL_START;
				}
				else
					mountType = RBOAT_MOUNT_JUMP;
			}
		}
	}

	if (!mountType)
		return RBOAT_MOUNT_NONE;

	if (!TestBoundsCollide(rBoat, laraItem, coll->Setup.Radius))
		return RBOAT_MOUNT_NONE;

	if (!TestCollision(rBoat, laraItem))
		return RBOAT_MOUNT_NONE;

	return mountType;
}

int TestWaterHeight(ItemInfo* rBoatItem, int zOffset, int xOffset, Vector3Int* pos)
{
	float s = phd_sin(rBoatItem->Pose.Orientation.y);
	float c = phd_cos(rBoatItem->Pose.Orientation.y);

	pos->x = rBoatItem->Pose.Position.x + zOffset * s + xOffset * c;
	pos->y = rBoatItem->Pose.Position.y - zOffset * phd_sin(rBoatItem->Pose.Orientation.x) + xOffset * phd_sin(rBoatItem->Pose.Orientation.z);
	pos->z = rBoatItem->Pose.Position.z + zOffset * c - xOffset * s;

	auto probe = GetCollision(pos->x, pos->y, pos->z, rBoatItem->RoomNumber);
	auto height = GetWaterHeight(pos->x, pos->y, pos->z, probe.RoomNumber);

	if (height == NO_HEIGHT)
	{
		height = probe.Position.Floor;
		if (height == NO_HEIGHT)
			return height;
	}

	return (height - 5);
}

static void DoRubberBoatShift(ItemInfo* laraItem, int itemNumber)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* boatItem = &g_Level.Items[itemNumber];

	for (short currentItemNumber : g_Level.Rooms[boatItem->RoomNumber].Items)
	{
		auto* item = &g_Level.Items[currentItemNumber];

		if (item->ObjectNumber == ID_RUBBER_BOAT && currentItemNumber != itemNumber && lara->Vehicle != currentItemNumber)
		{
			int x = item->Pose.Position.x - boatItem->Pose.Position.x;
			int z = item->Pose.Position.z - boatItem->Pose.Position.z;

			int distance = pow(x, 2) + pow(z, 2);
			if (distance < 1000000)
			{
				boatItem->Pose.Position.x = item->Pose.Position.x - x * 1000000 / distance;
				boatItem->Pose.Position.z = item->Pose.Position.z - z * 1000000 / distance;
			}

			return;
		}
	}
}

static int DoRubberBoatShift2(ItemInfo* rBoatItem, Vector3Int* pos, Vector3Int* old)
{
	int x = pos->x / SECTOR(1);
	int z = pos->z / SECTOR(1);

	int xOld = old->x / SECTOR(1);
	int zOld = old->z / SECTOR(1);

	int xShift = pos->x & (SECTOR(1) - 1);
	int zShift = pos->z & (SECTOR(1) - 1);

	if (x == xOld)
	{
		if (z == zOld)
		{
			rBoatItem->Pose.Position.z += (old->z - pos->z);
			rBoatItem->Pose.Position.x += (old->x - pos->x);
		}
		else if (z > zOld)
		{
			rBoatItem->Pose.Position.z -= zShift + 1;
			return (pos->x - rBoatItem->Pose.Position.x);
		}
		else
		{
			rBoatItem->Pose.Position.z += SECTOR(1) - zShift;
			return (rBoatItem->Pose.Position.x - pos->x);
		}
	}
	else if (z == zOld)
	{
		if (x > xOld)
		{
			rBoatItem->Pose.Position.x -= xShift + 1;
			return (rBoatItem->Pose.Position.z - pos->z);
		}
		else
		{
			rBoatItem->Pose.Position.x += SECTOR(1) - xShift;
			return (pos->z - rBoatItem->Pose.Position.z);
		}
	}
	else
	{
		x = 0;
		z = 0;

		int height = GetCollision(old->x, pos->y, pos->z, rBoatItem->RoomNumber).Position.Floor;
		if (height < (old->y - CLICK(1)))
		{
			if (pos->z > old->z)
				z = -zShift - 1;
			else
				z = SECTOR(1) - zShift;
		}

		height = GetCollision(pos->x, pos->y, old->z, rBoatItem->RoomNumber).Position.Floor;
		if (height < (old->y - CLICK(1)))
		{
			if (pos->x > old->x)
				x = -xShift - 1;
			else
				x = SECTOR(1) - xShift;
		}

		if (x && z)
		{
			rBoatItem->Pose.Position.z += z;
			rBoatItem->Pose.Position.x += x;
		}
		else if (z)
		{
			rBoatItem->Pose.Position.z += z;
			if (z > 0)
				return (rBoatItem->Pose.Position.x - pos->x);
			else
				return (pos->x - rBoatItem->Pose.Position.x);
		}
		else if (x)
		{
			rBoatItem->Pose.Position.x += x;
			if (x > 0)
				return (pos->z - rBoatItem->Pose.Position.z);
			else
				return (rBoatItem->Pose.Position.z - pos->z);
		}
		else
		{
			rBoatItem->Pose.Position.z += (old->z - pos->z);
			rBoatItem->Pose.Position.x += (old->x - pos->x);
		}
	}

	return 0;
}

static int GetRubberBoatCollisionAnim(ItemInfo* rBoatItem, Vector3Int* moved)
{
	moved->x = rBoatItem->Pose.Position.x - moved->x;
	moved->z = rBoatItem->Pose.Position.z - moved->z;

	if (moved->x || moved->z)
	{
		float c = phd_cos(rBoatItem->Pose.Orientation.y);
		float s = phd_sin(rBoatItem->Pose.Orientation.y);
		long front = moved->z * c + moved->x * s;
		long side = -moved->z * s + moved->x * c;

		if (abs(front) > abs(side))
		{
			if (front > 0)
				return RBOAT_ANIM_HIT_BACK;
			else
				return RBOAT_ANIM_HIT_FRONT;
		}
		else
		{
			if (side > 0)
				return RBOAT_ANIM_HIT_RIGHT;
			else
				return RBOAT_ANIM_HIT_LEFT;
		}
	}
	return 0;
}

static int RubberBoatDynamics(ItemInfo* laraItem, short itemNumber)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* rBoatItem = &g_Level.Items[itemNumber];
	auto* rBoat = (RubberBoatInfo*)rBoatItem->Data;

	rBoatItem->Pose.Orientation.z -= rBoat->LeanAngle;

	Vector3Int frontLeftOld, frontRightOld, backLeftOld, backRightOld, frontOld;
	int heightFrontLeftOld = TestWaterHeight(rBoatItem, RBOAT_FRONT, -RBOAT_SIDE, &frontLeftOld);
	int heightFrontRightOld = TestWaterHeight(rBoatItem, RBOAT_FRONT, RBOAT_SIDE, &frontRightOld);
	int heightBackLeftOld = TestWaterHeight(rBoatItem, -RBOAT_FRONT, -RBOAT_SIDE, &backLeftOld);
	int heightBackRightOld = TestWaterHeight(rBoatItem, -RBOAT_FRONT, RBOAT_SIDE, &backRightOld);
	int heightFrontOld = TestWaterHeight(rBoatItem, 1000, 0, &frontOld);
	
	Vector3Int old;
	old.x = rBoatItem->Pose.Position.x;
	old.y = rBoatItem->Pose.Position.y;
	old.z = rBoatItem->Pose.Position.z;

	if (backLeftOld.y > heightBackLeftOld)
		backLeftOld.y = heightBackLeftOld;
	if (backRightOld.y > heightBackRightOld)
		backRightOld.y = heightBackRightOld;
	if (frontLeftOld.y > heightFrontLeftOld)
		frontLeftOld.y = heightFrontLeftOld;
	if (frontRightOld.y > heightFrontRightOld)
		frontRightOld.y = heightFrontRightOld;
	if (frontOld.y > heightFrontOld)
		frontOld.y = heightFrontOld;

	rBoatItem->Pose.Orientation.y += rBoat->TurnRate + rBoat->ExtraRotation;
	rBoat->LeanAngle = rBoat->TurnRate * 6;

	rBoatItem->Pose.Position.z += rBoatItem->Animation.Velocity * phd_cos(rBoatItem->Pose.Orientation.y);
	rBoatItem->Pose.Position.x += rBoatItem->Animation.Velocity * phd_sin(rBoatItem->Pose.Orientation.y);
	if (rBoatItem->Animation.Velocity >= 0)
		rBoat->PropellerRotation += (rBoatItem->Animation.Velocity * ANGLE(3.0f)) + ANGLE(2.0f);
	else
		rBoat->PropellerRotation += ANGLE(33.0f);

	int slip = RBOAT_SIDE_SLIP * phd_sin(rBoatItem->Pose.Orientation.z);
	if (!slip && rBoatItem->Pose.Orientation.z)
		slip = (rBoatItem->Pose.Orientation.z > 0) ? 1 : -1;

	rBoatItem->Pose.Position.z -= slip * phd_sin(rBoatItem->Pose.Orientation.y);
	rBoatItem->Pose.Position.x += slip * phd_cos(rBoatItem->Pose.Orientation.y);

	slip = RBOAT_SLIP * phd_sin(rBoatItem->Pose.Orientation.x);
	if (!slip && rBoatItem->Pose.Orientation.x)
		slip = (rBoatItem->Pose.Orientation.x > 0) ? 1 : -1;

	rBoatItem->Pose.Position.z -= slip * phd_cos(rBoatItem->Pose.Orientation.y);
	rBoatItem->Pose.Position.x -= slip * phd_sin(rBoatItem->Pose.Orientation.y);

	Vector3Int moved;
	moved.x = rBoatItem->Pose.Position.x;
	moved.z = rBoatItem->Pose.Position.z;

	DoRubberBoatShift(laraItem, itemNumber);

	Vector3Int frontLeft, frontRight, backRight, backLeft, front;
	short rotation = 0;

	int heightBackLeft = TestWaterHeight(rBoatItem, -RBOAT_FRONT, -RBOAT_SIDE, &backLeft);
	if (heightBackLeft < (backLeftOld.y - CLICK(0.5f)))
		rotation = DoRubberBoatShift2(rBoatItem, &backLeft, &backLeftOld);

	int heightBackRight = TestWaterHeight(rBoatItem, -RBOAT_FRONT, RBOAT_SIDE, &backRight);
	if (heightBackRight < (backRightOld.y - CLICK(0.5f)))
		rotation += DoRubberBoatShift2(rBoatItem, &backRight, &backRightOld);

	int heightFrontLeft = TestWaterHeight(rBoatItem, RBOAT_FRONT, -RBOAT_SIDE, &frontLeft);
	if (heightFrontLeft < (frontLeftOld.y - CLICK(0.5f)))
		rotation += DoRubberBoatShift2(rBoatItem, &frontLeft, &frontLeftOld);

	int heightFrontRight = TestWaterHeight(rBoatItem, RBOAT_FRONT, RBOAT_SIDE, &frontRight);
	if (heightFrontRight < (frontRightOld.y - CLICK(0.5f)))
		rotation += DoRubberBoatShift2(rBoatItem, &frontRight, &frontRightOld);

	if (!slip)
	{
		int heightFront = TestWaterHeight(rBoatItem, 1000, 0, &front);
		if (heightFront < (frontOld.y - CLICK(0.5f)))
			DoRubberBoatShift2(rBoatItem, &front, &frontOld);
	}

	short roomNumber = rBoatItem->RoomNumber;
	auto floor = GetFloor(rBoatItem->Pose.Position.x, rBoatItem->Pose.Position.y, rBoatItem->Pose.Position.z, &roomNumber);
	int height = GetWaterHeight(rBoatItem->Pose.Position.x, rBoatItem->Pose.Position.y, rBoatItem->Pose.Position.z, roomNumber);

	if (height == NO_HEIGHT)
		height = GetFloorHeight(floor, rBoatItem->Pose.Position.x, rBoatItem->Pose.Position.y, rBoatItem->Pose.Position.z);

	if (height < (rBoatItem->Pose.Position.y - CLICK(0.5f)))
		DoRubberBoatShift2(rBoatItem, (Vector3Int*)&rBoatItem->Pose, &old);

	rBoat->ExtraRotation = rotation;
	int collide = GetRubberBoatCollisionAnim(rBoatItem, &moved);

	if (slip || collide)
	{
		int newVelocity = (rBoatItem->Pose.Position.z - old.z) * phd_cos(rBoatItem->Pose.Orientation.y) + (rBoatItem->Pose.Position.x - old.x) * phd_sin(rBoatItem->Pose.Orientation.y);

		if (lara->Vehicle == itemNumber &&
			rBoatItem->Animation.Velocity > (RBOAT_MAX_VELOCITY + RBOAT_ACCELERATION) &&
			newVelocity < rBoatItem->Animation.Velocity - 10)
		{
			laraItem->HitPoints -= rBoatItem->Animation.Velocity;
			laraItem->HitStatus = 1;
			SoundEffect(SFX_TR4_LARA_INJURY, &laraItem->Pose, 0);
			newVelocity /= 2;
			rBoatItem->Animation.Velocity /= 2;
		}

		if (slip)
		{
			if (rBoatItem->Animation.Velocity <= RBOAT_MAX_VELOCITY + 10)
				rBoatItem->Animation.Velocity = newVelocity;
		}
		else
		{
			if (rBoatItem->Animation.Velocity > 0 && newVelocity < rBoatItem->Animation.Velocity)
				rBoatItem->Animation.Velocity = newVelocity;
			else if (rBoatItem->Animation.Velocity < 0 && newVelocity > rBoatItem->Animation.Velocity)
				rBoatItem->Animation.Velocity = newVelocity;
		}

		if (rBoatItem->Animation.Velocity < RBOAT_MAX_BACK)
			rBoatItem->Animation.Velocity = RBOAT_MAX_BACK;
	}

	return collide;
}

static int DoRubberBoatDynamics(int height, int verticalVelocity, int* y)
{
	if (height > *y)
	{
		*y += verticalVelocity;
		if (*y > height)
		{
			*y = height;
			verticalVelocity = 0;
		}
		else
			verticalVelocity += 6;
	}
	else
	{
		verticalVelocity += (height - *y - verticalVelocity) / 8;
		if (verticalVelocity < -20)
			verticalVelocity = -20;

		if (*y > height)
			*y = height;
	}

	return verticalVelocity;
}

bool RubberBoatUserControl(ItemInfo* laraItem, ItemInfo* rBoatItem)
{
	auto* rBoat = (RubberBoatInfo*)rBoatItem->Data;

	bool noTurn = true;

	if (rBoatItem->Pose.Position.y >= (rBoat->Water - 128) && 
		rBoat->Water != NO_HEIGHT)
	{
		if (!(TrInput & RBOAT_IN_DISMOUNT) && !(TrInput & IN_LOOK) || rBoatItem->Animation.Velocity)
		{
			if ((TrInput & RBOAT_IN_LEFT && !(TrInput & RBOAT_IN_BACK)) ||
				(TrInput & RBOAT_IN_RIGHT && TrInput & RBOAT_IN_BACK))
			{
				if (rBoat->TurnRate > 0)
					rBoat->TurnRate -= ANGLE(0.25f);
				else
				{
					rBoat->TurnRate -= ANGLE(0.25f) / 2;
					if (rBoat->TurnRate < -ANGLE(4.0f))
						rBoat->TurnRate = -ANGLE(4.0f);
				}

				noTurn = false;
			}
			else if ((TrInput & RBOAT_IN_RIGHT && !(TrInput & RBOAT_IN_BACK)) ||
				(TrInput & RBOAT_IN_LEFT && TrInput & RBOAT_IN_BACK))
			{
				if (rBoat->TurnRate < 0)
					rBoat->TurnRate += ANGLE(0.25f);
				else
				{
					rBoat->TurnRate += ANGLE(0.25f) / 2;
					if (rBoat->TurnRate > ANGLE(4.0f))
						rBoat->TurnRate = ANGLE(4.0f);
				}

				noTurn = false;
			}

			if (TrInput & RBOAT_IN_BACK)
			{
				if (rBoatItem->Animation.Velocity > 0)
					rBoatItem->Animation.Velocity -= 5;
				else if (rBoatItem->Animation.Velocity > -20)
					rBoatItem->Animation.Velocity += -2;
			}
			else if (TrInput & RBOAT_IN_FORWARD)
			{
				int maxVelocity;
				if (TrInput & RBOAT_IN_SPEED)
					maxVelocity = 185;
				else
					maxVelocity = (TrInput & RBOAT_IN_SLOW) ? 37 : 110;

				if (rBoatItem->Animation.Velocity < maxVelocity)
					rBoatItem->Animation.Velocity += 3 + (5 * rBoatItem->Animation.Velocity) / (maxVelocity * 2);
				else if (rBoatItem->Animation.Velocity > (maxVelocity + 1))
					rBoatItem->Animation.Velocity -= 1;

			}
			else if (TrInput & (RBOAT_IN_LEFT | RBOAT_IN_RIGHT) &&
				rBoatItem->Animation.Velocity >= 0 &&
				rBoatItem->Animation.Velocity < 20)
			{
				if (!(TrInput & RBOAT_IN_DISMOUNT) && rBoatItem->Animation.Velocity == 0)
					rBoatItem->Animation.Velocity = 20;
			}
			else if (rBoatItem->Animation.Velocity > 1)
				rBoatItem->Animation.Velocity -= 1;
			else
				rBoatItem->Animation.Velocity = 0;
		}
		else
		{
			if (TrInput & (RBOAT_IN_LEFT | RBOAT_IN_RIGHT) &&
				rBoatItem->Animation.Velocity >= 0 &&
				rBoatItem->Animation.Velocity < 20)
			{
				if (!(TrInput & RBOAT_IN_DISMOUNT) && rBoatItem->Animation.Velocity == 0)
					rBoatItem->Animation.Velocity = 20;
			}
			else if (rBoatItem->Animation.Velocity > 1)
				rBoatItem->Animation.Velocity -= 1;
			else
				rBoatItem->Animation.Velocity = 0;

			if (TrInput & IN_LOOK && rBoatItem->Animation.Velocity == 0)
				LookUpDown(laraItem);
		}
	}

	return noTurn;
}

void RubberBoatCollision(short itemNum, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(laraItem);

	if (laraItem->HitPoints <= 0 || lara->Vehicle != NO_ITEM)
		return;

	auto* item = &g_Level.Items[itemNum];
	int mountType = GetRubberBoatMountType(laraItem, itemNum, coll);

	if (!mountType)
	{
		coll->Setup.EnableObjectPush = true;
		ObjectCollision(itemNum, laraItem, coll);
		return;
	}

	lara->Vehicle = itemNum;
	
	if (mountType == 1)
		laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_MOUNT_RIGHT;
	else if (mountType == 2)
		laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_MOUNT_LEFT;
	else if (mountType == 3)
		laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_MOUNT_JUMP;
	else
		laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_IDLE;

	laraItem->Pose.Position.x = item->Pose.Position.x;
	laraItem->Pose.Position.y = item->Pose.Position.y - 5;
	laraItem->Pose.Position.z = item->Pose.Position.z;
	laraItem->Pose.Orientation.x = 0;
	laraItem->Pose.Orientation.y = item->Pose.Orientation.y;
	laraItem->Pose.Orientation.z = 0;
	laraItem->Animation.Velocity = 0;
	laraItem->Animation.VerticalVelocity = 0;
	laraItem->Animation.Airborne = false;
	laraItem->Animation.ActiveState = 0;
	laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
	lara->Control.WaterStatus = WaterStatus::Dry;

	if (laraItem->RoomNumber != item->RoomNumber)
		ItemNewRoom(lara->ItemNumber, item->RoomNumber);

	AnimateItem(laraItem);

	if (g_Level.Items[itemNum].Status != ITEM_ACTIVE)
	{
		AddActiveItem(itemNum);
		g_Level.Items[itemNum].Status = ITEM_ACTIVE;
	}
}

static bool TestRubberBoatDismount(ItemInfo* laraItem, int direction)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* sBoatItem = &g_Level.Items[lara->Vehicle];

	short angle;
	if (direction < 0)
		angle = sBoatItem->Pose.Orientation.y - ANGLE(90.0f);
	else
		angle = sBoatItem->Pose.Orientation.y + ANGLE(90.0f);

	int x = sBoatItem->Pose.Position.x + SECTOR(1) * phd_sin(angle);
	int y = sBoatItem->Pose.Position.y;
	int z = sBoatItem->Pose.Position.z + SECTOR(1) * phd_cos(angle);

	auto collResult = GetCollision(x, y, z, sBoatItem->RoomNumber);

	if ((collResult.Position.Floor - sBoatItem->Pose.Position.y) < -512)
		return false;

	if (collResult.Position.FloorSlope || collResult.Position.Floor == NO_HEIGHT)
		return false;

	if ((collResult.Position.Ceiling - sBoatItem->Pose.Position.y) > -LARA_HEIGHT ||
		(collResult.Position.Floor - collResult.Position.Ceiling) < LARA_HEIGHT)
	{
		return false;
	}

	return true;
}

void RubberBoatAnimation(ItemInfo* laraItem, ItemInfo* rBoatItem, int collide)
{
	auto* rBoat = (RubberBoatInfo*)rBoatItem->Data;

	if (laraItem->HitPoints <= 0)
	{
		if (laraItem->Animation.ActiveState!= RBOAT_STATE_DEATH)
		{
			laraItem->Animation.TargetState = RBOAT_STATE_DEATH;
			laraItem->Animation.ActiveState = RBOAT_STATE_DEATH;
			laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_IDLE_DEATH;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
		}
	}
	else if (rBoatItem->Pose.Position.y < (rBoat->Water - CLICK(0.5f)) &&
		rBoatItem->Animation.VerticalVelocity > 0)
	{
		if (laraItem->Animation.ActiveState != RBOAT_STATE_FALL)
		{
			laraItem->Animation.TargetState = RBOAT_STATE_FALL;
			laraItem->Animation.ActiveState = RBOAT_STATE_FALL;
			laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_LEAP_START;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
		}
	}
	else if (collide)
	{
		if (laraItem->Animation.ActiveState != RBOAT_STATE_HIT)
		{
			laraItem->Animation.TargetState = RBOAT_STATE_HIT;
			laraItem->Animation.ActiveState = RBOAT_STATE_HIT;
			laraItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + collide;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
		}
	}
	else
	{
		switch (laraItem->Animation.ActiveState)
		{
		case RBOAT_STATE_IDLE:
			if (TrInput & RBOAT_IN_DISMOUNT)
			{
				if (rBoatItem->Animation.Velocity == 0)
				{
					if (TrInput & IN_RIGHT && TestRubberBoatDismount(laraItem, rBoatItem->Pose.Orientation.y + ANGLE(90.0f)))
						laraItem->Animation.TargetState = RBOAT_STATE_JUMP_RIGHT;
					else if (TrInput & IN_LEFT && TestRubberBoatDismount(laraItem, rBoatItem->Pose.Orientation.y - ANGLE(90.0f)))
						laraItem->Animation.TargetState = RBOAT_STATE_JUMP_LEFT;
				}
			}

			if (rBoatItem->Animation.Velocity > 0)
				laraItem->Animation.TargetState = RBOAT_STATE_MOVING;

			break;

		case RBOAT_STATE_MOVING:
			if (rBoatItem->Animation.Velocity <= 0)
				laraItem->Animation.TargetState = RBOAT_STATE_IDLE;

			if (TrInput & RBOAT_IN_RIGHT)
				laraItem->Animation.TargetState = RBOAT_STATE_TURN_RIGHT;
			else if (TrInput & RBOAT_IN_LEFT)
				laraItem->Animation.TargetState = RBOAT_STATE_TURN_LEFT;
			
			break;

		case RBOAT_STATE_FALL:
			laraItem->Animation.TargetState = RBOAT_STATE_MOVING;
			break;

		case RBOAT_STATE_TURN_RIGHT:
			if (rBoatItem->Animation.Velocity <= 0)
				laraItem->Animation.TargetState = RBOAT_STATE_IDLE;
			else if (!(TrInput & RBOAT_IN_RIGHT))
				laraItem->Animation.TargetState = RBOAT_STATE_MOVING;

			break;

		case RBOAT_STATE_TURN_LEFT:
			if (rBoatItem->Animation.Velocity <= 0)
				laraItem->Animation.TargetState = RBOAT_STATE_IDLE;
			else if (!(TrInput & RBOAT_IN_LEFT))
				laraItem->Animation.TargetState = RBOAT_STATE_MOVING;

			break;
		}
	}
}

static void TriggerRubberBoatMist(long x, long y, long z, long velocity, short angle, long snow)
{
	auto* sptr = &Sparks[GetFreeSpark()];

	sptr->on = 1;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;

	if (snow)
	{
		sptr->dR = 255;
		sptr->dG = 255;
		sptr->dB = 255;
	}
	else
	{
		sptr->dR = 64;
		sptr->dG = 64;
		sptr->dB = 64;
	}

	sptr->colFadeSpeed = 4 + (GetRandomControl() & 3);
	sptr->fadeToBlack = 12 - (snow * 8);
	sptr->sLife = sptr->life = (GetRandomControl() & 3) + 20;
	sptr->transType = TransTypeEnum::COLADD;
	sptr->extras = 0;
	sptr->dynamic = -1;

	sptr->x = x * ((GetRandomControl() & 15) - 8);
	sptr->y = y * ((GetRandomControl() & 15) - 8);
	sptr->z = z * ((GetRandomControl() & 15) - 8);
	long zv = velocity * phd_cos(angle) / 4;
	long xv = velocity * phd_sin(angle) / 4;
	sptr->xVel = xv + ((GetRandomControl() & 127) - 64);
	sptr->yVel = (velocity * 8) + (velocity * 4);
	sptr->zVel = zv + ((GetRandomControl() & 127) - 64);
	sptr->friction = 3;

	if (GetRandomControl() & 1)
	{
		sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		sptr->rotAng = GetRandomControl() & 4095;

		if (GetRandomControl() & 1)
			sptr->rotAdd = -(GetRandomControl() & 15) - 16;
		else
			sptr->rotAdd = (GetRandomControl() & 15) + 16;
	}
	else
		sptr->flags = SP_SCALE | SP_DEF | SP_EXPDEF;

	sptr->def = Objects[ID_EXPLOSION_SPRITES].meshIndex;

	if (!snow)
	{
		sptr->scalar = 4;
		sptr->gravity = 0;
		sptr->maxYvel = 0;
		long size = (GetRandomControl() & 7) + (velocity / 2) + 16;
	}
}

void DoRubberBoatDismount(ItemInfo* laraItem, ItemInfo* rBoatItem)
{
	auto* lara = GetLaraInfo(laraItem);

	if ((laraItem->Animation.ActiveState == RBOAT_STATE_JUMP_RIGHT || laraItem->Animation.ActiveState == RBOAT_STATE_JUMP_LEFT) &&
		laraItem->Animation.FrameNumber == g_Level.Anims[laraItem->Animation.AnimNumber].frameEnd)
	{
		if (laraItem->Animation.ActiveState == RBOAT_STATE_JUMP_LEFT)
			laraItem->Pose.Orientation.y -= ANGLE(90.0f);
		else
			laraItem->Pose.Orientation.y += ANGLE(90.0f);

		laraItem->Animation.TargetState = LS_JUMP_FORWARD;
		laraItem->Animation.ActiveState = LS_JUMP_FORWARD;
		laraItem->Animation.AnimNumber = LA_JUMP_FORWARD;
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
		laraItem->Animation.Velocity = 20;
		laraItem->Animation.VerticalVelocity = -40;
		laraItem->Animation.Airborne = true;
		laraItem->Pose.Orientation.x = 0;
		laraItem->Pose.Orientation.z = 0;
		lara->Vehicle = NO_ITEM;

		int x = laraItem->Pose.Position.x + 360 * phd_sin(laraItem->Pose.Orientation.y);
		int y = laraItem->Pose.Position.y - 90;
		int z = laraItem->Pose.Position.z + 360 * phd_cos(laraItem->Pose.Orientation.y);

		auto probe = GetCollision(x, y, z, laraItem->RoomNumber);
		if (probe.Position.Floor >= (y - CLICK(1)))
		{
			laraItem->Pose.Position.x = x;
			laraItem->Pose.Position.z = z;

			if (probe.RoomNumber != laraItem->RoomNumber)
				ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
		}
		laraItem->Pose.Position.y = y;

		rBoatItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT].animIndex;
		rBoatItem->Animation.FrameNumber = g_Level.Anims[rBoatItem->Animation.AnimNumber].frameBase;
	}
}

void RubberBoatControl(short itemNumber)
{
	auto* laraItem = LaraItem;
	auto* lara = GetLaraInfo(laraItem);
	auto* rBoatItem = &g_Level.Items[itemNumber];
	auto* rBoat = (RubberBoatInfo*)rBoatItem->Data;

	bool noTurn = true;
	bool drive = false;

	int pitch, height, ofs, nowake;

	Vector3Int frontLeft, frontRight;
	int collide = RubberBoatDynamics(laraItem, itemNumber);
	int heightFrontLeft = TestWaterHeight(rBoatItem, RBOAT_FRONT, -RBOAT_SIDE, &frontLeft);
	int heightFrontRight = TestWaterHeight(rBoatItem, RBOAT_FRONT, RBOAT_SIDE, &frontRight);


	if (lara->Vehicle == itemNumber)
	{
		TestTriggers(rBoatItem, false);
		TestTriggers(rBoatItem, true);
	}

	auto probe = GetCollision(rBoatItem);
	int water = GetWaterHeight(rBoatItem->Pose.Position.x, rBoatItem->Pose.Position.y, rBoatItem->Pose.Position.z, probe.RoomNumber);
	rBoat->Water = water;

	if (lara->Vehicle == itemNumber && laraItem->HitPoints > 0)
	{
		switch (laraItem->Animation.ActiveState)
		{
		case RBOAT_STATE_MOUNT:
		case RBOAT_STATE_JUMP_RIGHT:
		case RBOAT_STATE_JUMP_LEFT:
			break;

		default:
			drive = true;
			noTurn = RubberBoatUserControl(laraItem, rBoatItem);
			break;
		}
	}
	else
	{
		if (rBoatItem->Animation.Velocity > RBOAT_SLOW_DOWN)
			rBoatItem->Animation.Velocity -= RBOAT_SLOW_DOWN;
		else
			rBoatItem->Animation.Velocity = 0;
	}

	if (noTurn)
	{
		if (rBoat->TurnRate < -RBOAT_UNDO_TURN)
			rBoat->TurnRate += RBOAT_UNDO_TURN;
		else if (rBoat->TurnRate > RBOAT_UNDO_TURN)
			rBoat->TurnRate -= RBOAT_UNDO_TURN;
		else
			rBoat->TurnRate = 0;
	}

	height = probe.Position.Floor;

	rBoatItem->Floor = height - 5;
	if (rBoat->Water == NO_HEIGHT)
		rBoat->Water = height;
	else
		rBoat->Water -= 5;

	rBoat->LeftVerticalVelocity = DoRubberBoatDynamics(heightFrontLeft, rBoat->LeftVerticalVelocity, (int*)&frontLeft.y);
	rBoat->RightVerticalVelocity = DoRubberBoatDynamics(heightFrontRight, rBoat->RightVerticalVelocity, (int*)&frontRight.y);
	ofs = rBoatItem->Animation.VerticalVelocity;
	rBoatItem->Animation.VerticalVelocity = DoRubberBoatDynamics(rBoat->Water, rBoatItem->Animation.VerticalVelocity, (int*)&rBoatItem->Pose.Position.y);

	height = frontLeft.y + frontRight.y;
	if (height < 0)
		height = -(abs(height) / 2);
	else
		height = height / 2;

	short xRot = phd_atan(RBOAT_FRONT, rBoatItem->Pose.Position.y - height);
	short rRot = phd_atan(RBOAT_SIDE, height - frontLeft.y);

	rBoatItem->Pose.Orientation.x += ((xRot - rBoatItem->Pose.Orientation.x) / 2);
	rBoatItem->Pose.Orientation.z += ((rRot - rBoatItem->Pose.Orientation.z) / 2);

	if (!xRot && abs(rBoatItem->Pose.Orientation.x) < 4)
		rBoatItem->Pose.Orientation.x = 0;
	if (!rRot && abs(rBoatItem->Pose.Orientation.z) < 4)
		rBoatItem->Pose.Orientation.z = 0;

	if (lara->Vehicle == itemNumber)
	{
		RubberBoatAnimation(laraItem, rBoatItem, collide);

		if (probe.RoomNumber != rBoatItem->RoomNumber)
		{
			ItemNewRoom(itemNumber, probe.RoomNumber);
			ItemNewRoom(lara->ItemNumber, probe.RoomNumber);
		}

		rBoatItem->Pose.Orientation.z += rBoat->LeanAngle;
		laraItem->Pose.Position.x = rBoatItem->Pose.Position.x;
		laraItem->Pose.Orientation.x = rBoatItem->Pose.Orientation.x;
		laraItem->Pose.Position.y = rBoatItem->Pose.Position.y;
		laraItem->Pose.Orientation.y = rBoatItem->Pose.Orientation.y;
		laraItem->Pose.Position.z = rBoatItem->Pose.Position.z;
		laraItem->Pose.Orientation.z = rBoatItem->Pose.Orientation.z;

		AnimateItem(laraItem);

		if (laraItem->HitPoints > 0)
		{
			rBoatItem->Animation.AnimNumber = Objects[ID_RUBBER_BOAT].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex);
			rBoatItem->Animation.FrameNumber = g_Level.Anims[rBoatItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);
		}

		Camera.targetElevation = -ANGLE(20);
		Camera.targetDistance = 2048;
	}
	else
	{
		if (probe.RoomNumber != rBoatItem->RoomNumber)
			ItemNewRoom(itemNumber, probe.RoomNumber);

		rBoatItem->Pose.Orientation.z += rBoat->LeanAngle;
	}

	pitch = rBoatItem->Animation.Velocity;
	rBoat->Pitch += ((pitch - rBoat->Pitch) / 4);

	if (rBoatItem->Animation.Velocity > 8)
		SoundEffect(SFX_TR3_RUBBERBOAT_MOVING, &rBoatItem->Pose, 0, 0.5f + (float)abs(rBoat->Pitch) / (float)RBOAT_MAX_VELOCITY);
	else if (drive)
		SoundEffect(SFX_TR3_RUBBERBOAT_IDLE, &rBoatItem->Pose, 0, 0.5f + (float)abs(rBoat->Pitch) / (float)RBOAT_MAX_VELOCITY);

	if (lara->Vehicle != itemNumber)
		return;

	DoRubberBoatDismount(laraItem, rBoatItem);

	short probedRoomNumber = GetCollision(rBoatItem->Pose.Position.x, rBoatItem->Pose.Position.y + 128, rBoatItem->Pose.Position.z, rBoatItem->RoomNumber).RoomNumber;
	height = GetWaterHeight(rBoatItem->Pose.Position.x, rBoatItem->Pose.Position.y + 128, rBoatItem->Pose.Position.z, probedRoomNumber);
	if (height > rBoatItem->Pose.Position.y + 32 || height == NO_HEIGHT)
		height = 0;
	else
		height = 1;

	Vector3Int prop = { 0, 0, -80 };
	GetJointAbsPosition(rBoatItem, &prop, 2);

	probedRoomNumber = GetCollision(prop.x, prop.y, prop.z, rBoatItem->RoomNumber).RoomNumber;

	if (rBoatItem->Animation.Velocity &&
		height < prop.y &&
		height != NO_HEIGHT)
	{
		TriggerRubberBoatMist(prop.x, prop.y, prop.z, abs(rBoatItem->Animation.Velocity), rBoatItem->Pose.Orientation.y + 0x8000, 0);
		if ((GetRandomControl() & 1) == 0)
		{
			PHD_3DPOS pos;
			pos.Position.x = prop.x + (GetRandomControl() & 63) - 32;
			pos.Position.y = prop.y + (GetRandomControl() & 15);
			pos.Position.z = prop.z + (GetRandomControl() & 63) - 32;

			short roomNumber = rBoatItem->RoomNumber;
			GetFloor(pos.Position.x, pos.Position.y, pos.Position.z, &roomNumber);
			CreateBubble((Vector3Int*)&pos, roomNumber, 16, 8, 0, 0, 0, 0);
		}
	}
	else
	{
		height = GetCollision(prop.x, prop.y, prop.z, rBoatItem->RoomNumber).Position.Floor;
		if (prop.y > height &&
			!TestEnvironment(ENV_FLAG_WATER, probedRoomNumber))
		{
			GameVector pos;
			pos.x = prop.x;
			pos.y = prop.y;
			pos.z = prop.z;

			long cnt = (GetRandomControl() & 3) + 3;
			for (;cnt>0;cnt--)
				TriggerRubberBoatMist(prop.x, prop.y, prop.z, ((GetRandomControl() & 15) + 96) * 16, rBoatItem->Pose.Orientation.y + 0x4000 + GetRandomControl(), 1);
		}
	}
}
