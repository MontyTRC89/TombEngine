#pragma once

struct ITEM_INFO;
struct CollisionInfo;

namespace TEN::Entities::Generic
{
	void PoleCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
}
