#pragma once
#include "lara_struct.h"

/*crouch/duck start*/
void lara_as_duck(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_duck(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crouch_roll(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crouch_roll(ITEM_INFO* item, COLL_INFO* coll);
/*crouch/duck end*/
/*-*/
/*crawl start*/
void lara_as_all4s(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_all4s(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crawl(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crawl(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_all4turnl(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_all4turnr(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_all4turnlr(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_crawlb(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_crawlb(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_duckl(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_duckr(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_ducklr(ITEM_INFO* item, COLL_INFO* coll);
/*crawl end*/
void lara_col_crawl2hang(ITEM_INFO* item, COLL_INFO* coll);