#pragma once
#include "items.h"
#include "collide.h"
#include "room.h"

namespace TEN::Entities::Doors
{
	void DoubleDoorCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
}