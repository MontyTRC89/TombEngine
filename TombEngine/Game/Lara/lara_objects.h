#pragma once

struct ItemInfo;
struct CollisionInfo;

// -----------------------------------
// MISCELLANEOUS INTERACTABLE OBJECT
// Control & Collision Functions
// -----------------------------------

// Pickup
void lara_as_pickup(ItemInfo* item, CollisionInfo* coll);
void lara_as_pickup_flare(ItemInfo* item, CollisionInfo* coll);

// Switch
void lara_as_switch_on(ItemInfo* item, CollisionInfo* coll);
void lara_as_switch_off(ItemInfo* item, CollisionInfo* coll);
void lara_col_turn_switch(ItemInfo* item, CollisionInfo* coll);

// Receptacle
void lara_as_use_key(ItemInfo* item, CollisionInfo* coll);
void lara_as_use_puzzle(ItemInfo* item, CollisionInfo* coll);

// Pushable
void lara_as_pushable_push(ItemInfo* item, CollisionInfo* coll);
void lara_as_pushable_pull(ItemInfo* item, CollisionInfo* coll);
void lara_as_pushable_grab(ItemInfo* item, CollisionInfo* coll);

// Pulley
void lara_as_pulley(ItemInfo* item, CollisionInfo* coll);

// Horizontal bar
void lara_as_horizontal_bar_swing(ItemInfo* item, CollisionInfo* coll);
void lara_as_horizontal_bar_leap(ItemInfo* item, CollisionInfo* coll);

// Tightrope
#ifdef NEW_TIGHTROPE
void lara_as_tightrope_dismount(ItemInfo* item, CollisionInfo* coll);
#endif
void lara_as_tightrope_idle(ItemInfo* item, CollisionInfo* coll);
void lara_as_tightrope_walk(ItemInfo* item, CollisionInfo* coll);
void lara_as_tightrope_fall(ItemInfo* item, CollisionInfo* coll);

// Rope
void lara_as_rope_turn_clockwise(ItemInfo* item, CollisionInfo* coll);
void lara_as_rope_turn_counter_clockwise(ItemInfo* item, CollisionInfo* coll);
void lara_as_rope_idle(ItemInfo* item, CollisionInfo* coll);
void lara_col_rope_idle(ItemInfo* item, CollisionInfo* coll);
void lara_col_rope_swing(ItemInfo* item, CollisionInfo* coll);
void lara_as_rope_up(ItemInfo* item, CollisionInfo* coll);
void lara_as_rope_down(ItemInfo* item, CollisionInfo* coll);

// Ladder
void lara_as_ladder_idle(ItemInfo* item, CollisionInfo* coll);
void lara_col_ladder_idle(ItemInfo* item, CollisionInfo* coll);
void lara_as_ladder_up(ItemInfo* item, CollisionInfo* coll);
void lara_col_ladder_up(ItemInfo* item, CollisionInfo* coll);
void lara_as_ladder_down(ItemInfo* item, CollisionInfo* coll);
void lara_col_ladder_down(ItemInfo* item, CollisionInfo* coll);

// Vertical pole
void lara_as_pole_idle(ItemInfo* item, CollisionInfo* coll);
void lara_col_pole_idle(ItemInfo* item, CollisionInfo* coll);
void lara_as_pole_up(ItemInfo* item, CollisionInfo* coll);
void lara_col_pole_up(ItemInfo* item, CollisionInfo* coll);
void lara_as_pole_down(ItemInfo* item, CollisionInfo* coll);
void lara_col_pole_down(ItemInfo* item, CollisionInfo* coll);
void lara_as_pole_turn_clockwise(ItemInfo* item, CollisionInfo* coll);
void lara_col_pole_turn_clockwise(ItemInfo* item, CollisionInfo* coll);
void lara_as_pole_turn_counter_clockwise(ItemInfo* item, CollisionInfo* coll);
void lara_col_pole_turn_counter_clockwise(ItemInfo* item, CollisionInfo* coll);

// Zip-line
void lara_as_zip_line(ItemInfo* item, CollisionInfo* coll);
