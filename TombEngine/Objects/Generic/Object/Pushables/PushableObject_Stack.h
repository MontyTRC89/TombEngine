#pragma once

struct ItemInfo;

namespace TEN::Entities::Generic
{
	void InitializePushablesStacks();
	std::vector<int> FindAllPushables(const std::vector<ItemInfo>& objectsList);

	void StackPushable(int itemNumber, int itemNumber_target);
	void UnpilePushable(int itemNumber);
	
	int SearchNearPushablesStack(int itemNumber);
	int FindPushableStackInRoom(int itemNumber, int roomNumber);

	int CountPushablesInStack(int itemNumber);
	bool IsUnderStackLimit(int itemNumber);

	void StartMovePushableStack(int itemNumber);
	void StopMovePushableStack(int itemNumber);

	void StartFallPushableStack(int itemNumber);
	void StopFallPushableStack(int itemNumber);

	void VerticalPosAddition(int itemNumber, int deltaY);
}