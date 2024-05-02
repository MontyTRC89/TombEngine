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
		Vector3							  _normal	= Vector3::Zero;
		BoundingBox						  _box		= BoundingBox();

	public:
		// Constructors
		CollisionTriangle();
		CollisionTriangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2);

		// Getters
		const std::array<Vector3, VERTEX_COUNT>& GetVertices() const;
		const Vector3&							 GetNormal() const;

		// Inquirers
		bool Intersects(const Ray& ray, float& dist) const;
	};

	// TODO
	struct CollisionMeshCollisionData
	{
		const CollisionTriangle& Triangle;
		float Distance = 0.0f;
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

		// Getters
		const std::vector<CollisionTriangle>& GetTriangles() const;

		// Inquireres
		bool Intersects(const Ray& ray, float& dist) const;
	};
}
