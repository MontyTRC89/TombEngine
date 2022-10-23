#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Generic
{
	void LadderCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
