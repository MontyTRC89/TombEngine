#pragma once
#include "Objects/TR4/Vehicles/jeep_info.h"
#include "Objects/Utils/VehicleHelpers.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	JeepInfo& GetJeepInfo(ItemInfo& jeepItem);
	void InitialiseJeep(short itemNumber);

	void JeepPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	int JeepControl(ItemInfo& laraItem);
	int JeepUserControl(ItemInfo& jeepItem, ItemInfo& laraItem, int height, int* pitch);
	void AnimateJeep(ItemInfo& jeepItem, ItemInfo& laraItem, VehicleImpactType impactDirection, bool isDead);

	void DoJeepMount(ItemInfo& jeepItem, ItemInfo& laraItem, VehicleMountType mountType);
	bool TestJeepDismount(ItemInfo& jeepItem, ItemInfo& laraItem);
	int DoJeepDismount(ItemInfo& laraItem);
	void DoJeepImpact(ItemInfo& jeepItem, ItemInfo& laraItem, VehicleImpactType impactDirection);

	VehicleImpactType JeepDynamics(ItemInfo& jeepItem, ItemInfo& laraItem);

	void TriggerJeepExhaustSmokeEffect(int x, int y, int z, short angle, short speed, int moving);
}
