#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Objects/Utils/VehicleHelpers.h"

namespace TEN::Entities::Vehicles
{
	void InitialiseMinecart(short itemNumber);

	void MinecartPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoMinecartMount(ItemInfo* minecartItem, ItemInfo* laraItem, VehicleMountType mountType);
	
	bool MinecartControl(ItemInfo* laraItem);
}
