#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"



void InitialiseMotorbike(short itemNumber);
void MotorbikeCollision(short itemNumber, ITEM_INFO* laraitem, CollisionInfo* coll);
int MotorbikeControl(void);
void DrawMotorbike(ITEM_INFO* item);
void DrawMotorbikeEffect(ITEM_INFO* item);