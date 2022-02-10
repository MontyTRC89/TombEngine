#include "framework.h"
#include "Objects/TR3/Vehicles/rubberboat.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/effects/bubble.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
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

#define RBOAT_MAX_SPEED		110
#define RBOAT_SLOW_SPEED	RBOAT_MAX_SPEED / 3
#define RBOAT_FAST_SPEED	RBOAT_MAX_SPEED + 75
#define RBOAT_MIN_SPEED		20
#define RBOAT_MAX_BACK		-20
#define RBOAT_MAX_KICK		-80

#define RBOAT_ACCELERATION	5
#define RBOAT_BRAKE			5
#define RBOAT_SLOWDOWN		1
#define RBOAT_UNDO_TURN		ANGLE(0.25f)
#define RBOAT_TURN			ANGLE(0.125f)

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

void InitialiseRubberBoat(short itemNum)
{
	ITEM_INFO* rBoat = &g_Level.Items[itemNum];
	rBoat->Data = RUBBER_BOAT_INFO();
	auto rBoatInfo = (RUBBER_BOAT_INFO*)rBoat->Data;

	rBoatInfo->boatTurn = 0;
	rBoatInfo->tiltAngle = 0;
	rBoatInfo->rightFallspeed = 0;
	rBoatInfo->leftFallspeed = 0;
	rBoatInfo->extraRotation = 0;
	rBoatInfo->water = 0;
	rBoatInfo->pitch = 0;
}

void DrawRubberBoat(ITEM_INFO *item)
{
	/* TODO: WTF?
	RUBBER_BOAT_INFO *b;

	b = item->data;
	item->data = &b->propRot;
	DrawAnimatingItem(item);
	item->data = b;
	*/
}

RubberBoatMountType RubberBoatCheckGeton(short itemNum, ITEM_INFO* lara, COLL_INFO *coll)
{
	ITEM_INFO* rBoat = &g_Level.Items[itemNum];
	RubberBoatMountType getOn = RBOAT_MOUNT_NONE;

	if (!(TrInput & IN_ACTION) ||
		Lara.Control.HandStatus != HandStatus::Free ||
		lara->Airborne ||
		rBoat->Velocity)
	{
		return RBOAT_MOUNT_NONE;
	}

	short xDelta = lara->Position.xPos - rBoat->Position.xPos;
	short zDelta = lara->Position.zPos - rBoat->Position.zPos;
	int dist = zDelta * phd_cos(-rBoat->Position.yRot) - xDelta * phd_sin(-rBoat->Position.yRot);

	if (dist > 512)
		return RBOAT_MOUNT_NONE;

	short rot = rBoat->Position.yRot - lara->Position.yRot;
	if (Lara.Control.WaterStatus == WaterStatus::WaterSurface || Lara.Control.WaterStatus == WaterStatus::Wade)
	{
		if (rot > ANGLE(45.0f) && rot < ANGLE(135.0f))
			getOn = RBOAT_MOUNT_LEFT;
		if (rot < -ANGLE(135.0f) && rot > -ANGLE(45.0f))
			getOn = RBOAT_MOUNT_RIGHT;
	}
	else if (Lara.Control.WaterStatus == WaterStatus::Dry)
	{
		if (lara->VerticalVelocity > 0)
		{
			if ((lara->Position.yPos + (STEP_SIZE * 2)) > rBoat->Position.yPos)
				getOn = RBOAT_MOUNT_JUMP;
		}
		else if (lara->VerticalVelocity == 0)
		{
			if (rot > -ANGLE(135.0f) && rot < ANGLE(135.0f))
			{
				if (lara->Position.xPos == rBoat->Position.xPos &&
					lara->Position.yPos == rBoat->Position.xPos &&
					lara->Position.zPos == rBoat->Position.zPos)
				{
					getOn = RBOAT_MOUNT_LEVEL_START;
				}
				else
					getOn = RBOAT_MOUNT_JUMP;
			}
		}
	}

	if (!getOn)
		return RBOAT_MOUNT_NONE;

	if (!TestBoundsCollide(rBoat, lara, coll->Setup.Radius))
		return RBOAT_MOUNT_NONE;

	if (!TestCollision(rBoat, lara))
		return RBOAT_MOUNT_NONE;

	return getOn;
}

