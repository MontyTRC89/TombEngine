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
#define UPDOWN_ACCEL		(ANGLE(2.0f) * 65536)
#define UPDOWN_SLOWACCEL	(ANGLE(1.0f) * 65536)
#define UPDOWN_FRICTION		(ANGLE(1.0f) * 65536)
#define MAX_UPDOWN			(ANGLE(2.0f) * 65536)
#define UPDOWN_LIMIT		ANGLE(80.0f)
#define UPDOWN_SPEED		10
#define SURFACE_DIST		210
#define SURFACE_ANGLE		ANGLE(30.0f)
#define DIVE_ANGLE			ANGLE(15.0f)
#define DIVE_SPEED			ANGLE(5.0f)
#define SUB_DRAW_SHIFT		128
#define SUB_RADIUS			300
#define SUB_HEIGHT			400
#define SUB_LENGTH			WALL_SIZE
#define FRONT_TOLERANCE		(ANGLE(45.0f) * 65536)
#define TOP_TOLERANCE		(ANGLE(45.0f) * 65536)
#define WALLDEFLECT			(ANGLE(2.0f) * 65536)
#define GETOFF_DIST 		WALL_SIZE
#define HARPOON_SPEED		256
#define HARPOON_TIME		256
#define HARPOON_RELOAD		15

#define UPV_TURBINE_BONE 3

#define DEATH_FRAME_1					16
#define DEATH_FRAME_2					17
#define DISMOUNT_SURFACE_FRAME			51
#define MOUNT_SURFACE_SOUND_FRAME		30
#define MOUNT_SURFACE_CONTROL_FRAME		50
#define DISMOUNT_UNDERWATER_FRAME		42
#define MOUNT_UNDERWATER_SOUND_FRAME	30
#define MOUNT_UNDERWATER_CONTROL_FRAME	42

#define UPV_IN_PROPEL		IN_JUMP
#define UPV_IN_UP			IN_FORWARD
#define UPV_IN_DOWN			IN_BACK
#define UPV_IN_LEFT			IN_LEFT
#define UPV_IN_RIGHT		IN_RIGHT
#define UPV_IN_FIRE			IN_ACTION
#define UPV_IN_DISMOUNT		IN_ROLL

BITE_INFO sub_bites[6] =
{
	{ 0, 0, 0, 3 },
	{ 0, 96, 256, 0 },
	{ -128, 0, -64, 1 },
	{ 0, 0, -64, 1 },
	{ 128, 0, -64, 2 },
	{ 0, 0, -64, 2 }
};

enum SUB_BITE_FLAG
{
	SUB_FAN = 0,
	SUB_FRONT_LIGHT,
	SUB_LEFT_FIN_LEFT,
	SUB_LEFT_FIN_RIGHT,
	SUB_RIGHT_FIN_RIGHT,
	SUB_RIGHT_FIN_LEFT
};

enum UPVState
{
	UPV_STATE_DEATH,
	UPV_STATE_HIT,
	UPV_STATE_DISMOUNT_SURFACE,
	UPV_STATE_UNK1,
	UPV_STATE_MOVE,
	UPV_STATE_IDLE,
	UPV_STATE_UNK2,
	UPV_STATE_UNK3,
	UPV_STATE_MOUNT,
	UPV_STATE_DISMOUNT_UNDERWATER
};

// TODO
enum UPVAnim
{
	UPV_ANIM_DEATH = 0,

	UPV_ANIM_IDLE = 5,

	UPV_ANIM_DISMOUNT_SURFACE = 9,
	UPV_ANIM_MOUNT_SURFACE_START = 10,
	UPV_ANIM_MOUNT_SURFACE_END = 11,
	UPV_ANIM_DISMOUNT_UNDERWATER = 12,
	UPV_ANIM_MOUNT_UNDERWATER = 13,
};

void SubInitialise(short itemNum)
{
	ITEM_INFO* UPVItem = &g_Level.Items[itemNum];
	UPVItem->data = SUB_INFO();
	SUB_INFO* UPVInfo = UPVItem->data;

	UPVInfo->Vel = UPVInfo->Rot = 0;
	UPVInfo->Flags = UPV_SURFACE;
	UPVInfo->WeaponTimer = 0;
}

