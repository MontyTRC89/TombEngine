#pragma once
#include "items.h"
#include "collide.h"

enum class CLIMB_DIRECTION : short
{
	North = 0x0100,
	East  = 0x0200,
	South = 0x0400,
	West  = 0x0800
};

static CLIMB_DIRECTION LeftIntRightExtTab[4] =
{
	CLIMB_DIRECTION::West, CLIMB_DIRECTION::North, CLIMB_DIRECTION::East, CLIMB_DIRECTION::South
};

static CLIMB_DIRECTION LeftExtRightIntTab[4] =
{
	CLIMB_DIRECTION::East, CLIMB_DIRECTION::South, CLIMB_DIRECTION::West, CLIMB_DIRECTION::North
};

short GetClimbFlags(int x, int y, int z, short roomNumber);
short GetClimbFlags(FLOOR_INFO* floor);

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
void lara_as_stepoff_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_stepoff_right(ITEM_INFO* item, COLL_INFO* coll);

int LaraTestClimbPos(ITEM_INFO* item, int front, int right, int origin, int height, int* shift);
void LaraDoClimbLeftRight(ITEM_INFO* item, COLL_INFO* coll, int result, int shift);
int LaraClimbRightCornerTest(ITEM_INFO* item, COLL_INFO* coll);
int LaraClimbLeftCornerTest(ITEM_INFO* item, COLL_INFO* coll);
int LaraTestClimb(int x, int y, int z, int xFront, int zFront, int itemHeight, int itemRoom, int* shift);
int LaraTestClimbUpPos(ITEM_INFO* item, int front, int right, int* shift, int* ledge);
int LaraCheckForLetGo(ITEM_INFO* item, COLL_INFO* coll);
