#pragma once

#include "Math/Math.h"
#include "Specific/Structures/BoundingTree.h"

using namespace TEN::Math;
using namespace TEN::Structures;

namespace TEN::Physics
{
	class LocalCollisionTriangle
	{
	public:
		static constexpr auto VERTEX_COUNT = 3;

	private:
		// Members

		std::array<int, VERTEX_COUNT> _vertexIds		= {};
		int							  _normalID			= 0;
		BoundingBox					  _aabb				= BoundingBox();
		int							  _portalRoomNumber = NO_VALUE;

	public:
		// Constructors

		LocalCollisionTriangle(int vertex0ID, int vertex1ID, int vertex2ID, int normalID, const BoundingBox& aabb, int portalRoomNumber);

		// Getters

		const Vector3&	   GetVertex0(const std::vector<Vector3>& vertices) const;
		const Vector3&	   GetVertex1(const std::vector<Vector3>& vertices) const;
		const Vector3&	   GetVertex2(const std::vector<Vector3>& vertices) const;
		const Vector3&	   GetNormal(const std::vector<Vector3>& normals) const;
		const BoundingBox& GetAabb() const;
		int				   GetPortalRoomNumber() const;

		Vector3 GetTangent(const BoundingSphere& sphere, const std::vector<Vector3>& vertices, const std::vector<Vector3>& normals) const;

		// Inquirers

		bool Intersects(const Ray& ray, float distMax, float& dist, const std::vector<Vector3>& vertices, const std::vector<Vector3>& normals) const;
		bool Intersects(const BoundingSphere& sphere, const std::vector<Vector3>& vertices, const std::vector<Vector3>& normals) const;
		bool IsPortal() const;

		// Debug

		void DrawDebug(const Matrix& transformMatrix, const Matrix& rotMatrix, const std::vector<Vector3>& vertices, const std::vector<Vector3>& normals) const;
	};

	struct CollisionTriangleData
	{
		std::array<Vector3, LocalCollisionTriangle::VERTEX_COUNT> Vertices = {};
		Vector3 Normal			 = Vector3::Zero;
		int		PortalRoomNumber = NO_VALUE;
	};

	struct CollisionMeshRayCollisionData
	{
		CollisionTriangleData Triangle = {};
		float Distance = 0.0f;
	};
	
	struct CollisionMeshSphereCollisionData
	{
		std::vector<CollisionTriangleData> Triangles = {};
		std::vector<Vector3>			   Tangents	 = {};

		unsigned int Count = 0;
	};

	class CollisionMesh
	{
	private:
		struct Cache
		{
			std::unordered_map<Vector3, int> VertexMap = {}; // Key = vertex, value = vertex ID.
			std::unordered_map<Vector3, int> NormalMap = {}; // Key = normal, value = normal ID.
		};

		// Members

		Vector3								_position	 = Vector3::Zero;
		Quaternion							_orientation = Quaternion::Identity;
		std::vector<LocalCollisionTriangle> _triangles	 = {};
		std::vector<Vector3>				_vertices	 = {};
		std::vector<Vector3>				_normals	 = {};

		BoundingTree _triangleTree = BoundingTree();
		Cache		 _cache		   = {};

	public:
		// Constructors

		CollisionMesh() = default;

		// Getters

		std::optional<CollisionMeshRayCollisionData>	GetCollision(const Ray& ray, float dist) const;
		std::optional<CollisionMeshSphereCollisionData> GetCollision(const BoundingSphere& sphere) const;

		// Setters

		void SetPosition(const Vector3& pos);
		void SetOrientation(const Quaternion& orient);

		// Utilities

		void InsertTriangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& normal, int portalRoomNumber = NO_VALUE);
		void Cook();

		// Debug

		void DrawDebug() const;

	private:
		// Helpers

		Matrix GetTransformMatrix() const;
		Matrix GetRotationMatrix() const;
	};
}
