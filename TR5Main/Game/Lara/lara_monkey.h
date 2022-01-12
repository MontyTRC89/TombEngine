#pragma once

struct ITEM_INFO;
struct COLL_INFO;

// -----------------------------
// MONKEY SWING
// Control & Collision Functions
// -----------------------------

void lara_as_monkey_idle(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_monkey_idle(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_monkeyswing(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_monkeyswing(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_monkeyr(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_monkeyr(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_monkeyl(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_monkeyl(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_monkey180(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_monkey180(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_hangturnr(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_hangturnl(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_hangturnlr(ITEM_INFO* item, COLL_INFO* coll);

short TestMonkeyRight(ITEM_INFO* item, COLL_INFO* coll);
short TestMonkeyLeft(ITEM_INFO* item, COLL_INFO* coll);
