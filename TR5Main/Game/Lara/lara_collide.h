#pragma once

struct ITEM_INFO;
struct COLL_INFO;

constexpr auto DEFLECT_STRAIGHT_ANGLE = 5.0f;
constexpr auto DEFLECT_DIAGONAL_ANGLE = 12.0f;
constexpr auto DEFLECT_STRAIGHT_ANGLE_CRAWL = 2.0f;
constexpr auto DEFLECT_DIAGONAL_ANGLE_CRAWL = 5.0f;

// -----------------------------
// COLLISION TEST FUNCTIONS
// For State Control & Collision
// -----------------------------

bool LaraDeflectEdge(ITEM_INFO* item, COLL_INFO* coll);
bool LaraDeflectEdgeJump(ITEM_INFO* item, COLL_INFO* coll);
void LaraSlideEdgeJump(ITEM_INFO* item, COLL_INFO* coll);
bool LaraDeflectEdgeCrawl(ITEM_INFO* item, COLL_INFO* coll);
bool LaraDeflectEdgeMonkey(ITEM_INFO* item, COLL_INFO* coll);
void LaraCollideStop(ITEM_INFO* item, COLL_INFO* coll);
void LaraCollideStopCrawl(ITEM_INFO* item, COLL_INFO* coll);
void LaraCollideStopMonkey(ITEM_INFO* item, COLL_INFO* coll);
void LaraSnapToEdgeOfBlock(ITEM_INFO* item, COLL_INFO* coll, short angle);
void LaraResetGravityStatus(ITEM_INFO* item, COLL_INFO* coll);
void LaraSnapToHeight(ITEM_INFO* item, COLL_INFO* coll);
void GetLaraDeadlyBounds();

void LaraJumpCollision(ITEM_INFO* item, COLL_INFO* coll, short moveAngle);
void LaraSurfaceCollision(ITEM_INFO* item, COLL_INFO* coll);
void LaraSwimCollision(ITEM_INFO* item, COLL_INFO* coll);

// TODO: Temporary placement.
bool TestLaraHitCeiling(COLL_INFO* coll);
void SetLaraHitCeiling(ITEM_INFO* item, COLL_INFO* coll);

bool TestLaraObjectCollision(ITEM_INFO* item, short angle, int distance, int height = 0, int side = 0);
