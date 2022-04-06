#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/collision//floordata.h"

static CLIMB_DIRECTION LeftIntRightExtTab[4] =
{
	CLIMB_DIRECTION::West, CLIMB_DIRECTION::North, CLIMB_DIRECTION::East, CLIMB_DIRECTION::South
};

static CLIMB_DIRECTION LeftExtRightIntTab[4] =
{
	CLIMB_DIRECTION::East, CLIMB_DIRECTION::South, CLIMB_DIRECTION::West, CLIMB_DIRECTION::North
};

// -----------------------------
// LADDER CLIMB
// Control & Collision Functions
// -----------------------------

void lara_col_climb_end(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_climb_end(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_climb_down(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_climb_down(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_climb_up(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_climb_up(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_climb_right(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_climb_right(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_climb_left(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_climb_left(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_climb_idle(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_climb_idle(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_climb_stepoff_left(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_climb_stepoff_right(ITEM_INFO* item, CollisionInfo* coll);

short GetClimbFlags(int x, int y, int z, short roomNumber);
short GetClimbFlags(FLOOR_INFO* floor);
CLIMB_DIRECTION GetClimbDirection(short angle);

int LaraTestClimbPos(ITEM_INFO* item, int front, int right, int origin, int height, int* shift);
void LaraDoClimbLeftRight(ITEM_INFO* item, CollisionInfo* coll, int result, int shift);
int LaraClimbRightCornerTest(ITEM_INFO* item, CollisionInfo* coll);
int LaraClimbLeftCornerTest(ITEM_INFO* item, CollisionInfo* coll);
int LaraTestClimb(ITEM_INFO* item, int x, int y, int z, int xFront, int zFront, int itemHeight, int itemRoom, int* shift);
int LaraTestClimbUpPos(ITEM_INFO* item, int front, int right, int* shift, int* ledge);
bool LaraCheckForLetGo(ITEM_INFO* item, CollisionInfo* coll);
