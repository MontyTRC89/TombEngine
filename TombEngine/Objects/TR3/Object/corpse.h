#pragma once

#include "Game/items.h"
#include "Game/collision/collide_room.h"

void TrainControl(short itemNumber);
void TrainCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
