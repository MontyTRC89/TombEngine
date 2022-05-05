#pragma once
#include "Game/Lara/lara_struct.h"

struct ITEM_INFO;
struct CollisionInfo;

// -----------------------------------
// LEDGE HANG
// State Control & Collision Functions
// -----------------------------------

void lara_as_hang(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_hang(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_shimmy_left(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_shimmy_left(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_shimmy_right(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_shimmy_right(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_shimmy_corner(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_handstand(ITEM_INFO* item, CollisionInfo* coll);