static long TestWaterHeight(ITEM_INFO *item, long zOff, long xOff, PHD_VECTOR *pos)
{
	FLOOR_INFO *floor;
	long height;
	short roomNum;
	float s, c;

	pos->y = item->Position.yPos - zOff * phd_sin(item->Position.xRot) + xOff * phd_sin(item->Position.zRot);

	c = phd_cos(item->Position.yRot);
	s = phd_sin(item->Position.yRot);

	pos->z = item->Position.zPos + zOff * c - xOff * s;
	pos->x = item->Position.xPos + zOff * s + xOff * c;

	roomNum = item->RoomNumber;
	GetFloor(pos->x, pos->y, pos->z, &roomNum);
	height = GetWaterHeight(pos->x, pos->y, pos->z, roomNum);
	if (height == NO_HEIGHT)
	{
		floor = GetFloor(pos->x, pos->y, pos->z, &roomNum);
		height = GetFloorHeight(floor, pos->x, pos->y, pos->z);
			if (height == NO_HEIGHT)
				return height;
	}

	return height - 5;
}

static void DoRubberBoatShift(int boatNum)
{
	ITEM_INFO *item, *boat;
	int itemNum, distance, x, z;

	boat = &g_Level.Items[boatNum];

	itemNum = g_Level.Rooms[boat->RoomNumber].itemNumber;
	while (itemNum != NO_ITEM)
	{

		item = &g_Level.Items[itemNum];

		if (item->ObjectNumber == ID_RUBBER_BOAT && itemNum != boatNum && Lara.Vehicle != itemNum)
		{
			x = item->Position.xPos - boat->Position.xPos;
			z = item->Position.zPos - boat->Position.zPos;
			distance = SQUARE(x) + SQUARE(z);
			if (distance < 1000000)
			{
				boat->Position.xPos = item->Position.xPos - x * 1000000 / distance;
				boat->Position.zPos = item->Position.zPos - z * 1000000 / distance;
			}
			return;
		}
		itemNum = item->NextItem;
	}
}

static int DoRubberBoatShift2(ITEM_INFO *skidoo, PHD_VECTOR *pos, PHD_VECTOR *old)
{
	int x, z;
	int x_old, z_old;
	int shift_x, shift_z;

	x = pos->x / SECTOR(1);
	z = pos->z / SECTOR(1);

	x_old = old->x / SECTOR(1);
	z_old = old->z / SECTOR(1);

	shift_x = pos->x & (WALL_SIZE - 1);
	shift_z = pos->z & (WALL_SIZE - 1);

	if (x == x_old)
	{
		if (z == z_old)
		{
			skidoo->Position.zPos += (old->z - pos->z);
			skidoo->Position.xPos += (old->x - pos->x);
		}
		else if (z > z_old)
		{
			skidoo->Position.zPos -= shift_z + 1;
			return (pos->x - skidoo->Position.xPos);
		}
		else
		{
			skidoo->Position.zPos += WALL_SIZE - shift_z;
			return (skidoo->Position.xPos - pos->x);
		}
	}
	else if (z == z_old)
	{
		if (x > x_old)
		{
			skidoo->Position.xPos -= shift_x + 1;
			return (skidoo->Position.zPos - pos->z);
		}
		else
		{
			skidoo->Position.xPos += WALL_SIZE - shift_x;
			return (pos->z - skidoo->Position.zPos);
		}
	}
	else
	{
		short roomNumber;
		FLOOR_INFO* floor;
		int height;

		x = z = 0;

		roomNumber = skidoo->RoomNumber;
		floor = GetFloor(old->x, pos->y, pos->z, &roomNumber);
		height = GetFloorHeight(floor, old->x, pos->y, pos->z);
		if (height < old->y - STEP_SIZE)
		{
			if (pos->z > old->z)
				z = -shift_z - 1;
			else
				z = WALL_SIZE - shift_z;
		}

		roomNumber = skidoo->RoomNumber;
		floor = GetFloor(pos->x, pos->y, old->z, &roomNumber);
		height = GetFloorHeight(floor, pos->x, pos->y, old->z);
		if (height < old->y - STEP_SIZE)
		{
			if (pos->x > old->x)
				x = -shift_x - 1;
			else
				x = WALL_SIZE - shift_x;
		}

		if (x && z)
		{
			skidoo->Position.zPos += z;
			skidoo->Position.xPos += x;
		}
		else if (z)
		{
			skidoo->Position.zPos += z;
			if (z > 0)
				return (skidoo->Position.xPos - pos->x);
			else
				return (pos->x - skidoo->Position.xPos);
		}
		else if (x)
		{
			skidoo->Position.xPos += x;
			if (x > 0)
				return (pos->z - skidoo->Position.zPos);
			else
				return (skidoo->Position.zPos - pos->z);
		}
		else
		{
			skidoo->Position.zPos += (old->z - pos->z);
			skidoo->Position.xPos += (old->x - pos->x);
		}
	}

	return 0;
}

