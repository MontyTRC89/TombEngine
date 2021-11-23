#pragma once
struct ITEM_INFO;
struct COLL_INFO;

constexpr auto DEFLECT_STRAIGHT_ANGLE = 5.0f;
constexpr auto DEFLECT_DIAGONAL_ANGLE = 12.0f;

bool LaraDeflectEdge(ITEM_INFO* item, COLL_INFO* coll);
void LaraDeflectEdgeJump(ITEM_INFO* item, COLL_INFO* coll);
bool LaraDeflectEdgeCrawl(ITEM_INFO* item, COLL_INFO* coll);
bool LaraHitCeiling(ITEM_INFO* item, COLL_INFO* coll);
void LaraCollideStop(ITEM_INFO* item, COLL_INFO* coll);
void LaraSnapToEdgeOfBlock(ITEM_INFO* item, COLL_INFO* coll, short angle);
void LaraResetGravityStatus(ITEM_INFO* item, COLL_INFO* coll);
void LaraSnapToHeight(ITEM_INFO* item, COLL_INFO* coll);
short GetDirOctant(int rot);
void GetLaraDeadlyBounds();

void LaraSurfaceCollision(ITEM_INFO* item, COLL_INFO* coll);
void LaraSwimCollision(ITEM_INFO* item, COLL_INFO* coll);
