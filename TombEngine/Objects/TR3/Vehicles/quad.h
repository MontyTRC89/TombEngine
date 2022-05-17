#pragma once

struct ItemInfo;
struct CollisionInfo;

void InitialiseQuadBike(short itemNumber);
void QuadBikeCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
bool QuadBikeControl(ItemInfo* laraItem, CollisionInfo* coll);
