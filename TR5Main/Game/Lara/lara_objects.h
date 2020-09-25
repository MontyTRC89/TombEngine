#pragma once
#include "lara_struct.h"

// Pickup control functions.
void lara_as_pickup(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_pickup_flare(ITEM_INFO* item, COLL_INFO* coll);

// Switch control & collision functions.
void lara_as_switch(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_turnswitch(ITEM_INFO* item, COLL_INFO* coll);

// Puzzle & key control functions.
void lara_as_use_key(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_use_puzzle(ITEM_INFO* item, COLL_INFO* coll);

// Pushable state functions.
void lara_as_pushable_push(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_pushable_pull(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_pushable_ready(ITEM_INFO* item, COLL_INFO* coll);

// Pulley control function.
void lara_as_pulley(ITEM_INFO* item, COLL_INFO* coll);

// Swingbar control functions.
void lara_as_swing_bar(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_swing_bar_leap(ITEM_INFO* item, COLL_INFO* coll);

// Tightrope control functions.
void lara_as_tightrope_stop(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_tightrope_walk(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_tightrope_fall(ITEM_INFO* item, COLL_INFO* coll);

// Rope control & collision functions.
void lara_as_rope_turn_clockwise(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_rope_turn_counter_clockwise(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_rope(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_rope(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_rope_swing(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_rope_up(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_rope_down(ITEM_INFO* item, COLL_INFO* coll);

// Pole control & collision functions.
void lara_col_pole_stop(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_pole_up(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_pole_down(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_pole_turn_clockwise(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_pole_turn_counter_clockwise(ITEM_INFO* item, COLL_INFO* coll);

// Zipline control functions.
void lara_as_zipline(ITEM_INFO* item, COLL_INFO* coll);