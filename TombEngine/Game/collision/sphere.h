#pragma once

struct ItemInfo;

namespace TEN::Collision::Sphere
{
	std::vector<BoundingSphere> GetSpheres(const ItemInfo& item);
	bool HandleItemSphereCollision(ItemInfo& item0, ItemInfo& item1);
}
