#include "framework.h"
#include "Specific/Structures/StaticBoundingVolumeHierarchy.h"

#include "Renderer/Renderer.h"

using TEN::Renderer::g_Renderer;

namespace TEN::Structures
{
	bool StaticBoundingVolumeHierarchy::Node::IsLeaf() const
	{
		return (LeftChildID == NO_VALUE && RightChildID == NO_VALUE);
	}

	StaticBoundingVolumeHierarchy::StaticBoundingVolumeHierarchy(const std::vector<int>& ids, const std::vector<BoundingBox>& aabbs)
	{
		assertion(ids.size() == aabbs.size(), "StaticBoundingVolumeHierarchy(): ID and AABB counts must be equal.");

		Generate(ids, aabbs, 0, (int)ids.size());
	}

	void StaticBoundingVolumeHierarchy::DrawDebug() const
	{
		constexpr auto BOX_COLOR = Color(1.0f, 0.0f, 1.0f);

		for (const auto& node : _nodes)
			g_Renderer.AddDebugBox(node.Aabb, BOX_COLOR);
	}

	int StaticBoundingVolumeHierarchy::Generate(const std::vector<int>& ids, const std::vector<BoundingBox>& aabbs, int start, int end)
	{
		constexpr auto ID_COUNT_PER_LEAF_MAX = 4;

		// FAILSAFE.
		if (start >= end)
			return NO_VALUE;

		auto node = Node{};

		// Combine AABBs.
		node.Aabb = aabbs[start];
		for (int i = (start + 1); i < end; i++)
			node.Aabb = Geometry::CombineBoundingBoxes(node.Aabb, aabbs[i]);

		// Leaf node.
		if ((end - start) <= ID_COUNT_PER_LEAF_MAX)
		{
			node.Ids.insert(node.Ids.end(), ids.begin() + start, ids.begin() + end);
			_nodes.push_back(node);
			return int(_nodes.size() - 1);
		}
		// Split node.
		else
		{
			int mid = (start + end) / 2;
			node.LeftChildID = Generate(ids, aabbs, start, mid);
			node.RightChildID = Generate(ids, aabbs, mid, end);
			_nodes.push_back(node);
			return int(_nodes.size() - 1);
		}
	}
}
