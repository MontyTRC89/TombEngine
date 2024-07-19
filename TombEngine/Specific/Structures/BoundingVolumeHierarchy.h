#pragma once

namespace TEN::Structures
{
	class BoundingVolumeHierarchy
	{
	private:
		struct Node
		{
			BoundingBox Aabb;
			int			ObjectID = NO_VALUE; // NOTE: Only leaf node stores ID directly.

			int ParentID = NO_VALUE;
			int Child0ID = NO_VALUE;
			int Child1ID = NO_VALUE;

			bool IsLeaf() const;
		};

		// Members

		std::vector<Node> _nodes;
		int				  _rootID; // TODO

	public:
		// Constructors

		BoundingVolumeHierarchy() = default;
		BoundingVolumeHierarchy(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs);

		// Getters

		std::vector<int> GetBoundedObjectIds(const Ray& ray, float dist) const;
		std::vector<int> GetBoundedObjectIds(const BoundingBox& aabb) const;
		std::vector<int> GetBoundedObjectIds(const BoundingOrientedBox& obb) const;
		std::vector<int> GetBoundedObjectIds(const BoundingSphere& sphere) const;

		// Debug

		void DrawDebug() const;

	private:
		// Helpers

		int				 Build(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, int start, int end);
		std::vector<int> GetBoundedObjectIds(const std::function<bool(const Node& node)>& testCollRoutine) const;
	};
}
