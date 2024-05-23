#pragma once

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
		BoundingBox					  _box		 = BoundingBox();

		int _portalRoomNumber = NO_VALUE;

	public:
		// Constructors
		CollisionTriangle(int vertexID0, int vertexID1, int vertexID2, const Vector3& normal, const BoundingBox& box, int portalRoomNumber);

		// Getters
		const Vector3&	   GetNormal() const;
		const BoundingBox& GetBox() const;
		int				   GetPortalRoomNumber() const;

		// Inquirers
		bool Intersects(const std::vector<Vector3>& vertices, const Ray& ray, float& dist) const;
		bool IsPortal() const;

		// Debug
		void DrawDebug(const std::vector<Vector3>& vertices) const;
	};

	struct CollisionMeshCollisionData
	{
		const CollisionTriangle& Triangle;
		float Distance = 0.0f;
	};

	// Note: Utilizes bounding volume hierarchy.
	class CollisionMesh
	{
	private:
		struct BvhNode
		{
			std::vector<int> TriangleIds = {}; // NOTE: Only leaf nodes store triangles directly.
			BoundingBox		 Box		 = BoundingBox();

			int LeftChildID	 = NO_VALUE;
			int RightChildID = NO_VALUE;

			bool IsLeaf() const;
		};

		class Bvh
		{
		public:
			// Members
			std::vector<BvhNode> Nodes = {};

			// Constructors
			Bvh() = default;
			Bvh(const std::vector<CollisionTriangle>& tris);

			// Utilities
			std::optional<CollisionMeshCollisionData> GetCollision(const Ray& ray, float dist,
																   const std::vector<CollisionTriangle>& tris,
																   const std::vector<Vector3>& vertices) const;

		private:
			int Generate(const std::vector<CollisionTriangle>& tris, const std::vector<int>& triIds, int start, int end);
		};

		// Members
		std::vector<CollisionTriangle> _triangles = {};
		std::vector<Vector3>		   _vertices  = {};
		Bvh							   _bvh		  = Bvh();

	public:
		// Constructors
		CollisionMesh() = default;
		CollisionMesh(const std::vector<CollisionTriangle>& tris);

		// Getters
		std::optional<CollisionMeshCollisionData> GetCollision(const Ray& ray, float dist) const;

		// Utilities
		void InsertTriangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& normal, int portalRoomNumber = NO_VALUE);
		void GenerateBvh();

		// Debug
		void DrawDebug() const;
	};
}
