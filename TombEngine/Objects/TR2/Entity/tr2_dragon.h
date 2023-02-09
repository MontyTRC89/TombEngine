#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Creatures::TR2
{
	void DragonCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void DragonControl(short backNumber);
	void BartoliControl(short itemNumber);

	void SphereOfDoomControl(short itemNumber);
}
