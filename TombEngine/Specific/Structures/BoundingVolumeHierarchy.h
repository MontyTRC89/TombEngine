#pragma once

// References:
// https://github.com/erincatto/box2d/blob/main/src/collision/b2_dynamic_tree.cpp
// https://github.com/erincatto/box2d/blob/main/include/box2d/b2_dynamic_tree.h
// https://www.gdcvault.com/play/1025909/Math-for-Game-Developers-Dynamic

namespace TEN::Structures
{
	enum class BvhBuildStrategy
	{
		Fast,	  // O(n): Top-down approach with median split. Fast build, okay quality.
		Balanced, // O(n): Top-down approach with limited surface area heuristic (SAH). Slower build, good quality.
		Accurate  // O(n^2): Top-down approach with exhaustive surface area heuristic (SAH). Slow build, optimal quality.
	};

	// Dynamic bounding volume hierarchy using AABBs.
	typedef class BoundingVolumeHierarchy
	{
	private:
		struct Node
		{
			int			ObjectID = NO_VALUE; // NOTE: Only stored by leaf.
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

		BoundingVolumeHierarchy() = default;
		BoundingVolumeHierarchy(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, BvhBuildStrategy strategy = BvhBuildStrategy::Balanced);

		// Getters

		std::vector<int> GetBoundedObjectIds() const;
		std::vector<int> GetBoundedObjectIds(const Ray& ray, float dist) const;
		std::vector<int> GetBoundedObjectIds(const BoundingBox& aabb) const;
		std::vector<int> GetBoundedObjectIds(const BoundingOrientedBox& obb) const;
		std::vector<int> GetBoundedObjectIds(const BoundingSphere& sphere) const;

		// Utilities

		unsigned int Size() const;
		bool		 Empty() const;

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

		void Build(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, BvhBuildStrategy strategy);
		int	 Build(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, int start, int end, BvhBuildStrategy strategy);

		// Debug helpers

		void Validate() const;
		void Validate(int nodeID) const;
	} Bvh;
}
