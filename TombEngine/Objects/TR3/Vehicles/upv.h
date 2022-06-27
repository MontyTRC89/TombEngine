#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"
#include "Objects/Utils/VehicleHelpers.h"

namespace TEN::Entities::Vehicles
{
	void UPVInitialise(short itemNumber);

	void UPVPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoUPVMount(ItemInfo* UPVItem, ItemInfo* laraItem, VehicleMountType mountType);

	void UPVEffects(short itemNumber);
	bool UPVControl(ItemInfo* laraItem, CollisionInfo* coll);
}
