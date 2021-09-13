#pragma once
#include "collide.h"

struct ITEM_INFO;
struct COLL_INFO;

bool TestLaraVault(ITEM_INFO* item, COLL_INFO* coll);
bool TestLaraStandUp(COLL_INFO* coll);
bool TestLaraSlide(ITEM_INFO* item, COLL_INFO* coll);
SPLAT_COLL TestWall(ITEM_INFO* item, int front, int right, int down);
bool LaraHangTest(ITEM_INFO* item, COLL_INFO* coll);
bool LaraHangLeftCornerTest(ITEM_INFO* item, COLL_INFO* coll);
bool LaraHangRightCornerTest(ITEM_INFO* item, COLL_INFO* coll);
bool LaraTestClimbStance(ITEM_INFO* item, COLL_INFO* coll);
bool LaraTestHangOnClimbWall(ITEM_INFO* item, COLL_INFO* coll);
int LaraTestEdgeCatch(ITEM_INFO* item, COLL_INFO* coll, int* edge);
bool IsValidHangPos(ITEM_INFO* item, COLL_INFO* coll);
bool TestHangSwingIn(ITEM_INFO* item, short angle);
bool TestHangFeet(ITEM_INFO* item, short angle);
bool CanLaraHangSideways(ITEM_INFO* item, COLL_INFO* coll, short angle);
void SetCornerAnim(ITEM_INFO* item, COLL_INFO* coll, short rot, short flip);
void SetCornerAnimFeet(ITEM_INFO* item, COLL_INFO* coll, short rot, short flip);
bool LaraFacingCorner(ITEM_INFO* item, short ang, int dist);
int LaraFloorFront(ITEM_INFO* item, short ang, int dist);
COLL_RESULT LaraCollisionFront(ITEM_INFO* item, short ang, int dist);
int LaraCeilingFront(ITEM_INFO* item, short ang, int dist, int h);
COLL_RESULT LaraCeilingCollisionFront(ITEM_INFO* item, short ang, int dist, int h);
bool LaraFallen(ITEM_INFO* item, COLL_INFO* coll);
bool LaraLandedBad(ITEM_INFO* l, COLL_INFO* coll);
#ifndef NEW_TIGHTROPE
void GetTighRopeFallOff(int Regularity);
#endif // !NEW_TIGHTROPE

bool TestLaraLean(ITEM_INFO* item, COLL_INFO* coll);
