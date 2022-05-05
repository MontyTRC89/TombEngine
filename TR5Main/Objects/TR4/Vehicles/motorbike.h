#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"



void InitialiseMotorbike(short itemNumber);
void MotorbikeCollision(short itemNumber, ItemInfo* laraitem, CollisionInfo* coll);
int MotorbikeControl(void);
void DrawMotorbike(ItemInfo* item);
void DrawMotorbikeEffect(ItemInfo* item);