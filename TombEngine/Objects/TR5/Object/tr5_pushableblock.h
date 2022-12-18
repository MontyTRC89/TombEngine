#pragma once

class Vector3i;
struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Generic
{
	struct PushableInfo;

	PushableInfo& GetPushableInfo(ItemInfo& item);
	void InitialisePushableBlock(short itemNumber);

	void ClearMovableBlockSplitters(const Vector3i& pos, short roomNumber);
	void PushableBlockControl(short itemNumber);
	void PushableBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	bool TestBlockMovable(ItemInfo* item, int blockHeight);
	bool TestBlockPush(ItemInfo* item, int blockHeight, int quadrant);
	bool TestBlockPull(ItemInfo* item, int blockHeight, int quadrant);

	// Pushable stack utilities
	void MoveStackXZ(short itemNumber);
	void MoveStackY(short itemNumber, int y);
	void RemoveBridgeStack(short itemNumber);
	void AddBridgeStack(short itemNumber);
	void RemoveFromStack(short itemNumber);
	int FindStack(short itemNumber);
	int GetStackHeight(ItemInfo& item);
	bool CheckStackLimit(ItemInfo& item);

	void PushLoop(ItemInfo& item);
	void PushEnd(ItemInfo& item);

	std::optional<int> PushableBlockFloor(short itemNumber, int x, int y, int z);
	std::optional<int> PushableBlockCeiling(short itemNumber, int x, int y, int z);
	int PushableBlockFloorBorder(short itemNumber);
	int PushableBlockCeilingBorder(short itemNumber);
}
