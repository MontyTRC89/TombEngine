#pragma once

class EulerAngles;
struct PoseData;

//namespace TEN::Math
//{
	struct BOUNDING_BOX
	{
		// Components
		short X1, X2 = 0;
		short Y1, Y2 = 0;
		short Z1, Z2 = 0;

		// Getters
		int Height() const;

		// Utilities
		void RotNoPersp(const EulerAngles& orient, const BOUNDING_BOX& bounds);

		// Converters
		BoundingOrientedBox ToDXBoundingOrientedBox(const PoseData& pose) const;
		BoundingOrientedBox ToDXBoundingOrientedBox(const Vector3& pos, const Quaternion& orient) const;

		// Operators
		BOUNDING_BOX operator +(const PoseData& pose) const;
		BOUNDING_BOX operator *(float scale) const;
	};
//}
