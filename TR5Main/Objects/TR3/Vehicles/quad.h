#pragma once

struct ITEM_INFO;
struct CollisionInfo;

void InitialiseQuadBike(short itemNumber);
void QuadBikeCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
bool QuadBikeControl(ITEM_INFO* laraItem, CollisionInfo* coll);
