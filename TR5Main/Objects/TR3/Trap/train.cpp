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

long TrainTestHeight(ITEM_INFO* item, long x, long z, short* roomNumber)
{
	float s = phd_sin(item->Position.yRot);
	float c = phd_cos(item->Position.yRot);

	PHD_VECTOR pos;
	pos.x = item->Position.xPos + z * s + x * c;
	pos.y = item->Position.yPos - z * phd_sin(item->Position.xRot) + x * phd_sin(item->Position.zRot);
	pos.z = item->Position.zPos + z * c - x * s;

	auto probe = GetCollisionResult(pos.x, pos.y, pos.z, item->RoomNumber);

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

	float s = phd_sin(item->Position.yRot);
	float c = phd_cos(item->Position.yRot);

	item->Position.xPos += item->ItemFlags[1] * s;
	item->Position.zPos += item->ItemFlags[1] * c;

	short roomNumber;
	long rh = TrainTestHeight(item, 0, SECTOR(5), &roomNumber);
	long floorHeight = TrainTestHeight(item, 0, 0, &roomNumber);
	item->Position.yPos = floorHeight;

	if (floorHeight == NO_HEIGHT)
	{
		KillItem(itemNumber);
		return;
	}

	item->Position.yPos -= 32;// ?

	short probedRoomNumber = GetCollisionResult(item).RoomNumber;
	if (probedRoomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, probedRoomNumber);

	item->Position.xRot = -(rh - floorHeight) * 2;

	TriggerDynamicLight(item->Position.xPos + SECTOR(3) * s, item->Position.yPos, item->Position.zPos + SECTOR(3) * c, 16, 31, 31, 31);

	if (item->ItemFlags[1] != TRAIN_VEL)
	{
		item->ItemFlags[1] -= 48;
		if (item->ItemFlags[1] < 0)
			item->ItemFlags[1] = 0;

		if (!UseForcedFixedCamera)
		{
			ForcedFixedCamera.x = item->Position.xPos + SECTOR(8) * s;
			ForcedFixedCamera.z = item->Position.zPos + SECTOR(8) * c;

			ForcedFixedCamera.y = GetCollisionResult(ForcedFixedCamera.x, item->Position.yPos - CLICK(2), ForcedFixedCamera.z, item->RoomNumber).Position.Floor;

			ForcedFixedCamera.roomNumber = roomNumber;
			UseForcedFixedCamera = 1;
		}
	}
	else
		SoundEffect(SFX_TR3_TUBE_LOOP, &item->Position, SFX_ALWAYS);
}

void TrainCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* trainItem = &g_Level.Items[itemNumber];

	if (!TestBoundsCollide(trainItem, laraItem, coll->Setup.Radius))
		return;
	if (!TestCollision(trainItem, laraItem))
		return;

	SoundEffect(SFX_LARA_GENERAL_DEATH, &laraItem->Position, SFX_ALWAYS);
	SoundEffect(SFX_LARA_HIGH_FALL_DEATH, &laraItem->Position, SFX_ALWAYS);
	StopSoundEffect(SFX_TR3_TUBE_LOOP);

	laraItem->AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex + LARA_TRAIN_DEATH_ANIM;
	laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
//	larA->ActiveState = EXTRA_TRAINKILL;
//	larA->TargetState = EXTRA_TRAINKILL;
	laraItem->HitPoints = 0;
	laraItem->Position.yRot = trainItem->Position.yRot;
	laraItem->Velocity = 0;
	laraItem->VerticalVelocity = 0;
	laraItem->Airborne = false;

	AnimateItem(laraItem);

	laraInfo->ExtraAnim = 1;
	laraInfo->Control.HandStatus = HandStatus::Busy;
	laraInfo->Control.WeaponControl.GunType = WEAPON_NONE;
	laraInfo->hitDirection = -1;
	laraInfo->Air = -1;

	trainItem->ItemFlags[1] = 160;

	float s = phd_sin(trainItem->Position.yRot);
	float c = phd_cos(trainItem->Position.yRot);

	long x = laraItem->Position.xPos + CLICK(1) * s;
	long z = laraItem->Position.zPos + CLICK(1) * c;

	DoLotsOfBlood(x, laraItem->Position.yPos - CLICK(2), z, SECTOR(1), trainItem->Position.yRot, laraItem->RoomNumber, 15);
}
