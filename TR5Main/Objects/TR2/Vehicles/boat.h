#pragma once
#include "items.h"
#include "collide.h"

void InitialiseBoat(short itemNum);
void BoatCollision(short itemNum, ITEM_INFO* litem, COLL_INFO* coll);
void BoatControl(short itemNumber);
void DoBoatWakeEffect(ITEM_INFO* boat);
void GetBoatGetOff(ITEM_INFO* boat);
bool CanGetOff(int direction);