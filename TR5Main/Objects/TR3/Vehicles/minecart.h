#pragma once
#include "items.h"
#include "collide.h"



void InitialiseMineCart(short itemNum);
void MineCartCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
int MineCartControl(void);