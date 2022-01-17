#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"

void InitialiseSarcophagus(short itemNum);
void SarcophagusCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);