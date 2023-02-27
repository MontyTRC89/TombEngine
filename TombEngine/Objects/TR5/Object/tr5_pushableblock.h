#pragma once

class Vector3i;
struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Generic
{
	struct PushableInfo;
	PushableInfo* GetPushableInfo(ItemInfo* item);

	// Main functions
	void InitialisePushableBlock(short itemNumber);
	void PushableBlockControl(short itemNumber);
	void PushableBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	// Behaviour functions
	bool PushableBlockManageGravity(ItemInfo& pushableItem, PushableInfo& pushableInfo, const short itemNumber); //Maybe itemNumber could be cut out.
	void PushableBlockManageIdle(ItemInfo& pushableItem, PushableInfo& pushableInfo, const short itemNumber);
	bool PushableBlockManageMoving(ItemInfo& pushableItem, PushableInfo& pushableInfo, const short itemNumber);
	void PushableBlockManageSounds(const ItemInfo& pushableItem, PushableInfo& pushableInfo);

	//Floor Data update functions
	void AdjustStopperFlag(ItemInfo* item, int direction);
	void ClearMovableBlockSplitters(const Vector3i& pos, short roomNumber);

	// Test functions
	bool IsNextSectorValid(ItemInfo& item, const int blockHeight, const int quadrant, const bool isPulling = true);
	bool IsValidForLara(const ItemInfo& pushableItem, const PushableInfo& pushableInfo, const int quadrant);
	bool TestBlockMovable(ItemInfo& item, int blockHeight);

	// Stack utilities functions
	void MoveStackXZ(short itemNumber);
	void MoveStackY(short itemNumber, int y);
	void RemoveBridgeStack(short itemNumber);
	void AddBridgeStack(short itemNumber);
	void RemoveFromStack(short itemNumber);
	int FindStack(short itemNumber);
	int GetStackHeight(ItemInfo* item);
	bool CheckStackLimit(ItemInfo* item);

	// Floor data collision functions
	std::optional<int> PushableBlockFloor(short itemNumber, int x, int y, int z);
	std::optional<int> PushableBlockCeiling(short itemNumber, int x, int y, int z);
	int PushableBlockFloorBorder(short itemNumber);
	int PushableBlockCeilingBorder(short itemNumber);
}
