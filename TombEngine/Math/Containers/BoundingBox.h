#pragma once

struct PoseData;

//namespace TEN::Math
//{
	struct BOUNDING_BOX
	{
		// Components
		short X1;
		short X2;
		short Y1;
		short Y2;
		short Z1;
		short Z2;

		// Utilities
		int Height() const;

		// Converters
		BoundingOrientedBox ToDXBoundingOrientedBox(const PoseData& pose) const;
		BoundingOrientedBox ToDXBoundingOrientedBox(const Vector3& pos, const Quaternion& orient) const;

		// Operators
		BOUNDING_BOX operator +(const PoseData& pose) const;
		BOUNDING_BOX operator *(float scale) const;
	};
//}
