#pragma once

struct ITEM_INFO;
struct CollisionInfo;

// -----------------------------
// JUMP
// Control & Collision Functions
// -----------------------------

void lara_as_jump_forward(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_jump_forward(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_freefall(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_freefall(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_reach(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_reach(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_land(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_jump_prepare(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_jump_prepare(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_jump_back(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_jump_back(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_jump_right(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_jump_right(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_jump_left(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_jump_left(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_jump_up(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_jump_up(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_fall_back(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_fall_back(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_swan_dive(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_swan_dive(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_freefall_dive(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_freefall_dive(ITEM_INFO* item, CollisionInfo* coll);
