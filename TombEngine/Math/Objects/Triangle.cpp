#include "framework.h"
#include "Math/Objects/Triangle.h"

#include "Math/Constants.h"

namespace TEN::Math
{
	Triangle::Triangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2)
	{
		Vertex0 = vertex0;
		Vertex1 = vertex1;
		Vertex2 = vertex2;
	}

	bool Triangle::Intersects(const Ray& ray, float& dist) const
	{
		// Calculate edge vectors.
		auto edge0 = Vertex1 - Vertex0;
		auto edge1 = Vertex2 - Vertex0;

		// Calculate normal.
		auto normal = edge0.Cross(edge1);
		normal.Normalize();

		// Calculate determinant.
		auto h = ray.direction.Cross(edge1);
		float det = edge0.Dot(h);

		// Test if ray and triangle are parallel.
		if (det > -EPSILON && det < EPSILON)
			return false;

		float invDet = 1.0f / det;

		// Calculate barycentric coordinates.
		float s = (ray.position - Vertex0).Dot(h) * invDet;
		if (s < 0.0f || s > 1.0f)
			return false;

		auto q = (ray.position - Vertex0).Cross(edge0);
		float t = ray.direction.Dot(q) * invDet;
		if (t < 0.0f || s + t > 1.0f)
			return false;

		// Calculate distance along ray to intersection point.
		dist = edge1.Dot(q) * invDet;
		return true;
	}
}
