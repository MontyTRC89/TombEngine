#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"

void InitialiseSkidooMan(short itemNumber);
void SkidooManCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void SkidooManControl(short riderNumber);
