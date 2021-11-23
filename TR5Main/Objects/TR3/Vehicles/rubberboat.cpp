#include "framework.h"
#include "rubberboat.h"
#include "items.h"
#include "level.h"
#include "collide.h"
#include "lara.h"
#include "input.h"
#include "sphere.h"
#include "Sound/sound.h"
#include "effects/bubble.h"
#include "animation.h"
#include "camera.h"
#include "setup.h"
#include "rubberboat_info.h"

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

#define RBOAT_IN_SPEED		(IN_SPRINT | IN_DUCK)
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
	rBoat->data = RUBBER_BOAT_INFO();
	auto rBoatInfo = (RUBBER_BOAT_INFO*)rBoat->data;

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
		Lara.gunStatus != LG_NO_ARMS ||
		lara->gravityStatus ||
		rBoat->speed)
	{
		return RBOAT_MOUNT_NONE;
	}

	short xDelta = lara->pos.xPos - rBoat->pos.xPos;
	short zDelta = lara->pos.zPos - rBoat->pos.zPos;
	int dist = zDelta * phd_cos(-rBoat->pos.yRot) - xDelta * phd_sin(-rBoat->pos.yRot);

	if (dist > 512)
		return RBOAT_MOUNT_NONE;

	short rot = rBoat->pos.yRot - lara->pos.yRot;
	if (Lara.waterStatus == LW_SURFACE || Lara.waterStatus == LW_WADE)
	{
		if (rot > ANGLE(45.0f) && rot < ANGLE(135.0f))
			getOn = RBOAT_MOUNT_LEFT;
		if (rot < -ANGLE(135.0f) && rot > -ANGLE(45.0f))
			getOn = RBOAT_MOUNT_RIGHT;
	}
	else if (Lara.waterStatus == LW_ABOVE_WATER)
	{
		if (lara->fallspeed > 0)
		{
			if ((lara->pos.yPos + (STEP_SIZE * 2)) > rBoat->pos.yPos)
				getOn = RBOAT_MOUNT_JUMP;
		}
		else if (lara->fallspeed == 0)
		{
			if (rot > -ANGLE(135.0f) && rot < ANGLE(135.0f))
			{
				if (lara->pos.xPos == rBoat->pos.xPos &&
					lara->pos.yPos == rBoat->pos.xPos &&
					lara->pos.zPos == rBoat->pos.zPos)
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

	pos->y = item->pos.yPos - zOff * phd_sin(item->pos.xRot) + xOff * phd_sin(item->pos.zRot);

	c = phd_cos(item->pos.yRot);
	s = phd_sin(item->pos.yRot);

	pos->z = item->pos.zPos + zOff * c - xOff * s;
	pos->x = item->pos.xPos + zOff * s + xOff * c;

	roomNum = item->roomNumber;
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

	itemNum = g_Level.Rooms[boat->roomNumber].itemNumber;
	while (itemNum != NO_ITEM)
	{

		item = &g_Level.Items[itemNum];

		if (item->objectNumber == ID_RUBBER_BOAT && itemNum != boatNum && Lara.Vehicle != itemNum)
		{
			x = item->pos.xPos - boat->pos.xPos;
			z = item->pos.zPos - boat->pos.zPos;
			distance = SQUARE(x) + SQUARE(z);
			if (distance < 1000000)
			{
				boat->pos.xPos = item->pos.xPos - x * 1000000 / distance;
				boat->pos.zPos = item->pos.zPos - z * 1000000 / distance;
			}
			return;
		}
		itemNum = item->nextItem;
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
			skidoo->pos.zPos += (old->z - pos->z);
			skidoo->pos.xPos += (old->x - pos->x);
		}
		else if (z > z_old)
		{
			skidoo->pos.zPos -= shift_z + 1;
			return (pos->x - skidoo->pos.xPos);
		}
		else
		{
			skidoo->pos.zPos += WALL_SIZE - shift_z;
			return (skidoo->pos.xPos - pos->x);
		}
	}
	else if (z == z_old)
	{
		if (x > x_old)
		{
			skidoo->pos.xPos -= shift_x + 1;
			return (skidoo->pos.zPos - pos->z);
		}
		else
		{
			skidoo->pos.xPos += WALL_SIZE - shift_x;
			return (pos->z - skidoo->pos.zPos);
		}
	}
	else
	{
		short roomNumber;
		FLOOR_INFO* floor;
		int height;

		x = z = 0;

		roomNumber = skidoo->roomNumber;
		floor = GetFloor(old->x, pos->y, pos->z, &roomNumber);
		height = GetFloorHeight(floor, old->x, pos->y, pos->z);
		if (height < old->y - STEP_SIZE)
		{
			if (pos->z > old->z)
				z = -shift_z - 1;
			else
				z = WALL_SIZE - shift_z;
		}

		roomNumber = skidoo->roomNumber;
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
			skidoo->pos.zPos += z;
			skidoo->pos.xPos += x;
		}
		else if (z)
		{
			skidoo->pos.zPos += z;
			if (z > 0)
				return (skidoo->pos.xPos - pos->x);
			else
				return (pos->x - skidoo->pos.xPos);
		}
		else if (x)
		{
			skidoo->pos.xPos += x;
			if (x > 0)
				return (pos->z - skidoo->pos.zPos);
			else
				return (skidoo->pos.zPos - pos->z);
		}
		else
		{
			skidoo->pos.zPos += (old->z - pos->z);
			skidoo->pos.xPos += (old->x - pos->x);
		}
	}

	return 0;
}

