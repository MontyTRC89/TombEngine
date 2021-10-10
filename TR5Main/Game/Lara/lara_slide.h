#pragma once

struct ITEM_INFO;
struct COLL_INFO;

/*Tests and others*/
void lara_slide_slope(ITEM_INFO* item, COLL_INFO* coll);
void LaraSlideEdgeJump(ITEM_INFO* item, COLL_INFO* coll);
/*end tests and others*/
/*-*/
/*Lara state code*/
void lara_as_slide(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_slide(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_slideback(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_slideback(ITEM_INFO* item, COLL_INFO* coll);
/*end Lara state code*/
