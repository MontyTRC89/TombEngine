#include "framework.h"
#include "upv.h"
#include "lara.h"
#include "items.h"
#include "sphere.h"
#include "effects/effects.h"
#include "collide.h"
#include "control/box.h"
#include "lara_flare.h"
#include "animation.h"
#include "camera.h"
#include "setup.h"
#include "effects/bubble.h"
#include "level.h"
#include "input.h"
#include "savegame.h"
#include "Sound/sound.h"
#include "upv_info.h"
#include "control/los.h"

#define	UPV_CONTROL 1
#define	UPV_SURFACE 2
#define	UPV_DIVE 4
#define	UPV_DEAD 8

#define ACCELERATION		0x40000
#define FRICTION			0x18000
#define MAX_SPEED			0x400000
#define ROT_ACCELERATION	0x400000
#define ROT_SLOWACCEL		0x200000
#define ROT_FRICTION 		0x100000
#define MAX_ROTATION		0x1c00000
#define UPDOWN_ACCEL		(ANGLE(2) * 65536)
#define UPDOWN_SLOWACCEL	(ANGLE(1) * 65536)
#define UPDOWN_FRICTION		(ANGLE(1) * 65536)
#define MAX_UPDOWN			(ANGLE(2) * 65536)
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
#define FRONT_TOLERANCE		(ANGLE(45) * 65536)
#define TOP_TOLERANCE		(ANGLE(45) * 65536)
#define WALLDEFLECT			(ANGLE(2) * 65536)
#define GETOFF_DIST 		WALL_SIZE
#define HARPOON_SPEED		256
#define HARPOON_TIME		256
#define HARPOON_RELOAD		15

BITE_INFO sub_bites[6] = {
	{ 0, 0, 0, 3 },
	{ 0, 96, 256, 0 },
	{ -128, 0, -64, 1 },
	{ 0, 0, -64, 1 },
	{ 128, 0, -64, 2 },
	{ 0, 0, -64, 2 }
};

enum SUB_BITE_FLAG {
	SUB_FAN = 0,
	SUB_FRONT_LIGHT,
	SUB_LEFT_FIN_LEFT,
	SUB_LEFT_FIN_RIGHT,
	SUB_RIGHT_FIN_RIGHT,
	SUB_RIGHT_FIN_LEFT
};

/*Animations and frames*/

//death
#define SUB_DEATH1		0
#define DEATH_FRAME1	16
#define DEATH_FRAME2	17

//pose
#define SUB_POSE_A			5

//get off on the surface
#define SUB_GETOFFSURF1_A	9
#define GETOFFSURF_FRAME	51

//get on on the surface
#define SUB_GETONSURF_A		10
#define SUB_GETONSURF1_A	11
#define GETONSURF_SOUND_FRAME	30
#define GETONSURF_CONTROL_FRAME	50

//get off underwater
#define SUB_GETOFF_A		12
#define GETOFF_FRAME		42

//get on underwater
#define SUB_GETON_A			13
#define GETON_SOUND_FRAME	30
#define GETON_CONTROL_FRAME	42

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

		item = &g_Level.Items[itemNum];

		item->objectNumber = ID_HARPOON;
		item->shade = 0xC210;
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

		item->fallspeed = -HARPOON_SPEED * phd_sin(item->pos.xRot);
		item->speed = HARPOON_SPEED * phd_cos(item->pos.xRot);
		item->hitPoints = HARPOON_TIME;
		item->itemFlags[0] = 1;

		AddActiveItem(itemNum);

		SoundEffect(SFX_TR3_LARA_HARPOON_FIRE_WATER, &LaraItem->pos, 2);

		if (Lara.Weapons[WEAPON_HARPOON_GUN].Ammo[WEAPON_AMMO1])
			Lara.Weapons[WEAPON_HARPOON_GUN].Ammo[WEAPON_AMMO1]--;
		Statistics.Game.AmmoUsed++;

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
	sptr->transType = TransTypeEnum::COLADD;
	sptr->extras = 0;
	sptr->dynamic = -1;

	sptr->x = x + ((GetRandomControl() & 15) - 8);
	sptr->y = y + ((GetRandomControl() & 15) - 8);
	sptr->z = z + ((GetRandomControl() & 15) - 8);
	zv = speed * phd_cos(angle) / 4;
	xv = speed * phd_sin(angle) / 4;
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
	size = (GetRandomControl() & 7) + (speed / 2) + 16;
	sptr->size = sptr->sSize = size / 4;
	sptr->dSize = size;
}

