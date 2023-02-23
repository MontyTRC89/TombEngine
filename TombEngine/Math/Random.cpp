#include "framework.h"
#include "Math/Random.h"

#include <random>

#include "Math/Constants.h"

namespace TEN::Math::Random
{
	static std::mt19937 Engine;

	int32_t GenerateInt(int32_t low, int32_t high)
	{
		return (Engine() / (Engine.max() / (high - low + 1) + 1) + low);
	}

	float GenerateFloat(float low, float high)
	{
		return ((high - low) * Engine() / Engine.max() + low);
	}

	short GenerateAngle(short low, short high)
	{
		return (short)GenerateInt(low, high);
	}

	Vector2 GenerateDirection2D()
	{
		float angle = GenerateFloat(0.0f, 2.0f * PI); // Generate angle in full circle.
		return Vector2(cos(angle), sin(angle));
	}

	Vector3 GenerateDirection()
	{
		float theta = GenerateFloat(0.0f, 2.0f * PI); // Generate angle in full circle.
		float phi = GenerateFloat(0.0f, PI);		  // Generate angle in sphere's upper half.

		auto direction = Vector3(
			sin(phi) * cos(theta),
			sin(phi) * sin(theta),
			cos(phi));
		direction.Normalize();
		return direction;
	}

	Vector3 GenerateDirectionInCone(const Vector3& direction, float semiangleInDeg)
	{
		float x = GenerateFloat(-semiangleInDeg, semiangleInDeg) * RADIAN;
		float y = GenerateFloat(-semiangleInDeg, semiangleInDeg) * RADIAN;
		float z = GenerateFloat(-semiangleInDeg, semiangleInDeg) * RADIAN;
		auto matrix = Matrix::CreateRotationX(x) * Matrix::CreateRotationY(y) * Matrix::CreateRotationZ(z);

		auto vector = Vector3::TransformNormal(direction, matrix);
		vector.Normalize();
		return vector;
	}

	Vector3 GeneratePointInBox(const BoundingOrientedBox& box)
	{
		auto rotMatrix = Matrix::CreateFromQuaternion(box.Orientation);
		auto vector = Vector3(
			GenerateFloat(-box.Extents.x, box.Extents.x),
			GenerateFloat(-box.Extents.y, box.Extents.y),
			GenerateFloat(-box.Extents.z, box.Extents.z));

		return (box.Center + Vector3::Transform(vector, rotMatrix));
	}

	Vector3 GeneratePointInSphere(const BoundingSphere& sphere)
	{
		return (sphere.Center + (GenerateDirection() * GenerateFloat(0.0f, sphere.Radius)));
	}

	Vector3 GeneratePointOnSphere(const BoundingSphere& sphere)
	{
		float u = GenerateFloat(0.0f, 1.0f);
		float v = GenerateFloat(0.0f, 1.0f);

		float theta = u * PI_MUL_2;
		float phi = acos((v * 2) - 1.0f);

		auto offset = Vector3(
			sphere.Radius * (sin(phi) * cos(theta)),
			sphere.Radius * (sin(phi) * sin(theta)),
			sphere.Radius * cos(phi));
		return (sphere.Center + offset);
	}

	bool TestProbability(float probability)
	{
		probability = std::clamp(probability, 0.0f, 1.0f);

		if (probability == 0.0f)
			return false;
		else if (probability == 1.0f)
			return true;

		return (GenerateFloat(0.0f, 1.0f) < probability);
	}
}
