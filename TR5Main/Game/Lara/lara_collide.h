#pragma once

struct ITEM_INFO;
struct CollisionInfo;

constexpr auto DEFLECT_STRAIGHT_ANGLE = 5.0f;
constexpr auto DEFLECT_DIAGONAL_ANGLE = 12.0f;
constexpr auto DEFLECT_STRAIGHT_ANGLE_CRAWL = 2.0f;
constexpr auto DEFLECT_DIAGONAL_ANGLE_CRAWL = 5.0f;

// -----------------------------
// COLLISION TEST FUNCTIONS
// For State Control & Collision
// -----------------------------

bool LaraDeflectEdge(ITEM_INFO* item, CollisionInfo* coll);
bool LaraDeflectEdgeJump(ITEM_INFO* item, CollisionInfo* coll);
void LaraSlideEdgeJump(ITEM_INFO* item, CollisionInfo* coll);
bool LaraDeflectEdgeCrawl(ITEM_INFO* item, CollisionInfo* coll);
bool LaraDeflectEdgeMonkey(ITEM_INFO* item, CollisionInfo* coll);
void LaraCollideStop(ITEM_INFO* item, CollisionInfo* coll);
void LaraCollideStopCrawl(ITEM_INFO* item, CollisionInfo* coll);
void LaraCollideStopMonkey(ITEM_INFO* item, CollisionInfo* coll);
void LaraSnapToEdgeOfBlock(ITEM_INFO* item, CollisionInfo* coll, short angle);
void LaraResetGravityStatus(ITEM_INFO* item, CollisionInfo* coll);
void LaraSnapToHeight(ITEM_INFO* item, CollisionInfo* coll);
void GetLaraDeadlyBounds();

void LaraJumpCollision(ITEM_INFO* item, CollisionInfo* coll, short moveAngle);
void LaraSurfaceCollision(ITEM_INFO* item, CollisionInfo* coll);
void LaraSwimCollision(ITEM_INFO* item, CollisionInfo* coll);

// TODO: Temporary placement.
bool TestLaraHitCeiling(CollisionInfo* coll);
void SetLaraHitCeiling(ITEM_INFO* item, CollisionInfo* coll);

bool TestLaraObjectCollision(ITEM_INFO* item, short angle, int distance, int height = 0, int side = 0);
