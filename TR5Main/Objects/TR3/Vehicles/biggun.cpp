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

#define BGUN_TURN_RATE	EulerAngle::DegToRad(2.0f)
#define BGUN_TURN_MAX	EulerAngle::DegToRad(16.0f)

#define RECOIL_TIME 26
#define RECOIL_Z	25

#define BGUN_UP_DOWN_FRAMES	59
#define BGUN_DISMOUNT_FRAME	30

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

void BigGunInitialise(short itemNumber)
{
	auto* bigGunItem = &g_Level.Items[itemNumber];
	bigGunItem->Data = BigGunInfo();
	auto* bigGun = (BigGunInfo*)bigGunItem->Data;

	bigGun->Orientation.x = BGUN_DISMOUNT_FRAME;
	bigGun->Orientation.z = 0;
	bigGun->StartYRot = bigGunItem->Orientation.y;
	bigGun->GunRotYAdd = 0;
	bigGun->FireCount = 0;
	bigGun->Flags = 0;
	bigGun->BarrelRotating = false;
}

static bool BigGunTestMount(ITEM_INFO* laraItem, ITEM_INFO* bigGunItem)
{
	// TODO: If Lara global is not used, the game crashes upon level load. Not sure why. @Sezz 2022.01.09
	auto* lara = &Lara/* GetLaraInfo(laraItem)*/;

	if (!(TrInput & IN_ACTION) ||
		lara->Control.HandStatus != HandStatus::Free ||
		laraItem->Animation.Airborne)
	{
		return false;
	}

	int x = laraItem->Pose.Position.x - bigGunItem->Pose.Position.x;
	int y = laraItem->Pose.Position.y - bigGunItem->Pose.Position.y;
	int z = laraItem->Pose.Position.z - bigGunItem->Pose.Position.z;

	int distance = pow(x, 2) + pow(y, 2) + pow(z, 2);
		return false;

	float deltaAngle = EulerAngle::Clamp(laraItem->Orientation.y - bigGunItem->Orientation.y);
	if (deltaAngle > EulerAngle::DegToRad(35.0f) || deltaAngle < EulerAngle::DegToRad(-35.0f))
		return false;

	return true;
}

void BigGunFire(ITEM_INFO* laraItem, ITEM_INFO* bigGunItem)
{
	auto* bigGun = (BigGunInfo*)bigGunItem->Data;

	short itemNumber = CreateItem();
	auto* projectileItem = &g_Level.Items[itemNumber];

	if (itemNumber != NO_ITEM)
	{
		projectileItem->ObjectNumber = ID_ROCKET;
		projectileItem->RoomNumber = laraItem->RoomNumber;

		auto pos = Vector3Int(0, 0, CLICK(1)); // CLICK(1) or 520?
		GetJointAbsPosition(bigGunItem, &pos, 2);
			
		projectileItem->Pose.Position = pos;

		InitialiseItem(itemNumber);

		projectileItem->Orientation.x = -((bigGun->Orientation.x - 32) * EulerAngle::DegToRad(1.0f));
		projectileItem->Orientation.y = bigGunItem->Orientation.y;
		projectileItem->Orientation.z = 0;
		projectileItem->Animation.Velocity = 16;
		projectileItem->ItemFlags[0] = BGUN_FLAG_UP_DOWN;

		AddActiveItem(itemNumber);

		SmokeCountL = 32;
		SmokeWeapon = LaraWeaponType::RocketLauncher;

		for (int i = 0; i < 5; i++)
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 1, LaraWeaponType::RocketLauncher, 32);

		SoundEffect(SFX_TR4_EXPLOSION1, 0, 0);
	}
}

void BigGunCollision(short itemNum, ITEM_INFO* laraItem, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* bigGunItem = &g_Level.Items[itemNum];
	auto* bigGun = (BigGunInfo*)bigGunItem->Data;

	if (laraItem->HitPoints <= 0 || lara->Vehicle != NO_ITEM)
		return;

	if (BigGunTestMount(bigGunItem, laraItem))
	{
		lara->Vehicle = itemNum;

		if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
		{
			CreateFlare(laraItem, ID_FLARE_ITEM, false);
			UndrawFlareMeshes(laraItem);

			lara->Flare.ControlLeft = false;
			lara->Control.Weapon.RequestGunType = LaraWeaponType::None;
			lara->Control.Weapon.GunType = LaraWeaponType::None;
		}

		laraItem->Animation.AnimNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_MOUNT;
		laraItem->Animation.FrameNumber = g_Level.Anims[Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_MOUNT].frameBase;
		laraItem->Animation.TargetState = BGUN_STATE_MOUNT;
		laraItem->Animation.ActiveState = BGUN_STATE_MOUNT;
		laraItem->Pose = bigGunItem->Pose;
		laraItem->Animation.Airborne = false;
		lara->Control.HandStatus = HandStatus::Busy;
		bigGunItem->HitPoints = 1;
		bigGun->Flags = 0;
		bigGun->Orientation.x = BGUN_DISMOUNT_FRAME;

	}
	else
		ObjectCollision(itemNum, laraItem, coll);
}

