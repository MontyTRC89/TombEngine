#pragma once

namespace TEN::Math::Random
{
	// Value generation
	int	  GenerateInt(int low = 0, int high = SHRT_MAX);
	float GenerateFloat(float low = 0.0f, float high = 1.0f);
	short GenerateAngle(short low = SHRT_MIN, short high = SHRT_MAX);

	// 2D geometric generation
	Vector2 GenerateDirection2D();
	Vector2 GeneratePoint2DInSquare(const Vector2& pos2D, short orient2D, float radius);
	Vector2 GeneratePoint2DInCircle(const Vector2& pos2D, float radius);

	// 3D geometric generation
	Vector3 GenerateDirection();
	Vector3 GenerateDirectionInCone(const Vector3& direction, float semiangleInDeg);
	Vector3 GeneratePointInBox(const BoundingOrientedBox& box);
	Vector3 GeneratePointInSphere(const BoundingSphere& sphere);
	Vector3 GeneratePointOnSphere(const BoundingSphere& sphere);

	bool TestProbability(float probability);
}
