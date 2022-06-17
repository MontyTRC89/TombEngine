#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Vehicles
{
	void InitialiseMinecart(short itemNumber);
	void MinecartCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	bool MinecartControl(ItemInfo* laraItem);
}
