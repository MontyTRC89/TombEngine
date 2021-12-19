#pragma once
#include "items.h"
#include "collision/collide_room.h"

void InitialiseSarcophagus(short itemNum);
void SarcophagusCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);