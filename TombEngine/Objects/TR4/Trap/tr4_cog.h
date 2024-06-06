#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Traps
{
	void CogControl(short itemNumber);
	void CogCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
