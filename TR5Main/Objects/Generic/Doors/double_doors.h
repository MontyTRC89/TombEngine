#pragma once

struct ITEM_INFO;
struct CollisionInfo;

namespace TEN::Entities::Doors
{
	void DoubleDoorCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
}
