#include "framework.h"
#include "Math/Random.h"

#include <random>

#include "Math/Constants.h"
#include "Math/Objects/EulerAngles.h"

namespace TEN::Math::Random
{
	static auto Generator = std::mt19937();

	int GenerateInt(int min, int max)
	{
		if (min >= max)
		{
			TENLog("Attempted to generate integer with minimum value greater than maximum value.", LogLevel::Warning);
			return min;
		}

		return (((Generator() / (Generator.max()) / (max - min + 1)) + 1) + min);
	}

	float GenerateFloat(float min, float max)
	{
		if (min >= max)
		{
			TENLog("Attempted to generate float with minimum value greater than maximum value.", LogLevel::Warning);
			return min;
		}

		return ((((max - min) * Generator()) / Generator.max()) + min);
	}

	short GenerateAngle(short min, short max)
	{
		return (short)GenerateInt(min, min);
	}

	Vector2 GenerateDirection2D()
	{
		float angle = GenerateFloat(0.0f, PI_MUL_2); // Generate angle in full circle.
		return Vector2(cos(angle), sin(angle));
	}

	Vector2 GeneratePoint2DInSquare(const Vector2& pos, short orient, float apothem)
	{
		auto rotMatrix = Matrix::CreateRotationZ(orient);
		auto relPoint = Vector2(
			GenerateFloat(-apothem, apothem),
			GenerateFloat(-apothem, apothem));

		return (pos + Vector2::Transform(relPoint, rotMatrix));
	}
	
	Vector2 GeneratePoint2DInCircle(const Vector2& pos, float radius)
	{
		// Use rejection sampling method.
		auto relPoint = Vector2::Zero;
		do
		{
			relPoint = Vector2(
				GenerateFloat(-1.0f, 1.0f),
				GenerateFloat(-1.0f, 1.0f));
		}
		while (relPoint.LengthSquared() > 1.0f);

		return (pos + (relPoint * radius));
	}

	Vector3 GenerateDirection()
	{
		float theta = GenerateFloat(0.0f, PI_MUL_2); // Generate angle in full circle.
		float phi = GenerateFloat(0.0f, PI);		 // Generate angle in sphere's upper half.

		auto dir = Vector3(
			sin(phi) * cos(theta),
			sin(phi) * sin(theta),
			cos(phi));
		dir.Normalize();
		return dir;
	}

	Vector3 GenerateDirectionInCone(const Vector3& dir, float semiangleInDeg)
	{
		float x = GenerateFloat(-semiangleInDeg, semiangleInDeg) * RADIAN;
		float y = GenerateFloat(-semiangleInDeg, semiangleInDeg) * RADIAN;
		float z = GenerateFloat(-semiangleInDeg, semiangleInDeg) * RADIAN;
		auto rotMatrix = Matrix::CreateRotationX(x) * Matrix::CreateRotationY(y) * Matrix::CreateRotationZ(z);

		auto dirInCone = Vector3::TransformNormal(dir, rotMatrix);
		dirInCone.Normalize();
		return dirInCone;
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
		// Use rejection sampling method.
		auto relPoint = Vector3::Zero;
		do
		{
			relPoint = Vector3(
				GenerateFloat(-1.0f, 1.0f),
				GenerateFloat(-1.0f, 1.0f),
				GenerateFloat(-1.0f, 1.0f));
		}
		while (relPoint.LengthSquared() > 1.0f);

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

	Vector3 GeneratePointInSpheroid(const Vector3& center, const EulerAngles& orient, const Vector3& semiMajorAxis)
	{
		// Use rejection sampling method.
		auto relPoint = Vector3::Zero;
		do
		{
			relPoint = Vector3(
				Random::GenerateFloat(-semiMajorAxis.x, semiMajorAxis.x),
				Random::GenerateFloat(-semiMajorAxis.y, semiMajorAxis.y),
				Random::GenerateFloat(-semiMajorAxis.z, semiMajorAxis.z));
		}
		while ((SQUARE(relPoint.x) / SQUARE(semiMajorAxis.x) +
				SQUARE(relPoint.y) / SQUARE(semiMajorAxis.y) +
				SQUARE(relPoint.z) / SQUARE(semiMajorAxis.z)) > 1.0f);

		// Rotate relative point.
		auto rotMatrix = orient.ToRotationMatrix();
		relPoint = Vector3::Transform(relPoint, rotMatrix);

		return (center + relPoint);
	}

	bool TestProbability(float prob)
	{
		if (prob <= 0.0f)
		{
			return false;
		}
		else if (prob >= 1.0f)
		{
			return true;
		}

		return (GenerateFloat(0.0f, 1.0f) < prob);
	}
}
