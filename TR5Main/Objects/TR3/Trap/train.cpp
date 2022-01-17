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

	c = phd_cos(item->pos.yRot);
	s = phd_sin(item->pos.yRot);

	pos.x = item->pos.xPos + z * s + x * c;
	pos.y = item->pos.yPos - z * phd_sin(item->pos.xRot) + x * phd_sin(item->pos.zRot);
	pos.z = item->pos.zPos + z * c - x * s;

	*room_number = item->roomNumber;
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

	if (train->itemFlags[0] == 0)
		train->itemFlags[0] = train->itemFlags[1] = TRAIN_VEL;

	c = phd_cos(train->pos.yRot);
	s = phd_sin(train->pos.yRot);

	train->pos.xPos += train->itemFlags[1] * s;
	train->pos.zPos += train->itemFlags[1] * c;

	rh = TrainTestHeight(train, 0, 5120, &roomNum);
	train->pos.yPos = fh = TrainTestHeight(train, 0, 0, &roomNum);

	if (fh == NO_HEIGHT)
	{
		KillItem(trainNum);
		return;
	}

	train->pos.yPos -= 32;// ?

	roomNum = train->roomNumber;
	GetFloor(train->pos.xPos, train->pos.yPos, train->pos.zPos, &roomNum);

	if (roomNum != train->roomNumber)
		ItemNewRoom(trainNum, roomNum);

	train->pos.xRot = -(rh - fh) * 2;

	TriggerDynamicLight(train->pos.xPos + 3072 * s, train->pos.yPos, train->pos.zPos + 3072 * c, 16, 31, 31, 31);

	if (train->itemFlags[1] != TRAIN_VEL)
	{
		if ((train->itemFlags[1] -= 48) < 0)
			train->itemFlags[1] = 0;

		if (!UseForcedFixedCamera)
		{
			ForcedFixedCamera.x = train->pos.xPos + 8192 * s;
			ForcedFixedCamera.z = train->pos.zPos + 8192 * c;

			roomNum = train->roomNumber;
			floor = GetFloor(ForcedFixedCamera.x, train->pos.yPos - 512, ForcedFixedCamera.z, &roomNum);
			ForcedFixedCamera.y = GetFloorHeight(floor, ForcedFixedCamera.x, train->pos.yPos - 512, ForcedFixedCamera.z);

			ForcedFixedCamera.roomNumber = roomNum;

			UseForcedFixedCamera = 1;
		}
	}
	else
		SoundEffect(SFX_TR3_TUBE_LOOP, &train->pos, SFX_ALWAYS);
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

	SoundEffect(SFX_TR3_LARA_GENERAL_DEATH, &larA->pos, SFX_ALWAYS);
	SoundEffect(SFX_TR3_LARA_FALLDETH, &larA->pos, SFX_ALWAYS);
	StopSoundEffect(SFX_TR3_TUBE_LOOP);

	larA->animNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex + LARA_TRAIN_DEATH_ANIM;
	larA->frameNumber = g_Level.Anims[larA->animNumber].frameBase;
//	larA->currentAnimState = EXTRA_TRAINKILL;
//	larA->goalAnimState = EXTRA_TRAINKILL;
	larA->hitPoints = 0;

	larA->pos.yRot = train->pos.yRot;

	larA->fallspeed = 0;
	larA->gravityStatus = false;
	larA->speed = 0;

	AnimateItem(larA);

	Lara.ExtraAnim = 1;
	Lara.gunStatus = LG_HANDS_BUSY;
	Lara.gunType = WEAPON_NONE;
	Lara.hitDirection = -1;
	Lara.air = -1;

	train->itemFlags[1] = 160;

	c = phd_cos(train->pos.yRot);
	s = phd_sin(train->pos.yRot);

	x = larA->pos.xPos + 256 * s;
	z = larA->pos.zPos + 256 * c;

	DoLotsOfBlood(x, larA->pos.yPos - 512, z, 1024, train->pos.yRot, larA->roomNumber, 15);

}
