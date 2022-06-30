#pragma once
#include "Objects/TR2/Vehicles/skidoo_info.h"
#include "Objects/Utils/VehicleHelpers.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	SkidooInfo* GetSkidooInfo(ItemInfo* skidooItem);
	void InitialiseSkidoo(short itemNumber);

	void SkidooPlayerCollision(short skidooItemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	bool SkidooControl(ItemInfo* laraItem, CollisionInfo* coll);
	bool SkidooUserControl(ItemInfo* skidooItem, ItemInfo* laraItem, int height, int* pitch);
	void AnimateSkidoo(ItemInfo* skidooItem, ItemInfo* laraItem, VehicleImpactDirection impactDirection, bool isDead);

	void DoSkidooMount(ItemInfo* skidooItem, ItemInfo* laraItem, VehicleMountType mountType);
	bool TestSkidooDismount(ItemInfo* skidooItem, int direction);
	bool DoSkidooDismount(ItemInfo* skidooItem, ItemInfo* laraItem);
	void DoSkidooImpact(ItemInfo* skidooItem, ItemInfo* laraItem, VehicleImpactDirection impactDirection);

	VehicleImpactDirection SkidooDynamics(ItemInfo* skidooItem, ItemInfo* laraItem);

	void HandleSkidooGuns(ItemInfo* skidooItem, ItemInfo* laraItem);
	void TriggerSkidooSnowEffect(ItemInfo* skidooItem);
}
