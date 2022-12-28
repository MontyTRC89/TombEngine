#include "framework.h"
#include "Math/Solvers.h"

#include "Math/Constants.h"
#include "Math/Legacy.h"

namespace TEN::Math::Solvers
{
	std::pair<float, float> SolveQuadratic(float a, float b, float c)
	{
		auto solution = INVALID_QUADRATIC_SOLUTION;

		if (abs(a) < FLT_EPSILON)
		{
			if (abs(b) < FLT_EPSILON)
				return solution; // Zero solutions.

			solution.first = -c / b;
			solution.second = solution.first;
			return solution; // One solution.
		}

		float discriminant = SQUARE(b) - (4.0f * a * c);
		if (discriminant < 0.0f)
			return solution; // Zero solutions.

		float inv2a = 1.0f / (2.0f * a);

		if (discriminant < FLT_EPSILON)
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

	bool SolveIK2D(const Vector2& target, float length0, float length1, Vector2& middle)
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

		if (quadratic != INVALID_QUADRATIC_SOLUTION)
		{
			middle.x = flipXY ? quadratic.second : (m - (n * quadratic.second));
			middle.y = flipXY ? (m - (n * quadratic.second)) : quadratic.second;
			return true;
		}

		middle = target * (length0 / length);
		return false;
	}

	IK3DSolution SolveIK3D(const Vector3& origin, const Vector3 target, const Vector3& pole, float length0, float length1)
	{
		auto solution = IK3DSolution();

		// Define key variables.
		float totalLength = length0 + length1;
		float ikDistance = Vector3::Distance(origin, target);
		auto ikDirection = target - origin;
		ikDirection.Normalize();

		// If necessary, scale the target position to make it reachable.
		auto scaledTarget = target;
		if (ikDistance > totalLength)
		{
			scaledTarget = origin + (ikDirection * totalLength);
			ikDistance = totalLength;
		}

		// Define distances.
		float a = length0;
		float b = length1;
		float c = ikDistance;

		// Calculate angles between points.
		float startMiddleAngle = acos((SQUARE(b) + SQUARE(c) - SQUARE(a)) / (2.0f * b * c));
		float middleEndAngle = acos((SQUARE(a) + SQUARE(c) - SQUARE(b)) / (2.0f * a * c));
		//float startEndAngle = acos((SQUARE(a) + SQUARE(b) - SQUARE(c)) / (2.0f * a * b));

		// Store relative Xangles of joints.
		solution.OrientA.x = FROM_RAD(startMiddleAngle);
		solution.OrientB.x = FROM_RAD(-middleEndAngle);

		// Store the base joint position.
		solution.Base = origin;

		auto poleDirection = pole - origin;
		poleDirection.Normalize();

		// Calculate the cross product of the ikDirection and pole direction vectors to get a vector perpendicular to both
		auto perp = ikDirection.Cross(poleDirection);

		// Calculate the middle joint position and store it.
		// Rotating it around the perp vector by the angle between the base joint and the middle joint
		solution.Middle = origin + (ikDirection * length0);
		solution.Middle = solution.Middle + (perp * (sin(startMiddleAngle) * length0));

		// Calculate the end joint position by moving along the direction vector by the sum of the two lengths
		//solution.End = origin + (direction * totalLength);

		// Store the end joint position.
		solution.End = scaledTarget;

		return solution;
	}
}
