#pragma once

namespace TEN::Math
{
	class TriangleMesh;

	class CollisionMesh
	{
	private:
		// Members
		std::vector<TriangleMesh> _triangles = {};

	public:
		// Constructors
		CollisionMesh();
		CollisionMesh(const std::vector<TriangleMesh>& tris);

		// Inquireres
		bool Intersects(const Ray& ray, float& dist) const;
	};
}
