#pragma once
#include "Objects/TR2/Vehicles/SkidooInfo.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	enum class VehicleImpactType;
	enum class VehicleMountType;

	SkidooInfo& GetSkidooInfo(ItemInfo& skidooItem);
	void InitializeSkidoo(short itemNumber);

	bool SkidooControl(ItemInfo& laraItem, CollisionInfo& coll);
	VehicleImpactType SkidooDynamics(ItemInfo& skidooItem, ItemInfo& laraItem);
	bool SkidooUserControl(ItemInfo& skidooItem, ItemInfo& laraItem, int height, int* pitch);
	void AnimateSkidoo(ItemInfo& skidooItem, ItemInfo& laraItem, VehicleImpactType impact, bool isDead);

	void SkidooPlayerCollision(short skidooItemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoSkidooMount(ItemInfo& skidooItem, ItemInfo& laraItem, VehicleMountType mountType);
	bool TestSkidooDismount(ItemInfo& skidooItem, int direction);
	bool DoSkidooDismount(ItemInfo& skidooItem, ItemInfo& laraItem);
	void DoSkidooImpact(ItemInfo& skidooItem, ItemInfo& laraItem, VehicleImpactType impact);

	void HandleSkidooGuns(ItemInfo& skidooItem, ItemInfo& laraItem);
	void SpawnSkidooSnow(ItemInfo& skidooItem);
}
