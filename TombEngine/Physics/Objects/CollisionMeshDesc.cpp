#include "framework.h"
#include "Physics/Objects/CollisionMeshDesc.h"

#include "Math/Math.h"

using namespace TEN::Math;

namespace TEN::Physics
{
	unsigned int CollisionMeshDesc::GetTriangleCount() const
	{
		return ((unsigned int)_ids.size() / VERTEX_COUNT);
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
		auto getVertexId = [&](const Vector3& vertex)
		{
			// Get existing vertex ID.
			auto it = _vertexMap.find(vertex);
			if (it != _vertexMap.end())
			{
				int vertexId = it->second;
				return vertexId;
			}

			// Add and cache new vertex.
			int vertexId = (int)_vertices.size();
			_vertices.push_back(vertex);
			_vertexMap[vertex] = vertexId;

			// Get new vertex ID.
			return vertexId;
		};

		// Allocate vertices.
		int vertexId0 = getVertexId(vertex0);
		int vertexId1 = getVertexId(vertex1);
		int vertexId2 = getVertexId(vertex2);

		// Add vertex IDs.
		_ids.push_back(vertexId0);
		_ids.push_back(vertexId1);
		_ids.push_back(vertexId2);
	}

	// TEMP
	// Check if 3 vertices form convex angle.
	bool IsConvex(const Vector3& a, const Vector3& b, const Vector3& c, const Plane& plane)
	{
		auto cross = (b - a).Cross(c - b);
		return (cross.Dot(plane.Normal()) > 0.0f); // Ensure consistent winding.
	}
	
	// TEMP
	bool PointInTriangle(const Vector3& pt, const Vector3& v0, const Vector3& v1, const Vector3& v2)
	{
		// Compute vectors.
		auto v0v1 = v1 - v0;
		auto v0v2 = v2 - v0;
		auto v0p = pt - v0;

		// Compute dot products.
		float dot00 = v0v1.Dot(v0v1);
		float dot01 = v0v1.Dot(v0v2);
		float dot02 = v0v1.Dot(v0p);
		float dot11 = v0v2.Dot(v0v2);
		float dot12 = v0v2.Dot(v0p);

		// Compute determinant (denominator).
		float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);

		// Compute barycentric coordinates.
		float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
		float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