void SubEffects(short item_number)
{
	if (item_number == NO_ITEM)
		return;
	ITEM_INFO* v;
	SUB_INFO* sub;
	PHD_VECTOR pos;
	long lp;

	v = &g_Level.Items[item_number];
	sub = (SUB_INFO*)v->data;

	if (Lara.Vehicle == item_number)
	{
		if (!sub->Vel)
			sub->FanRot += ANGLE(2);
		else
			sub->FanRot += (sub->Vel / 4069);

		if (sub->Vel)
		{
			pos.x = sub_bites[SUB_FAN].x;
			pos.y = sub_bites[SUB_FAN].y;
			pos.z = sub_bites[SUB_FAN].z;
			GetJointAbsPosition(v, &pos, sub_bites[SUB_FAN].meshNum);
			TriggerSubMist(pos.x, pos.y + SUB_DRAW_SHIFT, pos.z, abs(sub->Vel) / 65536, v->pos.yRot + ANGLE(180));

			if ((GetRandomControl() & 1) == 0)
			{
				PHD_3DPOS pos3d;
				short roomNumber;

				pos3d.xPos = pos.x + (GetRandomControl() & 63) - 32;
				pos3d.yPos = pos.y + SUB_DRAW_SHIFT;
				pos3d.zPos = pos.z + (GetRandomControl() & 63) - 32;
				roomNumber = v->roomNumber;
				GetFloor(pos3d.xPos, pos3d.yPos, pos3d.zPos, &roomNumber);
				CreateBubble((PHD_VECTOR*)&pos3d, roomNumber, 4, 8, BUBBLE_FLAG_CLUMP, 0, 0, 0);
			}
		}
	}
	
	for (lp = 0; lp < 2; lp++)
	{
		GAME_VECTOR	source, target;
		long		r;

		r = 31 - (GetRandomControl() & 3);
		pos.x = sub_bites[SUB_FRONT_LIGHT].x;
		pos.y = sub_bites[SUB_FRONT_LIGHT].y;
		pos.z = sub_bites[SUB_FRONT_LIGHT].z << (lp * 6);
		GetJointAbsPosition(v, &pos, sub_bites[SUB_FRONT_LIGHT].meshNum);

		if (lp == 1)
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
		TriggerDynamicLight(pos.x, pos.y, pos.z, 16 + (lp << 3), r, r, r);
	}

	if (sub->WeaponTimer)
		sub->WeaponTimer--;
}

static int CanGetOff(ITEM_INFO* v)
{
	FLOOR_INFO* floor;
	short roomNumber;
	int x, y, z, height, ceiling, speed;
	short	yangle;

	if (Lara.currentXvel || Lara.currentZvel)
		return 0;

	yangle = v->pos.yRot + ANGLE(180);
	speed = GETOFF_DIST * phd_cos(v->pos.xRot);
	x = v->pos.xPos + speed * phd_sin(yangle);
	z = v->pos.zPos + speed * phd_cos(yangle);
	y = v->pos.yPos - GETOFF_DIST * phd_sin(-v->pos.xRot);

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
	int dist;
	int x, y, z;
	ITEM_INFO* v, * l;
	FLOOR_INFO* floor;
	short roomNumber, ang;
	unsigned short tempang;

	l = LaraItem;
	v = &g_Level.Items[item_number];

	if ((!(TrInput & IN_ACTION)) || (Lara.gunStatus != LG_HANDS_FREE) || (l->gravityStatus))
		return 0;

	y = abs(l->pos.yPos - (v->pos.yPos - 128));
	if (y > 256)
		return 0;

	x = l->pos.xPos - v->pos.xPos;
	z = l->pos.zPos - v->pos.zPos;

	dist = SQUARE(x) + SQUARE(z);
	if (dist > SQUARE(512))
		return 0;

	short yr = abs(l->pos.yRot - v->pos.yRot);
	if (yr > ANGLE(35.0f) || yr < -ANGLE(35.0f))
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
		target.x = g_Level.Sinks[sinkval].x;
		target.y = g_Level.Sinks[sinkval].y;
		target.z = g_Level.Sinks[sinkval].z;
		angle = ((mGetAngle(target.x, target.z, LaraItem->pos.xPos, LaraItem->pos.zPos) - ANGLE(90)) / 16) & 4095;

		dx = target.x - LaraItem->pos.xPos;
		dz = target.z - LaraItem->pos.zPos;

		speed = g_Level.Sinks[sinkval].strength;
		dx = phd_sin(angle * 16) * speed * 1024;
		dz = phd_cos(angle * 16) * speed * 1024;

		Lara.currentXvel += ((dx - Lara.currentXvel) / 16);
		Lara.currentZvel += ((dz - Lara.currentZvel) / 16);
	}

	item->pos.xPos += (Lara.currentXvel / 256);
	item->pos.zPos += (Lara.currentZvel / 256);
	Lara.currentActive = 0;
}

