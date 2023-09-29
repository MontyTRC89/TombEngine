#pragma once

namespace TEN::Entities::Generic
{
	std::optional<int> PushableBridgeFloor(int itemNumber, int x, int y, int z);
	std::optional<int> PushableBridgeCeiling(int itemNumber, int x, int y, int z);
	int PushableBridgeFloorBorder(int itemNumber);
	int PushableBridgeCeilingBorder(int itemNumber);

	void AddPushableBridge(int itemNumber);
	void RemovePushableBridge(int itemNumber);
	void UpdatePushableBridge(int itemNumber);

	void AddPushableStackBridge(int itemNumber, bool addBridge);
}
