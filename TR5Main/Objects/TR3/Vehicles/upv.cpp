#include "framework.h"
#include "upv.h"
#include "lara.h"
#include "items.h"
#include "sphere.h"
#include "effect2.h"
#include "effect.h"
#include "collide.h"
#include "box.h"
#include "laraflar.h"
#include "draw.h"
#include "tomb4fx.h"
#include "misc.h"
#include "camera.h"
#include "setup.h"
#include "bubble.h"
#include "level.h"
#include "input.h"
#include "savegame.h"
#include "sound.h"

enum UPV_FLAG
{
	UPV_CONTROL = 1,
	UPV_SURFACE = 2,
	UPV_DIVE = 4,
	UPV_DEAD = 8
};

#define ACCELERATION		0x40000
#define FRICTION			0x18000
#define MAX_SPEED			0x400000
#define ROT_ACCELERATION	0x400000
#define ROT_SLOWACCEL		0x200000
#define ROT_FRICTION 		0x100000
#define MAX_ROTATION		0x1c00000
#define UPDOWN_ACCEL		(ANGLE(2) << 16)
#define UPDOWN_SLOWACCEL	(ANGLE(1) << 16)
#define UPDOWN_FRICTION		(ANGLE(1) << 16)
#define MAX_UPDOWN			(ANGLE(2) << 16)
#define UPDOWN_LIMIT		ANGLE(80)
#define UPDOWN_SPEED		10
#define SURFACE_DIST		210
#define SURFACE_ANGLE		ANGLE(50)
#define DIVE_ANGLE			ANGLE(15)
#define DIVE_SPEED			ANGLE(5)
#define SUB_DRAW_SHIFT		128
#define SUB_RADIUS			300
#define SUB_HEIGHT			400
#define SUB_LENGTH			WALL_SIZE
#define FRONT_TOLERANCE		(ANGLE(45) << 16)
#define TOP_TOLERANCE		(ANGLE(45) << 16)
#define WALLDEFLECT			(ANGLE(2) << 16)
#define GETOFF_DIST 		WALL_SIZE
#define HARPOON_SPEED		256
#define HARPOON_TIME		256
#define HARPOON_RELOAD		15

BITE_INFO sub_bites[6] = {
	{ 0, 0, 0, 3 },			// Fan.
	{ 0, 96, 256, 0 },		// Front light.
	{ -128, 0, -64, 1 },	// Left Fin Left.
	{ 0, 0, -64, 1 },		// Left Fin right.
	{ 128, 0, -64, 2 },		// Right Fin Right.
	{ 0, 0, -64, 2 }		// Right fin left.
};

enum SUB_BITE_FLAG {
	SUB_FAN = 0,
	SUB_FRONT_LIGHT,
	SUB_LEFT_FIN_LEFT,
	SUB_LEFT_FIN_RIGHT,
	SUB_RIGHT_FIN_RIGHT,
	SUB_RIGHT_FIN_LEFT
};

enum UPV_STATE {
	SUBS_DIE,
	SUBS_HIT,
	SUBS_GETOFFS,
	SUBS_1,
	SUBS_MOVE,
	SUBS_POSE,
	SUBS_2,
	SUBS_3,
	SUBS_GETON,
	SUBS_GETOFF
};