static int GetRubberBoatCollisionAnim(ITEM_INFO *rubber, PHD_VECTOR *moved)
{
	long front, side;
	float c, s;

	moved->x = rubber->pos.xPos - moved->x;
	moved->z = rubber->pos.zPos - moved->z;

	if (moved->x || moved->z)
	{
		c = phd_cos(rubber->pos.yRot);
		s = phd_sin(rubber->pos.yRot);
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
	binfo = (RUBBER_BOAT_INFO*)boat->data;

	boat->pos.zRot -= binfo->tiltAngle;

	hfl_old = TestWaterHeight(boat, RBOAT_FRONT, -RBOAT_SIDE, &fl_old);
	hfr_old = TestWaterHeight(boat, RBOAT_FRONT, RBOAT_SIDE, &fr_old);
	hbl_old = TestWaterHeight(boat, -RBOAT_FRONT, -RBOAT_SIDE, &bl_old);
	hbr_old = TestWaterHeight(boat, -RBOAT_FRONT, RBOAT_SIDE, &br_old);
	hf_old = TestWaterHeight(boat, 1000, 0, &f_old);
	old.x = boat->pos.xPos;
	old.y = boat->pos.yPos;
	old.z = boat->pos.zPos;

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

	boat->pos.yRot += binfo->boatTurn + binfo->extraRotation;
	binfo->tiltAngle = binfo->boatTurn * 6;

	boat->pos.zPos += boat->speed * phd_cos(boat->pos.yRot);
	boat->pos.xPos += boat->speed * phd_sin(boat->pos.yRot);
	if (boat->speed >= 0)
		binfo->propRot += (boat->speed * (ONE_DEGREE * 3)) + (ONE_DEGREE * 2);
	else
		binfo->propRot += ONE_DEGREE * 33;

	slip = RBOAT_SIDE_SLIP * phd_sin(boat->pos.zRot);
	if (!slip && boat->pos.zRot)
		slip = (boat->pos.zRot > 0) ? 1 : -1;
	boat->pos.zPos -= slip * phd_sin(boat->pos.yRot);
	boat->pos.xPos += slip * phd_cos(boat->pos.yRot);

	slip = RBOAT_SLIP * phd_sin(boat->pos.xRot);
	if (!slip && boat->pos.xRot)
		slip = (boat->pos.xRot > 0) ? 1 : -1;
	boat->pos.zPos -= slip * phd_cos(boat->pos.yRot);
	boat->pos.xPos -= slip * phd_sin(boat->pos.yRot);

	moved.x = boat->pos.xPos;
	moved.z = boat->pos.zPos;

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

	room_number = boat->roomNumber;
	floor = GetFloor(boat->pos.xPos, boat->pos.yPos, boat->pos.zPos, &room_number);
	height = GetWaterHeight(boat->pos.xPos, boat->pos.yPos, boat->pos.zPos, room_number);
	if (height == NO_HEIGHT)
		height = GetFloorHeight(floor, boat->pos.xPos, boat->pos.yPos, boat->pos.zPos);
	if (height < boat->pos.yPos - STEP_SIZE / 2)
		DoRubberBoatShift2(boat, (PHD_VECTOR*)&boat->pos, &old);

	binfo->extraRotation = rot;

	collide = GetRubberBoatCollisionAnim(boat, &moved);

	if (slip || collide)
	{
		newspeed = (boat->pos.zPos - old.z) * phd_cos(boat->pos.yRot) + (boat->pos.xPos - old.x) * phd_sin(boat->pos.yRot);

		if (Lara.Vehicle == boat_number && boat->speed > RBOAT_MAX_SPEED + RBOAT_ACCELERATION && newspeed < boat->speed - 10)
		{
			LaraItem->hitPoints -= boat->speed;
			LaraItem->hitStatus = 1;
			SoundEffect(SFX_TR4_LARA_INJURY, &LaraItem->pos, 0);
			newspeed /= 2;
			boat->speed /= 2;
		}

		if (slip)
		{
			if (boat->speed <= RBOAT_MAX_SPEED + 10)
				boat->speed = newspeed;
		}
		else
		{
			if (boat->speed > 0 && newspeed < boat->speed)
				boat->speed = newspeed;
			else if (boat->speed < 0 && newspeed > boat->speed)
				boat->speed = newspeed;
		}

		if (boat->speed < RBOAT_MAX_BACK)
			boat->speed = RBOAT_MAX_BACK;
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

	binfo = (RUBBER_BOAT_INFO*)boat->data;

	if (boat->pos.yPos >= binfo->water - 128 && binfo->water != NO_HEIGHT)
	{
		if (!(TrInput & RBOAT_IN_DISMOUNT) && !(TrInput & IN_LOOK) || boat->speed)
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
				if (boat->speed > 0)
					boat->speed -= 5;
				else if (boat->speed > -20)
					boat->speed += -2;
			}
			else if (TrInput & RBOAT_IN_FORWARD)
			{
				if (TrInput & RBOAT_IN_SPEED)
					maxSpeed = 185;
				else
					maxSpeed = (TrInput & RBOAT_IN_SLOW) ? 37 : 110;

				if (boat->speed < maxSpeed)
					boat->speed += 3 + (5 * boat->speed) / (2 * maxSpeed);
				else if (boat->speed > maxSpeed + 1)
					boat->speed -= 1;

			}
			else if (boat->speed >= 0 && boat->speed < 20 && TrInput & (RBOAT_IN_LEFT | RBOAT_IN_RIGHT))
			{
				if (boat->speed == 0 && !(TrInput & RBOAT_IN_DISMOUNT))
					boat->speed = 20;
			}
			else if (boat->speed > 1)
				boat->speed -= 1;
			else
				boat->speed = 0;
		}
		else
		{
			if (boat->speed >= 0 && boat->speed < 20 && TrInput & (RBOAT_IN_LEFT | RBOAT_IN_RIGHT))
			{
				if (boat->speed == 0 && !(TrInput & RBOAT_IN_DISMOUNT))
					boat->speed = 20;
			}
			else if (boat->speed > 1)
				boat->speed -= 1;
			else
				boat->speed = 0;

			if (TrInput & IN_LOOK && boat->speed == 0)
				LookUpDown();
		}
	}
	return noTurn;
}

