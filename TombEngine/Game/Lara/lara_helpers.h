#pragma once
#include "Game/collision/collide_room.h"

struct ItemInfo;
struct CollisionInfo;
struct LaraInfo;
struct VaultTestResult;

// -----------------------------
// HELPER FUNCTIONS
// For State Control & Collision
// -----------------------------

void HandleLaraMovementParameters(ItemInfo* item, CollisionInfo* coll);
bool HandleLaraVehicle(ItemInfo* item, CollisionInfo* coll);
void ApproachLaraTargetOrientation(ItemInfo* item, Vector3Shrt targetOrient, float rate = 1.0f);
void EaseOutLaraHeight(ItemInfo* item, int height);
void DoLaraLean(ItemInfo* item, CollisionInfo* coll, short maxAngle, short rate);
void DoLaraStep(ItemInfo* item, CollisionInfo* coll);
void DoLaraMonkeyStep(ItemInfo* item, CollisionInfo* coll);
void DoLaraCrawlToHangSnap(ItemInfo* item, CollisionInfo* coll);
void DoLaraCrawlFlex(ItemInfo* item, CollisionInfo* coll, short maxAngle, short rate);
void DoLaraTightropeBalance(ItemInfo* item);
void DoLaraTightropeLean(ItemInfo* item);
void DoLaraTightropeBalanceRegen(ItemInfo* item);
void DoLaraFallDamage(ItemInfo* item);

LaraInfo*& GetLaraInfo(ItemInfo* item);
short GetLaraSlideDirection(ItemInfo* item, CollisionInfo* coll);

void ModulateLaraSlideVelocity(ItemInfo* item, CollisionInfo* coll);
void UpdateLaraSubsuitAngles(ItemInfo* item);
void ModulateLaraSubsuitSwimTurn(ItemInfo* item);
void ModulateLaraSwimTurn(ItemInfo* item, CollisionInfo* coll);

void SetLaraJumpDirection(ItemInfo* item, CollisionInfo* coll);
void SetLaraRunJumpQueue(ItemInfo* item, CollisionInfo* coll);
void SetLaraVault(ItemInfo* item, CollisionInfo* coll, VaultTestResult vaultResult);
void SetLaraLand(ItemInfo* item, CollisionInfo* coll);
void SetLaraFallAnimation(ItemInfo* item);
void SetLaraFallBackAnimation(ItemInfo* item);
void SetLaraMonkeyFallAnimation(ItemInfo* item);
void SetLaraMonkeyRelease(ItemInfo* item);
void SetLaraSlideAnimation(ItemInfo* item, CollisionInfo* coll);
void SetLaraCornerAnimation(ItemInfo* item, CollisionInfo* coll, bool flip);
void SetLaraSwimDiveAnimation(ItemInfo* item);

void ResetLaraLean(ItemInfo* item, float rate = 1.0f, bool resetRoll = true, bool resetPitch = true);
void ResetLaraFlex(ItemInfo* item, float rate = 1.0f);
