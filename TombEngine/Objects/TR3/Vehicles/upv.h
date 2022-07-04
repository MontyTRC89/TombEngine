#pragma once
#include "Objects/Utils/VehicleHelpers.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	void UPVInitialise(short itemNumber);

	void UPVPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoUPVMount(ItemInfo* UPVItem, ItemInfo* laraItem, VehicleMountType mountType);

	void UPVEffects(short itemNumber);
	bool UPVControl(ItemInfo* laraItem, CollisionInfo* coll);
}