static int GetRubberBoatCollisionAnim(ITEM_INFO *rubber, PHD_VECTOR *moved)
{
	long front, side;
	float c, s;

	moved->x = rubber->Position.xPos - moved->x;
	moved->z = rubber->Position.zPos - moved->z;

	if (moved->x || moved->z)
	{
		c = phd_cos(rubber->Position.yRot);
		s = phd_sin(rubber->Position.yRot);
		front = moved->z * c + moved->x * s;
		side = -moved->z * s + moved->x * c;

		if (abs(front) > abs(side))
		{
			if (front > 0)
				return 14;
			else
				return 13;
		}
		else
		{
			if (side > 0)
				return 11;
			else
				return 12;
		}
	}
	return 0;
}

static int RubberBoatDynamics(short boat_number)
{
	ITEM_INFO* boat;
	RUBBER_BOAT_INFO* binfo;
	PHD_VECTOR moved, fl, fr, br, bl, f;
	PHD_VECTOR old, fl_old, fr_old, bl_old, br_old, f_old;
	int hfl, hfr, hbr, hbl, hf;
	int hfr_old, hfl_old, hbr_old, hbl_old, hf_old;
	FLOOR_INFO* floor;
	int height, slip, collide;
	short room_number, rot;
	int newspeed;

	boat = &g_Level.Items[boat_number];
	binfo = (RUBBER_BOAT_INFO*)boat->Data;

	boat->Position.zRot -= binfo->tiltAngle;

	hfl_old = TestWaterHeight(boat, RBOAT_FRONT, -RBOAT_SIDE, &fl_old);
	hfr_old = TestWaterHeight(boat, RBOAT_FRONT, RBOAT_SIDE, &fr_old);
	hbl_old = TestWaterHeight(boat, -RBOAT_FRONT, -RBOAT_SIDE, &bl_old);
	hbr_old = TestWaterHeight(boat, -RBOAT_FRONT, RBOAT_SIDE, &br_old);
	hf_old = TestWaterHeight(boat, 1000, 0, &f_old);
	old.x = boat->Position.xPos;
	old.y = boat->Position.yPos;
	old.z = boat->Position.zPos;

	if (bl_old.y > hbl_old)
		bl_old.y = hbl_old;
	if (br_old.y > hbr_old)
		br_old.y = hbr_old;
	if (fl_old.y > hfl_old)
		fl_old.y = hfl_old;
	if (fr_old.y > hfr_old)
		fr_old.y = hfr_old;
	if (f_old.y > hf_old)
		f_old.y = hf_old;

	boat->Position.yRot += binfo->boatTurn + binfo->extraRotation;
	binfo->tiltAngle = binfo->boatTurn * 6;

	boat->Position.zPos += boat->Velocity * phd_cos(boat->Position.yRot);
	boat->Position.xPos += boat->Velocity * phd_sin(boat->Position.yRot);
	if (boat->Velocity >= 0)
		binfo->propRot += (boat->Velocity * (ONE_DEGREE * 3)) + (ONE_DEGREE * 2);
	else
		binfo->propRot += ONE_DEGREE * 33;

	slip = RBOAT_SIDE_SLIP * phd_sin(boat->Position.zRot);
	if (!slip && boat->Position.zRot)
		slip = (boat->Position.zRot > 0) ? 1 : -1;
	boat->Position.zPos -= slip * phd_sin(boat->Position.yRot);
	boat->Position.xPos += slip * phd_cos(boat->Position.yRot);

	slip = RBOAT_SLIP * phd_sin(boat->Position.xRot);
	if (!slip && boat->Position.xRot)
		slip = (boat->Position.xRot > 0) ? 1 : -1;
	boat->Position.zPos -= slip * phd_cos(boat->Position.yRot);
	boat->Position.xPos -= slip * phd_sin(boat->Position.yRot);

	moved.x = boat->Position.xPos;
	moved.z = boat->Position.zPos;

	DoRubberBoatShift(boat_number);

	rot = 0;
	hbl = TestWaterHeight(boat, -RBOAT_FRONT, -RBOAT_SIDE, &bl);
	if (hbl < bl_old.y - STEP_SIZE / 2)
		rot = DoRubberBoatShift2(boat, &bl, &bl_old);

	hbr = TestWaterHeight(boat, -RBOAT_FRONT, RBOAT_SIDE, &br);
	if (hbr < br_old.y - STEP_SIZE / 2)
		rot += DoRubberBoatShift2(boat, &br, &br_old);

	hfl = TestWaterHeight(boat, RBOAT_FRONT, -RBOAT_SIDE, &fl);
	if (hfl < fl_old.y - STEP_SIZE / 2)
		rot += DoRubberBoatShift2(boat, &fl, &fl_old);

	hfr = TestWaterHeight(boat, RBOAT_FRONT, RBOAT_SIDE, &fr);
	if (hfr < fr_old.y - STEP_SIZE / 2)
		rot += DoRubberBoatShift2(boat, &fr, &fr_old);

	if (!slip)
	{
		hf = TestWaterHeight(boat, 1000, 0, &f);
		if (hf < f_old.y - STEP_SIZE / 2)
			DoRubberBoatShift2(boat, &f, &f_old);
	}

	room_number = boat->RoomNumber;
	floor = GetFloor(boat->Position.xPos, boat->Position.yPos, boat->Position.zPos, &room_number);
	height = GetWaterHeight(boat->Position.xPos, boat->Position.yPos, boat->Position.zPos, room_number);
	if (height == NO_HEIGHT)
		height = GetFloorHeight(floor, boat->Position.xPos, boat->Position.yPos, boat->Position.zPos);
	if (height < boat->Position.yPos - STEP_SIZE / 2)
		DoRubberBoatShift2(boat, (PHD_VECTOR*)&boat->Position, &old);

	binfo->extraRotation = rot;

	collide = GetRubberBoatCollisionAnim(boat, &moved);

	if (slip || collide)
	{
		newspeed = (boat->Position.zPos - old.z) * phd_cos(boat->Position.yRot) + (boat->Position.xPos - old.x) * phd_sin(boat->Position.yRot);

		if (Lara.Vehicle == boat_number && boat->Velocity > RBOAT_MAX_SPEED + RBOAT_ACCELERATION && newspeed < boat->Velocity - 10)
		{
			LaraItem->HitPoints -= boat->Velocity;
			LaraItem->HitStatus = 1;
			SoundEffect(SFX_TR4_LARA_INJURY, &LaraItem->Position, 0);
			newspeed /= 2;
			boat->Velocity /= 2;
		}

		if (slip)
		{
			if (boat->Velocity <= RBOAT_MAX_SPEED + 10)
				boat->Velocity = newspeed;
		}
		else
		{
			if (boat->Velocity > 0 && newspeed < boat->Velocity)
				boat->Velocity = newspeed;
			else if (boat->Velocity < 0 && newspeed > boat->Velocity)
				boat->Velocity = newspeed;
		}

		if (boat->Velocity < RBOAT_MAX_BACK)
			boat->Velocity = RBOAT_MAX_BACK;
	}

	return collide;
}

