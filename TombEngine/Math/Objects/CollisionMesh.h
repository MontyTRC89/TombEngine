#pragma once

namespace TEN::Math
{
	class CollisionTriangle
	{
	public:
		// Constants
		static constexpr auto VERTEX_COUNT = 3;

	private:
		// Members
		std::array<const Vector3*, VERTEX_COUNT> _vertices = {};
		Vector3		_normal	= Vector3::Zero;
		BoundingBox _box	= BoundingBox();

	public:
		// Constructors
		CollisionTriangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& normal = Vector3::Zero);

		// Getters
		const std::array<const Vector3*, VERTEX_COUNT>& GetVertices() const; //temp
		const Vector3&	   GetNormal() const;
		const BoundingBox& GetBox() const;

		// Inquirers
		bool Intersects(const Ray& ray, float& dist) const;
	};

	struct CollisionMeshCollisionData
	{
		const CollisionTriangle& Triangle;
		float Distance = 0.0f;
	};

	class CollisionMesh
	{
	private:
		struct BvhNode
		{
			std::vector<int> TriangleIndices = {}; // NOTE: Only leaf nodes store triangles directly.
			BoundingBox		 Box			 = BoundingBox();

			int LeftChildIndex	= NO_VALUE;
			int RightChildIndex = NO_VALUE;

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
			std::optional<CollisionMeshCollisionData> GetIntersection(const Ray& ray, const std::vector<CollisionTriangle>& tris) const;

		private:
			int Build(int start, int end, const std::vector<int>& trIndices, const std::vector<CollisionTriangle>& tris);
		};

		// Members
		std::vector<CollisionTriangle> _triangles = {};
		std::set<Vector3>			   _vertices  = {};
		Bvh							   _bvh		  = Bvh();

	public:
		// Constructors
		CollisionMesh();
		CollisionMesh(const std::vector<CollisionTriangle>& tris);

		// Getters
		const std::vector<CollisionTriangle>&	  GetTriangles() const; //temp
		std::optional<CollisionMeshCollisionData> GetIntersection(const Ray& ray) const;

		// Utilities
		void InsertTriangle(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& normal = Vector3::Zero);
		void UpdateBvh();
	};
}
