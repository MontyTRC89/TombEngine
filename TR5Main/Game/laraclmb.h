#pragma once
#include "global.h"
#include "collide.h"

short GetClimbTrigger(int x, int y, int z, short roomNumber);
void lara_col_climbend(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_climbend(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_climbdown(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_climbdown(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_climbing(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_climbing(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_climbright(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_climbright(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_climbleft(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_climbleft(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_climbstnc(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_climbstnc(ITEM_INFO* item, COLL_INFO* coll);

int LaraTestClimbPos(ITEM_INFO* item, int front, int right, int origin, int height, int* shift);
void LaraDoClimbLeftRight(ITEM_INFO* item, COLL_INFO* coll, int result, int shift);
int LaraClimbRightCornerTest(ITEM_INFO* item, COLL_INFO* coll);
int LaraClimbLeftCornerTest(ITEM_INFO* item, COLL_INFO* coll);
int LaraTestClimb(int x, int y, int z, int xFront, int zFront, int itemHeight, int itemRoom, int* shift);
int LaraTestClimbUpPos(ITEM_INFO* item, int front, int right, int* shift, int* ledge);
int LaraCheckForLetGo(ITEM_INFO* item, COLL_INFO* coll);
