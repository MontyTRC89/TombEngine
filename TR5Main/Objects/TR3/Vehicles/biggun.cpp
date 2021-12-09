#include "framework.h"
#include "biggun.h"
#include "items.h"
#include "level.h"
#include "collide.h"
#include "input.h"
#include "lara.h"
#include "lara_flare.h"
#include "Sound/sound.h"
#include "effects/effects.h"
#include "lara_struct.h"
#include "effects/tomb4fx.h"
#include "animation.h"
#include "setup.h"
#include "camera.h"
#include "biggun_info.h"

#define RECOIL_TIME 26
#define RECOIL_Z	25

// Frames
#define BGUN_STATE_UP_DOWN_FRAMES	59
#define BGUN_DISMOUNT_FRAME			30

#define BGUN_IN_FIRE		IN_ACTION
#define BGUN_IN_DISMOUNT	(IN_ROLL | IN_JUMP)
#define BGUN_IN_UP			IN_FORWARD
#define BGUN_IN_DOWN		IN_BACK
#define BGUN_IN_LEFT		IN_LEFT
#define BGUN_IN_RIGHT		IN_RIGHT

enum BigGunState {
	BGUN_STATE_MOUNT = 0,
	BGUN_STATE_DISMOUNT = 1,
	BGUN_STATE_UP_DOWN = 2,
	BGUN_STATE_RECOIL = 3
};

enum BigGunAnim {
	BGUN_ANIM_MOUNT = 0,
	BGUN_ANIM_DISMOUNT = 1,
	BGUN_ANIM_UP_DOWN = 2,
	BGUN_ANIM_RECOIL = 3
};

enum BigGunFlags {
	 BGUN_FLAG_UP_DOWN = 1,
	 BGUN_FLAG_AUTO_ROT = 2,
	 BGUN_FLAG_DISMOUNT = 4,
	 BGUN_FLAG_FIRE = 8
};

static long GunRotYAdd = 0;
bool barrelRotating;

void FireBigGun(ITEM_INFO* bigGun)
{
	auto bigGunInfo = (BIGGUNINFO*)bigGun->data;
	auto itemNumber = CreateItem();
	ITEM_INFO* projectile = &g_Level.Items[itemNumber];

	if (itemNumber != NO_ITEM)
	{
		projectile->objectNumber = ID_ROCKET;
		projectile->roomNumber = LaraItem->roomNumber;

		PHD_VECTOR pos = { 0, 0, 256 }; // 256 or 520?
		GetJointAbsPosition(bigGun, &pos, 2);
			
		projectile->pos.xPos = pos.x;
		projectile->pos.yPos = pos.y;
		projectile->pos.zPos = pos.z;

		InitialiseItem(itemNumber);

		projectile->pos.xRot = -((bigGunInfo->xRot - 32) * ANGLE(1.0f));
		projectile->pos.yRot = bigGun->pos.yRot;
		projectile->pos.zRot = 0;
		projectile->speed = 16;
		projectile->itemFlags[0] = 1;

		AddActiveItem(itemNumber);

		SmokeCountL = 32;
		SmokeWeapon = WEAPON_ROCKET_LAUNCHER;

		for (int i = 0; i < 5; i++)
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 1, WEAPON_ROCKET_LAUNCHER, 32);

		SoundEffect(SFX_TR4_EXPLOSION1, 0, 0);
	}
}

static bool CanUseGun(ITEM_INFO* lara, ITEM_INFO* bigGun)
{
	//LaraInfo*& laraInfo = lara->data; // This function is presumably called before Lara is initialised, so global must be used. @Sezz 2021.11.16

	if (!(TrInput & IN_ACTION) ||
		Lara.gunStatus != LG_NO_ARMS ||
		lara->gravityStatus) // BUG: Lara can still mount when jumping up. @Sezz 2021.11.16
	{
		return false;
	}

	auto dist = pow(lara->pos.xPos - bigGun->pos.xPos, 2) + pow(lara->pos.zPos - bigGun->pos.zPos, 2);
	if (dist > 30000)
		return false;

	short angle = abs(lara->pos.yRot - bigGun->pos.yRot);
	if (angle > ANGLE(35.0f) || angle < -ANGLE(35.0f))
		return false;

	return true;
}

void BigGunInitialise(short itemNum)
{
	ITEM_INFO* bigGun = &g_Level.Items[itemNum];
	bigGun->data = BIGGUNINFO();
	auto bigGunInfo = (BIGGUNINFO*)bigGun->data;

	bigGunInfo->flags = 0;
	bigGunInfo->fireCount = 0;
	bigGunInfo->xRot = BGUN_DISMOUNT_FRAME;
	bigGunInfo->yRot = 0;
	bigGunInfo->startYRot = bigGun->pos.yRot;
}

void BigGunCollision(short itemNum, ITEM_INFO* lara, COLL_INFO* coll)
{
	LaraInfo*& laraInfo = lara->data;
	ITEM_INFO* bigGun = &g_Level.Items[itemNum];
	auto bigGunInfo = (BIGGUNINFO*)bigGun->data;

	if (lara->hitPoints <= 0 || laraInfo->Vehicle != NO_ITEM)
		return;

	if (CanUseGun(bigGun, lara))
	{
		laraInfo->Vehicle = itemNum;

		if (laraInfo->gunType == WEAPON_FLARE)
		{
			CreateFlare(lara, ID_FLARE_ITEM, false);
			UndrawFlareMeshes(lara);

			laraInfo->flareControlLeft = false;
			laraInfo->requestGunType = WEAPON_NONE;
			laraInfo->gunType = WEAPON_NONE;
		}

		lara->animNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_MOUNT;
		lara->frameNumber = g_Level.Anims[Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_MOUNT].frameBase;
		lara->currentAnimState = BGUN_STATE_MOUNT;
		lara->goalAnimState = BGUN_STATE_MOUNT;
		lara->pos = bigGun->pos;
		laraInfo->gunStatus = LG_HANDS_BUSY;
		bigGun->hitPoints = 1;
		bigGunInfo->flags = 0;
		bigGunInfo->xRot = BGUN_DISMOUNT_FRAME;

	}
	else
		ObjectCollision(itemNum, lara, coll);
}

