#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Player::Context
{
	struct EdgeCatchData;
	struct MonkeySwingCatchData;

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

	// Context data getters
	std::optional<EdgeCatchData>		GetEdgeCatchData(ItemInfo& item, CollisionInfo& coll);
	std::optional<MonkeySwingCatchData> GetMonkeySwingCatchData(ItemInfo& item, const CollisionInfo& coll);
}
