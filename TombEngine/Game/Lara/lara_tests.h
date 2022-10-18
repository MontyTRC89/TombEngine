#pragma once
#include "Game/collision/collide_room.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara_test_structs.h"

using std::vector;

struct ItemInfo;
struct CollisionInfo;

// -----------------------------
// TEST FUNCTIONS
// For State Control & Collision
// -----------------------------

bool TestValidLedge(ItemInfo* item, CollisionInfo* coll, bool ignoreHeadroom = false, bool heightLimit = false);
bool TestValidLedgeAngle(ItemInfo* item, CollisionInfo* coll);

bool TestLaraHang(ItemInfo* item, CollisionInfo* coll);
bool DoLaraLedgeHang(ItemInfo* item, CollisionInfo* coll);

bool TestLaraClimbIdle(ItemInfo* item, CollisionInfo* coll);
bool TestLaraHangOnClimbableWall(ItemInfo* item, CollisionInfo* coll);
bool TestLaraNearClimbableWall(ItemInfo* item, FloorInfo* floor = nullptr);

bool TestLaraValidHangPosition(ItemInfo* item, CollisionInfo* coll);
CornerType TestLaraHangCorner(ItemInfo* item, CollisionInfo* coll, float testAngle);
CornerTestResult TestItemAtNextCornerPosition(ItemInfo* item, CollisionInfo* coll, float angle, bool outer);
bool TestHangSwingIn(ItemInfo* item, CollisionInfo* coll);
bool TestLaraHangSideways(ItemInfo* item, CollisionInfo* coll, short angle);

bool TestLaraWall(ItemInfo* item, int distance, int height, int side = 0);
bool TestLaraFacingCorner(ItemInfo* item, short angle, int distance);
bool LaraPositionOnLOS(ItemInfo* item, short angle, int distance);
int LaraFloorFront(ItemInfo* item, short angle, int distance);
int LaraCeilingFront(ItemInfo* item, short angle, int distance, int height);
CollisionResult LaraCollisionFront(ItemInfo* item, short angle, int distance);
CollisionResult LaraCeilingCollisionFront(ItemInfo* item, short angle, int distance, int height);

bool TestLaraWaterStepOut(ItemInfo* item, CollisionInfo* coll);
WaterClimbOutTestResult TestLaraWaterClimbOut(ItemInfo* item, CollisionInfo* coll);
bool TestLaraLadderClimbOut(ItemInfo* item, CollisionInfo* coll);
void TestLaraWaterDepth(ItemInfo* item, CollisionInfo* coll);

#ifndef NEW_TIGHTROPE
void GetTightropeFallOff(ItemInfo* item, int regularity);
#endif

bool TestLaraWeaponType(LaraWeaponType refWeaponType, const vector<LaraWeaponType>& weaponTypeList);
bool IsStandingWeapon(ItemInfo* item, LaraWeaponType weaponType);
bool IsVaultState(int state);
bool IsJumpState(int state);
bool IsRunJumpQueueableState(int state);
bool IsRunJumpCountableState(int state);

bool TestLaraSlide(ItemInfo* item, CollisionInfo* coll);
bool TestLaraLand(ItemInfo* item, CollisionInfo* coll);
bool TestLaraFall(ItemInfo* item, CollisionInfo* coll);
bool TestLaraMonkeyFall(ItemInfo* item, CollisionInfo* coll);

bool TestLaraStep(ItemInfo* item, CollisionInfo* coll);
bool TestLaraStepUp(ItemInfo* item, CollisionInfo* coll);
bool TestLaraStepDown(ItemInfo* item, CollisionInfo* coll);
bool TestLaraMonkeyStep(ItemInfo* item, CollisionInfo* coll);

