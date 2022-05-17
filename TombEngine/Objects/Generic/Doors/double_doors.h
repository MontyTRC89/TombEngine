#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Doors
{
	void DoubleDoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
