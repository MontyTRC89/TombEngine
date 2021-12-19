#pragma once
#include "items.h"
#include "collision/collision.h"

void SubInitialise(short itemNum);
void SubCollision(short itemNum, ITEM_INFO* laraItem, COLL_INFO* coll);
void SubEffects(short itemNum);
bool SubControl(ITEM_INFO* laraItem, COLL_INFO* coll);
