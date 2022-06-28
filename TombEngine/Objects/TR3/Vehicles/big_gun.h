#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Vehicles
{
	void BigGunInitialise(short itemNumber);
	static bool BigGunTestMount(ItemInfo* bigGunItem, ItemInfo* laraItem);
	void BigGunFire(ItemInfo* bigGunItem, ItemInfo* laraItem);
	void BigGunCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	bool BigGunControl(ItemInfo* laraItem, CollisionInfo* coll);
}
