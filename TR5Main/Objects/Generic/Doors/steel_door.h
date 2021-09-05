#pragma once
#include "items.h"
#include "collide.h"
#include "room.h"

namespace TEN::Entities::Doors
{
	void InitialiseSteelDoor(short itemNumber);
	void SteelDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
}