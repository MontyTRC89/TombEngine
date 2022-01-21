#pragma once
#include "Game/collision/collide_room.h"

struct ITEM_INFO;
struct COLL_INFO;

// -----------------------------
// HELPER FUNCTIONS
// For State Control & Collision
// -----------------------------

void DoLaraLean(ITEM_INFO* item, COLL_INFO* coll, short maxAngle, short rate);
void DoLaraStep(ITEM_INFO* item, COLL_INFO* coll);
void DoLaraMonkeyStep(ITEM_INFO* item, COLL_INFO* coll);
void DoLaraCrawlVault(ITEM_INFO* item, COLL_INFO* coll);
void DoLaraCrawlToHangSnap(ITEM_INFO* item, COLL_INFO* coll);
void DoLaraCrawlFlex(ITEM_INFO* item, COLL_INFO* coll, short maxAngle, short rate);
void DoLaraLand(ITEM_INFO* item, COLL_INFO* coll);

void SetLaraJumpDirection(ITEM_INFO* item, COLL_INFO* coll);
void SetLaraRunJumpQueue(ITEM_INFO* item, COLL_INFO* coll);

void SetLaraFallState(ITEM_INFO* item);
void SetLaraFallBackState(ITEM_INFO* item);
void SetLaraMonkeyFallState(ITEM_INFO* item);
void SetLaraMonkeyRelease(ITEM_INFO* item);
short GetLaraSlideDirection(COLL_INFO* coll);
void SetLaraSlideState(ITEM_INFO* item, COLL_INFO* coll);

void ResetLaraFlex(ITEM_INFO* item, float rate = 1.0f);
void HandleLaraMovementParameters(ITEM_INFO* item, COLL_INFO* coll);
