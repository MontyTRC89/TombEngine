#pragma once
#include "lara_struct.h"

// Hanging and shimmying control & collision functions.
void lara_as_hang(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_hang(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_shimmy_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_shimmy_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_shimmy_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_shimmy_right(ITEM_INFO* item, COLL_INFO* coll);

// Shimmying around corners control & collision functions.
void lara_as_extcornerl(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_extcornerr(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_intcornerl(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_intcornerr(ITEM_INFO* item, COLL_INFO* coll);

// Hanging and shimmying by feet control & collision functions.
void lara_as_hang_feet(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_hang_feet(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_shimmy_feet_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_shimmy_feet_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_shimmy_feet_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_shimmy_feet_left(ITEM_INFO* item, COLL_INFO* coll);

// Shimmying by feet around corners control & collision functions. 
void lara_as_hang_feet_right_corner_inner(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_hang_feet_left_corner_inner(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_hang_feet_right_corner_outer(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_hang_feet_left_corner_outer(ITEM_INFO* item, COLL_INFO* coll);
