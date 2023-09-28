#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Traps
{
	void InitializeCrumblingPlatform(short itemNumber);
	void ControlCrumblingPlatform(short itemNumber);
	void CollideCrumblingPlatform(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	std::optional<int> CrumblingPlatformFloor(short itemNumber, int x, int y, int z);
	std::optional<int> CrumblingPlatformCeiling(short itemNumber, int x, int y, int z);
	int				   CrumblingPlatformFloorBorder(short itemNumber);
	int				   CrumblingPlatformCeilingBorder(short itemNumber);
}
