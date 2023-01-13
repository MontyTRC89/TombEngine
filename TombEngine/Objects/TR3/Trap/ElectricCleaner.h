#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Traps
{
	void InitialiseCleaner(short itemNumber);
	void CleanerControl(short itemNumber);
	void CleanerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
