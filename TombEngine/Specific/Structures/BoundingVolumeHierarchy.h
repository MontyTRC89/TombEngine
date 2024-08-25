#pragma once

// References:
// https://github.com/erincatto/box2d/blob/main/include/box2d/b2_dynamic_tree.h
// https://github.com/erincatto/box2d/blob/main/src/collision/b2_dynamic_tree.cpp
// https://www.gdcvault.com/play/1025909/Math-for-Game-Developers-Dynamic

namespace TEN::Structures
{
	enum class BvhBuildStrategy
	{
		Fast,	  // O(n): Fast build, okay quality. Top-down approach with median split.
		Balanced, // O(n * m): Efficient build, good quality. Top-down approach with limited surface area heuristic.
		Accurate  // O(n²): Slow build, optimal quality. Top-down approach with exhaustive surface area heuristic.
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

		std::vector<Node>			 _nodes		  = {};
		std::vector<int>			 _freeNodeIds = {};
		std::unordered_map<int, int> _leafIDMap	  = {}; // Key = object ID, value = leaf ID.
		int							 _rootID	  = NO_VALUE;

	public:
		// Constructors

		BoundingVolumeHierarchy() = default;
		BoundingVolumeHierarchy(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, BvhBuildStrategy strategy = BvhBuildStrategy::Balanced);

		// Getters

		unsigned int GetSize() const;

		std::vector<int> GetBoundedObjectIds() const;
		std::vector<int> GetBoundedObjectIds(const Ray& ray, float dist) const;
		std::vector<int> GetBoundedObjectIds(const BoundingBox& aabb) const;
		std::vector<int> GetBoundedObjectIds(const BoundingOrientedBox& obb) const;
		std::vector<int> GetBoundedObjectIds(const BoundingSphere& sphere) const;

		// Utilities

		void Insert(int objectID, const BoundingBox& aabb, float boundary = 0.0f);
		void Move(int objectID, const BoundingBox& aabb, float boundary = 0.0f);
		void Remove(int objectID);

		// Inquirers

		bool IsEmpty() const;

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
