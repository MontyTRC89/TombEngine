#pragma once

struct ITEM_INFO;
struct CollisionInfo;

// -----------------------------
// WATER SURFACE TREAD
// Control & Collision Functions
// -----------------------------

void lara_as_surface_dive(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_surface_dive(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_as_surface_idle(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_col_surface_idle(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_as_surface_swim_forward(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_col_surface_swim_forward(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_as_surface_swim_left(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_col_surface_swim_left(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_as_surface_swim_right(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_col_surface_swim_right(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_as_surface_swim_back(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_col_surface_swim_back(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_surface_climb_out(ITEM_INFO* item, CollisionInfo* coll);
