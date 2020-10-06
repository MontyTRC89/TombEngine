#pragma once
#include "items.h"
#include "collide.h"

void ClearMovableBlockSplitters(int x, int y, int z, short roomNumber);
void InitialisePushableBlock(short itemNum);
void PushableBlockControl(short itemNum);
void PushableBlockCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll);
bool TestBlockMovable(ITEM_INFO* item, int blockHeight);
bool TestBlockPush(ITEM_INFO* item, int blockHeight, unsigned short quadrant);
bool TestBlockPull(ITEM_INFO* item, int blocHeight, short quadrant);
