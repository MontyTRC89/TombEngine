#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"

void UPVInitialise(short itemNumber);
void UPVCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
void UPVEffects(short itemNumber);
bool UPVControl(ITEM_INFO* laraItem, CollisionInfo* coll);
