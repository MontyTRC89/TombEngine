#pragma once

// References:
// https://www.gdcvault.com/play/1025909/Math-for-Game-Developers-Dynamic
// https://github.com/erincatto/box2d/blob/main/src/collision/b2_dynamic_tree.cpp

namespace TEN::Math
{
	// Dynamic bounding volume hierarchy using AABBs.
	class BoundingTree
	{
	private:
		struct Node
		{
			int			ObjectID = -1; // NOTE: Only leaf node stores object ID directly.
			BoundingBox Aabb	 = BoundingBox();
			int			Height	 = -1;

			int ParentID	 = -1;
			int LeftChildID	 = -1;
			int RightChildID = -1;

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
		BoundingTree(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs);

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

	private:
		// Collision helpers

		std::vector<int> GetBoundedObjectIds(const std::function<bool(const Node& node)>& testCollRoutine) const;

		// Dynamic helpers

		int GetNewNodeID();
		int GetBestSiblingLeafID(int leafID) const;

		void InsertLeaf(int leafID);
		void RemoveLeaf(int leafID);

		void RefitNode(int nodeID);
		void RemoveNode(int nodeID);
		int	 BalanceNode(int nodeID);

		// Static helpers

		void Build(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs);
		int	 Build(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, int start, int end);

		// Debug helpers

		void Validate() const;
		void Validate(int nodeID) const;
	};
}
