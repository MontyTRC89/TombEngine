#pragma once

#include "Specific/Structures/BoundingTree.h"

using namespace TEN::Structures;

namespace TEN::Physics
{
	class CollisionTriangle
	{
	public:
		// Constants

		static constexpr auto VERTEX_COUNT = 3;

	private:
		// Members

		std::array<int, VERTEX_COUNT> _vertexIds = {};
		Vector3						  _normal	 = Vector3::Zero;
		BoundingBox					  _aabb		 = BoundingBox();

		int _portalRoomNumber = NO_VALUE;

	public:
		// Constructors

		CollisionTriangle(int vertex0ID, int vertex1ID, int vertex2ID, const Vector3& normal, const BoundingBox& box, int portalRoomNumber);

		// Getters

		const Vector3&	   GetNormal() const;
		const BoundingBox& GetAabb() const;
		int				   GetPortalRoomNumber() const;

		Vector3 GetTangent(const std::vector<Vector3>& vertices, const BoundingSphere& sphere) const;

		// Inquirers

		bool Intersects(const std::vector<Vector3>& vertices, const Ray& ray, float& dist) const;
		bool Intersects(const std::vector<Vector3>& vertices, const BoundingSphere& sphere) const;
		bool IsPortal() const;

		// Debug

		void DrawDebug(const std::vector<Vector3>& vertices) const;
	};

	struct CollisionMeshRayCollisionData
	{
		const CollisionTriangle& Triangle;
		float Distance;
	};
	
	struct CollisionMeshSphereCollisionData
	{
		std::vector<const CollisionTriangle*> Triangles = {};
		std::vector<Vector3>				  Tangents	= {};

		unsigned int Count;
	};

	class CollisionMesh
	{
	private:
		// Members

		std::vector<CollisionTriangle> _triangles = {};
		std::vector<Vector3>		   _vertices  = {};
		BoundingTree				   _tree	  = {};

	public:
		// Constructors

		CollisionMesh() = default;
		CollisionMesh(const std::vector<CollisionTriangle>& tris);

		// Getters

		std::optional<CollisionMeshRayCollisionData>	GetCollision(const Ray& ray, float dist) const;
		std::optional<CollisionMeshSphereCollisionData> GetCollision(const BoundingSphere& sphere) const;

		// Utilities

		void InsertTriangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& normal, int portalRoomNumber = NO_VALUE);
		void BuildTree();

		// Debug

		void DrawDebug() const;
	};
}
