#pragma once

#include "..\Global\global.h"

void __cdecl LaraWaterCurrent(COLL_INFO* coll);
int __cdecl GetWaterDepth(int x, int y, int z, short roomNumber);
void __cdecl lara_col_waterroll(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_uwdeath(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_dive(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_tread(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_glide(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_col_swim(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_waterroll(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_uwdeath(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_dive(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_tread(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_glide(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_swim(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_swimcheat(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl LaraUnderWater(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl UpdateSubsuitAngles();
void __cdecl SwimTurnSubsuit(ITEM_INFO* item);
void __cdecl SwimTurn(ITEM_INFO* item);
void __cdecl LaraSwimCollision(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl LaraTestWaterDepth(ITEM_INFO* item, COLL_INFO* coll);