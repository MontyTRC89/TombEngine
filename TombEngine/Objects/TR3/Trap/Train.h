#pragma once

struct ItemInfo;
struct CollisionInfo;

void TrainControl(short itemNumber);
void TrainCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
