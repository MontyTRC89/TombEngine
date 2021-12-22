#include "framework.h"
#include "kayak.h"
#include "effects/effects.h"
#include "animation.h"
#include "camera.h"
#include "Lara.h"
#include "collide.h"
#include "lara_flare.h"
#include "items.h"
#include "level.h"
#include "setup.h"
#include "input.h"
#include "control/control.h"
#include "kayak_info.h"

using std::vector;

#define KAYAK_COLLIDE			64
#define GETOFF_DIST 			768
#define KAYAK_TO_BADDIE_RADIUS	256
#define MAX_SPEED		0x380000
#define KAYAK_FRICTION	0x8000
#define KAYAK_ROT_FRIC	0x50000
#define KAYAK_DFLECT_ROT	0x80000
#define KAYAK_FWD_VEL	0x180000
#define KAYAK_FWD_ROT	0x800000
#define KAYAK_LR_VEL	0x100000
#define KAYAK_LR_ROT	0xc00000
#define KAYAK_MAX_LR	0xc00000
#define KAYAK_TURN_ROT	0x200000
#define KAYAK_MAX_TURN	0x1000000
#define KAYAK_TURN_BRAKE	0x8000
#define KAYAK_HARD_ROT	0x1000000
#define KAYAK_MAX_STAT	0x1000000
#define BOAT_SLIP		50
#define BOAT_SIDE_SLIP	50

enum 
{
	STATE_KAYAK_BACK,
	STATE_KAYAK_POSE,
	STATE_KAYAK_LEFT,
	STATE_KAYAK_RIGHT,
	STATE_KAYAK_CLIMBIN,
	STATE_KAYAK_DEATHIN,
	STATE_KAYAK_FORWARD,
	STATE_KAYAK_ROLL,
	STATE_KAYAK_DROWNIN,
	STATE_KAYAK_JUMPOUT,
	STATE_KAYAK_TURNL,
	STATE_KAYAK_TURNR,
	STATE_KAYAK_CLIMBINR,
	STATE_KAYAK_CLIMBOUTL,
	STATE_KAYAK_CLIMBOUTR,
};

#define HIT_BACK	1
#define HIT_FRONT	2
#define HIT_LEFT	3
#define HIT_RIGHT	4

#define KAYAK_BACK_A		2
#define KAYAK_CLIMBIN_A		3
#define KAYAK_CLIMBIN_F		GetFrameNumber(KAYAK_CLIMBIN_A, 0)
#define KAYAK_CLIMBIN2_A	4
#define KAYAK_DEATHIN_A		5
#define KAYAK_FORWARD_A		8
#define KAYAK_2FORWARD_A	9
#define KAYAK_JUMPOUT1_A	14
#define KAYAK_POSE_A		16
#define KAYAK_POSE_F		GetFrameNumber(KAYAK_POSE_A, 0)
#define KAYAK_JUMPOUT2_A	24
#define KAYAK_DROWN_A		25
#define KAYAK_TURNL_A		26
#define KAYAK_TURNR_A		27
#define KAYAK_CLIMBINR_A	28
#define KAYAK_CLIMBINR_F	GetFrameNumber(KAYAK_CLIMBINR_A, 0)
#define KAYAK_JUMPOUTR_A	32
#define KAYAK_DRAW_SHIFT	32
#define LARA_LEG_BITS		((1<<LM_HIPS)|(1<<LM_LTHIGH)|(1<<LM_LSHIN)|(1<<LM_LFOOT)|(1<<LM_RTHIGH)|(1<<LM_RSHIN)|(1<<LM_RFOOT))
#define NUM_WAKE_SPRITES	32
#define WAKE_SIZE 			32
#define WAKE_SPEED 			4
#define KAYAK_X				128
#define KAYAK_Z				128
#define SKIDOO_MAX_KICK		-80
#define SKIDOO_MIN_BOUNCE	((MAX_SPEED/2)/ 256)

struct WAKE_PTS 
{
	int 	x[2];
	int 	y;
	int		z[2];
	short	xvel[2];
	short	zvel[2];
	byte 	life;
	byte	pad[3];
};

WAKE_PTS WakePts[NUM_WAKE_SPRITES][2];
byte CurrentStartWake = 0;
byte WakeShade = 0;

