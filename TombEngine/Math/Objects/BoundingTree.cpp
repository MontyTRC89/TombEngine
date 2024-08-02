#include "framework.h"
#include "Math/Objects/BoundingTree.h"

#include "Math/Math.h"
#include "Renderer/Renderer.h"

using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

// TODO: Add licence? Heavily referenced this implementation, which has an MIT licence and requests attribution:
// https://github.com/erincatto/box2d/blob/main/src/collision/b2_dynamic_tree.cpp

// TODO:
// - Set correct node depths.
// - Why is traversal so expensive? Is the tree unbalanced?

namespace TEN::Math
{
	bool BoundingTree::Node::IsLeaf() const
	{
		return (LeftChildID == NO_VALUE && RightChildID == NO_VALUE);
	}

	std::vector<int> BoundingTree::GetBoundedObjectIds() const
	{
		auto objectIds = std::vector<int>{};
		if (_leafIDMap.empty())
			return objectIds;

		// Collect all object IDs.
		for (const auto& [objectID, nodeID] : _leafIDMap)
			objectIds.push_back(objectID);

		return objectIds;
	}

	BoundingTree::BoundingTree(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, float boundary)
	{
		TENAssert(objectIds.size() == aabbs.size(), "BoundingTree ctor: object ID and AABB counts not equal.");

		// Debug
		_nodes.clear();
		for (int i = 0; i < objectIds.size(); i++)
			Insert(objectIds[i], aabbs[i], boundary);

		Validate(_rootID);

		//Rebuild(objectIds, aabbs, 0, (int)objectIds.size());
		//_rootID = int(_nodes.size() - 1);
	}

	std::vector<int> BoundingTree::GetBoundedObjectIds(const Ray& ray, float dist) const
	{
		auto testColl = [&](const Node& node)
		{
			float intersectDist = 0.0f;
			return (node.Aabb.Intersects(ray.position, ray.direction, intersectDist) && intersectDist <= dist);
		};

		DrawDebugLine(ray.position, Geometry::TranslatePoint(ray.position, ray.direction, dist), Color(1,1,1));

		return GetBoundedObjectIds(testColl);
	}

	std::vector<int> BoundingTree::GetBoundedObjectIds(const BoundingBox& aabb) const
	{
		auto testColl = [&](const Node& node)
		{
			return node.Aabb.Intersects(aabb);
		};

		return GetBoundedObjectIds(testColl);
	}

	std::vector<int> BoundingTree::GetBoundedObjectIds(const BoundingOrientedBox& obb) const
	{
		auto testColl = [&](const Node& node)
		{
			return node.Aabb.Intersects(obb);
		};

		return GetBoundedObjectIds(testColl);
	}

	std::vector<int> BoundingTree::GetBoundedObjectIds(const BoundingSphere& sphere) const
	{
		auto testColl = [&](const Node& node)
		{
			return node.Aabb.Intersects(sphere);
		};

		return GetBoundedObjectIds(testColl);
	}

	void BoundingTree::Insert(int objectID, const BoundingBox& aabb, float boundary)
	{
		// Allocate new leaf.
		int leafID = GetNewNodeID();
		auto& leaf = _nodes[leafID];

		// Set object ID and AABB.
		leaf.ObjectID = objectID;
		leaf.Aabb = BoundingBox(aabb.Center, aabb.Extents + Vector3(boundary));

		// Insert new leaf.
		InsertLeaf(leafID);
	}

	void BoundingTree::Move(int objectID, const BoundingBox& aabb, float boundary)
	{
		// Find leaf containing object ID.
		auto it = _leafIDMap.find(objectID);
		if (it == _leafIDMap.end())
			return;

		// Get leaf.
		int leafID = it->second;
		auto& leaf = _nodes[leafID];

		// Test if current AABB is inside expanded AABB within extents threshold.
		if (leaf.Aabb.Contains(aabb) == ContainmentType::CONTAINS)
		{
			auto deltaExtents = leaf.Aabb.Extents - aabb.Extents;
			float threshold = boundary * 2;

			if (deltaExtents.x < threshold &&
				deltaExtents.y < threshold &&
				deltaExtents.z < threshold)
			{
				return;
			}
		}

		// Reinsert leaf.
		RemoveLeaf(leafID);
		Insert(objectID, aabb, boundary);
	}

	void BoundingTree::Remove(int objectID)
	{
		// Find leaf containing object ID.
		auto it = _leafIDMap.find(objectID);
		if (it == _leafIDMap.end())
			return;

		// Prune leaf.
		int leafID = it->second;
		RemoveLeaf(leafID);
	}

