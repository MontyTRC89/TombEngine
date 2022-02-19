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
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Objects/TR3/Vehicles/biggun_info.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Specific/setup.h"

#define BGUN_TURN_RATE	ANGLE(2.0f)
#define BGUN_TURN_MAX	ANGLE(16.0f)

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
	BGUN_FLAG_NONE = 0,
	BGUN_FLAG_UP_DOWN = 1,
	BGUN_FLAG_AUTO_ROT = 2,
	BGUN_FLAG_DISMOUNT = 4,
	BGUN_FLAG_FIRE = 8
};

void BigGunInitialise(short itemNum)
{
	auto* bGunItem = &g_Level.Items[itemNum];
	bGunItem->Data = BigGunInfo();
	auto* bGunInfo = (BigGunInfo*)bGunItem->Data;

	bGunInfo->XRot = BGUN_DISMOUNT_FRAME;
	bGunInfo->YRot = 0;
	bGunInfo->StartYRot = bGunItem->Position.yRot;
	bGunInfo->GunRotYAdd = 0;
	bGunInfo->FireCount = 0;
	bGunInfo->Flags = BGUN_FLAG_NONE;
	bGunInfo->BarrelRotating = false;
}

static bool BigGunTestMount(ITEM_INFO* laraItem, ITEM_INFO* bGunItem)
{
	//LaraInfo*& laraInfo = lara->data; // If Lara global is not used, the game crashes upon level load. Not sure why. @Sezz 2022.01.09

	if (!(TrInput & IN_ACTION) ||
		Lara.Control.HandStatus != HandStatus::Free ||
		laraItem->Airborne)
	{
		return false;
	}

	int x = laraItem->Position.xPos - bGunItem->Position.xPos;
	int y = laraItem->Position.yPos - bGunItem->Position.yPos;
	int z = laraItem->Position.zPos - bGunItem->Position.zPos;

	int distance = pow(x, 2) + pow(y, 2) + pow(z, 2);
	if (distance > 30000)
		return false;

	short deltaAngle = abs(laraItem->Position.yRot - bGunItem->Position.yRot);
	if (deltaAngle > ANGLE(35.0f) || deltaAngle < -ANGLE(35.0f))
		return false;

	return true;
}

