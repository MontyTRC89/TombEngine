#pragma once

#include "items.h"
#include "collision/collide_room.h"

void TrainControl(short trainNum);
void TrainCollision(short trainNum, ITEM_INFO *larA, COLL_INFO *coll);
