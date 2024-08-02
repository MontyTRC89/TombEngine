#include "framework.h"
#include "Math/Objects/BoundingTree.h"

#include "Math/Math.h"
#include "Renderer/Renderer.h"

using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

// TODO: Add licence? Heavily referenced this implementation, which has an MIT licence and requests attribution:
// https://github.com/erincatto/box2d/blob/main/src/collision/b2_dynamic_tree.cpp

namespace TEN::Math
{
	bool BoundingTree::Node::IsLeaf() const
	{
		return (Child0ID == NO_VALUE && Child1ID == NO_VALUE);
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
		int leafID = GetNewNodeID();
		auto& leaf = _nodes[leafID];

		leaf.ObjectID = objectID;
		leaf.Aabb = BoundingBox(aabb.Center, aabb.Extents + Vector3(boundary));

		InsertLeaf(leafID);
	}

	void BoundingTree::Move(int objectID, const BoundingBox& aabb, float boundary)
	{
		// Find leaf containing object ID.
		auto it = _leafIDMap.find(objectID);
		if (it == _leafIDMap.end())
			return;

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

		int farthestDepth = 0;
		//for (const auto& node : _nodes)
		{
			//if (node.Depth == 3)
				DrawDebugBox(_nodes[_rootID].Aabb, BOX_COLOR);
				//DrawDebugBox(node.Aabb, BOX_COLOR);

			//farthestDepth = std::max(farthestDepth, node.Depth);
			//PrintDebugMessage("object ID: %d", _rootID);
		}

		PrintDebugMessage("nodes: %d", (int)_nodes.size());
		//PrintDebugMessage("farthest depth: %d", farthestDepth);
	}

	void BoundingTree::Validate() const
	{
		Validate(_rootID);
	}

