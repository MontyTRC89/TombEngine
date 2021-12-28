#pragma once
#include "Specific/trmath.h"

struct ITEM_INFO;
struct COLL_INFO;

void lara_col_slopeclimb(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_slopeclimb(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_slopefall(ITEM_INFO* lara, COLL_INFO* coll);
void lara_col_slopehang(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_slopehang(ITEM_INFO* lara, COLL_INFO* coll);
void lara_col_slopeshimmy(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_slopeshimmy(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_slopeclimbup(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_slopeclimbdown(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_sclimbstart(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_sclimbstop(ITEM_INFO* lara, COLL_INFO* coll);
void lara_as_sclimbend(ITEM_INFO* lara, COLL_INFO* coll);

void SlopeHangExtra(ITEM_INFO* lara, COLL_INFO* coll);
void SlopeReachExtra(ITEM_INFO* lara, COLL_INFO* coll);
void SlopeClimbExtra(ITEM_INFO* lara, COLL_INFO* coll);
void SlopeClimbDownExtra(ITEM_INFO* lara, COLL_INFO* coll);
void SlopeMonkeyExtra(ITEM_INFO* lara, COLL_INFO* coll);
