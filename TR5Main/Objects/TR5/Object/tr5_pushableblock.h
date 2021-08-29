#pragma once
struct ITEM_INFO;
struct COLL_INFO;



void ClearMovableBlockSplitters(int x, int y, int z, short roomNumber);
void InitialisePushableBlock(short itemNum);
void PushableBlockControl(short itemNumber);
void PushableBlockCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll);
int TestBlockMovable(ITEM_INFO* item, int blokhite);
int TestBlockPush(ITEM_INFO* item, int blockhite, unsigned short quadrant);
int TestBlockPull(ITEM_INFO* item, int blockhite, short quadrant);
void MoveStackXZ(short itemNum);
void MoveStackY(short itemNum, int y);
void RemoveBridgeStack(short itemNum);
void AddBridgeStack(short itemNum);
void RemoveFromStack(short itemNum);
int FindStack(short itemNum);
int GetStackHeight(ITEM_INFO* item);
bool CheckStackLimit(ITEM_INFO* item);
void pushLoop(ITEM_INFO* item);
void pushEnd(ITEM_INFO* item);
std::optional<int> PushableBlockFloor(short itemNumber, int x, int y, int z);
std::optional<int> PushableBlockCeiling(short itemNumber, int x, int y, int z);
int PushableBlockFloorBorder(short itemNumber);
int PushableBlockCeilingBorder(short itemNumber);
