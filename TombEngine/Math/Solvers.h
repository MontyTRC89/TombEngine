#pragma once

using std::pair;

namespace TEN::Math::Solver
{
	pair<float, float> SolveQuadratic(float a, float b, float c);

	bool SolveIK2D(const Vector2& target, float length0, float length1, Vector2& middle);
	bool SolveIK3D(const Vector3& origin, const Vector3& target, const Vector3& pole, float length0, float length1, Vector3& middle);
}
