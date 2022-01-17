#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"

void InitialiseMineCart(short itemNum);
void MineCartCollision(short itemNum, ITEM_INFO* laraItem, COLL_INFO* coll);
int MineCartControl(void);
