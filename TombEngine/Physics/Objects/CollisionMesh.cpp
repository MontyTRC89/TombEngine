#include "framework.h"
#include "Physics/Objects/CollisionMesh.h"

#include "Math/Math.h"
#include "Specific/Structures/StaticBoundingVolumeHierarchy.h"
#include "Specific/trutils.h"

using namespace TEN::Math;
using namespace TEN::Structures;
using namespace TEN::Utils;

namespace TEN::Physics
{
	CollisionTriangle::CollisionTriangle(int vertexID0, int vertexID1, int vertexID2, const Vector3& normal, const BoundingBox& box, int portalRoomNumber)
	{
		_vertexIds = std::array<int, VERTEX_COUNT>{ vertexID0, vertexID1, vertexID2 };
		_normal = normal;
		_aabb = box;
		_portalRoomNumber = portalRoomNumber;
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

	bool CollisionTriangle::Intersects(const std::vector<Vector3>& vertices, const Ray& ray, float& dist) const
	{
		// Test if ray intersects triangle AABB.
		float boxDist = 0.0f;
		if (!ray.Intersects(_aabb, boxDist))
			return false;

		// Test if ray is facing triangle.
		if (ray.direction.Dot(_normal) > EPSILON)
			return false;

		// Calculate edge vectors.
		auto edge0 = vertices[_vertexIds[1]] - vertices[_vertexIds[0]];
		auto edge1 = vertices[_vertexIds[2]] - vertices[_vertexIds[0]];

		// Calculate determinant.
		auto dirCrossEdge1 = ray.direction.Cross(edge1);
		float det = edge0.Dot(dirCrossEdge1);

		// Test if ray and triangle are parallel.
		if (abs(det) < EPSILON)
			return false;

		float invDet = 1.0f / det;

		// Calculate barycentric coordinates.
		float baryCoordVert0 = (ray.position - vertices[_vertexIds[0]]).Dot(dirCrossEdge1) * invDet;
		if (baryCoordVert0 < 0.0f || baryCoordVert0 > 1.0f)
			return false;

		auto rayToVert0CrossEdge0 = (ray.position - vertices[_vertexIds[0]]).Cross(edge0);
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

	bool CollisionTriangle::IsPortal() const
	{
		return (_portalRoomNumber != NO_VALUE);
	}

	void CollisionTriangle::DrawDebug(const std::vector<Vector3>& vertices) const
	{
		const auto& vertex0 = vertices[_vertexIds[0]];
		const auto& vertex1 = vertices[_vertexIds[1]];
		const auto& vertex2 = vertices[_vertexIds[2]];
		DrawDebugTriangle(vertex0, vertex1, vertex2, Color(1.0f, IsPortal() ? 0.0f : 1.0f, 0.0f, 0.2f), RendererDebugPage::RoomMeshStats);

		auto center = (vertex0 + vertex1 + vertex2) / 3;
		DrawDebugLine(center, Geometry::TranslatePoint(center, _normal, BLOCK(0.2f)), Color(1.0f, IsPortal() ? 0.0f : 1.0f, 0.0f), RendererDebugPage::RoomMeshStats);
	}

	CollisionMesh::Bvh::Bvh(const std::vector<CollisionTriangle>& tris)
	{
		auto ids = std::vector<int>{};
		auto aabbs = std::vector<BoundingBox>{};

		ids.reserve(tris.size());
		aabbs.reserve(tris.size());

		int i = 0;
		for (const auto& tri : tris)
		{
			ids.push_back(i);
			aabbs.push_back(tri.GetAabb());

			i++;
		}

		Generate(ids, aabbs, 0, (int)tris.size());
	}

	std::optional<CollisionMeshCollisionData> CollisionMesh::Bvh::GetCollision(const Ray& ray, float dist,
																			   const std::vector<CollisionTriangle>& tris,
																			   const std::vector<Vector3>& vertices) const
	{
		if (_nodes.empty())
			return std::nullopt;

		const CollisionTriangle* closestTri = nullptr;
		float closestDist = INFINITY;
		bool isIntersected = false;

		std::function<void(int)> traverseBvh = [&](int nodeID)
		{
			// Invalid node; return early.
			if (nodeID == NO_VALUE)
				return;

			const auto& node = _nodes[nodeID];

			// Test node intersection.
			float intersectDist = 0.0f;
			if (!node.Aabb.Intersects(ray.position, ray.direction, intersectDist) || intersectDist > dist)
				return;

			// Traverse nodes.
			if (node.IsLeaf())
			{
				for (int id : node.Ids)
				{
					float intersectDist = 0.0f;
					if (tris[id].Intersects(vertices, ray, intersectDist) &&
						intersectDist <= dist && intersectDist < closestDist)
					{
						closestTri = &tris[id];
						closestDist = intersectDist;
						isIntersected = true;
					}
				}
			}
			else
			{
				traverseBvh(node.LeftChildID);
				traverseBvh(node.RightChildID);
			}
		};
		
		// Traverse BVH from root node.
		traverseBvh((int)_nodes.size() - 1);

		if (isIntersected)
			return CollisionMeshCollisionData{ *closestTri, closestDist };

		return std::nullopt;
	}

	void CollisionMesh::Bvh::DrawDebug() const
	{
		constexpr auto BOX_COLOR = Color(1.0f, 1.0f, 1.0f);

		for (const auto& node : _nodes)
			DrawDebugBox(node.Aabb, BOX_COLOR, RendererDebugPage::RoomMeshStats);
	}

	CollisionMesh::CollisionMesh(const std::vector<CollisionTriangle>& tris)
	{
		_triangles = tris;
	}

	std::optional<CollisionMeshCollisionData> CollisionMesh::GetCollision(const Ray& ray, float dist) const
	{
		return _bvh.GetCollision(ray, dist, _triangles, _vertices);
	}

	void CollisionMesh::InsertTriangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& normal, int portalRoomNumber)
	{
		int vertexID0 = NO_VALUE;
		int vertexID1 = NO_VALUE;
		int vertexID2 = NO_VALUE;

		// Add new vertices and get IDs.
		if (!Contains(_vertices, vertex0))
		{
			_vertices.push_back(vertex0);
			vertexID0 = (int)_vertices.size() - 1;
		}
		if (!Contains(_vertices, vertex1))
		{
			_vertices.push_back(vertex1);
			vertexID1 = (int)_vertices.size() - 1;
		}
		if (!Contains(_vertices, vertex2))
		{
			_vertices.push_back(vertex2);
			vertexID2 = (int)_vertices.size() - 1;
		}

		// Get IDs of existing vertices.
		if (vertexID0 == NO_VALUE || vertexID1 == NO_VALUE || vertexID2 == NO_VALUE)
		{
			for (int i = 0; i < _vertices.size(); i++)
			{
				const auto& vertex = _vertices[i];

				if (vertexID0 == NO_VALUE && vertex == vertex0)
					vertexID0 = i;
				if (vertexID1 == NO_VALUE && vertex == vertex1)
					vertexID1 = i;
				if (vertexID2 == NO_VALUE && vertex == vertex2)
					vertexID2 = i;
			}
		}
		
		auto box = Geometry::GetBoundingBox(std::vector<Vector3>{ vertex0, vertex1, vertex2 });
		_triangles.push_back(CollisionTriangle(vertexID0, vertexID1, vertexID2, normal, box, portalRoomNumber));
	}
	
	void CollisionMesh::GenerateBvh()
	{
		_bvh = Bvh(_triangles);
	}

	void CollisionMesh::DrawDebug() const
	{
		for (const auto& tri : _triangles)
		{
			if (tri.IsPortal())
				tri.DrawDebug(_vertices);
		}

		//_bvh.DrawDebug();
	}
}
