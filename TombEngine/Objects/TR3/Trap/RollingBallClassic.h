#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"

void InitializeClassicRollingBall(short itemNumber);
void ClassicRollingBallCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void ClassicRollingBallControl(short itemNumber);
