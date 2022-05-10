#pragma once

struct ItemInfo;
struct CollisionInfo;

void InitialiseMineCart(short itemNumber);
void MineCartCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
bool MineCartControl(ItemInfo* laraItem);
