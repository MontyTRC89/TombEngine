#pragma once

class Vector3i;
struct ItemInfo;

namespace TEN::Entities::Generic
{
	std::optional<int> GetPushableBridgeFloorHeight(const ItemInfo& item, const Vector3i& pos);
	std::optional<int> GetPushableBridgeCeilingHeight(const ItemInfo& item, const Vector3i& pos);
	int GetPushableBridgeFloorBorder(const ItemInfo& item);
	int GetPushableBridgeCeilingBorder(const ItemInfo& item);

	void AddPushableBridge(ItemInfo& pushableItem);
	void AddPushableStackBridge(ItemInfo& pushableItem, bool addBridge);
	void RemovePushableBridge(ItemInfo& pushableItem);
	void UpdatePushableBridge(ItemInfo& pushableItem);
}
