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

	struct PairHash
	{
		template <typename T1, typename T2>
		std::size_t operator()(const std::pair<T1, T2>& pair) const
		{
			return std::hash<T1>{}(pair.first) ^ (std::hash<T2>{}(pair.second) << 1);
		}
	};

	// TODO: Not working right.
	void CollisionMeshDesc::Optimize()
	{
		constexpr auto PLANE_HEIGHT_STEP = BLOCK(1 / 32.0f);

		return;

		using VertexArray = std::array<int, LocalCollisionTriangle::VERTEX_COUNT>;

		// 1) Group triangles by planes.
		auto trisByPlane = std::unordered_map<Plane, std::vector<VertexArray>>{};
		for (int i = 0; i < _ids.size(); i += LocalCollisionTriangle::VERTEX_COUNT)
		{
			// Get vertices.
			const auto& vertex0 = _vertices[_ids[i]];
			const auto& vertex1 = _vertices[_ids[i + 1]];
			const auto& vertex2 = _vertices[_ids[i + 2]];

			// Calculate edges.
			auto edge0 = vertex1 - vertex0;
			auto edge1 = vertex2 - vertex0;

			// Calculate edge normal.
			auto normal = edge0.Cross(edge1);
			normal.Normalize();

			// Calculate plane distance.
			float dist = RoundToStep(normal.Dot(vertex0), PLANE_HEIGHT_STEP);

			// Collect triangle IDs.
			auto plane = Plane(normal, dist);
			trisByPlane[plane].push_back({ _ids[i], _ids[i + 1], _ids[i + 2] });
		}

		// 2) Process plane triangles.
		auto optimizedIds = std::vector<int>{};
		for (const auto& [plane, tris] : trisByPlane)
		{
			auto edgeCount = std::unordered_map<std::pair<int, int>, int, PairHash>{};
			auto adjacencyMap = std::unordered_map<int, std::unordered_set<int>>{};

			// Count shared edges.
			for (const auto& tri : tris)
			{
				for (int j = 0; j < 3; j++)
				{
					int a = tri[j];
					int b = tri[(j + 1) % 3];
					if (a > b)
						std::swap(a, b);

					edgeCount[{ a, b }]++;
					adjacencyMap[a].insert(b);
					adjacencyMap[b].insert(a);
				}
			}

			// Extract boundary edges (edges that appear only once).
			auto boundaryEdges = std::vector<std::pair<int, int>>{};
			for (const auto& [edge, count] : edgeCount)
			{
				if (count == 1)
					boundaryEdges.push_back(edge);
			}

			// Order boundary edges into polygon.
			auto polygon = std::vector<int>{};
			if (!boundaryEdges.empty())
			{
				polygon.push_back(boundaryEdges.front().first);
				polygon.push_back(boundaryEdges.front().second);
				boundaryEdges.erase(boundaryEdges.begin());

				while (!boundaryEdges.empty())
				{
					bool found = false;
					for (auto it = boundaryEdges.begin(); it != boundaryEdges.end(); ++it)
					{
						if (it->first == polygon.back())
						{
							polygon.push_back(it->second);
							boundaryEdges.erase(it);
							found = true;
							break;
						}
						else if (it->second == polygon.back())
						{
							polygon.push_back(it->first);
							boundaryEdges.erase(it);
							found = true;
							break;
						}
					}

					// FAILSAFE: Prevent infinite loop if no matching edge found.
					if (!found)
						break;
				}
			}

			// Triangulate polygon using Ear Clipping.
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
		constexpr auto TRI_COLOR		  = Color(1.0f, 1.0f, 0.0f, 0.1f);
		constexpr auto NORMAL_LINE_LENGTH = BLOCK(0.15f);
		constexpr auto NORMAL_LINE_COLOR  = Color(1.0f, 1.0f, 1.0f);

		// Get vertices.
		auto vertex0 = Vector3::Transform(GetVertex0(vertices), transformMatrix);
		auto vertex1 = Vector3::Transform(GetVertex1(vertices), transformMatrix);
		auto vertex2 = Vector3::Transform(GetVertex2(vertices), transformMatrix);

		// Get normal.
		auto normal = Vector3::Transform(GetNormal(vertices), rotMatrix);

		// Draw triangle.
		DrawDebugTriangle(vertex0, vertex1, vertex2, TRI_COLOR);

		DrawDebugLine(vertex0, vertex1, Color(1,1,1));
		DrawDebugLine(vertex1, vertex2, Color(1,1,1));
		DrawDebugLine(vertex2, vertex0, Color(1,1,1));

		// Draw normal line.
		auto center = (vertex0 + vertex1 + vertex2) / VERTEX_COUNT;
		DrawDebugLine(center, Geometry::TranslatePoint(center, normal, NORMAL_LINE_LENGTH), NORMAL_LINE_COLOR);
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
