#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"

void InitialiseSkidman(short itemNum);
void SkidManCollision(short itemNum, ITEM_INFO* laraitem, CollisionInfo* coll);
void SkidManControl(short riderNum);