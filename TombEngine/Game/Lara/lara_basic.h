#pragma once

struct ItemInfo;
struct CollisionInfo;
struct CollisionResult;

// ------------------------------
// BASIC MOVEMENT & MISCELLANEOUS
// Control & Collision Functions
// ------------------------------

// --------------
// MISCELLANEOUS:
// --------------

void lara_void_func(ItemInfo* item, CollisionInfo* coll);
void lara_default_col(ItemInfo* item, CollisionInfo* coll);
void lara_as_special(ItemInfo* item, CollisionInfo* coll);
void lara_as_null(ItemInfo* item, CollisionInfo* coll);
void lara_as_controlled(ItemInfo* item, CollisionInfo* coll);
void lara_as_controlled_no_look(ItemInfo* item, CollisionInfo* coll);
void lara_as_vault(ItemInfo* item, CollisionInfo* coll);
void lara_as_auto_jump(ItemInfo* item, CollisionInfo* coll);

// ---------------
// BASIC MOVEMENT:
// ---------------

void lara_as_walk_forward(ItemInfo* item, CollisionInfo* coll);
void lara_col_walk_forward(ItemInfo* item, CollisionInfo* coll);
void lara_as_run_forward(ItemInfo* item, CollisionInfo* coll);
void lara_col_run_forward(ItemInfo* item, CollisionInfo* coll);
void lara_as_idle(ItemInfo* item, CollisionInfo* coll);
void PseudoLaraAsWadeIdle(ItemInfo* item, CollisionInfo* coll);
void PseudoLaraAsSwampIdle(ItemInfo* item, CollisionInfo* coll);
void lara_col_idle(ItemInfo* item, CollisionInfo* coll);
void lara_as_pose(ItemInfo* item, CollisionInfo* coll);
void lara_as_run_back(ItemInfo* item, CollisionInfo* coll);
void lara_col_run_back(ItemInfo* item, CollisionInfo* coll);
void lara_as_turn_right_slow(ItemInfo* item, CollisionInfo* coll);
void PsuedoLaraAsSwampTurnRightSlow(ItemInfo* item, CollisionInfo* coll);
void PsuedoLaraAsWadeTurnRightSlow(ItemInfo* item, CollisionInfo* coll);
void lara_col_turn_right_slow(ItemInfo* item, CollisionInfo* coll);
void lara_as_turn_left_slow(ItemInfo* item, CollisionInfo* coll);
void PsuedoLaraAsWadeTurnLeftSlow(ItemInfo* item, CollisionInfo* coll);
void PsuedoLaraAsSwampTurnLeftSlow(ItemInfo* item, CollisionInfo* coll);
void lara_col_turn_left_slow(ItemInfo* item, CollisionInfo* coll);
void lara_as_death(ItemInfo* item, CollisionInfo* coll);
void lara_col_death(ItemInfo* item, CollisionInfo* coll);
void lara_as_splat(ItemInfo* item, CollisionInfo* coll);
void lara_col_splat(ItemInfo* item, CollisionInfo* coll);
void lara_as_walk_back(ItemInfo* item, CollisionInfo* coll);
void PseudoLaraAsSwampWalkBack(ItemInfo* item, CollisionInfo* coll);
void lara_col_walk_back(ItemInfo* item, CollisionInfo* coll);
void lara_as_turn_right_fast(ItemInfo* item, CollisionInfo* coll);
void lara_col_turn_right_fast(ItemInfo* item, CollisionInfo* coll);
void lara_as_turn_left_fast(ItemInfo* item, CollisionInfo* coll);
void lara_col_turn_left_fast(ItemInfo* item, CollisionInfo* coll);
void lara_as_step_right(ItemInfo* item, CollisionInfo* coll);
void lara_col_step_right(ItemInfo* item, CollisionInfo* coll);
void lara_as_step_left(ItemInfo* item, CollisionInfo* coll);
void lara_col_step_left(ItemInfo* item, CollisionInfo* coll);
void lara_as_roll_back(ItemInfo* item, CollisionInfo* coll);
void lara_col_roll_back(ItemInfo* item, CollisionInfo* coll);
void lara_as_roll_forward(ItemInfo* item, CollisionInfo* coll);
void lara_col_roll_forward(ItemInfo* item, CollisionInfo* coll);
void lara_as_wade_forward(ItemInfo* item, CollisionInfo* coll);
void PseudoLaraAsSwampWadeForward(ItemInfo* item, CollisionInfo* coll);
void lara_col_wade_forward(ItemInfo* item, CollisionInfo* coll);
void lara_as_sprint(ItemInfo* item, CollisionInfo* coll);
void lara_col_sprint(ItemInfo* item, CollisionInfo* coll);
void lara_as_sprint_dive(ItemInfo* item, CollisionInfo* coll);
void lara_col_sprint_dive(ItemInfo* item, CollisionInfo* coll);
