#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Player
{
	struct ClimbContextData;
	struct WaterTreadStepOutContextData;

	// Vault climb contexts

	std::optional<ClimbContextData>				GetStandVaultClimbContext(const ItemInfo& item, const CollisionInfo& coll);
	std::optional<ClimbContextData>				GetCrawlVaultClimbContext(const ItemInfo& item, const CollisionInfo& coll);
	std::optional<ClimbContextData>				GetTreadWaterVaultClimbContext(ItemInfo& item, const CollisionInfo& coll);
	std::optional<WaterTreadStepOutContextData> GetTreadWaterStepOutContext(const ItemInfo& item);
}
