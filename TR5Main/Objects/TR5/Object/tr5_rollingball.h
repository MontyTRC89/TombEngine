#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"

void RollingBallCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
void RollingBallControl(short itemNumber);
void InitialiseClassicRollingBall(short itemNumber);
void ClassicRollingBallCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
void ClassicRollingBallControl(short itemNumber);
