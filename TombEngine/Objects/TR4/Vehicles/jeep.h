#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"
#include "Objects/Utils/VehicleHelpers.h"

namespace TEN::Entities::Vehicles
{
	void InitialiseJeep(short itemNumber);

	void JeepPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoJeepMount(ItemInfo* jeepItem, ItemInfo* laraItem, VehicleMountType mountType);

	int JeepControl(ItemInfo* laraItem);
}
