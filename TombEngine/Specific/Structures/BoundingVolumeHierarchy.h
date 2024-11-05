#pragma once

// References:
// https://github.com/erincatto/box2d/blob/main/include/box2d/b2_dynamic_tree.h
// https://github.com/erincatto/box2d/blob/main/src/collision/b2_dynamic_tree.cpp
// https://www.gdcvault.com/play/1025909/Math-for-Game-Developers-Dynamic

// !!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!
// TODO: Can't use NO_VALUE because of compiler errors. Using magic -1 for now.
// !!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!

// NOTE: _leafIDMap is a hash map for convenience. If performance ever suffers because of too many Move() and Remove() calls, it can be substituted for a method
// with faster access. However, this optimised solution will require maintaining a weird index variable outside the BVH instance,
// so I'd prefer to keep the hash map if possible. -- Sezz 2024.11.05

namespace TEN::Structures
{
	enum class BvhBuildStrategy
	{
		Fast,	  // O(n): Fast build, okay quality. Top-down approach with median split.
		Balanced, // O(n * m): Efficient build, good quality. Top-down approach with constrained surface area heuristic.
		Accurate  // O(n²): Slow build, optimal quality. Top-down approach with exhaustive surface area heuristic.
	};

	// Dynamic bounding volume hierarchy using AABBs.
	typedef class BoundingVolumeHierarchy
	{
	private:
		struct Node
		{
			int			ObjectID = 1; // NOTE: Only stored by leaf.
			BoundingBox Aabb	 = BoundingBox();

			int Height		 = 0;
			int ParentID	 = -1;
			int LeftChildID	 = -1;
			int RightChildID = -1;

			bool IsLeaf() const;
		};

		// Members

		std::vector<Node>			 _nodes		  = {};
		std::vector<int>			 _freeNodeIds = {};
		std::unordered_map<int, int> _leafIDMap	  = {}; // Key = object ID, value = leaf ID.
		int							 _rootID	  = -1;

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

		// Inquirers

		bool IsEmpty() const;

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

		void Build(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, BvhBuildStrategy strategy);
		int	 Build(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, int start, int end, BvhBuildStrategy strategy);

		// Debug helpers

		void Validate() const;
		void Validate(int nodeID) const;
	} Bvh;
}
