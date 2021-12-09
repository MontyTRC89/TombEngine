#pragma once
#include "collide.h"

struct ITEM_INFO;
struct COLL_INFO;

SPLAT_COLL TestLaraWall(ITEM_INFO* item, int front, int right, int down);
bool TestValidLedge(ITEM_INFO* item, COLL_INFO* coll, bool ignoreHeadroom = false, bool heightLimit = false);
bool TestValidLedgeAngle(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraVault(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraStandUp(COLL_INFO* coll);
bool TestLaraSlide(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraSwamp(ITEM_INFO* item);
bool TestLaraWater(ITEM_INFO* item);
bool TestLaraLean(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraHang(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraHangJump(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraHangJumpUp(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraClimbStance(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraHangOnClimbWall(ITEM_INFO* item, COLL_INFO* coll);
int  TestLaraEdgeCatch(ITEM_INFO* item, COLL_INFO* coll, int* edge);
bool TestLaraValidHangPos(ITEM_INFO* item, COLL_INFO* coll);
CORNER_RESULT TestLaraHangCorner(ITEM_INFO* item, COLL_INFO* coll, float testAngle);
bool TestHangSwingIn(ITEM_INFO* item, short angle);
bool TestLaraHangSideways(ITEM_INFO* item, COLL_INFO* coll, short angle);
bool LaraFacingCorner(ITEM_INFO* item, short ang, int dist);
bool LaraPositionOnLOS(ITEM_INFO* item, short ang, int dist);

int  LaraFloorFront(ITEM_INFO* item, short ang, int dist);
int  LaraCeilingFront(ITEM_INFO* item, short ang, int dist, int h);
COLL_RESULT LaraCollisionFront(ITEM_INFO* item, short ang, int dist);
COLL_RESULT LaraCeilingCollisionFront(ITEM_INFO* item, short ang, int dist, int h);
COLL_RESULT LaraCollisionAboveFront(ITEM_INFO* item, short ang, int dist, int h);

bool LaraFallen(ITEM_INFO* item, COLL_INFO* coll);
bool LaraLandedBad(ITEM_INFO* item, COLL_INFO* coll);

bool TestLaraWaterStepOut(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraWaterClimbOut(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraLadderClimbOut(ITEM_INFO* item, COLL_INFO* coll);
void TestLaraWaterDepth(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraPoleCollision(ITEM_INFO* item, COLL_INFO* coll, bool up, float offset = 0.0f);

#ifndef NEW_TIGHTROPE
void GetTighRopeFallOff(int Regularity);
#endif // !NEW_TIGHTROPE