static void FireSubHarpoon(ITEM_INFO* laraItem, ITEM_INFO* UPVItem)
{
	short itemNum = CreateItem();

	if (itemNum != NO_ITEM)
	{
		static char lr = 0;
		PHD_VECTOR pos { (lr ? 22 : -22), 24, 230 };
		ITEM_INFO* harpoonItem = &g_Level.Items[itemNum];

		harpoonItem->objectNumber = ID_HARPOON;
		harpoonItem->shade = 0xC210;
		harpoonItem->roomNumber = UPVItem->roomNumber;

		GetJointAbsPosition(UPVItem, &pos, UPV_TURBINE_BONE);

		harpoonItem->pos.xPos = pos.x;
		harpoonItem->pos.yPos = pos.y;
		harpoonItem->pos.zPos = pos.z;
		InitialiseItem(itemNum);

		harpoonItem->pos.xRot = UPVItem->pos.xRot;
		harpoonItem->pos.yRot = UPVItem->pos.yRot;
		harpoonItem->pos.zRot = 0;

		harpoonItem->fallspeed = -HARPOON_SPEED * phd_sin(harpoonItem->pos.xRot);
		harpoonItem->speed = HARPOON_SPEED * phd_cos(harpoonItem->pos.xRot);
		harpoonItem->hitPoints = HARPOON_TIME;
		harpoonItem->itemFlags[0] = 1;

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

void SubEffects(short itemNum)
{
	if (itemNum == NO_ITEM)
		return;

	ITEM_INFO* laraItem = LaraItem;
	LaraInfo*& laraInfo = laraItem->data;
	ITEM_INFO* UPVItem = &g_Level.Items[itemNum];
	SUB_INFO* UPVInfo = UPVItem->data;
	PHD_VECTOR pos;
	long lp;

	if (laraInfo->Vehicle == itemNum)
	{
		if (!UPVInfo->Vel)
			UPVInfo->FanRot += ANGLE(2.0f);
		else
			UPVInfo->FanRot += (UPVInfo->Vel / 4069);

		if (UPVInfo->Vel)
		{
			pos = { sub_bites[SUB_FAN].x, sub_bites[SUB_FAN].y, sub_bites[SUB_FAN].z };
			GetJointAbsPosition(UPVItem, &pos, sub_bites[SUB_FAN].meshNum);
			TriggerSubMist(pos.x, pos.y + SUB_DRAW_SHIFT, pos.z, abs(UPVInfo->Vel) / 65536, UPVItem->pos.yRot + ANGLE(180.0f));

			if ((GetRandomControl() & 1) == 0)
			{
				PHD_3DPOS pos3d;
				short roomNum;

				pos3d.xPos = pos.x + (GetRandomControl() & 63) - 32;
				pos3d.yPos = pos.y + SUB_DRAW_SHIFT;
				pos3d.zPos = pos.z + (GetRandomControl() & 63) - 32;
				roomNum = UPVItem->roomNumber;
				GetFloor(pos3d.xPos, pos3d.yPos, pos3d.zPos, &roomNum);
				CreateBubble((PHD_VECTOR*)&pos3d, roomNum, 4, 8, BUBBLE_FLAG_CLUMP, 0, 0, 0);
			}
		}
	}
	
	for (lp = 0; lp < 2; lp++)
	{
		GAME_VECTOR	source, target;
		long r;

		r = 31 - (GetRandomControl() & 3);
		pos = { sub_bites[SUB_FRONT_LIGHT].x, sub_bites[SUB_FRONT_LIGHT].y, sub_bites[SUB_FRONT_LIGHT].z << (lp * 6) };
		GetJointAbsPosition(UPVItem, &pos, sub_bites[SUB_FRONT_LIGHT].meshNum);

		if (lp == 1)
		{
			target.x = pos.x;
			target.y = pos.y;
			target.z = pos.z;
			target.roomNumber = UPVItem->roomNumber;
			LOS(&source, &target);
			pos = { target.x, target.y, target.z };
		}
		else
		{
			source.x = pos.x;
			source.y = pos.y;
			source.z = pos.z;
			source.roomNumber = UPVItem->roomNumber;
		}

		TriggerDynamicLight(pos.x, pos.y, pos.z, 16 + (lp << 3), r, r, r);
	}

	if (UPVInfo->WeaponTimer)
		UPVInfo->WeaponTimer--;
}

static bool TestUPVDismount(ITEM_INFO* laraItem, ITEM_INFO* UPVItem)
{
	LaraInfo*& laraInfo = laraItem->data;

	if (laraInfo->currentXvel || laraInfo->currentZvel)
		return false;

	short moveAngle = UPVItem->pos.yRot + ANGLE(180.0f);
	int speed = GETOFF_DIST * phd_cos(UPVItem->pos.xRot);
	int x = UPVItem->pos.xPos + speed * phd_sin(moveAngle);
	int z = UPVItem->pos.zPos + speed * phd_cos(moveAngle);
	int y = UPVItem->pos.yPos - GETOFF_DIST * phd_sin(-UPVItem->pos.xRot);
	auto probe = GetCollisionResult(x, y, z, UPVItem->roomNumber);

	if (probe.Position.Floor < y ||
		(probe.Position.Floor - probe.Position.Ceiling) < STEP_SIZE ||
		probe.Position.Ceiling > y ||
		probe.Position.Floor == NO_HEIGHT ||
		probe.Position.Ceiling == NO_HEIGHT)
	{
		return false;
	}

	return true;
}

static bool TestUPVMount(ITEM_INFO* laraItem, ITEM_INFO* UPVItem)
{
	LaraInfo*& laraInfo = laraItem->data;

	if (!(TrInput & IN_ACTION) ||
		laraInfo->gunStatus != LG_NO_ARMS ||
		laraItem->gravityStatus)
	{
		return false;
	}

	int y = abs(laraItem->pos.yPos - (UPVItem->pos.yPos - STEP_SIZE / 2));
	int dist = pow(laraItem->pos.xPos - UPVItem->pos.xPos, 2) + pow(laraItem->pos.zPos - UPVItem->pos.zPos, 2);
	short rotDelta = abs(laraItem->pos.yRot - UPVItem->pos.yRot);

	if (y > STEP_SIZE ||
		dist > pow(STEP_SIZE * 2, 2) ||
		rotDelta > ANGLE(35.0f) || rotDelta < -ANGLE(35.0f) ||
		GetCollisionResult(UPVItem).Position.Floor < -32000)
	{
		return false;
	}

	return true;
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

static void BackgroundCollision(ITEM_INFO* laraItem, ITEM_INFO* UPVItem)
{
	LaraInfo*& laraInfo = laraItem->data;
	SUB_INFO* UPVInfo = UPVItem->data;
	COLL_INFO cinfo, * coll = &cinfo; // ??

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -SUB_HEIGHT;
	coll->Setup.BadCeilingHeight = SUB_HEIGHT;
	coll->Setup.OldPosition.x = UPVItem->pos.xPos;
	coll->Setup.OldPosition.y = UPVItem->pos.yPos;
	coll->Setup.OldPosition.z = UPVItem->pos.zPos;
	coll->Setup.Radius = SUB_RADIUS;
	coll->Setup.SlopesAreWalls = false;
	coll->Setup.SlopesArePits = false;
	coll->Setup.DeathFlagIsPit = false;
	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = true;
	coll->Setup.Mode = COLL_PROBE_MODE::QUADRANTS;

	if ((UPVItem->pos.xRot >= -16384) && (UPVItem->pos.xRot <= 16384))
		coll->Setup.ForwardAngle = laraInfo->moveAngle = UPVItem->pos.yRot;
	else
		coll->Setup.ForwardAngle = laraInfo->moveAngle = UPVItem->pos.yRot - ANGLE(180.0f);

	int height = phd_sin(UPVItem->pos.xRot) * SUB_LENGTH;
	if (height < 0)
		height = -height;
	if (height < 200)
		height = 200;

	coll->Setup.BadHeightUp = -height;
	coll->Setup.Height = height;

	GetCollisionInfo(coll, UPVItem, PHD_VECTOR(0, height / 2, 0));
	ShiftItem(UPVItem, coll);

	if (coll->CollisionType == CT_FRONT)
	{
		if (UPVInfo->RotX > FRONT_TOLERANCE)
			UPVInfo->RotX += WALLDEFLECT;
		else if (UPVInfo->RotX < -FRONT_TOLERANCE)
			UPVInfo->RotX -= WALLDEFLECT;
		else
		{
			if (abs(UPVInfo->Vel) >= MAX_SPEED)
			{
				laraItem->goalAnimState = UPV_STATE_HIT;
				UPVInfo->Vel = -UPVInfo->Vel / 2;
			}
			else
				UPVInfo->Vel = 0;
		}
	}
	else if (coll->CollisionType == CT_TOP)
	{
		if (UPVInfo->RotX >= -TOP_TOLERANCE)
			UPVInfo->RotX -= WALLDEFLECT;
	}
	else if (coll->CollisionType == CT_TOP_FRONT)
		UPVInfo->Vel = 0;
	else if (coll->CollisionType == CT_LEFT)
		UPVItem->pos.yRot += ANGLE(5.0f);
	else if (coll->CollisionType == CT_RIGHT)
		UPVItem->pos.yRot -= ANGLE(5.0f);
	else if (coll->CollisionType == CT_CLAMP)
	{
		UPVItem->pos.xPos = coll->Setup.OldPosition.x;
		UPVItem->pos.yPos = coll->Setup.OldPosition.y;
		UPVItem->pos.zPos = coll->Setup.OldPosition.z;
		UPVInfo->Vel = 0;
		return;
	}

	if (coll->Middle.Floor < 0)
	{
		UPVItem->pos.yPos += coll->Middle.Floor;
		UPVInfo->RotX += WALLDEFLECT;
	}
}

static void UserInput(ITEM_INFO* laraItem, ITEM_INFO* UPVItem)
{
	LaraInfo*& laraInfo = laraItem->data;
	SUB_INFO* UPVInfo = UPVItem->data;

	TestUPVDismount(laraItem, UPVItem);

	int anim = laraItem->animNumber - Objects[ID_UPV_LARA_ANIMS].animIndex;
	int frame = laraItem->frameNumber - g_Level.Anims[laraItem->animNumber].frameBase;

	switch (laraItem->currentAnimState)
	{
	case UPV_STATE_MOVE:
		if (laraItem->hitPoints <= 0)
		{
			laraItem->goalAnimState = UPV_STATE_DEATH;
			break;
		}

		if (TrInput & UPV_IN_LEFT)
			UPVInfo->Rot -= ROT_ACCELERATION;

		else if (TrInput & UPV_IN_RIGHT)
			UPVInfo->Rot += ROT_ACCELERATION;

		if (UPVInfo->Flags & UPV_SURFACE)
		{
			int xa = UPVItem->pos.xRot - SURFACE_ANGLE;
			int ax = SURFACE_ANGLE - UPVItem->pos.xRot;

			if (xa > 0)
			{
				if (xa > ANGLE(1.0f))
					UPVItem->pos.xRot -= ANGLE(1.0f);
				else
					UPVItem->pos.xRot -= ANGLE(0.1f);
			}
			else if (ax)
			{
				if (ax > ANGLE(1.0f))
					UPVItem->pos.xRot += ANGLE(1.0f);
				else
					UPVItem->pos.xRot += ANGLE(0.1f);
			}
			else
				UPVItem->pos.xRot = SURFACE_ANGLE;
		}
		else
		{
			if (TrInput & UPV_IN_UP)
				UPVInfo->RotX -= UPDOWN_ACCEL;
			else if (TrInput & UPV_IN_DOWN)
				UPVInfo->RotX += UPDOWN_ACCEL;
		}

		if (TrInput & UPV_IN_PROPEL)
		{
			if (TrInput & UPV_IN_UP &&
				UPVInfo->Flags & UPV_SURFACE &&
				UPVItem->pos.xRot > -DIVE_ANGLE)
			{
				UPVInfo->Flags |= UPV_DIVE;
			}

			UPVInfo->Vel += ACCELERATION;
		}

		else
			laraItem->goalAnimState = UPV_STATE_IDLE;

		break;

	case UPV_STATE_IDLE:
		if (laraItem->hitPoints <= 0)
		{
			laraItem->goalAnimState = UPV_STATE_DEATH;
			break;
		}

		if (TrInput & UPV_IN_LEFT)
			UPVInfo->Rot -= ROT_SLOWACCEL;
		else if (TrInput & UPV_IN_RIGHT)
			UPVInfo->Rot += ROT_SLOWACCEL;

		if (UPVInfo->Flags & UPV_SURFACE)
		{
			int xa = UPVItem->pos.xRot - SURFACE_ANGLE;
			int ax = SURFACE_ANGLE - UPVItem->pos.xRot;
			if (xa > 0)
			{
				if (xa > ANGLE(1.0f))
					UPVItem->pos.xRot -= ANGLE(1.0f);
				else
					UPVItem->pos.xRot -= ANGLE(0.1f);
			}
			else if (ax)
			{
				if (ax > ANGLE(1.0f))
					UPVItem->pos.xRot += ANGLE(1.0f);
				else
					UPVItem->pos.xRot += ANGLE(0.1f);
			}
			else
				UPVItem->pos.xRot = SURFACE_ANGLE;
		}
		else
		{
			if (TrInput & UPV_IN_UP)
				UPVInfo->RotX -= UPDOWN_ACCEL;
			else if (TrInput & UPV_IN_DOWN)
				UPVInfo->RotX += UPDOWN_ACCEL;
		}

		if (TrInput & UPV_IN_DISMOUNT && TestUPVDismount(laraItem, UPVItem))
		{
			if (UPVInfo->Vel > 0)
				UPVInfo->Vel -= ACCELERATION;
			else
			{
				if (UPVInfo->Flags & UPV_SURFACE)
					laraItem->goalAnimState = UPV_STATE_DISMOUNT_SURFACE;
				else
					laraItem->goalAnimState = UPV_STATE_DISMOUNT_UNDERWATER;

				//sub->Flags &= ~UPV_CONTROL; having this here causes the UPV glitch, moving it directly to the states' code is better

				StopSoundEffect(SFX_TR3_LITTLE_SUB_LOOP);
				SoundEffect(SFX_TR3_LITTLE_SUB_STOP, (PHD_3DPOS*)&UPVItem->pos.xPos, 2);
			}
		}

		else if (TrInput & UPV_IN_PROPEL)
		{
			if (TrInput & UPV_IN_UP &&
				UPVInfo->Flags & UPV_SURFACE &&
				UPVItem->pos.xRot > -DIVE_ANGLE)
			{
				UPVInfo->Flags |= UPV_DIVE;
			}

			laraItem->goalAnimState = UPV_STATE_MOVE;
		}

		break;

	case UPV_STATE_MOUNT:
		if (anim == UPV_ANIM_MOUNT_SURFACE_END)
		{
			UPVItem->pos.yPos += 4;
			UPVItem->pos.xRot += ANGLE(1.0f);

			if (frame == MOUNT_SURFACE_SOUND_FRAME)
				SoundEffect(SFX_TR3_LITTLE_SUB_LOOP, (PHD_3DPOS*)&UPVItem->pos.xPos, 2);

			if (frame == MOUNT_SURFACE_CONTROL_FRAME)
				UPVInfo->Flags |= UPV_CONTROL;
		}

		else if (anim == UPV_ANIM_MOUNT_UNDERWATER)
		{
			if (frame == MOUNT_UNDERWATER_SOUND_FRAME)
				SoundEffect(SFX_TR3_LITTLE_SUB_LOOP, (PHD_3DPOS*)&UPVItem->pos.xPos, 2);

			if (frame == MOUNT_UNDERWATER_CONTROL_FRAME)
				UPVInfo->Flags |= UPV_CONTROL;
		}

		break;

	case UPV_STATE_DISMOUNT_UNDERWATER:
		if (anim == UPV_ANIM_DISMOUNT_UNDERWATER && frame == DISMOUNT_UNDERWATER_FRAME)
		{
			UPVInfo->Flags &= ~UPV_CONTROL;
			PHD_VECTOR vec = { 0, 0, 0 };
			GAME_VECTOR VPos, LPos;

			GetLaraJointPosition(&vec, LM_HIPS);

			LPos.x = vec.x;
			LPos.y = vec.y;
			LPos.z = vec.z;
			LPos.roomNumber = UPVItem->roomNumber;
			VPos.x = UPVItem->pos.xPos;
			VPos.y = UPVItem->pos.yPos;
			VPos.z = UPVItem->pos.zPos;
			VPos.roomNumber = UPVItem->roomNumber;
			LOSAndReturnTarget(&VPos, &LPos, 0);

			laraItem->pos.xPos = LPos.x;
			laraItem->pos.yPos = LPos.y;
			laraItem->pos.zPos = LPos.z;

			SetAnimation(laraItem, LA_UNDERWATER_IDLE);
			laraItem->fallspeed = 0;
			laraItem->gravityStatus = false;
			laraItem->pos.xRot = laraItem->pos.zRot = 0;

			UpdateItemRoom(laraItem, 0);

			laraInfo->waterStatus = LW_UNDERWATER;
			laraInfo->gunStatus = LG_NO_ARMS;
			laraInfo->Vehicle = NO_ITEM;

			UPVItem->hitPoints = 0;
		}

		break;

	case UPV_STATE_DISMOUNT_SURFACE:
		if (anim == UPV_ANIM_DISMOUNT_SURFACE && frame == DISMOUNT_SURFACE_FRAME)
		{
			UPVInfo->Flags &= ~UPV_CONTROL;
			int waterDepth, waterHeight, heightFromWater;
			PHD_VECTOR vec = { 0, 0, 0 };

			waterDepth = GetWaterSurface(laraItem->pos.xPos, laraItem->pos.yPos, laraItem->pos.zPos, laraItem->roomNumber);
			waterHeight = GetWaterHeight(laraItem->pos.xPos, laraItem->pos.yPos, laraItem->pos.zPos, laraItem->roomNumber);

			if (waterHeight != NO_HEIGHT)
				heightFromWater = laraItem->pos.yPos - waterHeight;
			else
				heightFromWater = NO_HEIGHT;

			GetLaraJointPosition(&vec, LM_HIPS);

			laraItem->pos.xPos = vec.x;
			//laraItem->pos.yPos += -heightFromWater + 1; // Doesn't work as intended.
			laraItem->pos.yPos = vec.y;
			laraItem->pos.zPos = vec.z;

			SetAnimation(laraItem, LA_ONWATER_IDLE);
			laraItem->fallspeed = 0;
			laraItem->gravityStatus = false;
			laraItem->pos.xRot = laraItem->pos.zRot = 0;

			UpdateItemRoom(laraItem, -LARA_HEIGHT / 2);

			laraInfo->waterStatus = LW_SURFACE;
			laraInfo->waterSurfaceDist = -heightFromWater;
			laraInfo->diveCount = 11;
			laraInfo->torsoXrot = 0;
			laraInfo->torsoYrot = 0;
			laraInfo->headXrot = 0;
			laraInfo->headYrot = 0;
			laraInfo->gunStatus = LG_NO_ARMS;
			laraInfo->Vehicle = NO_ITEM;

			UPVItem->hitPoints = 0;
		}
		else
		{
			UPVInfo->RotX -= UPDOWN_ACCEL;
			if (UPVItem->pos.xRot < 0)
				UPVItem->pos.xRot = 0;
		}

		break;

	case UPV_STATE_DEATH:
		if (anim == UPV_ANIM_DEATH && (frame == DEATH_FRAME_1 || frame == DEATH_FRAME_2))
		{
			PHD_VECTOR vec = { 0, 0, 0 };

			GetLaraJointPosition(&vec, LM_HIPS);

			laraItem->pos.xPos = vec.x;
			laraItem->pos.yPos = vec.y;
			laraItem->pos.zPos = vec.z;
			laraItem->pos.xRot = 0;
			laraItem->pos.zRot = 0;

			SetAnimation(UPVItem, LA_UNDERWATER_DEATH, 17);
			laraItem->fallspeed = 0;
			laraItem->gravityStatus = 0;
			
			UPVInfo->Flags |= UPV_DEAD;
		}

		UPVItem->speed = 0;
		break;
	}

	if (UPVInfo->Flags & UPV_DIVE)
	{
		if (UPVItem->pos.xRot > -DIVE_ANGLE)
			UPVItem->pos.xRot -= DIVE_SPEED;
		else
			UPVInfo->Flags &= ~UPV_DIVE;
	}

	if (UPVInfo->Vel > 0)
	{
		UPVInfo->Vel -= FRICTION;
		if (UPVInfo->Vel < 0)
			UPVInfo->Vel = 0;
	}
	else if (UPVInfo->Vel < 0)
	{
		UPVInfo->Vel += FRICTION;
		if (UPVInfo->Vel > 0)
			UPVInfo->Vel = 0;
	}

	if (UPVInfo->Vel > MAX_SPEED)
		UPVInfo->Vel = MAX_SPEED;
	else if (UPVInfo->Vel < -MAX_SPEED)
		UPVInfo->Vel = -MAX_SPEED;

	if (UPVInfo->Rot > 0)
	{
		UPVInfo->Rot -= ROT_FRICTION;
		if (UPVInfo->Rot < 0)
			UPVInfo->Rot = 0;
	}
	else if (UPVInfo->Rot < 0)
	{
		UPVInfo->Rot += ROT_FRICTION;
		if (UPVInfo->Rot > 0)
			UPVInfo->Rot = 0;
	}

	if (UPVInfo->RotX > 0)
	{
		UPVInfo->RotX -= UPDOWN_FRICTION;
		if (UPVInfo->RotX < 0)
			UPVInfo->RotX = 0;
	}
	else if (UPVInfo->RotX < 0)
	{
		UPVInfo->RotX += UPDOWN_FRICTION;
		if (UPVInfo->RotX > 0)
			UPVInfo->RotX = 0;
	}

	if (UPVInfo->Rot > MAX_ROTATION)
		UPVInfo->Rot = MAX_ROTATION;
	else if (UPVInfo->Rot < -MAX_ROTATION)
		UPVInfo->Rot = -MAX_ROTATION;

	if (UPVInfo->RotX > MAX_UPDOWN)
		UPVInfo->RotX = MAX_UPDOWN;
	else if (UPVInfo->RotX < -MAX_UPDOWN)
		UPVInfo->RotX = -MAX_UPDOWN;
}

void NoGetOnCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	ITEM_INFO* item = &g_Level.Items[itemNum];

	if (!TestBoundsCollide(item, laraitem, coll->Setup.Radius))
		return;
	if (!TestCollision(item, laraitem))
		return;

	ItemPushItem(item, laraitem, coll, 0, 0);
}

void SubCollision(short itemNum, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	LaraInfo*& laraInfo = laraItem->data;

	if (laraItem->hitPoints <= 0 || laraInfo->Vehicle != NO_ITEM)
		return;

	ITEM_INFO* UPVItem = &g_Level.Items[itemNum];

	if (TestUPVMount(laraItem, UPVItem))
	{
		laraInfo->Vehicle = itemNum;
		laraInfo->waterStatus = LW_ABOVE_WATER;

		if (laraInfo->gunType == WEAPON_FLARE)
		{
			CreateFlare(LaraItem, ID_FLARE_ITEM, 0);
			UndrawFlareMeshes(laraItem);

			laraInfo->flareControlLeft = false;
			laraInfo->requestGunType = laraInfo->gunType = WEAPON_NONE;
		}

		laraInfo->gunStatus = LG_HANDS_BUSY;
		laraItem->pos.xPos = UPVItem->pos.xPos;
		laraItem->pos.yPos = UPVItem->pos.yPos;
		laraItem->pos.zPos = UPVItem->pos.zPos;
		laraItem->pos.xRot = UPVItem->pos.xRot;
		laraItem->pos.yRot = UPVItem->pos.yRot;
		laraItem->pos.zRot = UPVItem->pos.zRot;
		UPVItem->hitPoints = 1;

		if (laraItem->currentAnimState == LS_ONWATER_STOP || laraItem->currentAnimState == LS_ONWATER_FORWARD)
		{
			laraItem->animNumber = Objects[ID_UPV_LARA_ANIMS].animIndex + UPV_ANIM_MOUNT_SURFACE_START;
			laraItem->currentAnimState = laraItem->goalAnimState = UPV_STATE_MOUNT;
		}
		else
		{
			laraItem->animNumber = Objects[ID_UPV_LARA_ANIMS].animIndex + UPV_ANIM_MOUNT_UNDERWATER;
			laraItem->currentAnimState = laraItem->goalAnimState = UPV_STATE_MOUNT;
		}
		laraItem->frameNumber = g_Level.Anims[laraItem->animNumber].frameBase;

		AnimateItem(laraItem);
	}
	else
	{
		UPVItem->pos.yPos += SUB_DRAW_SHIFT;
		NoGetOnCollision(itemNum, laraItem, coll);
		UPVItem->pos.yPos -= SUB_DRAW_SHIFT;
	}
}

bool SubControl(ITEM_INFO* laraItem, COLL_INFO* coll)
{
	LaraInfo*& laraInfo = laraItem->data;
	ITEM_INFO* UPVItem = &g_Level.Items[laraInfo->Vehicle];
	SUB_INFO* UPVInfo = UPVItem->data;
	auto probe = GetCollisionResult(UPVItem);

	if (!(UPVInfo->Flags & UPV_DEAD))
	{
		UserInput(laraItem, UPVItem);

		UPVItem->speed = UPVInfo->Vel / 65536;

		UPVItem->pos.xRot += UPVInfo->RotX / 65536;
		UPVItem->pos.yRot += UPVInfo->Rot / 65536;
		UPVItem->pos.zRot = UPVInfo->Rot / 65536;

		if (UPVItem->pos.xRot > UPDOWN_LIMIT)
			UPVItem->pos.xRot = UPDOWN_LIMIT;
		else if (UPVItem->pos.xRot < -UPDOWN_LIMIT)
			UPVItem->pos.xRot = -UPDOWN_LIMIT;

		UPVItem->pos.xPos += phd_sin(UPVItem->pos.yRot) * UPVItem->speed * phd_cos(UPVItem->pos.xRot);
		UPVItem->pos.yPos -= phd_sin(UPVItem->pos.xRot) * UPVItem->speed;
		UPVItem->pos.zPos += phd_cos(UPVItem->pos.yRot) * UPVItem->speed * phd_cos(UPVItem->pos.xRot);
	}

	UPVItem->floor = probe.Position.Floor;

	if (UPVInfo->Flags & UPV_CONTROL &&
		!(UPVInfo->Flags & UPV_DEAD))
	{
		int waterHeight = GetWaterHeight(UPVItem->pos.xPos, UPVItem->pos.yPos, UPVItem->pos.zPos, UPVItem->roomNumber);

		if (!(g_Level.Rooms[UPVItem->roomNumber].flags & ENV_FLAG_WATER) &&
			waterHeight != NO_HEIGHT)
		{
			if ((waterHeight - UPVItem->pos.yPos) >= -SURFACE_DIST)
				UPVItem->pos.yPos = waterHeight + SURFACE_DIST;

			if (!(UPVInfo->Flags & UPV_SURFACE))
			{
				SoundEffect(SFX_TR4_LARA_BREATH, &LaraItem->pos, 2);
				UPVInfo->Flags &= ~UPV_DIVE;
			}

			UPVInfo->Flags |= UPV_SURFACE;
		}

		else if ((waterHeight - UPVItem->pos.yPos) >= -SURFACE_DIST &&
			waterHeight != NO_HEIGHT)
		{
			UPVItem->pos.yPos = waterHeight + SURFACE_DIST;

			if (!(UPVInfo->Flags & UPV_SURFACE))
			{
				SoundEffect(SFX_TR4_LARA_BREATH, &LaraItem->pos, 2);
				UPVInfo->Flags &= ~UPV_DIVE;
			}

			UPVInfo->Flags |= UPV_SURFACE;
		}

		else
			UPVInfo->Flags &= ~UPV_SURFACE;

		if (!(UPVInfo->Flags & UPV_SURFACE))
		{
			if (laraItem->hitPoints > 0)
			{
				laraInfo->air--;

				if (laraInfo->air < 0)
				{
					laraInfo->air = -1;
					laraItem->hitPoints -= 5;
				}
			}
		}
		else
		{
			if (laraItem->hitPoints >= 0)
			{
				laraInfo->air += 10;

				if (laraInfo->air > 1800)
					laraInfo->air = 1800;
			}
		}
	}

	TestTriggers(UPVItem, false);
	SubEffects(laraInfo->Vehicle);

	if (!(UPVInfo->Flags & UPV_DEAD) &&
		laraInfo->Vehicle != NO_ITEM)
	{
		DoCurrent(UPVItem);

		if (TrInput & UPV_IN_FIRE &&
			UPVInfo->Flags & UPV_CONTROL &&
			!UPVInfo->WeaponTimer)
		{
			if (laraItem->currentAnimState != UPV_STATE_DISMOUNT_UNDERWATER &&
				laraItem->currentAnimState != UPV_STATE_DISMOUNT_SURFACE &&
				laraItem->currentAnimState != UPV_STATE_MOUNT)
			{
				FireSubHarpoon(laraItem, UPVItem);
				UPVInfo->WeaponTimer = HARPOON_RELOAD;
			}
		}

		if (probe.RoomNumber != UPVItem->roomNumber)
		{
			ItemNewRoom(laraInfo->Vehicle, probe.RoomNumber);
			ItemNewRoom(laraInfo->itemNumber, probe.RoomNumber);
		}

		laraItem->pos.xPos = UPVItem->pos.xPos;
		laraItem->pos.yPos = UPVItem->pos.yPos;
		laraItem->pos.zPos = UPVItem->pos.zPos;
		laraItem->pos.xRot = UPVItem->pos.xRot;
		laraItem->pos.yRot = UPVItem->pos.yRot;
		laraItem->pos.zRot = UPVItem->pos.zRot;

		AnimateItem(laraItem);
		BackgroundCollision(laraItem, UPVItem);

		if (UPVInfo->Flags & UPV_CONTROL)
			SoundEffect(SFX_TR3_LITTLE_SUB_LOOP, (PHD_3DPOS*)&UPVItem->pos.xPos, 2 | 4 | 0x1000000 | (UPVItem->speed * 65536));

		UPVItem->animNumber = Objects[ID_UPV].animIndex + (laraItem->animNumber - Objects[ID_UPV_LARA_ANIMS].animIndex);
		UPVItem->frameNumber = g_Level.Anims[UPVItem->animNumber].frameBase + (laraItem->frameNumber - g_Level.Anims[laraItem->animNumber].frameBase);

		if (UPVInfo->Flags & UPV_SURFACE)
			Camera.targetElevation = -ANGLE(60.0f);
		else
			Camera.targetElevation = 0;

		return true;
	}
	else if (UPVInfo->Flags & UPV_DEAD)
	{
		AnimateItem(laraItem);

		if (probe.RoomNumber != UPVItem->roomNumber)
			ItemNewRoom(laraInfo->Vehicle, probe.RoomNumber);

		BackgroundCollision(laraItem, UPVItem);

		UPVInfo->RotX = 0;

		SetAnimation(UPVItem, UPV_ANIM_IDLE);
		UPVItem->fallspeed = 0;
		UPVItem->speed = 0;
		UPVItem->gravityStatus = true;
		AnimateItem(UPVItem);

		return true;
	}
	else
		return false;
}