static int DoRubberBoatDynamics(int height, int fallspeed, int *y)
{
	if (height > *y)
	{
		*y += fallspeed;
		if (*y > height)
		{
			*y = height;
			fallspeed = 0;
		}
		else
			fallspeed += 6;
	}
	else
	{
		fallspeed += ((height - *y - fallspeed) / 8);
		if (fallspeed < -20)
			fallspeed = -20;

		if (*y > height)
			*y = height;
	}
	return fallspeed;
}

int RubberBoatUserControl(ITEM_INFO *boat)
{
	int noTurn(1), maxSpeed;
	RUBBER_BOAT_INFO *binfo;

	binfo = (RUBBER_BOAT_INFO*)boat->Data;

	if (boat->Position.yPos >= binfo->water - 128 && binfo->water != NO_HEIGHT)
	{
		if (!(TrInput & RBOAT_IN_DISMOUNT) && !(TrInput & IN_LOOK) || boat->Velocity)
		{
			if ((TrInput & RBOAT_IN_LEFT && !(TrInput & RBOAT_IN_BACK)) ||
				(TrInput & RBOAT_IN_RIGHT && TrInput & RBOAT_IN_BACK))
			{
				if (binfo->boatTurn > 0)
					binfo->boatTurn -= ANGLE(1) / 4;
				else
				{
					binfo->boatTurn -= ANGLE(1) / 8;
					if (binfo->boatTurn < -ANGLE(4))
						binfo->boatTurn = -ANGLE(4);
				}
				noTurn = 0;
			}
			else if ((TrInput & RBOAT_IN_RIGHT && !(TrInput & RBOAT_IN_BACK)) ||
				(TrInput & RBOAT_IN_LEFT && TrInput & RBOAT_IN_BACK))
			{
				if (binfo->boatTurn < 0)
					binfo->boatTurn += ANGLE(1) / 4;
				else
				{
					binfo->boatTurn += ANGLE(1) / 8;
					if (binfo->boatTurn > ANGLE(4))
						binfo->boatTurn = ANGLE(4);
				}
				noTurn = 0;
			}

			if (TrInput & RBOAT_IN_BACK)
			{
				if (boat->Velocity > 0)
					boat->Velocity -= 5;
				else if (boat->Velocity > -20)
					boat->Velocity += -2;
			}
			else if (TrInput & RBOAT_IN_FORWARD)
			{
				if (TrInput & RBOAT_IN_SPEED)
					maxSpeed = 185;
				else
					maxSpeed = (TrInput & RBOAT_IN_SLOW) ? 37 : 110;

				if (boat->Velocity < maxSpeed)
					boat->Velocity += 3 + (5 * boat->Velocity) / (2 * maxSpeed);
				else if (boat->Velocity > maxSpeed + 1)
					boat->Velocity -= 1;

			}
			else if (boat->Velocity >= 0 && boat->Velocity < 20 && TrInput & (RBOAT_IN_LEFT | RBOAT_IN_RIGHT))
			{
				if (boat->Velocity == 0 && !(TrInput & RBOAT_IN_DISMOUNT))
					boat->Velocity = 20;
			}
			else if (boat->Velocity > 1)
				boat->Velocity -= 1;
			else
				boat->Velocity = 0;
		}
		else
		{
			if (boat->Velocity >= 0 && boat->Velocity < 20 && TrInput & (RBOAT_IN_LEFT | RBOAT_IN_RIGHT))
			{
				if (boat->Velocity == 0 && !(TrInput & RBOAT_IN_DISMOUNT))
					boat->Velocity = 20;
			}
			else if (boat->Velocity > 1)
				boat->Velocity -= 1;
			else
				boat->Velocity = 0;

			if (TrInput & IN_LOOK && boat->Velocity == 0)
				LookUpDown();
		}
	}
	return noTurn;
}

