#include "framework.h"
#include "Physics/Objects/CollisionMesh.h"

#include "Math/Math.h"
#include "Specific/Structures/BoundingTree.h"
#include "Specific/trutils.h"

using namespace TEN::Math;
using namespace TEN::Structures;
using namespace TEN::Utils;

namespace TEN::Physics
{
	CollisionTriangle::CollisionTriangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& normal, const BoundingBox& box, int portalRoomNumber) :
		_vertices(std::array<const Vector3*, VERTEX_COUNT>{ &vertex0, &vertex1, &vertex2 })
	{
		_normal = normal;
		_aabb = box;
		_portalRoomNumber = portalRoomNumber;
	}

	const Vector3& CollisionTriangle::GetVertex0() const
	{
		return *_vertices[0];
	}

	const Vector3& CollisionTriangle::GetVertex1() const
	{
		return *_vertices[1];
	}

	const Vector3& CollisionTriangle::GetVertex2() const
	{
		return *_vertices[2];
	}

	const Vector3& CollisionTriangle::GetNormal() const
	{
		return _normal;
	}

	const BoundingBox& CollisionTriangle::GetAabb() const
	{
		return _aabb;
	}

	int CollisionTriangle::GetPortalRoomNumber() const
	{
		return _portalRoomNumber;
	}

	// TODO: Triangle treated as infinite.
	Vector3 CollisionTriangle::GetTangent(const BoundingSphere& sphere) const
	{
		// Calculate edges.
		auto edge0 = GetVertex1() - GetVertex0();
		auto edge1 = GetVertex2() - GetVertex0();

		// Calculate edge normal.
		auto normal = edge0.Cross(edge1);
		normal.Normalize();

		// Calculate tangent.
		float dist = normal.Dot(sphere.Center - GetVertex0());
		return (sphere.Center - (normal * dist));
	}

	// "More accurate" version but it doesn't work.
	/*Vector3 CollisionTriangle::GetTangent(const std::vector<Vector3>& vertices, const BoundingSphere& sphere) const
	{
		// Get vertices.

		// Calculate edges.
		auto edge0 = GetVertex1() - GetVertex0();
		auto edge1 = GetVertex2() - GetVertex0();

		// Calculate vectors.
		auto v2 = sphere.Center - vertex0;

		// Calculate dot products.
		float dot00 = edge1.Dot(edge1);
		float dot01 = edge1.Dot(edge0);
		float dot02 = edge1.Dot(v2);
		float dot11 = edge0.Dot(edge0);
		float dot12 = edge0.Dot(v2);

		// Calculate barycentric coordinates.
		float invDenom = 1 / ((dot00 * dot11) - (dot01 * dot01));
		float u = ((dot11 * dot02) - (dot01 * dot12)) * invDenom;
		float v = ((dot00 * dot12) - (dot01 * dot02)) * invDenom;

		// Check if point is in triangle
		if (u >= 0 && v >= 0 && (u + v) <= 1)
			return (vertex0 + (edge1 * u) + (edge0 * v));

		// Clamp to nearest edge if point is outside triangle.
		auto c1 = Geometry::GetClosestPointOnLine(vertex0, vertex1, sphere.Center);
		auto c2 = Geometry::GetClosestPointOnLine(vertex1, vertex2, sphere.Center);
		auto c3 = Geometry::GetClosestPointOnLine(vertex2, vertex0, sphere.Center);

		float d1 = (c1 - sphere.Center).Length();
		float d2 = (c2 - sphere.Center).Length();
		float d3 = (c3 - sphere.Center).Length();

		if (d1 < d2 && d1 < d3)
		{
			return c1;
		}
		else if (d2 < d1 && d2 < d3)
		{
			return c2;
		}

		return c3;
	}*/

	bool CollisionTriangle::Intersects(const Ray& ray, float& dist) const
	{
		// Test if ray intersects triangle AABB.
		float boxDist = 0.0f;
		if (!ray.Intersects(_aabb, boxDist))
			return false;

		// Test if ray is facing triangle.
		if (ray.direction.Dot(_normal) > EPSILON)
			return false;

		// Calculate edges.
		auto edge0 = GetVertex1() - GetVertex0();
		auto edge1 = GetVertex2() - GetVertex0();

		// Calculate determinant.
		auto dirCrossEdge1 = ray.direction.Cross(edge1);
		float det = edge0.Dot(dirCrossEdge1);

		// Test if ray and triangle are parallel.
		if (abs(det) < EPSILON)
			return false;

		float invDet = 1.0f / det;

		// Calculate barycentric coordinates.
		float baryCoordVert0 = (ray.position - GetVertex0()).Dot(dirCrossEdge1) * invDet;
		if (baryCoordVert0 < 0.0f || baryCoordVert0 > 1.0f)
			return false;

		auto rayToVert0CrossEdge0 = (ray.position - GetVertex0()).Cross(edge0);
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

	bool CollisionTriangle::Intersects(const BoundingSphere& sphere) const
	{
		// Calculate edges.
		auto edge0 = GetVertex1() - GetVertex0();
		auto edge1 = GetVertex2() - GetVertex0();

		// Calculate edge normal.
		auto normal = edge0.Cross(edge1);
		normal.Normalize();

		// Test intersection.
		float dist = abs(normal.Dot(sphere.Center - GetVertex0()));
		return (dist <= sphere.Radius);
	}

	bool CollisionTriangle::IsPortal() const
	{
		return (_portalRoomNumber != NO_VALUE);
	}

	void CollisionTriangle::DrawDebug() const
	{
		DrawDebugTriangle(GetVertex0(), GetVertex1(), GetVertex2(), Color(1.0f, IsPortal() ? 0.0f : 1.0f, 0.0f, 0.2f), RendererDebugPage::RoomMeshStats);

		auto center = (GetVertex0() + GetVertex1() + GetVertex2()) / 3;
		DrawDebugLine(center, Geometry::TranslatePoint(center, _normal, BLOCK(0.2f)), Color(1.0f, IsPortal() ? 0.0f : 1.0f, 0.0f), RendererDebugPage::RoomMeshStats);
	}

	CollisionMesh::CollisionMesh(const std::vector<CollisionTriangle>& tris)
	{
		_triangles = tris;
	}

	std::optional<CollisionMeshRayCollisionData> CollisionMesh::GetCollision(const Ray& ray, float dist) const
	{
		// Get bounded triangle IDs.
		auto triIds = _tree.GetBoundedObjectIds(ray, dist);
		if (triIds.empty())
			return std::nullopt;

		const CollisionTriangle* closestTri = nullptr;
		float closestDist = INFINITY;
		bool isIntersected = false;

		// Find closest triangle.
		for (int triID : triIds)
		{
			const auto& tri = _triangles[triID];

			float intersectDist = 0.0f;
			if (tri.Intersects(ray, intersectDist) &&
				intersectDist <= dist && intersectDist < closestDist)
			{
				closestTri = &tri;
				closestDist = intersectDist;
				isIntersected = true;
			}
		}

		if (isIntersected)
			return CollisionMeshRayCollisionData{ *closestTri, closestDist };

		return std::nullopt;
	}

	std::optional<CollisionMeshSphereCollisionData> CollisionMesh::GetCollision(const BoundingSphere& sphere) const
	{
		// Get bounded triangle IDs.
		auto triIds = _tree.GetBoundedObjectIds(sphere);
		if (triIds.empty())
			return std::nullopt;

		auto meshColl = CollisionMeshSphereCollisionData{};

		// Collect triangles.
		for (int triID : triIds)
		{
			const auto& tri = _triangles[triID];

			if (!tri.IsPortal() && tri.Intersects(sphere))
			{
				meshColl.Triangles.push_back(&tri);
				meshColl.Tangents.push_back(tri.GetTangent(sphere));
				meshColl.Count++;
			}
		}

		if (meshColl.Count > 0)
			return meshColl;

		return std::nullopt;
	}

	void CollisionMesh::InsertTriangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& normal, int portalRoomNumber)
	{
		int vertex0ID = NO_VALUE;
		int vertex1ID = NO_VALUE;
		int vertex2ID = NO_VALUE;

		// Add new vertices and get IDs.
		if (!Contains(_vertices, vertex0))
		{
			_vertices.push_back(vertex0);
			vertex0ID = (int)_vertices.size() - 1;
		}
		if (!Contains(_vertices, vertex1))
		{
			_vertices.push_back(vertex1);
			vertex1ID = (int)_vertices.size() - 1;
		}
		if (!Contains(_vertices, vertex2))
		{
			_vertices.push_back(vertex2);
			vertex2ID = (int)_vertices.size() - 1;
		}

		// Get IDs of existing vertices.
		if (vertex0ID == NO_VALUE || vertex1ID == NO_VALUE || vertex2ID == NO_VALUE)
		{
			for (int i = 0; i < _vertices.size(); i++)
			{
				const auto& vertex = _vertices[i];

				if (vertex0ID == NO_VALUE && vertex == vertex0)
					vertex0ID = i;
				if (vertex1ID == NO_VALUE && vertex == vertex1)
					vertex1ID = i;
				if (vertex2ID == NO_VALUE && vertex == vertex2)
					vertex2ID = i;
			}
		}
		
		// Add new triangle.
		auto box = Geometry::GetBoundingBox(std::vector<Vector3>{ vertex0, vertex1, vertex2 });
		_triangles.push_back(CollisionTriangle(_vertices[vertex0ID], _vertices[vertex1ID], _vertices[vertex2ID], normal, box, portalRoomNumber));
	}
	
	void CollisionMesh::BuildTree()
	{
		auto ids = std::vector<int>{};
		auto aabbs = std::vector<BoundingBox>{};

		ids.reserve(_triangles.size());
		aabbs.reserve(_triangles.size());

		for (int i = 0; i < _triangles.size(); i++)
		{
			const auto& tri = _triangles[i];

			ids.push_back(i);
			aabbs.push_back(tri.GetAabb());
		}

		_tree = BoundingTree(ids, aabbs);
	}

	void CollisionMesh::DrawDebug() const
	{
		for (const auto& tri : _triangles)
		{
			if (tri.IsPortal())
				tri.DrawDebug();
		}

		if (GetDebugPage() == RendererDebugPage::RoomMeshStats)
			_tree.DrawDebug();
	}
}
