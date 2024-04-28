#pragma once

namespace TEN::Math
{
	class TriangleMesh
	{
	private:
		// Constants
		static constexpr auto VERTEX_COUNT = 3;

		// Members
		std::array<Vector3, VERTEX_COUNT> _vertices = {};
		BoundingBox						  _box		= BoundingBox();

	public:
		// Constructors
		TriangleMesh();
		TriangleMesh(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2);

		// Getters
		const std::array<Vector3, VERTEX_COUNT>& GetVertices() const;

		// Inquirers
		bool Intersects(const Ray& ray, float& dist) const;
	};
}