void RubberBoatCollision(short itemNum, ITEM_INFO *lara, COLL_INFO *coll)
{
	int getOn;
	ITEM_INFO *item;

	if (lara->HitPoints <= 0 || Lara.Vehicle != NO_ITEM)
		return;

	item = &g_Level.Items[itemNum];
	getOn = RubberBoatCheckGeton(itemNum, lara, coll);

	if (!getOn)
	{
		coll->Setup.EnableObjectPush = true;
		ObjectCollision(itemNum, lara, coll);
		return;
	}

	Lara.Vehicle = itemNum;
	
	if (getOn == 1)
		lara->AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_MOUNT_RIGHT;
	else if (getOn == 2)
		lara->AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_MOUNT_LEFT;
	else if (getOn == 3)
		lara->AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_MOUNT_JUMP;
	else
		lara->AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_IDLE;

	Lara.Control.WaterStatus = WaterStatus::Dry;
	lara->Position.xPos = item->Position.xPos;
	lara->Position.yPos = item->Position.yPos - 5;
	lara->Position.zPos = item->Position.zPos;
	lara->Position.xRot = 0;
	lara->Position.yRot = item->Position.yRot;
	lara->Position.zRot = 0;
	lara->Airborne = false;
	lara->VerticalVelocity = 0;
	lara->Velocity = 0;
	lara->FrameNumber = g_Level.Anims[lara->AnimNumber].frameBase;
	lara->ActiveState = 0;
	lara->TargetState = 0;

	if (lara->RoomNumber != item->RoomNumber)
		ItemNewRoom(Lara.itemNumber, item->RoomNumber);

	AnimateItem(lara);

	if (g_Level.Items[itemNum].Status != ITEM_ACTIVE)
	{
		AddActiveItem(itemNum);
		g_Level.Items[itemNum].Status = ITEM_ACTIVE;
	}
}

static int CanGetOffRubberBoat(int direction)
{
	short angle;
	auto boat = &g_Level.Items[Lara.Vehicle];

	if (direction < 0)
		angle = boat->Position.yRot - ANGLE(90);
	else
		angle = boat->Position.yRot + ANGLE(90);

	auto x = boat->Position.xPos + 1024 * phd_sin(angle);
	auto y = boat->Position.yPos;
	auto z = boat->Position.zPos + 1024 * phd_cos(angle);

	auto collResult = GetCollisionResult(x, y, z, boat->RoomNumber);

	if (collResult.Position.Floor - boat->Position.yPos < -512)
		return 0;

	if (collResult.Position.FloorSlope || collResult.Position.Floor == NO_HEIGHT)
		return 0;

	if ((collResult.Position.Ceiling - boat->Position.yPos > -LARA_HEIGHT) || (collResult.Position.Floor - collResult.Position.Ceiling < LARA_HEIGHT))
		return 0;

	return 1;
}

