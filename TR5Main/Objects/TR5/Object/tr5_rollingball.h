#pragma once
#include "items.h"
#include "collision/collision.h"

void RollingBallCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void RollingBallControl(short itemNumber);
void InitialiseClassicRollingBall(short itemNum);
void ClassicRollingBallCollision(short itemNum, ITEM_INFO* lara, COLL_INFO* coll);
void ClassicRollingBallControl(short itemNum);