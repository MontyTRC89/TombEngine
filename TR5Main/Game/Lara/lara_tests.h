#pragma once
#include "Game/collision/collide_room.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara_test_structs.h"

struct ITEM_INFO;
struct CollisionInfo;

// -----------------------------
// TEST FUNCTIONS
// For State Control & Collision
// -----------------------------

bool TestValidLedge(ITEM_INFO* item, CollisionInfo* coll, bool ignoreHeadroom = false, bool heightLimit = false);
bool TestValidLedgeAngle(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraKeepLow(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraSlide(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraHang(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraHangJump(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraHangJumpUp(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraClimbStance(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraHangOnClimbWall(ITEM_INFO* item, CollisionInfo* coll);
int  TestLaraEdgeCatch(ITEM_INFO* item, CollisionInfo* coll, int* edge);
bool TestLaraValidHangPos(ITEM_INFO* item, CollisionInfo* coll);
CornerResult TestLaraHangCorner(ITEM_INFO* item, CollisionInfo* coll, float testAngle);
bool TestHangSwingIn(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraHangSideways(ITEM_INFO* item, CollisionInfo* coll, short angle);
bool LaraPositionOnLOS(ITEM_INFO* item, short angle, int distance);
bool TestLaraFacingCorner(ITEM_INFO* item, short angle, int distance);

int LaraFloorFront(ITEM_INFO* item, short angle, int distance);
int LaraCeilingFront(ITEM_INFO* item, short angle, int distance, int height);
CollisionResult LaraCollisionFront(ITEM_INFO* item, short angle, int distance);
CollisionResult LaraCeilingCollisionFront(ITEM_INFO* item, short angle, int distance, int height);
CollisionResult LaraCollisionAboveFront(ITEM_INFO* item, short angle, int distance, int height);

bool TestLaraFall(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraMonkeyGrab(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraMonkeyFall(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraLand(ITEM_INFO* item, CollisionInfo* coll);

bool TestLaraWaterStepOut(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraWaterClimbOut(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraLadderClimbOut(ITEM_INFO* item, CollisionInfo* coll);
void TestLaraWaterDepth(ITEM_INFO* item, CollisionInfo* coll);

#ifndef NEW_TIGHTROPE
void GetTightropeFallOff(ITEM_INFO* item, int regularity);
#endif

bool IsStandingWeapon(LaraWeaponType gunType);
bool IsJumpState(LaraState state);
bool IsRunJumpQueueableState(LaraState state);
bool IsRunJumpCountableState(LaraState state);
bool IsVaultState(LaraState state);

bool TestLaraSplat(ITEM_INFO* item, int distance, int height, int side = 0);
bool TestLaraPose(ITEM_INFO* item, CollisionInfo* coll);

bool TestLaraStep(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraStepUp(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraStepDown(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraMonkeyStep(ITEM_INFO* item, CollisionInfo* coll);

bool TestLaraMoveTolerance(ITEM_INFO* item, CollisionInfo* coll, MoveTestSetup testSetup, bool useCrawlSetup = false);
bool TestLaraRunForward(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraWalkForward(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraWalkBack(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraRunBack(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraStepLeft(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraStepRight(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraWadeForwardSwamp(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraWalkBackSwamp(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraStepLeftSwamp(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraStepRightSwamp(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraCrawlForward(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraCrawlBack(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraCrouchRoll(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraCrouchToCrawl(ITEM_INFO* item);

bool TestLaraMonkeyMoveTolerance(ITEM_INFO* item, CollisionInfo* coll, MonkeyMoveTestSetup testSetup);
bool TestLaraMonkeyForward(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraMonkeyBack(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraMonkeyShimmyLeft(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraMonkeyShimmyRight(ITEM_INFO* item, CollisionInfo* coll);

VaultTestResult TestLaraVaultTolerance(ITEM_INFO* item, CollisionInfo* coll, VaultTestSetup testSetup);
VaultTestResult TestLaraVault2Steps(ITEM_INFO* item, CollisionInfo* coll);
VaultTestResult TestLaraVault3Steps(ITEM_INFO* item, CollisionInfo* coll);
VaultTestResult TestLaraVault1StepToCrouch(ITEM_INFO* item, CollisionInfo* coll);
VaultTestResult TestLaraVault2StepsToCrouch(ITEM_INFO* item, CollisionInfo* coll);
VaultTestResult TestLaraVault3StepsToCrouch(ITEM_INFO* item, CollisionInfo* coll);
VaultTestResult TestLaraLedgeAutoJump(ITEM_INFO* item, CollisionInfo* coll);
VaultTestResult TestLaraLadderAutoJump(ITEM_INFO* item, CollisionInfo* coll);
VaultTestResult TestLaraLadderMount(ITEM_INFO* item, CollisionInfo* coll);
VaultTestResult TestLaraMonkeyAutoJump(ITEM_INFO* item, CollisionInfo* coll);
VaultTestResult TestLaraVault(ITEM_INFO* item, CollisionInfo* coll);
bool TestAndDoLaraLadderClimb(ITEM_INFO* item, CollisionInfo* coll);

CrawlVaultTestResult TestLaraCrawlVaultTolerance(ITEM_INFO* item, CollisionInfo* coll, CrawlVaultTestSetup testSetup);
CrawlVaultTestResult TestLaraCrawlUpStep(ITEM_INFO* item, CollisionInfo* coll);
CrawlVaultTestResult TestLaraCrawlDownStep(ITEM_INFO* item, CollisionInfo* coll);
CrawlVaultTestResult TestLaraCrawlExitDownStep(ITEM_INFO* item, CollisionInfo* coll);
CrawlVaultTestResult TestLaraCrawlExitJump(ITEM_INFO* item, CollisionInfo* coll);
CrawlVaultTestResult TestLaraCrawlVault(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraCrawlToHang(ITEM_INFO* item, CollisionInfo* coll);

bool TestLaraJumpTolerance(ITEM_INFO* item, CollisionInfo* coll, JumpTestSetup testSetup);
bool TestLaraRunJumpForward(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraJumpForward(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraJumpBack(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraJumpLeft(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraJumpRight(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraJumpUp(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraSlideJump(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraCrawlspaceDive(ITEM_INFO* item, CollisionInfo* coll);

bool TestLaraTightropeDismount(ITEM_INFO* item, CollisionInfo* coll);

bool TestLaraPoleCollision(ITEM_INFO* item, CollisionInfo* coll, bool up, float offset = 0.0f);
bool TestLaraPoleUp(ITEM_INFO* item, CollisionInfo* coll);
bool TestLaraPoleDown(ITEM_INFO* item, CollisionInfo* coll);
