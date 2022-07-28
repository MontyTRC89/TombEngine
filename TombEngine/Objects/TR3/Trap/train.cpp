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
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#define TRAIN_VEL	260
#define LARA_TRAIN_DEATH_ANIM 3;

long TrainTestHeight(ItemInfo* item, long x, long z, short* roomNumber)
{
	float s = phd_sin(item->Pose.Orientation.y);
	float c = phd_cos(item->Pose.Orientation.y);

	Vector3Int pos;
	pos.x = round(item->Pose.Position.x + z * s + x * c);
	pos.y = round(item->Pose.Position.y - z * phd_sin(item->Pose.Orientation.x) + x * phd_sin(item->Pose.Orientation.z));
	pos.z = round(item->Pose.Position.z + z * c - x * s);

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

	float s = phd_sin(item->Pose.Orientation.y);
	float c = phd_cos(item->Pose.Orientation.y);

	item->Pose.Position.x += item->ItemFlags[1] * s;
	item->Pose.Position.z += item->ItemFlags[1] * c;

	short roomNumber;
	long rh = TrainTestHeight(item, 0, SECTOR(5), &roomNumber);
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

	TriggerDynamicLight(item->Pose.Position.x + SECTOR(3) * s, item->Pose.Position.y, item->Pose.Position.z + SECTOR(3) * c, 16, 31, 31, 31);

	if (item->ItemFlags[1] != TRAIN_VEL)
	{
		item->ItemFlags[1] -= 48;
		if (item->ItemFlags[1] < 0)
			item->ItemFlags[1] = 0;

		if (!UseForcedFixedCamera)
		{
			ForcedFixedCamera.x = item->Pose.Position.x + SECTOR(8) * s;
			ForcedFixedCamera.z = item->Pose.Position.z + SECTOR(8) * c;

			ForcedFixedCamera.y = GetCollision(ForcedFixedCamera.x, item->Pose.Position.y - CLICK(2), ForcedFixedCamera.z, item->RoomNumber).Position.Floor;

			ForcedFixedCamera.roomNumber = roomNumber;
			UseForcedFixedCamera = 1;
		}
	}
	else
		SoundEffect(SFX_TR3_TUBE_LOOP, &item->Pose, SoundEnvironment::Always);
}

void TrainCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* trainItem = &g_Level.Items[itemNumber];

	if (!TestBoundsCollide(trainItem, laraItem, coll->Setup.Radius))
		return;
	if (!TestCollision(trainItem, laraItem))
		return;

	SoundEffect(SFX_TR4_LARA_GENERAL_DEATH, &laraItem->Pose, SoundEnvironment::Always);
	SoundEffect(SFX_TR4_LARA_HIGH_FALL_DEATH, &laraItem->Pose, SoundEnvironment::Always);
	StopSoundEffect(SFX_TR3_TUBE_LOOP);

	laraItem->Animation.AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex + LARA_TRAIN_DEATH_ANIM;
	laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
	// laraItem->Animation.ActiveState = EXTRA_TRAINKILL;
	// laraItem->Animation.TargetState = EXTRA_TRAINKILL;
	laraItem->Pose.Orientation.y = trainItem->Pose.Orientation.y;
	laraItem->Animation.Velocity = 0;
	laraItem->Animation.VerticalVelocity = 0;
	laraItem->Animation.IsAirborne = false;

	DoDamage(laraItem, INT_MAX);

	AnimateItem(laraItem);

	laraInfo->ExtraAnim = 1;
	laraInfo->Control.HandStatus = HandStatus::Busy;
	laraInfo->Control.Weapon.GunType = LaraWeaponType::None;
	laraInfo->HitDirection = -1;
	laraInfo->Air = -1;

	trainItem->ItemFlags[1] = 160;

	float s = phd_sin(trainItem->Pose.Orientation.y);
	float c = phd_cos(trainItem->Pose.Orientation.y);

	long x = laraItem->Pose.Position.x + CLICK(1) * s;
	long z = laraItem->Pose.Position.z + CLICK(1) * c;

	DoLotsOfBlood(x, laraItem->Pose.Position.y - CLICK(2), z, SECTOR(1), trainItem->Pose.Orientation.y, laraItem->RoomNumber, 15);
}