static void BackgroundCollision(ITEM_INFO* v, ITEM_INFO* l, SUB_INFO* sub)
{
	int height;
	COLL_INFO cinfo, *coll = &cinfo;

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -SUB_HEIGHT;
	coll->Setup.BadCeilingHeight = SUB_HEIGHT;
	coll->Setup.OldPosition.x = v->pos.xPos;
	coll->Setup.OldPosition.y = v->pos.yPos;
	coll->Setup.OldPosition.z = v->pos.zPos;
	coll->Setup.Radius = SUB_RADIUS;
	coll->Setup.SlopesAreWalls = false;
	coll->Setup.SlopesArePits = false;
	coll->Setup.DeathFlagIsPit = false;
	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.Mode = COLL_PROBE_MODE::QUADRANTS;

	if ((v->pos.xRot >= -16384) && (v->pos.xRot <= 16384))
		coll->Setup.ForwardAngle = Lara.moveAngle = v->pos.yRot;
	else
		coll->Setup.ForwardAngle = Lara.moveAngle = v->pos.yRot - ANGLE(180);

	height = phd_sin(v->pos.xRot) * SUB_LENGTH;
	if (height < 0)
		height = -height;
	if (height < 200)
		height = 200;

	coll->Setup.BadHeightUp = -height;
	coll->Setup.Height = height;

	GetCollisionInfo(coll, v, PHD_VECTOR(0, height / 2, 0));
	ShiftItem(v, coll);

	if (coll->CollisionType == CT_FRONT)
	{
		if (sub->RotX > FRONT_TOLERANCE)
			sub->RotX += WALLDEFLECT;
		else if (sub->RotX < -FRONT_TOLERANCE)
			sub->RotX -= WALLDEFLECT;
		else
		{
			if (abs(sub->Vel) > 0x40000)
			{
				l->goalAnimState = SUBS_HIT;
				sub->Vel = -sub->Vel / 2;
			}
			else
			sub->Vel = 0;
		}
	}
	else if (coll->CollisionType == CT_TOP)
	{
		if (sub->RotX >= -TOP_TOLERANCE)
			sub->RotX -= WALLDEFLECT;
	}
	else if (coll->CollisionType == CT_TOP_FRONT)
		sub->Vel = 0;
	else if (coll->CollisionType == CT_LEFT)
		v->pos.yRot += ANGLE(5);
	else if (coll->CollisionType == CT_RIGHT)
		v->pos.yRot -= ANGLE(5);
	else if (coll->CollisionType == CT_CLAMP)
	{
		v->pos.xPos = coll->Setup.OldPosition.x;
		v->pos.yPos = coll->Setup.OldPosition.y;
		v->pos.zPos = coll->Setup.OldPosition.z;
		sub->Vel = 0;
		return;
	}

	if (coll->Middle.Floor < 0)
	{
		v->pos.yPos += coll->Middle.Floor;
		sub->RotX += WALLDEFLECT;
	}
}

