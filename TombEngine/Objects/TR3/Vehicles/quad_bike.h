#pragma once
#include "Objects/TR3/Vehicles/quad_bike_info.h"
#include "Objects/Utils/VehicleHelpers.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	QuadBikeInfo& GetQuadBikeInfo(ItemInfo* quadBikeItem);
	void InitialiseQuadBike(short itemNumber);

	void QuadBikePlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	bool QuadBikeControl(ItemInfo* laraItem, CollisionInfo* coll);
	int QuadUserControl(ItemInfo* quadBikeItem, int height, int* pitch);
	void AnimateQuadBike(ItemInfo* quadBikeItem, ItemInfo* laraItem, VehicleImpactDirection impactDirection, bool dead);

	void DoQuadBikeMount(ItemInfo* quadBikeItem, ItemInfo* laraItem, VehicleMountType mountType);
	int TestQuadBikeDismount(ItemInfo* laraItem, int direction);
	bool DoQuadBikeDismount(ItemInfo* quadBikeItem, ItemInfo* laraItem);
	void DoQuadBikeImpact(ItemInfo* quadBikeItem, ItemInfo* laraItem, VehicleImpactDirection impactDirection);

	VehicleImpactDirection QuadDynamics(ItemInfo* quadBikeItem, ItemInfo* laraItem);
	void TriggerQuadBikeExhaustSmokeEffect(int x, int y, int z, short angle, int speed, int moving);
}
