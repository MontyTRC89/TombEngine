#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"

void UPVInitialise(short itemNumber);
void UPVCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void UPVEffects(short itemNumber);
bool UPVControl(ItemInfo* laraItem, CollisionInfo* coll);
