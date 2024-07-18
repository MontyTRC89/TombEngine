#include "framework.h"
#include "Specific/Structures/BoundingVolumeHierarchy.h"

#include "Renderer/Renderer.h"

using TEN::Renderer::g_Renderer;

namespace TEN::Structures
{
	bool BoundingVolumeHierarchy::Node::IsLeaf() const
	{
		return (Child0ID == NO_VALUE && Child1ID == NO_VALUE);
	}

	BoundingVolumeHierarchy::BoundingVolumeHierarchy(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs)
	{
		TENAssert(objectIds.size() == aabbs.size(), "BoundingVolumeHierarchy(): Object ID and AABB counts must be equal.");

		Generate(objectIds, aabbs, 0, (int)objectIds.size());
	}

	void BoundingVolumeHierarchy::DrawDebug() const
	{
		constexpr auto BOX_COLOR = Color(1.0f, 1.0f, 1.0f);

		for (const auto& node : _nodes)
			DrawDebugBox(node.Aabb, BOX_COLOR);
	}

	std::vector<int> BoundingVolumeHierarchy::GetNodeCollisionObjectIds(const Ray& ray, float dist) const
	{
		auto testColl = [&](const Node& node)
		{
			float intersectDist = 0.0f;
			return (node.Aabb.Intersects(ray.position, ray.direction, intersectDist) && intersectDist <= dist);
		};

		return GetNodeCollisionObjectIds(testColl);
	}

	std::vector<int> BoundingVolumeHierarchy::GetNodeCollisionObjectIds(const BoundingSphere& sphere) const
	{
		auto testColl = [&](const Node& node)
		{
			return node.Aabb.Intersects(sphere);
		};

		return GetNodeCollisionObjectIds(testColl);
	}

	int BoundingVolumeHierarchy::Generate(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, int start, int end)
	{
		// FAILSAFE.
		if (start >= end)
			return NO_VALUE;

		auto node = Node{};

		// Combine AABBs.
		node.Aabb = aabbs[start];
		for (int i = (start + 1); i < end; i++)
			BoundingBox::CreateMerged(node.Aabb, node.Aabb, aabbs[i]);

		// Leaf node.
		if ((end - start) == 1)
		{
			node.ObjectID = start;
			_nodes.push_back(node);
			return int(_nodes.size() - 1);
		}
		// Split node.
		else
		{
			int mid = (start + end) / 2;
			node.Child0ID = Generate(objectIds, aabbs, start, mid);
			node.Child1ID = Generate(objectIds, aabbs, mid, end);
			_nodes.push_back(node);
			return int(_nodes.size() - 1);
		}
	}
	
	std::vector<int> BoundingVolumeHierarchy::GetNodeCollisionObjectIds(const std::function<bool(const Node& node)>& testCollRoutine) const
	{
		auto objectIds = std::vector<int>{};
		if (_nodes.empty())
			return objectIds;

		std::function<void(int)> traverse = [&](int nodeID)
		{
			// Invalid node; return early.
			if (nodeID == NO_VALUE)
				return;

			const auto& node = _nodes[nodeID];

			// Test node collision.
			if (!testCollRoutine(node))
				return;

			// Traverse nodes.
			if (node.IsLeaf())
			{
				objectIds.push_back(node.ObjectID);
			}
			else
			{
				traverse(node.Child0ID);
				traverse(node.Child1ID);
			}
		};

		// TODO: Root can be any node?
		// Traverse BVH from root node.
		traverse((int)_nodes.size() - 1);

		return objectIds;
	}

	float BoundingVolumeHierarchy::GetCost() const
	{
		// Calculate cost ignoring root and leaves.
		float cost = 0.0f;
		for (int i = 0; i < (_nodes.size() - 1); i++)
		{
			const auto& node = _nodes[i];
			if (node.IsLeaf())
				continue;

			cost += Geometry::GetBoundingBoxArea(node.Aabb);
		}

		return cost;
	}
}
