#include "framework.h"
#include "Math/Random.h"

#include <random>

#include "Math/Constants.h"

namespace TEN::Math::Random
{
	static std::mt19937 Engine;

	int GenerateInt(int low, int high)
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
		float angle = GenerateFloat(0.0f, PI_MUL_2); // Generate angle in full circle.
		return Vector2(cos(angle), sin(angle));
	}

	Vector2 GeneratePoint2DInSquare(const Vector2& pos2D, short orient2D, float radius)
	{
		auto rotMatrix = Matrix::CreateRotationZ(orient2D);
		auto relPoint = Vector2(
			GenerateFloat(-radius, radius),
			GenerateFloat(-radius, radius));

		return (pos2D + Vector2::Transform(relPoint, rotMatrix));
	}
	
	Vector2 GeneratePoint2DInCircle(const Vector2& pos2D, float radius)
	{
		// Use rejection sampling.
		auto relPoint = Vector2::Zero;
		do
		{
			relPoint = Vector2(
				GenerateFloat(-1.0f, 1.0f),
				GenerateFloat(-1.0f, 1.0f));
		} while (relPoint.LengthSquared() > 1.0f);

		return (pos2D + (relPoint * radius));
	}

	Vector3 GenerateDirection()
	{
		float theta = GenerateFloat(0.0f, PI_MUL_2); // Generate angle in full circle.
		float phi = GenerateFloat(0.0f, PI);		 // Generate angle in sphere's upper half.

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
		// Use rejection sampling.
		auto relPoint = Vector3::Zero;
		do
		{
			relPoint = Vector3(
				GenerateFloat(-1.0f, 1.0f),
				GenerateFloat(-1.0f, 1.0f),
				GenerateFloat(-1.0f, 1.0f));
		} while (relPoint.LengthSquared() > 1.0f);

		return (sphere.Center + (relPoint * sphere.Radius));
	}

	Vector3 GeneratePointOnSphere(const BoundingSphere& sphere)
	{
		float u = GenerateFloat(0.0f, 1.0f);
		float v = GenerateFloat(0.0f, 1.0f);

		float theta = u * PI_MUL_2;
		float phi = acos((v * 2) - 1.0f);

		auto relPoint = Vector3(
			sin(phi) * cos(theta),
			sin(phi) * sin(theta),
			cos(phi));
		return (sphere.Center + (relPoint * sphere.Radius));
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
