#pragma once

struct CollisionInfo;
struct ItemInfo;
struct ObjectInfo;

namespace TEN::Entities::Traps
{
	static void InitializeHammer(ItemInfo& frontItem);
	void InitializeHandle(short itemNumber);
	static void	SyncHammerSegment(ItemInfo& frontItem);
	void ControlHandle(short itemNumber);
	void CollideHandle(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void CollideHammer(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
