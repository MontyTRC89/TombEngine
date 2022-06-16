#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"

void InitialiseMotorbike(short itemNumber);
void MotorbikeCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
bool MotorbikeControl(ItemInfo* laraItem, CollisionInfo* coll);
