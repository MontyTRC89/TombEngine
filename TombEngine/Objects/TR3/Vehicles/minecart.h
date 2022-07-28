#pragma once
#include "Objects/Utils/VehicleHelpers.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	void InitialiseMinecart(short itemNumber);

	void MinecartPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoMinecartMount(ItemInfo* minecartItem, ItemInfo* laraItem, VehicleMountType mountType);
	
	bool MinecartControl(ItemInfo* laraItem);
}
