#pragma once

struct ITEM_INFO;
struct COLL_INFO;

void InitialiseQuadBike(short itemNumber);
void QuadBikeCollision(short itemNumber, ITEM_INFO* lara, COLL_INFO* coll);
int QuadBikeControl(ITEM_INFO* lara, COLL_INFO* coll);
