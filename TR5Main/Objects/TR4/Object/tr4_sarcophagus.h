#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"

void InitialiseSarcophagus(short itemNumber);
void SarcophagusCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
