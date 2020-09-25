#pragma once
#include "lara_struct.h"

// Auxiliary functions.
bool TestLaraKeepCrouched(ITEM_INFO* player, COLL_INFO* coll);
bool TestLaraCrawl(ITEM_INFO* player);
bool TestLaraCrouchTurn(ITEM_INFO* player);
bool TestLaraCrouchRoll(ITEM_INFO* player);
void SetLaraCrawlWallDeflect(ITEM_INFO* item, COLL_INFO* coll);

// Crouching control & collision functions.
void lara_as_crouch(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crouch(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crouch_roll(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crouch_roll(ITEM_INFO* item, COLL_INFO* coll);

// Crawling control & collision functions.
void lara_as_crawl_stop(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crawl_stop(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crawl_forward(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crawl_forward(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crawl_turn_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crawl_turn_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crawl_turn(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crawl_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crawl_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crouch_turn_left(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crouch_turn_right(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crouch_turn(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crawl_to_hang(ITEM_INFO* item, COLL_INFO* coll);
