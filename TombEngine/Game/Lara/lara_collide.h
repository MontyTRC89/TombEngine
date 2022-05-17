#pragma once

struct ItemInfo;
struct CollisionInfo;

constexpr auto DEFLECT_STRAIGHT_ANGLE = 5.0f;
constexpr auto DEFLECT_DIAGONAL_ANGLE = 12.0f;
constexpr auto DEFLECT_STRAIGHT_ANGLE_CRAWL = 2.0f;
constexpr auto DEFLECT_DIAGONAL_ANGLE_CRAWL = 5.0f;

// -----------------------------
// COLLISION TEST FUNCTIONS
// For State Control & Collision
// -----------------------------

bool LaraDeflectEdge(ItemInfo* item, CollisionInfo* coll);
bool LaraDeflectEdgeJump(ItemInfo* item, CollisionInfo* coll);
void LaraSlideEdgeJump(ItemInfo* item, CollisionInfo* coll);
bool LaraDeflectEdgeCrawl(ItemInfo* item, CollisionInfo* coll);
bool LaraDeflectEdgeMonkey(ItemInfo* item, CollisionInfo* coll);
void LaraCollideStop(ItemInfo* item, CollisionInfo* coll);
void LaraCollideStopCrawl(ItemInfo* item, CollisionInfo* coll);
void LaraCollideStopMonkey(ItemInfo* item, CollisionInfo* coll);
void LaraSnapToEdgeOfBlock(ItemInfo* item, CollisionInfo* coll, short angle);
void LaraResetGravityStatus(ItemInfo* item, CollisionInfo* coll);
void LaraSnapToHeight(ItemInfo* item, CollisionInfo* coll);
void GetLaraDeadlyBounds();

void LaraJumpCollision(ItemInfo* item, CollisionInfo* coll, short moveAngle);
void LaraSurfaceCollision(ItemInfo* item, CollisionInfo* coll);
void LaraSwimCollision(ItemInfo* item, CollisionInfo* coll);

void LaraWaterCurrent(ItemInfo* item, CollisionInfo* coll);

bool TestLaraHitCeiling(CollisionInfo* coll);
void SetLaraHitCeiling(ItemInfo* item, CollisionInfo* coll);

bool TestLaraObjectCollision(ItemInfo* item, short angle, int distance, int height = 0, int side = 0);
