#pragma once

namespace TEN::Math
{
	class CollisionTriangle
	{
	private:
		// Constants
		static constexpr auto VERTEX_COUNT = 3;

		// Members
		std::array<Vector3, VERTEX_COUNT> _vertices = {};
		BoundingBox						  _box		= BoundingBox();

	public:
		// Constructors
		CollisionTriangle();
		CollisionTriangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2);

		// Getters
		const std::array<Vector3, VERTEX_COUNT>& GetVertices() const;

		// Inquirers
		bool Intersects(const Ray& ray, float& dist) const;
	};

	class CollisionMesh
	{
	private:
		// Members
		std::vector<CollisionTriangle> _triangles = {};

	public:
		// Constructors
		CollisionMesh();
		CollisionMesh(const std::vector<CollisionTriangle>& tris);

		// Inquireres
		bool Intersects(const Ray& ray, float& dist) const;
	};
}