	void BoundingTree::DrawDebug() const
	{
		constexpr auto BOX_COLOR = Color(1.0f, 1.0f, 1.0f);

		PrintDebugMessage("BOUNDING TREE DEBUG");

		int farthestDepth = 0;
		for (const auto& node : _nodes)
		{
			//DrawDebugBox(node.Aabb, BOX_COLOR);
			farthestDepth = std::max(farthestDepth, node.Depth);
		}

		PrintDebugMessage("Nodes: %d", (int)_nodes.size());
		PrintDebugMessage("Farthest depth: %d", farthestDepth);
	}

	std::vector<int> BoundingTree::GetBoundedObjectIds(const std::function<bool(const Node& node)>& testCollRoutine) const
	{
		auto objectIds = std::vector<int>{};
		if (_nodes.empty())
			return objectIds;

		int traversalCount = 0;

		std::function<void(int)> traverse = [&](int nodeID)
		{
			traversalCount++;

			// Invalid node; return early.
			if (nodeID == NO_VALUE)
				return;

			const auto& node = _nodes[nodeID];

			// Test node collision.
			if (!testCollRoutine(node))
				return;

			// Collect object ID.
			if (node.IsLeaf())
			{
				objectIds.push_back(node.ObjectID);
			}
			// Traverse nodes.
			else
			{
				traverse(node.LeftChildID);
				traverse(node.RightChildID);
			}
		};

		// Traverse tree from root node.
		traverse(_rootID);

		PrintDebugMessage("traversal count: %d", traversalCount);
		return objectIds;
	}

	int BoundingTree::GetNewNodeID()
	{
		int nodeID = 0;

		// Get existing empty node ID.
		if (!_freeNodeIds.empty())
		{
			nodeID = _freeNodeIds.back();
			_freeNodeIds.pop_back();
		}
		// Allocate and get new empty node ID.
		else
		{
			_nodes.emplace_back();
			nodeID = (int)_nodes.size() - 1;
		}

		return nodeID;
	}

	int BoundingTree::GetBestSiblingLeafID(int leafID) const
	{
		const auto& leaf = _nodes[leafID];

		// Branch and bound for best sibling leaf.
		int siblingID = _rootID;
		while (!_nodes[siblingID].IsLeaf())
		{
			const auto& sibling = _nodes[siblingID];
			int child0ID = sibling.LeftChildID;
			int child1ID = sibling.RightChildID;

			float area = Geometry::GetBoundingBoxArea(sibling.Aabb);
			float inheritCost = Geometry::GetBoundingBoxArea(leaf.Aabb) * 2;

			// Calculate cost of creating new parent for sibling and new leaf.
			auto mergedAabb = BoundingBox();
			BoundingBox::CreateMerged(mergedAabb, sibling.Aabb, leaf.Aabb);
			float mergedArea = Geometry::GetBoundingBoxArea(mergedAabb);
			float cost = mergedArea * 2;

			// Calculate cost of descending into child 0.
			float cost0 = INFINITY;
			if (child0ID != NO_VALUE)
			{
				const auto& child0 = _nodes[child0ID];
				auto aabb = BoundingBox();
				BoundingBox::CreateMerged(aabb, child0.Aabb, leaf.Aabb);
				float newArea = Geometry::GetBoundingBoxArea(aabb);

				cost0 = child0.IsLeaf() ?
					newArea + inheritCost :
					(newArea - Geometry::GetBoundingBoxArea(child0.Aabb)) + inheritCost;
			}

			// Calculate cost of descending into child 1.
			float cost1 = INFINITY;
			if (child1ID != NO_VALUE)
			{
				const auto& child1 = _nodes[child1ID];
				auto aabb = BoundingBox();
				BoundingBox::CreateMerged(aabb, child1.Aabb, leaf.Aabb);
				float newArea = Geometry::GetBoundingBoxArea(aabb);

				cost1 = child1.IsLeaf() ?
					newArea + inheritCost :
					(newArea - Geometry::GetBoundingBoxArea(child1.Aabb)) + inheritCost;
			}

			// Test if descent is worthwhile according to minimum cost.
			if (cost < cost0 && cost < cost1)
				break;

			// Descend.
			siblingID = (cost0 < cost1) ? child0ID : child1ID;

			// FAILSAFE.
			if (siblingID == NO_VALUE)
				break;
		}

		return siblingID;
	}