void KayakDoWake(ITEM_INFO* v, short xoff, short zoff, short rotate)
{
	int xv[2], zv[2];
	
	if (WakePts[CurrentStartWake][rotate].life)
		return;

	float s = phd_sin(v->pos.yRot);
	float c = phd_cos(v->pos.yRot);
	
	int x = v->pos.xPos + zoff * s + xoff * c;
	int z = v->pos.zPos + zoff * c - xoff * s;

	short angle1, angle2;

	short roomNumber = v->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, v->pos.yPos, z, &roomNumber);

	if (GetWaterHeight(x, v->pos.yPos, z, roomNumber) != NO_HEIGHT)
	{
		if (v->speed < 0)
		{
			if (!rotate)
			{
				angle1 = v->pos.yRot - ANGLE(10);
				angle2 = v->pos.yRot - ANGLE(30);
			}
			else
			{
				angle1 = v->pos.yRot + ANGLE(10);
				angle2 = v->pos.yRot + ANGLE(30);
			}
		}
		else
		{
			if (!rotate)
			{
				angle1 = v->pos.yRot - ANGLE(170);
				angle2 = v->pos.yRot - ANGLE(150);
			}
			else
			{
				angle1 = v->pos.yRot + ANGLE(170);
				angle2 = v->pos.yRot + ANGLE(150);
			}
		}

		xv[0] = WAKE_SPEED * phd_sin(angle1);
		zv[0] = WAKE_SPEED * phd_cos(angle1);
		xv[1] = (WAKE_SPEED + 2) * phd_sin(angle2);
		zv[1] = (WAKE_SPEED + 2) * phd_cos(angle2);

		WakePts[CurrentStartWake][rotate].y = v->pos.yPos + KAYAK_DRAW_SHIFT;
		WakePts[CurrentStartWake][rotate].life = 0x40;
		
		for (int i = 0; i < 2; i++)
		{
			WakePts[CurrentStartWake][rotate].x[i] = x;
			WakePts[CurrentStartWake][rotate].z[i] = z;
			WakePts[CurrentStartWake][rotate].xvel[i] = xv[i];
			WakePts[CurrentStartWake][rotate].zvel[i] = zv[i];
		}

		if (rotate == 1)
		{
			CurrentStartWake++;
			CurrentStartWake &= (NUM_WAKE_SPRITES - 1);
		}
	}
}

void KayakDoRipple(ITEM_INFO* v, short xoff, short zoff)
{
	float s = phd_sin(v->pos.yRot);
	float c = phd_cos(v->pos.yRot);

	int x = v->pos.xPos + zoff * s + xoff * c;
	int z = v->pos.zPos + zoff * c - xoff * s;

	short roomNumber = v->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, v->pos.yPos, z, &roomNumber);

	if (GetWaterHeight(x, v->pos.yPos, z, roomNumber) != NO_HEIGHT)
	{
		SetupRipple(x, v->pos.yPos, z, -2 - (GetRandomControl() & 1), 0, Objects[ID_KAYAK_PADDLE_TRAIL_SPRITE].meshIndex,TO_RAD(v->pos.yRot));
	}
}

void KayakUpdateWakeFX()
{
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < NUM_WAKE_SPRITES; j++)
		{
			if (WakePts[j][i].life)
			{
				WakePts[j][i].life--;
				WakePts[j][i].x[0] += WakePts[j][i].xvel[0];
				WakePts[j][i].z[0] += WakePts[j][i].zvel[0];
				WakePts[j][i].x[1] += WakePts[j][i].xvel[1];
				WakePts[j][i].z[1] += WakePts[j][i].zvel[1];
			}
		}
	}
}


int KayakGetIn(short itemNumber, COLL_INFO* coll)
{
	ITEM_INFO* l = LaraItem;

	if ((!(TrInput & IN_ACTION))
		|| (Lara.gunStatus != LG_NO_ARMS)
		|| (l->gravityStatus))
		return 0;

	ITEM_INFO* v = &g_Level.Items[itemNumber];

	int x = l->pos.xPos - v->pos.xPos;
	int z = l->pos.zPos - v->pos.zPos;

	int dist = SQUARE(x) + SQUARE(z);
	if (dist > SQUARE(360))
		return 0;

	short roomNumber = v->roomNumber;
	FLOOR_INFO* floor = GetFloor(v->pos.xPos, v->pos.yPos, v->pos.zPos, &roomNumber);
	if (GetFloorHeight(floor, v->pos.xPos, v->pos.yPos, v->pos.zPos) > -32000)
	{
		short ang = phd_atan(v->pos.zPos - l->pos.zPos, v->pos.xPos - l->pos.xPos);
		ang -= v->pos.yRot;

		int tempang;

		tempang = l->pos.yRot - v->pos.yRot;
		if ((ang > -ANGLE(45)) && (ang < ANGLE(135)))
		{
			tempang = l->pos.yRot - v->pos.yRot;
			if (tempang > ANGLE(45) && tempang < ANGLE(135))
				return -1;
		}
		else
		{
			tempang = l->pos.yRot - v->pos.yRot;
			if (tempang > ANGLE(225) && tempang < ANGLE(315))
				return 1;
		}
	}

	return 0;
}

