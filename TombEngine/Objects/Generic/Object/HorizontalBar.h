#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Generic
{
	void HorizontalBarCollision(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll);
}
