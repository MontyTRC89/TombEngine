#include "framework.h"
#include "Objects/Generic/Object/Pushables/PushableBridge.h"

#include "Game/collision/floordata.h"
#include "Objects/Generic/Object/Pushables/PushableObject.h"
#include "Specific/level.h"

using namespace TEN::Collision::Floordata;

namespace TEN::Entities::Generic
{
	std::optional<int> PushableBridgeFloor(int itemNumber, int x, int y, int z)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		const auto& pushable = GetPushableInfo(pushableItem);

		auto boxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, false);

		if (pushableItem.Active &&pushableItem.Status != ITEM_INVISIBLE &&
			pushable.UsesRoomCollision &&
			boxHeight.has_value())
		{
			int height = pushableItem.Pose.Position.y - GetPushableHeight(pushableItem);
			return height;
		}

		return std::nullopt;
	}

	std::optional<int> PushableBridgeCeiling(int itemNumber, int x, int y, int z)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		const auto& pushable = GetPushableInfo(pushableItem);

		auto boxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, true);

		if (pushableItem.Active && pushableItem.Status != ITEM_INVISIBLE &&
			pushable.UsesRoomCollision &&
			boxHeight.has_value())
		{
			return pushableItem.Pose.Position.y;
		}

		return std::nullopt;
	}

	int PushableBridgeFloorBorder(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];

		return (pushableItem.Pose.Position.y - GetPushableHeight(pushableItem));
	}

	int PushableBridgeCeilingBorder(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];

		return pushableItem.Pose.Position.y;
	}

	void AddPushableBridge(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		if (pushable.UsesRoomCollision)
		{
			AddBridge(itemNumber);
			pushable.BridgeColliderFlag = true;
		}
	}

	void RemovePushableBridge(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		if (pushable.UsesRoomCollision)
		{
			RemoveBridge(itemNumber);
			pushable.BridgeColliderFlag = false;
		}
	}

	void UpdatePushableBridge(int itemNumber)
	{
		const auto& pushableItem = g_Level.Items[itemNumber];
		const auto& pushable = GetPushableInfo(pushableItem);

		if (pushable.UsesRoomCollision)
			UpdateBridgeItem(itemNumber);
	}

	void AddPushableStackBridge(int itemNumber, bool addBridge)
	{
		auto* pushableItemPtr = &g_Level.Items[itemNumber];
		const auto* pushablePtr = &GetPushableInfo(*pushableItemPtr);

		// Can't have stacked items or bridge.
		if (!pushablePtr->UsesRoomCollision)
			return;

		if (pushablePtr->BridgeColliderFlag)
			addBridge ? AddBridge(itemNumber) : RemoveBridge(itemNumber);
		
		while (pushablePtr->StackUpperItem != NO_ITEM)
		{
			pushableItemPtr = &g_Level.Items[pushablePtr->StackUpperItem];
			pushablePtr = &GetPushableInfo(*pushableItemPtr);

			if (pushablePtr->BridgeColliderFlag)
				addBridge ? AddBridge(pushableItemPtr->Index) : RemoveBridge(pushableItemPtr->Index);
		}
	}
}
