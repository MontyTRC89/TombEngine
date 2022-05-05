#pragma once

#include "Game/items.h"
#include "Game/collision/collide_room.h"

void InitialiseRubberBoat(short itemNumber);
void RubberBoatCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
void RubberBoatControl(short itemNumber);
void DrawRubberBoat(ITEM_INFO* rBoatItem);
