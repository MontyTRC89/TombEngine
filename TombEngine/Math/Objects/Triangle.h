#pragma once

namespace TEN::Math
{
	class Triangle
	{
	public:
		// Members
		Vector3 Vertex0 = Vector3::Zero;
		Vector3 Vertex1 = Vector3::Zero;
		Vector3 Vertex2 = Vector3::Zero;

		// Constructors
		Triangle() {};
		Triangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2);

		// Inquirers
		bool Intersects(const Ray& ray, float& dist) const;
	};
}
