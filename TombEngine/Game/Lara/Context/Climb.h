#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Player
{
	struct ClimbContextData;

	// Ledge inquirers

	bool CanPerformLedgeJump(const ItemInfo& item, const CollisionInfo& coll);
	bool CanPerformLedgeHandstand(const ItemInfo& item, CollisionInfo& coll);
	bool CanClimbLedgeToCrouch(const ItemInfo& item, CollisionInfo& coll);
	bool CanClimbLedgeToStand(const ItemInfo& item, CollisionInfo& coll);

	// Hang climb contexts

	std::optional<ClimbContextData> GetEdgeHangShimmyUpContext(const ItemInfo& item, const CollisionInfo& coll);
	std::optional<ClimbContextData> GetEdgeHangShimmyDownContext(const ItemInfo& item, const CollisionInfo& coll);
	std::optional<ClimbContextData> GetEdgeHangShimmyLeftContext(const ItemInfo& item, const CollisionInfo& coll);
	std::optional<ClimbContextData> GetEdgeHangShimmyRightContext(const ItemInfo& item, const CollisionInfo& coll);
	bool CanEdgeHangToWallClimb(const ItemInfo& item, const CollisionInfo& coll);

	// Climbable wall climb contexts

	std::optional<ClimbContextData> GetWallClimbUpContext(const ItemInfo& item, const CollisionInfo& coll);
	std::optional<ClimbContextData> GetWallClimbDownContext(const ItemInfo& item, const CollisionInfo& coll);
	std::optional<ClimbContextData> GetWallClimbLeftContext(const ItemInfo& item, const CollisionInfo& coll);
	std::optional<ClimbContextData> GetWallClimbRightContext(const ItemInfo& item, const CollisionInfo& coll);
	bool CanWallClimbToEdgeHang(const ItemInfo& item, const CollisionInfo& coll);
}