int KayakGetCollisionAnim(ITEM_INFO* v, int xdiff, int zdiff)
{
	xdiff = v->pos.xPos - xdiff;
	zdiff = v->pos.zPos - zdiff;

	if ((xdiff) || (zdiff))
	{
		float s = phd_sin(v->pos.yRot);
		float c = phd_cos(v->pos.yRot);
		
		int front = zdiff * c + xdiff * s;
		int side = -zdiff * s + xdiff * c;

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

int KayakDoDynamics(int height, int fallspeed, int* y)
{
	if (height > * y)
	{
		*y += fallspeed;

		if (*y > height)
		{
			*y = height;
			fallspeed = 0;
		}
		else
			fallspeed += GRAVITY;
	}
	else
	{
		int kick = (height - *y) * 4;

		if (kick < SKIDOO_MAX_KICK)
			kick = SKIDOO_MAX_KICK;

		fallspeed += ((kick - fallspeed) / 8);

		if (*y > height)
			*y = height;
	}

	return fallspeed;
}

void KayakDoCurrent(ITEM_INFO* item)
{
	ROOM_INFO* r = &g_Level.Rooms[item->roomNumber];

	if (!Lara.currentActive)
	{
		int absvel = abs(Lara.currentXvel);
		int shifter;

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

		if (Lara.currentXvel == 0 &&
			Lara.currentZvel == 0)
			return;
	}
	else
	{
		int sinkval = Lara.currentActive - 1;
		
		PHD_VECTOR target;
		target.x = g_Level.Sinks[sinkval].x;
		target.y = g_Level.Sinks[sinkval].y;
		target.z = g_Level.Sinks[sinkval].z;
		
		int angle = (((mGetAngle(target.x, target.z, LaraItem->pos.xPos, LaraItem->pos.zPos) - ANGLE(90))) / 16) & 4095;

		int dx = target.x - LaraItem->pos.xPos;
		int dz = target.z - LaraItem->pos.zPos;

		int speed = g_Level.Sinks[sinkval].strength;
		dx = phd_sin(angle * 16) * speed * 1024;
		dz = phd_cos(angle * 16) * speed * 1024;

		Lara.currentXvel += (dx - Lara.currentXvel) / 16;
		Lara.currentZvel += (dz - Lara.currentZvel) / 16;
	}

	item->pos.xPos += Lara.currentXvel / 256;
	item->pos.zPos += Lara.currentZvel / 256;

	Lara.currentActive = 0;
}

int KayakTestHeight(ITEM_INFO* item, int x, int z, PHD_VECTOR* pos)
{
	Matrix world =
		Matrix::CreateFromYawPitchRoll(TO_RAD(item->pos.yRot), TO_RAD(item->pos.xRot), TO_RAD(item->pos.zRot)) *
		Matrix::CreateTranslation(item->pos.xPos, item->pos.yPos, item->pos.zPos);

	Vector3 vec = Vector3(x, 0, z);
	vec = Vector3::Transform(vec, world);
	
	pos->x = vec.x;
	pos->y = vec.y;
	pos->z = vec.z;

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(pos->x, pos->y, pos->z, &roomNumber);
	
	int h;

	if ((h = GetWaterHeight(pos->x, pos->y, pos->z, roomNumber)) == NO_HEIGHT)
	{
		roomNumber = item->roomNumber;
		floor = GetFloor(pos->x, pos->y, pos->z, &roomNumber);
		if ((h = GetFloorHeight(floor, pos->x, pos->y, pos->z)) == NO_HEIGHT)
			return h;
	}

	return h - 5;
}

bool KayakCanGetOut(ITEM_INFO* v, int direction)
{
	PHD_VECTOR pos;

	int height = KayakTestHeight(v, (direction < 0) ? -GETOFF_DIST : GETOFF_DIST, 0, &pos);

	if ((v->pos.yPos - height) > 0)
		return false;

	return true;
}

int KayakDoShift(ITEM_INFO* v, PHD_VECTOR* pos, PHD_VECTOR* old)
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
		x = z = 0;

		short roomNumber = v->roomNumber;
		FLOOR_INFO* floor = GetFloor(old->x, pos->y, pos->z, &roomNumber);
		int height = GetFloorHeight(floor, old->x, pos->y, pos->z);
		if (height < old->y - STEP_SIZE)
		{
			if (pos->z > old->z)
				z = -shift_z - 1;
			else
				z = WALL_SIZE - shift_z;
		}

		roomNumber = v->roomNumber;
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
			v->pos.zPos += z;
			v->pos.xPos += x;
		}
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
		else
		{
			v->pos.zPos += (old->z - pos->z);
			v->pos.xPos += (old->x - pos->x);
		}
	}

	return 0;
}