bool BigGunControl(ITEM_INFO* laraItem, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(laraItem);
	auto* bigGunItem = &g_Level.Items[lara->Vehicle];
	auto* bigGun = (BigGunInfo*)bigGunItem->Data;

	if (bigGun->Flags & BGUN_FLAG_UP_DOWN)
	{
		if (bigGun->BarrelRotating)
			bigGun->BarrelZRotation--;

		if (!bigGun->BarrelZRotation)
			bigGun->BarrelRotating = false;

		if (TrInput & BGUN_IN_DISMOUNT || laraItem->HitPoints <= 0)
			bigGun->Flags = BGUN_FLAG_AUTO_ROT;
		else
		{
			if (TrInput & BGUN_IN_FIRE && bigGun->FireCount == 0)
			{
				BigGunFire(laraItem, bigGunItem);
				bigGun->FireCount = RECOIL_TIME;
				bigGun->BarrelZRotation = RECOIL_Z;
				bigGun->BarrelRotating = true;
			}

			if (TrInput & BGUN_IN_LEFT)
			{
				if (bigGun->GunRotYAdd > 0)
					bigGun->GunRotYAdd /= 2;

				bigGun->GunRotYAdd -= BGUN_TURN_RATE;
				if (bigGun->GunRotYAdd < -BGUN_TURN_MAX)
					bigGun->GunRotYAdd = -BGUN_TURN_MAX;
			}
			else if (TrInput & BGUN_IN_RIGHT)
			{
				if (bigGun->GunRotYAdd < 0)
					bigGun->GunRotYAdd /= 2;

				bigGun->GunRotYAdd += BGUN_TURN_RATE;
				if (bigGun->GunRotYAdd > BGUN_TURN_MAX)
					bigGun->GunRotYAdd = BGUN_TURN_MAX;
			}
			else
			{
				bigGun->GunRotYAdd -= bigGun->GunRotYAdd / 4;
				if (abs(bigGun->GunRotYAdd) < BGUN_TURN_RATE)
					bigGun->GunRotYAdd = 0;
			}

			bigGun->Orientation.z += bigGun->GunRotYAdd / 4;

			if (TrInput & BGUN_IN_UP && bigGun->Orientation.x < BGUN_UP_DOWN_FRAMES)
				bigGun->Orientation.x++;			
			else if (TrInput & BGUN_IN_DOWN && bigGun->Orientation.x)
				bigGun->Orientation.x--;
		}
	}

	if (bigGun->Flags & BGUN_FLAG_AUTO_ROT)
	{
		if (bigGun->Orientation.x == BGUN_DISMOUNT_FRAME)
		{
			laraItem->Animation.AnimNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_DISMOUNT;
			laraItem->Animation.FrameNumber = g_Level.Anims[Objects[ID_BIGGUN].animIndex + BGUN_ANIM_DISMOUNT].frameBase;
			laraItem->Animation.ActiveState = BGUN_STATE_DISMOUNT;
			laraItem->Animation.TargetState = BGUN_STATE_DISMOUNT;
			bigGun->GunRotYAdd = 0;
			bigGun->BarrelRotating = false;
			bigGun->Flags = BGUN_FLAG_DISMOUNT;
		}
		else if (bigGun->Orientation.x > BGUN_DISMOUNT_FRAME)
			bigGun->Orientation.x--;
		else if (bigGun->Orientation.x < BGUN_DISMOUNT_FRAME)
			bigGun->Orientation.x++;
	}

	switch (laraItem->Animation.ActiveState)
	{
	case BGUN_STATE_MOUNT:
	case BGUN_STATE_DISMOUNT:
		AnimateItem(laraItem);
		bigGunItem->Animation.AnimNumber = Objects[ID_BIGGUN].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_BIGGUN_ANIMS].animIndex);
		bigGunItem->Animation.FrameNumber = g_Level.Anims[bigGunItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);

		if (bigGun->Flags & BGUN_FLAG_DISMOUNT && TestLastFrame(laraItem))
		{
			SetAnimation(laraItem, LA_STAND_IDLE);
			lara->Vehicle = NO_ITEM;
			lara->Control.HandStatus = HandStatus::Free;
			bigGunItem->HitPoints = 0;
		}

		break;

	case BGUN_STATE_UP_DOWN:
		laraItem->Animation.AnimNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_UP_DOWN;
		laraItem->Animation.FrameNumber = g_Level.Anims[Objects[ID_BIGGUN].animIndex + BGUN_ANIM_UP_DOWN].frameBase + bigGun->Orientation.x;
		bigGunItem->Animation.AnimNumber = Objects[ID_BIGGUN].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_BIGGUN_ANIMS].animIndex);
		bigGunItem->Animation.FrameNumber = g_Level.Anims[bigGunItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);

		if (bigGun->FireCount > 0)
			bigGun->FireCount--;
		else
			bigGun->FireCount = 0;

		bigGun->Flags = BGUN_FLAG_UP_DOWN;
		break;
	}
	
	Camera.targetElevation = EulerAngle::DegToRad(-15.0f);

	bigGunItem->Orientation.y = bigGun->StartYRot + bigGun->Orientation.z;
	laraItem->Orientation.y = bigGunItem->Orientation.y;
	coll->Setup.EnableSpasm = false;
	coll->Setup.EnableObjectPush = false;

	DoObjectCollision(laraItem, coll);
	
	return true;
}