static void FireSubHarpoon(ITEM_INFO* v)
{
	short itemNum;

	itemNum = CreateItem();
	if (itemNum != NO_ITEM)
	{
		PHD_VECTOR pos;
		ITEM_INFO* item;
		static char lr = 0;

		item = &Items[itemNum];

		item->objectNumber = ID_HARPOON;
		item->shade = 0x4210 | 0x8000;
		item->roomNumber = v->roomNumber;

		pos.x = (lr) ? 22 : -22;
		pos.y = 24;
		pos.z = 230;
		GetJointAbsPosition(v, &pos, 3);

		item->pos.xPos = pos.x;
		item->pos.yPos = pos.y;
		item->pos.zPos = pos.z;
		InitialiseItem(itemNum);

		item->pos.xRot = v->pos.xRot;
		item->pos.yRot = v->pos.yRot;
		item->pos.zRot = 0;

		item->fallspeed = (short)(-HARPOON_SPEED * phd_sin(item->pos.xRot) >> W2V_SHIFT);
		item->speed =     (short)(HARPOON_SPEED * phd_cos(item->pos.xRot) >> W2V_SHIFT);
		item->hitPoints = HARPOON_TIME;
		item->itemFlags[0] = 1;

		AddActiveItem(itemNum);

		SoundEffect(SFX_TR3_LARA_HARPOON_FIRE_WATER, &LaraItem->pos, 2);

		// if lara have ammo, reduce it.
		if (Lara.Weapons[WEAPON_HARPOON_GUN].Ammo[0])
			Lara.Weapons[WEAPON_HARPOON_GUN].Ammo[0]--;
		Savegame.Game.AmmoUsed++;

		lr ^= 1;
	}
}

static void TriggerSubMist(long x, long y, long z, long speed, short angle)
{
	long size, xv, zv;
	SPARKS* sptr;

	sptr = &Sparks[GetFreeSpark()];

	sptr->on = 1;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;

	sptr->dR = 64;
	sptr->dG = 64;
	sptr->dB = 64;

	sptr->colFadeSpeed = 4 + (GetRandomControl() & 3);
	sptr->fadeToBlack = 12;
	sptr->sLife = sptr->life = (GetRandomControl() & 3) + 20;
	sptr->transType = 2;
	sptr->extras = 0;
	sptr->dynamic = -1;

	sptr->x = x + ((GetRandomControl() & 15) - 8);
	sptr->y = y + ((GetRandomControl() & 15) - 8);
	sptr->z = z + ((GetRandomControl() & 15) - 8);
	zv = (speed * phd_cos(angle)) >> (W2V_SHIFT + 2);
	xv = (speed * phd_sin(angle)) >> (W2V_SHIFT + 2);
	sptr->xVel = xv + ((GetRandomControl() & 127) - 64);
	sptr->yVel = 0;
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

	sptr->scalar = 3;
	sptr->gravity = sptr->maxYvel = 0;
	size = (GetRandomControl() & 7) + (speed >> 1) + 16;
	sptr->size = sptr->sSize = size >> 2;
	sptr->dSize = size;
}

static void SubEffects(short item_number)
{
	ITEM_INFO* v;
	SUB_INFO* sub;
	PHD_VECTOR pos;
	long lp;

	v = &Items[item_number];
	sub = (SUB_INFO*)v->data;

	/* -------- Lara is using this vehicle */

	if (Lara.Vehicle == item_number)
	{
		if (!sub->Vel)
			sub->FanRot += ANGLE(2);
		else
			sub->FanRot += sub->Vel >> 12;

		if (sub->Vel)
		{
			pos.x = sub_bites[SUB_FAN].x;
			pos.y = sub_bites[SUB_FAN].y;
			pos.z = sub_bites[SUB_FAN].z;
			GetJointAbsPosition(v, &pos, sub_bites[SUB_FAN].meshNum);
			TriggerSubMist(pos.x, pos.y + SUB_DRAW_SHIFT, pos.z, abs(sub->Vel) >> 16, v->pos.yRot + 0x8000);

			if ((GetRandomControl() & 1) == 0)
			{
				PHD_3DPOS pos3d;
				short roomNumber;

				pos3d.xPos = pos.x + (GetRandomControl() & 63) - 32;
				pos3d.yPos = pos.y + SUB_DRAW_SHIFT;
				pos3d.zPos = pos.z + (GetRandomControl() & 63) - 32;
				roomNumber = v->roomNumber;
				GetFloor(pos3d.xPos, pos3d.yPos, pos3d.zPos, &roomNumber);
				CreateBubble((PHD_VECTOR*)&pos3d, roomNumber, 4, 8, BUBBLE_FLAG_CLUMP, 0, 0, 0); // CHECK
			}
		}
	}

	// TODO: enable the light for UPV

	/*
	for (lp = 0; lp < 2; lp++)
	{
		GAME_VECTOR	source, target;
		long		r;

		r = 31 - (GetRandomControl() & 3);
		pos.x = sub_bites[SUB_FRONT_LIGHT].x;
		pos.y = sub_bites[SUB_FRONT_LIGHT].y;
		pos.z = sub_bites[SUB_FRONT_LIGHT].z << (lp * 6);
		GetJointAbsPosition(v, &pos, sub_bites[SUB_FRONT_LIGHT].meshNum);

		if (lp == 1)	// LOS light?
		{
			target.x = pos.x;
			target.y = pos.y;
			target.z = pos.z;
			target.roomNumber = v->roomNumber;
			LOS(&source, &target);
			pos.x = target.x;
			pos.y = target.y;
			pos.z = target.z;
		}
		else
		{
			source.x = pos.x;
			source.y = pos.y;
			source.z = pos.z;
			source.roomNumber = v->roomNumber;
		}

		// TODO: enable the light for UPV
		//TriggerDynamic(pos.x, pos.y, pos.z, 16 + (lp << 3), r, r, r);
	}
	*/
}

