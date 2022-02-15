#pragma once
#include "Game/collision/collide_room.h"

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
void UpdateSubsuitAngles(ITEM_INFO* item);
void SwimTurnSubsuit(ITEM_INFO* item);
void SwimTurn(ITEM_INFO* item, COLL_INFO* coll);
void SwimDive(ITEM_INFO* item);
void LaraWaterCurrent(ITEM_INFO* item, COLL_INFO* coll);
