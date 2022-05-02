#pragma once
#include "Game/collision/collide_room.h"

class EulerAngles;
struct ITEM_INFO;
struct CollisionInfo;
struct LaraInfo;
struct VaultTestResult;

// -----------------------------
// HELPER FUNCTIONS
// For State Control & Collision
// -----------------------------

void HandleLaraMovementParameters(ITEM_INFO* item, CollisionInfo* coll);
bool HandleLaraVehicle(ITEM_INFO* item, CollisionInfo* coll);
void EaseOutLaraHeight(ITEM_INFO* item, int height);
void DoLaraLean(ITEM_INFO* item, CollisionInfo* coll, float maxAngle, float rate);
void DoLaraStep(ITEM_INFO* item, CollisionInfo* coll);
void DoLaraMonkeyStep(ITEM_INFO* item, CollisionInfo* coll);
void DoLaraCrawlToHangSnap(ITEM_INFO* item, CollisionInfo* coll);
void DoLaraCrawlFlex(ITEM_INFO* item, CollisionInfo* coll, float maxAngle, float rate);
void DoLaraTightropeBalance(ITEM_INFO* item);
void DoLaraTightropeLean(ITEM_INFO* item);
void DoLaraTightropeBalanceRegen(ITEM_INFO* item);
void DoLaraFallDamage(ITEM_INFO* item);

LaraInfo*& GetLaraInfo(ITEM_INFO* item);
float GetLaraSlideDirection(ITEM_INFO* item, CollisionInfo* coll);

void ModulateLaraSlideVelocity(ITEM_INFO* item, CollisionInfo* coll);
void UpdateLaraSubsuitAngles(ITEM_INFO* item);
void ModulateLaraSubsuitSwimTurn(ITEM_INFO* item);
void ModulateLaraSwimTurn(ITEM_INFO* item, CollisionInfo* coll);

void SetLaraJumpDirection(ITEM_INFO* item, CollisionInfo* coll);
void SetLaraRunJumpQueue(ITEM_INFO* item, CollisionInfo* coll);
void SetLaraVault(ITEM_INFO* item, CollisionInfo* coll, VaultTestResult vaultResult);
void SetLaraLand(ITEM_INFO* item, CollisionInfo* coll);
void SetLaraFallAnimation(ITEM_INFO* item);
void SetLaraFallBackAnimation(ITEM_INFO* item);
void SetLaraMonkeyFallAnimation(ITEM_INFO* item);
void SetLaraMonkeyRelease(ITEM_INFO* item);
void SetLaraSlideAnimation(ITEM_INFO* item, CollisionInfo* coll);
void SetLaraCornerAnimation(ITEM_INFO* item, CollisionInfo* coll, bool flip);
void SetLaraSwimDiveAnimation(ITEM_INFO* item);

void ResetLaraLean(ITEM_INFO* item, float rate = 1.0f, bool resetRoll = true, bool resetPitch = true);
void ResetLaraFlex(ITEM_INFO* item, float rate = 1.0f);
