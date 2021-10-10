#pragma once
#include "items.h"
#include "collide.h"

void SubInitialise(short itemNum);
void SubCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
int SubControl(void);
void SubEffects(short item_number);