void KayakToBackground(ITEM_INFO* v, KAYAK_INFO* Kayak)
{
	int slip = 0, rot = 0;
	PHD_VECTOR pos;
	int height[8];
	PHD_VECTOR oldpos[9];
	PHD_VECTOR fpos, lpos, rpos;

	Kayak->OldPos = v->pos;

	height[0] = KayakTestHeight(v, 0, 1024, &oldpos[0]);
	height[1] = KayakTestHeight(v, -96, 512, &oldpos[1]);
	height[2] = KayakTestHeight(v, 96, 512, &oldpos[2]);
	height[3] = KayakTestHeight(v, -128, 128, &oldpos[3]);
	height[4] = KayakTestHeight(v, 128, 128, &oldpos[4]);
	height[5] = KayakTestHeight(v, -128, -320, &oldpos[5]);
	height[6] = KayakTestHeight(v, 128, -320, &oldpos[6]);
	height[7] = KayakTestHeight(v, 0, -640, &oldpos[7]);

	for (int i = 0; i < 8; i++)
	{
		if (oldpos[i].y > height[i])
			oldpos[i].y = height[i];
	}

	oldpos[8].x = v->pos.xPos;
	oldpos[8].y = v->pos.yPos;
	oldpos[8].z = v->pos.zPos;
 
	int fh = KayakTestHeight(v, 0, 1024, &fpos);
	int lh = KayakTestHeight(v, -KAYAK_X, KAYAK_Z, &lpos);
	int rh = KayakTestHeight(v, KAYAK_X, KAYAK_Z, &rpos);

	v->pos.yRot += (Kayak->Rot / 65536);

	v->pos.xPos += v->speed * phd_sin(v->pos.yRot);
	v->pos.zPos += v->speed * phd_cos(v->pos.yRot);

	KayakDoCurrent(v);

	Kayak->FallSpeedL = KayakDoDynamics(lh, Kayak->FallSpeedL, &lpos.y);
	Kayak->FallSpeedR = KayakDoDynamics(rh, Kayak->FallSpeedR, &rpos.y);
	Kayak->FallSpeedF = KayakDoDynamics(fh, Kayak->FallSpeedF, &fpos.y);

	v->fallspeed = KayakDoDynamics(Kayak->Water, v->fallspeed, &v->pos.yPos);

	int h = (lpos.y + rpos.y) / 2;
	int x = phd_atan(1024, v->pos.yPos - fpos.y);
	int z = phd_atan(KAYAK_X, h - lpos.y);

	v->pos.xRot = x;
	v->pos.zRot = z;

	int oldx = v->pos.xPos;
	int oldz = v->pos.zPos;

	if ((h = KayakTestHeight(v, 0, -640, &pos)) < (oldpos[7].y - KAYAK_COLLIDE))
		rot = KayakDoShift(v, &pos, &oldpos[7]);

	if ((h = KayakTestHeight(v, 128, -320, &pos)) < (oldpos[6].y - KAYAK_COLLIDE))
		rot += KayakDoShift(v, &pos, &oldpos[6]);

	if ((h = KayakTestHeight(v, -128, -320, &pos)) < (oldpos[5].y - KAYAK_COLLIDE))
		rot += KayakDoShift(v, &pos, &oldpos[5]);

	if ((h = KayakTestHeight(v, 128, 128, &pos)) < (oldpos[4].y - KAYAK_COLLIDE))
		rot += KayakDoShift(v, &pos, &oldpos[4]);

	if ((h = KayakTestHeight(v, -128, 128, &pos)) < (oldpos[3].y - KAYAK_COLLIDE))
		rot += KayakDoShift(v, &pos, &oldpos[3]);

	if ((h = KayakTestHeight(v, 96, 512, &pos)) < (oldpos[2].y - KAYAK_COLLIDE))
		rot += KayakDoShift(v, &pos, &oldpos[2]);

	if ((h = KayakTestHeight(v, -96, 512, &pos)) < (oldpos[1].y - KAYAK_COLLIDE))
		rot += KayakDoShift(v, &pos, &oldpos[1]);

	if ((h = KayakTestHeight(v, 0, 1024, &pos)) < (oldpos[0].y - KAYAK_COLLIDE))
		rot += KayakDoShift(v, &pos, &oldpos[0]);

	v->pos.yRot += rot;

	short roomNumber = v->roomNumber;
	FLOOR_INFO* floor = GetFloor(v->pos.xPos, v->pos.yPos, v->pos.zPos, &roomNumber);
	h = GetWaterHeight(v->pos.xPos, v->pos.yPos, v->pos.zPos, roomNumber);

	if (h == NO_HEIGHT)
		h = GetFloorHeight(floor, v->pos.xPos, v->pos.yPos, v->pos.zPos);

	if (h < (v->pos.yPos - KAYAK_COLLIDE))
		KayakDoShift(v, (PHD_VECTOR*)&v->pos, &oldpos[8]);

	roomNumber = v->roomNumber;
	floor = GetFloor(v->pos.xPos, v->pos.yPos, v->pos.zPos, &roomNumber);
	h = GetWaterHeight(v->pos.xPos, v->pos.yPos, v->pos.zPos, roomNumber);

	if (h == NO_HEIGHT)
		h = GetFloorHeight(floor, v->pos.xPos, v->pos.yPos, v->pos.zPos);

	if (h == NO_HEIGHT)
	{
		GAME_VECTOR kpos;

		kpos.x = Kayak->OldPos.xPos;
		kpos.y = Kayak->OldPos.yPos;
		kpos.z = Kayak->OldPos.zPos;
		kpos.roomNumber = v->roomNumber;

		CameraCollisionBounds(&kpos, 256, 0);
		{
			v->pos.xPos = kpos.x;
			v->pos.yPos = kpos.y;
			v->pos.zPos = kpos.z;
			v->roomNumber = kpos.roomNumber;
		}
	}

	int collide = KayakGetCollisionAnim(v, oldx, oldz);

	if (slip || collide)
	{
		int newspeed;

		newspeed = (v->pos.zPos - oldpos[8].z) * phd_cos(v->pos.yRot) + (v->pos.xPos - oldpos[8].x) * phd_sin(v->pos.yRot);

		newspeed *= 256;

		if (slip)
		{
			if (Kayak->Vel <= MAX_SPEED)
				Kayak->Vel = newspeed;
		}
		else
		{
			if (Kayak->Vel > 0 && newspeed < Kayak->Vel)
				Kayak->Vel = newspeed;

			else if (Kayak->Vel < 0 && newspeed > Kayak->Vel)
				Kayak->Vel = newspeed;
		}

		if (Kayak->Vel < -MAX_SPEED)
			Kayak->Vel = -MAX_SPEED;
	}
}

