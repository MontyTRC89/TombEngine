#pragma once
#include "types.h"

void InitialiseBoat(short itemNum);
void BoatCollision(short itemNum, ITEM_INFO* litem, COLL_INFO* coll);
void BoatControl(short itemNumber);