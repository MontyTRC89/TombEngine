#pragma once
#include "Game/collision/collide_room.h"

class EulerAngles;
struct ItemInfo;
struct CollisionInfo;
struct LaraInfo;
struct VaultTestResult;
struct WaterClimbOutTestResult;
enum LaraState;

// -----------------------------
// HELPER FUNCTIONS
// For State Control & Collision
// -----------------------------

void HandleLaraMovementParameters(ItemInfo* item, CollisionInfo* coll);
bool HandleLaraVehicle(ItemInfo* item, CollisionInfo* coll);
void EaseOutLaraHeight(ItemInfo* item, int height);
void DoLaraStep(ItemInfo* item, CollisionInfo* coll);
void DoLaraMonkeyStep(ItemInfo* item, CollisionInfo* coll);
void DoLaraCrawlToHangSnap(ItemInfo* item, CollisionInfo* coll);
void DoLaraTightropeBalance(ItemInfo* item);
void DoLaraTightropeLean(ItemInfo* item);
void DoLaraTightropeBalanceRegen(ItemInfo* item);
void DoLaraFallDamage(ItemInfo* item);

LaraInfo*& GetLaraInfo(ItemInfo* item);
LaraState GetLaraCornerShimmyState(ItemInfo* item, CollisionInfo* coll);
float GetLaraSlideDirection(ItemInfo* item, CollisionInfo* coll);

float ModulateLaraTurnRate(float turnRate, float accelRate, float minTurnRate, float maxTurnRate, float axisCoeff, bool invert);
void ModulateLaraTurnRateX(ItemInfo* item, float accelRate, float minTurnRate, float maxTurnRate, bool invert = true);
void ModulateLaraTurnRateY(ItemInfo* item, float accelRate, float minTurnRate, float maxTurnRate, bool invert = false);
void ModulateLaraSwimTurnRates(ItemInfo* item, CollisionInfo* coll);
void ModulateLaraSubsuitSwimTurnRates(ItemInfo* item);
void UpdateLaraSubsuitAngles(ItemInfo* item);
void ModulateLaraLean(ItemInfo* item, CollisionInfo* coll, float baseRate, float maxAngle);
void ModulateLaraCrawlFlex(ItemInfo* item, float baseRate, float maxAngle);
void ModulateLaraSlideVelocity(ItemInfo* item, CollisionInfo* coll);
void AlignLaraToSurface(ItemInfo* item, float alpha = 0.15f);

void SetLaraJumpDirection(ItemInfo* item, CollisionInfo* coll);
void SetLaraRunJumpQueue(ItemInfo* item, CollisionInfo* coll);
void SetLaraVault(ItemInfo* item, CollisionInfo* coll, VaultTestResult vaultResult);
void SetContextWaterClimbOut(ItemInfo* item, CollisionInfo* coll, WaterClimbOutTestResult climbOutContext);
void SetLaraLand(ItemInfo* item, CollisionInfo* coll);
void SetLaraFallAnimation(ItemInfo* item);
void SetLaraFallBackAnimation(ItemInfo* item);
void SetLaraMonkeyFallAnimation(ItemInfo* item);
void SetLaraMonkeyRelease(ItemInfo* item);
void SetLaraSlideAnimation(ItemInfo* item, CollisionInfo* coll);
void SetLaraHang(ItemInfo* item);
void SetLaraHangReleaseAnimation(ItemInfo* item);
void SetLaraCornerAnimation(ItemInfo* item, CollisionInfo* coll, bool flip);
void SetLaraSwimDiveAnimation(ItemInfo* item);

void ResetLaraTurnRate(ItemInfo* item, bool divesuit = false);
void ResetLaraLean(ItemInfo* item, float alpha = 1.0f, bool resetRoll = true, bool resetPitch = true);
void ResetLaraFlex(ItemInfo* item, float alpha = 1.0f);

void RumbleLaraHealthCondition(ItemInfo* item);
