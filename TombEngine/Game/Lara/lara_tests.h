#pragma once
#include "Game/collision/collide_room.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara_test_structs.h"

struct ItemInfo;
struct CollisionInfo;

// -----------------------------
// TEST FUNCTIONS
// For State Control & Collision
// -----------------------------

bool TestValidLedge(ItemInfo* item, CollisionInfo* coll, bool ignoreHeadroom = false, bool heightLimit = false);
bool TestValidLedgeAngle(ItemInfo* item, CollisionInfo* coll);

bool TestLaraHang(ItemInfo* item, CollisionInfo* coll);
bool TestLaraHangJump(ItemInfo* item, CollisionInfo* coll);
bool TestLaraHangJumpUp(ItemInfo* item, CollisionInfo* coll);
int  TestLaraEdgeCatch(ItemInfo* item, CollisionInfo* coll, int* edge);

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
bool TestLaraWaterClimbOut(ItemInfo* item, CollisionInfo* coll);
bool TestLaraLadderClimbOut(ItemInfo* item, CollisionInfo* coll);
void TestLaraWaterDepth(ItemInfo* item, CollisionInfo* coll);

#ifndef NEW_TIGHTROPE
void GetTightropeFallOff(ItemInfo* item, int regularity);
#endif

bool CheckLaraState(LaraState state, std::vector<LaraState> stateList);
bool IsStandingWeapon(ItemInfo* item, LaraWeaponType weaponType);
bool IsVaultState(LaraState state);
bool IsJumpState(LaraState state);
bool IsRunJumpQueueableState(LaraState state);
bool IsRunJumpCountableState(LaraState state);

bool TestLaraPose(ItemInfo* item, CollisionInfo* coll);
bool TestLaraKeepLow(ItemInfo* item, CollisionInfo* coll);
bool TestLaraSlide(ItemInfo* item, CollisionInfo* coll);
bool TestLaraLand(ItemInfo* item, CollisionInfo* coll);
bool TestLaraFall(ItemInfo* item, CollisionInfo* coll);
bool TestLaraMonkeyGrab(ItemInfo* item, CollisionInfo* coll);
bool TestLaraMonkeyFall(ItemInfo* item, CollisionInfo* coll);

bool TestLaraStep(ItemInfo* item, CollisionInfo* coll);
bool TestLaraStepUp(ItemInfo* item, CollisionInfo* coll);
bool TestLaraStepDown(ItemInfo* item, CollisionInfo* coll);
bool TestLaraMonkeyStep(ItemInfo* item, CollisionInfo* coll);

bool TestLaraMoveTolerance(ItemInfo* item, CollisionInfo* coll, MoveTestSetup testSetup, bool useCrawlSetup = false);
bool TestLaraRunForward(ItemInfo* item, CollisionInfo* coll);
bool TestLaraWalkForward(ItemInfo* item, CollisionInfo* coll);
bool TestLaraWalkBack(ItemInfo* item, CollisionInfo* coll);
bool TestLaraRunBack(ItemInfo* item, CollisionInfo* coll);
bool TestLaraStepLeft(ItemInfo* item, CollisionInfo* coll);
bool TestLaraStepRight(ItemInfo* item, CollisionInfo* coll);
bool TestLaraWadeForwardSwamp(ItemInfo* item, CollisionInfo* coll);
bool TestLaraWalkBackSwamp(ItemInfo* item, CollisionInfo* coll);
bool TestLaraStepLeftSwamp(ItemInfo* item, CollisionInfo* coll);
bool TestLaraStepRightSwamp(ItemInfo* item, CollisionInfo* coll);
bool TestLaraCrawlForward(ItemInfo* item, CollisionInfo* coll);
bool TestLaraCrawlBack(ItemInfo* item, CollisionInfo* coll);
bool TestLaraCrouchRoll(ItemInfo* item, CollisionInfo* coll);
bool TestLaraCrouch(ItemInfo* item);
bool TestLaraCrouchToCrawl(ItemInfo* item);
bool TestLaraFastTurn(ItemInfo* item);

bool TestLaraMonkeyMoveTolerance(ItemInfo* item, CollisionInfo* coll, MonkeyMoveTestSetup testSetup);
bool TestLaraMonkeyForward(ItemInfo* item, CollisionInfo* coll);
bool TestLaraMonkeyBack(ItemInfo* item, CollisionInfo* coll);
bool TestLaraMonkeyShimmyLeft(ItemInfo* item, CollisionInfo* coll);
bool TestLaraMonkeyShimmyRight(ItemInfo* item, CollisionInfo* coll);

VaultTestResult TestLaraVaultTolerance(ItemInfo* item, CollisionInfo* coll, VaultTestSetup testSetup);
VaultTestResult TestLaraVault2Steps(ItemInfo* item, CollisionInfo* coll);
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

bool TestLaraJumpTolerance(ItemInfo* item, CollisionInfo* coll, JumpTestSetup testSetup);
bool TestLaraRunJumpForward(ItemInfo* item, CollisionInfo* coll);
bool TestLaraJumpForward(ItemInfo* item, CollisionInfo* coll);
bool TestLaraJumpBack(ItemInfo* item, CollisionInfo* coll);
bool TestLaraJumpLeft(ItemInfo* item, CollisionInfo* coll);
bool TestLaraJumpRight(ItemInfo* item, CollisionInfo* coll);
bool TestLaraJumpUp(ItemInfo* item, CollisionInfo* coll);
bool TestLaraSlideJump(ItemInfo* item, CollisionInfo* coll);
bool TestLaraCrawlspaceDive(ItemInfo* item, CollisionInfo* coll);

bool TestLaraTightropeDismount(ItemInfo* item, CollisionInfo* coll);

bool TestLaraPoleCollision(ItemInfo* item, CollisionInfo* coll, bool up, float offset = 0.0f);
bool TestLaraPoleUp(ItemInfo* item, CollisionInfo* coll);
bool TestLaraPoleDown(ItemInfo* item, CollisionInfo* coll);
