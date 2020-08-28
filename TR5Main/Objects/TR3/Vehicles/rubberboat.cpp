#include "framework.h"
//#include "rubberboat.h" - was giving me bs errors idk why
#include "items.h"
#include "level.h"
#include "collide.h"
#include "lara.h"
#include "input.h"
#include "sphere.h"
#include "sound.h"
#include "bubble.h"
#include "draw.h"

typedef struct {
	int boatTurn;
	int leftFallspeed;
	int rightFallspeed;
	short tiltAngle;
	short extraRotation;
	int water;
	int pitch;
	short propRot;
}RUBBER_BOAT_INFO;

void DrawRubberBoat(ITEM_INFO *item)
{
	RUBBER_BOAT_INFO *b;

	b = (RUBBER_BOAT_INFO*)item->data;
	item->data = &b->propRot;
	DrawAnimatingItem(item);
	item->data = (void *)b;
}

int RubberBoatCheckGeton(short itemNum, COLL_INFO *coll)
{
	int getOn (0), dist;
	short rot;
	ITEM_INFO *boat;
	short xDelta, zDelta;
	boat = &g_Level.Items[itemNum];
	if (Lara.gunStatus != LG_NO_ARMS)
		return 0;

	if (!TestBoundsCollide(boat, LaraItem, coll->radius))
		return 0;

	if (!TestCollision(boat, LaraItem))
		return 0;


	xDelta = LaraItem->pos.xPos - boat->pos.xPos;
	zDelta = LaraItem->pos.zPos - boat->pos.zPos;
	dist = ((zDelta * phd_cos(-boat->pos.yRot)) - (xDelta * phd_sin(-boat->pos.yRot))) >> W2V_SHIFT;

	if (dist > 512) return 0;

	rot = boat->pos.yRot - LaraItem->pos.yRot;

	if (Lara.waterStatus == LW_SURFACE || Lara.waterStatus == LW_WADE)
	{
		if (!(TrInput & IN_ACTION) || LaraItem->gravityStatus || boat->speed)
			return 0;

		if (rot > ANGLE(45) && rot < ANGLE(135))
			getOn = 1;
		if (rot < ANGLE(135) && rot > ANGLE(45))
			getOn = 2;
	}
	else if (Lara.waterStatus == LW_ABOVE_WATER)
	{
		if (LaraItem->fallspeed > 0)
		{
			if (LaraItem->pos.yPos + 512 > boat->pos.yPos)
				getOn = 3;
		}
		else if (LaraItem->fallspeed == 0)
		{
			if (rot > -ANGLE(135) && rot < ANGLE(135))
			{
				if (LaraItem->pos.xPos == boat->pos.xPos &&
					LaraItem->pos.yPos == boat->pos.xPos &&
					LaraItem->pos.zPos == boat->pos.zPos)
					getOn = 4;
				else
					getOn = 3;
			}
		}
	}

	if (!getOn)
		return 0;

	return getOn;
}

