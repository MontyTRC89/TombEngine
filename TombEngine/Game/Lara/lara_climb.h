#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/collision//floordata.h"

static ClimbDirection LeftIntRightExtTab[4] =
{
	ClimbDirection::West, ClimbDirection::North, ClimbDirection::East, ClimbDirection::South
};

static ClimbDirection LeftExtRightIntTab[4] =
{
	ClimbDirection::East, ClimbDirection::South, ClimbDirection::West, ClimbDirection::North
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
ClimbDirection GetClimbDirection(short angle);

int LaraTestClimbPos(ItemInfo* item, int front, int right, int origin, int height, int* shift);
void LaraDoClimbLeftRight(ItemInfo* item, CollisionInfo* coll, int result, int shift);
int LaraClimbRightCornerTest(ItemInfo* item, CollisionInfo* coll);
int LaraClimbLeftCornerTest(ItemInfo* item, CollisionInfo* coll);
int LaraTestClimb(ItemInfo* item, int x, int y, int z, int xFront, int zFront, int itemHeight, int* shift);
int LaraTestClimbUpPos(ItemInfo* item, int front, int right, int* shift, int* ledge);
bool LaraCheckForLetGo(ItemInfo* item, CollisionInfo* coll);
