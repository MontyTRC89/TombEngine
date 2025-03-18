#include "framework.h"
#include "Physics/Objects/CollisionMeshDesc.h"

#include "Math/Math.h"
#include "Specific/trutils.h"

using namespace TEN::Math;
using namespace TEN::Utils;

namespace TEN::Physics
{
	unsigned int CollisionMeshDesc::GetTriangleCount() const
	{
		return ((unsigned int)_vertexIds.size() / VERTEX_COUNT);
	}

	const std::vector<Vector3>& CollisionMeshDesc::GetVertices() const
	{
		return _vertices;
	}

	const std::vector<int>& CollisionMeshDesc::GetIds() const
	{
		return _vertexIds;
	}

	void CollisionMeshDesc::InsertTriangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2)
	{
		auto getVertexId = [&](const Vector3& vertex)
		{
			// Get existing vertex ID.
			auto it = _vertexMap.find(vertex);
			if (it != _vertexMap.end())
			{
				const auto& [vertex, vertexId] = *it;
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
		_vertexIds.push_back(vertexId0);
		_vertexIds.push_back(vertexId1);
		_vertexIds.push_back(vertexId2);
	}

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
				TriangulatePolygon(optimizedVertexIds, polygon, plane.Normal());
		}

		auto optimizedVertices = std::vector<Vector3>{};
		optimizedVertices.reserve(optimizedVertexIds.size());
		
		auto reoptimizedVertexIds = std::vector<int>{};
		reoptimizedVertexIds.reserve(optimizedVertexIds.size());

		// 3) Finalize optimized vertices and vertex IDs.
		for (int vertexId : optimizedVertexIds)
		{
			const auto& vertex = _vertices[vertexId];

			optimizedVertices.push_back(vertex);
			reoptimizedVertexIds.push_back((int)optimizedVertices.size() - 1);
		}

		// 4) Store optimized vertices and vertex IDs.
		_vertices = std::move(optimizedVertices);
		_vertexIds = std::move(reoptimizedVertexIds);
	}

	std::vector<std::vector<int>> CollisionMeshDesc::GetPolygons(const std::vector<TriangleVertexIds>& tris) const
	{
		// 1) Get key mesh collections.
		auto edgeCountMap = GetEdgeCountMap(tris);
		auto vertexAdjacencyMap = GetVertexAdjacencyMap(tris, edgeCountMap); // TODO: Not used yet.
		auto boundaryEdges = GetBoundaryEdges(edgeCountMap);

		// TODO: This function only works for already monotone polygons. Also make it work for:
		//	- Irregular polygons.
		//	- Holed polygons.
		//
		// Steps:
		// 1) Split boundary edges into islands to be processed later. Can use vertex adjacency map?
		// 2) Run through this collection of split boundary edges. Deconstruct irregular or holed polygons into monotone ones.
		// 3) Process the collection of monotone polygons using the simple method below.

		// 2) Process boundary edges into optimal polygon loops.
		auto polygons = std::vector<std::vector<int>>{};
		while (!boundaryEdges.empty())
		{
			auto polygon = std::vector<int>{};

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

					// If current vertex is equal to one in polygon, add next vertex.
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

				// No matching edge found; finalize monotone polygon.
				if (!isFound)
					break;
			}

			// 2.3) Simplify and collect valid polygon.
			SimplifyPolygon(polygon);
			if (polygon.size() >= VERTEX_COUNT)
				polygons.push_back(std::move(polygon));
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
		for (int i = 0; i < _vertexIds.size(); i += VERTEX_COUNT)
		{
			// Get vertices.
			const auto& vertex0 = _vertices[_vertexIds[i]];
			const auto& vertex1 = _vertices[_vertexIds[i + 1]];
			const auto& vertex2 = _vertices[_vertexIds[i + 2]];

			// Calculate edges.
			auto edge0 = vertex1 - vertex0;
			auto edge1 = vertex2 - vertex0;

			// Calculate plane normal.
			auto normal = edge0.Cross(edge1);
			normal.Normalize();
			normal = RoundNormal(normal, NORMAL_EPSILON);

			// TODO: Make rounding optional.
			// Calculate plane distance.
			float dist = RoundToStep(normal.Dot(vertex0), PLANE_HEIGHT_STEP);

			// Collect triangles by plane.
			auto plane = Plane(normal, dist);
			coplanarTriMap[plane].push_back({ _vertexIds[i], _vertexIds[i + 1], _vertexIds[i + 2] });
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
				auto edgePair = EdgeVertexIdPair(vertexId0, vertexId1);
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

	void CollisionMeshDesc::SimplifyPolygon(std::vector<int>& polygon) const
	{
		// Remove redundant collinear vertices from polygon.
		int i = 0;
		while (polygon.size() > VERTEX_COUNT)
		{
			// Get vertex IDs.
			int vertexId0 = polygon[i];
			int vertexId1 = polygon[(i + 1) % polygon.size()];
			int vertexId2 = polygon[(i + 2) % polygon.size()];

			// Get vertices.
			const auto& vertex0 = _vertices[vertexId0];
			const auto& vertex1 = _vertices[vertexId1];
			const auto& vertex2 = _vertices[vertexId2];

			// Calculate edges.
			auto edge0 = vertex1 - vertex0;
			auto edge1 = vertex2 - vertex1;

			// Edges are collinear; remove vertex 1 and remain at current vertex.
			auto edgeCross = edge0.Cross(edge1);
			if (edgeCross.LengthSquared() < EPSILON)
			{
				Erase(polygon, (i + 1) % polygon.size());
				if (i >= polygon.size())
					i = (int)polygon.size() - 1;
			}
			// Edges aren't collinear; move to next vertex.
			else
			{
				i++;
				if (i >= polygon.size())
					break;
			}
		}
	}

	void CollisionMeshDesc::TriangulatePolygon(std::vector<int>& optimizedVertexIds, const std::vector<int>& polygon, const Vector3& normal) const
	{
		// Invalid polygon; return early.
		if (polygon.size() < VERTEX_COUNT)
			return;

		// TEMP
		int count = 0;

		// Triangulate using ear clipping method.
		auto vertexIds = polygon;
		int i = 0;
		while (vertexIds.size() > 2)
		{
			// Get vertex IDs.
			int vertexId0 = vertexIds[i];
			int vertexId1 = vertexIds[(i + 1) % vertexIds.size()];
			int vertexId2 = vertexIds[(i + 2) % vertexIds.size()];

			// Ensure correct winding order.
			auto faceNormal = (_vertices[vertexId0] - _vertices[vertexId1]).Cross(_vertices[vertexId2] - _vertices[vertexId1]);
			if (faceNormal.Dot(normal) > 0.0f)
				std::swap(vertexId0, vertexId2);

			// Get vertices.
			const auto& vertex0 = _vertices[vertexId0];
			const auto& vertex1 = _vertices[vertexId1];
			const auto& vertex2 = _vertices[vertexId2];

			// Calculate edges.
			auto edge0 = vertex1 - vertex0;
			auto edge1 = vertex2 - vertex1;

			// Project edges onto plane by removing component along normal.
			auto projEdge0 = edge0 - (normal * (edge0.Dot(normal) / normal.LengthSquared()));
			auto projEdge1 = edge1 - (normal * (edge1.Dot(normal) / normal.LengthSquared()));

			// Calculate cross product of projected edges.
			auto edgeCross = projEdge0.Cross(projEdge1);

			// Angle between edges is convex (< 180 degrees).
			//if (true)
			if (edgeCross.LengthSquared() >= 0.0f)
			{
				// FAILSAFE: Skip degenerate triangle.
				//if (abs(edgeCross.LengthSquared()) > EPSILON)
				//	continue;
				
				// Collect optimized vertex IDs.
				optimizedVertexIds.push_back(vertexId0);
				optimizedVertexIds.push_back(vertexId1);
				optimizedVertexIds.push_back(vertexId2);

				// Remove vertex 1.
				vertexIds.erase(vertexIds.begin() + ((i + 1) % vertexIds.size()));
				if (i == vertexIds.size())
					i--;

				count = 0;
			}
			// Angle between edges is reflex (>= 180 degrees).
			else
			{
				// Skip current vertex.
				i = (i + 1) % vertexIds.size();
				count++;

				if (count > 10)
				{
					PrintDebugMessage("Fail!");
					break;
				}
			}
		}
	}
}