void RubberBoatCollision(short itemNum, ITEM_INFO *lara, COLL_INFO *coll)
{
	int getOn;
	ITEM_INFO *item;

	if (lara->hitPoints <= 0 || Lara.Vehicle != NO_ITEM)
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
		lara->animNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_MOUNT_RIGHT;
	else if (getOn == 2)
		lara->animNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_MOUNT_LEFT;
	else if (getOn == 3)
		lara->animNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_MOUNT_JUMP;
	else
		lara->animNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_IDLE;

	Lara.waterStatus = LW_ABOVE_WATER;
	lara->pos.xPos = item->pos.xPos;
	lara->pos.yPos = item->pos.yPos - 5;
	lara->pos.zPos = item->pos.zPos;
	lara->pos.xRot = 0;
	lara->pos.yRot = item->pos.yRot;
	lara->pos.zRot = 0;
	lara->gravityStatus = false;
	lara->fallspeed = 0;
	lara->speed = 0;
	lara->frameNumber = g_Level.Anims[lara->animNumber].frameBase;
	lara->currentAnimState = 0;
	lara->goalAnimState = 0;

	if (lara->roomNumber != item->roomNumber)
		ItemNewRoom(Lara.itemNumber, item->roomNumber);

	AnimateItem(lara);

	if (g_Level.Items[itemNum].status != ITEM_ACTIVE)
	{
		AddActiveItem(itemNum);
		g_Level.Items[itemNum].status = ITEM_ACTIVE;
	}
}

