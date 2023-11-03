#pragma once

struct CollisionResult;
struct ItemInfo;

namespace TEN::Collision::Attractors
{
	class Attractor;

	void HandleAttractorDebug(ItemInfo& item);
	std::vector<Attractor*> GetDebugAttractorPtrs(ItemInfo& item);
	std::vector<Attractor> GenerateSectorAttractors(const CollisionResult& pointColl);;
}
