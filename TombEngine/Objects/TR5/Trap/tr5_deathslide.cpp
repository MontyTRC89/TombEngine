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

using namespace TEN::Input;

OBJECT_COLLISION_BOUNDS DeathSlideBounds =
{
	-256, 256,
	-100, 100,
	256, 512,
	0, 0,
	-ANGLE(25.0f), ANGLE(25.0f),
	0, 0
};

Vector3Int DeathSlidePosition(0, 0, 371);

void InitialiseDeathSlide(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	item->Data = GameVector();
	auto* pos = (GameVector*)item->Data;

	pos->x = item->Pose.Position.x;
	pos->y = item->Pose.Position.y;
	pos->z = item->Pose.Position.z;
	pos->roomNumber = item->RoomNumber;
}

void DeathSlideCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	if (!(TrInput & IN_ACTION) ||
		laraItem->Animation.ActiveState != LS_IDLE ||
		laraItem->Animation.IsAirborne ||
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
		zipLineItem->Flags |= IFLAG_INVISIBLE;
	}
}

void ControlDeathSlide(short itemNumber)
{
	auto* zipLineItem = &g_Level.Items[itemNumber];

	if (zipLineItem->Status == ITEM_ACTIVE)
	{
		if (!(zipLineItem->Flags & IFLAG_INVISIBLE))
		{
			auto* old = (GameVector*)zipLineItem->Data;

			zipLineItem->Pose.Position.x = old->x;
			zipLineItem->Pose.Position.y = old->y;
			zipLineItem->Pose.Position.z = old->z;

			if (old->roomNumber != zipLineItem->RoomNumber)
				ItemNewRoom(itemNumber, old->roomNumber);

			zipLineItem->Status = ITEM_NOT_ACTIVE;
			zipLineItem->Animation.ActiveState = zipLineItem->Animation.TargetState = 1;
			zipLineItem->Animation.AnimNumber = Objects[zipLineItem->ObjectNumber].animIndex;
			zipLineItem->Animation.FrameNumber = g_Level.Anims[zipLineItem->Animation.AnimNumber].frameBase;

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

		float c = phd_cos(zipLineItem->Pose.Orientation.y);
		float s = phd_sin(zipLineItem->Pose.Orientation.y);

		zipLineItem->Pose.Position.z += zipLineItem->Animation.VerticalVelocity * c;
		zipLineItem->Pose.Position.x += zipLineItem->Animation.VerticalVelocity * s;
		zipLineItem->Pose.Position.y += zipLineItem->Animation.VerticalVelocity / 4;

		short roomNumber = zipLineItem->RoomNumber;
		GetFloor(zipLineItem->Pose.Position.x, zipLineItem->Pose.Position.y, zipLineItem->Pose.Position.z, &roomNumber);
		if (roomNumber != zipLineItem->RoomNumber)
			ItemNewRoom(itemNumber, roomNumber);

		if (LaraItem->Animation.ActiveState == LS_ZIP_LINE)
		{
			LaraItem->Pose.Position.x = zipLineItem->Pose.Position.x;
			LaraItem->Pose.Position.y = zipLineItem->Pose.Position.y;
			LaraItem->Pose.Position.z = zipLineItem->Pose.Position.z;
		}

		int x = zipLineItem->Pose.Position.x + 1024 * s;
		int y = zipLineItem->Pose.Position.y + 64;
		int z = zipLineItem->Pose.Position.z + 1024 * c;

		FloorInfo* floor = GetFloor(x, y, z, &roomNumber);

		if (GetFloorHeight(floor, x, y, z) <= y + 256 || GetCeiling(floor, x, y, z) >= y - 256)
		{
			if (LaraItem->Animation.ActiveState == LS_ZIP_LINE)
			{
				LaraItem->Animation.TargetState = LS_JUMP_FORWARD;
				AnimateLara(LaraItem);
				LaraItem->Animation.IsAirborne = true;
				LaraItem->Animation.Velocity = zipLineItem->Animation.VerticalVelocity;
				LaraItem->Animation.VerticalVelocity = zipLineItem->Animation.VerticalVelocity / 4;
			}

			// Stop
			SoundEffect(SFX_TR4_VONCROY_KNIFE_SWISH, &zipLineItem->Pose);
			RemoveActiveItem(itemNumber);
			zipLineItem->Status = ITEM_NOT_ACTIVE;
			zipLineItem->Flags -= IFLAG_INVISIBLE;
		}
		else
		{
			// Whizz
			SoundEffect(SFX_TR4_TRAIN_DOOR_CLOSE, &zipLineItem->Pose);
		}
	}
}
