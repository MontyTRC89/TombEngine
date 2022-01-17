#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"

void SubInitialise(short itemNum);
void SubCollision(short itemNum, ITEM_INFO* laraItem, COLL_INFO* coll);
void SubEffects(short itemNum);
bool SubControl(ITEM_INFO* laraItem, COLL_INFO* coll);