	void BoundingTree::InsertLeaf(int leafID)
	{
		// Create root if empty.
		if (_rootID == NO_VALUE)
		{
			auto& leaf = _nodes[leafID];
			leaf.Depth = 0;

			_rootID = leafID;
			_leafIDMap.insert({ leaf.ObjectID, leafID });
			return;
		}

		// Allocate new parent.
		int parentID = GetNewNodeID();
		auto& parent = _nodes[parentID];

		// Get sibling leaf and new leaf.
		int siblingID = GetBestSiblingLeafID(leafID);
		auto& sibling = _nodes[siblingID];
		auto& leaf = _nodes[leafID];

		// Calculate merged AABB of sibling leaf and new leaf.
		auto aabb = BoundingBox();
		BoundingBox::CreateMerged(aabb, sibling.Aabb, leaf.Aabb);

		// Get previous parent.
		int prevParentID = sibling.ParentID;

		// Update nodes.
		parent.Aabb = aabb;
		parent.ParentID = prevParentID;
		parent.LeftChildID = siblingID;
		parent.RightChildID = leafID;
		sibling.ParentID = parentID;
		leaf.ParentID = parentID;

		if (prevParentID == NO_VALUE)
		{
			_rootID = parentID;
			parent.Depth = 0;
		}
		else
		{
			auto& prevParent = _nodes[prevParentID];

			// Update previous parent's child reference.
			if (prevParent.LeftChildID == siblingID)
			{
				prevParent.LeftChildID = parentID;
			}
			else
			{
				prevParent.RightChildID = parentID;
			}

			parent.Depth = prevParent.Depth + 1;
		}

		sibling.Depth = parent.Depth + 1;
		leaf.Depth = parent.Depth + 1;

		// Refit.
		RefitNode(leafID);

		// Store object-leaf association.
		_leafIDMap.insert({ leaf.ObjectID, leafID });

		//Validate(leafID);
	}

	void BoundingTree::RemoveLeaf(int leafID)
	{
		// Prune branch.
		int nodeID = leafID;
		while (nodeID != NO_VALUE)
		{
			const auto& node = _nodes[nodeID];

			// Remove node if both children are empty.
			if (node.LeftChildID == NO_VALUE && node.RightChildID == NO_VALUE)
			{
				int parentID = node.ParentID;
				if (parentID != NO_VALUE)
				{
					auto& parentNode = _nodes[parentID];
					if (parentNode.LeftChildID == nodeID)
					{
						parentNode.LeftChildID = NO_VALUE;
					}
					else if (parentNode.RightChildID == nodeID)
					{
						parentNode.RightChildID = NO_VALUE;
					}
				}

				RemoveNode(nodeID);
				nodeID = parentID;
			}
			// Refit last node.
			else
			{
				RefitNode(nodeID);
				break;
			}
		}
	}

