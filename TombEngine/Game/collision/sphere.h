#pragma once

struct ItemInfo;

namespace TEN::Collision::Sphere
{
	enum class SphereSpaceFlags
	{
		Local	   = 1,
		World	   = 1 << 1,
		BoneOrigin = 1 << 2
	};

	std::vector<BoundingSphere> GetSpheres(const ItemInfo& item, int spaceFlags);
	bool SetSphereTouchBits(ItemInfo* creatureItemPtr, ItemInfo* playerItemPtr);
}
