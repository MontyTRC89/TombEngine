#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Player
{
	struct ClimbContextData;

	// Hang descent climb contexts

	std::optional<ClimbContextData> GetStandHangDescentFrontClimbContext(const ItemInfo& item, const CollisionInfo& coll);
	std::optional<ClimbContextData> GetStandHangDescentBackClimbContext(const ItemInfo& item, const CollisionInfo& coll);
	std::optional<ClimbContextData> GetCrawlHangDescentFrontClimbContext(const ItemInfo& item, const CollisionInfo& coll);
	std::optional<ClimbContextData> GetCrawlHangDescentBackClimbContext(const ItemInfo& item, const CollisionInfo& coll);
}
