#include "framework.h"
#include "tr5_deathslide.h"
#include "Specific/input.h"
#include "Specific/trmath.h"
#include "Game/Lara/lara.h"
#include "Specific/setup.h"
#include "Sound/sound.h"
#include "Game/control/box.h"
#include "Game/animation.h"
#include "Game/items.h"
#include "Game/collision/collide_item.h"

OBJECT_COLLISION_BOUNDS DeathSlideBounds = { -256, 256, -100, 100, 256, 512, 0, 0, -ANGLE(25.0f), ANGLE(25.0f), 0, 0 };
PHD_VECTOR DeathSlidePosition(0, 0, 371);

void InitialiseDeathSlide(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	item->Data = GAME_VECTOR();
	GAME_VECTOR* pos = item->Data;
	pos->x = item->Position.xPos;
	pos->y = item->Position.yPos;
	pos->z = item->Position.zPos;
	pos->roomNumber = item->RoomNumber;
}

void DeathSlideCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	if (!(TrInput & IN_ACTION) || l->Airborne || Lara.Control.HandStatus != HandStatus::Free || l->ActiveState != LS_IDLE)
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	if (item->Status != ITEM_NOT_ACTIVE)
		return;

	if (TestLaraPosition(&DeathSlideBounds, item, LaraItem))
	{
		AlignLaraPosition(&DeathSlidePosition, item, LaraItem);
		Lara.Control.HandStatus = HandStatus::Busy;

		l->TargetState = LS_ZIP_LINE;
		do
			AnimateItem(l);
		while (l->ActiveState != LS_GRABBING);

		if (!item->Active)
			AddActiveItem(itemNumber);

		item->Status = ITEM_ACTIVE;
		item->Flags |= ONESHOT;
	}
}

void ControlDeathSlide(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->Status == ITEM_ACTIVE)
	{
		if (!(item->Flags & ONESHOT))
		{
			GAME_VECTOR* old = (GAME_VECTOR*)item->Data;

			item->Position.xPos = old->x;
			item->Position.yPos = old->y;
			item->Position.zPos = old->z;

			if (old->roomNumber != item->RoomNumber)
				ItemNewRoom(itemNumber, old->roomNumber);

			item->Status = ITEM_NOT_ACTIVE;
			item->ActiveState = item->TargetState = 1;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;

			RemoveActiveItem(itemNumber);

			return;
		}

		if (item->ActiveState == 1)
		{
			AnimateItem(item);
			return;
		}

		AnimateItem(item);

		if (item->VerticalVelocity < 100)
			item->VerticalVelocity += 5;

		float c = phd_cos(item->Position.yRot);
		float s = phd_sin(item->Position.yRot);

		item->Position.zPos += item->VerticalVelocity * c;
		item->Position.xPos += item->VerticalVelocity * s;
		item->Position.yPos += item->VerticalVelocity / 4;

		short roomNumber = item->RoomNumber;
		GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
		if (roomNumber != item->RoomNumber)
			ItemNewRoom(itemNumber, roomNumber);

		if (LaraItem->ActiveState == LS_ZIP_LINE)
		{
			LaraItem->Position.xPos = item->Position.xPos;
			LaraItem->Position.yPos = item->Position.yPos;
			LaraItem->Position.zPos = item->Position.zPos;
		}

		int x = item->Position.xPos + 1024 * s;
		int y = item->Position.yPos + 64;
		int z = item->Position.zPos + 1024 * c;

		FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);

		if (GetFloorHeight(floor, x, y, z) <= y + 256 || GetCeiling(floor, x, y, z) >= y - 256)
		{
			if (LaraItem->ActiveState == LS_ZIP_LINE)
			{
				LaraItem->TargetState = LS_JUMP_FORWARD;
				AnimateLara(LaraItem);
				LaraItem->Airborne = true;
				LaraItem->Velocity = item->VerticalVelocity;
				LaraItem->VerticalVelocity = item->VerticalVelocity / 4;
			}

			// Stop
			SoundEffect(SFX_TR4_VONCROY_KNIFE_SWISH, &item->Position, 0);
			RemoveActiveItem(itemNumber);
			item->Status = ITEM_NOT_ACTIVE;
			item->Flags -= ONESHOT;
		}
		else
		{
			// Whizz
			SoundEffect(SFX_TR4_TRAIN_DOOR_CLOSE, &item->Position, 0);
		}
	}
}