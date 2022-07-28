#include "framework.h"
#include "Objects/TR3/Vehicles/big_gun.h"

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
#include "Objects/TR3/Vehicles/big_gun_info.h"
#include "Objects/Utils/VehicleHelpers.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Specific/setup.h"

using std::vector;
using namespace TEN::Input;

namespace TEN::Entities::Vehicles
{
	const vector<VehicleMountType> BigGunMountTypes =
	{
		VehicleMountType::LevelStart,
		VehicleMountType::Back
	};

	constexpr auto BGUN_MOUNT_DISTANCE = CLICK(2);

	constexpr auto BGUN_RECOIL_TIME = 26;
	constexpr auto BGUN_RECOIL_Z = 25;

	constexpr auto BGUN_X_ORIENT_NUM_FRAMES = 59;
	constexpr auto BGUN_X_ORIENT_MIDDLE_FRAME = 30;

	#define BGUN_TURN_RATE_ACCEL ANGLE(0.5f)
	#define BGUN_TURN_RATE_MAX	 ANGLE(4.0f)
	#define BGUN_X_ORIENT_STEP	 (ANGLE(80.0f) / BGUN_X_ORIENT_NUM_FRAMES)
	#define BGUN_X_ORIENT_MAX	 ANGLE(40.0f)

	enum BigGunState
	{
		BGUN_STATE_MOUNT = 0,
		BGUN_STATE_DISMOUNT = 1,
		BGUN_STATE_ROTATE_VERTICALLY = 2,
		BGUN_STATE_RECOIL = 3
	};

	enum BigGunAnim
	{
		BGUN_ANIM_MOUNT = 0,
		BGUN_ANIM_DISMOUNT = 1,
		BGUN_ANIM_ROTATE_VERTICALLY = 2,
		BGUN_ANIM_RECOIL = 3
	};

	enum BigGunFlags
	{
		BGUN_FLAG_UP_DOWN = (1 << 0),
		BGUN_FLAG_AUTO_ROT = (1 << 2),
		BGUN_FLAG_DISMOUNT = (1 << 3),
		BGUN_FLAG_FIRE = (1 << 4)
	};

	BigGunInfo* GetBigGunInfo(ItemInfo* bigGunItem)
	{
		return (BigGunInfo*)bigGunItem->Data;
	}

	void BigGunInitialise(short itemNumber)
	{
		auto* bigGunItem = &g_Level.Items[itemNumber];
		bigGunItem->Data = BigGunInfo();
		auto* bigGun = GetBigGunInfo(bigGunItem);

		bigGun->BaseOrientation = bigGunItem->Pose.Orientation;
		bigGun->XOrientFrame = BGUN_X_ORIENT_MIDDLE_FRAME;
	}

	static bool BigGunTestMount(ItemInfo* bigGunItem, ItemInfo* laraItem)
	{
		// TODO: If Lara global is not used, the game crashes upon level load. Not sure why. @Sezz 2022.01.09
		auto* lara = &Lara/* GetLaraInfo(laraItem)*/;

		if (!(TrInput & IN_ACTION) ||
			lara->Control.HandStatus != HandStatus::Free ||
			laraItem->Animation.IsAirborne)
		{
			return false;
		}

		int x = laraItem->Pose.Position.x - bigGunItem->Pose.Position.x;
		int y = laraItem->Pose.Position.y - bigGunItem->Pose.Position.y;
		int z = laraItem->Pose.Position.z - bigGunItem->Pose.Position.z;

		int distance = pow(x, 2) + pow(y, 2) + pow(z, 2);
		if (distance > SECTOR(30))
			return false;

		short deltaAngle = abs(laraItem->Pose.Orientation.y - bigGunItem->Pose.Orientation.y);
		if (deltaAngle > ANGLE(35.0f) || deltaAngle < -ANGLE(35.0f))
			return false;

		return true;
	}

