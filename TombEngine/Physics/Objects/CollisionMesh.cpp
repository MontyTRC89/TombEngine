#include "framework.h"
#include "Physics/Objects/CollisionMesh.h"

#include "Math/Math.h"
#include "Specific/Structures/BoundingVolumeHierarchy.h"
#include "Specific/trutils.h"

using namespace TEN::Math;
using namespace TEN::Structures;
using namespace TEN::Utils;

namespace TEN::Physics
{
	unsigned int CollisionMeshDesc::GetTriangleCount() const
	{
		return ((unsigned int)_ids.size() / LocalCollisionTriangle::VERTEX_COUNT);
	}

	const std::vector<Vector3>& CollisionMeshDesc::GetVertices() const
	{
		return _vertices;
	}

	const std::vector<int>& CollisionMeshDesc::GetIds() const
	{
		return _ids;
	}

	void CollisionMeshDesc::InsertTriangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2)
	{
		auto getVertexID = [&](const Vector3& vertex)
		{
			// Get existing vertex ID.
			auto it = _vertexMap.find(vertex);
			if (it != _vertexMap.end())
			{
				int vertexID = it->second;
				return vertexID;
			}

			// Add and cache new vertex.
			int vertexID = (int)_vertices.size();
			_vertices.push_back(vertex);
			_vertexMap[vertex] = vertexID;

			// Get new vertex ID.
			return vertexID;
		};

		// Allocate vertices.
		int vertex0ID = getVertexID(vertex0);
		int vertex1ID = getVertexID(vertex1);
		int vertex2ID = getVertexID(vertex2);

		// Add vertex IDs.
		_ids.push_back(vertex0ID);
		_ids.push_back(vertex1ID);
		_ids.push_back(vertex2ID);
	}

	// Check if 3 vertices form convex angle.
	bool IsConvex(const Vector3& a, const Vector3& b, const Vector3& c, const Plane& plane)
	{
		auto cross = (b - a).Cross(c - b);
		return (cross.Dot(plane.Normal()) > 0.0f); // Ensure consistent winding.
	}

	void TriangulateMonotonePolygon(const std::vector<int>& polygon, std::vector<int>& optimizedIds, const std::vector<Vector3>& vertices, const Plane& plane)
	{
		if (polygon.size() < 3)
			return;

		// Sort vertices by projection onto dominant plane axis.
		auto sortedPolygon = polygon;
		std::sort(sortedPolygon.begin(), sortedPolygon.end(), [&](int a, int b)
		{
			return vertices[a].Dot(plane.Normal()) < vertices[b].Dot(plane.Normal());
		});

		auto stack = std::vector<int>{};
		stack.push_back(sortedPolygon[0]);
		stack.push_back(sortedPolygon[1]);

		for (int i = 2; i < sortedPolygon.size(); i++)
		{
			int top = stack.back();
			stack.pop_back();

			while (!stack.empty() && IsConvex(vertices[stack.back()], vertices[top], vertices[sortedPolygon[i]], plane))
			{
				optimizedIds.push_back(stack.back());
				optimizedIds.push_back(top);
				optimizedIds.push_back(sortedPolygon[i]);
				top = stack.back();
				stack.pop_back();
			}

			stack.push_back(top);
			stack.push_back(sortedPolygon[i]);
		}
	}

	// TODO: Not working right.
	void CollisionMeshDesc::Optimize()
	{
		return;
		constexpr auto PLANE_HEIGHT_STEP = BLOCK(1 / 32.0f);

		using VertexArray = std::array<int, LocalCollisionTriangle::VERTEX_COUNT>; // Vertex IDs of vertices defining triangle.
		using EdgePair	  = std::pair<int, int>;								   // Vertex IDs of vertices defining edge.

		// 1) Group coplanar triangles.
		auto coplanarTriMap = std::unordered_map<Plane, std::vector<VertexArray>>{};
		for (int i = 0; i < _ids.size(); i += LocalCollisionTriangle::VERTEX_COUNT)
		{
			// Get vertices.
			const auto& vertex0 = _vertices[_ids[i]];
			const auto& vertex1 = _vertices[_ids[i + 1]];
			const auto& vertex2 = _vertices[_ids[i + 2]];

			// Calculate edges.
			auto edge0 = vertex1 - vertex0;
			auto edge1 = vertex2 - vertex0;

			// Calculate plane normal. TODO: Should be rounded to account for floating-point imprecision?
			auto normal = edge0.Cross(edge1);
			normal.Normalize();

			// Calculate plane distance. NOTE: Rounded to discrete steps to account for floating-point imprecision.
			float dist = RoundToStep(normal.Dot(vertex0), PLANE_HEIGHT_STEP);

			// Collect triangles by plane.
			auto plane = Plane(normal, dist);
			coplanarTriMap[plane].push_back({ _ids[i], _ids[i + 1], _ids[i + 2] });
		}

		// TEST: Check triangle groupings.
		/*auto optimizedIds2 = std::vector<int>{};
		for (auto& [plane, tris] : trisByPlane)
		{
			if (tris.size() > 1)
				tris.erase(tris.begin() + 1, tris.end());

			optimizedIds2.push_back(tris[0][0]);
			optimizedIds2.push_back(tris[0][1]);
			optimizedIds2.push_back(tris[0][2]);
		}
		_ids = std::move(optimizedIds2);
		return;*/

		// 2) Process coplanar triangles.
		auto optimizedIds = std::vector<int>{};
		for (const auto& [plane, tris] : coplanarTriMap)
		{
			// 2.1) Count shared edges and track vertex adjacency.
			auto edgeCountMap = std::unordered_map<EdgePair, int, PairHash>{};
			auto adjacentVertexMap = std::unordered_map<int, std::unordered_set<int>>{};
			for (const auto& tri : tris)
			{
				// Run through vertices.
				for (int i = 0; i < LocalCollisionTriangle::VERTEX_COUNT; i++)
				{
					// Get edge vertices.
					int vertexID0 = tri[i];
					int vertexID1 = tri[(i + 1) % LocalCollisionTriangle::VERTEX_COUNT];

					// Order ID pairs in sequence.
					if (vertexID0 > vertexID1)
						std::swap(vertexID0, vertexID1);

					// Update edge count map.
					auto edgePair = EdgePair{ vertexID0, vertexID1 };
					edgeCountMap[edgePair]++;

					// Update adjacent vertex map.
					adjacentVertexMap[vertexID0].insert(vertexID1);
					adjacentVertexMap[vertexID1].insert(vertexID0);
				}
			}

			// 2.2) Extract boundary edges.
			auto boundaryEdges = std::vector<EdgePair>{};
			for (const auto& [edge, count] : edgeCountMap)
			{
				if (count <= 1)
					boundaryEdges.push_back(edge);
			}

			// 2.3) Order boundary edges into closed polygon.
			auto polygon = std::vector<int>{};
			if (!boundaryEdges.empty())
			{
				polygon.push_back(boundaryEdges.front().first);
				polygon.push_back(boundaryEdges.front().second);
				boundaryEdges.erase(boundaryEdges.begin());

				while (!boundaryEdges.empty())
				{
					bool isFound = false;
					for (auto it = boundaryEdges.begin(); it != boundaryEdges.end(); it++)
					{
						if (it->first == polygon.back())
						{
							polygon.push_back(it->second);
							boundaryEdges.erase(it);
							isFound = true;
							break;
						}
						else if (it->second == polygon.back())
						{
							polygon.push_back(it->first);
							boundaryEdges.erase(it);
							isFound = true;
							break;
						}
					}

					// FAILSAFE: Prevent infinite loop if no matching edge found.
					if (!isFound)
						break;
				}
			}

			// 2.4) Triangulate polygon using optimized monotone approach.
			//TriangulateMonotonePolygon(polygon, optimizedIds, _vertices, plane);

			// 2.4) Triangulate polygon using Ear Clipping.
			if (polygon.size() >= 3)
			{
				auto remaining = polygon;
				while (remaining.size() > 2)
				{
					for (int i = 0; i < remaining.size(); i++)
					{
						int prev = remaining[(i + remaining.size() - 1) % remaining.size()];
						int current = remaining[i];
						int next = remaining[(i + 1) % remaining.size()];

						// Ensure correct winding order.
						auto faceNormal = (_vertices[prev] - _vertices[current]).Cross(_vertices[next] - _vertices[current]);
						if (faceNormal.Dot(plane.Normal()) > 0.0f)
							std::swap(prev, next);

						optimizedIds.push_back(prev);
						optimizedIds.push_back(current);
						optimizedIds.push_back(next);
						remaining.erase(remaining.begin() + i);
						break;
					}
				}
			}
		}

		_ids = std::move(optimizedIds);
	}

	LocalCollisionTriangle::LocalCollisionTriangle(int vertex0ID, int vertex1ID, int vertex2ID)
	{
		_vertexIds[0] = vertex0ID;
		_vertexIds[1] = vertex1ID;
		_vertexIds[2] = vertex2ID;
	}

	const Vector3& LocalCollisionTriangle::GetVertex0(const std::vector<Vector3>& vertices) const
	{
		return vertices[_vertexIds[0]];
	}

	const Vector3& LocalCollisionTriangle::GetVertex1(const std::vector<Vector3>& vertices) const
	{
		return vertices[_vertexIds[1]];
	}

	const Vector3& LocalCollisionTriangle::GetVertex2(const std::vector<Vector3>& vertices) const
	{
		return vertices[_vertexIds[2]];
	}

	Vector3 LocalCollisionTriangle::GetNormal(const std::vector<Vector3>& vertices) const
	{
		// Get vertices.
		const auto& vertex0 = GetVertex0(vertices);
		const auto& vertex1 = GetVertex1(vertices);
		const auto& vertex2 = GetVertex2(vertices);

		// Calculate edges.
		auto edge0 = vertex1 - vertex0;
		auto edge1 = vertex2 - vertex0;

		// Calculate edge normal.
		auto normal = edge0.Cross(edge1);
		normal.Normalize();
		return normal;
	}

	BoundingBox LocalCollisionTriangle::GetAabb(const std::vector<Vector3>& vertices) const
	{
		// Get vertices.
		const auto& vertex0 = GetVertex0(vertices);
		const auto& vertex1 = GetVertex1(vertices);
		const auto& vertex2 = GetVertex2(vertices);

		return Geometry::GetBoundingBox({ vertex0, vertex1, vertex2 });
	}

	bool LocalCollisionTriangle::Intersects(const Ray& ray, float& dist, const std::vector<Vector3>& vertices) const
	{
		// Get normal.
		const auto& normal = GetNormal(vertices);

		// Test if ray is facing triangle.
		if (ray.direction.Dot(normal) > EPSILON)
			return false;

		// Get vertices.
		const auto& vertex0 = GetVertex0(vertices);
		const auto& vertex1 = GetVertex1(vertices);
		const auto& vertex2 = GetVertex2(vertices);

		// Calculate edges.
		auto edge0 = vertex1 - vertex0;
		auto edge1 = vertex2 - vertex0;

		// Calculate determinant.
		auto dirCrossEdge1 = ray.direction.Cross(edge1);
		float det = edge0.Dot(dirCrossEdge1);

		// Test if ray and triangle are parallel.
		if (abs(det) < EPSILON)
			return false;

		float invDet = 1.0f / det;

		// Calculate barycentric coordinates.
		float baryCoordVert0 = (ray.position - vertex0).Dot(dirCrossEdge1) * invDet;
		if (baryCoordVert0 < 0.0f || baryCoordVert0 > 1.0f)
			return false;

		auto rayToVert0CrossEdge0 = (ray.position - vertex0).Cross(edge0);
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

	void LocalCollisionTriangle::DrawDebug(const Matrix& transformMatrix, const Matrix& rotMatrix, const std::vector<Vector3>& vertices) const
	{
		constexpr auto TRI_SURF_COLOR	 = Color(1.0f, 1.0f, 0.0f, 0.1f);
		constexpr auto TRI_OUTLINE_COLOR = Color(1.0f, 1.0f, 1.0f, 0.25f);
		constexpr auto NORMAL_LENGTH	 = BLOCK(0.2f);
		constexpr auto NORMAL_COLOR		 = Color(1.0f, 1.0f, 1.0f);

		// Get vertices.
		auto vertex0 = Vector3::Transform(GetVertex0(vertices), transformMatrix);
		auto vertex1 = Vector3::Transform(GetVertex1(vertices), transformMatrix);
		auto vertex2 = Vector3::Transform(GetVertex2(vertices), transformMatrix);

		// Get normal.
		auto normal = Vector3::Transform(GetNormal(vertices), rotMatrix);

		// Draw triangle surface.
		DrawDebugTriangle(vertex0, vertex1, vertex2, TRI_SURF_COLOR);

		// Draw triangle outline.
		DrawDebugLine(vertex0, vertex1, TRI_OUTLINE_COLOR);
		DrawDebugLine(vertex1, vertex2, TRI_OUTLINE_COLOR);
		DrawDebugLine(vertex2, vertex0, TRI_OUTLINE_COLOR);

		// Draw normal.
		auto center = (vertex0 + vertex1 + vertex2) / VERTEX_COUNT;
		DrawDebugLine(center, Geometry::TranslatePoint(center, normal, NORMAL_LENGTH), NORMAL_COLOR);
	}

	CollisionMesh::CollisionMesh(const Vector3& pos, const Quaternion& orient, const CollisionMeshDesc& desc)
	{
		_position = pos;
		_orientation = orient;
		_vertices = desc.GetVertices();

		// Add triangles.
		for (int i = 0; i < desc.GetIds().size(); i += LocalCollisionTriangle::VERTEX_COUNT)
			_triangles.push_back(LocalCollisionTriangle(desc.GetIds()[i], desc.GetIds()[i + 1], desc.GetIds()[i + 2]));

		// Collect triangle IDs and AABBs.
		auto triIds = std::vector<int>(_triangles.size());
		auto triAabbs = std::vector<BoundingBox>(_triangles.size());
		for (int i = 0; i < _triangles.size(); i++)
		{
			const auto& tri = _triangles[i];

			triIds.push_back(i);
			triAabbs.push_back(tri.GetAabb(_vertices));
		}

		// Create triangle tree.
		_triangleTree = Bvh(triIds, triAabbs);
	}

	std::optional<CollisionMeshRayCollisionData> CollisionMesh::GetCollision(const Ray& ray, float dist) const
	{
		// Get matrices.
		auto rotMatrix = GetRotationMatrix();
		auto transformMatrix = GetTranslationMatrix() * rotMatrix;

		// Calculate local ray.
		auto localRay = Ray(Vector3::Transform(ray.position, transformMatrix.Invert()), Vector3::Transform(ray.direction, rotMatrix.Invert()));

		// Get bounded triangle IDs.
		auto triIds = _triangleTree.GetBoundedObjectIds(localRay, dist);
		if (triIds.empty())
			return std::nullopt;

		const LocalCollisionTriangle* closestTri = nullptr;
		float closestDist = dist;

		// Find closest triangle.
		for (int triID : triIds)
		{
			const auto& tri = _triangles[triID];

			float intersectDist = 0.0f;
			if (tri.Intersects(localRay, intersectDist, _vertices) && intersectDist <= closestDist)
			{
				closestTri = &tri;
				closestDist = intersectDist;
			}
		}

		// No triangle collision; return early.
		if (closestTri == nullptr)
			return std::nullopt;

		// Create and return ray-mesh collision.
		auto meshColl = CollisionMeshRayCollisionData{};
		meshColl.Triangle.Vertices =
		{
			Vector3::Transform(closestTri->GetVertex0(_vertices), transformMatrix),
			Vector3::Transform(closestTri->GetVertex1(_vertices), transformMatrix),
			Vector3::Transform(closestTri->GetVertex2(_vertices), transformMatrix)
		};
		meshColl.Triangle.Normal = Vector3::Transform(closestTri->GetNormal(_vertices), rotMatrix);
		meshColl.Distance = closestDist;
		return meshColl;
	}

	void CollisionMesh::SetPosition(const Vector3& pos)
	{
		_position = pos;
	}

	void CollisionMesh::SetOrientation(const Quaternion& orient)
	{
		_orientation = orient;
	}

	void CollisionMesh::DrawDebug() const
	{
		auto rotMatrix = GetRotationMatrix();
		auto transformMatrix = GetTranslationMatrix() * rotMatrix;

		for (const auto& tri : _triangles)
			tri.DrawDebug(transformMatrix, rotMatrix, _vertices);
	}

	Matrix CollisionMesh::GetTranslationMatrix() const
	{
		return Matrix::CreateTranslation(_position);
	}

	Matrix CollisionMesh::GetRotationMatrix() const
	{
		return Matrix::CreateFromQuaternion(_orientation);
	}
}
