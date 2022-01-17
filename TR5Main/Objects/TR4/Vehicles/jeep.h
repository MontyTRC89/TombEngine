#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"

void InitialiseJeep(short itemNumber);
void JeepCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
int JeepControl(void);