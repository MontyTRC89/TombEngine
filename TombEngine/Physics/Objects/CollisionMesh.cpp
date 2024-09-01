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

	LocalCollisionTriangle::LocalCollisionTriangle(int vertex0ID, int vertex1ID, int vertex2ID)
	{
		_vertexIds = { vertex0ID, vertex1ID, vertex2ID };
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

	bool LocalCollisionTriangle::Intersects(const Ray& ray, float distMax, float& dist, const std::vector<Vector3>& vertices) const
	{
		// Test if ray intersects triangle AABB.
		float aabbDist = 0.0f;
		if (!ray.Intersects(GetAabb(vertices), aabbDist) || aabbDist >= distMax)
			return false;

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
		constexpr auto TRI_COLOR		  = Color(1.0f, 1.0f, 0.0f, 0.2f);
		constexpr auto NORMAL_LINE_LENGTH = BLOCK(0.1f);
		constexpr auto NORMAL_LINE_COLOR  = Color(1.0f, 1.0f, 1.0f);

		// Get vertices.
		const auto& vertex0 = Vector3::Transform(GetVertex0(vertices), transformMatrix);
		const auto& vertex1 = Vector3::Transform(GetVertex1(vertices), transformMatrix);
		const auto& vertex2 = Vector3::Transform(GetVertex2(vertices), transformMatrix);

		// Get normal.
		auto normal = Vector3::Transform(GetNormal(vertices), rotMatrix);

		// Draw triangle.
		DrawDebugTriangle(vertex0, vertex1, vertex2, TRI_COLOR);

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
		constexpr auto THRESHOLD = 0.001f;

		// Get matrices.
		auto transformMatrix = GetTransformMatrix();
		auto rotMatrix = GetRotationMatrix();

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
			if (tri.Intersects(localRay, closestDist, intersectDist, _vertices))
			{
				closestTri = &tri;
				closestDist = intersectDist;
			}
		}

		if (closestTri != nullptr)
		{
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

		return std::nullopt;
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
		for (const auto& tri : _triangles)
			tri.DrawDebug(GetTransformMatrix(), GetRotationMatrix(), _vertices);
	}

	Matrix CollisionMesh::GetTransformMatrix() const
	{
		auto translationMatrix = Matrix::CreateTranslation(_position);
		auto rotMatrix = Matrix::CreateFromQuaternion(_orientation);
		return (rotMatrix * translationMatrix);
	}

	Matrix CollisionMesh::GetRotationMatrix() const
	{
		return Matrix::CreateFromQuaternion(_orientation);
	}
}
