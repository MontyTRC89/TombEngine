#pragma once

namespace TEN::Math
{
	class TriangleMesh
	{
	private:
		static constexpr auto VERTEX_COUNT = 3;

	public:
		// Members
		std::array<Vector3, VERTEX_COUNT> Vertices = {};

		// Constructors
		TriangleMesh();
		TriangleMesh(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2);

		// Inquirers
		bool Intersects(const Ray& ray, float& dist) const;
	};
}
