#pragma once
#include "items.h"
#include "collision/collide_room.h"



void InitialiseMotorbike(short itemNumber);
void MotorbikeCollision(short itemNumber, ITEM_INFO* laraitem, COLL_INFO* coll);
int MotorbikeControl(void);
void DrawMotorbike(ITEM_INFO* item);
void DrawMotorbikeEffect(ITEM_INFO* item);