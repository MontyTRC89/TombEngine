#pragma once

struct ItemInfo;
struct CollisionInfo;

void ClearMovableBlockSplitters(int x, int y, int z, short roomNumber);
void InitialisePushableBlock(short itemNumber);
void PushableBlockControl(short itemNumber);
void PushableBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
bool TestBlockMovable(ItemInfo* item, int blockHeight);
bool TestBlockPush(ItemInfo* item, int blockHeight, unsigned short quadrant);
bool TestBlockPull(ItemInfo* item, int blockHeight, short quadrant);
void MoveStackXZ(short itemNumber);
void MoveStackY(short itemNumber, int y);
void RemoveBridgeStack(short itemNumber);
void AddBridgeStack(short itemNumber);
void RemoveFromStack(short itemNumber);
int FindStack(short itemNumber);
int GetStackHeight(ItemInfo* item);
bool CheckStackLimit(ItemInfo* item);
void PushLoop(ItemInfo* item);
void PushEnd(ItemInfo* item);
std::optional<int> PushableBlockFloor(short itemNumber, int x, int y, int z);
std::optional<int> PushableBlockCeiling(short itemNumber, int x, int y, int z);
int PushableBlockFloorBorder(short itemNumber);
int PushableBlockCeilingBorder(short itemNumber);
