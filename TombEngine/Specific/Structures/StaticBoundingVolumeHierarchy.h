#pragma once

namespace TEN::Structures
{
	class StaticBoundingVolumeHierarchy
	{
	protected:
		struct Node
		{
			BoundingBox		 Aabb = BoundingBox();
			std::vector<int> Ids  = {}; // NOTE: Only leaf nodes store IDs directly.

			int LeftChildID	 = NO_VALUE;
			int RightChildID = NO_VALUE;

			bool IsLeaf() const;
		};

		// Members

		std::vector<Node> _nodes = {};

	public:
		// Constructors

		StaticBoundingVolumeHierarchy() = default;
		StaticBoundingVolumeHierarchy(const std::vector<int>& ids, const std::vector<BoundingBox>& aabbs);

		// Debug

		void DrawDebug() const;

	protected:
		int Generate(const std::vector<int>& ids, const std::vector<BoundingBox>& aabbs, int start, int end);
	};
}
