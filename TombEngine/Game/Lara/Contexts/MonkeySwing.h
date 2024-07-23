#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Player
{
	// Monkey swing inquirers

	bool CanPerformMonkeySwingStep(const ItemInfo& item, const CollisionInfo& coll);
	bool CanFallFromMonkeySwing(const ItemInfo& item, const CollisionInfo& coll);
	bool CanGrabMonkeySwing(const ItemInfo& item, const CollisionInfo& coll);
	bool CanMonkeySwingForward(const ItemInfo& item, const CollisionInfo& coll);
	bool CanMonkeySwingBackward(const ItemInfo& item, const CollisionInfo& coll);
	bool CanMonkeySwingShimmyLeft(const ItemInfo& item, const CollisionInfo& coll);
	bool CanMonkeySwingShimmyRight(const ItemInfo& item, const CollisionInfo& coll);
}
