#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Doors
{
	void UnderwaterWallCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
