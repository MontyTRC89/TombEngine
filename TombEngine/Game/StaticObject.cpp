#include "framework.h"

#include "Game/Setup.h"

//namespace TEN
//{
	GameBoundingBox MESH_INFO::GetAabb(bool getVisibilityAabb) const
	{
		static auto bounds = GameBoundingBox();

		if (getVisibilityAabb)
		{
			bounds = Statics[ObjectId].visibilityBox;
		}
		else
		{
			bounds = Statics[ObjectId].collisionBox;
		}

		auto center = bounds.GetCenter();
		auto extents = bounds.GetExtents();
		auto scaledExtents = extents * Transform.Scale;
		auto scaledOffset = (center * Transform.Scale) - center;

		bounds.X1 = (center.x - scaledExtents.x) + scaledOffset.x;
		bounds.X2 = (center.x + scaledExtents.x) + scaledOffset.x;
		bounds.Y1 = (center.y - scaledExtents.y) + scaledOffset.y;
		bounds.Y2 = (center.y + scaledExtents.y) + scaledOffset.y;
		bounds.Z1 = (center.z - scaledExtents.z) + scaledOffset.z;
		bounds.Z2 = (center.z + scaledExtents.z) + scaledOffset.z;
		return bounds;
	}

	GameBoundingBox& GetBoundsAccurate(const MESH_INFO& staticObj, bool getVisibilityBox)
	{
		static auto bounds = GameBoundingBox();

		bounds = staticObj.GetAabb(getVisibilityBox);
		return bounds;
	}
//}
