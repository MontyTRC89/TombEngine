#include "newobjects.h"
#include "../Game/lara.h"
#include "../Game/effect2.h"
#include "../Game/draw.h"
#include "../Game/camera.h"
#include "../Game/collide.h"
#include "../Game/effects.h"
#include "../Game/laraflar.h"
#include "../Game/items.h"

#define MAX_SPEED 0x380000
#define KAYAK_COLLIDE 64
#define GETOFF_DIST 768		// minimum collision distance
#define KAYAK_TO_BADDIE_RADIUS 256
#define KAYAK_FRICTION 0x8000
#define KAYAK_ROT_FRIC 0x50000
#define KAYAK_DFLECT_ROT 0x80000
#define KAYAK_FWD_VEL 0x180000
#define KAYAK_FWD_ROT 0x800000
#define KAYAK_LR_VEL 0x100000
#define KAYAK_LR_ROT 0xc00000
#define KAYAK_MAX_LR 0xc00000
#define KAYAK_TURN_ROT 0x200000
#define KAYAK_MAX_TURN 0x1000000
#define KAYAK_TURN_BRAKE 0x8000
#define KAYAK_HARD_ROT 0x1000000
#define KAYAK_MAX_STAT 0x1000000
#define BOAT_SLIP 50
#define BOAT_SIDE_SLIP 50
#define HIT_BACK 1
#define HIT_FRONT 2
#define HIT_LEFT 3
#define HIT_RIGHT 4
#define KAYAK_DRAW_SHIFT 32
#define LARA_LEG_BITS ((1<<HIPS)|(1<<THIGH_L)|(1<<CALF_L)|(1<<FOOT_L)|(1<<THIGH_R)|(1<<CALF_R)|(1<<FOOT_R))
#define KAYAK_X	128
#define KAYAK_Z	128
#define SKIDOO_MAX_KICK -80
#define SKIDOO_MIN_BOUNCE ((MAX_SPEED/2)>>8)

enum KAYAK_STATE {
	KS_BACK,
	KS_POSE,
	KS_LEFT,
	KS_RIGHT,
	KS_CLIMBIN,
	KS_DEATHIN,
	KS_FORWARD,
	KS_ROLL,
	KS_DROWNIN,
	KS_JUMPOUT,
	KS_TURNL,
	KS_TURNR,
	KS_CLIMBINR,
	KS_CLIMBOUTL,
	KS_CLIMBOUTR,
};

void __cdecl DoKayakRipple(ITEM_INFO* v, __int16 xoff, __int16 zoff)
{
	RIPPLE_STRUCT* r;
	int s, c, x, z;
	FLOOR_INFO* floor;
	__int16 room_number;

	c = COS(v->pos.yRot);
	s = SIN(v->pos.yRot);

	x = v->pos.xPos + (((zoff * s) + (xoff * c)) >> W2V_SHIFT);
	z = v->pos.zPos + (((zoff * c) - (xoff * s)) >> W2V_SHIFT);

	room_number = v->roomNumber;
	floor = GetFloor(x, v->pos.yPos, z, &room_number);

	if (GetWaterHeight(x, v->pos.yPos, z, room_number) != NO_HEIGHT)
	{
		SetupRipple(x, v->pos.yPos, z, -2 - (GetRandomControl() & 1), 0);
	}
}

void __cdecl KayakSplash(ITEM_INFO* item, long fallspeed, long water)
{
	/*
	splash_setup.x = item->pos.x_pos;
	splash_setup.y = water;
	splash_setup.z = item->pos.z_pos;
	splash_setup.InnerXZoff = 16 << 3;
	splash_setup.InnerXZsize = 12 << 2;
	splash_setup.InnerYsize = -96 << 2;
	splash_setup.InnerXZvel = 0xa0;
	splash_setup.InnerYvel = -fallspeed << 5;
	splash_setup.InnerGravity = 0x80;
	splash_setup.InnerFriction = 7;
	splash_setup.MiddleXZoff = 24 << 3;
	splash_setup.MiddleXZsize = 24 << 2;
	splash_setup.MiddleYsize = -64 << 2;
	splash_setup.MiddleXZvel = 0xe0;
	splash_setup.MiddleYvel = -fallspeed << 4;
	splash_setup.MiddleGravity = 0x48;
	splash_setup.MiddleFriction = 8;
	splash_setup.OuterXZoff = 32 << 3;
	splash_setup.OuterXZsize = 32 << 2;
	splash_setup.OuterXZvel = 0x110;
	splash_setup.OuterFriction = -9;
	SetupSplash(&splash_setup);
	SplashCount = 16;
	*/
}

void __cdecl TriggerRapidsMist(long x, long y, long z)
{
	/*
	SPARKS* sptr;
	long xsize;

	sptr = &Sparks[GetFreeSpark()];
	sptr->on = 1;
	sptr->sR = 128;
	sptr->sG = 128;
	sptr->sB = 128;
	sptr->dR = 192;
	sptr->dG = 192;
	sptr->dB = 192;
	sptr->colFadeSpeed = 2;
	sptr->fadeToBlack = 4;	// 8
	sptr->sLife = sptr->life = 6 + (GetRandomControl() & 3);
	sptr->transType = 1;
	sptr->extras = 0;
	sptr->dynamic = -1;
	sptr->x = x + ((GetRandomControl() & 15) - 8);
	sptr->y = y + ((GetRandomControl() & 15) - 8);
	sptr->z = z + ((GetRandomControl() & 15) - 8);
	sptr->xVel = ((GetRandomControl() & 255) - 128);
	sptr->zVel = ((GetRandomControl() & 255) - 128);
	sptr->yVel = ((GetRandomControl() & 255) - 128);
	sptr->friction = 3;
	if (GetRandomControl() & 1)
	{
		sptr->flags = SP_SCALE|SP_DEF|SP_ROTATE|SP_EXPDEF;
		sptr->rotAng = GetRandomControl() & 4095;
		if (GetRandomControl() & 1)
			sptr->rotAdd = -(GetRandomControl() & 15) - 16;
		else
			sptr->rotAdd = (GetRandomControl() & 15) + 16;
	}
	else
	{
		sptr->flags = SP_SCALE|SP_DEF|SP_EXPDEF;
	}
	sptr->def = Objects[ID_DEFAULT_SPRITES].meshIndex;
	sptr->scalar = 4;
	sptr->gravity = 0;
	sptr->maxYvel = 0;
	xsize = (GetRandomControl() & 7) + 16;
	sptr->size = sptr->sSize = xsize >> 1;
	sptr->dSize = xsize;
	*/
}

