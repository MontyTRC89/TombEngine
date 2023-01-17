#include "framework.h"
#include "Math/Solvers.h"

#include "Math/Constants.h"
#include "Math/Legacy.h"

namespace TEN::Math::Solvers
{
	std::pair<float, float> SolveQuadratic(float a, float b, float c)
	{
		auto solution = INVALID_QUADRATIC_SOLUTION;

		if (abs(a) < EPSILON)
		{
			if (abs(b) < EPSILON)
				return solution; // Zero solutions.

			solution.first = -c / b;
			solution.second = solution.first;
			return solution; // One solution.
		}

		float discriminant = SQUARE(b) - (4.0f * a * c);
		if (discriminant < 0.0f)
			return solution; // Zero solutions.

		float inv2a = 1.0f / (2.0f * a);

		if (discriminant < EPSILON)
		{
			solution.first = -b * inv2a;
			solution.second = solution.first;
			return solution; // One solution.
		}

		discriminant = sqrt(discriminant);
		solution.first = (-b - discriminant) * inv2a;
		solution.second = (-b + discriminant) * inv2a;
		return solution; // Two solutions.
	}

	// Revised version to try.
	IK2DSolution SolveIK2D(const Vector2& origin, const Vector2& target, float length0, float length1)
	{
		auto scaledTarget = target;
		auto maxLength = length0 + length1;
		auto direction = target - origin;
		direction.Normalize();

		// Check if target is within reach.
		float distance = Vector2::Distance(origin, target);
		if (distance > maxLength)
			scaledTarget = origin + (direction * maxLength);

		// Ensure line slope is well defined.
		bool flipXY = (scaledTarget.x < scaledTarget.y);

		float a = flipXY ? (scaledTarget.y - origin.y) : (scaledTarget.x - origin.x);
		float b = flipXY ? (scaledTarget.x - origin.x) : (scaledTarget.y - origin.y);
		assert(abs(a) >= EPSILON);

		float m = ((SQUARE(length0) - SQUARE(length1)) + (SQUARE(a) + SQUARE(b))) / (2.0f * a);
		float n = b / a;

		auto quadratic = SolveQuadratic(1.0f + SQUARE(n), -2.0f * (m * n), SQUARE(m) - SQUARE(length0));
		auto middle = Vector2::Zero;

		// Solution is valid; define middle accurately.
		if (quadratic != INVALID_QUADRATIC_SOLUTION)
		{
			middle = origin + (flipXY ?
				Vector2(quadratic.second, (m - (n * quadratic.second))) :
				Vector2(quadratic.first, (m - (n * quadratic.first))));
		}
		// Solution is invalid: define middle as point on line segment between origin and target.
		else
		{
			middle = origin + (direction * (maxLength / 2));
		}

		return IK2DSolution{ origin, middle, scaledTarget };
	}

	IK3DSolution SolveIK3D(const Vector3& origin, const Vector3& target, const Vector3& pole, float length0, float length1)
	{
		auto direction = (target - origin);
		direction.Normalize();
		auto normal = direction.Cross(pole - origin);
		normal.Normalize();

		Matrix matrix;
		matrix(0, 0) = normal.Cross(direction).x;
		matrix(0, 1) = normal.Cross(direction).y;
		matrix(0, 2) = normal.Cross(direction).z;
		matrix(0, 3) = 0.0f;
		matrix(1, 0) = direction.x;
		matrix(1, 1) = direction.y;
		matrix(1, 2) = direction.z;
		matrix(1, 3) = 0.0f;
		matrix(2, 0) = normal.x;
		matrix(2, 1) = normal.y;
		matrix(2, 2) = normal.z;
		matrix(2, 3) = 0.0f;
		matrix(3, 0) = origin.x;
		matrix(3, 1) = origin.y;
		matrix(3, 2) = origin.z;
		matrix(3, 3) = 1.0f;

		auto inverseMatrix = matrix.Invert();
		auto endInLocalSpace = Vector3::Transform(target, inverseMatrix);
		auto solution2D = SolveIK2D(Vector2::Zero, Vector2(endInLocalSpace), length0, length1);
		auto middleInLocalSpace = Vector3(solution2D.Middle.x, solution2D.Middle.y, 0.0f);
		auto middle = Vector3::Transform(middleInLocalSpace, matrix);

		return IK3DSolution{ origin, middle, target };
	}
}
