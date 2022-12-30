#pragma once

#include "Game/items.h"
#include "Game/collision/collide_room.h"

void InitialiseCleaner(short itemNumber);
void CleanerControl(short itemNumber);
void CleanerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
