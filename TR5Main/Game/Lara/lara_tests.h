#pragma once
#include "Game/collision/collide_room.h"
#include "Game/Lara/lara_struct.h"

struct ITEM_INFO;
struct COLL_INFO;

// -----------------------------
// TEST FUNCTIONS
// For State Control & Collision
// -----------------------------

SPLAT_COLL TestLaraWall(ITEM_INFO* item, int front, int right, int down);
bool TestValidLedge(ITEM_INFO* item, COLL_INFO* coll, bool ignoreHeadroom = false, bool heightLimit = false);
bool TestValidLedgeAngle(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraVault(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraKeepCrouched(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraSlide(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraSwamp(ITEM_INFO* item);
bool TestLaraWater(ITEM_INFO* item);
bool TestLaraHang(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraHangJump(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraHangJumpUp(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraClimbStance(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraHangOnClimbWall(ITEM_INFO* item, COLL_INFO* coll);
int  TestLaraEdgeCatch(ITEM_INFO* item, COLL_INFO* coll, int* edge);
bool TestLaraValidHangPos(ITEM_INFO* item, COLL_INFO* coll);
CORNER_RESULT TestLaraHangCorner(ITEM_INFO* item, COLL_INFO* coll, float testAngle);
bool TestHangSwingIn(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraHangSideways(ITEM_INFO* item, COLL_INFO* coll, short angle);
bool LaraPositionOnLOS(ITEM_INFO* item, short ang, int dist);
bool TestLaraFacingCorner(ITEM_INFO* item, short angle, int dist);
bool TestLaraStandingJump(ITEM_INFO* item, COLL_INFO* coll, short angle);

int  LaraFloorFront(ITEM_INFO* item, short ang, int dist);
int  LaraCeilingFront(ITEM_INFO* item, short ang, int dist, int h);
COLL_RESULT LaraCollisionFront(ITEM_INFO* item, short ang, int dist);
COLL_RESULT LaraCeilingCollisionFront(ITEM_INFO* item, short ang, int dist, int h);
COLL_RESULT LaraCollisionAboveFront(ITEM_INFO* item, short ang, int dist, int h);

bool TestLaraFall(ITEM_INFO* item, COLL_INFO* coll);
bool LaraFallen(ITEM_INFO* item, COLL_INFO* coll);
bool LaraLandedBad(ITEM_INFO* item, COLL_INFO* coll);

bool TestLaraWaterStepOut(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraWaterClimbOut(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraLadderClimbOut(ITEM_INFO* item, COLL_INFO* coll);
void TestLaraWaterDepth(ITEM_INFO* item, COLL_INFO* coll);

#ifndef NEW_TIGHTROPE
void GetTighRopeFallOff(int Regularity);
#endif

bool IsStandingWeapon(LARA_WEAPON_TYPE gunType);

bool TestLaraPose(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraStep(COLL_INFO* coll);
bool TestLaraStepUp(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraStepDown(ITEM_INFO* item, COLL_INFO* coll);

bool TestLaraMove(ITEM_INFO* item, COLL_INFO* coll, short angle, int lowerBound, int upperBound, bool checkSlope = true, bool checkDeath = true);
bool TestLaraMoveCrawl(ITEM_INFO* item, COLL_INFO* coll, short angle, int lowerBound, int upperBound, bool checkSlope = true, bool checkDeath = true);
bool TestLaraRunForward(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraWalkForward(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraWalkBack(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraHopBack(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraStepLeft(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraStepRight(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraWalkBackSwamp(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraStepLeftSwamp(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraStepRightSwamp(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraCrawlForward(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraCrawlBack(ITEM_INFO* item, COLL_INFO* coll);

bool TestLaraCrouchToCrawl(ITEM_INFO* item);
bool TestLaraCrouchRoll(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraCrawlUpStep(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraCrawlDownStep(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraCrawlExitDownStep(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraCrawlExitJump(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraCrawlVault(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraCrawlToHang(ITEM_INFO* item, COLL_INFO* coll);

bool TestLaraPoleCollision(ITEM_INFO* item, COLL_INFO* coll, bool up, float offset = 0.0f);
bool TestLaraPoleUp(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraPoleDown(ITEM_INFO* item, COLL_INFO* coll);
