#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Player::Context
{
	struct EdgeCatchData;
	struct MonkeySwingCatchData;

	// Ledge context inquirers
	bool CanSwingOnLedge(const ItemInfo& item, const CollisionInfo& coll);
	bool CanPerformLedgeJump(const ItemInfo& item, const CollisionInfo& coll);
	bool CanPerformLedgeHandstand(const ItemInfo& item, CollisionInfo& coll);
	bool CanClimbLedgeToCrouch(const ItemInfo& item, CollisionInfo& coll);
	bool CanClimbLedgeToStand(const ItemInfo& item, CollisionInfo& coll);
	bool CanShimmyUp(const ItemInfo& item, const CollisionInfo& coll);
	bool CanShimmyDown(const ItemInfo& item, const CollisionInfo& coll);
	bool CanShimmyLeft(ItemInfo& item, CollisionInfo& coll);
	bool CanShimmyRight(ItemInfo& item,CollisionInfo& coll);

	// Context data getters
	std::optional<EdgeCatchData>		GetEdgeCatchData(ItemInfo& item, CollisionInfo& coll);
	std::optional<MonkeySwingCatchData> GetMonkeySwingCatchData(const ItemInfo& item, const CollisionInfo& coll);
}
