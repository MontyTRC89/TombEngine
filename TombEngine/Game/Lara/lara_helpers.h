#pragma once
#include "Game/collision/collide_room.h"

struct ItemInfo;
struct CollisionInfo;
struct LaraInfo;
struct LimbRotationData;
struct VaultTestResult;

// -----------------------------
// HELPER FUNCTIONS
// For State Control & Collision
// -----------------------------

void HandleLaraMovementParameters(ItemInfo* item, CollisionInfo* coll);
bool HandleLaraVehicle(ItemInfo* item, CollisionInfo* coll);
void EaseOutLaraHeight(ItemInfo* item, int height);
void SolvePlayerLegIK(ItemInfo& item, LimbRotationData& limbRot, int joint0, int joint1, int joint2, float heelHeight);
void DoPlayerLegIK(ItemInfo& item, CollisionInfo* coll, float heightTolerance = CLICK(1));
void DoPlayerArmIK(ItemInfo& item, CollisionInfo* coll, float heightTolerance = CLICK(1));
void DoLaraStep(ItemInfo* item, CollisionInfo* coll);
void DoLaraMonkeyStep(ItemInfo* item, CollisionInfo* coll);
void DoLaraCrawlToHangSnap(ItemInfo* item, CollisionInfo* coll);
void DoLaraTightropeBalance(ItemInfo* item);
void DoLaraTightropeLean(ItemInfo* item);
void DoLaraTightropeBalanceRegen(ItemInfo* item);
void DoLaraFallDamage(ItemInfo* item);

LaraInfo*& GetLaraInfo(ItemInfo* item);
short GetLaraSlideDirection(ItemInfo* item, CollisionInfo* coll);

short ModulateLaraTurnRate(short turnRate, short accelRate, short minTurnRate, short maxTurnRate, float axisCoeff, bool invert);
void ModulateLaraTurnRateX(ItemInfo* item, short accelRate, short minTurnRate, short maxTurnRate, bool invert = true);
void ModulateLaraTurnRateY(ItemInfo* item, short accelRate, short minTurnRate, short maxTurnRate, bool invert = false);
void ModulateLaraSwimTurnRates(ItemInfo* item, CollisionInfo* coll);
void ModulateLaraSubsuitSwimTurnRates(ItemInfo* item);
void UpdateLaraSubsuitAngles(ItemInfo* item);
void ModulateLaraLean(ItemInfo* item, CollisionInfo* coll, short baseRate, short maxAngle);
void ModulateLaraCrawlFlex(ItemInfo* item, short baseRate, short maxAngle);
void ModulateLaraSlideVelocity(ItemInfo* item, CollisionInfo* coll);
void AlignLaraToSurface(ItemInfo* item, float alpha = 0.15f);

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
void SetLaraVehicle(ItemInfo* item, ItemInfo* vehicle = nullptr);

void ResetLaraLean(ItemInfo* item, float rate = 1.0f, bool resetRoll = true, bool resetPitch = true);
void ResetLaraFlex(ItemInfo* item, float rate = 1.0f);

void RumbleLaraHealthCondition(ItemInfo* item);
