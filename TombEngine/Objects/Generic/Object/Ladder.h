#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Generic
{
	void CollideLadder(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