bool BigGunControl(ITEM_INFO* lara, COLL_INFO* coll)
{
	LaraInfo*& laraInfo = lara->data;
	ITEM_INFO* bigGun = &g_Level.Items[laraInfo->Vehicle];
	auto bigGunInfo = (BIGGUNINFO*)bigGun->data;

	if (bigGunInfo->flags & BGUN_FLAG_UP_DOWN)
	{
		if (barrelRotating)
			bigGunInfo->barrelZ--;

		if (!bigGunInfo->barrelZ)
			barrelRotating = false;

		if (TrInput & BGUN_IN_DISMOUNT || lara->hitPoints <= 0)
			bigGunInfo->flags = BGUN_FLAG_AUTO_ROT;
		else
		{
			if (TrInput & BGUN_IN_FIRE && bigGunInfo->fireCount == 0)
			{
				FireBigGun(bigGun);
				bigGunInfo->fireCount = RECOIL_TIME;
				bigGunInfo->barrelZ = RECOIL_Z;
				barrelRotating = true;
			}

			if (TrInput & BGUN_IN_LEFT)
			{
				if (GunRotYAdd > 0)
					GunRotYAdd /= 2;

				GunRotYAdd -= 8;

				if (GunRotYAdd < -64)
					GunRotYAdd = -64;
			}
			else
			if (TrInput & BGUN_IN_RIGHT)
			{		
				if (GunRotYAdd < 0)
					GunRotYAdd /= 2;

				GunRotYAdd += 8;

				if (GunRotYAdd > 64)
					GunRotYAdd = 64;
			}
			else
			{
				GunRotYAdd -= GunRotYAdd / 4;
				if (abs(GunRotYAdd) < 8)			
					GunRotYAdd = 0;
			}

			bigGunInfo->yRot += GunRotYAdd / 4;

			if (TrInput & BGUN_IN_UP && bigGunInfo->xRot < BGUN_STATE_UP_DOWN_FRAMES)
				bigGunInfo->xRot++;			
			else if (TrInput & BGUN_IN_DOWN && bigGunInfo->xRot)
				bigGunInfo->xRot--;
		}
	}

	if (bigGunInfo->flags & BGUN_FLAG_AUTO_ROT)
	{
		if (bigGunInfo->xRot == BGUN_DISMOUNT_FRAME)
		{
			lara->animNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_DISMOUNT;
			lara->frameNumber = g_Level.Anims[Objects[ID_BIGGUN].animIndex + BGUN_ANIM_DISMOUNT].frameBase;
			lara->currentAnimState = BGUN_STATE_DISMOUNT;
			lara->goalAnimState = BGUN_STATE_DISMOUNT;
			bigGunInfo->flags = BGUN_FLAG_DISMOUNT;
		}
		else if (bigGunInfo->xRot > BGUN_DISMOUNT_FRAME)
			bigGunInfo->xRot--;
		else if (bigGunInfo->xRot < BGUN_DISMOUNT_FRAME)
			bigGunInfo->xRot++;
	}

	switch (lara->currentAnimState)
	{
	case BGUN_STATE_MOUNT:
	case BGUN_STATE_DISMOUNT:
		AnimateItem(lara);
		bigGun->animNumber = Objects[ID_BIGGUN].animIndex + (lara->animNumber - Objects[ID_BIGGUN_ANIMS].animIndex);
		bigGun->frameNumber = g_Level.Anims[bigGun->animNumber].frameBase + (lara->frameNumber - g_Level.Anims[lara->animNumber].frameBase);

		if (bigGunInfo->flags & BGUN_FLAG_DISMOUNT && TestLastFrame(lara))
		{
			SetAnimation(lara, LA_STAND_IDLE);
			laraInfo->Vehicle = NO_ITEM;
			laraInfo->gunStatus = LG_NO_ARMS;
			bigGun->hitPoints = 0;
		}

		break;

	case BGUN_STATE_UP_DOWN:
		lara->animNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_UP_DOWN;
		lara->frameNumber = g_Level.Anims[Objects[ID_BIGGUN].animIndex + BGUN_ANIM_UP_DOWN].frameBase + bigGunInfo->xRot;
		bigGun->animNumber = Objects[ID_BIGGUN].animIndex + (lara->animNumber - Objects[ID_BIGGUN_ANIMS].animIndex);
		bigGun->frameNumber = g_Level.Anims[bigGun->animNumber].frameBase + (lara->frameNumber - g_Level.Anims[lara->animNumber].frameBase);

		if (bigGunInfo->fireCount > 0)
			bigGunInfo->fireCount--;
		else if (bigGunInfo->fireCount <= 0)
			bigGunInfo->fireCount = 0;

		bigGunInfo->flags = BGUN_FLAG_UP_DOWN;

		break;
	}
	
	Camera.targetElevation = -ANGLE(15.0f);
	lara->pos.yRot = bigGunInfo->startYRot + bigGunInfo->yRot * (ANGLE(1.0f) / 4);
	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = false;
	bigGun->pos.yRot = bigGunInfo->startYRot + bigGunInfo->yRot * (ANGLE(1.0f) / 4);
	DoObjectCollision(lara, coll);
	
	return true;
}