static void UserInput(ITEM_INFO* v, ITEM_INFO* l, SUB_INFO* sub)
{
	short anim, frame;

	CanGetOff(v);

	anim = l->animNumber - Objects[ID_UPV_LARA_ANIMS].animIndex;
	frame = l->frameNumber - g_Level.Anims[l->animNumber].frameBase;

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
			int xa = v->pos.xRot - SURFACE_ANGLE;
			int ax = SURFACE_ANGLE - v->pos.xRot;
			if (xa > 0)
			{
				if (xa > ANGLE(1))
					v->pos.xRot -= ANGLE(1.0f);
				else
					v->pos.xRot -= ANGLE(0.1f);
			}
			else if (ax)
			{
				if (ax > ANGLE(1))
				{
					v->pos.xRot += ANGLE(1.0f);
				}
				else
					v->pos.xRot += ANGLE(0.1f);
			}
			else
				v->pos.xRot = SURFACE_ANGLE;
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
			int xa = v->pos.xRot - SURFACE_ANGLE;
			int ax = SURFACE_ANGLE - v->pos.xRot;
			if (xa > 0)
			{
				if (xa > ANGLE(1))
					v->pos.xRot -= ANGLE(1.0f);
				else
					v->pos.xRot -= ANGLE(0.1f);
			}
			else if (ax)
			{
				if (ax > ANGLE(1))
				{
					v->pos.xRot += ANGLE(1.0f);
				}
				else
					v->pos.xRot += ANGLE(0.1f);
			}
			else
				v->pos.xRot = SURFACE_ANGLE;
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
			if (sub->Vel > 0)
				sub->Vel -= ACCELERATION;
			else
			{
				if (sub->Flags & UPV_SURFACE)
					l->goalAnimState = SUBS_GETOFFS;
				else
					l->goalAnimState = SUBS_GETOFF;

				//sub->Flags &= ~UPV_CONTROL; having this here causes the UPV glitch, moving it directly to the states' code is better

				StopSoundEffect(SFX_TR3_LITTLE_SUB_LOOP);
				SoundEffect(SFX_TR3_LITTLE_SUB_STOP, (PHD_3DPOS*)&v->pos.xPos, 2);
			}
		}

		else if (TrInput & IN_JUMP)
		{
			if ((sub->Flags & UPV_SURFACE) && (TrInput & IN_FORWARD) && (v->pos.xRot > -DIVE_ANGLE))
				sub->Flags |= UPV_DIVE;

			l->goalAnimState = SUBS_MOVE;
		}

		break;

	case SUBS_GETON:
		if (anim == SUB_GETONSURF1_A)
		{
			v->pos.yPos += 4;
			v->pos.xRot += ANGLE(1);

			if (frame == GETONSURF_SOUND_FRAME)
				SoundEffect(SFX_TR3_LITTLE_SUB_LOOP, (PHD_3DPOS*)&v->pos.xPos, 2);

			if (frame == GETONSURF_CONTROL_FRAME)
				sub->Flags |= UPV_CONTROL;
		}

		else if (anim == SUB_GETON_A)
		{
			if (frame == GETON_SOUND_FRAME)
				SoundEffect(SFX_TR3_LITTLE_SUB_LOOP, (PHD_3DPOS*)&v->pos.xPos, 2);

			if (frame == GETON_CONTROL_FRAME)
				sub->Flags |= UPV_CONTROL;
		}

		break;

	case SUBS_GETOFF:
		if ((anim == SUB_GETOFF_A) && (frame == GETOFF_FRAME))
		{
			sub->Flags &= ~UPV_CONTROL;
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
			l->animNumber = LA_UNDERWATER_IDLE;
			l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
			l->currentAnimState = LS_UNDERWATER_STOP;

			l->fallspeed = 0;
			l->gravityStatus = false;
			l->pos.xRot = l->pos.zRot = 0;

			UpdateItemRoom(l, 0);

			Lara.waterStatus = LW_UNDERWATER;
			Lara.gunStatus = LG_HANDS_FREE;
			Lara.Vehicle = NO_ITEM;

			v->hitPoints = 0;
		}

		break;

	case SUBS_GETOFFS:
		if ((anim == SUB_GETOFFSURF1_A) && (frame == GETOFFSURF_FRAME))
		{
			sub->Flags &= ~UPV_CONTROL;
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
			l->animNumber = LA_UNDERWATER_RESURFACE;
			l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
			l->currentAnimState = l->goalAnimState = LS_ONWATER_FORWARD;

			l->fallspeed = 0;
			l->gravityStatus = false;
			l->pos.xRot = l->pos.zRot = 0;

			UpdateItemRoom(l, -LARA_HEIGHT / 2);

			Lara.waterStatus = LW_SURFACE;
			Lara.waterSurfaceDist = -hfw;
			Lara.diveCount = 11;
			Lara.torsoXrot = Lara.torsoYrot = 0;
			Lara.headXrot = Lara.headYrot = 0;
			Lara.gunStatus = LG_HANDS_FREE;
			Lara.Vehicle = NO_ITEM;

			v->hitPoints = 0;
		}

		break;

	case SUBS_DIE:
		if (((anim == SUB_DEATH1) && (frame == DEATH_FRAME1)) || ((anim == SUB_DEATH1) && (frame == DEATH_FRAME2)))
		{
			PHD_VECTOR vec = { 0, 0, 0 };

			GetLaraJointPosition(&vec, LM_HIPS);

			l->pos.xPos = vec.x;
			l->pos.yPos = vec.y;
			l->pos.zPos = vec.z;
			SetAnimation(v, LA_UNDERWATER_DEATH, 17);
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

	v = &g_Level.Items[itemNum];
	v->data = SUB_INFO();
	SUB_INFO* sub = v->data;
	sub->Vel = sub->Rot = 0;
	sub->Flags = UPV_SURFACE; 
	sub->WeaponTimer = 0;
}

