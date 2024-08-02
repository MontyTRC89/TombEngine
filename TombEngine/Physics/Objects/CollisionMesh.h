#pragma once

#include "Math/Math.h"

using namespace TEN::Math;

namespace TEN::Physics
{
	class CollisionTriangle
	{
	public:
		static const auto VERTEX_COUNT = 3;

	private:
		// Members

		std::array<int, VERTEX_COUNT> _vertexIds = {};
		const std::vector<Vector3>*	  _vertices	 = {};


		Vector3		_normal			  = Vector3::Zero;
		BoundingBox _aabb			  = BoundingBox();
		int			_portalRoomNumber = NO_VALUE;

		// TODO: Relative positions?
		//const Vector3*	  _offset	= nullptr;
		//const Quaternion* _rotation = nullptr;

	public:
		// Constructors

		CollisionTriangle(int vertex0ID, int vertex1ID, int vertex2ID, const std::vector<Vector3>& vertices, const Vector3& normal, const BoundingBox& box, int portalRoomNumber);

		// Getters

		const Vector3&	   GetVertex0() const;
		const Vector3&	   GetVertex1() const;
		const Vector3&	   GetVertex2() const;
		const Vector3&	   GetNormal() const;
		const BoundingBox& GetAabb() const;
		int				   GetPortalRoomNumber() const;

		Vector3 GetTangent(const BoundingSphere& sphere) const;

		// Inquirers

		bool Intersects(const Ray& ray, float& dist) const;
		bool Intersects(const BoundingSphere& sphere) const;
		bool IsPortal() const;

		// Debug

		void DrawDebug() const;
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
