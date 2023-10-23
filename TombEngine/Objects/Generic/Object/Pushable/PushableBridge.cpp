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

		auto boxHeight = GetBridgeItemIntersect(Vector3i(x, y, z), itemNumber, false);

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

		auto boxHeight = GetBridgeItemIntersect(Vector3i(x, y, z), itemNumber, true);

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

	void AddPushableBridge(ItemInfo& pushableItem)
	{
		auto& pushable = GetPushableInfo(pushableItem);

		if (pushable.UseRoomCollision)
		{
			AddBridge(pushableItem.Index);
			pushable.UseBridgeCollision = true;
		}
	}

	void AddPushableStackBridge(ItemInfo& pushableItem, bool addBridge)
	{
		auto* pushableItemPtr = &g_Level.Items[pushableItem.Index];
		const auto* pushablePtr = &GetPushableInfo(*pushableItemPtr);

		// NOTE: Can't have stacked items on bridge.
		if (!pushablePtr->UseRoomCollision)
			return;

		if (pushablePtr->UseBridgeCollision)
			addBridge ? AddBridge(pushableItem.Index) : RemoveBridge(pushableItem.Index);
		
		while (pushablePtr->Stack.ItemNumberAbove != NO_ITEM)
		{
			pushableItemPtr = &g_Level.Items[pushablePtr->Stack.ItemNumberAbove];
			pushablePtr = &GetPushableInfo(*pushableItemPtr);

			if (pushablePtr->UseBridgeCollision)
				addBridge ? AddBridge(pushableItemPtr->Index) : RemoveBridge(pushableItemPtr->Index);
		}
	}

	void RemovePushableBridge(ItemInfo& pushableItem)
	{
		auto& pushable = GetPushableInfo(pushableItem);

		if (pushable.UseRoomCollision)
		{
			RemoveBridge(pushableItem.Index);
			pushable.UseBridgeCollision = false;
		}
	}

	void UpdatePushableBridge(const ItemInfo& pushableItem)
	{
		const auto& pushable = GetPushableInfo(pushableItem);

		if (pushable.UseRoomCollision)
			UpdateBridgeItem(pushableItem.Index);
	}
}
