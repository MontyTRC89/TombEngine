#pragma once
#include "lara_struct.h"

/*normal hanging and shimmying*/
void lara_as_hang(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_hang(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_hangleft(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_hangleft(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_hangright(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_hangright(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_gymnast(ITEM_INFO* item, COLL_INFO* coll);
/*go around corners*/
void lara_as_extcornerl(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_extcornerr(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_intcornerl(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_intcornerr(ITEM_INFO* item, COLL_INFO* coll);
//feet hanging and shimmying
void lara_as_hang_feet(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_hang_feet(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_hang_feet_shimmyr(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_hang_feet_shimmyr(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_hang_feet_shimmyl(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_hang_feet_shimmyl(ITEM_INFO* item, COLL_INFO* coll);
//go around corners feet
void lara_as_hang_feet_inRcorner(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_hang_feet_inLcorner(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_hang_feet_outRcorner(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_hang_feet_outLcorner(ITEM_INFO* item, COLL_INFO* coll);
