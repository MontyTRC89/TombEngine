#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"

namespace TEN::Entities::Vehicles
{
	void InitialiseJeep(short itemNumber);
	void JeepCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	int JeepControl(ItemInfo* laraItem);
}
