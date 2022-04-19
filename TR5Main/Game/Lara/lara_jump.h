#pragma once

struct ITEM_INFO;
struct COLL_INFO;

// -----------------------------
// JUMP
// Control & Collision Functions
// -----------------------------

void lara_as_forwardjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_forwardjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_fastfall(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_fastfall(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_reach(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_reach(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_land(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_compress(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_compress(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_backjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_backjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_rightjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_rightjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_leftjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_leftjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_upjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_upjump(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_fallback(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_fallback(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_swandive(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_swandive(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_fastdive(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_fastdive(ITEM_INFO* item, COLL_INFO* coll);