static long TestWaterHeight(ITEM_INFO *item, long zOff, long xOff, PHD_VECTOR *pos)
{
	FLOOR_INFO *floor;
	long s, c, height;
	short roomNum;

	pos->y = item->pos.yPos - (zOff * phd_sin(item->pos.xRot) >> W2V_SHIFT) +
		(xOff * phd_sin(item->pos.zRot) >> W2V_SHIFT);

	c = phd_cos(item->pos.yRot);
	s = phd_sin(item->pos.yRot);

	pos->z = item->pos.zPos + ((zOff *c - xOff * s) >> W2V_SHIFT);
	pos->x = item->pos.xPos + ((zOff *s + xOff * c) >> W2V_SHIFT);

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

static int DoRubberBoatShift2(ITEM_INFO *rubber, PHD_VECTOR *pos, PHD_VECTOR *old)
{
	int x, z, xOld, zOld, shiftX, shiftZ;
	short roomNum, height;
	FLOOR_INFO *floor;


	x = pos->x >> WALL_SHIFT;
	z = pos->z >> WALL_SHIFT;

	xOld = old->x >> WALL_SHIFT;
	zOld = old->z >> WALL_SHIFT;

	shiftX = pos->x & (1023);
	shiftZ = pos->z & (1023);

	if (x == xOld)
	{
		if (z == zOld)
		{
			rubber->pos.zPos += (old->z - pos->z);
			rubber->pos.xPos += (old->x - pos->x);	
		}
		else if (z > zOld)
		{
			rubber->pos.zPos -= shiftZ + 1;
			return (pos->x - rubber->pos.xPos);
		}
		else
		{
			rubber->pos.zPos += 1024 - shiftZ;
			return (rubber->pos.xPos - pos->x);
		}
	}
	else if (z == zOld)
	{
		if (x > xOld)
		{
			rubber->pos.xPos += 1024 - shiftX;
			return (pos->z - rubber->pos.zPos);
		}
		else
		{
			rubber->pos.xPos += 1024 - shiftX;
			return (pos->z - rubber->pos.zPos);
		}
	}
	else
	{
		x = 0;
		z = 0;
		roomNum = rubber->roomNumber;
		floor = GetFloor(old->x, old->y, old->z, &roomNum);
		height = GetFloorHeight(floor, old->x, old->y, old->z);
		if (height < old->y - 256)
		{
			if (pos->z > old->z)
				z = -shiftZ - 1;
			else
				z = 1024 - shiftZ;
		}

		roomNum = rubber->roomNumber;
		floor = GetFloor(pos->x, pos->y, old->z, &roomNum);
		height = GetFloorHeight(floor, pos->x, pos->y, old->z);
		if (height < old->y - 256)
		{
			if (pos->x > old->x)
				x = -shiftX - 1;
			else
				x = 1024 - shiftX;
		}

		if (x && z)
		{
			rubber->pos.zPos += z;
			rubber->pos.xPos += x;
		}
		else if (z)
		{
			rubber->pos.zPos += z;
			if (z > 0)
				return (rubber->pos.xPos - pos->x);
			else
				return (pos->x - rubber->pos.xPos);
		}
		else if (x)
		{
			rubber->pos.xPos += x;
			if (x > 0)
				return (pos->z - rubber->pos.zPos);
			else
				return (rubber->pos.zPos - pos->z);
		}
		else
		{
			rubber->pos.zPos += (old->z - pos->z);
			rubber->pos.xPos += (old->x - pos->z);
		}
	}
	return 0;
}

static int GetRubberBoatCollisionAnim(ITEM_INFO *rubber, PHD_VECTOR *moved)
{
	long c, s, front, side;

	moved->x = rubber->pos.xPos - moved->x;
	moved->z = rubber->pos.zPos - moved->z;

	if (moved->x || moved->z)
	{
		c = phd_cos(rubber->pos.yRot);
		s = phd_sin(rubber->pos.yRot);
		front = (moved->z * c + moved->x * s) >> W2V_SHIFT;
		side = (-moved->z * s + moved->x * c) >> W2V_SHIFT;

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

static int RubberBoatDynamics(short boatNum)
{
	ITEM_INFO *boat;
	RUBBER_BOAT_INFO *binfo;
	FLOOR_INFO *floor;
	PHD_VECTOR moved, fl, fr, bl, br, f;
	PHD_VECTOR old, flOld, frOld, blOld, brOld, fOld;
	long hfl, hfr, hbr, hbl, hf;
	long hflOld, hfrOld, hbrOld, hblOld, hfOld;
	long height, slip, collide;
	short roomNum, rot;
	int newSpeed;

	boat = &g_Level.Items[boatNum];
	binfo = (RUBBER_BOAT_INFO*)boat->data;

	boat->pos.zRot -= binfo->tiltAngle;

	hflOld = TestWaterHeight(boat, 750, -300, &flOld);
	hfrOld = TestWaterHeight(boat, 750, 300, &frOld);
	hblOld = TestWaterHeight(boat, -750, -300, &blOld);
	hbrOld = TestWaterHeight(boat, -750, 300, &brOld);
	hfOld = TestWaterHeight(boat, 1000, 0, &fOld);
	old.x = boat->pos.xPos;
	old.y = boat->pos.yPos;
	old.z = boat->pos.zPos;

	if (blOld.y > hblOld)
		blOld.y = hblOld;
	if (brOld.y > hbrOld)
		brOld.y = hbrOld;
	if (flOld.y > hflOld)
		flOld.y = hflOld;
	if (frOld.y > hfrOld)
		frOld.y = hfrOld;
	if (fOld.y > hfOld)
		fOld.y = hfOld;

	boat->pos.yRot += binfo->boatTurn + binfo->extraRotation;
	binfo->tiltAngle = binfo->boatTurn * 6;

	boat->pos.zPos += boat->speed * phd_cos(boat->pos.yRot) >> W2V_SHIFT;
	boat->pos.zPos += boat->speed * phd_sin(boat->pos.yRot) >> W2V_SHIFT;
	if (boat->speed >= 0)
		binfo->propRot += (boat->speed * ANGLE(3)) + (ANGLE(1) << 1);
	else
		binfo->propRot += ANGLE(33);
	
	slip = 30 * phd_sin(boat->pos.zRot) >> W2V_SHIFT;
	if (!slip && boat->pos.zRot)
		slip = (boat->pos.zRot > 0) ? 1 : -1;
	boat->pos.zPos -= slip * phd_sin(boat->pos.yRot) >> W2V_SHIFT;
	boat->pos.xPos += slip * phd_cos(boat->pos.yRot) >> W2V_SHIFT;

	slip = 10 * phd_sin(boat->pos.xRot) >> W2V_SHIFT;
if (!slip && boat->pos.xRot)
slip = (boat->pos.xRot > 0) ? 1 : -1;
boat->pos.zPos -= slip * phd_cos(boat->pos.yRot) >> W2V_SHIFT;
boat->pos.xPos -= slip * phd_sin(boat->pos.yRot) >> W2V_SHIFT;

moved.x = boat->pos.xPos;
moved.z = boat->pos.zPos;

DoRubberBoatShift(boatNum);

rot = 0;
hbl = TestWaterHeight(boat, -750, -300, &bl);
if (hbl < blOld.y - 128)
	rot = DoRubberBoatShift2(boat, &bl, &blOld);

hbr = TestWaterHeight(boat, -750, 300, &br);
if (hbr < brOld.y - 128)
	rot = DoRubberBoatShift2(boat, &br, &brOld);

hfl = TestWaterHeight(boat, 750, -300, &fl);
if (hfl < flOld.y - 128)
	rot += DoRubberBoatShift2(boat, &fl, &flOld);

hfr = TestWaterHeight(boat, 750, 300, &fr);
if (hfr < frOld.y - 128)
	rot += DoRubberBoatShift2(boat, &fr, &frOld);

if (!slip)
{
	hf = TestWaterHeight(boat, 1000, 0, &f);
	if (hf > fOld.y - 128)
		DoRubberBoatShift2(boat, &f, &fOld);
}

roomNum = boat->roomNumber;
floor = GetFloor(boat->pos.xPos, boat->pos.yPos, boat->pos.zPos, &roomNum);
height = GetWaterHeight(boat->pos.xPos, boat->pos.yPos, boat->pos.zPos, roomNum);
if (height == NO_HEIGHT)
height = GetFloorHeight(floor, boat->pos.xPos, boat->pos.yPos, boat->pos.zPos);
if (height < boat->pos.yPos - 128)
	DoRubberBoatShift2(boat, (PHD_VECTOR*)&boat->pos, &old);

binfo->extraRotation = rot;

collide = GetRubberBoatCollisionAnim(boat, &moved);

if (slip || collide)
{
	newSpeed = ((boat->pos.zPos - old.z) * phd_cos(boat->pos.yRot) +
		(boat->pos.xPos - old.x) * phd_sin(boat->pos.yRot)) >> W2V_SHIFT;
	if (Lara.Vehicle == boatNum && boat->speed > 115 && newSpeed < boat->speed - 10)
	{
		LaraItem->hitPoints -= boat->speed;
		LaraItem->hitStatus = true;
		SoundEffect(SFX_TR3_LARA_INJURY, &LaraItem->pos, NULL);
		newSpeed >>= 1;
		boat->speed >>= 1;
	}
	if (slip)
	{
		if (boat->speed <= 120)
			boat->speed = newSpeed;
	}
	else
	{
		if (boat->speed > 0 && newSpeed < boat->speed)
			boat->speed = newSpeed;
		else if (boat->speed < 0 && newSpeed > boat->speed)
			boat->speed = newSpeed;
	}
	if (boat->speed < -20)
		boat->speed = -20;
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
		fallspeed += ((height - *y - fallspeed) >> 3);
		if (fallspeed < -20)
			fallspeed - 20;

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
		if ((!(TrInput & IN_ROLL) && !(TrInput & IN_LOOK) || boat->speed))
		{
			if (((TrInput & (IN_LEFT | IN_LSTEP)) && !(TrInput & IN_JUMP)) || ((TrInput & (IN_RIGHT | IN_RSTEP)) && (TrInput & IN_JUMP)))
			{
				if (binfo->boatTurn > 0)
					binfo->boatTurn -= ANGLE(1)/4;
				else
				{
					binfo->boatTurn -= ANGLE(1)/8;
					if (binfo->boatTurn < -ANGLE(4))
						binfo->boatTurn = -ANGLE(4);
				}
				noTurn = 0;
			}
			else if (((TrInput & (IN_RIGHT | IN_RSTEP)) && !(TrInput & IN_JUMP)) || ((TrInput & (IN_LEFT | IN_LSTEP)) && (TrInput & IN_JUMP)))
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

			if (TrInput & IN_JUMP)
			{
				if (boat->speed > 0)
					boat->speed -= 5;
				else if (boat->speed > -20)
					boat->speed += -2;
			}
			else if (TrInput & IN_ACTION)
			{
				if (TrInput & (IN_SPRINT | IN_DUCK))
					maxSpeed = 185;
				else
					maxSpeed = (TrInput & IN_WALK) ? 37 : 110;

				if (boat->speed < maxSpeed)
					boat->speed += 3 + (5 * boat->speed) / (2 * maxSpeed);
				else if (boat->speed > maxSpeed + 1)
					boat->speed -= 1;

			}
			else if (boat->speed >= 0 && boat->speed < 20 && (TrInput & (IN_LEFT | IN_LSTEP) | (IN_RIGHT | IN_RSTEP)))
			{
				if (boat->speed == 0 && !(TrInput & IN_ROLL))
					boat->speed = 20;
			}
			else if (boat->speed > 1)
				boat->speed -= 1;
			else
				boat->speed = 0;
		}
		else
		{
			if (boat->speed >= 0 && boat->speed < 20 && (TrInput & (IN_LEFT | IN_LSTEP) | (IN_RIGHT | IN_RSTEP)))
			{
				if (boat->speed == 0 && !(TrInput & IN_ROLL))
					boat->speed = 20;
			}
			else if (boat->speed > 1)
				boat->speed -= 1;
			else
				boat->speed = 0;

			if ((TrInput & IN_LOOK) && boat->speed == 0)
				LookUpDown();
		}
	}
	return noTurn;
}

static void RubberBoatSplash(ITEM_INFO* item, long fallspeed, long water)
{
	/*
	splash_setup.x = item->pos.x_pos;
	splash_setup.y = water;
	splash_setup.z = item->pos.z_pos;
	splash_setup.InnerXZoff = 16<<2;
	splash_setup.InnerXZsize = 12<<2;
	splash_setup.InnerYsize = -96<<2;
	splash_setup.InnerXZvel = 0xa0;
	splash_setup.InnerYvel = -fallspeed<<7;
	splash_setup.InnerGravity = 0x80;
	splash_setup.InnerFriction = 7;
	splash_setup.MiddleXZoff = 24<<2;
	splash_setup.MiddleXZsize = 24<<2;
	splash_setup.MiddleYsize = -64<<2;
	splash_setup.MiddleXZvel = 0xe0;
	splash_setup.MiddleYvel = -fallspeed<<6;
	splash_setup.MiddleGravity = 0x48;
	splash_setup.MiddleFriction = 8;
	splash_setup.OuterXZoff = 32<<2;
	splash_setup.OuterXZsize = 32<<2;
	splash_setup.OuterXZvel = 0x110;
	splash_setup.OuterFriction = 9;
	SetupSplash(&splash_setup);
	SplashCount = 16;
	*/
}

void InitialiseRubberBoat(short itemNum)
{
	ITEM_INFO* boat;
	RUBBER_BOAT_INFO* binfo;

	boat = &g_Level.Items[itemNum];
	binfo = (RUBBER_BOAT_INFO*)malloc(sizeof(RUBBER_BOAT_INFO));
	boat->data = malloc(sizeof(RUBBER_BOAT_INFO));
	binfo->boatTurn = 0;
	binfo->tiltAngle = 0;
	binfo->rightFallspeed = 0;
	binfo->leftFallspeed = 0;
	binfo->extraRotation = 0;
	binfo->water = 0;
	binfo->pitch = 0;
}

void RubberBoatCollision(short itemNum, ITEM_INFO *lara, COLL_INFO *coll)
{
	int getOn;
	ITEM_INFO *item;

	if (lara->hitPoints <= 0 || Lara.Vehicle == NO_ITEM)
		return;
	item = &g_Level.Items[itemNum];
	getOn = RubberBoatCheckGeton(itemNum, coll);

	if (!getOn)
	{
		coll->enableBaddiePush = true;
		ObjectCollision(itemNum, lara, coll);
		return;
	}

	Lara.Vehicle = itemNum;
	
	if (getOn == 1)
		lara->animNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + 8;
	else if (getOn == 2)
		lara->animNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + 0;
	else if (getOn == 3)
		lara->animNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + 6;
	else
		lara->animNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + 1;

	Lara.waterStatus = LW_ABOVE_WATER;
	lara->pos.xPos = item->pos.xPos;
	lara->pos.zPos = item->pos.zPos;
	lara->pos.yPos = item->pos.yPos - 5;
	lara->pos.yRot = item->pos.yRot;
	lara->pos.xRot = 0;
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
	/*if you wanna hardcode which music track to play when you get on the boat, put it here.*/
	/*but I'm not doing that :P*/
}

static int CanGetOffRubberBoat(int direction)
{
	ITEM_INFO *boat;
	FLOOR_INFO *floor;
	short roomNum, angle;
	int x, y, z, height, ceiling;

	boat = &g_Level.Items[Lara.Vehicle];

	if (direction < 0)
		angle = boat->pos.yRot - ANGLE(90);
	else
		angle = boat->pos.yRot + ANGLE(90);

	x = boat->pos.xPos + (1024 * phd_sin(angle) >> W2V_SHIFT);
	y = boat->pos.yPos;
	z = boat->pos.zPos + (1024 * phd_cos(angle) >> W2V_SHIFT);

	roomNum = boat->roomNumber;
	floor = GetFloor(x, y, z, &roomNum);
	height = GetFloorHeight(floor, x, y, z);
	ceiling = GetCeiling(floor, x, y, z);

	if (height - boat->pos.yPos < -512)
		return 0;

	if (HeightType == BIG_SLOPE || HeightType == DIAGONAL)
		return 0;

	if ((ceiling - boat->pos.yPos > -762) || (height - ceiling < 762))
		return 0;

	return 1;
}

void RubberBoatAnimation(ITEM_INFO *boat, int collide)
{
	RUBBER_BOAT_INFO *binfo;
	binfo = (RUBBER_BOAT_INFO*)boat->data;

	if (LaraItem->hitPoints <= 0)
	{
		if (LaraItem->currentAnimState!= 8)
		{
			LaraItem->animNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + 18;
			LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
			LaraItem->currentAnimState = 8;
			LaraItem->goalAnimState = 8;
		}
	}
	else if (boat->pos.yPos < binfo->water - 128 && boat->fallspeed > 0)
	{
		if (LaraItem->currentAnimState != 6)
		{
			LaraItem->animNumber = Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + 15;
			LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
			LaraItem->currentAnimState = 6;
			LaraItem->goalAnimState = 6;
		}
	}
	else if (collide)
	{
		if (LaraItem->currentAnimState != 5)
		{
			LaraItem->animNumber = (short)(Objects[ID_RUBBER_BOAT_LARA_ANIMS].animIndex + collide);
			LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
			LaraItem->currentAnimState = 5;
			LaraItem->goalAnimState = 5;
		}
	}
	else
	{
		switch (LaraItem->currentAnimState)
		{
		case 1:
			if (TrInput & IN_ROLL)
			{
				if (boat->speed == 0)
				{
					if ((TrInput & IN_RIGHT | IN_RSTEP) && CanGetOffRubberBoat(boat->pos.yRot + ANGLE(90)))
						LaraItem->goalAnimState = 3;
					else if ((TrInput & IN_LEFT | IN_LSTEP) && CanGetOffRubberBoat(boat->pos.yRot - ANGLE(90)))
						LaraItem->goalAnimState = 4;
				}
			}

			if (boat->speed > 0)
				LaraItem->goalAnimState = 2;

			break;
		case 2:
			if (boat->speed <= 0)
				LaraItem->goalAnimState = 1;
			if (TrInput & IN_RIGHT | IN_RSTEP)
				LaraItem->goalAnimState = 7;
			else if (TrInput & IN_LEFT | IN_LSTEP)
				LaraItem->goalAnimState = 9;
			
			break;
		case 6:
			LaraItem->goalAnimState = 2;
		case 7:
			if (boat->speed <= 0)
				LaraItem->goalAnimState = 1;
			else if (!(TrInput & IN_RIGHT | IN_RSTEP))
				LaraItem->goalAnimState = 2;
			break;
		case 9:
			if (boat->speed <= 0)
				LaraItem->goalAnimState = 1;
			else if (!(TrInput & IN_LEFT | IN_LSTEP))
				LaraItem->goalAnimState = 2;
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
	sptr->fadeToBlack = 12 - (snow << 3);
	sptr->sLife = sptr->life = (GetRandomControl() & 3) + 20;
	sptr->transType = COLADD;
	sptr->extras = 0;
	sptr->dynamic = -1;

	sptr->x = x * ((GetRandomControl() & 15) - 8);
	sptr->y = y * ((GetRandomControl() & 15) - 8);
	sptr->z = z * ((GetRandomControl() & 15) - 8);
	zv = (speed * phd_cos(angle)) >> (W2V_SHIFT + 2);
	xv = (speed * phd_sin(angle)) >> (W2V_SHIFT + 2);
	sptr->xVel = xv + ((GetRandomControl() & 127) - 64);
	sptr->yVel = (speed << 3) + (speed << 2);
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
		size = (GetRandomControl() & 7) + (speed >> 1) + 16;
	}
}

void RubberBoatControl(short itemNum)
{
	ITEM_INFO *boat;
	RUBBER_BOAT_INFO *binfo;
	PHD_VECTOR fr, fl, prop;
	FLOOR_INFO *floor;
	long hfl, hfr, noTurn(1), drive(0);
	long height, collide, water, ceiling, pitch, h, ofs, nowake;
	short roomNum, xRot, zRot;


	boat = &g_Level.Items[itemNum];
	binfo = (RUBBER_BOAT_INFO*)boat->data;
	collide = RubberBoatDynamics(itemNum);
	
	hfl = TestWaterHeight(boat, 750, -300, &fl);
	hfr = TestWaterHeight(boat, 750, 300, &fr);

	roomNum = boat->roomNumber;
	floor = GetFloor(boat->pos.xPos, boat->pos.yPos, boat->pos.zPos, &roomNum);
	height = GetFloorHeight(floor, boat->pos.xPos, boat->pos.yPos, boat->pos.zPos);
	ceiling = GetCeiling(floor, boat->pos.xPos, boat->pos.yPos, boat->pos.zPos);
	if (Lara.Vehicle == itemNum)
	{
		TestTriggers(TriggerIndex, 0, 0);
		TestTriggers(TriggerIndex, 1, 1);
	}

	binfo->water = water = GetWaterHeight(boat->pos.xPos, boat->pos.xPos, boat->pos.xPos, roomNum);

	if (Lara.Vehicle == itemNum && LaraItem->hitPoints > 0)
	{
		switch (LaraItem->currentAnimState)
		{
		case 0:
		case 3:
		case 4:
			break;
		
		default:
			drive = 1;
			noTurn = RubberBoatUserControl(boat);
		}
	}
	else
	{
		if (boat->speed > 1)
			boat->speed -= 1;
		else
			boat->speed = 0;
	}

	if (noTurn)
	{
		if (binfo->boatTurn < -(ANGLE(1) / 4))
			binfo->boatTurn += ANGLE(1) / 4;
		else if (binfo->boatTurn > (ANGLE(1) / 4))
			binfo -= ANGLE(1) / 4;
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
	if ((ofs - boat->fallspeed) > 32 && boat->fallspeed == 0 && water != NO_HEIGHT)
		RubberBoatSplash(boat, ofs - boat->fallspeed, water);

	height = (fl.y + fr.y);
	if (height < 0)
		height = -(abs(height) >> 1);
	else
		height = height >> 1;

	xRot = phd_atan(750, boat->pos.yPos - height);
	zRot = phd_atan(300, height - fl.y);
	boat->pos.xRot += (xRot - boat->pos.xRot) >> 1;
	boat->pos.zRot += (zRot - boat->pos.zRot) >> 1;

	if (!xRot && abs(boat->pos.xRot) < 4)
		boat->pos.xRot = 0;
	if (!zRot && abs(boat->pos.zRot) < 4)
		boat->pos.zRot = 0;

	if (Lara.Vehicle == itemNum)
	{
		RubberBoatAnimation(boat, collide);

		if (roomNum != boat->roomNumber)
		{
			ItemNewRoom(itemNum, roomNum);
			ItemNewRoom(Lara.itemNumber, roomNum);
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
			boat->frameNumber = g_Level.Anims[boat->animNumber].frameBase + (LaraItem->frameNumber - g_Level.Anims[LaraItem->frameNumber].frameBase);
		}

		Camera.targetElevation = -ANGLE(20);
		Camera.targetDistance = 2048;
	}
	else
	{
		if (roomNum != boat->roomNumber)
			ItemNewRoom(itemNum, roomNum);
		boat->pos.zRot += binfo->tiltAngle;
	}

	pitch = boat->speed;
	binfo->pitch += (pitch - binfo->pitch) >> 2;

	if (boat->speed > 8)
		SoundEffect(SFX_TR3_BOAT_MOVING, &boat->pos, PITCH_SHIFT + ((0x10000 - (110 - binfo->pitch)) << 8));
	else if (drive)
		SoundEffect(SFX_TR3_BOAT_IDLE, &boat->pos, PITCH_SHIFT + ((0x10000 - (110 - binfo->pitch)) << 8));

	if (Lara.Vehicle != itemNum)
		return;

	enum boat_anims {
		BOAT_GETON,
		BOAT_STILL,
		BOAT_MOVING,
		BOAT_JUMPR,
		BOAT_JUMPL,
		BOAT_HIT,
		BOAT_FALL,
		BOAT_TURNR,
		BOAT_DEATH,
		BOAT_TURNL
	};

	if ((LaraItem->currentAnimState == 3 || LaraItem->currentAnimState == 4) && 
		(LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameEnd))
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
		x = LaraItem->pos.xPos + (360 * phd_sin(LaraItem->pos.yRot) >> W2V_SHIFT);
		y = LaraItem->pos.yPos - 90;
		z = LaraItem->pos.zPos + (360 * phd_cos(LaraItem->pos.yRot) >> W2V_SHIFT);
		floor = GetFloor(x, y, z, &roomNum);
		if (GetFloorHeight(floor, x, y, z) >= y - 256)
		{
			LaraItem->pos.xPos = x;
			LaraItem->pos.zPos = z;
			if (roomNum != LaraItem->roomNumber)
				ItemNewRoom(Lara.itemNumber, roomNum);
		}
		LaraItem->pos.yPos = y;

		boat->animNumber = Objects[ID_RUBBER_BOAT].animIndex + 0;
		boat->frameNumber = g_Level.Anims[boat->animNumber].frameBase;
	}

	roomNum = boat->roomNumber;
	floor = GetFloor(boat->pos.xPos, boat->pos.yPos + 128, boat->pos.zPos, &roomNum);
	h = GetWaterHeight(boat->pos.xPos, boat->pos.yPos + 128, boat->pos.zPos, roomNum);

	if (h > boat->pos.yPos + 32 || h == NO_HEIGHT)
		h = 0;
	else
		h = 1;

/*	if (LaraItem->currentAnimState == 3 || LaraItem->currentAnimState == 4)
		nowake = 1;
	else
		nowake = 0;*/

	prop.x = 0;
	prop.y = 0;
	prop.z = -80;
	GetJointAbsPosition(boat, &prop, 2);
	roomNum = boat->roomNumber;
	floor = GetFloor(prop.x, prop.y, prop.z, &roomNum);

	if (boat->speed && h < prop.y && h != NO_HEIGHT)
	{
		TriggerRubberBoatMist(prop.x, prop.y, prop.z, abs(boat->speed), boat->pos.yRot + 0x8000, 0);
		if ((GetRandomControl() & 1) == 0)
		{
			PHD_3DPOS pos;
			short roomNum;

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
		if (prop.y > h && !(g_Level.Rooms[roomNum].flags & ENV_FLAG_WATER))
		{
			pos.x = prop.x;
			pos.y = prop.y;
			pos.z = prop.z;

			cnt = (GetRandomControl() & 3) + 3;
			for (;cnt>0;cnt--)
			TriggerRubberBoatMist(prop.x, prop.y, prop.z, ((GetRandomControl() & 15) + 96) << 4, boat->pos.yRot + 0x4000 + GetRandomControl(), 1);

		}
	}
	//update wake effects
}
