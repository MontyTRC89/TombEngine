#pragma once
#include "collide.h"

void lara_col_waterroll(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_uwdeath(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_dive(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_tread(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_glide(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_swim(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_waterroll(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_uwdeath(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_dive(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_tread(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_glide(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_swim(ITEM_INFO* item, COLL_INFO* coll);
void UpdateSubsuitAngles();
void SwimTurnSubsuit(ITEM_INFO* item);
void SwimTurn(ITEM_INFO* item);
void SwimDive(ITEM_INFO* item);
void LaraWaterCurrent(COLL_INFO* coll);