void RubberBoatAnimation(ITEM_INFO *boat, int collide)
{
	RUBBER_BOAT_INFO *binfo;
	binfo = (RUBBER_BOAT_INFO*)boat->Data;

	if (LaraItem->HitPoints <= 0)
	{
		if (LaraItem->ActiveState!= RBOAT_STATE_DEATH)
		{
			LaraItem->AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_IDLE_DEATH;
			LaraItem->FrameNumber = g_Level.Anims[LaraItem->AnimNumber].frameBase;
			LaraItem->TargetState = RBOAT_STATE_DEATH;
			LaraItem->ActiveState = RBOAT_STATE_DEATH;
		}
	}
	else if (boat->Position.yPos < binfo->water - 128 && boat->VerticalVelocity > 0)
	{
		if (LaraItem->ActiveState != RBOAT_STATE_FALL)
		{
			LaraItem->AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_LEAP_START;
			LaraItem->FrameNumber = g_Level.Anims[LaraItem->AnimNumber].frameBase;
			LaraItem->TargetState = RBOAT_STATE_FALL;
			LaraItem->ActiveState = RBOAT_STATE_FALL;
		}
	}
	else if (collide)
	{
		if (LaraItem->ActiveState != RBOAT_STATE_HIT)
		{
			LaraItem->AnimNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + collide;
			LaraItem->FrameNumber = g_Level.Anims[LaraItem->AnimNumber].frameBase;
			LaraItem->TargetState = RBOAT_STATE_HIT;
			LaraItem->ActiveState = RBOAT_STATE_HIT;
		}
	}
	else
	{
		switch (LaraItem->ActiveState)
		{
		case RBOAT_STATE_IDLE:
			if (TrInput & IN_ROLL)
			{
				if (boat->Velocity == 0)
				{
					if ((TrInput & IN_RIGHT) && CanGetOffRubberBoat(boat->Position.yRot + ANGLE(90)))
						LaraItem->TargetState = RBOAT_STATE_JUMP_RIGHT;
					else if ((TrInput & IN_LEFT) && CanGetOffRubberBoat(boat->Position.yRot - ANGLE(90)))
						LaraItem->TargetState = RBOAT_STATE_JUMP_LEFT;
				}
			}

			if (boat->Velocity > 0)
				LaraItem->TargetState = RBOAT_STATE_MOVING;

			break;
		case RBOAT_STATE_MOVING:
			if (boat->Velocity <= 0)
				LaraItem->TargetState = RBOAT_STATE_IDLE;
			if (TrInput & IN_RIGHT)
				LaraItem->TargetState = RBOAT_STATE_TURN_RIGHT;
			else if (TrInput & IN_LEFT)
				LaraItem->TargetState = RBOAT_STATE_TURN_LEFT;
			
			break;
		case RBOAT_STATE_FALL:
			LaraItem->TargetState = RBOAT_STATE_MOVING;
			break;
		case RBOAT_STATE_TURN_RIGHT:
			if (boat->Velocity <= 0)
				LaraItem->TargetState = RBOAT_STATE_IDLE;
			else if (!(TrInput & IN_RIGHT))
				LaraItem->TargetState = RBOAT_STATE_MOVING;
			break;
		case RBOAT_STATE_TURN_LEFT:
			if (boat->Velocity <= 0)
				LaraItem->TargetState = RBOAT_STATE_IDLE;
			else if (!(TrInput & IN_LEFT))
				LaraItem->TargetState = RBOAT_STATE_MOVING;
			break;
		}
	}
}

static void TriggerRubberBoatMist(long x, long y, long z, long speed, short angle, long snow)
{
	long size, xv, zv;
	SPARKS *sptr;

	sptr = &Sparks[GetFreeSpark()];

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
	zv = speed * phd_cos(angle) / 4;
	xv = speed * phd_sin(angle) / 4;
	sptr->xVel = xv + ((GetRandomControl() & 127) - 64);
	sptr->yVel = (speed * 8) + (speed * 4);
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
		size = (GetRandomControl() & 7) + (speed / 2) + 16;
	}
}

