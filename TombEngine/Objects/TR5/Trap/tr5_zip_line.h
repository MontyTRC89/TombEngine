#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"

void InitialiseZipLine(short itemNumber);
void ZipLineCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void ControlZipLine(short itemNumber);
