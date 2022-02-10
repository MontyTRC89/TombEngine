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
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

#define TRAIN_VEL	260
#define LARA_TRAIN_DEATH_ANIM 3;

long TrainTestHeight(ITEM_INFO *item, long x, long z, short *room_number)
{
	float s, c;
	PHD_VECTOR pos;
	FLOOR_INFO *floor;

	c = phd_cos(item->Position.yRot);
	s = phd_sin(item->Position.yRot);

	pos.x = item->Position.xPos + z * s + x * c;
	pos.y = item->Position.yPos - z * phd_sin(item->Position.xRot) + x * phd_sin(item->Position.zRot);
	pos.z = item->Position.zPos + z * c - x * s;

	*room_number = item->RoomNumber;
	floor = GetFloor(pos.x, pos.y, pos.z, room_number);
	return GetFloorHeight(floor, pos.x, pos.y, pos.z);
}

void TrainControl(short trainNum)
{
	ITEM_INFO *train;
	long fh, rh;
	FLOOR_INFO *floor;
	short roomNum;
	float s, c;

	train = &g_Level.Items[trainNum];

	if (!TriggerActive(train))
		return;

	if (train->ItemFlags[0] == 0)
		train->ItemFlags[0] = train->ItemFlags[1] = TRAIN_VEL;

	c = phd_cos(train->Position.yRot);
	s = phd_sin(train->Position.yRot);

	train->Position.xPos += train->ItemFlags[1] * s;
	train->Position.zPos += train->ItemFlags[1] * c;

	rh = TrainTestHeight(train, 0, 5120, &roomNum);
	train->Position.yPos = fh = TrainTestHeight(train, 0, 0, &roomNum);

	if (fh == NO_HEIGHT)
	{
		KillItem(trainNum);
		return;
	}

	train->Position.yPos -= 32;// ?

	roomNum = train->RoomNumber;
	GetFloor(train->Position.xPos, train->Position.yPos, train->Position.zPos, &roomNum);

	if (roomNum != train->RoomNumber)
		ItemNewRoom(trainNum, roomNum);

	train->Position.xRot = -(rh - fh) * 2;

	TriggerDynamicLight(train->Position.xPos + 3072 * s, train->Position.yPos, train->Position.zPos + 3072 * c, 16, 31, 31, 31);

	if (train->ItemFlags[1] != TRAIN_VEL)
	{
		if ((train->ItemFlags[1] -= 48) < 0)
			train->ItemFlags[1] = 0;

		if (!UseForcedFixedCamera)
		{
			ForcedFixedCamera.x = train->Position.xPos + 8192 * s;
			ForcedFixedCamera.z = train->Position.zPos + 8192 * c;

			roomNum = train->RoomNumber;
			floor = GetFloor(ForcedFixedCamera.x, train->Position.yPos - 512, ForcedFixedCamera.z, &roomNum);
			ForcedFixedCamera.y = GetFloorHeight(floor, ForcedFixedCamera.x, train->Position.yPos - 512, ForcedFixedCamera.z);

			ForcedFixedCamera.roomNumber = roomNum;

			UseForcedFixedCamera = 1;
		}
	}
	else
		SoundEffect(SFX_TR3_TUBE_LOOP, &train->Position, SFX_ALWAYS);
}

void TrainCollision(short trainNum, ITEM_INFO *larA, COLL_INFO *coll)
{
	ITEM_INFO *train;
	long x, z;
	float s, c;

	train = &g_Level.Items[trainNum];

	if (!TestBoundsCollide(train, larA, coll->Setup.Radius))
		return;
	if (!TestCollision(train, larA))
		return;

	SoundEffect(SFX_TR3_LARA_GENERAL_DEATH, &larA->Position, SFX_ALWAYS);
	SoundEffect(SFX_TR3_LARA_FALLDETH, &larA->Position, SFX_ALWAYS);
	StopSoundEffect(SFX_TR3_TUBE_LOOP);

	larA->AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex + LARA_TRAIN_DEATH_ANIM;
	larA->FrameNumber = g_Level.Anims[larA->AnimNumber].frameBase;
//	larA->ActiveState = EXTRA_TRAINKILL;
//	larA->TargetState = EXTRA_TRAINKILL;
	larA->HitPoints = 0;

	larA->Position.yRot = train->Position.yRot;

	larA->VerticalVelocity = 0;
	larA->Airborne = false;
	larA->Velocity = 0;

	AnimateItem(larA);

	Lara.ExtraAnim = 1;
	Lara.Control.HandStatus = HandStatus::Busy;
	Lara.Control.WeaponControl.GunType = WEAPON_NONE;
	Lara.hitDirection = -1;
	Lara.air = -1;

	train->ItemFlags[1] = 160;

	c = phd_cos(train->Position.yRot);
	s = phd_sin(train->Position.yRot);

	x = larA->Position.xPos + 256 * s;
	z = larA->Position.zPos + 256 * c;

	DoLotsOfBlood(x, larA->Position.yPos - 512, z, 1024, train->Position.yRot, larA->RoomNumber, 15);

}
