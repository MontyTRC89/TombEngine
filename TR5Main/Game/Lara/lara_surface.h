#pragma once

struct ITEM_INFO;
struct CollisionInfo;

void _cdecl lara_as_surface_idle(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_col_surface_idle(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_as_surface_right(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_col_surface_right(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_as_surface_left(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_col_surface_left(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_as_surface_back(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_col_surface_back(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_as_surface_forward(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_col_surface_forward(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_surface_climb_out(ITEM_INFO* item, CollisionInfo* coll);