void KayakUserInput(ITEM_INFO* v, ITEM_INFO* l, KAYAK_INFO* Kayak)
{
	short frame;

	if ((l->hitPoints <= 0)
		&& (l->currentAnimState != STATE_KAYAK_DEATHIN))
	{
		l->animNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_DEATHIN_A;
		l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
		l->currentAnimState = l->goalAnimState = STATE_KAYAK_DEATHIN;
	}

	frame = l->frameNumber - g_Level.Anims[l->animNumber].frameBase;


	switch (l->currentAnimState)
	{
		static char lr;
	case STATE_KAYAK_POSE:
		if ((TrInput & IN_ROLL)
			&& (!Lara.currentActive)
			&& (!Lara.currentXvel)
			&& (!Lara.currentZvel))
		{
			if ((TrInput & IN_LEFT) && (KayakCanGetOut(v, -1)))
			{
				l->goalAnimState = STATE_KAYAK_JUMPOUT;
				l->requiredAnimState = STATE_KAYAK_CLIMBOUTL;
			}

			else if ((TrInput & IN_RIGHT) && (KayakCanGetOut(v, 1)))
			{
				l->goalAnimState = STATE_KAYAK_JUMPOUT;
				l->requiredAnimState = STATE_KAYAK_CLIMBOUTR;
			}
		}
		else if (TrInput & IN_FORWARD)
		{
			l->goalAnimState = STATE_KAYAK_RIGHT;
			Kayak->Turn = 0;
			Kayak->Forward = 1;
		}
		else if (TrInput & IN_BACK)
		{
			l->goalAnimState = STATE_KAYAK_BACK;
		}
		else if (TrInput & IN_LEFT)
		{
			l->goalAnimState = STATE_KAYAK_LEFT;

			if (Kayak->Vel)
				Kayak->Turn = 0;
			else
				Kayak->Turn = 1;

			Kayak->Forward = 0;
		}

		else if (TrInput & IN_RIGHT)
		{
			l->goalAnimState = STATE_KAYAK_RIGHT;

			if (Kayak->Vel)
				Kayak->Turn = 0;
			else
				Kayak->Turn = 1;

			Kayak->Forward = 0;
		}
		else if ((TrInput & IN_LSTEP)
			&& ((Kayak->Vel)
				|| (Lara.currentXvel)
				|| (Lara.currentZvel)))
		{
			l->goalAnimState = STATE_KAYAK_TURNL;
		}
		else if ((TrInput & IN_RSTEP)
			&& ((Kayak->Vel)
				|| (Lara.currentXvel)
				|| (Lara.currentZvel)))
		{
			l->goalAnimState = STATE_KAYAK_TURNR;
		}

		break;
		
	case STATE_KAYAK_LEFT:
		
		if (Kayak->Forward)
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
						l->goalAnimState = STATE_KAYAK_RIGHT;
				}
				else
					l->goalAnimState = STATE_KAYAK_RIGHT;
			}
			else
				l->goalAnimState = STATE_KAYAK_POSE;
		}
		else if (!(TrInput & IN_LEFT))
			l->goalAnimState = STATE_KAYAK_POSE;

	
		if (frame == 7)
		{
			if (Kayak->Forward)
			{
				if ((Kayak->Rot -= KAYAK_FWD_ROT) < -KAYAK_MAX_TURN)
					Kayak->Rot = -KAYAK_MAX_TURN;

				Kayak->Vel += KAYAK_FWD_VEL;
			}

			else if (Kayak->Turn)
			{
				if ((Kayak->Rot -= KAYAK_HARD_ROT) < -KAYAK_MAX_STAT)
					Kayak->Rot = -KAYAK_MAX_STAT;
			}
			else
			{
				if ((Kayak->Rot -= KAYAK_LR_ROT) < -KAYAK_MAX_LR)
					Kayak->Rot = -KAYAK_MAX_LR;

				Kayak->Vel += KAYAK_LR_VEL;
			}
		}


		if ((frame > 6) && (frame < 24) && (frame & 1))
			KayakDoRipple(v, -384, -64);

		break;
		
	case STATE_KAYAK_RIGHT:	
		if (Kayak->Forward)
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
						l->goalAnimState = STATE_KAYAK_LEFT;
				}
				else
					l->goalAnimState = STATE_KAYAK_LEFT;
			}
			else
				l->goalAnimState = STATE_KAYAK_POSE;
		}

		else if (!(TrInput & IN_RIGHT))
			l->goalAnimState = STATE_KAYAK_POSE;

		if (frame == 7)
		{
			if (Kayak->Forward)
			{
				if ((Kayak->Rot += KAYAK_FWD_ROT) > KAYAK_MAX_TURN)
					Kayak->Rot = KAYAK_MAX_TURN;

				Kayak->Vel += KAYAK_FWD_VEL;
			}

			else if (Kayak->Turn)
			{
				if ((Kayak->Rot += KAYAK_HARD_ROT) > KAYAK_MAX_STAT)
					Kayak->Rot = KAYAK_MAX_STAT;
			}
			else
			{
				if ((Kayak->Rot += KAYAK_LR_ROT) > KAYAK_MAX_LR)
					Kayak->Rot = KAYAK_MAX_LR;

				Kayak->Vel += KAYAK_LR_VEL;
			}
		}

		if ((frame > 6) && (frame < 24) && (frame & 1))
			KayakDoRipple(v, 384, -64);

		break;
		
	case STATE_KAYAK_BACK:
		
		if (!(TrInput & IN_BACK))
			l->goalAnimState = STATE_KAYAK_POSE;

		if ((l->animNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == KAYAK_BACK_A)
		{
			if (frame == 8)
			{
				Kayak->Rot += KAYAK_FWD_ROT;
				Kayak->Vel -= KAYAK_FWD_VEL;
			}

			if (frame == 31)
			{
				Kayak->Rot -= KAYAK_FWD_ROT;
				Kayak->Vel -= KAYAK_FWD_VEL;
			}

			if ((frame < 15) && (frame & 1))
				KayakDoRipple(v, 384, -128);

			else if ((frame >= 20) && (frame <= 34) && (frame & 1))
				KayakDoRipple(v, -384, -128);
		}

		break;
		
	case STATE_KAYAK_TURNL:
		
		if ((!(TrInput & IN_LSTEP))
			|| ((!Kayak->Vel)
				&& (!Lara.currentXvel)
				&& (!Lara.currentZvel)))
		{
			l->goalAnimState = STATE_KAYAK_POSE;
		}
		else if ((l->animNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == KAYAK_TURNL_A)
		{
			if (Kayak->Vel >= 0)
			{
				if ((Kayak->Rot -= KAYAK_TURN_ROT) < -KAYAK_MAX_TURN)
					Kayak->Rot = -KAYAK_MAX_TURN;

				if ((Kayak->Vel += -KAYAK_TURN_BRAKE) < 0)
					Kayak->Vel = 0;
			}
			if (Kayak->Vel < 0)
			{
				Kayak->Rot += KAYAK_TURN_ROT;

				if ((Kayak->Vel += KAYAK_TURN_BRAKE) > 0)
					Kayak->Vel = 0;
			}

			if (!(Wibble & 3))
				KayakDoRipple(v, -256, -256);
		}

		break;
		
	case STATE_KAYAK_TURNR:
		if ((!(TrInput & IN_RSTEP))
			|| ((!Kayak->Vel)
				&& (!Lara.currentXvel)
				&& (!Lara.currentZvel)))
		{
			l->goalAnimState = STATE_KAYAK_POSE;
		}
		else if ((l->animNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex) == KAYAK_TURNR_A)
		{
			if (Kayak->Vel >= 0)
			{
				if ((Kayak->Rot += KAYAK_TURN_ROT) > KAYAK_MAX_TURN)
					Kayak->Rot = KAYAK_MAX_TURN;

				if ((Kayak->Vel += -KAYAK_TURN_BRAKE) < 0)
					Kayak->Vel = 0;
			}
			if (Kayak->Vel < 0)
			{
				Kayak->Rot -= KAYAK_TURN_ROT;

				if ((Kayak->Vel += KAYAK_TURN_BRAKE) > 0)
					Kayak->Vel = 0;
			}

			if (!(Wibble & 3))
				KayakDoRipple(v, 256, -256);
		}

		break;
		
	case STATE_KAYAK_CLIMBIN:
		
		if ((l->animNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_CLIMBIN2_A)
			&& (frame == 24)
			&& (!(Kayak->Flags & 0x80)))
		{
			Lara.meshPtrs[LM_RHAND] = Objects[ID_KAYAK_LARA_ANIMS].meshIndex + LM_RHAND;
			l->meshBits &= ~LARA_LEG_BITS;

			Kayak->Flags |= 0x80;
		}
		break;
		
	case STATE_KAYAK_JUMPOUT:
		
		if ((l->animNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_JUMPOUT1_A)
			&& (frame == 27)
			&& (Kayak->Flags & 0x80))
		{
			Lara.meshPtrs[LM_RHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_RHAND;
			l->meshBits |= LARA_LEG_BITS;

			Kayak->Flags &= ~0x80;
		}

		l->goalAnimState = l->requiredAnimState;

		break;
		
	case STATE_KAYAK_CLIMBOUTL:
		
		if ((l->animNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_JUMPOUT2_A)
			&& (frame == 83))
		{
			PHD_VECTOR vec = { 0, 350, 500 };

			GetLaraJointPosition(&vec, LM_HIPS);

			l->pos.xPos = vec.x;
			l->pos.yPos = vec.y;
			l->pos.zPos = vec.z;
			l->pos.xRot = 0;
			l->pos.yRot = v->pos.yRot - ANGLE(90);
			l->pos.zRot = 0;

			l->animNumber = LA_FREEFALL;
			l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
			l->currentAnimState = l->goalAnimState = LS_FREEFALL;

			l->fallspeed = 0;
			l->gravityStatus = true;
			Lara.gunStatus = LG_NO_ARMS;
			Lara.Vehicle = NO_ITEM;
		}
		break;
		
	case STATE_KAYAK_CLIMBOUTR:
		
		if ((l->animNumber == Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_JUMPOUTR_A)
			&& (frame == 83))
		{
			PHD_VECTOR vec = { 0, 350, 500 };

			GetLaraJointPosition(&vec, LM_HIPS);

			l->pos.xPos = vec.x;
			l->pos.yPos = vec.y;
			l->pos.zPos = vec.z;
			l->pos.xRot = 0;
			l->pos.yRot = v->pos.yRot + ANGLE(90);
			l->pos.zRot = 0;

			l->animNumber = LA_FREEFALL;
			l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
			l->currentAnimState = l->goalAnimState = LS_FREEFALL;

			l->fallspeed = 0;
			l->gravityStatus = true;
			Lara.gunStatus = LG_NO_ARMS;
			Lara.Vehicle = NO_ITEM;
		}
	}

	if (Kayak->Vel > 0)
	{
		if ((Kayak->Vel -= KAYAK_FRICTION) < 0)
			Kayak->Vel = 0;
	}
	else if (Kayak->Vel < 0)
	{
		if ((Kayak->Vel += KAYAK_FRICTION) > 0)
			Kayak->Vel = 0;
	}

	if (Kayak->Vel > MAX_SPEED)
		Kayak->Vel = MAX_SPEED;

	else if (Kayak->Vel < -MAX_SPEED)
		Kayak->Vel = -MAX_SPEED;

	v->speed = (Kayak->Vel / 65536);

	if (Kayak->Rot >= 0)
	{
		if ((Kayak->Rot -= KAYAK_ROT_FRIC) < 0)
			Kayak->Rot = 0;
	}
	else if (Kayak->Rot < 0)
	{
		if ((Kayak->Rot += KAYAK_ROT_FRIC) > 0)
			Kayak->Rot = 0;
	}
}

void KayakToBaddieCollision(ITEM_INFO* v)
{
	short roomsToCheck[128];
	short numRoomsToCheck = 0;
	roomsToCheck[numRoomsToCheck++] = v->roomNumber;

	ROOM_INFO* room = &g_Level.Rooms[v->roomNumber];
	for (int i = 0; i < room->doors.size(); i++)
	{
		roomsToCheck[numRoomsToCheck++] = room->doors[i].room;
	}

	for (int i = 0; i < numRoomsToCheck; i++)
	{
		short itemNum = g_Level.Rooms[roomsToCheck[i]].itemNumber; 

		while (itemNum != NO_ITEM)
		{
			ITEM_INFO* item = &g_Level.Items[itemNum];
			short nex = item->nextItem;	 

			if ((item->collidable) && (item->status != ITEM_INVISIBLE))
			{
				OBJECT_INFO* object = &Objects[item->objectNumber];

				if (object->collision 
					&& (item->objectNumber == ID_TEETH_SPIKES
						|| item->objectNumber == ID_DARTS
							&& item->currentAnimState != 1))
				{
					int x = v->pos.xPos - item->pos.xPos;
					int y = v->pos.yPos - item->pos.yPos;
					int z = v->pos.zPos - item->pos.zPos;

					if ((x > -2048)
						&& (x < 2048)
						&& (z > -2048)
						&& (z < 2048)
						&& (y > -2048)
						&& (y < 2048))
					{
						if (TestBoundsCollide(item, v, KAYAK_TO_BADDIE_RADIUS))
						{
							DoLotsOfBlood(LaraItem->pos.xPos, LaraItem->pos.yPos - STEP_SIZE, LaraItem->pos.zPos, v->speed, v->pos.yRot, LaraItem->roomNumber, 3);
							LaraItem->hitPoints -= 5;
						}
					}
				}
			}
			itemNum = nex;
		}
	}
}

void KayakLaraRapidsDrown()
{
	ITEM_INFO* l = LaraItem;

	l->animNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_DROWN_A;
	l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
	l->currentAnimState = 12;
	l->goalAnimState = 12;
	l->hitPoints = 0;
	l->fallspeed = 0;
	l->gravityStatus = 0;
	l->speed = 0;

	AnimateItem(l);

	Lara.ExtraAnim = 1;
	Lara.gunStatus = LG_HANDS_BUSY;
	Lara.gunType = WEAPON_NONE;
	Lara.hitDirection = -1;
}

void InitialiseKayak(short itemNumber)
{
	ITEM_INFO* v = &g_Level.Items[itemNumber];
	KAYAK_INFO* kayak;
	v->data = KAYAK_INFO();
	kayak = v->data;

	kayak->Vel = kayak->Rot = kayak->Flags = 0;
	kayak->FallSpeedF = kayak->FallSpeedL = kayak->FallSpeedR = 0;
	kayak->OldPos = v->pos;

	for (int i = 0; i < NUM_WAKE_SPRITES; i++)
	{
		WakePts[i][0].life = 0;
		WakePts[i][1].life = 0;
	}
}

void KayakDraw(ITEM_INFO* v)
{
	DrawAnimatingItem(v);
}

void KayakCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	int geton;

	if ((l->hitPoints < 0)
		|| (Lara.Vehicle != NO_ITEM))
		return;

	if ((geton = KayakGetIn(itemNumber, coll)))
	{
		ITEM_INFO* v = &g_Level.Items[itemNumber];

		Lara.Vehicle = itemNumber;

		if (Lara.gunType == WEAPON_FLARE)
		{
			CreateFlare(LaraItem, ID_FLARE_ITEM, 0);
			UndrawFlareMeshes(l);
			Lara.flareControlLeft = 0;
			Lara.requestGunType = Lara.gunType = WEAPON_NONE;
		}

		if (geton > 0)
			l->animNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_CLIMBIN_A;
		else
			l->animNumber = Objects[ID_KAYAK_LARA_ANIMS].animIndex + KAYAK_CLIMBINR_A;

		l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
		l->currentAnimState = l->goalAnimState = STATE_KAYAK_CLIMBIN;

		Lara.waterStatus = LW_ABOVE_WATER;
		l->pos.xPos = v->pos.xPos;
		l->pos.yPos = v->pos.yPos;
		l->pos.zPos = v->pos.zPos;
		l->pos.yRot = v->pos.yRot;
		l->pos.xRot = l->pos.zRot = 0;
		l->gravityStatus = false;
		l->speed = 0;
		l->fallspeed = 0;

		if (l->roomNumber != v->roomNumber)
			ItemNewRoom(Lara.itemNumber, v->roomNumber);

		AnimateItem(l);

		KAYAK_INFO* kayak = (KAYAK_INFO*)v->data;
		kayak->Water = v->pos.yPos;
		kayak->Flags = 0;
	}
	else
	{
		coll->Setup.EnableObjectPush = true;
		ObjectCollision(itemNumber, l, coll);
	}
}

