#pragma once

class Vector3i;
struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Traps
{
	void InitializeCrumblingPlatform(short itemNumber);
	void ControlCrumblingPlatform(short itemNumber);
	void CollideCrumblingPlatform(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);

	std::optional<int> GetCrumblingPlatformFloorHeight(const ItemInfo& item, const Vector3i& pos);
	std::optional<int> GetCrumblingPlatformCeilingHeight(const ItemInfo& item, const Vector3i& pos);
	int GetCrumblingPlatformFloorBorder(const ItemInfo& item);
	int GetCrumblingPlatformCeilingBorder(const ItemInfo& item);
}
