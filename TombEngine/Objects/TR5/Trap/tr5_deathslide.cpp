#include "framework.h"
#include "Objects/TR5/Trap/tr5_deathslide.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/setup.h"

using namespace TEN::Input;

OBJECT_COLLISION_BOUNDS DeathSlideBounds =
{
	GameBoundingBox(
		-CLICK(1), CLICK(1),
		-100, 100,
		CLICK(1), CLICK(2)
	),
	0, 0,
	-ANGLE(25.0f), ANGLE(25.0f),
	0, 0
};

Vector3i DeathSlidePosition(0, 0, 371);

void InitialiseDeathSlide(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	item->Data = GameVector();
	auto* pos = (GameVector*)item->Data;

	*pos = GameVector(
		item->Pose.Position.x,
		item->Pose.Position.y,
		item->Pose.Position.z,
		item->RoomNumber
	);
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

	if (TestLaraPosition(DeathSlideBounds, zipLineItem, laraItem))
	{
		AlignLaraPosition(DeathSlidePosition, zipLineItem, laraItem);
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
	auto* laraItem = LaraItem;

	if (zipLineItem->Status == ITEM_ACTIVE)
	{
		if (!(zipLineItem->Flags & IFLAG_INVISIBLE))
		{
			auto* prevPos = (GameVector*)zipLineItem->Data;

			zipLineItem->Pose.Position = prevPos->ToVector3i();

			if (prevPos->RoomNumber != zipLineItem->RoomNumber)
				ItemNewRoom(itemNumber, prevPos->RoomNumber);

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

		if (zipLineItem->Animation.Velocity.y < 100.0f)
			zipLineItem->Animation.Velocity.y += 5.0f;

		float sinY = phd_sin(zipLineItem->Pose.Orientation.y);
		float cosY = phd_cos(zipLineItem->Pose.Orientation.y);

		zipLineItem->Pose.Position.x += zipLineItem->Animation.Velocity.y * sinY;
		zipLineItem->Pose.Position.y += zipLineItem->Animation.Velocity.y / 4;
		zipLineItem->Pose.Position.z += zipLineItem->Animation.Velocity.y * cosY;

		auto pointColl = GetCollision(zipLineItem);
		if (pointColl.RoomNumber != zipLineItem->RoomNumber)
			ItemNewRoom(itemNumber, pointColl.RoomNumber);

		if (laraItem->Animation.ActiveState == LS_ZIP_LINE)
			laraItem->Pose.Position = zipLineItem->Pose.Position;

		int x = zipLineItem->Pose.Position.x + (SECTOR(1) * sinY);
		int y = zipLineItem->Pose.Position.y + CLICK(0.25f);
		int z = zipLineItem->Pose.Position.z + (SECTOR(1) * cosY);

		if (pointColl.Position.Floor <= (y + CLICK(1)) || pointColl.Position.Ceiling >= (y - CLICK(1)))
		{
			if (laraItem->Animation.ActiveState == LS_ZIP_LINE)
			{
				laraItem->Animation.TargetState = LS_JUMP_FORWARD;
				AnimateLara(laraItem);
				laraItem->Animation.IsAirborne = true;
				laraItem->Animation.Velocity.y = zipLineItem->Animation.Velocity.y / 4;
				laraItem->Animation.Velocity.z = zipLineItem->Animation.Velocity.y;
			}

			// Stop.
			SoundEffect(SFX_TR4_VONCROY_KNIFE_SWISH, &zipLineItem->Pose);
			RemoveActiveItem(itemNumber);
			zipLineItem->Status = ITEM_NOT_ACTIVE;
			zipLineItem->Flags -= IFLAG_INVISIBLE;
		}
		else
		{
			// Whizz sound.
			SoundEffect(SFX_TR4_TRAIN_DOOR_CLOSE, &zipLineItem->Pose);
		}
	}
}
