#pragma once

struct ITEM_INFO;
struct COLL_INFO;

void InitialiseQuadBike(short itemNumber);
void QuadBikeCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
bool QuadBikeControl(ITEM_INFO* laraItem, COLL_INFO* coll);
