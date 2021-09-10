#pragma once
struct ITEM_INFO;
struct COLL_INFO;
int LaraDeflectEdge(ITEM_INFO* item, COLL_INFO* coll);
void LaraDeflectEdgeJump(ITEM_INFO* item, COLL_INFO* coll);
int LaraDeflectEdgeDuck(ITEM_INFO* item, COLL_INFO* coll);
int LaraHitCeiling(ITEM_INFO* item, COLL_INFO* coll);
void LaraCollideStop(ITEM_INFO* item, COLL_INFO* coll);
void SnapLaraToEdgeOfBlock(ITEM_INFO* item, COLL_INFO* coll, short angle);
short GetDirOctant(int rot);
void GetLaraDeadlyBounds();
