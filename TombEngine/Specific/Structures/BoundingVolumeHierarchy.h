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

			int ParentID = NO_VALUE; // TODO
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

		std::vector<int> GetNodeCollisionObjectIds(const Ray& ray, float dist) const;
		std::vector<int> GetNodeCollisionObjectIds(const BoundingSphere& sphere) const;

		// Debug

		void DrawDebug() const;

	private:
		// Helpers

		int				 Generate(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, int start, int end);
		std::vector<int> GetNodeCollisionObjectIds(const std::function<bool(const Node& node)>& testCollRoutine) const;
		float			 GetCost() const;
	};
}
