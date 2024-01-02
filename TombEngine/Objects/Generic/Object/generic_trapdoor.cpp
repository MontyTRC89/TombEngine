#include "framework.h"
#include "Objects/Generic/Object/generic_trapdoor.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Math/Math.h"
#include "Objects/Generic/Object/BridgeObject.h"
#include "Renderer/Renderer11.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Input;
using namespace TEN::Math;
using namespace TEN::Renderer;

namespace TEN::Entities::Generic
{
	const ObjectCollisionBounds CeilingTrapDoorBounds =
	{
		GameBoundingBox(
			-CLICK(1), CLICK(1),
			0, 900,
			-BLOCK(0.75f), -CLICK(1)),
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f)))
	};
	const auto CeilingTrapDoorPos = Vector3i(0, 1056, -480);

	const ObjectCollisionBounds FloorTrapDoorBounds =
	{
		GameBoundingBox(
			-CLICK(1), CLICK(1),
			0, 0,
			-BLOCK(1), -CLICK(1)),
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f)))
	};
	static auto FloorTrapDoorPos = Vector3i(0, 0, -655);

	static std::optional<int> GetTrapDoorFloorHeight(const ItemInfo& item, const Vector3i& pos)
	{
		if (!item.MeshBits.TestAny() || item.ItemFlags[2] == 0)
			return std::nullopt;

		return GetBridgeItemIntersect(item, pos, false);
	}

	static std::optional<int> GetTrapDoorCeilingHeight(const ItemInfo& item, const Vector3i& pos)
	{
		if (!item.MeshBits.TestAny() || item.ItemFlags[2] == 0)
			return std::nullopt;

		return GetBridgeItemIntersect(item, pos, true);
	}

	static int GetTrapDoorFloorBorder(const ItemInfo& item)
	{
		return GetBridgeBorder(item, false);
	}

	static int GetTrapDoorCeilingBorder(const ItemInfo& item)
	{
		return GetBridgeBorder(item, true);
	}

	void InitializeTrapDoor(short itemNumber)
	{
		auto& trapDoorItem = g_Level.Items[itemNumber];
		trapDoorItem.Data = BridgeObject();
		auto& bridge = GetBridgeObject(trapDoorItem);

		// Initialize routines.
		bridge.GetFloorHeight = GetTrapDoorFloorHeight;
		bridge.GetCeilingHeight = GetTrapDoorCeilingHeight;
		bridge.GetFloorBorder = GetTrapDoorFloorBorder;
		bridge.GetCeilingBorder = GetTrapDoorCeilingBorder;

		UpdateBridgeItem(trapDoorItem);
		CloseTrapDoor(itemNumber);
	}

	void TrapDoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* trapDoorItem = &g_Level.Items[itemNumber];

		if (trapDoorItem->Animation.ActiveState == 1 &&
			trapDoorItem->Animation.FrameNumber == GetAnimData(trapDoorItem).frameEnd)
		{
			ObjectCollision(itemNumber, laraItem, coll);
		}
	}

	void CeilingTrapDoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* trapDoorItem = &g_Level.Items[itemNumber];

		bool itemIsAbove = trapDoorItem->Pose.Position.y <= laraItem->Pose.Position.y - LARA_HEIGHT + LARA_HEADROOM;

		bool result = TestLaraPosition(CeilingTrapDoorBounds, trapDoorItem, laraItem);
		laraItem->Pose.Orientation.y += ANGLE(180.0f);
		bool result2 = TestLaraPosition(CeilingTrapDoorBounds, trapDoorItem, laraItem);
		laraItem->Pose.Orientation.y += ANGLE(180.0f);

		if (IsHeld(In::Action) &&
			laraItem->Animation.ActiveState == LS_JUMP_UP &&
			laraItem->Animation.IsAirborne &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			trapDoorItem->Status != ITEM_ACTIVE &&
			itemIsAbove &&
			(result || result2))
		{
			AlignLaraPosition(CeilingTrapDoorPos, trapDoorItem, laraItem);
			if (result2)
				laraItem->Pose.Orientation.y += ANGLE(180.0f);

			ResetPlayerFlex(laraItem);
			laraItem->Animation.Velocity.y = 0;
			laraItem->Animation.IsAirborne = false;
			laraItem->Animation.AnimNumber = LA_TRAPDOOR_CEILING_OPEN;
			laraItem->Animation.FrameNumber = GetAnimData(laraItem).frameBase;
			laraItem->Animation.ActiveState = LS_FREEFALL_BIS;
			laraInfo->Control.HandStatus = HandStatus::Busy;
			AddActiveItem(itemNumber);
			trapDoorItem->Status = ITEM_ACTIVE;
			trapDoorItem->Animation.TargetState = 1;

			UseForcedFixedCamera = 1;
			ForcedFixedCamera.x = trapDoorItem->Pose.Position.x - phd_sin(trapDoorItem->Pose.Orientation.y) * 1024;
			ForcedFixedCamera.y = trapDoorItem->Pose.Position.y + 1024;
			ForcedFixedCamera.z = trapDoorItem->Pose.Position.z - phd_cos(trapDoorItem->Pose.Orientation.y) * 1024;
			ForcedFixedCamera.RoomNumber = trapDoorItem->RoomNumber;
		}
		else
		{
			if (trapDoorItem->Animation.ActiveState == 1)
				UseForcedFixedCamera = 0;
		}

		if (trapDoorItem->Animation.ActiveState == 1 &&
			trapDoorItem->Animation.FrameNumber == GetAnimData(trapDoorItem).frameEnd)
		{
			ObjectCollision(itemNumber, laraItem, coll);
		}
	}

	void FloorTrapDoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* trapDoorItem = &g_Level.Items[itemNumber];

		if ((IsHeld(In::Action) &&
			laraItem->Animation.ActiveState == LS_IDLE &&
			laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			trapDoorItem->Status != ITEM_ACTIVE) ||
			(laraInfo->Control.IsMoving && laraInfo->Context.InteractedItem == itemNumber))
		{
			if (TestLaraPosition(FloorTrapDoorBounds, trapDoorItem, laraItem))
			{
				if (MoveLaraPosition(FloorTrapDoorPos, trapDoorItem, laraItem))
				{
					ResetPlayerFlex(laraItem);
					laraItem->Animation.AnimNumber = LA_TRAPDOOR_FLOOR_OPEN;
					laraItem->Animation.FrameNumber = GetAnimData(laraItem).frameBase;
					laraItem->Animation.ActiveState = LS_TRAPDOOR_FLOOR_OPEN;
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Busy;
					AddActiveItem(itemNumber);
					trapDoorItem->Status = ITEM_ACTIVE;
					trapDoorItem->Animation.TargetState = 1;

					UseForcedFixedCamera = 1;
					ForcedFixedCamera.x = trapDoorItem->Pose.Position.x - phd_sin(trapDoorItem->Pose.Orientation.y) * 2048;
					ForcedFixedCamera.y = trapDoorItem->Pose.Position.y - 2048;

					if (ForcedFixedCamera.y < g_Level.Rooms[trapDoorItem->RoomNumber].maxceiling)
						ForcedFixedCamera.y = g_Level.Rooms[trapDoorItem->RoomNumber].maxceiling;

					ForcedFixedCamera.z = trapDoorItem->Pose.Position.z - phd_cos(trapDoorItem->Pose.Orientation.y) * 2048;
					ForcedFixedCamera.RoomNumber = trapDoorItem->RoomNumber;
				}
				else
				{
					laraInfo->Context.InteractedItem = itemNumber;
				}
			}
		}
		else
		{
			if (trapDoorItem->Animation.ActiveState == 1)
				UseForcedFixedCamera = 0;
		}

		if (trapDoorItem->Animation.ActiveState == 1 && trapDoorItem->Animation.FrameNumber == GetAnimData(trapDoorItem).frameEnd)
			ObjectCollision(itemNumber, laraItem, coll);
	}

	void TrapDoorControl(short itemNumber)
	{
		auto* trapDoorItem = &g_Level.Items[itemNumber];

	if (TriggerActive(trapDoorItem))
	{
		if (!trapDoorItem->Animation.ActiveState && trapDoorItem->TriggerFlags >= 0)
			trapDoorItem->Animation.TargetState = 1;
	}
	else
	{
		trapDoorItem->Status = ITEM_ACTIVE;

			if (trapDoorItem->Animation.ActiveState == 1)
				trapDoorItem->Animation.TargetState = 0;
		}

		AnimateItem(trapDoorItem);

		if (trapDoorItem->Animation.ActiveState == 1 && (trapDoorItem->ItemFlags[2] || JustLoaded))
		{
			OpenTrapDoor(itemNumber);
		}
		else if (!trapDoorItem->Animation.ActiveState && !trapDoorItem->ItemFlags[2])
		{
			CloseTrapDoor(itemNumber);
		}
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
}