int __cdecl GetInKayak(__int16 item_number, COLL_INFO* coll)
{
	int dist;
	int x, z;
	ITEM_INFO* kayak;
	FLOOR_INFO* floor;
	__int16 room_number;

	if (!(TrInput & IN_ACTION) || Lara.gunStatus != LG_NO_ARMS || LaraItem->gravityStatus)
		return 0;

	kayak = &Items[item_number];

	/* -------- is Lara close enough to use the vehicle */

	x = LaraItem->pos.xPos - kayak->pos.xPos;
	z = LaraItem->pos.zPos - kayak->pos.zPos;

	dist = (x * x) + (z * z);

	if (dist > (130000))
		return 0;

	/*
	 * SONY BUG FIX 10000001: If player stops in exactly right place and buries
	 * skidoo in avalanche, then they can get on it and crash the game
	 */
	room_number = kayak->roomNumber;
	floor = GetFloor(kayak->pos.xPos, kayak->pos.yPos, kayak->pos.zPos, &room_number);
	if (GetFloorHeight(floor, kayak->pos.xPos, kayak->pos.yPos, kayak->pos.zPos) > -32000)
	{
		__int16 ang;
		unsigned __int16 tempang;

		ang = ATAN(kayak->pos.zPos - LaraItem->pos.zPos, kayak->pos.xPos - LaraItem->pos.xPos);
		ang -= kayak->pos.yRot;

		tempang = LaraItem->pos.yRot - kayak->pos.yRot;

		if (ang > -ANGLE(45) && ang < ANGLE(135))
		{
			if (tempang > ANGLE(45) && tempang < ANGLE(135))
				return -1;
		}
		else
		{
			if (tempang > ANGLE(225) && tempang < ANGLE(315))
				return 1;
		}
	}

	return 0;
}

int __cdecl GetKayakCollisionAnim(ITEM_INFO* v, int xdiff, int zdiff)
{
	xdiff = v->pos.xPos - xdiff;
	zdiff = v->pos.zPos - zdiff;

	/* -------- determine direction of travel relative to rotation */
	if ((xdiff) || (zdiff))
	{
		int c, s, front, side;

		c = COS(v->pos.yRot);
		s = SIN(v->pos.yRot);

		front = ((zdiff * c) + (xdiff * s)) >> W2V_SHIFT;
		side = ((-zdiff * s) + (xdiff * c)) >> W2V_SHIFT;

		if (abs(front) > abs(side))
		{
			if (front > 0)
				return HIT_BACK;
			else
				return HIT_FRONT;
		}
		else
		{
			if (side > 0)
				return HIT_LEFT;
			else
				return HIT_RIGHT;
		}
	}

	return 0;
}

int __cdecl DoKayakDynamics(int height, int fallspeed, int* y)
{
	int kick;

	if (height > * y)
	{
		/* In air */
		*y += fallspeed;

		if (*y > height)
			//		if (*y > height - SKIDOO_MIN_BOUNCE)
		{
			*y = height;
			fallspeed = 0;
		}
		else
			fallspeed += GRAVITY;
	}
	else
	{
		/* On ground: get up push from height change (if not a closed door and so NO_HEIGHT) */
		kick = (height - *y) << 2;

		if (kick < SKIDOO_MAX_KICK)
			kick = SKIDOO_MAX_KICK;

		fallspeed += ((kick - fallspeed) >> 3);
		if (*y > height)
			*y = height;
	}

	return fallspeed;
}

void __cdecl DoKayakCurrent(ITEM_INFO* item)
{
	ROOM_INFO* r;
	PHD_VECTOR target;
	long angle, dx, dz, speed, sinkval;

	r = &Rooms[item->roomNumber];

	if (!Lara.currentActive)
	{
		long shifter, absvel;
		absvel = abs(Lara.currentActive);

		if (absvel > 16)
			shifter = 4;
		else if (absvel > 8)
			shifter = 3;
		else
			shifter = 2;

		Lara.currentXvel -= Lara.currentXvel >> shifter;

		if (abs(Lara.currentXvel) < 4)
			Lara.currentXvel = 0;

		absvel = abs(Lara.currentZvel);
		if (absvel > 16)
			shifter = 4;
		else if (absvel > 8)
			shifter = 3;
		else
			shifter = 2;

		Lara.currentZvel -= Lara.currentZvel >> shifter;
		if (abs(Lara.currentZvel) < 4)
			Lara.currentZvel = 0;

		if (Lara.currentXvel == 0 && Lara.currentZvel == 0)
			return;
	}
	else
	{
		sinkval = Lara.currentActive - 1;
		target.x = Camera.fixed[sinkval].x;
		target.y = Camera.fixed[sinkval].y;
		target.z = Camera.fixed[sinkval].z;
		angle = ((mGetAngle(target.x, target.z, LaraItem->pos.xPos, LaraItem->pos.zPos) - 0x4000) >> 4) & 4095;

		dx = target.x - LaraItem->pos.xPos;
		dz = target.z - LaraItem->pos.zPos;

		speed = Camera.fixed[sinkval].data;
		dx = (((rcossin_tbl[(angle << 1)] * speed))) >> 2;
		dz = (((rcossin_tbl[(angle << 1) + 1] * speed))) >> 2;

		Lara.currentXvel += (dx - Lara.currentXvel) >> 4;
		Lara.currentZvel += (dz - Lara.currentZvel) >> 4;
	}

	/* Move Lara in direction of sink. */
	item->pos.xPos += Lara.currentXvel >> 8;
	item->pos.zPos += Lara.currentZvel >> 8;

	/* Reset current (will get set again so long as Lara is over triggers) */
	Lara.currentActive = 0;
}

