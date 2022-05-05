#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Generic
{
	void PoleCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
