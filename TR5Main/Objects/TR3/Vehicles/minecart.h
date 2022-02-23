#pragma once

struct ITEM_INFO;
struct COLL_INFO;

void InitialiseMineCart(short itemNumber);
void MineCartCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
bool MineCartControl(ITEM_INFO* laraItem);