int __cdecl TestKayakHeight(ITEM_INFO* item, int x, int z, PHD_VECTOR* pos)
{
	int h;
	FLOOR_INFO* floor;
	__int16 room_number;

	phd_PushUnitMatrix();
	MatrixPtr[M03] = item->pos.xPos;
	MatrixPtr[M13] = item->pos.yPos;
	MatrixPtr[M23] = item->pos.zPos;
	phd_RotYXZ(item->pos.yRot, item->pos.xRot, item->pos.zRot);
	phd_TranslateRel(x, 0, z);
	pos->x = (MatrixPtr[M03] >> W2V_SHIFT);
	pos->y = (MatrixPtr[M13] >> W2V_SHIFT);
	pos->z = (MatrixPtr[M23] >> W2V_SHIFT);

	room_number = item->roomNumber;
	GetFloor(pos->x, pos->y, pos->z, &room_number);

	if ((h = GetWaterHeight(pos->x, pos->y, pos->z, room_number)) == NO_HEIGHT)
	{
		room_number = item->roomNumber;
		floor = GetFloor(pos->x, pos->y, pos->z, &room_number);
		if ((h = GetFloorHeight(floor, pos->x, pos->y, pos->z)) == NO_HEIGHT)
			return h;
	}

	return h - 5;
}

int __cdecl CanKayakGetOut(ITEM_INFO* kayak, int direction)
{
	int height;
	PHD_VECTOR pos;

	height = TestKayakHeight(kayak, (direction < 0) ? -GETOFF_DIST : GETOFF_DIST, 0, &pos);

	if ((kayak->pos.yPos - height) > 0)
		return 0;

	return 1;
}

int __cdecl DoKayakShift(ITEM_INFO* v, PHD_VECTOR* pos, PHD_VECTOR* old)
{
	int x, z;
	int x_old, z_old;
	int shift_x, shift_z;
	int height;
	FLOOR_INFO* floor;
	__int16 room_number;

	x = pos->x >> WALL_SHIFT;
	z = pos->z >> WALL_SHIFT;

	x_old = old->x >> WALL_SHIFT;
	z_old = old->z >> WALL_SHIFT;

	shift_x = pos->x & (WALL_SIZE - 1);
	shift_z = pos->z & (WALL_SIZE - 1);

	if (x == x_old)
	{
		old->x = 0;

		if (z == z_old)
		{
			v->pos.zPos += (old->z - pos->z);
			v->pos.xPos += (old->x - pos->x);
		}

		else if (z > z_old)
		{
			v->pos.zPos -= shift_z + 1;
			return (pos->x - v->pos.xPos);
		}

		else
		{
			v->pos.zPos += WALL_SIZE - shift_z;
			return (v->pos.xPos - pos->x);
		}
	}

	else if (z == z_old)
	{
		old->z = 0;

		if (x > x_old)
		{
			v->pos.xPos -= shift_x + 1;
			return (v->pos.zPos - pos->z);
		}

		else
		{
			v->pos.xPos += WALL_SIZE - shift_x;
			return (pos->z - v->pos.zPos);
		}
	}

	else
	{
		/* A diagonal hit; means a barrage of tests needed to determine best shift */
		x = 0;
		z = 0;

		room_number = v->roomNumber;
		floor = GetFloor(old->x, pos->y, pos->z, &room_number);
		height = GetFloorHeight(floor, old->x, pos->y, pos->z);
		if (height < old->y - STEP_SIZE)
		{
			if (pos->z > old->z)
				z = -shift_z - 1;
			else
				z = WALL_SIZE - shift_z;
		}

		room_number = v->roomNumber;
		floor = GetFloor(pos->x, pos->y, old->z, &room_number);
		height = GetFloorHeight(floor, pos->x, pos->y, old->z);
		if (height < old->y - STEP_SIZE)
		{
			if (pos->x > old->x)
				x = -shift_x - 1;
			else
				x = WALL_SIZE - shift_x;
		}

		/* -------- corner collision */
		if (x && z)
		{
			v->pos.zPos += z;
			v->pos.xPos += x;
		}
		/* -------- edge collisions */
		else if (z)
		{
			v->pos.zPos += z;

			if (z > 0)
				return (v->pos.xPos - pos->x);
			else
				return (pos->x - v->pos.xPos);
		}
		else if (x)
		{
			v->pos.xPos += x;

			if (x > 0)
				return (pos->z - v->pos.zPos);
			else
				return (v->pos.zPos - pos->z);
		}
		/* -------- diagnal collision */
		else
		{
			v->pos.zPos += (old->z - pos->z);
			v->pos.xPos += (old->x - pos->x);
		}
	}

	return 0;
}

