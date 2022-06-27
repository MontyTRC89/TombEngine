#pragma once
#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Objects/Utils/VehicleHelpers.h"

namespace TEN::Entities::Vehicles
{
	void InitialiseQuadBike(short itemNumber);

	void QuadBikePlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoQuadBikeMount(ItemInfo* quadBikeItem, ItemInfo* laraItem, VehicleMountType mountType);

	bool QuadBikeControl(ItemInfo* laraItem, CollisionInfo* coll);
}