	void BoundingTree::Validate(int nodeID) const
	{
		if (nodeID == NO_VALUE)
			return;

		// Validate root.
		if (nodeID == _rootID)
			TENAssert(_nodes[nodeID].ParentID == NO_VALUE, "BoundingTree root node cannot have parent.");

		// Get node.
		const auto& node = _nodes[nodeID];

		// Validate leaf.
		if (node.IsLeaf())
		{
			TENAssert(node.ObjectID != NO_VALUE, "BoundingTree leaf node must have object ID.");
			TENAssert(node.Child0ID == NO_VALUE, "BoundingTree leaf node 0 cannot have children.");
			TENAssert(node.Child1ID == NO_VALUE, "BoundingTree leaf node 1 cannot have children.");
			return;
		}
		else
		{
			TENAssert(node.ObjectID == NO_VALUE, "BoundingTree non-leaf node cannot have object ID.");
		}

		// Validate parent.
		if (node.Child0ID != NO_VALUE)
			if (_nodes[node.Child0ID].ParentID != nodeID) TENLog("BoundingTree child node 0 has wrong parent " + std::to_string(_nodes[node.Child0ID].ParentID) + " instead of " + std::to_string(nodeID) + ".");
		//TENAssert(_nodes[node.Child0ID].ParentID == nodeID, "BoundingTree child node 0 has wrong parent " + std::to_string(_nodes[node.Child0ID].ParentID) + " instead of " + std::to_string(nodeID) + ".");
		if (node.Child1ID != NO_VALUE)
			if (_nodes[node.Child1ID].ParentID != nodeID) TENLog("BoundingTree child node 1 has wrong parent " + std::to_string(_nodes[node.Child1ID].ParentID) + " instead of " + std::to_string(nodeID) + ".");
		//TENAssert(_nodes[node.Child1ID].ParentID == nodeID, "BoundingTree child node 1 has wrong parent " + std::to_string(_nodes[node.Child1ID].ParentID) + " instead of " + std::to_string(nodeID) + ".");

		// Validate unique object ID.
		
		// Validate AABB.
		if (node.Child0ID != NO_VALUE && node.Child1ID != NO_VALUE)
		{
			auto aabb = BoundingBox();
			BoundingBox::CreateMerged(aabb, _nodes[node.Child0ID].Aabb, _nodes[node.Child1ID].Aabb);
			//TENAssert((Vector3)aabb.Center == node.Aabb.Center && (Vector3)aabb.Extents == node.Aabb.Extents, "BoundingTree node AABB does not contain children.");
		}

		// Validate recursively.
		Validate(node.Child0ID);
		Validate(node.Child1ID);
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
				//DrawDebugBox(node.Aabb, Color(1, 1, 1)); //debug
				//PrintDebugMessage("depth: %d", node.Depth);
				objectIds.push_back(node.ObjectID);
			}
			// Traverse nodes.
			else
			{
				traverse(node.Child0ID);
				traverse(node.Child1ID);
			}
		};

		// Traverse tree from root node.
		traverse(_rootID);

		//PrintDebugMessage("traversal count: %d", traversalCount);
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
			int child0ID = sibling.Child0ID;
			int child1ID = sibling.Child1ID;

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
		auto& leaf = _nodes[leafID];
		_leafIDMap.insert({ leaf.ObjectID, leafID });

		// Create root if empty.
		if (_rootID == NO_VALUE)
		{
			_rootID = leafID;
			leaf.Depth = 0;
			return;
		}

		// TODO: Watch out: data corruption occurs with references when vector resizes.
		int newParentID = GetNewNodeID();

		// Get sibling for new leaf.
		int siblingID = GetBestSiblingLeafID(leafID);
		auto& sibling = _nodes[siblingID];

		// Calculate merged AABB of sibling and new leaf.
		auto aabb = BoundingBox();
		BoundingBox::CreateMerged(aabb, sibling.Aabb, leaf.Aabb);

		// Create new parent.
		int prevParentID = sibling.ParentID;
		auto& newParent = _nodes[newParentID];

		// Update nodes.
		newParent.Aabb = aabb;
		newParent.ParentID = prevParentID;
		newParent.Child0ID = siblingID;
		newParent.Child1ID = leafID;
		sibling.ParentID = newParentID;
		leaf.ParentID = newParentID;

		if (prevParentID == NO_VALUE)
		{
			_rootID = newParentID;
			newParent.Depth = 0;
		}
		else
		{
			auto& prevParent = _nodes[prevParentID];

			// Update previous parent's child reference.
			if (prevParent.Child0ID == siblingID)
			{
				prevParent.Child0ID = newParentID;
			}
			else
			{
				prevParent.Child1ID = newParentID;
			}

			newParent.Depth = prevParent.Depth + 1;
		}

		sibling.Depth = newParent.Depth + 1;
		leaf.Depth = newParent.Depth + 1;

		// Refit.
		RefitNode(leafID);

		//Validate(prevParentID);
	}

	void BoundingTree::RemoveLeaf(int leafID)
	{
		// Prune branch.
		int nodeID = leafID;
		while (nodeID != NO_VALUE)
		{
			const auto& node = _nodes[nodeID];

			// Remove node if both children are empty.
			if (node.Child0ID == NO_VALUE && node.Child1ID == NO_VALUE)
			{
				int parentID = node.ParentID;
				if (parentID != NO_VALUE)
				{
					auto& parentNode = _nodes[parentID];
					if (parentNode.Child0ID == nodeID)
					{
						parentNode.Child0ID = NO_VALUE;
					}
					else if (parentNode.Child1ID == nodeID)
					{
						parentNode.Child1ID = NO_VALUE;
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

		int nodeIDB = nodeA.Child0ID;
		int nodeIDC = nodeA.Child1ID;
		if (nodeIDB == NO_VALUE || nodeIDC == NO_VALUE)
			return nodeID;

		auto& nodeB = _nodes[nodeIDB];
		auto& nodeC = _nodes[nodeIDC];

		int balance = nodeC.Depth - nodeB.Depth;

		// Rotate C up.
		if (balance > 1)
		{
			int nodeIDF = nodeC.Child0ID;
			int nodeIDG = nodeC.Child1ID;
			if (nodeIDF == NO_VALUE || nodeIDG == NO_VALUE)
				return nodeID;

			auto& nodeF = _nodes[nodeIDF];
			auto& nodeG = _nodes[nodeIDG];

			// Swap A and C.
			nodeC.ParentID = nodeA.ParentID;
			nodeC.Child0ID = nodeID;
			nodeA.ParentID = nodeIDC;

			// Make A's previous parent point to C.
			if (nodeC.ParentID != NO_VALUE)
			{
				if (_nodes[nodeC.ParentID].Child0ID == nodeID)
				{
					_nodes[nodeC.ParentID].Child0ID = nodeIDC;
				}
				else
				{
					_nodes[nodeC.ParentID].Child1ID = nodeIDC;
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
				nodeC.Child1ID = nodeIDF;
				nodeA.Child1ID = nodeIDG;
			}
			else
			{
				BoundingBox::CreateMerged(nodeA.Aabb, nodeB.Aabb, nodeF.Aabb);
				BoundingBox::CreateMerged(nodeC.Aabb, nodeA.Aabb, nodeG.Aabb);
				nodeA.Depth = std::max(nodeB.Depth, nodeF.Depth) + 1;
				nodeC.Depth = std::max(nodeA.Depth, nodeG.Depth) + 1;

				nodeF.ParentID = nodeID;
				nodeC.Child1ID = nodeIDG;
				nodeA.Child1ID = nodeIDF;
			}

			return nodeIDC;
		}

		// Rotate B up.
		if (balance < -1)
		{
			int nodeIDD = nodeB.Child0ID;
			int nodeIDE = nodeB.Child1ID;
			if (nodeIDD == NO_VALUE || nodeIDE == NO_VALUE)
				return nodeID;

			auto& nodeD = _nodes[nodeIDD];
			auto& nodeE = _nodes[nodeIDE];

			// Swap A and B.
			nodeB.ParentID = nodeA.ParentID;
			nodeB.Child0ID = nodeID;
			nodeA.ParentID = nodeIDB;

			// Make A's previous parent point to B.
			if (nodeB.ParentID != NO_VALUE)
			{
				if (_nodes[nodeB.ParentID].Child0ID == nodeID)
				{
					_nodes[nodeB.ParentID].Child0ID = nodeIDB;
				}
				else
				{
					_nodes[nodeB.ParentID].Child1ID = nodeIDB;
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

				nodeB.Child1ID = nodeIDD;
				nodeA.Child0ID = nodeIDE;
				nodeE.ParentID = nodeID;
			}
			else
			{
				BoundingBox::CreateMerged(nodeA.Aabb, nodeC.Aabb, nodeD.Aabb);
				BoundingBox::CreateMerged(nodeB.Aabb, nodeA.Aabb, nodeE.Aabb);
				nodeA.Depth = std::max(nodeC.Depth, nodeD.Depth) + 1;
				nodeB.Depth = std::max(nodeA.Depth, nodeE.Depth) + 1;

				nodeB.Child1ID = nodeIDE;
				nodeA.Child0ID = nodeIDD;
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

			if (parent.Child0ID != NO_VALUE && parent.Child1ID != NO_VALUE)
			{
				const auto& child0 = _nodes[parent.Child0ID];
				const auto& child1 = _nodes[parent.Child1ID];

				BoundingBox::CreateMerged(parent.Aabb, child0.Aabb, child1.Aabb);
				parent.Depth = std::max(child0.Depth, child1.Depth) + 1;
			}
			else if (parent.Child0ID != NO_VALUE)
			{
				const auto& child0 = _nodes[parent.Child0ID];

				parent.Aabb = child0.Aabb;
				parent.Depth = child0.Depth + 1;
			}
			else if (parent.Child1ID != NO_VALUE)
			{
				const auto& child1 = _nodes[parent.Child1ID];

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
}
