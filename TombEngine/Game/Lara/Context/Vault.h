#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Player
{
	struct ClimbContextData;
	struct WaterTreadStepOutContextData;

	// Vault inquirers

	bool CanVaultFromSprint(const ItemInfo& item, const CollisionInfo& coll);

	// Vault contexts

	std::optional<ClimbContextData>				GetStandVaultClimbContext(const ItemInfo& item, const CollisionInfo& coll);
	std::optional<ClimbContextData>				GetCrawlVaultClimbContext(const ItemInfo& item, const CollisionInfo& coll);
	std::optional<ClimbContextData>				GetTreadWaterVaultClimbContext(ItemInfo& item, const CollisionInfo& coll);
	std::optional<WaterTreadStepOutContextData> GetTreadWaterStepOutContext(const ItemInfo& item);
}
