#pragma once

struct ITEM_INFO;
struct CollisionInfo;

namespace TEN::Entities::Doors
{
	void UnderwaterDoorCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
}