	int BoundingTree::BalanceNode(int nodeID)
	{
		// Perform a left or right rotation if node A is imbalanced.
		// Returns the new root index.

		if (nodeID == NO_VALUE)
			return nodeID;

		auto& nodeA = _nodes[nodeID];
		if (nodeA.IsLeaf() || nodeA.Depth < 2)
			return nodeID;

		int nodeIDB = nodeA.LeftChildID;
		int nodeIDC = nodeA.RightChildID;
		if (nodeIDB == NO_VALUE || nodeIDC == NO_VALUE)
			return nodeID;

		auto& nodeB = _nodes[nodeIDB];
		auto& nodeC = _nodes[nodeIDC];

		int balance = nodeC.Depth - nodeB.Depth;

		// Rotate C up.
		if (balance > 1)
		{
			int nodeIDF = nodeC.LeftChildID;
			int nodeIDG = nodeC.RightChildID;
			if (nodeIDF == NO_VALUE || nodeIDG == NO_VALUE)
				return nodeID;

			auto& nodeF = _nodes[nodeIDF];
			auto& nodeG = _nodes[nodeIDG];

			// Swap A and C.
			nodeC.ParentID = nodeA.ParentID;
			nodeC.LeftChildID = nodeID;
			nodeA.ParentID = nodeIDC;

			// Make A's previous parent point to C.
			if (nodeC.ParentID != NO_VALUE)
			{
				if (_nodes[nodeC.ParentID].LeftChildID == nodeID)
				{
					_nodes[nodeC.ParentID].LeftChildID = nodeIDC;
				}
				else
				{
					_nodes[nodeC.ParentID].RightChildID = nodeIDC;
				}
			}
			else
			{
				_rootID = nodeIDC;
			}

			// Rotate.
			if (nodeF.Depth > nodeG.Depth)
			{
				BoundingBox::CreateMerged(nodeA.Aabb, nodeB.Aabb, nodeG.Aabb);
				BoundingBox::CreateMerged(nodeC.Aabb, nodeA.Aabb, nodeF.Aabb);
				nodeA.Depth = std::max(nodeB.Depth, nodeG.Depth) + 1;
				nodeC.Depth = std::max(nodeA.Depth, nodeF.Depth) + 1;

				nodeG.ParentID = nodeID;
				nodeC.RightChildID = nodeIDF;
				nodeA.RightChildID = nodeIDG;
			}
			else
			{
				BoundingBox::CreateMerged(nodeA.Aabb, nodeB.Aabb, nodeF.Aabb);
				BoundingBox::CreateMerged(nodeC.Aabb, nodeA.Aabb, nodeG.Aabb);
				nodeA.Depth = std::max(nodeB.Depth, nodeF.Depth) + 1;
				nodeC.Depth = std::max(nodeA.Depth, nodeG.Depth) + 1;

				nodeF.ParentID = nodeID;
				nodeC.RightChildID = nodeIDG;
				nodeA.RightChildID = nodeIDF;
			}

			return nodeIDC;
		}

		// Rotate B up.
		if (balance < -1)
		{
			int nodeIDD = nodeB.LeftChildID;
			int nodeIDE = nodeB.RightChildID;
			if (nodeIDD == NO_VALUE || nodeIDE == NO_VALUE)
				return nodeID;

			auto& nodeD = _nodes[nodeIDD];
			auto& nodeE = _nodes[nodeIDE];

			// Swap A and B.
			nodeB.ParentID = nodeA.ParentID;
			nodeB.LeftChildID = nodeID;
			nodeA.ParentID = nodeIDB;

			// Make A's previous parent point to B.
			if (nodeB.ParentID != NO_VALUE)
			{
				if (_nodes[nodeB.ParentID].LeftChildID == nodeID)
				{
					_nodes[nodeB.ParentID].LeftChildID = nodeIDB;
				}
				else
				{
					_nodes[nodeB.ParentID].RightChildID = nodeIDB;
				}
			}
			else
			{
				_rootID = nodeIDB;
			}

			// Rotate.
			if (nodeD.Depth > nodeE.Depth)
			{
				BoundingBox::CreateMerged(nodeA.Aabb, nodeC.Aabb, nodeE.Aabb);
				BoundingBox::CreateMerged(nodeB.Aabb, nodeA.Aabb, nodeD.Aabb);
				nodeA.Depth = std::max(nodeC.Depth, nodeE.Depth) + 1;
				nodeB.Depth = std::max(nodeA.Depth, nodeD.Depth) + 1;

				nodeB.RightChildID = nodeIDD;
				nodeA.LeftChildID = nodeIDE;
				nodeE.ParentID = nodeID;
			}
			else
			{
				BoundingBox::CreateMerged(nodeA.Aabb, nodeC.Aabb, nodeD.Aabb);
				BoundingBox::CreateMerged(nodeB.Aabb, nodeA.Aabb, nodeE.Aabb);
				nodeA.Depth = std::max(nodeC.Depth, nodeD.Depth) + 1;
				nodeB.Depth = std::max(nodeA.Depth, nodeE.Depth) + 1;

				nodeB.RightChildID = nodeIDE;
				nodeA.LeftChildID = nodeIDD;
				nodeD.ParentID = nodeID;
			}

			return nodeIDB;
		}

		return nodeID;
	}

	void BoundingTree::RefitNode(int nodeID)
	{
		const auto& node = _nodes[nodeID];

		// Retread tree branch to refit AABBs.
		int parentID = node.ParentID;
		while (parentID != NO_VALUE)
		{
			// Balance the node and get the new root of the subtree.
			int newParentID = BalanceNode(parentID);
			auto& parent = _nodes[newParentID];

			if (parent.LeftChildID != NO_VALUE && parent.RightChildID != NO_VALUE)
			{
				const auto& child0 = _nodes[parent.LeftChildID];
				const auto& child1 = _nodes[parent.RightChildID];

				BoundingBox::CreateMerged(parent.Aabb, child0.Aabb, child1.Aabb);
				parent.Depth = std::max(child0.Depth, child1.Depth) + 1;
			}
			else if (parent.LeftChildID != NO_VALUE)
			{
				const auto& child0 = _nodes[parent.LeftChildID];

				parent.Aabb = child0.Aabb;
				parent.Depth = child0.Depth + 1;
			}
			else if (parent.RightChildID != NO_VALUE)
			{
				const auto& child1 = _nodes[parent.RightChildID];

				parent.Aabb = child1.Aabb;
				parent.Depth = child1.Depth + 1;
			}

			int prevParentID = parentID;
			parentID = parent.ParentID;
		}
	}

