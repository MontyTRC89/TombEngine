#pragma once
#include "Game/collision/collide_room.h"

struct ITEM_INFO;
struct COLL_INFO;
struct LaraInfo;
struct VaultTestResult;

// -----------------------------
// HELPER FUNCTIONS
// For State Control & Collision
// -----------------------------

void DoLaraLean(ITEM_INFO* item, COLL_INFO* coll, short maxAngle, short rate);
void ApproachLaraTargetAngle(ITEM_INFO* item, short targetAngle, float rate = 1.0f);
void EaseOutLaraHeight(ITEM_INFO* item, int height);
void DoLaraStep(ITEM_INFO* item, COLL_INFO* coll);
void DoLaraMonkeyStep(ITEM_INFO* item, COLL_INFO* coll);
void DoLaraCrawlToHangSnap(ITEM_INFO* item, COLL_INFO* coll);
void DoLaraCrawlFlex(ITEM_INFO* item, COLL_INFO* coll, short maxAngle, short rate);
void DoLaraFallDamage(ITEM_INFO* item);

LaraInfo*& GetLaraInfo(ITEM_INFO* item);
short GetLaraSlideDirection(ITEM_INFO* item, COLL_INFO* coll);

void SetLaraJumpDirection(ITEM_INFO* item, COLL_INFO* coll);
void SetLaraRunJumpQueue(ITEM_INFO* item, COLL_INFO* coll);
void SetLaraVault(ITEM_INFO* item, COLL_INFO* coll, VaultTestResult vaultResult);
void SetLaraLand(ITEM_INFO* item, COLL_INFO* coll);
void SetLaraFallState(ITEM_INFO* item);
void SetLaraFallBackState(ITEM_INFO* item);
void SetLaraMonkeyFallState(ITEM_INFO* item);
void SetLaraMonkeyRelease(ITEM_INFO* item);
void SetLaraSlideState(ITEM_INFO* item, COLL_INFO* coll);
void SetCornerAnimation(ITEM_INFO* item, COLL_INFO* coll, bool flip);

void ResetLaraLean(ITEM_INFO* item, float rate = 1.0f, bool resetRoll = true, bool resetPitch = true);
void ResetLaraFlex(ITEM_INFO* item, float rate = 1.0f);

void HandleLaraMovementParameters(ITEM_INFO* item, COLL_INFO* coll);
void HandleLaraVehicle(ITEM_INFO* item, COLL_INFO* coll);
