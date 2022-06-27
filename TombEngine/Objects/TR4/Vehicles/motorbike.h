#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"
#include "Objects/Utils/VehicleHelpers.h"

namespace TEN::Entities::Vehicles
{
	void InitialiseMotorbike(short itemNumber);

	void MotorbikePlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoMotorbikeMount(ItemInfo* motorbikeItem, ItemInfo* laraItem, VehicleMountType mountType);

	bool MotorbikeControl(ItemInfo* laraItem, CollisionInfo* coll);
}
