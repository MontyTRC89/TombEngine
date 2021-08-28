#pragma once

struct ITEM_INFO;
struct COLL_INFO;
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
int LaraFloorFront(ITEM_INFO* item, short ang, int dist);
int LaraCeilingFront(ITEM_INFO* item, short ang, int dist, int h);
int LaraFallen(ITEM_INFO* item, COLL_INFO* coll);
int LaraLandedBad(ITEM_INFO* l, COLL_INFO* coll);
void GetTighRopeFallOff(int Regularity);
bool TestLaraLean(ITEM_INFO* item, COLL_INFO* coll);
