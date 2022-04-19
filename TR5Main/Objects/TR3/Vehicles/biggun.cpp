#include "framework.h"
#include "Objects/TR3/Vehicles/biggun.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_struct.h"
#include "Objects/TR3/Vehicles/biggun_info.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Specific/setup.h"

#define BGUN_TURN_RATE ANGLE(2.0f)
#define BGUN_TURN_MAX ANGLE(16.0f)

#define RECOIL_TIME 26
#define RECOIL_Z	25

#define BGUN_STATE_UP_DOWN_FRAMES	59
#define BGUN_DISMOUNT_FRAME			30

#define BGUN_IN_FIRE		IN_ACTION
#define BGUN_IN_DISMOUNT	(IN_ROLL | IN_JUMP)
#define BGUN_IN_UP			IN_FORWARD
#define BGUN_IN_DOWN		IN_BACK
#define BGUN_IN_LEFT		IN_LEFT
#define BGUN_IN_RIGHT		IN_RIGHT

enum BigGunState
{
	BGUN_STATE_MOUNT = 0,
	BGUN_STATE_DISMOUNT = 1,
	BGUN_STATE_UP_DOWN = 2,
	BGUN_STATE_RECOIL = 3
};

enum BigGunAnim
{
	BGUN_ANIM_MOUNT = 0,
	BGUN_ANIM_DISMOUNT = 1,
	BGUN_ANIM_UP_DOWN = 2,
	BGUN_ANIM_RECOIL = 3
};

enum BigGunFlags
{
	 BGUN_FLAG_UP_DOWN = 1,
	 BGUN_FLAG_AUTO_ROT = 2,
	 BGUN_FLAG_DISMOUNT = 4,
	 BGUN_FLAG_FIRE = 8
};

static long GunRotYAdd = 0;
bool barrelRotating;

void BigGunInitialise(short itemNum)
{
	ITEM_INFO* bGunItem = &g_Level.Items[itemNum];
	bGunItem->data = BIGGUNINFO();
	BIGGUNINFO* bigGunInfo = bGunItem->data;

	bigGunInfo->flags = 0;
	bigGunInfo->fireCount = 0;
	bigGunInfo->xRot = BGUN_DISMOUNT_FRAME;
	bigGunInfo->yRot = 0;
	bigGunInfo->startYRot = bGunItem->pos.yRot;
}

static bool BigGunTestMount(ITEM_INFO* bGunItem, ITEM_INFO* laraItem)
{
	//LaraInfo*& laraInfo = lara->data; // If Lara global is not used, the game crashes upon level load. Not sure why. @Sezz 2022.01.09

	if (!(TrInput & IN_ACTION) ||
		Lara.gunStatus != LG_HANDS_FREE ||
		laraItem->gravityStatus) // BUG: Lara can still mount when jumping up. @Sezz 2021.11.16
	{
		return false;
	}

	int x = laraItem->pos.xPos - bGunItem->pos.xPos;
	int y = laraItem->pos.yPos - bGunItem->pos.yPos;
	int z = laraItem->pos.zPos - bGunItem->pos.zPos;
	int dist = pow(x, 2) + pow(y, 2) + pow(z, 2);
	if (dist > 30000)
		return false;

	short deltaAngle = abs(laraItem->pos.yRot - bGunItem->pos.yRot);
	if (deltaAngle > ANGLE(35.0f) || deltaAngle < -ANGLE(35.0f))
		return false;

	return true;
}

