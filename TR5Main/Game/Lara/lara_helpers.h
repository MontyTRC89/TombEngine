#pragma once
#include "Game/collision/collide_room.h"

struct ITEM_INFO;
struct COLL_INFO;

// -----------------------------
// HELPER FUNCTIONS
// For State Control & Collision
// -----------------------------

void DoLaraLean(ITEM_INFO* item, COLL_INFO* coll, int maxAngle, short rate);
void DoLaraStep(ITEM_INFO* item, COLL_INFO* coll);
void DoLaraCrawlVault(ITEM_INFO* item, COLL_INFO* coll);
void DoLaraCrawlToHangSnap(ITEM_INFO* item, COLL_INFO* coll);
void DoLaraCrawlFlex(ITEM_INFO* item, COLL_INFO* coll, short maxAngle, short rate);

void SetLaraJumpDirection(ITEM_INFO* item, COLL_INFO* coll);
void SetLaraJumpQueue(ITEM_INFO* item, COLL_INFO* coll);

void SetLaraFallState(ITEM_INFO* item);
void SetLaraFallBackState(ITEM_INFO* item);
void SetLaraSlideState(ITEM_INFO* item, COLL_INFO* coll);

SplatType GetLaraSplatType(ITEM_INFO* item, int dist, int height, int side = 0);
short GetLaraSlideDirection(COLL_INFO* coll);

void ResetLaraFlex(ITEM_INFO* item, float rate = 1.0f);
void HandleLaraMovementParameters(ITEM_INFO* item, COLL_INFO* coll);