	void BigGunFire(ItemInfo* bigGunItem, ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* bigGun = GetBigGunInfo(bigGunItem);

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

			projectileItem->Animation.Velocity = 16;
			projectileItem->Pose.Orientation = Vector3Shrt(
				-((bigGun->XOrientFrame - 32) * ANGLE(1.0f)),
				bigGunItem->Pose.Orientation.y,
				0
			);
			projectileItem->ItemFlags[0] = BGUN_FLAG_UP_DOWN;

			AddActiveItem(itemNumber);

			lara->LeftArm.GunSmoke = 32;

			for (int i = 0; i < 5; i++)
				TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 1, LaraWeaponType::RocketLauncher, lara->LeftArm.GunSmoke);

			SoundEffect(SFX_TR4_EXPLOSION1, &projectileItem->Pose);
		}
	}

	void BigGunCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* bigGunItem = &g_Level.Items[itemNumber];
		auto* bigGun = GetBigGunInfo(bigGunItem);
		auto* lara = GetLaraInfo(laraItem);

		if (laraItem->HitPoints <= 0 || lara->Vehicle != NO_ITEM)
			return;

		if (BigGunTestMount(laraItem, bigGunItem))
		{
			lara->Vehicle = itemNumber;

			DoVehicleFlareDiscard(laraItem);
			laraItem->Animation.AnimNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_MOUNT;
			laraItem->Animation.FrameNumber = g_Level.Anims[Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_MOUNT].frameBase;
			laraItem->Animation.ActiveState = BGUN_STATE_MOUNT;
			laraItem->Animation.TargetState = BGUN_STATE_MOUNT;
			laraItem->Animation.IsAirborne = false;
			laraItem->Pose = bigGunItem->Pose;
			lara->Control.HandStatus = HandStatus::Busy;
			bigGunItem->HitPoints = 1;
			bigGun->XOrientFrame = BGUN_X_ORIENT_MIDDLE_FRAME;
			bigGun->Flags = 0;
		}
		else
			ObjectCollision(itemNumber, laraItem, coll);
	}

	bool BigGunControl(ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* bigGunItem = &g_Level.Items[lara->Vehicle];
		auto* bigGun = GetBigGunInfo(bigGunItem);

		if (bigGun->Flags & BGUN_FLAG_UP_DOWN)
		{
			if (bigGun->IsBarrelRotating)
				bigGun->BarrelRotation--;

			if (!bigGun->BarrelRotation)
				bigGun->IsBarrelRotating = false;

			if (TrInput & VEHICLE_IN_DISMOUNT || laraItem->HitPoints <= 0)
				bigGun->Flags = BGUN_FLAG_AUTO_ROT;
			else
			{
				if (TrInput & VEHICLE_IN_FIRE && !bigGun->FireCount)
				{
					BigGunFire(bigGunItem, laraItem);
					bigGun->FireCount = BGUN_RECOIL_TIME;
					bigGun->BarrelRotation = BGUN_RECOIL_Z;
					bigGun->IsBarrelRotating = true;
				}

				if (TrInput & VEHICLE_IN_UP)
				{
					if (bigGun->TurnRate.x < 0)
						bigGun->TurnRate.x /= 2;

					bigGun->TurnRate.x += BGUN_TURN_RATE_ACCEL;
					if (bigGun->TurnRate.x > (BGUN_TURN_RATE_MAX / 2))
						bigGun->TurnRate.x = (BGUN_TURN_RATE_MAX / 2);
				}
				else if (TrInput & VEHICLE_IN_DOWN)
				{
					if (bigGun->TurnRate.x > 0)
						bigGun->TurnRate.x /= 2;

					bigGun->TurnRate.x -= BGUN_TURN_RATE_ACCEL;
					if (bigGun->TurnRate.x < (-BGUN_TURN_RATE_MAX / 2))
						bigGun->TurnRate.x = (-BGUN_TURN_RATE_MAX / 2);
				}
				else
				{
					bigGun->TurnRate.x -= bigGun->TurnRate.x / 3;
					if (abs(bigGun->TurnRate.x) < BGUN_TURN_RATE_ACCEL)
						bigGun->TurnRate.x = 0;
				}

				if (TrInput & VEHICLE_IN_LEFT)
				{
					if (bigGun->TurnRate.y > 0)
						bigGun->TurnRate.y /= 2;

					bigGun->TurnRate.y -= BGUN_TURN_RATE_ACCEL;
					if (bigGun->TurnRate.y < -BGUN_TURN_RATE_MAX)
						bigGun->TurnRate.y = -BGUN_TURN_RATE_MAX;
				}
				else if (TrInput & VEHICLE_IN_RIGHT)
				{
					if (bigGun->TurnRate.y < 0)
						bigGun->TurnRate.y /= 2;

					bigGun->TurnRate.y += BGUN_TURN_RATE_ACCEL;
					if (bigGun->TurnRate.y > BGUN_TURN_RATE_MAX)
						bigGun->TurnRate.y = BGUN_TURN_RATE_MAX;
				}
				else
				{
					bigGun->TurnRate.y -= bigGun->TurnRate.y / 3;
					if (abs(bigGun->TurnRate.y) < BGUN_TURN_RATE_ACCEL)
						bigGun->TurnRate.y = 0;
				}

				bigGun->Rotation.x += bigGun->TurnRate.x;
				bigGun->Rotation.y += bigGun->TurnRate.y;

				if (bigGun->Rotation.x > BGUN_X_ORIENT_MAX)
					bigGun->Rotation.x = BGUN_X_ORIENT_MAX;
				else if (bigGun->Rotation.x < -BGUN_X_ORIENT_MAX)
					bigGun->Rotation.x = -BGUN_X_ORIENT_MAX;

				bigGun->XOrientFrame = (int)round((bigGun->Rotation.x + BGUN_X_ORIENT_MAX) / BGUN_X_ORIENT_STEP);
			}
		}

		if (bigGun->Flags & BGUN_FLAG_AUTO_ROT)
		{
			if (bigGun->XOrientFrame == BGUN_X_ORIENT_MIDDLE_FRAME)
			{
				laraItem->Animation.AnimNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_DISMOUNT;
				laraItem->Animation.FrameNumber = g_Level.Anims[Objects[ID_BIGGUN].animIndex + BGUN_ANIM_DISMOUNT].frameBase;
				laraItem->Animation.ActiveState = BGUN_STATE_DISMOUNT;
				laraItem->Animation.TargetState = BGUN_STATE_DISMOUNT;
				bigGun->TurnRate.y = 0;
				bigGun->IsBarrelRotating = false;
				bigGun->Flags = BGUN_FLAG_DISMOUNT;
			}
			else if (bigGun->Rotation.x > 0)
				bigGun->Rotation.x -= BGUN_X_ORIENT_STEP;
			else if (bigGun->Rotation.x < 0)
				bigGun->Rotation.x += BGUN_X_ORIENT_STEP;

			bigGun->XOrientFrame = (int)round((bigGun->Rotation.x + BGUN_X_ORIENT_MAX) / BGUN_X_ORIENT_STEP);
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

		case BGUN_STATE_ROTATE_VERTICALLY:
			bigGunItem->Animation.AnimNumber = Objects[ID_BIGGUN].animIndex + (laraItem->Animation.AnimNumber - Objects[ID_BIGGUN_ANIMS].animIndex);
			bigGunItem->Animation.FrameNumber = g_Level.Anims[bigGunItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);
			
			if (bigGun->FireCount > 0)
				bigGun->FireCount--;
			else
				bigGun->FireCount = 0;

			bigGun->Flags = BGUN_FLAG_UP_DOWN;

			laraItem->Animation.AnimNumber = Objects[ID_BIGGUN_ANIMS].animIndex + BGUN_ANIM_ROTATE_VERTICALLY;
			laraItem->Animation.FrameNumber = g_Level.Anims[Objects[ID_BIGGUN].animIndex + BGUN_ANIM_ROTATE_VERTICALLY].frameBase + bigGun->XOrientFrame;
			break;
		}

		Camera.targetElevation = -(bigGun->Rotation.x + ANGLE(15.0f));

		bigGunItem->Pose.Orientation.y = bigGun->BaseOrientation.y + bigGun->Rotation.y;
		laraItem->Pose.Orientation.y = bigGunItem->Pose.Orientation.y;
		coll->Setup.EnableObjectPush = false;
		coll->Setup.EnableSpasm = false;

		DoObjectCollision(laraItem, coll);

		return true;
	}
}