void NoGetOnCollision(short item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
	ITEM_INFO	*item;

	item = &g_Level.Items[item_num];
	if (!TestBoundsCollide(item, laraitem, coll->Setup.Radius))
		return;
	if (!TestCollision(item, laraitem))
		return;
	ItemPushItem(item, laraitem, coll, 0, 0);
}

void SubCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	int geton;
	ITEM_INFO* v;

	if ((l->hitPoints <= 0) || (Lara.Vehicle != NO_ITEM))
		return;

	v = &g_Level.Items[itemNum];

	geton = GetOnSub(itemNum, coll);
	if (geton)
	{
		Lara.Vehicle = itemNum;
		Lara.waterStatus = LW_ABOVE_WATER;

		if (Lara.gunType == WEAPON_FLARE)
		{
			CreateFlare(LaraItem, ID_FLARE_ITEM, 0);
			undraw_flare_meshes(l);
			Lara.flareControlLeft = false;
			Lara.requestGunType = Lara.gunType = WEAPON_NONE;
		}

		Lara.gunStatus = LG_HANDS_BUSY;

		v->hitPoints = 1;
		l->pos.xPos = v->pos.xPos;
		l->pos.yPos = v->pos.yPos;
		l->pos.zPos = v->pos.zPos;
		l->pos.xRot = v->pos.xRot;
		l->pos.yRot = v->pos.yRot;
		l->pos.zRot = v->pos.zRot;

		if (l->currentAnimState == LS_ONWATER_STOP || l->currentAnimState == LS_ONWATER_FORWARD)
		{
			SetAnimation(v, SUB_GETONSURF_A);
		}
		else
		{
			SetAnimation(v, SUB_GETON_A);
		}

		AnimateItem(l);
	}
	else
	{
		v->pos.yPos += SUB_DRAW_SHIFT;
		NoGetOnCollision(itemNum, l, coll);
		v->pos.yPos -= SUB_DRAW_SHIFT;
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
	v = &g_Level.Items[Lara.Vehicle];
	sub = (SUB_INFO*)v->data;

	if (!(sub->Flags & UPV_DEAD))
	{
		UserInput(v, l, sub);

		v->speed = sub->Vel / 65536;

		v->pos.xRot += sub->RotX / 65536;
		v->pos.yRot += (sub->Rot / 65536);
		v->pos.zRot = (sub->Rot / 65536);

		if (v->pos.xRot > UPDOWN_LIMIT)
			v->pos.xRot = UPDOWN_LIMIT;
		else if (v->pos.xRot < -UPDOWN_LIMIT)
			v->pos.xRot = -UPDOWN_LIMIT;

		v->pos.xPos += phd_sin(v->pos.yRot) * v->speed * phd_cos(v->pos.xRot);
		v->pos.yPos -= phd_sin(v->pos.xRot) * v->speed;
		v->pos.zPos += phd_cos(v->pos.yRot) * v->speed * phd_cos(v->pos.xRot);
	}

	roomNumber = v->roomNumber;
	floor = GetFloor(v->pos.xPos, v->pos.yPos, v->pos.zPos, &roomNumber);
	v->floor = GetFloorHeight(floor, v->pos.xPos, v->pos.yPos, v->pos.zPos);

	if ((sub->Flags & UPV_CONTROL) && (!(sub->Flags & UPV_DEAD)))
	{
		h = GetWaterHeight(v->pos.xPos, v->pos.yPos, v->pos.zPos, roomNumber);

		if ((h != NO_HEIGHT) && (!(g_Level.Rooms[v->roomNumber].flags & ENV_FLAG_WATER)))
		{
			if ((h - v->pos.yPos) >= -SURFACE_DIST)
				v->pos.yPos = h + SURFACE_DIST;

			if (!(sub->Flags & UPV_SURFACE))
			{
				SoundEffect(SFX_TR4_LARA_BREATH, &LaraItem->pos, 2);
				sub->Flags &= ~UPV_DIVE;
			}

			sub->Flags |= UPV_SURFACE;
		}

		else if ((h != NO_HEIGHT) && ((h - v->pos.yPos) >= -SURFACE_DIST))
		{
			v->pos.yPos = h + SURFACE_DIST;

			if (!(sub->Flags & UPV_SURFACE))
			{
				SoundEffect(SFX_TR4_LARA_BREATH, &LaraItem->pos, 2);
				sub->Flags &= ~UPV_DIVE;
			}

			sub->Flags |= UPV_SURFACE;
		}

		else
			sub->Flags &= ~UPV_SURFACE;

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

	TestTriggers(v, false);
	SubEffects(Lara.Vehicle);

	if ((Lara.Vehicle != NO_ITEM) && (!(sub->Flags & UPV_DEAD)))
	{
		DoCurrent(v);

		if ((TrInput & IN_ACTION) && (sub->Flags & UPV_CONTROL) && (!sub->WeaponTimer))
		{
			if (l->currentAnimState != SUBS_GETOFF 
				&& l->currentAnimState != SUBS_GETOFFS 
				&& l->currentAnimState != SUBS_GETON)
			FireSubHarpoon(v);
			sub->WeaponTimer = HARPOON_RELOAD;
		}

		if (roomNumber != v->roomNumber)
		{
			ItemNewRoom(Lara.Vehicle, roomNumber);
			ItemNewRoom(Lara.itemNumber, roomNumber);
		}

		l->pos.xPos = v->pos.xPos;
		l->pos.yPos = v->pos.yPos;
		l->pos.zPos = v->pos.zPos;
		l->pos.xRot = v->pos.xRot;
		l->pos.yRot = v->pos.yRot;
		l->pos.zRot = v->pos.zRot;

		AnimateItem(l);
		BackgroundCollision(v, l, sub);

		if (sub->Flags & UPV_CONTROL)
			SoundEffect(SFX_TR3_LITTLE_SUB_LOOP, (PHD_3DPOS*)&v->pos.xPos, 2 | 4 | 0x1000000 | (v->speed * 65536));

		v->animNumber = Objects[ID_UPV].animIndex + (l->animNumber - Objects[ID_UPV_LARA_ANIMS].animIndex);
		v->frameNumber = g_Level.Anims[v->animNumber].frameBase + (l->frameNumber - g_Level.Anims[l->animNumber].frameBase);

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

		SetAnimation(v, SUB_POSE_A);
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