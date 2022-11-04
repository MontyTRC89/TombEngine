#pragma once
#include "Objects/TR2/Vehicles/SkidooInfo.h"
#include "Objects/Utils/VehicleHelpers.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	SkidooInfo& GetSkidooInfo(ItemInfo* skidooItem);
	void InitialiseSkidoo(short itemNumber);

	bool SkidooControl(ItemInfo* laraItem, CollisionInfo* coll);
	VehicleImpactDirection SkidooDynamics(ItemInfo* skidooItem, ItemInfo* laraItem);
	bool SkidooUserControl(ItemInfo* skidooItem, ItemInfo* laraItem, int height, int* pitch);
	void AnimateSkidoo(ItemInfo* skidooItem, ItemInfo* laraItem, VehicleImpactDirection impactDirection, bool isDead);

	void SkidooPlayerCollision(short skidooItemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoSkidooMount(ItemInfo* skidooItem, ItemInfo* laraItem, VehicleMountType mountType);
	bool TestSkidooDismount(ItemInfo* skidooItem, int direction);
	bool DoSkidooDismount(ItemInfo* skidooItem, ItemInfo* laraItem);
	void DoSkidooImpact(ItemInfo* skidooItem, ItemInfo* laraItem, VehicleImpactDirection impactDirection);

	void HandleSkidooGuns(ItemInfo* skidooItem, ItemInfo* laraItem);
	void TriggerSkidooSnowEffect(ItemInfo* skidooItem);
}
