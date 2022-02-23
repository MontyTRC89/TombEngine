#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"

void InitialiseMineCart(short itemNumber);
void MineCartCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
int MineCartControl(void);
