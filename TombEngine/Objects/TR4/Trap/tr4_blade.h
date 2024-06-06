#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Traps
{
	void BladeCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
