#pragma once

struct ItemInfo;

namespace TEN::Entities::Generic
{
	std::optional<int> PushableBridgeFloor(int itemNumber, int x, int y, int z);
	std::optional<int> PushableBridgeCeiling(int itemNumber, int x, int y, int z);
	int PushableBridgeFloorBorder(int itemNumber);
	int PushableBridgeCeilingBorder(int itemNumber);

	void AddPushableBridge(ItemInfo& pushableItem);
	void AddPushableStackBridge(ItemInfo& pushableItem, bool addBridge);
	void RemovePushableBridge(ItemInfo& pushableItem);
	void UpdatePushableBridge(const ItemInfo& pushableItem);
}
