#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	void InitializeMinecart(short itemNumber);

	void MinecartPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DoMinecartMount(ItemInfo* minecartItem, ItemInfo* laraItem, enum class VehicleMountType mountType);

	bool MinecartControl(ItemInfo* laraItem);
}
