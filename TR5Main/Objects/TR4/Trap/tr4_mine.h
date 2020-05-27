#pragma once
#include "types.h"

void InitialiseMine(short itemNum);
void MineControl(short itemNum);
void MineCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);