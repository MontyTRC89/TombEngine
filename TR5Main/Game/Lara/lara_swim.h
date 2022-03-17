#pragma once
#include "Game/collision/collide_room.h"

void lara_as_underwater_roll_180(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_underwater_roll_180(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_underwater_death(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_underwater_death(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_underwater_dive(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_underwater_dive(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_underwater_idle(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_underwater_idle(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_underwater_glide(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_underwater_glide(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_underwater_swim_forward(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_underwater_swim_forward(ITEM_INFO* item, CollisionInfo* coll);

void UpdateSubsuitAngles(ITEM_INFO* item);
void SwimTurnSubsuit(ITEM_INFO* item);
void SwimTurn(ITEM_INFO* item, CollisionInfo* coll);
void SwimDive(ITEM_INFO* item);
void LaraWaterCurrent(ITEM_INFO* item, CollisionInfo* coll);
