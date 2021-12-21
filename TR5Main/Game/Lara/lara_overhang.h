#pragma once
#include "trmath.h"

constexpr short FACING_NORTH = 0;
constexpr short FACING_EAST = 16384;
constexpr short FACING_SOUTH = -32768;
constexpr short FACING_WEST = -16384;

constexpr auto HEIGHT_ADJUST = 20 + CLICK(2);

void lara_col_slopeclimb(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_slopeclimb(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_slopefall(ITEM_INFO* lara, COLL_INFO* coll);
void lara_col_slopehang(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_slopehang(ITEM_INFO* lara, COLL_INFO* coll);
void lara_col_slopeshimmy(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_slopeshimmy(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_slopeclimbup(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_slopeclimbdown(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_sclimbstart(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_sclimbstop(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_sclimbend(ITEM_INFO* lara, COLL_INFO* coll);

void hang_slope_extra(ITEM_INFO* lara, COLL_INFO* col);
void reach_slope_extra(ITEM_INFO* lara, COLL_INFO* col);
void climb_slope_extra(ITEM_INFO* lara, COLL_INFO* coll);
void climbdown_slope_extra(ITEM_INFO* lara, COLL_INFO* coll);
void monkey_slope_extra(ITEM_INFO* lara, COLL_INFO* coll);