static int CanGetOff(ITEM_INFO* v)
{
	FLOOR_INFO* floor;
	short roomNumber;
	int x, y, z, height, ceiling, speed;
	short	yangle;

	if (Lara.currentXvel || Lara.currentZvel)
		return 0;

	yangle = v->pos.yRot + 0x8000;
	speed = (GETOFF_DIST * phd_cos(v->pos.xRot)) >> W2V_SHIFT;
	x = v->pos.xPos + (speed * phd_sin(yangle) >> W2V_SHIFT);
	z = v->pos.zPos + (speed * phd_cos(yangle) >> W2V_SHIFT);
	y = v->pos.yPos - ((GETOFF_DIST * phd_sin(-v->pos.xRot)) >> W2V_SHIFT);

	roomNumber = v->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);

	height = GetFloorHeight(floor, x, y, z);
	if (height == NO_HEIGHT || y > height)
		return 0;

	ceiling = GetCeiling(floor, x, y, z);
	if (height - ceiling < 256 || y < ceiling || ceiling == NO_HEIGHT)
		return 0;

	return 1;
}

static int GetOnSub(short item_number, COLL_INFO* coll)
{
	/* Returns 0 if no get on, 1 if right get on and 2 if left get on */
	int dist;
	int x, y, z;
	ITEM_INFO* v, * l;
	FLOOR_INFO* floor;
	short roomNumber, ang;
	unsigned short tempang;

	l = LaraItem;
	v = &Items[item_number];

	if ((!(TrInput & IN_ACTION)) || (Lara.gunStatus != LG_NO_ARMS) || (l->gravityStatus))
		return 0;

	y = abs(l->pos.yPos - (v->pos.yPos - 128));
	if (y > 256)
		return 0;

	x = l->pos.xPos - v->pos.xPos;
	z = l->pos.zPos - v->pos.zPos;

	dist = (x * x) + (z * z);
	if (dist > SQUARE(512))
		return 0;

	roomNumber = v->roomNumber;
	floor = GetFloor(v->pos.xPos, v->pos.yPos, v->pos.zPos, &roomNumber);

	if (GetFloorHeight(floor, v->pos.xPos, v->pos.yPos, v->pos.zPos) < -32000)
		return 0;

	return 1;
}

