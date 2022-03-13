#pragma once

struct ITEM_INFO;
struct CollisionInfo;

void InitialiseMineCart(short itemNumber);
void MineCartCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
bool MineCartControl(ITEM_INFO* laraItem);
