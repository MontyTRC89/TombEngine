#include "framework.h"
#include "Objects/TR3/Trap/train.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/floordata.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Setup.h"
#include "Sound/sound.h"
#include "Specific/level.h"

constexpr auto TRAIN_VEL = 260;

long TrainTestHeight(ItemInfo* item, long x, long z, short* roomNumber)
{
	float sinX = phd_sin(item->Pose.Orientation.x);
	float sinY = phd_sin(item->Pose.Orientation.y);
	float cosY = phd_cos(item->Pose.Orientation.y);
	float sinZ = phd_sin(item->Pose.Orientation.z);

	auto pos = Vector3i(
		round(item->Pose.Position.x + ((z * sinY) + (x * cosY))),
		round(item->Pose.Position.y - ((z * sinX) + (x * sinZ))),
		round(item->Pose.Position.z + ((z * cosY) - (x * sinY)))
	);
	auto probe = GetCollision(pos.x, pos.y, pos.z, item->RoomNumber);

	*roomNumber = probe.RoomNumber;
	return probe.Position.Floor;
}

void TrainControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
		return;

	if (item->ItemFlags[0] == 0)
		item->ItemFlags[0] = item->ItemFlags[1] = TRAIN_VEL;

	float sinY = phd_sin(item->Pose.Orientation.y);
	float cosY = phd_cos(item->Pose.Orientation.y);

	item->Pose.Position.x += item->ItemFlags[1] * sinY;
	item->Pose.Position.z += item->ItemFlags[1] * cosY;

	short roomNumber;
	long rh = TrainTestHeight(item, 0, BLOCK(5), &roomNumber);
	long floorHeight = TrainTestHeight(item, 0, 0, &roomNumber);
	item->Pose.Position.y = floorHeight;

	if (floorHeight == NO_HEIGHT)
	{
		KillItem(itemNumber);
		return;
	}

	item->Pose.Position.y -= 32;// ?

	short probedRoomNumber = GetCollision(item).RoomNumber;
	if (probedRoomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, probedRoomNumber);

	item->Pose.Orientation.x = -(rh - floorHeight) * 2;

	TriggerDynamicLight(item->Pose.Position.x + BLOCK(3) * sinY, item->Pose.Position.y, item->Pose.Position.z + BLOCK(3) * cosY, 16, 31, 31, 31);

	if (item->ItemFlags[1] != TRAIN_VEL)
	{
		item->ItemFlags[1] -= 48;
		if (item->ItemFlags[1] < 0)
			item->ItemFlags[1] = 0;

		if (!UseForcedFixedCamera)
		{
			ForcedFixedCamera.x = item->Pose.Position.x + BLOCK(8) * sinY;
			ForcedFixedCamera.z = item->Pose.Position.z + BLOCK(8) * cosY;

			ForcedFixedCamera.y = GetCollision(ForcedFixedCamera.x, item->Pose.Position.y - CLICK(2), ForcedFixedCamera.z, item->RoomNumber).Position.Floor;

			ForcedFixedCamera.RoomNumber = roomNumber;
			UseForcedFixedCamera = 1;
		}
	}
	else
		SoundEffect(SFX_TR3_TUBE_LOOP, &item->Pose, SoundEnvironment::Always);
}

void TrainCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* laraInfo = GetLaraInfo(laraItem);

	if (!TestBoundsCollide(item, laraItem, coll->Setup.Radius))
		return;

	if (!TestCollision(item, laraItem))
		return;

	SoundEffect(SFX_TR4_LARA_GENERAL_DEATH, &laraItem->Pose, SoundEnvironment::Always);
	SoundEffect(SFX_TR4_LARA_HIGH_FALL_DEATH, &laraItem->Pose, SoundEnvironment::Always);
	StopSoundEffect(SFX_TR3_TUBE_LOOP);

	SetAnimation(*item, ID_LARA_EXTRA_ANIMS, LEA_TRAIN_DEATH_START);
	laraItem->Animation.IsAirborne = false;
	laraItem->Animation.Velocity.y = 0.0f;
	laraItem->Animation.Velocity.z = 0.0f;
	laraItem->Pose.Orientation.y = item->Pose.Orientation.y;

	DoDamage(laraItem, INT_MAX);

	AnimateItem(laraItem);

	laraInfo->ExtraAnim = 1;
	laraInfo->Control.HandStatus = HandStatus::Busy;
	laraInfo->Control.Weapon.GunType = LaraWeaponType::None;
	laraInfo->HitDirection = -1;
	laraInfo->Status.Air = -1;

	item->ItemFlags[1] = 160;

	float sinY = phd_sin(item->Pose.Orientation.y);
	float cosY = phd_cos(item->Pose.Orientation.y);

	long x = laraItem->Pose.Position.x + CLICK(1) * sinY;
	long z = laraItem->Pose.Position.z + CLICK(1) * cosY;

	DoLotsOfBlood(x, laraItem->Pose.Position.y - CLICK(2), z, BLOCK(1), item->Pose.Orientation.y, laraItem->RoomNumber, 15);
}