void BigGunFire(ITEM_INFO* laraItem, ITEM_INFO* bGunItem)
{
	auto* bGunInfo = (BigGunInfo*)bGunItem->Data;

	short itemNum = CreateItem();
	auto* projectileItem = &g_Level.Items[itemNum];

	if (itemNum != NO_ITEM)
	{
		projectileItem->ObjectNumber = ID_ROCKET;
		projectileItem->RoomNumber = laraItem->RoomNumber;

		PHD_VECTOR pos = { 0, 0, CLICK(1) }; // CLICK(1) or 520?
		GetJointAbsPosition(bGunItem, &pos, 2);
			
		projectileItem->Position.xPos = pos.x;
		projectileItem->Position.yPos = pos.y;
		projectileItem->Position.zPos = pos.z;

		InitialiseItem(itemNum);

		projectileItem->Position.xRot = -((bGunInfo->XRot - 32) * ANGLE(1.0f));
		projectileItem->Position.yRot = bGunItem->Position.yRot;
		projectileItem->Position.zRot = 0;
		projectileItem->Velocity = 16;
		projectileItem->ItemFlags[0] = BGUN_FLAG_UP_DOWN;

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
	auto* bGunItem = &g_Level.Items[itemNum];
	auto* bGunInfo = (BigGunInfo*)bGunItem->Data;
	auto* laraInfo = GetLaraInfo(laraItem);

	if (laraItem->HitPoints <= 0 || laraInfo->Vehicle != NO_ITEM)
		return;

	if (BigGunTestMount(bGunItem, laraItem))
	{
		laraInfo->Vehicle = itemNum;

		if (laraInfo->Control.WeaponControl.GunType == WEAPON_FLARE)
		{
			CreateFlare(laraItem, ID_FLARE_ITEM, false);
			UndrawFlareMeshes(laraItem);

			laraInfo->Flare.ControlLeft = false;
			laraInfo->Control.WeaponControl.RequestGunType = WEAPON_NONE;
			laraInfo->Control.WeaponControl.GunType = WEAPON_NONE;
		}

		laraItem->AnimNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_MOUNT;
		laraItem->FrameNumber = g_Level.Anims[Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_MOUNT].frameBase;
		laraItem->TargetState = BGUN_STATE_MOUNT;
		laraItem->ActiveState = BGUN_STATE_MOUNT;
		laraItem->Position = bGunItem->Position;
		laraItem->Airborne = false;
		laraInfo->Control.HandStatus = HandStatus::Busy;
		bGunItem->HitPoints = 1;
		bGunInfo->Flags = BGUN_FLAG_NONE;
		bGunInfo->XRot = BGUN_DISMOUNT_FRAME;

	}
	else
		ObjectCollision(itemNum, laraItem, coll);
}

bool BigGunControl(ITEM_INFO* laraItem, COLL_INFO* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* bGunItem = &g_Level.Items[laraInfo->Vehicle];
	auto* bGunInfo = (BigGunInfo*)bGunItem->Data;

	if (bGunInfo->Flags & BGUN_FLAG_UP_DOWN)
	{
		if (bGunInfo->BarrelRotating)
			bGunInfo->BarrelZRot--;

		if (!bGunInfo->BarrelZRot)
			bGunInfo->BarrelRotating = false;

		if (TrInput & BGUN_IN_DISMOUNT || laraItem->HitPoints <= 0)
			bGunInfo->Flags = BGUN_FLAG_AUTO_ROT;
		else
		{
			if (TrInput & BGUN_IN_FIRE && bGunInfo->FireCount == 0)
			{
				BigGunFire(laraItem, bGunItem);
				bGunInfo->FireCount = RECOIL_TIME;
				bGunInfo->BarrelZRot = RECOIL_Z;
				bGunInfo->BarrelRotating = true;
			}

			if (TrInput & BGUN_IN_LEFT)
			{
				if (bGunInfo->GunRotYAdd > 0)
					bGunInfo->GunRotYAdd /= 2;

				bGunInfo->GunRotYAdd -= BGUN_TURN_RATE;
				if (bGunInfo->GunRotYAdd < -BGUN_TURN_MAX)
					bGunInfo->GunRotYAdd = -BGUN_TURN_MAX;
			}
			else if (TrInput & BGUN_IN_RIGHT)
			{
				if (bGunInfo->GunRotYAdd < 0)
					bGunInfo->GunRotYAdd /= 2;

				bGunInfo->GunRotYAdd += BGUN_TURN_RATE;
				if (bGunInfo->GunRotYAdd > BGUN_TURN_MAX)
					bGunInfo->GunRotYAdd = BGUN_TURN_MAX;
			}
			else
			{
				bGunInfo->GunRotYAdd -= bGunInfo->GunRotYAdd / 4;
				if (abs(bGunInfo->GunRotYAdd) < BGUN_TURN_RATE)
					bGunInfo->GunRotYAdd = 0;
			}

			bGunInfo->YRot += bGunInfo->GunRotYAdd / 4;

			if (TrInput & BGUN_IN_UP && bGunInfo->XRot < BGUN_STATE_UP_DOWN_FRAMES)
				bGunInfo->XRot++;			
			else if (TrInput & BGUN_IN_DOWN && bGunInfo->XRot)
				bGunInfo->XRot--;
		}
	}

	if (bGunInfo->Flags & BGUN_FLAG_AUTO_ROT)
	{
		if (bGunInfo->XRot == BGUN_DISMOUNT_FRAME)
		{
			laraItem->AnimNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_DISMOUNT;
			laraItem->FrameNumber = g_Level.Anims[Objects[ID_BIGGUN].animIndex + BGUN_ANIM_DISMOUNT].frameBase;
			laraItem->ActiveState = BGUN_STATE_DISMOUNT;
			laraItem->TargetState = BGUN_STATE_DISMOUNT;
			bGunInfo->Flags = BGUN_FLAG_DISMOUNT;
			bGunInfo->GunRotYAdd = 0;
			bGunInfo->BarrelRotating = false;
		}
		else if (bGunInfo->XRot > BGUN_DISMOUNT_FRAME)
			bGunInfo->XRot--;
		else if (bGunInfo->XRot < BGUN_DISMOUNT_FRAME)
			bGunInfo->XRot++;
	}

	switch (laraItem->ActiveState)
	{
	case BGUN_STATE_MOUNT:
	case BGUN_STATE_DISMOUNT:
		AnimateItem(laraItem);
		bGunItem->AnimNumber = Objects[ID_BIGGUN].animIndex + (laraItem->AnimNumber - Objects[ID_BIGGUN_ANIMS].animIndex);
		bGunItem->FrameNumber = g_Level.Anims[bGunItem->AnimNumber].frameBase + (laraItem->FrameNumber - g_Level.Anims[laraItem->AnimNumber].frameBase);

		if (bGunInfo->Flags & BGUN_FLAG_DISMOUNT && TestLastFrame(laraItem))
		{
			SetAnimation(laraItem, LA_STAND_IDLE);
			laraInfo->Vehicle = NO_ITEM;
			laraInfo->Control.HandStatus = HandStatus::Free;
			bGunItem->HitPoints = 0;
		}

		break;

	case BGUN_STATE_UP_DOWN:
		laraItem->AnimNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_UP_DOWN;
		laraItem->FrameNumber = g_Level.Anims[Objects[ID_BIGGUN].animIndex + BGUN_ANIM_UP_DOWN].frameBase + bGunInfo->XRot;
		bGunItem->AnimNumber = Objects[ID_BIGGUN].animIndex + (laraItem->AnimNumber - Objects[ID_BIGGUN_ANIMS].animIndex);
		bGunItem->FrameNumber = g_Level.Anims[bGunItem->AnimNumber].frameBase + (laraItem->FrameNumber - g_Level.Anims[laraItem->AnimNumber].frameBase);

		if (bGunInfo->FireCount > 0)
			bGunInfo->FireCount--;
		else
			bGunInfo->FireCount = 0;

		bGunInfo->Flags = BGUN_FLAG_UP_DOWN;
		break;
	}
	
	Camera.targetElevation = -ANGLE(15.0f);

	bGunItem->Position.yRot = bGunInfo->StartYRot + bGunInfo->YRot;
	laraItem->Position.yRot = bGunItem->Position.yRot;
	coll->Setup.EnableSpasm = false;
	coll->Setup.EnableObjectPush = false;

	DoObjectCollision(laraItem, coll);
	
	return true;
}
