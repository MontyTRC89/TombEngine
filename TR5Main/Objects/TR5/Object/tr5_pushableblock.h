#pragma once
#include "items.h"
#include "collide.h"

void ClearMovableBlockSplitters(int x, int y, int z, short roomNumber);
void InitialisePushableBlock(short itemNum);
void PushableBlockControl(short itemNum);
void PushableBlockCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll);
int TestBlockMovable(ITEM_INFO* item, int blokhite);
int TestBlockPush(ITEM_INFO* item, int blockhite, unsigned short quadrant);
int TestBlockPull(ITEM_INFO* item, int blockhite, short quadrant);
