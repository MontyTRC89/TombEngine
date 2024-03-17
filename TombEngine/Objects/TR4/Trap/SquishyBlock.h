#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Traps
{
	void ControlSquishyBlock(short itemNumber);
	void FallingSquishyBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void ControlFallingSquishyBlock(short itemNumber);
}
