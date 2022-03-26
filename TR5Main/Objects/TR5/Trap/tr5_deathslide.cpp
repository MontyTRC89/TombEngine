#include "framework.h"
#include "tr5_deathslide.h"
#include "Specific/input.h"
#include "Specific/trmath.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Specific/setup.h"
#include "Sound/sound.h"
#include "Game/control/box.h"
#include "Game/animation.h"
#include "Game/items.h"
#include "Game/collision/collide_item.h"

OBJECT_COLLISION_BOUNDS DeathSlideBounds =
{
	-256, 256,
	-100, 100,
	256, 512,
	0, 0,
	-ANGLE(25.0f), ANGLE(25.0f),
	0, 0
};

PHD_VECTOR DeathSlidePosition(0, 0, 371);

void InitialiseDeathSlide(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	item->Data = GAME_VECTOR();
	auto* pos = (GAME_VECTOR*)item->Data;

	pos->x = item->Position.xPos;
	pos->y = item->Position.yPos;
	pos->z = item->Position.zPos;
	pos->roomNumber = item->RoomNumber;
}

void DeathSlideCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	if (!(TrInput & IN_ACTION) ||
		laraItem->Animation.ActiveState != LS_IDLE ||
		laraItem->Animation.Airborne ||
		laraInfo->Control.HandStatus != HandStatus::Free)
	{
		return;
	}

	auto* zipLineItem = &g_Level.Items[itemNumber];
	if (zipLineItem->Status != ITEM_NOT_ACTIVE)
		return;

	if (TestLaraPosition(&DeathSlideBounds, zipLineItem, laraItem))
	{
		AlignLaraPosition(&DeathSlidePosition, zipLineItem, laraItem);
		laraInfo->Control.HandStatus = HandStatus::Busy;

		laraItem->Animation.TargetState = LS_ZIP_LINE;
		do
			AnimateItem(laraItem);
		while (laraItem->Animation.ActiveState != LS_GRABBING);

		if (!zipLineItem->Active)
			AddActiveItem(itemNumber);

		zipLineItem->Status = ITEM_ACTIVE;
		zipLineItem->Flags |= ONESHOT;
	}
}

void ControlDeathSlide(short itemNumber)
{
	auto* zipLineItem = &g_Level.Items[itemNumber];

	if (zipLineItem->Status == ITEM_ACTIVE)
	{
		if (!(zipLineItem->Flags & ONESHOT))
		{
			auto* old = (GAME_VECTOR*)zipLineItem->Data;

			zipLineItem->Position.xPos = old->x;
			zipLineItem->Position.yPos = old->y;
			zipLineItem->Position.zPos = old->z;

			if (old->roomNumber != zipLineItem->RoomNumber)
				ItemNewRoom(itemNumber, old->roomNumber);

			zipLineItem->Status = ITEM_NOT_ACTIVE;
			zipLineItem->Animation.ActiveState = zipLineItem->Animation.TargetState = 1;
			zipLineItem->Animation.AnimNumber = Objects[zipLineItem->ObjectNumber].animIndex;
			zipLineItem->Animation.FrameNumber = g_Level.Anims[zipLineItem->Animation.AnimNumber].FrameBase;

			RemoveActiveItem(itemNumber);
			return;
		}

		if (zipLineItem->Animation.ActiveState == 1)
		{
			AnimateItem(zipLineItem);
			return;
		}

		AnimateItem(zipLineItem);

		if (zipLineItem->Animation.VerticalVelocity < 100)
			zipLineItem->Animation.VerticalVelocity += 5;

		float c = phd_cos(zipLineItem->Position.yRot);
		float s = phd_sin(zipLineItem->Position.yRot);

		zipLineItem->Position.zPos += zipLineItem->Animation.VerticalVelocity * c;
		zipLineItem->Position.xPos += zipLineItem->Animation.VerticalVelocity * s;
		zipLineItem->Position.yPos += zipLineItem->Animation.VerticalVelocity / 4;

		short roomNumber = zipLineItem->RoomNumber;
		GetFloor(zipLineItem->Position.xPos, zipLineItem->Position.yPos, zipLineItem->Position.zPos, &roomNumber);
		if (roomNumber != zipLineItem->RoomNumber)
			ItemNewRoom(itemNumber, roomNumber);

		if (LaraItem->Animation.ActiveState == LS_ZIP_LINE)
		{
			LaraItem->Position.xPos = zipLineItem->Position.xPos;
			LaraItem->Position.yPos = zipLineItem->Position.yPos;
			LaraItem->Position.zPos = zipLineItem->Position.zPos;
		}

		int x = zipLineItem->Position.xPos + 1024 * s;
		int y = zipLineItem->Position.yPos + 64;
		int z = zipLineItem->Position.zPos + 1024 * c;

		FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);

		if (GetFloorHeight(floor, x, y, z) <= y + 256 || GetCeiling(floor, x, y, z) >= y - 256)
		{
			if (LaraItem->Animation.ActiveState == LS_ZIP_LINE)
			{
				LaraItem->Animation.TargetState = LS_JUMP_FORWARD;
				AnimateLara(LaraItem);
				LaraItem->Animation.Airborne = true;
				LaraItem->Animation.Velocity = zipLineItem->Animation.VerticalVelocity;
				LaraItem->Animation.VerticalVelocity = zipLineItem->Animation.VerticalVelocity / 4;
			}

			// Stop
			SoundEffect(SFX_TR4_VONCROY_KNIFE_SWISH, &zipLineItem->Position, 0);
			RemoveActiveItem(itemNumber);
			zipLineItem->Status = ITEM_NOT_ACTIVE;
			zipLineItem->Flags -= ONESHOT;
		}
		else
		{
			// Whizz
			SoundEffect(SFX_TR4_TRAIN_DOOR_CLOSE, &zipLineItem->Position, 0);
		}
	}
}
