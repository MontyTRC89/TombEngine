#pragma once

// References:
// https://github.com/erincatto/box2d/blob/main/src/collision/b2_dynamic_tree.cpp
// https://github.com/erincatto/box2d/blob/main/include/box2d/b2_dynamic_tree.h
// https://www.gdcvault.com/play/1025909/Math-for-Game-Developers-Dynamic

namespace TEN::Structures
{
	// Dynamic bounding volume hierarchy using AABBs.
	class BoundingTree
	{
	private:
		struct Node
		{
			int			ObjectID = NO_VALUE; // NOTE: Only leaf node stores object ID directly.
			BoundingBox Aabb	 = BoundingBox();

			int Height		 = 0;
			int ParentID	 = NO_VALUE;
			int LeftChildID	 = NO_VALUE;
			int RightChildID = NO_VALUE;

			bool IsLeaf() const;
		};

		// Members

		std::vector<Node> _nodes = {};

		int							 _rootID	  = NO_VALUE;
		std::unordered_map<int, int> _leafIDMap	  = {}; // Key = object ID, value = leaf ID.
		std::vector<int>			 _freeNodeIds = {};

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