VaultTestResult TestLaraVaultTolerance(ItemInfo* item, CollisionInfo* coll, VaultTestSetup testSetup);
VaultTestResult TestLaraVault3Steps(ItemInfo* item, CollisionInfo* coll);
VaultTestResult TestLaraVault1StepToCrouch(ItemInfo* item, CollisionInfo* coll);
VaultTestResult TestLaraVault2StepsToCrouch(ItemInfo* item, CollisionInfo* coll);
VaultTestResult TestLaraVault3StepsToCrouch(ItemInfo* item, CollisionInfo* coll);
VaultTestResult TestLaraLedgeAutoJump(ItemInfo* item, CollisionInfo* coll);
VaultTestResult TestLaraLadderAutoJump(ItemInfo* item, CollisionInfo* coll);
VaultTestResult TestLaraLadderMount(ItemInfo* item, CollisionInfo* coll);
VaultTestResult TestLaraMonkeyAutoJump(ItemInfo* item, CollisionInfo* coll);
VaultTestResult TestLaraVault(ItemInfo* item, CollisionInfo* coll);
bool TestAndDoLaraLadderClimb(ItemInfo* item, CollisionInfo* coll);

CrawlVaultTestResult TestLaraCrawlVaultTolerance(ItemInfo* item, CollisionInfo* coll, CrawlVaultTestSetup testSetup);
CrawlVaultTestResult TestLaraCrawlUpStep(ItemInfo* item, CollisionInfo* coll);
CrawlVaultTestResult TestLaraCrawlDownStep(ItemInfo* item, CollisionInfo* coll);
CrawlVaultTestResult TestLaraCrawlExitDownStep(ItemInfo* item, CollisionInfo* coll);
CrawlVaultTestResult TestLaraCrawlExitJump(ItemInfo* item, CollisionInfo* coll);
CrawlVaultTestResult TestLaraCrawlVault(ItemInfo* item, CollisionInfo* coll);
bool TestLaraCrawlToHang(ItemInfo* item, CollisionInfo* coll);

WaterClimbOutTestResult TestLaraWaterClimbOutTolerance(ItemInfo* item, CollisionInfo* coll, WaterClimbOutTestSetup testSetup);
WaterClimbOutTestResult TestLaraWaterClimbOutDownStep(ItemInfo* item, CollisionInfo* coll);
WaterClimbOutTestResult TestLaraWaterClimbOutDownStepToCrouch(ItemInfo* item, CollisionInfo* coll);
WaterClimbOutTestResult TestLaraWaterClimbOutFlatStep(ItemInfo* item, CollisionInfo* coll);
WaterClimbOutTestResult TestLaraWaterClimbOutFlatStepToCrouch(ItemInfo* item, CollisionInfo* coll);
WaterClimbOutTestResult TestLaraWaterClimbOutUpStep(ItemInfo* item, CollisionInfo* coll);
WaterClimbOutTestResult TestLaraWaterClimbOutUpStepToCrouch(ItemInfo* item, CollisionInfo* coll);
WaterClimbOutTestResult TestLaraWaterClimbOut(ItemInfo* item, CollisionInfo* coll);

LedgeHangTestResult TestLaraLedgeHang(ItemInfo* item, CollisionInfo* coll);
bool TestLaraShimmyLeft(ItemInfo* item, CollisionInfo* coll);
bool TestLaraShimmyRight(ItemInfo* item, CollisionInfo* coll);
bool TestLaraLadderShimmyUp(ItemInfo* item, CollisionInfo* coll);
bool TestLaraLadderShimmyDown(ItemInfo* item, CollisionInfo* coll);
bool TestLaraHangClimbTolerance(ItemInfo* item, CollisionInfo* coll, HangClimbTestSetup testSetup);
bool TestLaraHangToCrouch(ItemInfo* item, CollisionInfo* coll);
bool TestLaraHangToStand(ItemInfo* item, CollisionInfo* coll);

bool TestLaraTightropeDismount(ItemInfo* item, CollisionInfo* coll);

bool TestLaraPoleCollision(ItemInfo* item, CollisionInfo* coll, bool isGoingUp, float offset = 0.0f);
bool TestLaraPoleUp(ItemInfo* item, CollisionInfo* coll);
bool TestLaraPoleDown(ItemInfo* item, CollisionInfo* coll);
