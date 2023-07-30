#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	void BigGunInitlize(short itemNumber);

	void BigGunCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	bool BigGunControl(ItemInfo& laraItem, CollisionInfo& coll);

	static bool BigGunTestMount(ItemInfo& bigGunItem, ItemInfo& laraItem);

	void BigGunFire(ItemInfo& bigGunItem, ItemInfo& laraItem);
}
