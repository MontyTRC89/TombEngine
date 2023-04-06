#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Player::Context
{
	struct EdgeCatchData;

	// Ledge context inquirers
	bool CanSwingOnLedge(ItemInfo& item, CollisionInfo& coll);
	bool CanPerformLedgeJump(ItemInfo& item, CollisionInfo& coll);
	bool CanPerformLedgeHandstand(ItemInfo& item, CollisionInfo& coll);
	bool CanClimbLedgeToCrouch(ItemInfo& item, CollisionInfo& coll);
	bool CanClimbLedgeToStand(ItemInfo& item, CollisionInfo& coll);
	bool CanLedgeShimmyLeft(ItemInfo& item, CollisionInfo& coll);
	bool CanLedgeShimmyRight(ItemInfo& item, CollisionInfo& coll);
	bool CanWallShimmyUp(ItemInfo& item, CollisionInfo& coll);
	bool CanWallShimmyDown(ItemInfo& item, CollisionInfo& coll);

	// Monkey swing
	bool CanCatchMonkeySwing(ItemInfo& item, CollisionInfo& coll);

	std::optional<EdgeCatchData> GetEdgeCatchData(ItemInfo& item, CollisionInfo& coll);
}
