#pragma once
#include "Specific/trmath.h"

struct ITEM_INFO;
struct CollisionInfo;

void lara_col_slopeclimb(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_slopeclimb(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_slopefall(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_slopehang(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_slopehang(ITEM_INFO* item, CollisionInfo* coll);
void lara_col_slopeshimmy(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_slopeshimmy(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_slopeclimbup(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_slopeclimbdown(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_sclimbstart(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_sclimbstop(ITEM_INFO* item, CollisionInfo* coll);
void lara_as_sclimbend(ITEM_INFO* item, CollisionInfo* coll);

void SlopeHangExtra(ITEM_INFO* item, CollisionInfo* coll);
void SlopeReachExtra(ITEM_INFO* item, CollisionInfo* coll);
void SlopeClimbExtra(ITEM_INFO* item, CollisionInfo* coll);
void SlopeClimbDownExtra(ITEM_INFO* item, CollisionInfo* coll);
void SlopeMonkeyExtra(ITEM_INFO* item, CollisionInfo* coll);
bool LadderMonkeyExtra(ITEM_INFO* item, CollisionInfo* coll);
