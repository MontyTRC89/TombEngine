#pragma once

namespace TEN::Math
{
	// TODO: Can't use NO_VALUE for some reason.
	
	// Dynamic bounding volume hierarchy using AABBs.
	class BoundingTree
	{
	private:
		struct Node
		{
			int			ObjectID = -1; // NOTE: Only leaf node stores object ID directly.
			BoundingBox Aabb	 = BoundingBox();
			int			Depth	 = -1;

			int ParentID = -1;
			int Child0ID = -1;
			int Child1ID = -1;

			bool IsLeaf() const;
		};

		// Members

		std::vector<Node> _nodes	   = {};
		std::vector<int>  _freeNodeIds = {};
		int				  _rootID	   = -1;

		std::unordered_map<int, int> _leafIDMap = {}; // Key = object ID.

	public:
		// Constructors

		BoundingTree() = default;
		BoundingTree(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, float boundary = 0.0f);

		// Getters

		std::vector<int> GetBoundedObjectIds() const;
		std::vector<int> GetBoundedObjectIds(const Ray& ray, float dist) const;
		std::vector<int> GetBoundedObjectIds(const BoundingBox& aabb) const;
		std::vector<int> GetBoundedObjectIds(const BoundingOrientedBox& obb) const;
		std::vector<int> GetBoundedObjectIds(const BoundingSphere& sphere) const;

		// Utilities

		void Insert(int objectID, const BoundingBox& aabb, float boundary = 0.0f);
		void Move(int objectID, const BoundingBox& aabb, float boundary = 0.0f);
		void Remove(int objectID);

		// Debug

		void DrawDebug() const;
		void Validate() const;
		void Validate(int nodeID) const;

	private:
		// Helper getters

		std::vector<int> GetBoundedObjectIds(const std::function<bool(const Node& node)>& testCollRoutine) const;
		int				 GetNewNodeID();
		int				 GetBestSiblingLeafID(int leafID) const;

		// Helper utilities

		void InsertLeaf(int leafID);
		void RemoveLeaf(int leafID);

		int	 BalanceNode(int nodeID);
		void RefitNode(int nodeID);
		void RemoveNode(int nodeID);

		int Rebuild(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, int start, int end, float boundary = 0.0f);
	};
}