void RubberBoatDoGetOff(ITEM_INFO* boat)
{
	if ((LaraItem->ActiveState == RBOAT_STATE_JUMP_RIGHT || LaraItem->ActiveState == RBOAT_STATE_JUMP_LEFT) 
		&& LaraItem->FrameNumber == g_Level.Anims[LaraItem->AnimNumber].frameEnd)
	{
		short roomNum;
		int x, y, z;
		FLOOR_INFO *floor;

		if (LaraItem->ActiveState == 4)
			LaraItem->Position.yRot -= ANGLE(90);
		else
			LaraItem->Position.yRot += ANGLE(90);

		LaraItem->AnimNumber = LA_JUMP_FORWARD;
		LaraItem->FrameNumber = g_Level.Anims[LaraItem->AnimNumber].frameBase;
		LaraItem->TargetState = LS_JUMP_FORWARD;
		LaraItem->ActiveState = LS_JUMP_FORWARD;
		LaraItem->Airborne = true;
		LaraItem->VerticalVelocity = -40;
		LaraItem->Velocity = 20;
		LaraItem->Position.xRot = 0;
		LaraItem->Position.zRot = 0;
		Lara.Vehicle = NO_ITEM;

		roomNum = LaraItem->RoomNumber;
		x = LaraItem->Position.xPos + 360 * phd_sin(LaraItem->Position.yRot);
		y = LaraItem->Position.yPos - 90;
		z = LaraItem->Position.zPos + 360 * phd_cos(LaraItem->Position.yRot);
		floor = GetFloor(x, y, z, &roomNum);
		if (GetFloorHeight(floor, x, y, z) >= y - 256)
		{
			LaraItem->Position.xPos = x;
			LaraItem->Position.zPos = z;
			if (roomNum != LaraItem->RoomNumber)
				ItemNewRoom(Lara.itemNumber, roomNum);
		}
		LaraItem->Position.yPos = y;

		boat->AnimNumber = Objects[ID_RUBBER_BOAT].animIndex;
		boat->FrameNumber = g_Level.Anims[boat->AnimNumber].frameBase;
	}
}