	void BoundingTree::RemoveNode(int nodeID)
	{
		auto& node = _nodes[nodeID];

		// Remove leaf from map.
		if (node.IsLeaf())
			_leafIDMap.erase(node.ObjectID);

		// Clear node and mark free.
		node = {};
		_freeNodeIds.push_back(nodeID);

		// Shrink capacity if empty to prevent memory bloat.
		if (_nodes.size() == _freeNodeIds.size())
			*this = {};
	}

	// TODO: Refactor into a fast bottom-up algorithm that produces a balanced tree.
	int BoundingTree::Rebuild(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, int start, int end, float boundary)
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
			node.LeftChildID = Rebuild(objectIds, aabbs, start, mid);
			node.RightChildID = Rebuild(objectIds, aabbs, mid, end);

			// Set parent ID for child nodes.
			int newNodeID = (int)_nodes.size();
			if (node.LeftChildID != NO_VALUE)
			{
				_nodes[node.LeftChildID].ParentID = newNodeID;
			}
			if (node.RightChildID != NO_VALUE)
			{
				_nodes[node.RightChildID].ParentID = newNodeID;
			}

			_nodes.push_back(node);
			return newNodeID;
		}
	}

	void BoundingTree::Validate() const
	{
		Validate(_rootID);

		// Validate inner node and leaf node count relation.
		unsigned int innerNodeCount = 0;
		unsigned int leafNodeCount = 0;
		for (const auto& node : _nodes)
			node.IsLeaf() ? leafNodeCount++ : innerNodeCount++;
		TENAssert(innerNodeCount == (leafNodeCount - 1), "BoundingTree: Unexpected relation between inner node and leaf node counts.");

		// Validate unique object ID.
		auto objectIds = GetBoundedObjectIds();
		for (int refObjectID : objectIds)
		{
			unsigned int count = 0;
			for (int objectID : objectIds)
			{
				if (refObjectID == refObjectID)
				count++;
			}

			TENAssert(count == 1, "BoundingTree contains duplicate object IDs.");
		}
	}

	void BoundingTree::Validate(int nodeID) const
	{
		if (nodeID == NO_VALUE)
			return;

		// Get node.
		const auto& node = _nodes[nodeID];

		// Validate root.
		if (nodeID == _rootID)
			TENAssert(node.ParentID == NO_VALUE, "BoundingTree: Root node cannot have parent.");

		// Validate leaf node.
		if (node.IsLeaf())
		{
			TENAssert(node.ObjectID != NO_VALUE, "BoundingTree: Leaf node must contain object ID.");
		}
		// Validate inner node.
		else
		{
			TENAssert(node.ObjectID == NO_VALUE, "BoundingTree: Inner node cannot contain object ID.");
		}

		// Validate parent.
		if (nodeID != _rootID)
			TENAssert(node.ParentID != NO_VALUE, "BoundingTree: Non-root node must have parent.");

		// Validate parent of children.
		if (node.LeftChildID != NO_VALUE)
		{
			const auto& child0 = _nodes[node.LeftChildID];
			TENAssert(child0.ParentID == nodeID, "BoundingTree: Child node 0 has wrong parent.");
		}
		if (node.RightChildID != NO_VALUE)
		{
			const auto& child1 = _nodes[node.RightChildID];
			TENAssert(child1.ParentID == nodeID, "BoundingTree: Child node 1 has wrong parent.");
		}

		// Validate AABB.
		if (node.LeftChildID != NO_VALUE && node.RightChildID != NO_VALUE)
		{
			const auto& child0 = _nodes[node.LeftChildID];
			const auto& child1 = _nodes[node.RightChildID];

			auto aabb = BoundingBox();
			BoundingBox::CreateMerged(aabb, _nodes[node.LeftChildID].Aabb, _nodes[node.RightChildID].Aabb);
			TENAssert((Vector3)aabb.Center == node.Aabb.Center && (Vector3)aabb.Extents == node.Aabb.Extents, "BoundingTree: Node AABB does not contain children.");
		}

		// TODO: Invalid.
		// Validate depth.
		if (nodeID != _rootID)
		{
			const auto& parent = _nodes[node.ParentID];
			//TENAssert(node.Depth == (parent.Depth + 1), "BoundingTree: Node depth inconsistent with parent.");

			//PrintDebugMessage("%d, %d", node.Depth, parent.Depth);
		}

		// Validate recursively.
		Validate(node.LeftChildID);
		Validate(node.RightChildID);
	}
}
