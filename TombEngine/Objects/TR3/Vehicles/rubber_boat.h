#pragma once
#include "Objects/TR3/Vehicles/rubber_boat_info.h"
#include "Objects/Utils/VehicleHelpers.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	RubberBoatInfo& GetRubberBoatInfo(ItemInfo& rBoatItem);
	void InitialiseRubberBoat(short itemNumber);

	void RubberBoatPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void RubberBoatControl(short itemNumber);
	void RubberBoatUserControl(ItemInfo& rBoatItem, ItemInfo& laraItem);
	void RubberBoatAnimation(ItemInfo& rBoatItem, ItemInfo& laraItem, VehicleImpactDirection impactDirection);

	void DoRubberBoatMount(ItemInfo& rBoatItem, ItemInfo& laraItem, VehicleMountType mountType);
	bool TestRubberBoatDismount(ItemInfo& laraItem, int direction);
	void DoRubberBoatDismount(ItemInfo& rBoatItem, ItemInfo& laraItem);
	void DoRubberBoatImpact(ItemInfo& rBoatItem, ItemInfo& laraItem, VehicleImpactDirection impactDirection);

	VehicleImpactDirection RubberBoatDynamics(short itemNumber, ItemInfo& laraItem);
	void DoRubberBoatShift(int itemNumber, ItemInfo& laraItem);

	void SpawnRubberBoatMist(long x, long y, long z, long velocity, short angle, long snow);
}
