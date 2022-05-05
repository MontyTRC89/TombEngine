#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Doors
{
	void UnderwaterDoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
