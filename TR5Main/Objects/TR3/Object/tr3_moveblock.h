#pragma once
#include "items.h"
#include "collide.h"

void InitialiseMovingBlock(short itemNum);
void ClearMovableBlockSplitters3(int x, int y, int z, short roomNumber);
void MovableBlockControl(short itemNumber);
void MovableBlockCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll);
bool TestBlockMovable3(ITEM_INFO* item, int blokhite);
bool TestBlockPush3(ITEM_INFO* item, int blokhite, unsigned short quadrant);
bool TestBlockPull3(ITEM_INFO* item, int blokhite, unsigned short quadrant);
