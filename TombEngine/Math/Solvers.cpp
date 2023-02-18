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

	IKSolution2D SolveIK2D(const Vector2& origin, const Vector2& target, float length0, float length1)
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
			middle = origin + (direction * length0);
		}

		return IKSolution2D{ origin, middle, scaledTarget };
	}

	IKSolution3D SolveIK3D(const Vector3& origin, const Vector3& target, const Vector3& pole, float length0, float length1)
	{
		auto direction = (target - origin);
		direction.Normalize();
		auto normal = direction.Cross(pole - origin);
		normal.Normalize();

		// Construct transform matrix.
		auto tMatrix = Matrix(
			Vector4(normal.Cross(direction)),
			Vector4(direction),
			Vector4(normal),
			Vector4(origin.x, origin.y, origin.z, 1.0f));

		// Get relative 2D IK solution.
		auto inverseMatrix = tMatrix.Invert();
		auto relTarget = Vector3::Transform(target, inverseMatrix);
		auto ikSolution2D = SolveIK2D(Vector2::Zero, Vector2(relTarget), length0, length1);

		// Calculate absolute middle position.
		auto relMiddle = Vector3(ikSolution2D.Middle.x, ikSolution2D.Middle.y, 0.0f);
		auto middle = Vector3::Transform(relMiddle, tMatrix);

		// Calculate absolute end position.
		auto localEnd = Vector3(ikSolution2D.End.x, ikSolution2D.End.y, 0.0f);
		auto end = Vector3::Transform(localEnd, tMatrix);

		return IKSolution3D{ origin, middle, end };
	}
}
