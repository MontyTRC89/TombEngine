#pragma once
#include "Objects/TR3/Vehicles/upv_info.h"
#include "Objects/Utils/VehicleHelpers.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	UpvInfo& GetUpvInfo(ItemInfo& upvItem);
	void UpvInitialise(short itemNumber);

	bool UpvControl(ItemInfo& laraItem, CollisionInfo& coll);
	void UpvUserControl(ItemInfo& upvItem, ItemInfo& laraItem);

	void UpvPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoUpvMount(ItemInfo& upvItem, ItemInfo& laraItem, VehicleMountType mountType);
	bool TestUpvDismount(ItemInfo& upvItem, ItemInfo& laraItem);

	void FireUpvHarpoon(ItemInfo& upvItem, ItemInfo& laraItem);
	void UpvBackgroundCollision(ItemInfo& upvItem, ItemInfo& laraItem);
	void DoUpvCurrent(ItemInfo& upvItem, ItemInfo& laraItem);
	void UpvEffects(short itemNumber);
	void TriggerUpvMistEffect(long x, long y, long z, long velocity, short angle);
}
