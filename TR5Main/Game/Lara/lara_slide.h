#pragma once

struct ITEM_INFO;
struct CollisionInfo;

// -----------------------------
// SLIDE
// Control & Collision Functions
// -----------------------------

void lara_as_slide_forward(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_slide_forward(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_slide_back(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_slide_back(ITEM_INFO* item, CollisionInfo* coll);
