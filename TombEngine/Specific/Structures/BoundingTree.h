#pragma once

namespace TEN::Structures
{
	// Bounding volume hierarchy implementation using AABBs.
	class BoundingTree
	{
	private:
		struct Node
		{
			BoundingBox Aabb	 = BoundingBox();
			int			ObjectID = NO_VALUE; // NOTE: Only leaf node stores object ID directly.

			int ParentID = NO_VALUE;
			int Child0ID = NO_VALUE;
			int Child1ID = NO_VALUE;

			bool IsLeaf() const;
		};

		// Members

		std::vector<Node> _nodes   = {};
		std::vector<int>  _freeIds = {};
		int				  _rootID  = NO_VALUE;

	public:
		// Constructors

		BoundingTree() = default;
		BoundingTree(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs);

		// Getters

		std::vector<int> GetBoundedObjectIds(const Ray& ray, float dist) const;
		std::vector<int> GetBoundedObjectIds(const BoundingBox& aabb) const;
		std::vector<int> GetBoundedObjectIds(const BoundingOrientedBox& obb) const;
		std::vector<int> GetBoundedObjectIds(const BoundingSphere& sphere) const;

		// Debug

		void DrawDebug() const;

	private:
		// Helpers

		std::vector<int> GetBoundedObjectIds(const std::function<bool(const Node& node)>& testCollRoutine) const;

		void InsertLeaf(int objectID, const BoundingBox& aabb, float boundary = 0.0f);

		int	 GetNewNodeID();
		int	 GetSiblingNodeID(int leafNodeID);
		void RemoveNode(int nodeID);
		void BalanceNode(int leafNodeID);

		int Rebuild(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, int start, int end, float boundary = 0.0f);

		// Debug

		void Validate(int nodeID) const;
	};
}
