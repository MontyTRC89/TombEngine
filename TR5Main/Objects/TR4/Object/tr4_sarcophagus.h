#pragma once
#include "items.h"
#include "collision/collision.h"

void InitialiseSarcophagus(short itemNum);
void SarcophagusCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);