void BigGunFire(ITEM_INFO* bGunItem, ITEM_INFO* laraItem)
{
	BIGGUNINFO* bGunInfo = bGunItem->data;

	short itemNum = CreateItem();
	ITEM_INFO* projectileItem = &g_Level.Items[itemNum];

	if (itemNum != NO_ITEM)
	{
		projectileItem->objectNumber = ID_ROCKET;
		projectileItem->roomNumber = laraItem->roomNumber;

		PHD_VECTOR pos = { 0, 0, CLICK(1) }; // CLICK(1) or 520?
		GetJointAbsPosition(bGunItem, &pos, 2);
			
		projectileItem->pos.xPos = pos.x;
		projectileItem->pos.yPos = pos.y;
		projectileItem->pos.zPos = pos.z;

		InitialiseItem(itemNum);

		projectileItem->pos.xRot = -((bGunInfo->xRot - 32) * ANGLE(1.0f));
		projectileItem->pos.yRot = bGunItem->pos.yRot;
		projectileItem->pos.zRot = 0;
		projectileItem->speed = 16;
		projectileItem->itemFlags[0] = 1;

		AddActiveItem(itemNum);

		SmokeCountL = 32;
		SmokeWeapon = WEAPON_ROCKET_LAUNCHER;

		for (int i = 0; i < 5; i++)
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 1, WEAPON_ROCKET_LAUNCHER, 32);

		SoundEffect(SFX_TR4_EXPLOSION1, 0, 0);
	}
}

void BigGunCollision(short itemNum, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	ITEM_INFO* bGunItem = &g_Level.Items[itemNum];
	BIGGUNINFO* bGunInfo = bGunItem->data;
	LaraInfo*& laraInfo = laraItem->data;

	if (laraItem->hitPoints <= 0 || laraInfo->Vehicle != NO_ITEM)
		return;

	if (BigGunTestMount(laraItem, bGunItem))
	{
		laraInfo->Vehicle = itemNum;

		if (laraInfo->gunType == WEAPON_FLARE)
		{
			CreateFlare(laraItem, ID_FLARE_ITEM, false);
			UndrawFlareMeshes(laraItem);

			laraInfo->flareControlLeft = false;
			laraInfo->requestGunType = WEAPON_NONE;
			laraInfo->gunType = WEAPON_NONE;
		}

		laraItem->animNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_MOUNT;
		laraItem->frameNumber = g_Level.Anims[Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_MOUNT].frameBase;
		laraItem->goalAnimState = BGUN_STATE_MOUNT;
		laraItem->currentAnimState = BGUN_STATE_MOUNT;
		laraItem->pos = bGunItem->pos;
		laraItem->gravityStatus = false;
		laraInfo->gunStatus = LG_HANDS_BUSY;
		bGunItem->hitPoints = 1;
		bGunInfo->flags = 0;
		bGunInfo->xRot = BGUN_DISMOUNT_FRAME;

	}
	else
		ObjectCollision(itemNum, laraItem, coll);
}

