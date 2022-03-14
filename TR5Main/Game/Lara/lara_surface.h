#pragma once

struct ITEM_INFO;
struct CollisionInfo;

void _cdecl lara_col_surftread(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_col_surfright(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_col_surfleft(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_col_surfback(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_col_surfswim(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_as_surftread(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_as_surfright(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_as_surfleft(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_as_surfback(ITEM_INFO* item, CollisionInfo* coll);
void _cdecl lara_as_surfswim(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_waterout(ITEM_INFO* item, CollisionInfo* coll);