static int CanGetOffRubberBoat(int direction)
{
	short angle;
	auto boat = &g_Level.Items[Lara.Vehicle];

	if (direction < 0)
		angle = boat->pos.yRot - ANGLE(90);
	else
		angle = boat->pos.yRot + ANGLE(90);

	auto x = boat->pos.xPos + 1024 * phd_sin(angle);
	auto y = boat->pos.yPos;
	auto z = boat->pos.zPos + 1024 * phd_cos(angle);

	auto collResult = GetCollisionResult(x, y, z, boat->roomNumber);

	if (collResult.Position.Floor - boat->pos.yPos < -512)
		return 0;

	if (collResult.Position.Slope || collResult.Position.Floor == NO_HEIGHT)
		return 0;

	if ((collResult.Position.Ceiling - boat->pos.yPos > -LARA_HEIGHT) || (collResult.Position.Floor - collResult.Position.Ceiling < LARA_HEIGHT))
		return 0;

	return 1;
}

void RubberBoatAnimation(ITEM_INFO *boat, int collide)
{
	RUBBER_BOAT_INFO *binfo;
	binfo = (RUBBER_BOAT_INFO*)boat->data;

	if (LaraItem->hitPoints <= 0)
	{
		if (LaraItem->currentAnimState!= RBOAT_STATE_DEATH)
		{
			LaraItem->animNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_IDLE_DEATH;
			LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
			LaraItem->goalAnimState = RBOAT_STATE_DEATH;
			LaraItem->currentAnimState = RBOAT_STATE_DEATH;
		}
	}
	else if (boat->pos.yPos < binfo->water - 128 && boat->fallspeed > 0)
	{
		if (LaraItem->currentAnimState != RBOAT_STATE_FALL)
		{
			LaraItem->animNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + RBOAT_ANIM_LEAP_START;
			LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
			LaraItem->goalAnimState = RBOAT_STATE_FALL;
			LaraItem->currentAnimState = RBOAT_STATE_FALL;
		}
	}
	else if (collide)
	{
		if (LaraItem->currentAnimState != RBOAT_STATE_HIT)
		{
			LaraItem->animNumber = (short)(Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + collide);
			LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
			LaraItem->goalAnimState = RBOAT_STATE_HIT;
			LaraItem->currentAnimState = RBOAT_STATE_HIT;
		}
	}
	else
	{
		switch (LaraItem->currentAnimState)
		{
		case RBOAT_STATE_IDLE:
			if (TrInput & IN_ROLL)
			{
				if (boat->speed == 0)
				{
					if ((TrInput & IN_RIGHT) && CanGetOffRubberBoat(boat->pos.yRot + ANGLE(90)))
						LaraItem->goalAnimState = RBOAT_STATE_JUMP_RIGHT;
					else if ((TrInput & IN_LEFT) && CanGetOffRubberBoat(boat->pos.yRot - ANGLE(90)))
						LaraItem->goalAnimState = RBOAT_STATE_JUMP_LEFT;
				}
			}

			if (boat->speed > 0)
				LaraItem->goalAnimState = RBOAT_STATE_MOVING;

			break;
		case RBOAT_STATE_MOVING:
			if (boat->speed <= 0)
				LaraItem->goalAnimState = RBOAT_STATE_IDLE;
			if (TrInput & IN_RIGHT)
				LaraItem->goalAnimState = RBOAT_STATE_TURN_RIGHT;
			else if (TrInput & IN_LEFT)
				LaraItem->goalAnimState = RBOAT_STATE_TURN_LEFT;
			
			break;
		case RBOAT_STATE_FALL:
			LaraItem->goalAnimState = RBOAT_STATE_MOVING;
			break;
		case RBOAT_STATE_TURN_RIGHT:
			if (boat->speed <= 0)
				LaraItem->goalAnimState = RBOAT_STATE_IDLE;
			else if (!(TrInput & IN_RIGHT))
				LaraItem->goalAnimState = RBOAT_STATE_MOVING;
			break;
		case RBOAT_STATE_TURN_LEFT:
			if (boat->speed <= 0)
				LaraItem->goalAnimState = RBOAT_STATE_IDLE;
			else if (!(TrInput & IN_LEFT))
				LaraItem->goalAnimState = RBOAT_STATE_MOVING;
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
	if ((LaraItem->currentAnimState == RBOAT_STATE_JUMP_RIGHT || LaraItem->currentAnimState == RBOAT_STATE_JUMP_LEFT) 
		&& LaraItem->frameNumber == g_Level.Anims[LaraItem->animNumber].frameEnd)
	{
		short roomNum;
		int x, y, z;
		FLOOR_INFO *floor;

		if (LaraItem->currentAnimState == 4)
			LaraItem->pos.yRot -= ANGLE(90);
		else
			LaraItem->pos.yRot += ANGLE(90);

		LaraItem->animNumber = LA_JUMP_FORWARD;
		LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
		LaraItem->goalAnimState = LS_JUMP_FORWARD;
		LaraItem->currentAnimState = LS_JUMP_FORWARD;
		LaraItem->gravityStatus = true;
		LaraItem->fallspeed = -40;
		LaraItem->speed = 20;
		LaraItem->pos.xRot = 0;
		LaraItem->pos.zRot = 0;
		Lara.Vehicle = NO_ITEM;

		roomNum = LaraItem->roomNumber;
		x = LaraItem->pos.xPos + 360 * phd_sin(LaraItem->pos.yRot);
		y = LaraItem->pos.yPos - 90;
		z = LaraItem->pos.zPos + 360 * phd_cos(LaraItem->pos.yRot);
		floor = GetFloor(x, y, z, &roomNum);
		if (GetFloorHeight(floor, x, y, z) >= y - 256)
		{
			LaraItem->pos.xPos = x;
			LaraItem->pos.zPos = z;
			if (roomNum != LaraItem->roomNumber)
				ItemNewRoom(Lara.itemNumber, roomNum);
		}
		LaraItem->pos.yPos = y;

		boat->animNumber = Objects[ID_RUBBER_BOAT].animIndex;
		boat->frameNumber = g_Level.Anims[boat->animNumber].frameBase;
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
	binfo = (RUBBER_BOAT_INFO*)boat->data;
	collide = RubberBoatDynamics(itemNum);
	hfl = TestWaterHeight(boat, RBOAT_FRONT, -RBOAT_SIDE, &fl);
	hfr = TestWaterHeight(boat, RBOAT_FRONT, RBOAT_SIDE, &fr);

	roomNumber = boat->roomNumber;
	floor = GetFloor(boat->pos.xPos, boat->pos.yPos, boat->pos.zPos, &roomNumber);
	height = GetFloorHeight(floor, boat->pos.xPos, boat->pos.yPos, boat->pos.zPos);
	ceiling = GetCeiling(floor, boat->pos.xPos, boat->pos.yPos, boat->pos.zPos);

	if (Lara.Vehicle == itemNum)
	{
		TestTriggers(boat, false);
		TestTriggers(boat, true);
	}

	binfo->water = water = GetWaterHeight(boat->pos.xPos, boat->pos.yPos, boat->pos.zPos, roomNumber);

	if (Lara.Vehicle == itemNum && LaraItem->hitPoints > 0)
	{
		switch (LaraItem->currentAnimState)
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
		if (boat->speed > RBOAT_SLOWDOWN)
			boat->speed -= RBOAT_SLOWDOWN;
		else
			boat->speed = 0;
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

	boat->floor = height - 5;
	if (binfo->water == NO_HEIGHT)
		binfo->water = height;
	else
		binfo->water -= 5;

	binfo->leftFallspeed = DoRubberBoatDynamics(hfl, binfo->leftFallspeed, (int*)&fl.y);
	binfo->rightFallspeed = DoRubberBoatDynamics(hfr, binfo->rightFallspeed, (int*)&fr.y);
	ofs = boat->fallspeed;
	boat->fallspeed = DoRubberBoatDynamics(binfo->water, boat->fallspeed, (int*)&boat->pos.yPos);

	height = (fl.y + fr.y);
	if (height < 0)
		height = -(abs(height) / 2);
	else
		height = height / 2;

	x_rot = phd_atan(RBOAT_FRONT, boat->pos.yPos - height);
	z_rot = phd_atan(RBOAT_SIDE, height - fl.y);

	boat->pos.xRot += ((x_rot - boat->pos.xRot) / 2);
	boat->pos.zRot += ((z_rot - boat->pos.zRot) / 2);

	if (!x_rot && abs(boat->pos.xRot) < 4)
		boat->pos.xRot = 0;
	if (!z_rot && abs(boat->pos.zRot) < 4)
		boat->pos.zRot = 0;

	if (Lara.Vehicle == itemNum)
	{
		RubberBoatAnimation(boat, collide);

		if (roomNumber != boat->roomNumber)
		{
			ItemNewRoom(itemNum, roomNumber);
			ItemNewRoom(Lara.itemNumber, roomNumber);
		}

		boat->pos.zRot += binfo->tiltAngle;
		LaraItem->pos.xPos = boat->pos.xPos;
		LaraItem->pos.xRot = boat->pos.xRot;
		LaraItem->pos.yPos = boat->pos.yPos;
		LaraItem->pos.yRot = boat->pos.yRot;
		LaraItem->pos.zPos = boat->pos.zPos;
		LaraItem->pos.zRot = boat->pos.zRot;

		AnimateItem(LaraItem);

		if (LaraItem->hitPoints > 0)
		{
			boat->animNumber = Objects[ID_RUBBER_BOAT].animIndex + (LaraItem->animNumber - Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex);
			boat->frameNumber = g_Level.Anims[boat->animNumber].frameBase + (LaraItem->frameNumber - g_Level.Anims[LaraItem->animNumber].frameBase);
		}

		Camera.targetElevation = -ANGLE(20);
		Camera.targetDistance = 2048;
	}
	else
	{
		if (roomNumber != boat->roomNumber)
			ItemNewRoom(itemNum, roomNumber);
		boat->pos.zRot += binfo->tiltAngle;
	}

	pitch = boat->speed;
	binfo->pitch += ((pitch - binfo->pitch) / 4);

	if (boat->speed > 8)
		SoundEffect(SFX_TR3_BOAT_MOVING, &boat->pos, 0, 0.5f + (float)abs(binfo->pitch) / (float)RBOAT_MAX_SPEED);
	else if (drive)
		SoundEffect(SFX_TR3_BOAT_IDLE, &boat->pos, 0, 0.5f + (float)abs(binfo->pitch) / (float)RBOAT_MAX_SPEED);

	if (Lara.Vehicle != itemNum)
		return;

	RubberBoatDoGetOff(boat);

	roomNumber = boat->roomNumber;
	floor = GetFloor(boat->pos.xPos, boat->pos.yPos + 128, boat->pos.zPos, &roomNumber);
	h = GetWaterHeight(boat->pos.xPos, boat->pos.yPos + 128, boat->pos.zPos, roomNumber);

	if (h > boat->pos.yPos + 32 || h == NO_HEIGHT)
		h = 0;
	else
		h = 1;

	prop.x = 0;
	prop.y = 0;
	prop.z = -80;
	GetJointAbsPosition(boat, &prop, 2);
	roomNumber = boat->roomNumber;
	floor = GetFloor(prop.x, prop.y, prop.z, &roomNumber);

	if (boat->speed && h < prop.y && h != NO_HEIGHT)
	{
		TriggerRubberBoatMist(prop.x, prop.y, prop.z, abs(boat->speed), boat->pos.yRot + 0x8000, 0);
		if ((GetRandomControl() & 1) == 0)
		{
			PHD_3DPOS pos;
			short roomNum = boat->roomNumber;

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
		if (prop.y > h && !(g_Level.Rooms[roomNumber].flags & ENV_FLAG_WATER))
		{
			pos.x = prop.x;
			pos.y = prop.y;
			pos.z = prop.z;

			cnt = (GetRandomControl() & 3) + 3;
			for (;cnt>0;cnt--)
			TriggerRubberBoatMist(prop.x, prop.y, prop.z, ((GetRandomControl() & 15) + 96) * 16, boat->pos.yRot + 0x4000 + GetRandomControl(), 1);

		}
	}
}
