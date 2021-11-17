#pragma once

#include  "items.h"
#include "collide.h"

void InitialiseRubberBoat(short itemNum);
void RubberBoatCollision(short itemNum, ITEM_INFO *lara, COLL_INFO *coll);
void RubberBoatControl(short itemNum);
void DrawRubberBoat(ITEM_INFO *item);
