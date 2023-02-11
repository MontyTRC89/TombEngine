#pragma once

class EulerAngles;
struct ItemInfo;
class Pose;

//namespace TEN::Math
//{
	class GameBoundingBox
	{
	public:
		// Components
		int X1 = 0;
		int X2 = 0;
		int Y1 = 0;
		int Y2 = 0;
		int Z1 = 0;
		int Z2 = 0;

		// Constants
		static const GameBoundingBox Zero;

		// Constructors
		GameBoundingBox();
		GameBoundingBox(float x1, float x2, float y1, float y2, float z1, float z2);
		GameBoundingBox(ItemInfo* item);

		// Getters
		int		GetWidth() const;
		int		GetHeight() const;
		int		GetDepth() const;
		Vector3 GetCenter() const;
		Vector3 GetExtents() const;

		// Utilities
		void RotateNoPersp(const EulerAngles& orient, const GameBoundingBox& bounds);

		// Converters
		BoundingOrientedBox ToBoundingOrientedBox(const Pose& pose) const;
		BoundingOrientedBox ToBoundingOrientedBox(const Vector3& pos, const Quaternion& orient) const;

		// Operators
		GameBoundingBox operator +(const GameBoundingBox& bounds) const;
		GameBoundingBox operator +(const Pose& pose) const;
		GameBoundingBox operator -(const GameBoundingBox& bounds) const;
		GameBoundingBox operator -(const Pose& pose) const;
		GameBoundingBox operator *(float scale) const;
		GameBoundingBox operator /(float scale) const;
	};
//}
