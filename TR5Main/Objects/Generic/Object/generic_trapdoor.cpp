#include "framework.h"
#include "Objects/Generic/Object/generic_trapdoor.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/collision/floordata.h"
#include "Specific/input.h"
#include "Game/camera.h"
#include "Game/control/control.h"
#include "Specific/level.h"
#include "Game/animation.h"
#include "Game/items.h"
#include "Renderer/Renderer11.h"
#include "Game/collision/collide_item.h"

using namespace TEN::Renderer;

using namespace TEN::Floordata;

OBJECT_COLLISION_BOUNDS CeilingTrapDoorBounds =
{
	-256, 256,
	0, 900,
	-768, -256,
	-1820, 1820,
	-5460, 5460,
	-1820, 1820
};
static PHD_VECTOR CeilingTrapDoorPos = { 0, 1056, -480 };

OBJECT_COLLISION_BOUNDS FloorTrapDoorBounds =
{
	-256, 256,
	0, 0,
	-1024, -256,
	-1820, 1820,
	-5460, 5460,
	-1820, 1820
};
static PHD_VECTOR FloorTrapDoorPos = { 0, 0, -655 };

void InitialiseTrapDoor(short itemNumber)
{
	auto* trapDoorItem = &g_Level.Items[itemNumber];
	TEN::Floordata::UpdateBridgeItem(itemNumber);
	CloseTrapDoor(itemNumber);
}

void TrapDoorCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	auto* trapDoorItem = &g_Level.Items[itemNumber];

	if (trapDoorItem->Animation.ActiveState == 1 &&
		trapDoorItem->Animation.FrameNumber == g_Level.Anims[trapDoorItem->Animation.AnimNumber].frameEnd)
	{
		ObjectCollision(itemNumber, laraItem, coll);
	}
}

void CeilingTrapDoorCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* trapDoorItem = &g_Level.Items[itemNumber];

	bool itemIsAbove = trapDoorItem->Position.yPos <= laraItem->Position.yPos - LARA_HEIGHT + LARA_HEADROOM;

	bool result = TestLaraPosition(&CeilingTrapDoorBounds, trapDoorItem, laraItem);
	laraItem->Position.yRot += ANGLE(180.0f);
	bool result2 = TestLaraPosition(&CeilingTrapDoorBounds, trapDoorItem, laraItem);
	laraItem->Position.yRot += ANGLE(180.0f);

	if (TrInput & IN_ACTION &&
		trapDoorItem->Status != ITEM_ACTIVE &&
		laraItem->Animation.ActiveState == LS_JUMP_UP &&
		laraItem->Animation.Airborne &&
		laraInfo->Control.HandStatus == HandStatus::Free &&
		itemIsAbove &&
		(result || result2))
	{
		AlignLaraPosition(&CeilingTrapDoorPos, trapDoorItem, laraItem);
		if (result2)
			laraItem->Position.yRot += ANGLE(180.0f);
		
		ResetLaraFlex(laraItem);
		laraItem->Animation.VerticalVelocity = 0;
		laraItem->Animation.Airborne = false;
		laraItem->Animation.AnimNumber = LA_TRAPDOOR_CEILING_OPEN;
		laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
		laraItem->Animation.ActiveState = LS_FREEFALL_BIS;
		laraInfo->Control.HandStatus = HandStatus::Busy;
		AddActiveItem(itemNumber);
		trapDoorItem->Status = ITEM_ACTIVE;
		trapDoorItem->Animation.TargetState = 1;

		UseForcedFixedCamera = 1;
		ForcedFixedCamera.x = trapDoorItem->Position.xPos - phd_sin(trapDoorItem->Position.yRot) * 1024;
		ForcedFixedCamera.y = trapDoorItem->Position.yPos + 1024;
		ForcedFixedCamera.z = trapDoorItem->Position.zPos - phd_cos(trapDoorItem->Position.yRot) * 1024;
		ForcedFixedCamera.roomNumber = trapDoorItem->RoomNumber;
	}
	else
	{
		if (trapDoorItem->Animation.ActiveState == 1)
			UseForcedFixedCamera = 0;
	}

	if (trapDoorItem->Animation.ActiveState == 1 &&
		trapDoorItem->Animation.FrameNumber == g_Level.Anims[trapDoorItem->Animation.AnimNumber].frameEnd)
	{
		ObjectCollision(itemNumber, laraItem, coll);
	}
}

void FloorTrapDoorCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* trapDoorItem = &g_Level.Items[itemNumber];

	if (TrInput & IN_ACTION &&
		laraItem->Animation.ActiveState == LS_IDLE &&
		laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
		laraInfo->Control.HandStatus == HandStatus::Free &&
		trapDoorItem->Status != ITEM_ACTIVE ||
		laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
	{
		if (TestLaraPosition(&FloorTrapDoorBounds, trapDoorItem, laraItem))
		{
			if (MoveLaraPosition(&FloorTrapDoorPos, trapDoorItem, laraItem))
			{
				ResetLaraFlex(laraItem);
				laraItem->Animation.AnimNumber = LA_TRAPDOOR_FLOOR_OPEN;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.ActiveState = LS_TRAPDOOR_FLOOR_OPEN;
				laraInfo->Control.IsMoving = false;
				laraInfo->Control.HandStatus = HandStatus::Busy;
				AddActiveItem(itemNumber);
				trapDoorItem->Status = ITEM_ACTIVE;
				trapDoorItem->Animation.TargetState = 1;

				UseForcedFixedCamera = 1;
				ForcedFixedCamera.x = trapDoorItem->Position.xPos - phd_sin(trapDoorItem->Position.yRot) * 2048;
				ForcedFixedCamera.y = trapDoorItem->Position.yPos - 2048;

				if (ForcedFixedCamera.y < g_Level.Rooms[trapDoorItem->RoomNumber].maxceiling)
					ForcedFixedCamera.y = g_Level.Rooms[trapDoorItem->RoomNumber].maxceiling;

				ForcedFixedCamera.z = trapDoorItem->Position.zPos - phd_cos(trapDoorItem->Position.yRot) * 2048;
				ForcedFixedCamera.roomNumber = trapDoorItem->RoomNumber;
			}
			else
				laraInfo->InteractedItem =itemNumber;
		}
	}
	else
	{
		if (trapDoorItem->Animation.ActiveState == 1)
			UseForcedFixedCamera = 0;
	}

	if (trapDoorItem->Animation.ActiveState == 1 && trapDoorItem->Animation.FrameNumber == g_Level.Anims[trapDoorItem->Animation.AnimNumber].frameEnd)
		ObjectCollision(itemNumber, laraItem, coll);
}

void TrapDoorControl(short itemNumber)
{
	auto* trapDoorItem = &g_Level.Items[itemNumber];

	if (TriggerActive(trapDoorItem))
	{
		if (!trapDoorItem->Animation.ActiveState && trapDoorItem->TriggerFlags >= 0)
			trapDoorItem->Animation.TargetState = 1;
		else if (trapDoorItem->Animation.FrameNumber == g_Level.Anims[trapDoorItem->Animation.AnimNumber].frameEnd && CurrentLevel == 14 && trapDoorItem->ObjectNumber == ID_TRAPDOOR1)
			trapDoorItem->Status = ITEM_INVISIBLE;
	}
	else
	{
		trapDoorItem->Status = ITEM_ACTIVE;

		if (trapDoorItem->Animation.ActiveState == 1)
			trapDoorItem->Animation.TargetState = 0;
	}

	AnimateItem(trapDoorItem);

	if (trapDoorItem->Animation.ActiveState == 1 && (trapDoorItem->ItemFlags[2] || JustLoaded))
		OpenTrapDoor(itemNumber);
	else if (!trapDoorItem->Animation.ActiveState && !trapDoorItem->ItemFlags[2])
		CloseTrapDoor(itemNumber);
}

void CloseTrapDoor(short itemNumber)
{
	auto* trapDoorItem = &g_Level.Items[itemNumber];
	trapDoorItem->ItemFlags[2] = 1;
}

void OpenTrapDoor(short itemNumber)
{
	auto* trapDoorItem = &g_Level.Items[itemNumber];
	trapDoorItem->ItemFlags[2] = 0;
}

int TrapDoorFloorBorder(short itemNumber)
{
	return GetBridgeBorder(itemNumber, false);
}

int TrapDoorCeilingBorder(short itemNumber)
{
	return GetBridgeBorder(itemNumber, true);
}

std::optional<int> TrapDoorFloor(short itemNumber, int x, int y, int z)
{
	ITEM_INFO* trapDoorItem = &g_Level.Items[itemNumber];
	if (!trapDoorItem->MeshBits || trapDoorItem->ItemFlags[2] == 0)
		return std::nullopt;

	return GetBridgeItemIntersect(itemNumber, x, y, z, false);
}

std::optional<int> TrapDoorCeiling(short itemNumber, int x, int y, int z)
{
	ITEM_INFO* trapDoorItem = &g_Level.Items[itemNumber];

	if (!trapDoorItem->MeshBits || trapDoorItem->ItemFlags[2] == 0)
		return std::nullopt;

	return GetBridgeItemIntersect(itemNumber, x, y, z, true);
}
