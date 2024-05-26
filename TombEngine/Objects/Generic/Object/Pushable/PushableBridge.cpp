#include "framework.h"
#include "Objects/Generic/Object/Pushable/PushableBridge.h"

#include "Game/collision/floordata.h"
#include "Objects/Generic/Object/Pushable/PushableObject.h"
#include "Specific/level.h"

using namespace TEN::Collision::Floordata;

namespace TEN::Entities::Generic
{
	std::optional<int> GetPushableBridgeFloorHeight(const ItemInfo& item, const Vector3i& pos)
	{
		const auto& pushable = GetPushableInfo(item);

		auto boxHeight = GetBridgeItemIntersect(item, pos, false);

		if (item.Active &&item.Status != ITEM_INVISIBLE &&
			pushable.UseRoomCollision &&
			boxHeight.has_value())
		{
			int height = item.Pose.Position.y - GetPushableHeight(item);
			return height;
		}

		return std::nullopt;
	}

	std::optional<int> GetPushableBridgeCeilingHeight(const ItemInfo& item, const Vector3i& pos)
	{
		const auto& pushable = GetPushableInfo(item);

		auto boxHeight = GetBridgeItemIntersect(item, pos, true);

		if (item.Active && item.Status != ITEM_INVISIBLE &&
			pushable.UseRoomCollision &&
			boxHeight.has_value())
		{
			return item.Pose.Position.y;
		}

		return std::nullopt;
	}

	int GetPushableBridgeFloorBorder(const ItemInfo& item)
	{
		return (item.Pose.Position.y - GetPushableHeight(item));
	}

	int GetPushableBridgeCeilingBorder(const ItemInfo& item)
	{
		return item.Pose.Position.y;
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
		
		while (pushablePtr->Stack.ItemNumberAbove != NO_VALUE)
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

	void UpdatePushableBridge(ItemInfo& pushableItem)
	{
		const auto& pushable = GetPushableInfo(pushableItem);
		auto& bridge = GetBridgeObject(pushableItem);

		if (pushable.UseRoomCollision)
			bridge.Update(pushableItem);
	}
}
