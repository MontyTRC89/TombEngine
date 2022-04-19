#pragma once

struct ITEM_INFO;
struct CollisionInfo;

// -----------------------------------
// MISCELLANEOUS INTERACTABLE OBJECT
// Control & Collision Functions
// -----------------------------------

// ------
// PICKUP
// ------

void lara_as_pickup(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_pickup_flare(ITEM_INFO* item, CollisionInfo* coll);

// ------
// SWITCH
// ------

void lara_as_switch_on(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_switch_off(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_turn_switch(ITEM_INFO* item, CollisionInfo* coll);

// ----------
// RECEPTACLE
// ----------

void lara_as_use_key(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_use_puzzle(ITEM_INFO* item, CollisionInfo* coll);

// --------
// PUSHABLE
// --------

void lara_as_pushable_push(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_pushable_pull(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_pushable_grab(ITEM_INFO* item, CollisionInfo* coll);

// ------
// PULLEY
// ------

void lara_as_pulley(ITEM_INFO* item, CollisionInfo* coll);

// --------------
// HORIZONTAL BAR
// --------------

void lara_as_horizontal_bar_swing(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_horizontal_bar_leap(ITEM_INFO* item, CollisionInfo* coll);

// ---------
// TIGHTROPE
// ---------

#ifdef NEW_TIGHTROPE
void lara_as_tightrope_dismount(ITEM_INFO* item, CollisionInfo* coll);
#endif
void lara_as_tightrope_idle(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_tightrope_walk(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_tightrope_fall(ITEM_INFO* item, CollisionInfo* coll);

// ----
// ROPE
// ----

void lara_as_rope_turn_clockwise(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_rope_turn_counter_clockwise(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_rope_idle(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_rope_idle(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_rope_swing(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_rope_up(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_rope_down(ITEM_INFO* item, CollisionInfo* coll);

// -------------
// VERTICAL POLE
// -------------

void lara_as_pole_idle(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_pole_idle(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_pole_up(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_pole_up(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_pole_down(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_pole_down(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_pole_turn_clockwise(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_pole_turn_clockwise(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_pole_turn_counter_clockwise(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_pole_turn_counter_clockwise(ITEM_INFO* item, CollisionInfo* coll);

// --------
// ZIP-LINE
// --------

void lara_as_zip_line(ITEM_INFO* item, CollisionInfo* coll);