int KayakControl()
{
	ITEM_INFO* l = LaraItem;
	ITEM_INFO* v = &g_Level.Items[Lara.Vehicle];
	KAYAK_INFO*  kayak = (KAYAK_INFO*)v->data;

	if (TrInput & IN_LOOK)
		LookUpDown();

	int ofs = v->fallspeed;

	KayakUserInput(v, l, kayak);

	KayakToBackground(v, kayak);

	short roomNumber = v->roomNumber;
	FLOOR_INFO* floor = GetFloor(v->pos.xPos, v->pos.yPos, v->pos.zPos, &roomNumber);
	int h = GetFloorHeight(floor, v->pos.xPos, v->pos.yPos, v->pos.zPos);

	TestTriggers(v, false);

	int water;
	if ((kayak->Water = water = GetWaterHeight(v->pos.xPos, v->pos.yPos, v->pos.zPos, roomNumber)) == NO_HEIGHT)
	{
		kayak->Water = water = h;
		kayak->TrueWater = 0;
	}
	else
	{
		kayak->Water -= 5;
		kayak->TrueWater = 1;
	}


	if (((ofs - v->fallspeed) > 128)
		&& (v->fallspeed == 0)
		&& (water != NO_HEIGHT))
	{
		int damage;
		if ((damage = (ofs - v->fallspeed)) > 160)
			l->hitPoints -= (damage - 160) * 8;

	}

	if (Lara.Vehicle != NO_ITEM)
	{
		if (v->roomNumber != roomNumber)
		{
			ItemNewRoom(Lara.Vehicle, roomNumber);
			ItemNewRoom(Lara.itemNumber, roomNumber);
		}

		l->pos.xPos = v->pos.xPos;
		l->pos.yPos = v->pos.yPos;;
		l->pos.zPos = v->pos.zPos;
		l->pos.xRot = v->pos.xRot;
		l->pos.yRot = v->pos.yRot;
		l->pos.zRot = v->pos.zRot / 2;

		AnimateItem(l);

		v->animNumber = Objects[ID_KAYAK].animIndex + (l->animNumber - Objects[ID_KAYAK_LARA_ANIMS].animIndex);
		v->frameNumber = g_Level.Anims[v->animNumber].frameBase + (l->frameNumber - g_Level.Anims[l->animNumber].frameBase);

		Camera.targetElevation = -ANGLE(30);
		Camera.targetDistance = WALL_SIZE * 2;
	}

	if ((!(Wibble & 15)) && (kayak->TrueWater))
	{
		KayakDoWake(v, -128, 0, 0);
		KayakDoWake(v, 128, 0, 1);
	}


	if ((Wibble & 7))
	{
		if ((!kayak->TrueWater) && (v->fallspeed < 20))
		{
			PHD_VECTOR dest;
			char cnt = 0;
			short MistZPos[10] = { 900, 750, 600, 450, 300, 150, 0,  -150, -300, -450 };
			short MistXPos[10] = { 32,  96,  170, 220, 300, 400, 400, 300,  200,  64 };

			cnt ^= 1;

			for (int i = cnt; i < 10; i += 2)
			{
				if (GetRandomControl() & 1)
					dest.x = (MistXPos[i] / 2);
				else
					dest.x = -(MistXPos[i] / 2);
				dest.y = 50;
				dest.z = MistZPos[i];
			}

		}
	}
	if ((v->speed == 0)
		&& (!Lara.currentXvel)
		&& (!Lara.currentZvel))
	{
		if (WakeShade)
			WakeShade--;
	}
	else
	{
		if (WakeShade < 16)
			WakeShade++;
	}

	KayakUpdateWakeFX();

	KayakToBaddieCollision(v);

	return (Lara.Vehicle != NO_ITEM) ? 1 : 0;
}