		// Check if point is inside triangle (barycentric coordinates must be in range [0, 1]).
		return (u >= 0.0f && v >= 0.0f && (u + v) <= 1.0f);
	}
	
	// TODO: Finish sub-function implementations.
	void CollisionMeshDesc::Optimize()
	{
		return;

		// 1) Get coplanar triangle map.
		auto coplanarTriMap = GetCoplanarTriangleMap();

		// 2) Process coplanar triangles into optimized vertex IDs.
		auto optimizedVertexIds = std::vector<int>{};
		for (const auto& [plane, tris] : coplanarTriMap)
		{
			// Get optimal polygons from coplanar triangles.
			auto polygons = GetPolygons(tris);

			// Triangulate polygons.
			for (const auto& polygon : polygons)
				TriangulatePolygon(optimizedVertexIds, polygon, plane);
		}

		// 3) Finalize optimized vertices and IDs.
		auto optimizedVertices = std::vector<Vector3>{};
		auto reoptimizedVertexIds = std::vector<int>{};
		for (int vertexId : optimizedVertexIds)
		{
			const auto& vertex = _vertices[vertexId];

			optimizedVertices.push_back(vertex);
			reoptimizedVertexIds.push_back((int)optimizedVertices.size() - 1);
		}

		// 4) Store optimized vertices and IDs.
		_vertices = std::move(optimizedVertices);
		_ids = std::move(reoptimizedVertexIds);
	}

	std::vector<std::vector<int>> CollisionMeshDesc::GetPolygons(const std::vector<TriangleVertexIds>& tris) const
	{
		// 1) Get key mesh collections.
		auto edgeCountMap = GetEdgeCountMap(tris);
		auto vertexAdjacencyMap = GetVertexAdjacencyMap(tris, edgeCountMap); // TODO: Not used yet.
		auto boundaryEdges = GetBoundaryEdges(edgeCountMap);

		// 2) Process boundary edges into optimal polygon loops.
		auto polygons = std::vector<std::vector<int>>{};
		while (!boundaryEdges.empty())
		{
			auto polygon = std::vector<int>{};

			// TODO:
			// - Split polygon into isolated islands.
			// - Process monotone, irregular, and holed polygons.

			const auto& firstEdgeIt = boundaryEdges.begin();
			const auto& [firstVertexId0, firstVertexId1] = *firstEdgeIt;

			// 2.1) Add first boundary edge.
			polygon.push_back(firstVertexId0);
			polygon.push_back(firstVertexId1);
			boundaryEdges.erase(firstEdgeIt);

			// 2.2) Run through remaining boundary edges.
			while (!boundaryEdges.empty())
			{
				// TODO: Optimise this. O(n) search here is too inefficient.
				// Find matching edge by checking last polygon vertex.
				bool isFound = false;
				for (auto it = boundaryEdges.begin(); it != boundaryEdges.end(); it++)
				{
					const auto& [vertexId0, vertexId1] = *it;

					// If current vertex is equal to the one in polygon, add next vertex.
					if (vertexId0 == polygon.back())
					{
						// Add vertex ID to polygon and remove used edge.
						polygon.push_back(vertexId1);
						boundaryEdges.erase(it);

						isFound = true;
						break;
					}
					else if (vertexId1 == polygon.back())
					{
						// Add vertex ID to polygon and remove used edge.
						polygon.push_back(vertexId0);
						boundaryEdges.erase(it);

						isFound = true;
						break;
					}
				}

				// No matching edge found; finalize polygon.
				if (!isFound)
					break;
			}

			// 2.3) Simplify polygon.
			auto simplifiedPolygon = GetSimplifiedPolygon(polygon);
			//simplifiedPolygon = polygon; // TEMP: Don't use optimised polygon for now.

			// 2.4) Collect valid polygon.
			if (simplifiedPolygon.size() >= VERTEX_COUNT)
				polygons.push_back(std::move(simplifiedPolygon));
		}

		// 3) Return optimal polygons.
		return polygons;
	}

	CollisionMeshDesc::CoplanarTriangleMap CollisionMeshDesc::GetCoplanarTriangleMap() const
	{
		constexpr auto NORMAL_EPSILON	 = 0.0001f;
		constexpr auto PLANE_HEIGHT_STEP = BLOCK(1 / 64.0f);

		// Collect coplanar triangle groups.
		auto coplanarTriMap = std::unordered_map<Plane, std::vector<TriangleVertexIds>>{}; // Key = plane, value = triangles.
		for (int i = 0; i < _ids.size(); i += VERTEX_COUNT)
		{
			// Get vertices.
			const auto& vertex0 = _vertices[_ids[i]];
			const auto& vertex1 = _vertices[_ids[i + 1]];
			const auto& vertex2 = _vertices[_ids[i + 2]];

			// Calculate edges.
			auto edge0 = vertex1 - vertex0;
			auto edge1 = vertex2 - vertex0;

			// Calculate plane normal.
			auto normal = edge0.Cross(edge1);
			normal.Normalize();
			normal = RoundNormal(normal, NORMAL_EPSILON);

			// Calculate plane distance.
			float dist = RoundToStep(normal.Dot(vertex0), PLANE_HEIGHT_STEP);

			// Collect triangles by plane.
			auto plane = Plane(normal, dist);
			coplanarTriMap[plane].push_back({ _ids[i], _ids[i + 1], _ids[i + 2] });
		}

		return coplanarTriMap;
	}

	CollisionMeshDesc::EdgeCountMap CollisionMeshDesc::GetEdgeCountMap(const std::vector<TriangleVertexIds>& tris) const
	{
		// Run through triangles.
		auto edgeCountMap = EdgeCountMap{};
		for (const auto& tri : tris)
		{
			// Run through vertices.
			for (int i = 0; i < VERTEX_COUNT; i++)
			{
				// Get edge vertices.
				int vertexId0 = tri[i];
				int vertexId1 = tri[(i + 1) % VERTEX_COUNT];

				// Order vertex ID pairs in sequence.
				if (vertexId0 > vertexId1)
					std::swap(vertexId0, vertexId1);

				// Count shared edges.
				auto edgePair = EdgeVertexIdPair{ vertexId0, vertexId1 };
				edgeCountMap[edgePair]++;
			}
		}

		return edgeCountMap;
	}

	CollisionMeshDesc::VertexAdjacencyMap CollisionMeshDesc::GetVertexAdjacencyMap(const std::vector<TriangleVertexIds>& tris, const EdgeCountMap& edgeCountMap) const
	{
		// Run through triangles.
		auto vertexAdjacencyMap = VertexAdjacencyMap{};
		for (const auto& tri : tris)
		{
			// Run through vertices.
			for (int i = 0; i < VERTEX_COUNT; i++)
			{
				// Get edge vertices.
				int vertexId0 = tri[i];
				int vertexId1 = tri[(i + 1) % VERTEX_COUNT];

				// Track vertex adjacency.
				vertexAdjacencyMap[vertexId0].insert(vertexId1);
				vertexAdjacencyMap[vertexId1].insert(vertexId0);
			}
		}

		return vertexAdjacencyMap;
	}

	std::vector<CollisionMeshDesc::EdgeVertexIdPair> CollisionMeshDesc::GetBoundaryEdges(const EdgeCountMap& edgeCountMap) const
	{
		// Extract boundary edges.
		auto boundaryEdges = std::vector<EdgeVertexIdPair>{};
		for (const auto& [edge, count] : edgeCountMap)
		{
			// Filter out non-boundary edge.
			if (count != 1)
				continue;

			boundaryEdges.push_back(edge);
		}

		return boundaryEdges;
	}

	// TODO: Fix. Sometimes creates holes.
	std::vector<int> CollisionMeshDesc::GetSimplifiedPolygon(const std::vector<int>& polygon) const
	{
		// Remove redundant collinear vertices.
		auto simplifiedPolygon = std::vector<int>{};
		simplifiedPolygon.reserve(polygon.size() / 3);
		for (int i = 0; i < polygon.size(); i++)
		{
			// Get vertex IDs.
			int vertexId = polygon[i];
			int prevVertexId = polygon[(i == 0) ? (polygon.size() - 1) : (i - 1)];
			int nextVertexId = polygon[(i == (polygon.size() - 1)) ? 0 : (i + 1)];

			// Calculate edges.
			auto edge0 = _vertices[vertexId] - _vertices[prevVertexId];
			auto edge1 = _vertices[nextVertexId] - _vertices[vertexId];

			// Check collinearity using cross product.
			if (edge0.Cross(edge1).LengthSquared() > EPSILON)
				simplifiedPolygon.emplace_back(vertexId);
		}

		return simplifiedPolygon;
	}

	// TODO: Use better method for complex polygons.
	void CollisionMeshDesc::TriangulatePolygon(std::vector<int>& optimizedVertexIds, const std::vector<int>& polygon, const Plane& plane) const
	{
		// Invalid polygon; return early.
		if (polygon.size() < VERTEX_COUNT)
			return;

		// Triangulate using ear clipping method.
		auto remainingVertexIds = polygon;
		while (remainingVertexIds.size() > 2)
		{
			for (int i = 0; i < remainingVertexIds.size(); i++)
			{
				int vertexId = remainingVertexIds[i];
				int prevVertexId = remainingVertexIds[(i + (remainingVertexIds.size() - 1)) % remainingVertexIds.size()];
				int nextVertexId = remainingVertexIds[(i + 1) % remainingVertexIds.size()];

				// Ensure correct winding order.
				auto faceNormal = (_vertices[prevVertexId] - _vertices[vertexId]).Cross(_vertices[nextVertexId] - _vertices[vertexId]);
				if (faceNormal.Dot(plane.Normal()) > 0.0f)
					std::swap(prevVertexId, nextVertexId);

				// Collect optimized vertex IDs.
				optimizedVertexIds.push_back(prevVertexId);
				optimizedVertexIds.push_back(vertexId);
				optimizedVertexIds.push_back(nextVertexId);

				// Remove vertex ID.
				remainingVertexIds.erase(remainingVertexIds.begin() + i);
				break;
			}
		}

/*
		// 2.4) Triangulate simplified polygons.
		auto optimizedIds = std::vector<int>{};
		for (const auto& polygon : polygons)
		{
			// Skip invalid polygons.
			if (polygon.size() < 3)
				continue;

			// Make mutable copy for triangulation.
			auto remaining = polygon;

			while (remaining.size() > 2)
			{
				bool earFound = false;
				for (int i = 0; i < remaining.size(); i++)
				{
					int prev = remaining[(i + remaining.size() - 1) % remaining.size()];
					int current = remaining[i];
					int next = remaining[(i + 1) % remaining.size()];

					// Ensure correct winding order.
					auto edge1 = _vertices[current] - _vertices[prev];
					auto edge2 = _vertices[next] - _vertices[current];
					auto normal = edge1.Cross(edge2);

					// Skip if triangle is flipped.
					if (normal.Dot(plane.Normal()) < 0.0f)
						continue;

					// Check if any other vertex is inside this triangle.
					bool isEar = true;
					for (int v : remaining)
					{
						if (v == prev || v == current || v == next)
							continue;

						if (PointInTriangle(_vertices[v], _vertices[prev], _vertices[current], _vertices[next]))
						{
							isEar = false;
							break;
						}
					}

					// Clip ear.
					if (isEar)
					{
						optimizedIds.push_back(prev);
						optimizedIds.push_back(current);
						optimizedIds.push_back(next);
						remaining.erase(remaining.begin() + i);
						earFound = true;
						break;
					}
				}

				// Prevent infinite loop if no ear found.
				if (!earFound)
					break;
			}
		}
*/
	}
}