void RubberBoatControl(short itemNum)
{
	ITEM_INFO* boat;
	RUBBER_BOAT_INFO* binfo;
	PHD_VECTOR fl, fr, prop;
	int hfl, hfr, no_turn = 1, drive = 0;
	FLOOR_INFO* floor;
	int height, collide, water, ceiling, pitch, h, ofs, nowake;
	short roomNumber, x_rot, z_rot;

	boat = &g_Level.Items[itemNum];
	binfo = (RUBBER_BOAT_INFO*)boat->Data;
	collide = RubberBoatDynamics(itemNum);
	hfl = TestWaterHeight(boat, RBOAT_FRONT, -RBOAT_SIDE, &fl);
	hfr = TestWaterHeight(boat, RBOAT_FRONT, RBOAT_SIDE, &fr);

	roomNumber = boat->RoomNumber;
	floor = GetFloor(boat->Position.xPos, boat->Position.yPos, boat->Position.zPos, &roomNumber);
	height = GetFloorHeight(floor, boat->Position.xPos, boat->Position.yPos, boat->Position.zPos);
	ceiling = GetCeiling(floor, boat->Position.xPos, boat->Position.yPos, boat->Position.zPos);

	if (Lara.Vehicle == itemNum)
	{
		TestTriggers(boat, false);
		TestTriggers(boat, true);
	}

	binfo->water = water = GetWaterHeight(boat->Position.xPos, boat->Position.yPos, boat->Position.zPos, roomNumber);

	if (Lara.Vehicle == itemNum && LaraItem->HitPoints > 0)
	{
		switch (LaraItem->ActiveState)
		{
		case RBOAT_STATE_MOUNT:
		case RBOAT_STATE_JUMP_RIGHT:
		case RBOAT_STATE_JUMP_LEFT:
			break;

		default:
			drive = 1;
			no_turn = RubberBoatUserControl(boat);
			break;
		}
	}
	else
	{
		if (boat->Velocity > RBOAT_SLOWDOWN)
			boat->Velocity -= RBOAT_SLOWDOWN;
		else
			boat->Velocity = 0;
	}

	if (no_turn)
	{
		if (binfo->boatTurn < -RBOAT_UNDO_TURN)
			binfo->boatTurn += RBOAT_UNDO_TURN;
		else if (binfo->boatTurn > RBOAT_UNDO_TURN)
			binfo->boatTurn -= RBOAT_UNDO_TURN;
		else
			binfo->boatTurn = 0;
	}

	boat->Floor = height - 5;
	if (binfo->water == NO_HEIGHT)
		binfo->water = height;
	else
		binfo->water -= 5;

	binfo->leftFallspeed = DoRubberBoatDynamics(hfl, binfo->leftFallspeed, (int*)&fl.y);
	binfo->rightFallspeed = DoRubberBoatDynamics(hfr, binfo->rightFallspeed, (int*)&fr.y);
	ofs = boat->VerticalVelocity;
	boat->VerticalVelocity = DoRubberBoatDynamics(binfo->water, boat->VerticalVelocity, (int*)&boat->Position.yPos);

	height = (fl.y + fr.y);
	if (height < 0)
		height = -(abs(height) / 2);
	else
		height = height / 2;

	x_rot = phd_atan(RBOAT_FRONT, boat->Position.yPos - height);
	z_rot = phd_atan(RBOAT_SIDE, height - fl.y);

	boat->Position.xRot += ((x_rot - boat->Position.xRot) / 2);
	boat->Position.zRot += ((z_rot - boat->Position.zRot) / 2);

	if (!x_rot && abs(boat->Position.xRot) < 4)
		boat->Position.xRot = 0;
	if (!z_rot && abs(boat->Position.zRot) < 4)
		boat->Position.zRot = 0;

	if (Lara.Vehicle == itemNum)
	{
		RubberBoatAnimation(boat, collide);

		if (roomNumber != boat->RoomNumber)
		{
			ItemNewRoom(itemNum, roomNumber);
			ItemNewRoom(Lara.itemNumber, roomNumber);
		}

		boat->Position.zRot += binfo->tiltAngle;
		LaraItem->Position.xPos = boat->Position.xPos;
		LaraItem->Position.xRot = boat->Position.xRot;
		LaraItem->Position.yPos = boat->Position.yPos;
		LaraItem->Position.yRot = boat->Position.yRot;
		LaraItem->Position.zPos = boat->Position.zPos;
		LaraItem->Position.zRot = boat->Position.zRot;

		AnimateItem(LaraItem);

		if (LaraItem->HitPoints > 0)
		{
			boat->AnimNumber = Objects[ID_RUBBER_BOAT].animIndex + (LaraItem->AnimNumber - Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex);
			boat->FrameNumber = g_Level.Anims[boat->AnimNumber].frameBase + (LaraItem->FrameNumber - g_Level.Anims[LaraItem->AnimNumber].frameBase);
		}

		Camera.targetElevation = -ANGLE(20);
		Camera.targetDistance = 2048;
	}
	else
	{
		if (roomNumber != boat->RoomNumber)
			ItemNewRoom(itemNum, roomNumber);
		boat->Position.zRot += binfo->tiltAngle;
	}

	pitch = boat->Velocity;
	binfo->pitch += ((pitch - binfo->pitch) / 4);

	if (boat->Velocity > 8)
		SoundEffect(SFX_TR3_BOAT_MOVING, &boat->Position, 0, 0.5f + (float)abs(binfo->pitch) / (float)RBOAT_MAX_SPEED);
	else if (drive)
		SoundEffect(SFX_TR3_BOAT_IDLE, &boat->Position, 0, 0.5f + (float)abs(binfo->pitch) / (float)RBOAT_MAX_SPEED);

	if (Lara.Vehicle != itemNum)
		return;

	RubberBoatDoGetOff(boat);

	roomNumber = boat->RoomNumber;
	floor = GetFloor(boat->Position.xPos, boat->Position.yPos + 128, boat->Position.zPos, &roomNumber);
	h = GetWaterHeight(boat->Position.xPos, boat->Position.yPos + 128, boat->Position.zPos, roomNumber);

	if (h > boat->Position.yPos + 32 || h == NO_HEIGHT)
		h = 0;
	else
		h = 1;

	prop.x = 0;
	prop.y = 0;
	prop.z = -80;
	GetJointAbsPosition(boat, &prop, 2);
	roomNumber = boat->RoomNumber;
	floor = GetFloor(prop.x, prop.y, prop.z, &roomNumber);

	if (boat->Velocity && h < prop.y && h != NO_HEIGHT)
	{
		TriggerRubberBoatMist(prop.x, prop.y, prop.z, abs(boat->Velocity), boat->Position.yRot + 0x8000, 0);
		if ((GetRandomControl() & 1) == 0)
		{
			PHD_3DPOS pos;
			short roomNum = boat->RoomNumber;

			pos.xPos = prop.x + (GetRandomControl() & 63) - 32;
			pos.yPos = prop.y + (GetRandomControl() & 15);
			pos.zPos = prop.z + (GetRandomControl() & 63) - 32;
			GetFloor(pos.xPos, pos.yPos, pos.zPos, &roomNum);
			CreateBubble((PHD_VECTOR*)&pos, roomNum, 16, 8, 0, 0, 0, 0);
		}
	}
	else
	{
		GAME_VECTOR pos;
		long cnt;
		h = GetFloorHeight(floor, prop.x, prop.y, prop.z);
		if (prop.y > h && !TestEnvironment(ENV_FLAG_WATER, roomNumber))
		{
			pos.x = prop.x;
			pos.y = prop.y;
			pos.z = prop.z;

			cnt = (GetRandomControl() & 3) + 3;
			for (;cnt>0;cnt--)
			TriggerRubberBoatMist(prop.x, prop.y, prop.z, ((GetRandomControl() & 15) + 96) * 16, boat->Position.yRot + 0x4000 + GetRandomControl(), 1);

		}
	}
}