bool BigGunControl(ITEM_INFO* laraItem, COLL_INFO* coll)
{
	LaraInfo*& laraInfo = laraItem->data;
	ITEM_INFO* bGunItem = &g_Level.Items[laraInfo->Vehicle];
	BIGGUNINFO* bGunInfo = bGunItem->data;

	if (bGunInfo->flags & BGUN_FLAG_UP_DOWN)
	{
		if (barrelRotating)
			bGunInfo->barrelZ--;

		if (!bGunInfo->barrelZ)
			barrelRotating = false;

		if (TrInput & BGUN_IN_DISMOUNT || laraItem->hitPoints <= 0)
			bGunInfo->flags = BGUN_FLAG_AUTO_ROT;
		else
		{
			if (TrInput & BGUN_IN_FIRE && bGunInfo->fireCount == 0)
			{
				BigGunFire(bGunItem, laraItem);
				bGunInfo->fireCount = RECOIL_TIME;
				bGunInfo->barrelZ = RECOIL_Z;
				barrelRotating = true;
			}

			if (TrInput & BGUN_IN_LEFT)
			{
				if (GunRotYAdd > 0)
					GunRotYAdd /= 2;

				GunRotYAdd -= BGUN_TURN_RATE;
				if (GunRotYAdd < -BGUN_TURN_MAX)
					GunRotYAdd = -BGUN_TURN_MAX;
			}
			else if (TrInput & BGUN_IN_RIGHT)
			{		
				if (GunRotYAdd < 0)
					GunRotYAdd /= 2;

				GunRotYAdd += BGUN_TURN_RATE;
				if (GunRotYAdd > BGUN_TURN_MAX)
					GunRotYAdd = BGUN_TURN_MAX;
			}
			else
			{
				GunRotYAdd -= GunRotYAdd / 4;
				if (abs(GunRotYAdd) < BGUN_TURN_RATE)
					GunRotYAdd = 0;
			}

			bGunInfo->yRot += GunRotYAdd / 4;

			if (TrInput & BGUN_IN_UP && bGunInfo->xRot < BGUN_STATE_UP_DOWN_FRAMES)
				bGunInfo->xRot++;			
			else if (TrInput & BGUN_IN_DOWN && bGunInfo->xRot)
				bGunInfo->xRot--;
		}
	}

	if (bGunInfo->flags & BGUN_FLAG_AUTO_ROT)
	{
		if (bGunInfo->xRot == BGUN_DISMOUNT_FRAME)
		{
			laraItem->animNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_DISMOUNT;
			laraItem->frameNumber = g_Level.Anims[Objects[ID_BIGGUN].animIndex + BGUN_ANIM_DISMOUNT].frameBase;
			laraItem->currentAnimState = BGUN_STATE_DISMOUNT;
			laraItem->goalAnimState = BGUN_STATE_DISMOUNT;
			bGunInfo->flags = BGUN_FLAG_DISMOUNT;
		}
		else if (bGunInfo->xRot > BGUN_DISMOUNT_FRAME)
			bGunInfo->xRot--;
		else if (bGunInfo->xRot < BGUN_DISMOUNT_FRAME)
			bGunInfo->xRot++;
	}

	switch (laraItem->currentAnimState)
	{
	case BGUN_STATE_MOUNT:
	case BGUN_STATE_DISMOUNT:
		AnimateItem(laraItem);
		bGunItem->animNumber = Objects[ID_BIGGUN].animIndex + (laraItem->animNumber - Objects[ID_BIGGUN_ANIMS].animIndex);
		bGunItem->frameNumber = g_Level.Anims[bGunItem->animNumber].frameBase + (laraItem->frameNumber - g_Level.Anims[laraItem->animNumber].frameBase);

		if (bGunInfo->flags & BGUN_FLAG_DISMOUNT && TestLastFrame(laraItem))
		{
			SetAnimation(laraItem, LA_STAND_IDLE);
			laraInfo->Vehicle = NO_ITEM;
			laraInfo->gunStatus = LG_HANDS_FREE;
			bGunItem->hitPoints = 0;
		}

		break;

	case BGUN_STATE_UP_DOWN:
		laraItem->animNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_UP_DOWN;
		laraItem->frameNumber = g_Level.Anims[Objects[ID_BIGGUN].animIndex + BGUN_ANIM_UP_DOWN].frameBase + bGunInfo->xRot;
		bGunItem->animNumber = Objects[ID_BIGGUN].animIndex + (laraItem->animNumber - Objects[ID_BIGGUN_ANIMS].animIndex);
		bGunItem->frameNumber = g_Level.Anims[bGunItem->animNumber].frameBase + (laraItem->frameNumber - g_Level.Anims[laraItem->animNumber].frameBase);

		if (bGunInfo->fireCount > 0)
			bGunInfo->fireCount--;
		else
			bGunInfo->fireCount = 0;

		bGunInfo->flags = BGUN_FLAG_UP_DOWN;
		break;
	}
	
	Camera.targetElevation = -ANGLE(15.0f);

	bGunItem->pos.yRot = bGunInfo->startYRot + bGunInfo->yRot;
	laraItem->pos.yRot = bGunItem->pos.yRot;
	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = false;

	DoObjectCollision(laraItem, coll);
	
	return true;
}
