#include "framework.h"
#include "Math/Objects/CollisionMesh.h"

#include "Math/Constants.h"
#include "Math/Geometry.h"

namespace TEN::Math
{
	CollisionTriangle::CollisionTriangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& normal)
	{
		_vertices = std::array<const Vector3*, VERTEX_COUNT>{ &vertex0, &vertex1, &vertex2 };
		_normal = normal;
		_box = Geometry::GetBoundingBox(std::vector<Vector3>{ vertex0, vertex1, vertex2 });
	}

	const std::array<const Vector3*, CollisionTriangle::VERTEX_COUNT>& CollisionTriangle::GetVertices() const
	{
		return _vertices;
	}

	const Vector3& CollisionTriangle::GetNormal() const
	{
		return _normal;
	}

	const BoundingBox& CollisionTriangle::GetBox() const
	{
		return _box;
	}

	bool CollisionTriangle::Intersects(const Ray& ray, float& dist) const
	{
		// Test if ray intersects triangle AABB.
		float boxDist = 0.0f;
		if (!ray.Intersects(_box, boxDist))
			return false;

		// Calculate edge vectors.
		auto edge0 = *_vertices[1] - *_vertices[0];
		auto edge1 = *_vertices[2] - *_vertices[0];

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
		float baryCoordVert0 = (ray.position - *_vertices[0]).Dot(dirCrossEdge1) * invDet;
		if (baryCoordVert0 < 0.0f || baryCoordVert0 > 1.0f)
			return false;

		auto rayToVert0CrossEdge0 = (ray.position - *_vertices[0]).Cross(edge0);
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

	bool CollisionMesh::BvhNode::IsLeaf() const
	{
		return (LeftChildIndex == NO_VALUE && RightChildIndex == NO_VALUE);
	}

	CollisionMesh::Bvh::Bvh(const std::vector<CollisionTriangle>& tris)
	{
		auto triIndices = std::vector<int>{};
		triIndices.reserve(tris.size());
		for (int i = 0; i < tris.size(); ++i)
			triIndices.push_back(i);
		
		Build(0, (int)tris.size(), triIndices, tris);
	}

	int CollisionMesh::Bvh::Build(int start, int end, const std::vector<int>& triIndices, const std::vector<CollisionTriangle>& tris)
	{
		constexpr auto TRI_COUNT_PER_LEAF_MAX = 4;

		// FAILSAFE.
		if (start >= end)
			return NO_VALUE;

		auto node = BvhNode{};

		// Combine boxes.
		node.Box = tris[triIndices[start]].GetBox();
		for (int i = start; i < end; i++)
			node.Box = Geometry::CombineBoundingBoxes(node.Box, tris[triIndices[i]].GetBox());

		if ((end - start) <= TRI_COUNT_PER_LEAF_MAX)
		{
			node.TriangleIndices.insert(node.TriangleIndices.end(), triIndices.begin() + start, triIndices.begin() + end);
			Nodes.push_back(node);
			return int(Nodes.size() - 1);
		}

		int mid = (start + end) / 2;
		node.LeftChildIndex = Build(start, mid, triIndices, tris);
		node.RightChildIndex = Build(mid, end, triIndices, tris);
		Nodes.push_back(node);

		return int(Nodes.size() - 1);
	}

	std::optional<CollisionMeshCollisionData> CollisionMesh::Bvh::GetIntersection(const Ray& ray, const std::vector<CollisionTriangle>& tris) const
	{
		if (Nodes.empty())
			return std::nullopt;

		const CollisionTriangle* closestTri = nullptr;
		float closestDist = INFINITY;
		bool isIntersected = false;

		std::function<void(int)> traverseBvh = [&](int nodeIndex)
		{
			// Invalid node; return early.
			if (nodeIndex == NO_VALUE)
				return;

			const auto& node = Nodes[nodeIndex];

			// Test node intersection.
			float dist = 0.0f;
			if (!node.Box.Intersects(ray.position, ray.direction, dist))
				return;

			// Traverse nodes.
			if (node.IsLeaf())
			{
				for (int triIndex : node.TriangleIndices)
				{
					float dist = 0.0f;
					if (tris[triIndex].Intersects(ray, dist) && dist < closestDist)
					{
						closestTri = &tris[triIndex];
						closestDist = dist;
						isIntersected = true;
					}
				}
			}
			else
			{
				traverseBvh(node.LeftChildIndex);
				traverseBvh(node.RightChildIndex);
			}
		};

		traverseBvh(0);

		if (isIntersected)
			return CollisionMeshCollisionData{ *closestTri, closestDist };

		return std::nullopt;
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

	std::optional<CollisionMeshCollisionData> CollisionMesh::GetIntersection(const Ray& ray) const
	{
		return _bvh.GetIntersection(ray, _triangles);
	}

	void CollisionMesh::InsertTriangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& normal)
	{
		const auto& insertedVertex0 = *_vertices.insert(vertex0).first;
		const auto& insertedVertex1 = *_vertices.insert(vertex1).first;
		const auto& insertedVertex2 = *_vertices.insert(vertex2).first;

		_triangles.push_back(CollisionTriangle(insertedVertex0, insertedVertex1, insertedVertex2, normal));
	}

	void CollisionMesh::UpdateBvh()
	{
		_bvh = CollisionMesh::Bvh(_triangles);
	}
}
