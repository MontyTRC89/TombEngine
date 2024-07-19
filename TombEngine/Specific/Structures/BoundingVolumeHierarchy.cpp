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

		Build(objectIds, aabbs, 0, (int)objectIds.size());
	}

	std::vector<int> BoundingVolumeHierarchy::GetBoundedObjectIds(const Ray& ray, float dist) const
	{
		auto testCollRoutine = [&](const Node& node)
		{
			float intersectDist = 0.0f;
			return (node.Aabb.Intersects(ray.position, ray.direction, intersectDist) && intersectDist <= dist);
		};

		return GetBoundedObjectIds(testCollRoutine);
	}

	std::vector<int> BoundingVolumeHierarchy::GetBoundedObjectIds(const BoundingBox& aabb) const
	{
		auto testCollRoutine = [&](const Node& node)
		{
			return node.Aabb.Intersects(aabb);
		};

		return GetBoundedObjectIds(testCollRoutine);
	}

	std::vector<int> BoundingVolumeHierarchy::GetBoundedObjectIds(const BoundingOrientedBox& obb) const
	{
		auto aabb = Geometry::GetBoundingBox(obb);
		auto testCollRoutine = [&](const Node& node)
		{
			if (!node.Aabb.Intersects(aabb))
				return false;

			return node.Aabb.Intersects(obb);
		};

		return GetBoundedObjectIds(testCollRoutine);
	}

	std::vector<int> BoundingVolumeHierarchy::GetBoundedObjectIds(const BoundingSphere& sphere) const
	{
		auto testCollRoutine = [&](const Node& node)
		{
			return node.Aabb.Intersects(sphere);
		};

		return GetBoundedObjectIds(testCollRoutine);
	}

	void BoundingVolumeHierarchy::DrawDebug() const
	{
		constexpr auto BOX_COLOR = Color(1.0f, 1.0f, 1.0f);

		for (const auto& node : _nodes)
			DrawDebugBox(node.Aabb, BOX_COLOR);
	}

	int BoundingVolumeHierarchy::Build(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, int start, int end)
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
			node.ObjectID = objectIds[start];

			int newNodeID = (int)_nodes.size();
			_nodes.push_back(node);
			return newNodeID;
		}
		// Split node.
		else
		{
			int mid = (start + end) / 2;
			node.Child0ID = Build(objectIds, aabbs, start, mid);
			node.Child1ID = Build(objectIds, aabbs, mid, end);

			// Set parent ID for child nodes.
			int newNodeID = (int)_nodes.size();
			if (node.Child0ID != NO_VALUE)
			{
				_nodes[node.Child0ID].ParentID = newNodeID;
			}
			if (node.Child1ID != NO_VALUE)
			{
				_nodes[node.Child1ID].ParentID = newNodeID;
			}

			_nodes.push_back(node);
			return newNodeID;
		}
	}
	
	std::vector<int> BoundingVolumeHierarchy::GetBoundedObjectIds(const std::function<bool(const Node& node)>& testCollRoutine) const
	{
		auto objectIds = std::vector<int>{};
		if (_nodes.empty())
			return objectIds;

		std::function<void(int)> traverseRoutine = [&](int nodeID)
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
				traverseRoutine(node.Child0ID);
				traverseRoutine(node.Child1ID);
			}
		};

		// TODO: Root can be any node?
		// Traverse BVH from root node.
		traverseRoutine((int)_nodes.size() - 1);

		return objectIds;
	}
}
