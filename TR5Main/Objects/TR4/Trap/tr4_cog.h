#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::TR4
{
	void CogControl(short itemNumber);
	void CogCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
