#pragma once

namespace TEN::Math::Random
{
	int32_t GenerateInt(int32_t low = 0, int32_t high = SHRT_MAX);
	float	GenerateFloat(float low = 0.0f, float high = 1.0f);

	short	GenerateAngle(short low = SHRT_MIN, short high = SHRT_MAX);
	Vector2 GenerateVector2(float length = 1.0f);
	Vector3 GenerateVector3(float length = 1.0f);
	Vector3 GenerateVector3InCone(const Vector3& direction, float semiangleInDeg, float length = 1.0f);

	bool TestProbability(float probability);
}
