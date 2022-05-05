#pragma once

struct ITEM_INFO;
struct CollisionInfo;
struct CollisionResult;

// ------------------------------
// BASIC MOVEMENT & MISCELLANEOUS
// Control & Collision Functions
// ------------------------------

// --------------
// MISCELLANEOUS:
// --------------

void lara_void_func(ITEM_INFO* item, CollisionInfo* coll);
void lara_default_col(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_special(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_null(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_controlled(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_controlled_no_look(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_vault(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_auto_jump(ITEM_INFO* item, CollisionInfo* coll);

// ---------------
// BASIC MOVEMENT:
// ---------------

void lara_as_walk_forward(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_walk_forward(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_run_forward(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_run_forward(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_idle(ITEM_INFO* item, CollisionInfo* coll);
void PseudoLaraAsWadeIdle(ITEM_INFO* item, CollisionInfo* coll);
void PseudoLaraAsSwampIdle(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_idle(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_pose(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_run_back(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_run_back(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_turn_right_slow(ITEM_INFO* item, CollisionInfo* coll);
void PsuedoLaraAsSwampTurnRightSlow(ITEM_INFO* item, CollisionInfo* coll);
void PsuedoLaraAsWadeTurnRightSlow(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_turn_right_slow(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_turn_left_slow(ITEM_INFO* item, CollisionInfo* coll);
void PsuedoLaraAsWadeTurnLeftSlow(ITEM_INFO* item, CollisionInfo* coll);
void PsuedoLaraAsSwampTurnLeftSlow(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_turn_left_slow(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_death(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_death(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_splat(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_splat(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_walk_back(ITEM_INFO* item, CollisionInfo* coll);
void PseudoLaraAsSwampWalkBack(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_walk_back(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_turn_right_fast(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_turn_right_fast(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_turn_left_fast(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_turn_left_fast(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_step_right(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_step_right(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_step_left(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_step_left(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_roll_back(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_roll_back(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_roll_forward(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_roll_forward(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_wade_forward(ITEM_INFO* item, CollisionInfo* coll);
void PseudoLaraAsSwampWadeForward(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_wade_forward(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_sprint(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_sprint(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_sprint_dive(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_sprint_dive(ITEM_INFO* item, CollisionInfo* coll);
