#pragma once
#include "lara_struct.h"

// -----------------------------
// CRAWL & CROUCH
// Control & Collision Functions
// -----------------------------

// -------
// CROUCH:
// -------

void lara_as_crouch_idle(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crouch_idle(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crouch_roll(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crouch_roll(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crouch_turn_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crouch_turn_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crouch_turn_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crouch_turn_right(ITEM_INFO* item, COLL_INFO* coll);

// ------
// CRAWL:
// ------

void lara_as_crawl_idle(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crawl_idle(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crawl_forward(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crawl_forward(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crawl_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crawl_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crawl_turn_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crawl_turn_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crawl_turn_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crawl_turn_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crawl_to_hang(ITEM_INFO* item, COLL_INFO* coll);
