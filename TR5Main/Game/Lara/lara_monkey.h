#pragma once

struct ITEM_INFO;
struct COLL_INFO;

// -----------------------------
// MONKEY SWING
// Control & Collision Functions
// -----------------------------

void lara_as_monkey_idle(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_monkey_idle(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_monkey_forward(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_monkey_forward(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_monkey_shimmy_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_monkey_shimmy_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_monkey_shimmy_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_monkey_shimmy_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_monkey_turn_180(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_monkey_turn_180(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_monkey_turn_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_monkey_turn_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_monkey_turn_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_monkey_turn_right(ITEM_INFO* item, COLL_INFO* coll);

short TestMonkeyRight(ITEM_INFO* item, COLL_INFO* coll);
short TestMonkeyLeft(ITEM_INFO* item, COLL_INFO* coll);
