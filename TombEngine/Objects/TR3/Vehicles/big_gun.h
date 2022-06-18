#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Vehicles
{
	void BigGunInitialise(short itemNumber);
	static bool BigGunTestMount(ItemInfo* laraItem, ItemInfo* bigGunItem);
	void BigGunFire(ItemInfo* laraItem, ItemInfo* bigGunItem);
	void BigGunCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	bool BigGunControl(ItemInfo* laraItem, CollisionInfo* coll);
}
