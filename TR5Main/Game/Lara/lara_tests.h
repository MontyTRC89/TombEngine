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
int TestHangFeet(ITEM_INFO* item, short angle);
int CanLaraHangSideways(ITEM_INFO* item, COLL_INFO* coll, short angle);
void SetCornerAnim(ITEM_INFO* item, COLL_INFO* coll, short rot, short flip);
void SetCornerAnimFeet(ITEM_INFO* item, COLL_INFO* coll, short rot, short flip);
short LaraFloorFront(ITEM_INFO* item, short ang, int dist);
short LaraCeilingFront(ITEM_INFO* item, short ang, int dist, int h);
int LaraLandedBad(ITEM_INFO* l, COLL_INFO* coll);
void GetTighropeFallOff(int Regularity);

bool TestLaraFall(COLL_INFO* coll);
void SetLaraFall(ITEM_INFO* item);

bool TestLaraLean(COLL_INFO* coll);
