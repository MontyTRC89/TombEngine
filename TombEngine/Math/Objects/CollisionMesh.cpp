#include "framework.h"
#include "Math/Objects/CollisionMesh.h"

#include "Math/Constants.h"
#include "Math/Geometry.h"

namespace TEN::Math
{
	CollisionTriangle::CollisionTriangle()
	{
	}

	CollisionTriangle::CollisionTriangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& normal)
	{
		_vertices[0] = vertex0;
		_vertices[1] = vertex1;
		_vertices[2] = vertex2;
		_normal = normal;
		_box = Geometry::GetBoundingBox(std::vector<Vector3>{ _vertices[0], _vertices[1], _vertices[2] });
	}

	const std::array<Vector3, CollisionTriangle::VERTEX_COUNT>& CollisionTriangle::GetVertices() const
	{
		return _vertices;
	}

	const Vector3& CollisionTriangle::GetNormal() const
	{
		return _normal;
	}

	bool CollisionTriangle::Intersects(const Ray& ray, float& dist) const
	{
		// Test if ray intersects triangle AABB.
		float boxDist = 0.0f;
		if (!ray.Intersects(_box, boxDist))
			return false;

		// Calculate edge vectors.
		auto edge0 = _vertices[1] - _vertices[0];
		auto edge1 = _vertices[2] - _vertices[0];

		// Calculate normal.
		auto normal = edge0.Cross(edge1);
		normal.Normalize();

		// Calculate determinant.
		auto dirCrossEdge1 = ray.direction.Cross(edge1);
		float det = edge0.Dot(dirCrossEdge1);

		// Test if ray and triangle are parallel.
		if (abs(det) < EPSILON)
			return false;

		float invDet = 1.0f / det;

		// Calculate barycentric coordinates.
		float baryCoordVert0 = (ray.position - _vertices[0]).Dot(dirCrossEdge1) * invDet;
		if (baryCoordVert0 < 0.0f || baryCoordVert0 > 1.0f)
			return false;

		auto rayToVert0CrossEdge0 = (ray.position - _vertices[0]).Cross(edge0);
		float baryCoordVert1 = ray.direction.Dot(rayToVert0CrossEdge0) * invDet;
		if (baryCoordVert1 < 0.0f || (baryCoordVert0 + baryCoordVert1) > 1.0f)
			return false;

		// Calculate distance along ray to intersection point.
		float intersectDist = edge1.Dot(rayToVert0CrossEdge0) * invDet;
		if (intersectDist < 0.0f)
			return false;

		dist = intersectDist;
		return true;
	}

	CollisionMesh::CollisionMesh()
	{
	}

	CollisionMesh::CollisionMesh(const std::vector<CollisionTriangle>& tris)
	{
		_triangles = tris;
	}

	const std::vector<CollisionTriangle>& CollisionMesh::GetTriangles() const
	{
		return _triangles;
	}

	// NOTE: O(n).
	bool CollisionMesh::Intersects(const Ray& ray, float& dist) const
	{
		// TODO: Vertex indexing, spacial partitioning/BVH tree, and whatever else is necessary to make this performant.
		// TODO: If triangles are made to be collidable only from the "front" as determined by the normal,
		// loop can end early. Best case becomes O(1).

		float closestDist = INFINITY;
		bool isIntersected = false;

		for (const auto& tri : _triangles)
		{
			float intersectDist = 0.0f;
			if (tri.Intersects(ray, intersectDist) && intersectDist < closestDist)
			{
				closestDist = intersectDist;
				isIntersected = true;
			}
		}

		if (isIntersected)
			dist = closestDist;

		return isIntersected;
	}
}
