#pragma once

namespace TEN::Math::Random
{
	int32_t GenerateInt(int32_t low = 0, int32_t high = SHRT_MAX);
	float	GenerateFloat(float low = 0.0f, float high = 1.0f);

	short	GenerateAngle(short low = SHRT_MIN, short high = SHRT_MAX);
	Vector2 GenerateDirection2D();
	Vector3 GenerateDirection();
	Vector3 GenerateDirectionInCone(const Vector3& direction, float semiangleInDeg);
	Vector3 GeneratePointInBox(const BoundingOrientedBox& box);
	Vector3 GeneratePointInSphere(const BoundingSphere& sphere);
	Vector3 GeneratePointOnSphere(const BoundingSphere& sphere);

	bool TestProbability(float probability);
}