static void DoCurrent(ITEM_INFO* item)
{
	PHD_VECTOR target;

	if (!Lara.currentActive)
	{
		long shifter, absvel;

		absvel = abs(Lara.currentXvel);

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
		long angle, dx, dz, speed, sinkval;

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

static void BackgroundCollision(ITEM_INFO* v, ITEM_INFO* l, SUB_INFO* sub)
{
	int height;
	COLL_INFO cinfo, *coll = &cinfo;

	coll->badPos = NO_BAD_POS;
	coll->badNeg = -SUB_HEIGHT;
	coll->badCeiling = SUB_HEIGHT;
	coll->old.x = v->pos.xPos;
	coll->old.y = v->pos.yPos;
	coll->old.z = v->pos.zPos;
	coll->radius = SUB_RADIUS;
	coll->trigger = NULL;
	coll->slopesAreWalls = false;
	coll->slopesArePits = false;
	coll->lavaIsPit = false;
	coll->enableSpaz = false;
	coll->enableBaddiePush = true;

	if ((v->pos.xRot >= -16384) && (v->pos.xRot <= 16384))
		coll->facing = Lara.moveAngle = v->pos.yRot;
	else
		coll->facing = Lara.moveAngle = v->pos.yRot - 32768;

	height = phd_sin(v->pos.xRot) * SUB_LENGTH >> W2V_SHIFT;
	if (height < 0)
		height = -height;
	if (height < 200)
		height = 200;

	coll->badNeg = -height;

	GetCollisionInfo(coll, v->pos.xPos, v->pos.yPos + height / 2, v->pos.zPos, v->roomNumber, height);
	ShiftItem(v, coll);

	if (coll->collType == CT_FRONT)
	{
		if (sub->RotX > FRONT_TOLERANCE)
			sub->RotX += WALLDEFLECT;
		else if (sub->RotX < -FRONT_TOLERANCE)
			sub->RotX -= WALLDEFLECT;
		else
			sub->Vel = 0;
	}
	else if (coll->collType == CT_TOP)
	{
		if (sub->RotX >= -TOP_TOLERANCE)
			sub->RotX -= WALLDEFLECT;
	}
	else if (coll->collType == CT_TOP_FRONT)
		sub->Vel = 0;
	else if (coll->collType == CT_LEFT)
		v->pos.yRot += ANGLE(5);
	else if (coll->collType == CT_RIGHT)
		v->pos.yRot -= ANGLE(5);
	else if (coll->collType == CT_CLAMP)
	{
		v->pos.xPos = coll->old.x;
		v->pos.yPos = coll->old.y;
		v->pos.zPos = coll->old.z;
		sub->Vel = 0;
		return;
	}

	if (coll->midFloor < 0)
	{
		v->pos.yPos += coll->midFloor;
		sub->RotX += WALLDEFLECT;
	}
}

static void UserInput(ITEM_INFO* v, ITEM_INFO* l, SUB_INFO* sub)
{
	short anim, frame;

	CanGetOff(v);

	anim = l->animNumber - Objects[ID_UPV_LARA_ANIMS].animIndex;
	frame = l->frameNumber - Anims[l->animNumber].frameBase;

	switch (l->currentAnimState)
	{
	case SUBS_MOVE:
		if (l->hitPoints <= 0)
		{
			l->goalAnimState = SUBS_DIE;
			break;
		}

		if (TrInput & IN_LEFT)
			sub->Rot -= ROT_ACCELERATION;

		else if (TrInput & IN_RIGHT)
			sub->Rot += ROT_ACCELERATION;

		if (sub->Flags & UPV_SURFACE)
		{
			if (v->pos.xRot > SURFACE_ANGLE)
				v->pos.xRot -= ANGLE(1);
			else if (v->pos.xRot < SURFACE_ANGLE)
				v->pos.xRot += ANGLE(1);
		}
		else
		{
			if (TrInput & IN_FORWARD)
				sub->RotX -= UPDOWN_ACCEL;
			else if (TrInput & IN_BACK)
				sub->RotX += UPDOWN_ACCEL;
		}

		if (TrInput & IN_JUMP)
		{
			if ((sub->Flags & UPV_SURFACE) && (TrInput & IN_FORWARD) && (v->pos.xRot > -DIVE_ANGLE))
				sub->Flags |= UPV_DIVE;

			sub->Vel += ACCELERATION;
		}

		else
			l->goalAnimState = SUBS_POSE;

		break;

	case SUBS_POSE:
		if (l->hitPoints <= 0)
		{
			l->goalAnimState = SUBS_DIE;
			break;
		}

		if (TrInput & IN_LEFT)
			sub->Rot -= ROT_SLOWACCEL;
		else if (TrInput & IN_RIGHT)
			sub->Rot += ROT_SLOWACCEL;

		if (sub->Flags & UPV_SURFACE)
		{
			if (v->pos.xRot > SURFACE_ANGLE)
				v->pos.xRot -= ANGLE(1);
			else if (v->pos.xRot < SURFACE_ANGLE)
				v->pos.xRot += ANGLE(1);
		}
		else
		{
			if (TrInput & IN_FORWARD)
				sub->RotX -= UPDOWN_ACCEL;
			else if (TrInput & IN_BACK)
				sub->RotX += UPDOWN_ACCEL;
		}

		if ((TrInput & IN_ROLL) && (CanGetOff(v)))
		{
			if (sub->Flags & UPV_SURFACE)
				l->goalAnimState = SUBS_GETOFFS;
			else
				l->goalAnimState = SUBS_GETOFF;

			sub->Flags &= ~UPV_CONTROL;

			StopSoundEffect(346);
			SoundEffect(348, (PHD_3DPOS*)&v->pos.xPos, 2);
		}

		else if (TrInput & IN_JUMP)
		{
			if ((sub->Flags & UPV_SURFACE) && (TrInput & IN_FORWARD) && (v->pos.xRot > -DIVE_ANGLE))
				sub->Flags |= UPV_DIVE;

			l->goalAnimState = SUBS_MOVE;
		}

		break;

	case SUBS_GETON:
		if (anim == 11)
		{
			v->pos.yPos += 4;
			v->pos.xRot += ANGLE(1);

			if (frame == 30)
				SoundEffect(347, (PHD_3DPOS*)&v->pos.xPos, 2);

			if (frame == 50)
				sub->Flags |= UPV_CONTROL;
		}

		else if (anim == 13)
		{
			if (frame == 30)
				SoundEffect(347, (PHD_3DPOS*)&v->pos.xPos, 2);

			if (frame == 42)
				sub->Flags |= UPV_CONTROL;
		}

		break;

	case SUBS_GETOFF:
		if ((anim == 12) && (frame == 42))
		{
			PHD_VECTOR vec = { 0, 0, 0 };
			GAME_VECTOR VPos, LPos;

			GetLaraJointPosition(&vec, LM_HIPS);

			LPos.x = vec.x;
			LPos.y = vec.y;
			LPos.z = vec.z;
			LPos.roomNumber = v->roomNumber;
			VPos.x = v->pos.xPos;
			VPos.y = v->pos.yPos;
			VPos.z = v->pos.zPos;
			VPos.roomNumber = v->roomNumber;
			mgLOS(&VPos, &LPos, 0);

			l->pos.xPos = LPos.x;
			l->pos.yPos = LPos.y;
			l->pos.zPos = LPos.z;
			l->animNumber = 108;
			l->frameNumber = Anims[l->animNumber].frameBase;
			l->currentAnimState = 13;

			l->fallspeed = 0;
			l->gravityStatus = false;
			l->pos.xRot = l->pos.zRot = 0;

			UpdateLaraRoom(l, 0);

			Lara.waterStatus = LW_UNDERWATER;
			Lara.gunStatus = LG_NO_ARMS;
			Lara.Vehicle = NO_ITEM;

			v->hitPoints = 0;
		}

		break;

	case SUBS_GETOFFS:
		if ((anim == 9) && (frame == 51))
		{
			int wd, wh, hfw;
			PHD_VECTOR vec = { 0, 0, 0 };

			wd = GetWaterSurface(l->pos.xPos, l->pos.yPos, l->pos.zPos, l->roomNumber);
			wh = GetWaterHeight(l->pos.xPos, l->pos.yPos, l->pos.zPos, l->roomNumber);

			if (wh != NO_HEIGHT)
				hfw = l->pos.yPos - wh;
			else
				hfw = NO_HEIGHT;

			GetLaraJointPosition(&vec, LM_HIPS);

			l->pos.xPos = vec.x;
			l->pos.yPos = l->pos.yPos + 1 - hfw;
			l->pos.yPos = vec.y;
			l->pos.zPos = vec.z;
			l->animNumber = 114;
			l->frameNumber = Anims[l->animNumber].frameBase;
			l->currentAnimState = l->goalAnimState = 34;

			l->fallspeed = 0;
			l->gravityStatus = false;
			l->pos.xRot = l->pos.zRot = 0;

			UpdateLaraRoom(l, -LARA_HITE / 2);

			Lara.waterStatus = LW_SURFACE;
			Lara.waterSurfaceDist = -hfw;
			Lara.diveCount = 11;
			Lara.torsoXrot = Lara.torsoYrot = 0;
			Lara.headXrot = Lara.headYrot = 0;
			Lara.gunStatus = LG_NO_ARMS;
			Lara.Vehicle = NO_ITEM;

			v->hitPoints = 0;
		}

		break;

	case SUBS_DIE:
		if (((anim == 0) && (frame == 16)) || ((anim == 0) && (frame == 17)))
		{
			PHD_VECTOR vec = { 0, 0, 0 };

			GetLaraJointPosition(&vec, LM_HIPS);

			l->pos.xPos = vec.x;
			l->pos.yPos = vec.y;
			l->pos.zPos = vec.z;
			l->animNumber = 124;
			l->frameNumber = GF(124, 17);
			l->currentAnimState = l->goalAnimState = 44;
			l->fallspeed = 0;
			l->gravityStatus = 0;
			l->pos.xRot = l->pos.zRot = 0;

			sub->Flags |= UPV_DEAD;
		}
		v->speed = 0;

		break;
	}

	if (sub->Flags & UPV_DIVE)
	{
		if (v->pos.xRot > -DIVE_ANGLE)
			v->pos.xRot -= DIVE_SPEED;
		else
			sub->Flags &= ~UPV_DIVE;
	}

	if (sub->Vel > 0)
	{
		if ((sub->Vel -= FRICTION) < 0)
			sub->Vel = 0;
	}
	else if (sub->Vel < 0)
	{
		if ((sub->Vel += FRICTION) > 0)
			sub->Vel = 0;
	}

	if (sub->Vel > MAX_SPEED)
		sub->Vel = MAX_SPEED;
	else if (sub->Vel < -MAX_SPEED)
		sub->Vel = -MAX_SPEED;

	if (sub->Rot > 0)
	{
		if ((sub->Rot -= ROT_FRICTION) < 0)
			sub->Rot = 0;
	}
	else if (sub->Rot < 0)
	{
		if ((sub->Rot += ROT_FRICTION) > 0)
			sub->Rot = 0;
	}

	if (sub->RotX > 0)
	{
		if ((sub->RotX -= UPDOWN_FRICTION) < 0)
			sub->RotX = 0;
	}
	else if (sub->RotX < 0)
	{
		if ((sub->RotX += UPDOWN_FRICTION) > 0)
			sub->RotX = 0;
	}

	if (sub->Rot > MAX_ROTATION)
		sub->Rot = MAX_ROTATION;
	else if (sub->Rot < -MAX_ROTATION)
		sub->Rot = -MAX_ROTATION;

	if (sub->RotX > MAX_UPDOWN)
		sub->RotX = MAX_UPDOWN;
	else if (sub->RotX < -MAX_UPDOWN)
		sub->RotX = -MAX_UPDOWN;
}

void SubInitialise(short itemNum)
{
	ITEM_INFO* v;
	SUB_INFO* sub;

	v = &Items[itemNum];
	sub = (SUB_INFO*)game_malloc(sizeof(SUB_INFO));
	v->data = (void*)sub;
	sub->Vel = sub->Rot = 0;
	sub->Flags = UPV_SURFACE; 
	sub->WeaponTimer = 0;
}

void SubCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	int geton;
	ITEM_INFO* v;

	if ((l->hitPoints <= 0) || (Lara.Vehicle != NO_ITEM))
		return;

	v = &Items[itemNum];

	geton = GetOnSub(itemNum, coll);
	if (geton)
	{
		Lara.Vehicle = itemNum;
		Lara.waterStatus = LW_ABOVE_WATER;

		/* -------- throw flare away if using */

		if (Lara.gunType == WEAPON_FLARE)
		{
			CreateFlare(ID_FLARE_ITEM, 0);
			undraw_flare_meshes();
			Lara.flareControlLeft = false;
			Lara.requestGunType = Lara.gunType = WEAPON_NONE;
		}

		/* -------- determine animation */
		Lara.gunStatus = LG_HANDS_BUSY;

		l->pos.xPos = v->pos.xPos;
		l->pos.yPos = v->pos.yPos;
		l->pos.zPos = v->pos.zPos;
		l->pos.yRot = v->pos.yRot;

		if ((l->currentAnimState == 33) || (l->currentAnimState == 34))
		{
			l->animNumber = Objects[ID_UPV_LARA_ANIMS].animIndex + 10;
			l->frameNumber = GF2(ID_UPV_LARA_ANIMS, 10, 0);
			l->currentAnimState = l->goalAnimState = SUBS_GETON;
		}
		else
		{
			l->animNumber = Objects[ID_UPV_LARA_ANIMS].animIndex + 13;
			l->frameNumber = GF2(ID_UPV, 13, 0);
			l->currentAnimState = l->goalAnimState = SUBS_GETON;
		}

		AnimateItem(l);
	}
	else
	{
		//v->pos.yPos += SUB_DRAW_SHIFT;
		
		if (!TestBoundsCollide(v, LaraItem, coll->radius))
			return;
		if (!TestCollision(v, LaraItem))
			return;
		ItemPushLara(v, LaraItem, coll, false, 0);

		//v->pos.yPos -= SUB_DRAW_SHIFT;
	}
}

int SubControl(void)
{
	int h;
	SUB_INFO* sub;
	ITEM_INFO* v, * l;
	FLOOR_INFO* floor;
	short roomNumber;

	l = LaraItem;
	v = &Items[Lara.Vehicle];
	sub = (SUB_INFO*)v->data;

	/* -------- update dynamics */
	if (!(sub->Flags & UPV_DEAD))
	{
		UserInput(v, l, sub);

		v->speed = sub->Vel >> 16;

		v->pos.xRot += sub->RotX >> 16;
		v->pos.yRot += (sub->Rot >> 16);
		v->pos.zRot = (sub->Rot >> 12);

		if (v->pos.xRot > UPDOWN_LIMIT)
			v->pos.xRot = UPDOWN_LIMIT;
		else if (v->pos.xRot < -UPDOWN_LIMIT)
			v->pos.xRot = -UPDOWN_LIMIT;

		v->pos.xPos += (((phd_sin(v->pos.yRot) * v->speed) >> W2V_SHIFT)* phd_cos(v->pos.xRot)) >> W2V_SHIFT;
		v->pos.yPos -= (phd_sin(v->pos.xRot) * v->speed) >> W2V_SHIFT;
		v->pos.zPos += (((phd_cos(v->pos.yRot) * v->speed) >> W2V_SHIFT)* phd_cos(v->pos.xRot)) >> W2V_SHIFT;
	}

	/* -------- determine if vehicle is near the surface */
	roomNumber = v->roomNumber;
	floor = GetFloor(v->pos.xPos, v->pos.yPos, v->pos.zPos, &roomNumber);
	v->floor = GetFloorHeight(floor, v->pos.xPos, v->pos.yPos, v->pos.zPos);

	if ((sub->Flags & UPV_CONTROL) && (!(sub->Flags & UPV_DEAD)))
	{
		h = GetWaterHeight(v->pos.xPos, v->pos.yPos, v->pos.zPos, roomNumber);

		if ((h != NO_HEIGHT) && (!(Rooms[v->roomNumber].flags & ENV_FLAG_WATER)))
		{
			if ((h - v->pos.yPos) >= -SURFACE_DIST)
				v->pos.yPos = h + SURFACE_DIST;

			if (!(sub->Flags & UPV_SURFACE))
			{
				SoundEffect(36, &LaraItem->pos, 2);
				sub->Flags &= ~UPV_DIVE;
			}

			sub->Flags |= UPV_SURFACE;
		}

		else if ((h != NO_HEIGHT) && ((h - v->pos.yPos) >= -SURFACE_DIST))
		{
			v->pos.yPos = h + SURFACE_DIST;

			if (!(sub->Flags & UPV_SURFACE))
			{
				SoundEffect(36, &LaraItem->pos, 2);
				sub->Flags &= ~UPV_DIVE;
			}

			sub->Flags |= UPV_SURFACE;
		}

		else
			sub->Flags &= ~UPV_SURFACE;

		/* -------- update air bar */
		if (!(sub->Flags & UPV_SURFACE))
		{
			if (l->hitPoints > 0)
			{
				Lara.air--;

				if (Lara.air < 0)
				{
					Lara.air = -1;
					l->hitPoints -= 5;
				}
			}
		}
		else
		{
			if (l->hitPoints >= 0)
			{
				Lara.air += 10;

				if (Lara.air > 1800)
					Lara.air = 1800;
			}
		}
	}

	TestTriggers(TriggerIndex, false, 0);
	SubEffects(Lara.Vehicle);

	/* -------- update vehicle & Lara */
	if ((Lara.Vehicle != NO_ITEM) && (!(sub->Flags & UPV_DEAD)))
	{
		DoCurrent(v);

		if ((TrInput & IN_ACTION) && (sub->Flags & UPV_CONTROL) && (!sub->WeaponTimer))
		{
			FireSubHarpoon(v);
			sub->WeaponTimer = HARPOON_RELOAD;
		}

		if (roomNumber != v->roomNumber)
		{
			ItemNewRoom(Lara.Vehicle, roomNumber);
			ItemNewRoom(Lara.itemNumber, roomNumber);
		}

		l->pos.xPos = v->pos.xPos;
		l->pos.yPos = v->pos.yPos + SUB_DRAW_SHIFT;
		l->pos.zPos = v->pos.zPos;
		l->pos.xRot = v->pos.xRot;
		l->pos.yRot = v->pos.yRot;
		l->pos.zRot = v->pos.zRot;

		AnimateItem(l);
		BackgroundCollision(v, l, sub);

		if (sub->Flags & UPV_CONTROL)
			SoundEffect(346, (PHD_3DPOS*)&v->pos.xPos, 2 | 4 | 0x1000000 | (v->speed << 16));

		v->animNumber = Objects[ID_UPV].animIndex + (l->animNumber - Objects[ID_UPV_LARA_ANIMS].animIndex);
		v->frameNumber = Anims[v->animNumber].frameBase + (l->frameNumber - Anims[l->animNumber].frameBase);

		if (sub->Flags & UPV_SURFACE)
			Camera.targetElevation = -ANGLE(60);
		else
			Camera.targetElevation = 0;

		return 1;
	}
	else if (sub->Flags & UPV_DEAD)
	{
		AnimateItem(l);

		if (roomNumber != v->roomNumber)
			ItemNewRoom(Lara.Vehicle, roomNumber);

		BackgroundCollision(v, l, sub);

		sub->RotX = 0;

		v->animNumber = 5;
		v->frameNumber = GF2(ID_UPV, 5, 0);
		v->goalAnimState = v->currentAnimState = SUBS_POSE;
		v->fallspeed = 0;
		v->speed = 0;
		v->gravityStatus = true;
		AnimateItem(v);

		return 1;
	}
	else
	{
		return 0;
	}
}