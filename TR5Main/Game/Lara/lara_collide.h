#pragma once
#include "lara_struct.h"

bool LaraDeflectEdge(ITEM_INFO* item, COLL_INFO* coll);
void LaraDeflectEdgeJump(ITEM_INFO* item, COLL_INFO* coll);
bool LaraHitCeiling(ITEM_INFO* item, COLL_INFO* coll);
void LaraCollideStop(ITEM_INFO* item, COLL_INFO* coll);
void SnapLaraToEdgeOfBlock(ITEM_INFO* item, COLL_INFO* coll, short angle);
short GetDirOctant(int rot);
void GetLaraDeadlyBounds();

bool TestLaraWallDeflect(COLL_INFO* coll);
void SetLaraWallDeflect(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraHitCeiling(COLL_INFO* coll);
void SetLaraHitCeiling(ITEM_INFO* item, COLL_INFO* coll);
