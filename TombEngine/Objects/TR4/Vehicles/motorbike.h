#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"

namespace TEN::Entities::Vehicles
{
	void InitialiseMotorbike(short itemNumber);
	void MotorbikeCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	bool MotorbikeControl(ItemInfo* laraItem, CollisionInfo* coll);
}
