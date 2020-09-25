#pragma once
#include "lara_struct.h"

// Auxiliary functions.
bool TestLaraStepDown(COLL_INFO* coll);
bool TestLaraStepUp(COLL_INFO* coll);
void DoLaraStep(ITEM_INFO* item, COLL_INFO* coll);

// General control & collision functions.
void lara_void_func(ITEM_INFO* item, COLL_INFO* coll);
void lara_default_col(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_special(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_null(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_controlled(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_controlledl(ITEM_INFO* item, COLL_INFO* coll);

// Basic movement control & collision functions.
void lara_as_walk_forward(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_walk_forward(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_run(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_run(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_stop(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_stop(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_jump_forward(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_jump_forward(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_pose(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_hop_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_hop_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_turn_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_turn_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_turn_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_turn_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_death(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_death(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_freefall(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_freefall(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_reach(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_reach(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_splat(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_splat(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_jump_prepare(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_jump_prepare(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_walk_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_walk_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_turn_right_fast(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_turn_right_fast(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_turn_left_fast(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_turn_left_fast(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_step_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_step_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_step_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_step_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_roll2(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_jump_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_jump_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_jump_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_jump_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_jump_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_jump_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_jump(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_jump_up(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_jump_up(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_fall_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_fall_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_roll(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_swandive_start(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_swandive_start(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_swandive_freefall(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_swandive_freefall(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_gymnast(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_wade(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_wade(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_sprint(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_sprint(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_sprint_roll(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_sprint_roll(ITEM_INFO* item, COLL_INFO* coll);
