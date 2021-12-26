#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"

void DragonCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll);
void DragonControl(short backNum);
void InitialiseBartoli(short itemNum);
void BartoliControl(short itemNum);