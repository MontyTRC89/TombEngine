#pragma once

#include "items.h"
#include "collide.h"

void TrainControl(short trainNum);
void TrainCollision(short trainNum, ITEM_INFO *larA, COLL_INFO *coll);
