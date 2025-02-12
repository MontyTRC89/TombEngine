#include "framework.h"

#include "Game/Setup.h"

//namespace TEN
//{
	GameBoundingBox MESH_INFO::GetVisibilityAabb() const
	{
		auto aabb = Statics[ObjectId].visibilityBox;
		ScaleAabb(aabb);
		return aabb;
	}

	GameBoundingBox MESH_INFO::GetCollisionAabb() const
	{
		auto aabb = Statics[ObjectId].collisionBox;
		ScaleAabb(aabb);
		return aabb;
	}

	void MESH_INFO::ScaleAabb(GameBoundingBox& aabb) const
	{
		// Calculate scaled parameters.
		auto center = aabb.GetCenter();
		auto extents = aabb.GetExtents();
		auto scaledExtents = extents * Transform.Scale;
		auto scaledOffset = (center * Transform.Scale) - center;

		// Scale AABB.
		aabb.X1 = (center.x - scaledExtents.x) + scaledOffset.x;
		aabb.X2 = (center.x + scaledExtents.x) + scaledOffset.x;
		aabb.Y1 = (center.y - scaledExtents.y) + scaledOffset.y;
		aabb.Y2 = (center.y + scaledExtents.y) + scaledOffset.y;
		aabb.Z1 = (center.z - scaledExtents.z) + scaledOffset.z;
		aabb.Z2 = (center.z + scaledExtents.z) + scaledOffset.z;
	}

	GameBoundingBox& GetBoundsAccurate(const MESH_INFO& staticObj, bool getVisibilityAabb)
	{
		static auto aabb = GameBoundingBox();

		aabb = getVisibilityAabb ? staticObj.GetVisibilityAabb() : staticObj.GetCollisionAabb();
		return aabb;
	}
//}
