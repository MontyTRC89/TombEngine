#pragma once
#include "Objects/TR3/Vehicles/upv_info.h"
#include "Objects/Utils/VehicleHelpers.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	UPVInfo* GetUPVInfo(ItemInfo* UPVItem);
	void UPVInitialise(short itemNumber);

	void UPVPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	bool UPVControl(ItemInfo* laraItem, CollisionInfo* coll);
	void UPVUserControl(ItemInfo* UPVItem, ItemInfo* laraItem);

	void DoUPVMount(ItemInfo* UPVItem, ItemInfo* laraItem, VehicleMountType mountType);
	bool TestUPVDismount(ItemInfo* UPVItem, ItemInfo* laraItem);

	void BackgroundCollision(ItemInfo* UPVItem, ItemInfo* laraItem);

	void DoCurrent(ItemInfo* UPVItem, ItemInfo* laraItem);
	void FireUPVHarpoon(ItemInfo* UPVItem, ItemInfo* laraItem);
	void TriggerUPVEffects(short itemNumber);
	void TriggerUPVMistEffect(long x, long y, long z, long velocity, short angle);
}
