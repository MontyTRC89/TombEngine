#pragma once
#include "lara_struct.h"

int LaraDeflectEdge(ITEM_INFO* item, COLL_INFO* coll);
void LaraDeflectEdgeJump(ITEM_INFO* item, COLL_INFO* coll);
int LaraDeflectEdgeDuck(ITEM_INFO* item, COLL_INFO* coll);
int LaraHitCeiling(ITEM_INFO* item, COLL_INFO* coll);
void LaraCollideStop(ITEM_INFO* item, COLL_INFO* coll);
