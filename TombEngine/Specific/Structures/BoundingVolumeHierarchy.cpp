#include "framework.h"
#include "Specific/Structures/BoundingVolumeHierarchy.h"

#include "Renderer/Renderer.h"

using TEN::Renderer::g_Renderer;

// TODO: Add licence? Heavily referenced this implementation, which has an MIT licence and requests attribution:
// https://github.com/erincatto/box2d/blob/main/src/collision/b2_dynamic_tree.cpp

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
		_nodes.clear();
		//for (int i = 0; i < objectIds.size(); i++)
		//	InsertLeaf(objectIds[i], aabbs[i]);

		Rebuild(objectIds, aabbs, 0, (int)objectIds.size());
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
			DrawDebugBox(node.Aabb, BOX_COLOR);

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

	void BoundingVolumeHierarchy::InsertLeaf(int objectID, const BoundingBox& aabb, float boundary)
	{
		// Calculate AABB with expanded boundary.
		auto expandedAabb = aabb;
		*(Vector3*)&expandedAabb.Extents += Vector3(boundary);

		// Get leaf node.
		int leafNodeID = GetNewNodeID();
		auto& leafNode = _nodes[leafNodeID];

		leafNode.Aabb = expandedAabb;
		leafNode.ObjectID = objectID;

		// 1) Create root.
		if (_rootID == NO_VALUE)
		{
			_rootID = 0;
			return;
		}

		// 2) Get best sibling node for new leaf.
		int siblingNodeID = GetSiblingNodeID(leafNodeID);
		auto& siblingNode = _nodes[siblingNodeID];

		// 3) Create new parent.
		int prevParentNodeID = siblingNode.ParentID;
		int newParentNodeID = GetNewNodeID();
		auto& newParentNode = _nodes[newParentNodeID];

		// TODO: Something with bounding boxes is wrong.

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

			newParentNode.Child0ID = siblingNodeID;
			newParentNode.Child0ID = leafNodeID;
			siblingNode.ParentID = newParentNodeID;
			leafNode.ParentID = newParentNodeID;
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
		int parentNodeID = leafNode.ParentID;
		while (parentNodeID != NO_VALUE)
		{
			// TODO
			//parentNodeID = BalanceNode(parentNodeID);

			auto& parentNode = _nodes[parentNodeID];

			if (parentNode.Child0ID != NO_VALUE && parentNode.Child1ID != NO_VALUE)
			{
				const auto& childNode0 = _nodes[parentNode.Child0ID];
				const auto& childNode1 = _nodes[parentNode.Child1ID];
				BoundingBox::CreateMerged(parentNode.Aabb, childNode0.Aabb, childNode1.Aabb);
			}
			else if (parentNode.Child0ID != NO_VALUE)
			{
				const auto& childNode0 = _nodes[parentNode.Child0ID];
				parentNode.Aabb = childNode0.Aabb;
			}
			else
			{
				const auto& childNode1 = _nodes[parentNode.Child1ID];
				parentNode.Aabb = childNode1.Aabb;
			}

			parentNodeID = parentNode.ParentID;
		}
	}

	int BoundingVolumeHierarchy::GetNewNodeID()
	{
		int nodeID = 0;

		// Get existing empty node.
		if (!_freeIds.empty())
		{
			nodeID = _freeIds.back();
			_freeIds.pop_back();
		}
		// Get new empty node.
		else
		{
			_nodes.emplace_back();
			nodeID = int(_nodes.size() - 1);
		}

		return nodeID;
	}

	int BoundingVolumeHierarchy::GetSiblingNodeID(int leafNodeID)
	{
		const auto& leafNode = _nodes[leafNodeID];
		int bestSiblingNodeID = _rootID;

		// Branch and bound for best sibling node.
		int prev = bestSiblingNodeID;
		while (!_nodes[bestSiblingNodeID].IsLeaf())
		{
			int child0ID = _nodes[bestSiblingNodeID].Child0ID;
			int child1ID = _nodes[bestSiblingNodeID].Child1ID;

			auto mergedAabb = BoundingBox();
			BoundingBox::CreateMerged(mergedAabb, _nodes[bestSiblingNodeID].Aabb, leafNode.Aabb);

			float area = Geometry::GetBoundingBoxArea(_nodes[bestSiblingNodeID].Aabb);
			float mergedArea = Geometry::GetBoundingBoxArea(mergedAabb);

			// Cost of creating new parent for this node and new leaf.
			float cost = mergedArea * 2;

			// Minimum cost of pushing leaf further down tree.
			float inheritCost = (mergedArea - area) * 2;

			// Cost of descending into child 0.
			float cost0 = 0.0f;
			if (child0ID != NO_VALUE && _nodes[child0ID].IsLeaf())
			{
				auto aabb = BoundingBox();
				BoundingBox::CreateMerged(aabb, leafNode.Aabb, _nodes[child0ID].Aabb);

				cost0 = Geometry::GetBoundingBoxArea(aabb) + inheritCost;
			}
			else if (child0ID != NO_VALUE)
			{
				auto aabb = BoundingBox();
				BoundingBox::CreateMerged(aabb, leafNode.Aabb, _nodes[child0ID].Aabb);

				float prevArea = Geometry::GetBoundingBoxArea(_nodes[child0ID].Aabb);
				float newArea = Geometry::GetBoundingBoxArea(aabb);
				cost0 = (newArea - prevArea) + inheritCost;
			}

			// Cost of descending into child 1.
			float cost1 = 0.0f;
			if (child1ID != NO_VALUE && _nodes[child1ID].IsLeaf())
			{
				auto aabb = BoundingBox();
				BoundingBox::CreateMerged(aabb, leafNode.Aabb, _nodes[child1ID].Aabb);

				cost1 = Geometry::GetBoundingBoxArea(aabb) + inheritCost;
			}
			else if (child1ID != NO_VALUE)
			{
				auto aabb = BoundingBox();
				BoundingBox::CreateMerged(aabb, leafNode.Aabb, _nodes[child1ID].Aabb);

				float prevArea = Geometry::GetBoundingBoxArea(_nodes[child1ID].Aabb);
				float newArea = Geometry::GetBoundingBoxArea(aabb);
				cost1 = newArea - prevArea + inheritCost;
			}

			// Descend according to minimum cost.
			if (cost < cost0 && cost < cost1)
				break;

			// Descend.
			if (cost0 < cost1)
			{
				if (child0ID != NO_VALUE)
				{
					bestSiblingNodeID = child0ID;
				}
				else
				{
					break;
				}
			}
			else if (child1ID != NO_VALUE)
			{
				if (child1ID != NO_VALUE)
				{
					bestSiblingNodeID = child1ID;
				}
				else
				{
					break;
				}
			}

			if (bestSiblingNodeID == prev)
				break;

			prev = bestSiblingNodeID;
		}

		return bestSiblingNodeID;
	}

	void BoundingVolumeHierarchy::RemoveNode(int nodeID)
	{
		if (nodeID < 0 || nodeID >= int(_nodes.size() - 1))
		{
			TENLog("BoundingVolumeHierarchy attempted to remove invalid node.", LogLevel::Warning);
			return;
		}

		// Clear node and mark free.
		auto& node = _nodes[nodeID];
		node = {};
		_freeIds.push_back(nodeID);

		// Clear tree if empty. NOTE: Prevents memory bloat, but may be slower.
		if (_nodes.size() == _freeIds.size())
			*this = {};
	}

	void BoundingVolumeHierarchy::BalanceNode(int leafNodeID)
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

	int BoundingVolumeHierarchy::Rebuild(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, int start, int end, float boundary)
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
			node.Child0ID = Rebuild(objectIds, aabbs, start, mid);
			node.Child1ID = Rebuild(objectIds, aabbs, mid, end);

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

	void BoundingVolumeHierarchy::Validate(int nodeID) const
	{
		if (nodeID == NO_VALUE)
			return;

		// Validate root.
		if (nodeID == _rootID)
			TENAssert(_nodes[nodeID].ParentID == NO_VALUE, "BVH root node cannot have parent.");

		// Get nodes.
		const auto& node = _nodes[nodeID];
		const auto& childNode0 = _nodes[node.Child0ID];
		const auto& childNode1 = _nodes[node.Child1ID];

		int child0ID = node.Child0ID;
		int child1ID = node.Child1ID;

		// Validate leaf.
		if (node.IsLeaf())
		{
			TENAssert(node.ObjectID != NO_VALUE, "BVH leaf node must have object ID.");
			TENAssert(child0ID == NO_VALUE, "BVH leaf node 0 cannot have children.");
			TENAssert(child1ID == NO_VALUE, "BVH leaf node 1 cannot have children.");
			return;
		}
		else
		{
			TENAssert(node.ObjectID == NO_VALUE, "BVH non-leaf node cannot have object ID.");
		}

		// Validate parent.
		TENAssert(_nodes[child0ID].ParentID == nodeID, "BVH node 0 has wrong parent.");
		TENAssert(_nodes[child1ID].ParentID == nodeID, "BVH node 1 has wrong parent.");

		// Validate AABB.
		auto aabb = BoundingBox();
		BoundingBox::CreateMerged(aabb, childNode0.Aabb, childNode1.Aabb);
		TENAssert(*(Vector3*)&aabb.Center == node.Aabb.Center && *(Vector3*)&aabb.Extents == node.Aabb.Extents, "BVH node AABB does not contain children.");

		// Validate recursively.
		Validate(child0ID);
		Validate(child1ID);
	}
}
