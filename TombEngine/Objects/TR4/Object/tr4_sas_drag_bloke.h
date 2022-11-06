#pragma once

#include "Game/items.h"
#include "Game/collision/collide_room.h"

void InitialiseDragSAS(short itemNumber);
void DragSASCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);