void __cdecl KayakToBackground(ITEM_INFO* kayak, KAYAK_INFO* kinfo)
{
	int h, slip = 0, rot = 0;
	PHD_VECTOR pos;
	int collide, oldx, oldz;
	FLOOR_INFO* floor;
	__int16 room_number;
	int height[8];
	PHD_VECTOR oldpos[9];
	int x, z;
	int fh, lh, rh;
	PHD_VECTOR fpos, lpos, rpos;

	kinfo->OldPos = kayak->pos;

	/* -------- determine valid Kayak positions */

	height[0] = TestKayakHeight(kayak, 0, 1024, &oldpos[0]);
	height[1] = TestKayakHeight(kayak, -96, 512, &oldpos[1]);
	height[2] = TestKayakHeight(kayak, 96, 512, &oldpos[2]);
	height[3] = TestKayakHeight(kayak, -KAYAK_X, KAYAK_Z, &oldpos[3]);
	height[4] = TestKayakHeight(kayak, KAYAK_X, KAYAK_Z, &oldpos[4]);
	height[5] = TestKayakHeight(kayak, -KAYAK_X, -320, &oldpos[5]);
	height[6] = TestKayakHeight(kayak, KAYAK_X, -320, &oldpos[6]);
	height[7] = TestKayakHeight(kayak, 0, -640, &oldpos[7]);

	for (collide = 0; collide < 8; collide++)
	{
		if (oldpos[collide].y > height[collide])
			oldpos[collide].y = height[collide];
	}

	oldpos[8].x = kayak->pos.xPos;
	oldpos[8].y = kayak->pos.yPos;
	oldpos[8].z = kayak->pos.zPos;

	/* -------- move Kayak according to velocities & current */
	fh = TestKayakHeight(kayak, 0, 1024, &fpos);
	lh = TestKayakHeight(kayak, -KAYAK_X, KAYAK_Z, &lpos);
	rh = TestKayakHeight(kayak, KAYAK_X, KAYAK_Z, &rpos);

	kayak->pos.yRot += (kinfo->Rot >> 16);
	kayak->pos.xPos += (kayak->speed * SIN(kayak->pos.yRot)) >> W2V_SHIFT;
	kayak->pos.zPos += (kayak->speed * COS(kayak->pos.yRot)) >> W2V_SHIFT;

	DoKayakCurrent(kayak);

	/* -------- move Kayak vertically */
	kinfo->FallSpeedL = DoKayakDynamics(lh, kinfo->FallSpeedL, &lpos.y);
	kinfo->FallSpeedR = DoKayakDynamics(rh, kinfo->FallSpeedR, &rpos.y);
	kinfo->FallSpeedF = DoKayakDynamics(fh, kinfo->FallSpeedF, &fpos.y);
	kayak->fallspeed = DoKayakDynamics(kinfo->Water, kayak->fallspeed, &kayak->pos.yPos);

	h = (lpos.y + rpos.y) >> 1;
	x = ATAN(1024, kayak->pos.yPos - fpos.y);
	z = ATAN(KAYAK_X, h - lpos.y);

	kayak->pos.xRot = x;
	kayak->pos.zRot = z;

	/* -------- slide Kayak back down slopes */
	oldx = kayak->pos.xPos;
	oldz = kayak->pos.zPos;

	/* -------- collide against background */

	if ((h = TestKayakHeight(kayak, 0, -640, &pos)) < (oldpos[7].y - KAYAK_COLLIDE))
		rot = DoKayakShift(kayak, &pos, &oldpos[7]);

	if ((h = TestKayakHeight(kayak, KAYAK_X, -320, &pos)) < (oldpos[6].y - KAYAK_COLLIDE))
		rot += DoKayakShift(kayak, &pos, &oldpos[6]);

	if ((h = TestKayakHeight(kayak, -KAYAK_X, -320, &pos)) < (oldpos[5].y - KAYAK_COLLIDE))
		rot += DoKayakShift(kayak, &pos, &oldpos[5]);

	if ((h = TestKayakHeight(kayak, KAYAK_X, KAYAK_Z, &pos)) < (oldpos[4].y - KAYAK_COLLIDE))
		rot += DoKayakShift(kayak, &pos, &oldpos[4]);

	if ((h = TestKayakHeight(kayak, -KAYAK_X, KAYAK_Z, &pos)) < (oldpos[3].y - KAYAK_COLLIDE))
		rot += DoKayakShift(kayak, &pos, &oldpos[3]);

	if ((h = TestKayakHeight(kayak, 96, 512, &pos)) < (oldpos[2].y - KAYAK_COLLIDE))
		rot += DoKayakShift(kayak, &pos, &oldpos[2]);

	if ((h = TestKayakHeight(kayak, -96, 512, &pos)) < (oldpos[1].y - KAYAK_COLLIDE))
		rot += DoKayakShift(kayak, &pos, &oldpos[1]);

	if ((h = TestKayakHeight(kayak, 0, 1024, &pos)) < (oldpos[0].y - KAYAK_COLLIDE))
		rot += DoKayakShift(kayak, &pos, &oldpos[0]);

	kayak->pos.yRot += rot;

	/* -------- update the vehicle's actual position */

	room_number = kayak->roomNumber;
	floor = GetFloor(kayak->pos.xPos, kayak->pos.yPos, kayak->pos.zPos, &room_number);
	h = GetWaterHeight(kayak->pos.xPos, kayak->pos.yPos, kayak->pos.zPos, room_number);

	if (h == NO_HEIGHT)
		h = GetFloorHeight(floor, kayak->pos.xPos, kayak->pos.yPos, kayak->pos.zPos);

	if (h < (kayak->pos.yPos - KAYAK_COLLIDE))
		DoKayakShift(kayak, (PHD_VECTOR*)&kayak->pos, &oldpos[8]);

	/* -------- if position is still invalid - find somewhere decent */
	room_number = kayak->roomNumber;
	floor = GetFloor(kayak->pos.xPos, kayak->pos.yPos, kayak->pos.zPos, &room_number);
	h = GetWaterHeight(kayak->pos.xPos, kayak->pos.yPos, kayak->pos.zPos, room_number);

	if (h == NO_HEIGHT)
		h = GetFloorHeight(floor, kayak->pos.xPos, kayak->pos.yPos, kayak->pos.zPos);

	if (h == NO_HEIGHT)
	{
		GAME_VECTOR kpos;

		kpos.x = kinfo->OldPos.xPos;
		kpos.y = kinfo->OldPos.yPos;
		kpos.z = kinfo->OldPos.zPos;
		kpos.roomNumber = kayak->roomNumber;

		CameraCollisionBounds(&kpos, 256, 0);
		{
			kayak->pos.xPos = kpos.x;
			kayak->pos.yPos = kpos.y;
			kayak->pos.zPos = kpos.z;
			kayak->roomNumber = kpos.roomNumber;
		}
	}

	/* -------- adjust speed upon collision */
	collide = GetKayakCollisionAnim(kayak, oldx, oldz);
	if (slip || collide)
	{
		int newspeed;

		newspeed = ((kayak->pos.zPos - oldpos[8].z) * COS(kayak->pos.yRot) + (kayak->pos.xPos - oldpos[8].x) * SIN(kayak->pos.yRot)) >> W2V_SHIFT;
		newspeed <<= 8;

		if (slip)
		{
			/* Only if slip is above certain amount, and boat is not in FAST speed range */
			if (kinfo->Vel <= MAX_SPEED)
				kinfo->Vel = newspeed;
		}
		else
		{
			if (kinfo->Vel > 0 && newspeed < kinfo->Vel)
				kinfo->Vel = newspeed;

			else if (kinfo->Vel < 0 && newspeed > kinfo->Vel)
				kinfo->Vel = newspeed;
		}

		if (kinfo->Vel < -MAX_SPEED)
			kinfo->Vel = -MAX_SPEED;
	}
}

