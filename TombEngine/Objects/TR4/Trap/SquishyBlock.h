#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Traps
{
	void InitializeSquishyBlock(short itemNumber);
	void ControlSquishyBlock(short itemNumber);
	void FallingSquishyBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void ControlFallingSquishyBlock(short itemNumber);
	bool IsNextSectorValid(const ItemInfo& item, const Vector3& dir, short& vel);
	void SquishyBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
