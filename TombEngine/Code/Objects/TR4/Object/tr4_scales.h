#pragma once

struct CollisionInfo;
struct ItemInfo;

void ScalesControl(short itemNumber);
void ScalesCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
