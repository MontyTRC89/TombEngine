#pragma once
#include "types.h"

void InitialiseSkidoo(short itemNum);
void SkidooCollision(short itemNum, ITEM_INFO* litem, COLL_INFO* coll);
int SkidooControl(void);
void DrawSkidoo(ITEM_INFO* item);