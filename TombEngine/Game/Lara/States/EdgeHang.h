#pragma once
#include "Game/Lara/lara_struct.h"

struct CollisionInfo;
struct ItemInfo;

void lara_as_hang_idle(ItemInfo* item, CollisionInfo* coll);
void lara_col_hang_idle(ItemInfo* item, CollisionInfo* coll);
void lara_as_shimmy_left(ItemInfo* item, CollisionInfo* coll);
void lara_col_shimmy_left(ItemInfo* item, CollisionInfo* coll);
void lara_as_shimmy_right(ItemInfo* item, CollisionInfo* coll);
void lara_col_shimmy_right(ItemInfo* item, CollisionInfo* coll);
void lara_as_shimmy_corner(ItemInfo* item, CollisionInfo* coll);
void lara_as_handstand(ItemInfo* item, CollisionInfo* coll);
