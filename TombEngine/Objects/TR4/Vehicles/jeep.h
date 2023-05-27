#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	void InitializeJeep(short itemNumber);

	void JeepPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoJeepMount(ItemInfo* jeepItem, ItemInfo* laraItem, enum class VehicleMountType mountType);

	int JeepControl(ItemInfo* laraItem);
}
