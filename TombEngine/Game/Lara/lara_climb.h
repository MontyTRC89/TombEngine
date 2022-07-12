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

void lara_col_climb_end(ItemInfo* item, CollisionInfo* coll);
void lara_as_climb_end(ItemInfo* item, CollisionInfo* coll);
void lara_col_climb_down(ItemInfo* item, CollisionInfo* coll);
void lara_as_climb_down(ItemInfo* item, CollisionInfo* coll);
void lara_col_climb_up(ItemInfo* item, CollisionInfo* coll);
void lara_as_climb_up(ItemInfo* item, CollisionInfo* coll);
void lara_col_climb_right(ItemInfo* item, CollisionInfo* coll);
void lara_as_climb_right(ItemInfo* item, CollisionInfo* coll);
void lara_col_climb_left(ItemInfo* item, CollisionInfo* coll);
void lara_as_climb_left(ItemInfo* item, CollisionInfo* coll);
void lara_col_climb_idle(ItemInfo* item, CollisionInfo* coll);
void lara_as_climb_idle(ItemInfo* item, CollisionInfo* coll);
void lara_as_climb_stepoff_left(ItemInfo* item, CollisionInfo* coll);
void lara_as_climb_stepoff_right(ItemInfo* item, CollisionInfo* coll);

short GetClimbFlags(int x, int y, int z, short roomNumber);
short GetClimbFlags(FloorInfo* floor);
CLIMB_DIRECTION GetClimbDirection(short angle);

int LaraTestClimbPos(ItemInfo* item, int front, int right, int origin, int height, int* shift);
void LaraDoClimbLeftRight(ItemInfo* item, CollisionInfo* coll, int result, int shift);
int LaraClimbRightCornerTest(ItemInfo* item, CollisionInfo* coll);
int LaraClimbLeftCornerTest(ItemInfo* item, CollisionInfo* coll);
int LaraTestClimb(ItemInfo* item, int x, int y, int z, int xFront, int zFront, int itemHeight, int* shift);
int LaraTestClimbUpPos(ItemInfo* item, int front, int right, int* shift, int* ledge);
bool LaraCheckForLetGo(ItemInfo* item, CollisionInfo* coll);
