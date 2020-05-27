#pragma once
#include "types.h"

void InitialiseJeep(short itemNumber);
void JeepCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
int JeepControl(void);