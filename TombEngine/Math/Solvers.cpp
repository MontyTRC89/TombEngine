#include "framework.h"
#include "Math/Solvers.h"

#include "Math/Constants.h"

using std::pair;

namespace TEN::Math::Solver
{
	pair<float, float> SolveQuadratic(float a, float b, float c)
	{
		pair<float, float> result = { INFINITY, INFINITY };

		if (abs(a) < FLT_EPSILON)
		{
			if (abs(b) < FLT_EPSILON)
				return result; // Zero solutions.

			result.first = -c / b;
			result.second = result.first;
			return result; // One solution.
		}

		float discriminant = SQUARE(b) - (4.0f * a * c);
		if (discriminant < 0.0f)
			return result; // Zero solutions.

		float inv2a = 1.0f / (2.0f * a);

		if (discriminant < FLT_EPSILON)
		{
			result.first = -b * inv2a;
			result.second = result.first;
			return result; // One solution.
		}

		discriminant = sqrt(discriminant);
		result.first = (-b - discriminant) * inv2a;
		result.second = (-b + discriminant) * inv2a;
		return result; // Two solutions.
	}

	bool SolveIK2D(const Vector2& target, float length0, float length1, Vector2& outMiddle)
	{
		float length = target.Length();
		if (length > (length0 + length1))
			return false;

		bool flipXY = (target.x < target.y);
		float a = flipXY ? target.y : target.x;
		float b = flipXY ? target.x : target.y;
		assert(abs(a) > FLT_EPSILON);

		float m = ((SQUARE(length0) - SQUARE(length1)) + (SQUARE(a) + SQUARE(b))) / (2.0f * a);
		float n = b / a;
		auto quadratic = SolveQuadratic(1.0f + SQUARE(n), -2.0f * (m * n), SQUARE(m) - SQUARE(length0));

		if (quadratic.first != INFINITY && quadratic.second != INFINITY)
		{
			outMiddle.x = flipXY ? quadratic.second : (m - (n * quadratic.second));
			outMiddle.y = flipXY ? (m - (n * quadratic.second)) : quadratic.second;
			return true;
		}

		outMiddle = target * (length0 / length);
		return false;
	}

	bool SolveIK3D(const Vector3& origin, const Vector3& target, const Vector3& pole, float length0, float length1, Vector3& outMiddle)
	{
		auto directionNorm = target - origin;
		directionNorm.Normalize();
		auto normal = directionNorm.Cross(pole - origin);
		normal.Normalize();

		// TODO: Check what this means.
		Matrix matrix;
		auto normalCrossDirectionNorm = normal.Cross(directionNorm);
		matrix._11 = normalCrossDirectionNorm.x;
		matrix._12 = normalCrossDirectionNorm.y;
		matrix._13 = normalCrossDirectionNorm.z;
		matrix._21 = directionNorm.x;
		matrix._22 = directionNorm.y;
		matrix._23 = directionNorm.z;
		matrix._31 = normal.x;
		matrix._32 = normal.y;
		matrix._33 = normal.z;
		matrix._41 = origin.x;
		matrix._42 = origin.y;
		matrix._43 = origin.z;
		matrix._44 = 1.0f;

		auto outMiddle2D = Vector2(outMiddle);
		bool result = SolveIK2D(Vector2(Vector3::Transform(target, matrix.Invert())), length0, length1, outMiddle2D);

		outMiddle = Vector3(outMiddle2D);
		outMiddle = Vector3::Transform(outMiddle, matrix);

		return result;
	}
}
