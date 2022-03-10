#pragma once
#include "Game/Lara/lara_struct.h"

struct ITEM_INFO;
struct COLL_INFO;

// Hanging and shimmying
void lara_as_hang(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_hang(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_shimmy_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_shimmy_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_shimmy_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_shimmy_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_handstand(ITEM_INFO* item, COLL_INFO* coll);

// Go around corners
void lara_as_corner(ITEM_INFO* item, COLL_INFO* coll);