void __cdecl KayakUserInput(ITEM_INFO* kayak, ITEM_INFO* lara, KAYAK_INFO* kinfo)
{
	__int16 frame;
	char lr;

	/* -------- kill Lara in boat */
	if ((lara->hitPoints <= 0) && (lara->currentAnimState != KS_DEATHIN))
	{
		lara->animNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + 5;
		lara->frameNumber = Anims[lara->animNumber].frameBase;
		lara->currentAnimState = lara->goalAnimState = KS_DEATHIN;
	}

	/* -------- control Kayak */
	frame = lara->frameNumber - Anims[lara->animNumber].frameBase;

	switch (lara->currentAnimState)
	{
	case KS_POSE:
		if ((TrInput & IN_ROLL) && (!Lara.currentActive) && (!Lara.currentXvel) && (!Lara.currentZvel))
		{
			if ((TrInput & IN_LEFT) && (CanKayakGetOut(kayak, -1)))
			{
				lara->goalAnimState = KS_JUMPOUT;
				lara->requiredAnimState = KS_CLIMBOUTL;
			}
			else if ((TrInput & IN_RIGHT) && (CanKayakGetOut(kayak, 1)))
			{
				lara->goalAnimState = KS_JUMPOUT;
				lara->requiredAnimState = KS_CLIMBOUTR;
			}
		}
		else if (TrInput & IN_FORWARD)
		{
			lara->goalAnimState = KS_RIGHT;
			kinfo->Turn = 0;
			kinfo->Forward = 1;
		}
		else if (TrInput & IN_BACK)
		{
			lara->goalAnimState = KS_BACK;
		}
		else if (TrInput & IN_LEFT)
		{
			lara->goalAnimState = KS_LEFT;

			if (kinfo->Vel)
				kinfo->Turn = 0;
			else
				kinfo->Turn = 1;		// harder turn

			kinfo->Forward = 0;
		}
		else if (TrInput & IN_RIGHT)
		{
			lara->goalAnimState = KS_RIGHT;

			if (kinfo->Vel)
				kinfo->Turn = 0;
			else
				kinfo->Turn = 1;		// harder turn

			kinfo->Forward = 0;
		}
		else if ((TrInput & IN_LSTEP) && ((kinfo->Vel) || (Lara.currentXvel) || (Lara.currentZvel)))
		{
			lara->goalAnimState = KS_TURNL;
		}
		else if ((TrInput & IN_RSTEP) && ((kinfo->Vel) || (Lara.currentXvel) || (Lara.currentZvel)))
		{
			lara->goalAnimState = KS_TURNR;
		}
		break;

	case KS_LEFT:
		if (kinfo->Forward)
		{
			if (!frame)
				lr = 0;
			if ((frame == 2) && (!(lr & 0x80)))
				lr++;
			else if (frame > 2)
				lr &= ~0x80;

			if (TrInput & IN_FORWARD)
			{
				if (TrInput & IN_LEFT)
				{
					if ((lr & ~0x80) >= 2)
						lara->goalAnimState = KS_RIGHT;
				}
				else
					lara->goalAnimState = KS_RIGHT;
			}
			else
				lara->goalAnimState = KS_POSE;
		}

		else if (!(TrInput & IN_LEFT))
			lara->goalAnimState = KS_POSE;

		/* -------- apply velocities */
		if (frame == 7)
		{
			if (kinfo->Forward)
			{
				if ((kinfo->Rot -= KAYAK_FWD_ROT) < -KAYAK_MAX_TURN)
					kinfo->Rot = -KAYAK_MAX_TURN;

				kinfo->Vel += KAYAK_FWD_VEL;
			}

			else if (kinfo->Turn)
			{
				if ((kinfo->Rot -= KAYAK_HARD_ROT) < -KAYAK_MAX_STAT)
					kinfo->Rot = -KAYAK_MAX_STAT;
			}
			else
			{
				if ((kinfo->Rot -= KAYAK_LR_ROT) < -KAYAK_MAX_LR)
					kinfo->Rot = -KAYAK_MAX_LR;

				kinfo->Vel += KAYAK_LR_VEL;
			}
		}

		/* -------- initialise ripple effects */

		if ((frame > 6) && (frame < 24) && (frame & 1))
			DoKayakRipple(kayak, -384, -64);
		break;
		/* --------------------- */
	case KS_RIGHT:
		/* --------------------- */
		if (kinfo->Forward)
		{
			if (!frame)
				lr = 0;

			if ((frame == 2) && (!(lr & 0x80)))
				lr++;

			else if (frame > 2)
				lr &= ~0x80;

			if (TrInput & IN_FORWARD)
			{
				if (TrInput & IN_RIGHT)
				{
					if ((lr & ~0x80) >= 2)
						lara->goalAnimState = KS_LEFT;
				}
				else
					lara->goalAnimState = KS_LEFT;
			}
			else
				lara->goalAnimState = KS_POSE;
		}

		else if (!(TrInput & IN_RIGHT))
			lara->goalAnimState = KS_POSE;

		/* -------- apply velocities */

		if (frame == 7)
		{
			if (kinfo->Forward)
			{
				if ((kinfo->Rot += KAYAK_FWD_ROT) > KAYAK_MAX_TURN)
					kinfo->Rot = KAYAK_MAX_TURN;

				kinfo->Vel += KAYAK_FWD_VEL;
			}

			else if (kinfo->Turn)
			{
				if ((kinfo->Rot += KAYAK_HARD_ROT) > KAYAK_MAX_STAT)
					kinfo->Rot = KAYAK_MAX_STAT;
			}
			else
			{
				if ((kinfo->Rot += KAYAK_LR_ROT) > KAYAK_MAX_LR)
					kinfo->Rot = KAYAK_MAX_LR;

				kinfo->Vel += KAYAK_LR_VEL;
			}
		}

		/* -------- initialise ripple effects */
		if ((frame > 6) && (frame < 24) && (frame & 1))
			DoKayakRipple(kayak, 384, -64);
		break;
		/* --------------------- */
	case KS_BACK:
		/* --------------------- */
		if (!(TrInput & IN_BACK))
			lara->goalAnimState = KS_POSE;

		if ((lara->animNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == 2)
		{
			if (frame == 8)
			{
				kinfo->Rot += KAYAK_FWD_ROT;
				kinfo->Vel -= KAYAK_FWD_VEL;
			}

			if (frame == 31)
			{
				kinfo->Rot -= KAYAK_FWD_ROT;
				kinfo->Vel -= KAYAK_FWD_VEL;
			}

			if ((frame < 15) && (frame & 1))
				DoKayakRipple(kayak, 384, -128);
			else if ((frame >= 20) && (frame <= 34) && (frame & 1))
				DoKayakRipple(kayak, -384, -128);
		}

		break;

	case KS_TURNL:
		if ((!(TrInput & IN_LSTEP)) || ((!kinfo->Vel) && (!Lara.currentXvel) && (!Lara.currentZvel)))
		{
			lara->goalAnimState = KS_POSE;
		}
		else if ((lara->animNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == 26)
		{
			if (kinfo->Vel >= 0)
			{
				if ((kinfo->Rot -= KAYAK_TURN_ROT) < -KAYAK_MAX_TURN)
					kinfo->Rot = -KAYAK_MAX_TURN;

				if ((kinfo->Vel += -KAYAK_TURN_BRAKE) < 0)
					kinfo->Vel = 0;
			}
			if (kinfo->Vel < 0)
			{
				kinfo->Rot += KAYAK_TURN_ROT;

				if ((kinfo->Vel += KAYAK_TURN_BRAKE) > 0)
					kinfo->Vel = 0;
			}

			if (!(Wibble & 3))
				DoKayakRipple(kayak, -256, -256);
		}
		break;

	case KS_TURNR:
		if (!(TrInput & IN_RSTEP) || ((!kinfo->Vel) && (!Lara.currentXvel) && (!Lara.currentZvel)))
		{
			lara->goalAnimState = KS_POSE;
		}
		else if ((lara->animNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == 27)
		{
			if (kinfo->Vel >= 0)
			{
				if ((kinfo->Rot += KAYAK_TURN_ROT) > KAYAK_MAX_TURN)
					kinfo->Rot = KAYAK_MAX_TURN;

				if ((kinfo->Vel += -KAYAK_TURN_BRAKE) < 0)
					kinfo->Vel = 0;
			}
			if (kinfo->Vel < 0)
			{
				kinfo->Rot -= KAYAK_TURN_ROT;

				if ((kinfo->Vel += KAYAK_TURN_BRAKE) > 0)
					kinfo->Vel = 0;
			}

			if (!(Wibble & 3))
				DoKayakRipple(kayak, 256, -256);
		}

		break;
		/* --------------------- */
	case KS_CLIMBIN:
		/* --------------------- */
		if ((lara->animNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + 4) && (frame == 24) && (!(kinfo->Flags & 0x80)))
		{
			__int16* tmp;
			tmp = Lara.meshPtrs[HAND_R];

			Lara.meshPtrs[HAND_R] = Meshes[Objects[ID_KAYAK_LARA_ANIMS].meshIndex + HAND_R];
			Meshes[Objects[ID_KAYAK_LARA_ANIMS].meshIndex + HAND_R] = tmp;
			
			lara->meshBits &= ~LARA_LEG_BITS;
			kinfo->Flags |= 0x80;
		}
		break;

	case KS_JUMPOUT:
		if ((lara->animNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + 14) && (frame == 27) && (kinfo->Flags & 0x80))
		{
			__int16* tmp;
			tmp = Lara.meshPtrs[HAND_R];

			Lara.meshPtrs[HAND_R] = Meshes[Objects[ID_KAYAK_LARA_ANIMS].meshIndex + HAND_R];
			Meshes[Objects[ID_KAYAK_LARA_ANIMS].meshIndex + HAND_R] = tmp;

			lara->meshBits |= LARA_LEG_BITS;
			kinfo->Flags &= ~0x80;
		}

		lara->goalAnimState = lara->requiredAnimState;
		break;

	case KS_CLIMBOUTL:
		if ((lara->animNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + 24) && (frame == 83))
		{
			PHD_VECTOR vec = { 0, 350, 500 };

			GetLaraJointPosition(&vec, HIPS);

			lara->pos.xPos = vec.x;
			lara->pos.yPos = vec.y;
			lara->pos.zPos = vec.z;
			lara->pos.xRot = 0;
			lara->pos.yRot = kayak->pos.yRot - 0x4000;
			lara->pos.zRot = 0;

			lara->animNumber = 23;
			lara->frameNumber = Anims[lara->animNumber].frameBase;
			lara->currentAnimState = lara->goalAnimState = 9;

			lara->fallspeed = 0;
			lara->gravityStatus = true;
			Lara.gunStatus = LG_NO_ARMS;
			g_LaraExtra.Vehicle = NO_ITEM;
		}
		break;

	case KS_CLIMBOUTR:
		if ((lara->animNumber ==Objects[ID_KAYAK_LARA_ANIMS].animIndex + 32) && (frame == 83))
		{
			PHD_VECTOR vec = { 0, 350, 500 };

			GetLaraJointPosition(&vec, HIPS);

			lara->pos.xPos = vec.x;
			lara->pos.yPos = vec.y;
			lara->pos.zPos = vec.z;
			lara->pos.xRot = 0;
			lara->pos.yRot = kayak->pos.yRot + 0x4000;
			lara->pos.zRot = 0;

			lara->animNumber = 23;
			lara->frameNumber = Anims[lara->animNumber].frameBase;
			lara->currentAnimState = lara->goalAnimState = 9;

			lara->fallspeed = 0;
			lara->gravityStatus = true;
			Lara.gunStatus = LG_NO_ARMS;
			g_LaraExtra.Vehicle = NO_ITEM;
		}
	}

	/* -------- slow Kayak with friction */
	if (kinfo->Vel > 0)
	{
		if ((kinfo->Vel -= KAYAK_FRICTION) < 0)
			kinfo->Vel = 0;
	}
	else if (kinfo->Vel < 0)
	{
		if ((kinfo->Vel += KAYAK_FRICTION) > 0)
			kinfo->Vel = 0;
	}

	if (kinfo->Vel > MAX_SPEED)
		kinfo->Vel = MAX_SPEED;

	else if (kinfo->Vel < -MAX_SPEED)
		kinfo->Vel = -MAX_SPEED;

	kayak->speed = (kinfo->Vel >> 16);

	/* -------- unwind rotation */

	if (kinfo->Rot >= 0)
	{
		if ((kinfo->Rot -= KAYAK_ROT_FRIC) < 0)
			kinfo->Rot = 0;
	}
	else if (kinfo->Rot < 0)
	{
		if ((kinfo->Rot += KAYAK_ROT_FRIC) > 0)
			kinfo->Rot = 0;
	}
}

void __cdecl KayakToBaddieCollision(ITEM_INFO* kayak)
{
#define TARGET_DIST (WALL_SIZE*2)              // Up to this Distance more Complicated checks are made
	vector<__int16> roomsList;
	__int16 numDoors, *door;

	roomsList.push_back(kayak->roomNumber);

	/* -------- get nearby rooms */
	door = Rooms[kayak->roomNumber].door;
	if (door)
	{
		numDoors = *door;
		door++;
		for (__int32 i = 0; i < numDoors; i++)
		{
			roomsList.push_back(*door);
			door += 16;
		}
	}

	/* -------- collide with all baddies in these rooms */

	for (__int32 i = 0; i < roomsList.size(); i++)
	{
		__int16 item_num;

		item_num = Rooms[roomsList[i]].itemNumber;	// Only do collision with Items on Draw list

		while (item_num != NO_ITEM)
		{
			__int16 nex;
			ITEM_INFO* item;

			item = &Items[item_num];
			nex = item->nextItem;				// Store next Item in list as Current may be deleted!!!

			if (item->collidable && item->status != ITEM_INVISIBLE)
			{
				OBJECT_INFO* object;

				object = &Objects[item->objectNumber];

				if ((object->collision)
				&& ((item->objectNumber == ID_TEETH_SPIKES)
				||  (item->objectNumber == ID_DARTS)
				//||  (item->objectNumber == ID_TEETH_TRAP)
				//|| ((item->objectNumber == BLADE)
				&&  (item->currentAnimState != 1))			// STOP
				//|| ((item->objectNumber == ICICLES)
				&&  (item->currentAnimState != 3))			// LAND
				{
					int x, y, z;

					x = kayak->pos.xPos - item->pos.xPos;
					y = kayak->pos.yPos - item->pos.yPos;
					z = kayak->pos.zPos - item->pos.zPos;

					if ((x > -TARGET_DIST)
					&&  (x < TARGET_DIST)
					&&  (z > -TARGET_DIST)
					&&  (z < TARGET_DIST)
					&&  (y > -TARGET_DIST)
					&&  (y < TARGET_DIST))
					{
						if (TestBoundsCollide(item, kayak, KAYAK_TO_BADDIE_RADIUS))
						{
							DoLotsOfBlood(LaraItem->pos.xPos, LaraItem->pos.yPos - STEP_SIZE, LaraItem->pos.zPos, kayak->speed, kayak->pos.yRot, LaraItem->roomNumber, 3);
							LaraItem->hitPoints -= 5;
						}
					}
				}
			}
			item_num = nex;
		}
	}

#undef TARGET_DIST
}

void __cdecl LaraRapidsDrown()
{
	ITEM_INFO* l = LaraItem;

	l->animNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + 25;
	l->frameNumber = Anims[l->animNumber].frameBase;
	l->currentAnimState = 12;
	l->goalAnimState = 12;
	l->hitPoints = 0;
	l->fallspeed = 0;
	l->gravityStatus = false;
	l->speed = 0;

	AnimateItem(l);

	g_LaraExtra.ExtraAnim = 1;
	Lara.gunStatus = LG_NO_ARMS;
	Lara.gunType = WEAPON_NONE;
	Lara.hitDirection = -1;
}

void __cdecl InitialiseKayak(__int16 item_number)
{
	int i;
	ITEM_INFO* v;
	KAYAK_INFO* Kayak;

	v = &Items[item_number];
	Kayak = (KAYAK_INFO*)GameMalloc(sizeof(KAYAK_INFO));
	v->data = (void*)Kayak;
	Kayak->Vel = 0;
	Kayak->Rot = 0;
	Kayak->Flags = 0;
	Kayak->FallSpeedF = 0;
	Kayak->FallSpeedL = 0;
	Kayak->FallSpeedR = 0;
	Kayak->OldPos = v->pos;
}

void __cdecl DrawKayak(ITEM_INFO* kayak)
{
	kayak->pos.yPos += KAYAK_DRAW_SHIFT;
	DrawAnimatingItem(kayak);
	kayak->pos.yPos -= KAYAK_DRAW_SHIFT;
}

void __cdecl KayakCollision(__int16 item_number, ITEM_INFO* l, COLL_INFO* coll)
{
	int geton;

	if ((l->hitPoints < 0) || (g_LaraExtra.Vehicle != NO_ITEM))
		return;

	if ((geton = GetInKayak(item_number, coll)))
	{
		KAYAK_INFO* Kayak;
		ITEM_INFO* v = &Items[item_number];

		g_LaraExtra.Vehicle = item_number;

		/* -------- throw flare away if using */
		if (Lara.gunType == WEAPON_FLARE)
		{
			CreateFlare(ID_FLARE_ITEM, FALSE);
			UndrawFlaresMeshes();
			Lara.flareControlLeft = 0;
			Lara.requestGunType = Lara.gunType = LG_NO_ARMS;
		}

		/* -------- initiate animation */
		if (geton > 0)
			l->animNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + 3;
		else
			l->animNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + 28;

		l->frameNumber = Anims[l->animNumber].frameBase;
		l->currentAnimState = l->goalAnimState = KS_CLIMBIN;

		Lara.waterStatus = LW_UNDERWATER;
		l->pos.xPos = v->pos.xPos;
		l->pos.yPos = v->pos.yPos;
		l->pos.zPos = v->pos.zPos;
		l->pos.yRot = v->pos.yRot;
		l->pos.xRot = l->pos.zRot = 0;
		l->gravityStatus = true;
		l->speed = 0;
		l->fallspeed = 0;

		if (l->roomNumber != v->roomNumber)
			ItemNewRoom(Lara.itemNumber, v->roomNumber);

		AnimateItem(l);

		Kayak = (KAYAK_INFO*)v->data;
		Kayak->Water = v->pos.yPos;
		Kayak->Flags = 0;
	}
	else
	{
		coll->enableBaddiePush = true;
		ObjectCollision(item_number, l, coll);
	}
}

int __cdecl KayakControl()
{
	int h;
	KAYAK_INFO* Kayak;
	ITEM_INFO* v, * l;
	FLOOR_INFO* floor;
	int ofs, water, lp;
	__int16 room_number;

	l = LaraItem;
	v = &Items[g_LaraExtra.Vehicle];
	Kayak = (KAYAK_INFO*)v->data;

	if (TrInput & IN_LOOK)
		LookUpDown();

	/* -------- update dynamics */
	ofs = v->fallspeed;
	KayakUserInput(v, l, Kayak);
	KayakToBackground(v, Kayak);

	/* -------- determine water level */
	room_number = v->roomNumber;
	floor = GetFloor(v->pos.xPos, v->pos.yPos, v->pos.zPos, &room_number);
	h = GetFloorHeight(floor, v->pos.xPos, v->pos.yPos, v->pos.zPos);
	TestTriggers(TriggerIndex, 0, 0);

	Kayak->Water = water = GetWaterHeight(v->pos.xPos, v->pos.yPos, v->pos.zPos, room_number);
	if (water == NO_HEIGHT)
	{
		Kayak->Water = water = h;
		Kayak->TrueWater = 0;
	}
	else
	{
		Kayak->Water -= 5;
		Kayak->TrueWater = 1;
	}


	if (((ofs - v->fallspeed) > 128) && (v->fallspeed == 0) && (water != NO_HEIGHT))
	{
		long damage;
		if ((damage = (ofs - v->fallspeed)) > 160)
			l->hitPoints -= (damage - 160) << 3;

		KayakSplash(v, ofs - v->fallspeed, water);
	}

	/* -------- move Lara to Kayak pos */
	if (g_LaraExtra.Vehicle != NO_ITEM)
	{
		if (v->roomNumber != room_number)
		{
			ItemNewRoom(g_LaraExtra.Vehicle, room_number);
			ItemNewRoom(Lara.itemNumber, room_number);
		}

		l->pos.xPos = v->pos.xPos;
		l->pos.yPos = v->pos.yPos + KAYAK_DRAW_SHIFT;
		l->pos.zPos = v->pos.zPos;
		l->pos.xRot = v->pos.xRot;
		l->pos.yRot = v->pos.yRot;
		l->pos.zRot = v->pos.zRot >> 1;

		/* -------- animate Lara then Kayak */
		AnimateItem(l);

		v->animNumber = Objects[ID_KAYAK].animIndex + (l->animNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex);
		v->frameNumber = Anims[v->animNumber].frameBase + (l->frameNumber - Anims[l->animNumber].frameBase);
		
		Camera.targetElevation = -ANGLE(30);
		Camera.targetDistance = WALL_SIZE * 2;
	}

	/* -------- process effects */
	/*
	if ((!(Wibble & 15)) && (Kayak->TrueWater))
	{
		DoWake(v, -128, 0, 0);
		DoWake(v, 128, 0, 1);
	}

	if ((wibble & 7))
	{
		if ((!Kayak->TrueWater) && (v->fallspeed < 20))
		{
			PHD_VECTOR dest;
			static char cnt = 0;
			static __int16 MistZPos[10] = { 900, 750, 600, 450, 300, 150, 0,  -150, -300, -450 };
			//			static sint16 MistXPos[10] = { 32,  64,  128, 192, 256, 480, 480, 350,  256,  64};
			static __int16 MistXPos[10] = { 32,  96,  170, 220, 300, 400, 400, 300,  200,  64 };

			cnt ^= 1;

			for (lp = cnt; lp < 10; lp += 2)
			{
				//dest.x = (GetRandomControl()%MistXPos[lp]) - (MistXPos[lp]>>1);
				if (GetRandomControl() & 1)
					dest.x = (MistXPos[lp] >> 1);
				else
					dest.x = -(MistXPos[lp] >> 1);
				dest.y = 50;
				dest.z = MistZPos[lp];

				phd_PushUnitMatrix();
				phd_RotYXZ(v->pos.y_rot, v->pos.x_rot, v->pos.z_rot);
				dest.x = v->pos.x_pos + ((*(mptr + M00) * dest.x + *(mptr + M01) * dest.y + *(mptr + M02) * dest.z) >> W2V_SHIFT);
				dest.y = v->pos.y_pos + ((*(mptr + M10) * dest.x + *(mptr + M11) * dest.y + *(mptr + M12) * dest.z) >> W2V_SHIFT);
				dest.z = v->pos.z_pos + ((*(mptr + M20) * dest.x + *(mptr + M21) * dest.y + *(mptr + M22) * dest.z) >> W2V_SHIFT);
				TriggerRapidsMist(dest.x, dest.y, dest.z);
				phd_PopMatrix();
			}

		}
	}
	*/

	KayakToBaddieCollision(v);
	return (g_LaraExtra.Vehicle != NO_ITEM) ? 1 : 0;
}