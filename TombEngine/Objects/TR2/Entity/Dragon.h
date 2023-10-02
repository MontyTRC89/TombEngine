#pragma once

struct CollisionInfo;
struct CreatureBiteInfo;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR2
{
	void InitializeDragon(short itemNumber);
	void ControlDragon(short itemNumber);
	void CollideDragon(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll);
}
