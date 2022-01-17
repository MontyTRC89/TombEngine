#pragma once

#include "Game/items.h"
#include "Game/collision/collide_room.h"

void TrainControl(short trainNum);
void TrainCollision(short trainNum, ITEM_INFO *larA, COLL_INFO *coll);
