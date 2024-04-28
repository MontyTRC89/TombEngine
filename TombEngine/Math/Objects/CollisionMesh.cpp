#include "framework.h"
#include "Math/Objects/CollisionMesh.h"

#include "Math/Objects/TriangleMesh.h"

namespace TEN::Math
{
	CollisionMesh::CollisionMesh()
	{
	}

	CollisionMesh::CollisionMesh(const std::vector<TriangleMesh>& tris)
	{
		_triangles = tris;
	}

	bool CollisionMesh::Intersects(const Ray& ray, float& dist) const
	{
		// TODO: Vertex indexing, spacial partitioning, BVH tree, whatever.

		for (const auto& tri : _triangles)
		{
			if (tri.Intersects(ray, dist))
				return true;
		}

		return false;
	}
}
