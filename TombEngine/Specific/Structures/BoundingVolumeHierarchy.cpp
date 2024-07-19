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

		// Debug
		//_nodes.clear();
		//for (int i = 0; i < objectIds.size(); i++)
		//	InsertLeaf(objectIds[i], aabbs[i]);

		Build(objectIds, aabbs, 0, (int)objectIds.size());
		_rootID = int(_nodes.size() - 1);
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
		{
			//if (node.IsLeaf() || _nodes[node.Child0ID].IsLeaf() || _nodes[node.Child1ID].IsLeaf())
			//	continue;

			DrawDebugBox(node.Aabb, BOX_COLOR);
		}

		PrintDebugMessage("%d", _rootID);
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
				DrawDebugBox(node.Aabb, Color(1, 1, 1));
				objectIds.push_back(node.ObjectID);
			}
			else
			{
				traverseRoutine(node.Child0ID);
				traverseRoutine(node.Child1ID);
			}
		};

		// Traverse tree from root node.
		traverseRoutine(_rootID);

		PrintDebugMessage("found: %d", (int)objectIds.size());
		PrintDebugMessage("root: %d", _rootID);

		return objectIds;
	}

	// TODO: Not working? Tree hierarchy looks okay, but bounded object IDs cannot be collected.
	void BoundingVolumeHierarchy::InsertLeaf(int objectID, const BoundingBox& aabb, float boundary)
	{
		auto nodeAabb = aabb;
		*(Vector3*)&nodeAabb.Extents += Vector3(boundary);

		// 1) No existing nodes; create root.
		if (_nodes.empty())
		{
			auto node = Node{};
			node.Aabb = nodeAabb;
			node.ObjectID = objectID;
			_nodes.push_back(node);
			_rootID = 0;
			return;
		}

		_nodes.emplace_back();
		int leafNodeID = int(_nodes.size() - 1);
		auto& leafNode = _nodes[leafNodeID];
		leafNode.Aabb = nodeAabb;
		leafNode.ObjectID = objectID;

		// 2) Find best sibling for new leaf.
		int siblingNodeID = GetSiblingNodeID(leafNodeID);

		// 3) Create new parent.
		int prevParentNodeID = _nodes[siblingNodeID].ParentID;

		_nodes.emplace_back(Node{});
		int newParentNodeID = int(_nodes.size() - 1);

		auto& newParentNode = _nodes[newParentNodeID];
		auto& siblingNode = _nodes[siblingNodeID];

		newParentNode.ParentID = prevParentNodeID;
		BoundingBox::CreateMerged(newParentNode.Aabb, siblingNode.Aabb, leafNode.Aabb);

		// Sibling is not root; update previous parent.
		if (prevParentNodeID != NO_VALUE)
		{
			auto& prevParentNode = _nodes[prevParentNodeID];
			if (prevParentNode.Child0ID == siblingNodeID)
			{
				prevParentNode.Child0ID = newParentNodeID;
			}
			else
			{
				prevParentNode.Child1ID = newParentNodeID;
			}
		}
		// Sibling is root; set new parent as root.
		else
		{
			newParentNode.Child0ID = siblingNodeID;
			newParentNode.Child1ID = leafNodeID;
			siblingNode.ParentID = newParentNodeID;
			leafNode.ParentID = newParentNodeID;
			_rootID = newParentNodeID;
		}

		// 4) Retread tree branch to refit AABBs.
		int parentNodeID = _nodes[leafNodeID].ParentID;
		while (parentNodeID != NO_VALUE)
		{
			int childID0 = _nodes[parentNodeID].Child0ID;
			int childID1 = _nodes[parentNodeID].Child1ID;

			BoundingBox::CreateMerged(_nodes[parentNodeID].Aabb, _nodes[childID0].Aabb, _nodes[childID1].Aabb);

			Rotate(leafNodeID);

			parentNodeID = _nodes[parentNodeID].ParentID;
		}
	}

	int BoundingVolumeHierarchy::GetSiblingNodeID(int leafNodeID)
	{
		auto leafAabb = _nodes[leafNodeID].Aabb;
		int bestSiblingID = NO_VALUE;
		float bestCost = INFINITY;

		// TODO: Use branch and bound algorithm instead.
		for (int i = 0; i < (_nodes.size() - 1); i++)
		{
			const auto& node = _nodes[i];
			if (!node.IsLeaf())
				continue;

			// Merge leaf node and prospective sibling node AABBs.
			auto mergedAabb = BoundingBox();
			BoundingBox::CreateMerged(mergedAabb, leafAabb, node.Aabb);

			// Update best sibling.
			float mergedCost = Geometry::GetBoundingBoxArea(mergedAabb);
			if (mergedCost < bestCost)
			{
				bestCost = mergedCost;
				bestSiblingID = i;
			}
		}

		return bestSiblingID;
	}

	void BoundingVolumeHierarchy::Rotate(int leafNodeID)
	{
		int parentNodeID = _nodes[leafNodeID].ParentID;
		int grandParentNodeID = _nodes[parentNodeID].ParentID;

		if (grandParentNodeID == NO_VALUE)
			return;

		auto& leafNode = _nodes[leafNodeID];
		auto& parentNode = _nodes[parentNodeID];
		auto& grandParentNode = _nodes[grandParentNodeID];

		int rotatedSiblingNodeID = NO_VALUE;
		if (grandParentNode.Child0ID != parentNodeID && grandParentNode.Child1ID == NO_VALUE)
		{
			rotatedSiblingNodeID = grandParentNode.Child0ID;
		}
		else if (grandParentNode.Child1ID != parentNodeID && grandParentNode.Child0ID == NO_VALUE)
		{
			rotatedSiblingNodeID = grandParentNode.Child0ID;
		}

		if (rotatedSiblingNodeID == NO_VALUE)
			return;
		
		auto& rotatedSiblingNode = _nodes[rotatedSiblingNodeID];

		auto mergedAabb = BoundingBox();
		BoundingBox::CreateMerged(mergedAabb, rotatedSiblingNode.Aabb, leafNode.Aabb);

		// Rotation more less optimal; return early.
		if (Geometry::GetBoundingBoxArea(mergedAabb) > Geometry::GetBoundingBoxArea(parentNode.Aabb))
			return;

		if (parentNode.Child0ID == leafNodeID)
		{
			parentNode.Child0ID = NO_VALUE;
			if (parentNode.Child1ID != NO_VALUE)
				parentNode.Aabb = _nodes[parentNode.Child1ID].Aabb;
		}
		else
		{
			parentNode.Child1ID = NO_VALUE;
			if (parentNode.Child0ID != NO_VALUE)
				parentNode.Aabb = _nodes[parentNode.Child0ID].Aabb;
		}
		parentNode.Aabb = {};

		if (grandParentNode.Child0ID == NO_VALUE)
		{
			grandParentNode.Child0ID = leafNodeID;
		}
		else
		{
			grandParentNode.Child1ID = leafNodeID;
		}
		grandParentNode.Aabb = mergedAabb;
	}

	int BoundingVolumeHierarchy::Build(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, int start, int end, float boundary)
	{
		// FAILSAFE.
		if (start >= end)
			return NO_VALUE;

		auto node = Node{};

		// Combine AABBs.
		node.Aabb = aabbs[start];
		*(Vector3*)&node.Aabb.Extents += Vector3(boundary);
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
}
