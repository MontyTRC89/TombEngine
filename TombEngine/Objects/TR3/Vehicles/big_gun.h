#pragma once
#include "Objects/Utils/VehicleHelpers.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	void BigGunInitialise(short itemNumber);
	static bool BigGunTestMount(ItemInfo* bigGunItem, ItemInfo* laraItem);
	void BigGunFire(ItemInfo* bigGunItem, ItemInfo* laraItem);
	void BigGunCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	bool BigGunControl(ItemInfo* laraItem, CollisionInfo* coll);
}
