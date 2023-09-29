#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR2
{
	void InitializeDragon(short itemNumber);
	void ControlDragon(short backNumber);
	void CollideDragon(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
