#pragma once
#include "lara_struct.h"

int TestLaraVault(ITEM_INFO* item, COLL_INFO* coll);
int TestWall(ITEM_INFO* item, int front, int right, int down);
int LaraHangTest(ITEM_INFO* item, COLL_INFO* coll);
int LaraHangLeftCornerTest(ITEM_INFO* item, COLL_INFO* coll);
int LaraHangRightCornerTest(ITEM_INFO* item, COLL_INFO* coll);
int LaraTestClimbStance(ITEM_INFO* item, COLL_INFO* coll);
int LaraTestHangOnClimbWall(ITEM_INFO* item, COLL_INFO* coll);
int LaraTestEdgeCatch(ITEM_INFO* item, COLL_INFO* coll, int* edge);
int IsValidHangPos(ITEM_INFO* item, COLL_INFO* coll);
int TestHangSwingIn(ITEM_INFO* item, short angle);
bool TestHangFeet(ITEM_INFO* item, short angle);
int CanLaraHangSideways(ITEM_INFO* item, COLL_INFO* coll, short angle);
void SetCornerAnim(ITEM_INFO* item, COLL_INFO* coll, short rot, short flip);
void SetCornerAnimFeet(ITEM_INFO* item, COLL_INFO* coll, short rot, short flip);
bool LaraFacingCorner(ITEM_INFO* item, short ang, int dist);
int LaraFloorFront(ITEM_INFO* item, short ang, int dist);
COLL_RESULT LaraCollisionFront(ITEM_INFO* item, short ang, int dist);
int LaraCeilingFront(ITEM_INFO* item, short ang, int dist, int h);
COLL_RESULT LaraCeilingCollisionFront(ITEM_INFO* item, short ang, int dist, int h);
int LaraFallen(ITEM_INFO* item, COLL_INFO* coll);
int LaraLandedBad(ITEM_INFO* l, COLL_INFO* coll);
#ifndef NEW_TIGHTROPE
void GetTighRopeFallOff(int Regularity);
#endif // !NEW_TIGHTROPE

bool TestLaraLean(ITEM_INFO* item, COLL_INFO* coll);
