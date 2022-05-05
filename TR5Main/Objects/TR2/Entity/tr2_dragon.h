#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"

void DragonCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void DragonControl(short backNumber);
void InitialiseBartoli(short itemNumber);
void BartoliControl(short itemNumber);
