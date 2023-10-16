#include "framework.h"
#include "Objects/Generic/Object/Pushable/PushableBridge.h"

#include "Game/collision/floordata.h"
#include "Objects/Generic/Object/Pushable/PushableObject.h"
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
			pushable.UseRoomCollision &&
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
			pushable.UseRoomCollision &&
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

		if (pushable.UseRoomCollision)
		{
			AddBridge(itemNumber);
			pushable.UseBridgeCollision = true;
		}
	}

	void RemovePushableBridge(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		if (pushable.UseRoomCollision)
		{
			RemoveBridge(itemNumber);
			pushable.UseBridgeCollision = false;
		}
	}

	void UpdatePushableBridge(int itemNumber)
	{
		const auto& pushableItem = g_Level.Items[itemNumber];
		const auto& pushable = GetPushableInfo(pushableItem);

		if (pushable.UseRoomCollision)
			UpdateBridgeItem(itemNumber);
	}

	void AddPushableStackBridge(int itemNumber, bool addBridge)
	{
		auto* pushableItemPtr = &g_Level.Items[itemNumber];
		const auto* pushablePtr = &GetPushableInfo(*pushableItemPtr);

		// NOTE: Can't have stacked items on bridge.
		if (!pushablePtr->UseRoomCollision)
			return;

		if (pushablePtr->UseBridgeCollision)
			addBridge ? AddBridge(itemNumber) : RemoveBridge(itemNumber);
		
		while (pushablePtr->Stack.ItemNumberAbove != NO_ITEM)
		{
			pushableItemPtr = &g_Level.Items[pushablePtr->Stack.ItemNumberAbove];
			pushablePtr = &GetPushableInfo(*pushableItemPtr);

			if (pushablePtr->UseBridgeCollision)
				addBridge ? AddBridge(pushableItemPtr->Index) : RemoveBridge(pushableItemPtr->Index);
		}
	}
}
