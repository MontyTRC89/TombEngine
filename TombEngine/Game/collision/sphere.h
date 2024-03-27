#pragma once

struct ItemInfo;

enum class SphereSpaceFlags
{
	Local	   = 1,
	World	   = 1 << 1,
	BoneOrigin = 1 << 2
};

std::vector<BoundingSphere> GetSpheres(const ItemInfo* itemPtr, int spaceFlags, const Matrix& localMatrix = Matrix::Identity);
int TestCollision(ItemInfo* creatureItemPtr, ItemInfo* playerItemPtr);
