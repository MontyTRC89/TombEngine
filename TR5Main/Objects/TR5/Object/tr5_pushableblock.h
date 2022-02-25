#pragma once

struct ITEM_INFO;
struct COLL_INFO;

void ClearMovableBlockSplitters(int x, int y, int z, short roomNumber);
void InitialisePushableBlock(short itemNumber);
void PushableBlockControl(short itemNumber);
void PushableBlockCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
bool TestBlockMovable(ITEM_INFO* item, int blockHeight);
bool TestBlockPush(ITEM_INFO* item, int blockHeight, unsigned short quadrant);
bool TestBlockPull(ITEM_INFO* item, int blockHeight, short quadrant);
void MoveStackXZ(short itemNumber);
void MoveStackY(short itemNumber, int y);
void RemoveBridgeStack(short itemNumber);
void AddBridgeStack(short itemNumber);
void RemoveFromStack(short itemNumber);
int FindStack(short itemNumber);
int GetStackHeight(ITEM_INFO* item);
bool CheckStackLimit(ITEM_INFO* item);
void PushLoop(ITEM_INFO* item);
void PushEnd(ITEM_INFO* item);
std::optional<int> PushableBlockFloor(short itemNumber, int x, int y, int z);
std::optional<int> PushableBlockCeiling(short itemNumber, int x, int y, int z);
int PushableBlockFloorBorder(short itemNumber);
int PushableBlockCeilingBorder(short itemNumber);
