#include "framework.h"
#include "Math/Random.h"

#include <random>

#include "Math/Constants.h"
#include "Math/Geometry.h"

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
		auto direction2D = Vector2(GenerateFloat(-1.0f, 1.0f), GenerateFloat(-1.0f, 1.0f));
		direction2D.Normalize();
		return direction2D;
	}

	Vector3 GenerateDirection()
	{
		auto direction = Vector3(GenerateFloat(-1.0f, 1.0f), GenerateFloat(-1.0f, 1.0f), GenerateFloat(-1.0f, 1.0f));
		direction.Normalize();
		return direction;
	}

	Vector3 GenerateDirectionInCone(const Vector3& direction, float semiangleInDeg)
	{
		float x = GenerateFloat(-semiangleInDeg, semiangleInDeg) * RADIAN;
		float y = GenerateFloat(-semiangleInDeg, semiangleInDeg) * RADIAN;
		float z = GenerateFloat(-semiangleInDeg, semiangleInDeg) * RADIAN;
		auto rotMatrix = Matrix::CreateRotationX(x) * Matrix::CreateRotationY(y) * Matrix::CreateRotationZ(z);

		auto directionInCone = Vector3::TransformNormal(direction, rotMatrix);
		directionInCone.Normalize();
		return directionInCone;
	}

	Vector3 GeneratePointInBox(const BoundingOrientedBox& box)
	{
		auto rotMatrix = Matrix::CreateFromQuaternion(box.Orientation);
		auto relPoint = Vector3(
			GenerateFloat(-box.Extents.x, box.Extents.x),
			GenerateFloat(-box.Extents.y, box.Extents.y),
			GenerateFloat(-box.Extents.z, box.Extents.z));

		return (box.Center + Vector3::Transform(relPoint, rotMatrix));
	}

	Vector3 GeneratePointInSphere(const BoundingSphere& sphere)
	{
		return Geometry::TranslatePoint(sphere.Center, GenerateDirection(), GenerateFloat(0.0f, sphere.Radius));
	}

	bool TestProbability(float probability)
	{
		probability = std::clamp(probability, 0.0f, 1.0f);

		if (probability == 0.0f)
		{
			return false;
		}
		else if (probability == 1.0f)
		{
			return true;
		}

		return (GenerateFloat(0.0f, 1.0f) < probability);
	}
}
