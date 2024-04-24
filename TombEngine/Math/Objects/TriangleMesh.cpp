#include "framework.h"
#include "Math/Objects/TriangleMesh.h"

#include "Math/Constants.h"

namespace TEN::Math
{
	TriangleMesh::TriangleMesh()
	{
	}

	TriangleMesh::TriangleMesh(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2)
	{
		Vertices[0] = vertex0;
		Vertices[1] = vertex1;
		Vertices[2] = vertex2;
	}

	bool TriangleMesh::Intersects(const Ray& ray, float& dist) const
	{
		// Calculate edge vectors.
		auto edge0 = Vertices[1] - Vertices[0];
		auto edge1 = Vertices[2] - Vertices[0];

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
		float baryCoordVert0 = (ray.position - Vertices[0]).Dot(dirCrossEdge1) * invDet;
		if (baryCoordVert0 < 0.0f || baryCoordVert0 > 1.0f)
			return false;

		auto rayToVert0CrossEdge0 = (ray.position - Vertices[0]).Cross(edge0);
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
}
