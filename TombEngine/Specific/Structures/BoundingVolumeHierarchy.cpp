#include "framework.h"
#include "Specific/Structures/BoundingVolumeHierarchy.h"

#include <stack>

#include "Math/Math.h"

using namespace TEN::Math;

namespace TEN::Structures
{
	bool BoundingVolumeHierarchy::Node::IsLeaf() const
	{
		return (LeftChildID == NO_VALUE && RightChildID == NO_VALUE);
	}

	BoundingVolumeHierarchy::BoundingVolumeHierarchy(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, BvhBuildStrategy strategy)
	{
		TENAssert(objectIds.size() == aabbs.size(), "BoundingTree: Object ID and AABB counts unequal in static constructor.");

		Build(objectIds, aabbs, strategy);
	}

	std::vector<int> BoundingVolumeHierarchy::GetBoundedObjectIds() const
	{
		auto objectIds = std::vector<int>{};
		if (_leafIDMap.empty())
			return objectIds;

		// Collect all object IDs.
		for (const auto& [objectID, leafID] : _leafIDMap)
			objectIds.push_back(objectID);

		return objectIds;
	}

	std::vector<int> BoundingVolumeHierarchy::GetBoundedObjectIds(const Ray& ray, float dist) const
	{
		auto testColl = [&](const Node& node)
		{
			float intersectDist = 0.0f;
			return (node.Aabb.Intersects(ray.position, ray.direction, intersectDist) && intersectDist <= dist);
		};

		return GetBoundedObjectIds(testColl);
	}

	std::vector<int> BoundingVolumeHierarchy::GetBoundedObjectIds(const BoundingBox& aabb) const
	{
		auto testColl = [&](const Node& node)
		{
			return node.Aabb.Intersects(aabb);
		};

		return GetBoundedObjectIds(testColl);
	}

	std::vector<int> BoundingVolumeHierarchy::GetBoundedObjectIds(const BoundingOrientedBox& obb) const
	{
		auto testColl = [&](const Node& node)
		{
			return node.Aabb.Intersects(obb);
		};

		return GetBoundedObjectIds(testColl);
	}

	std::vector<int> BoundingVolumeHierarchy::GetBoundedObjectIds(const BoundingSphere& sphere) const
	{
		auto testColl = [&](const Node& node)
		{
			return node.Aabb.Intersects(sphere);
		};

		return GetBoundedObjectIds(testColl);
	}

	unsigned int BoundingVolumeHierarchy::Size() const
	{
		return (unsigned int)_leafIDMap.size();
	}

	bool BoundingVolumeHierarchy::Empty() const
	{
		return _leafIDMap.empty();
	}

	void BoundingVolumeHierarchy::Insert(int objectID, const BoundingBox& aabb, float boundary)
	{
		// Allocate new leaf.
		int leafID = GetNewNodeID();
		auto& leaf = _nodes[leafID];

		// Set initial parameters.
		leaf.ObjectID = objectID;
		leaf.Aabb = BoundingBox(aabb.Center, aabb.Extents + Vector3(boundary));
		leaf.Height = 0;

		// Insert new leaf.
		InsertLeaf(leafID);
	}

	void BoundingVolumeHierarchy::Move(int objectID, const BoundingBox& aabb, float boundary)
	{
		// Find leaf containing object ID.
		auto it = _leafIDMap.find(objectID);
		if (it == _leafIDMap.end())
		{
			TENLog("BoundingTree: Attempted to move missing leaf with object ID " + std::to_string(objectID) + ".", LogLevel::Warning);
			return;
		}

		// Get leaf.
		int leafID = it->second;
		auto& leaf = _nodes[leafID];

		// Test if object AABB is inside node AABB.
		if (leaf.Aabb.Contains(aabb) == ContainmentType::CONTAINS)
		{
			auto deltaExtents = leaf.Aabb.Extents - aabb.Extents;
			float threshold = boundary * 2;

			// Test if object AABB is significantly smaller than node AABB.
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

	void BoundingVolumeHierarchy::Remove(int objectID)
	{
		// Find leaf containing object ID.
		auto it = _leafIDMap.find(objectID);
		if (it == _leafIDMap.end())
		{
			TENLog("BoundingTree: Attempted to remove missing leaf with object ID " + std::to_string(objectID) + ".", LogLevel::Warning);
			return;
		}

		// Remove leaf.
		int leafID = it->second;
		RemoveLeaf(leafID);
	}

	void BoundingVolumeHierarchy::DrawDebug() const
	{
		constexpr auto BOX_COLOR = Color(1.0f, 1.0f, 1.0f, 0.5f);

		PrintDebugMessage("BOUNDING TREE DEBUG");

		if (!_nodes.empty())
		{
			PrintDebugMessage("Nodes: %d", (int)_nodes.size());
			PrintDebugMessage("Root height: %d", _nodes[_rootID].Height);
		}

		for (const auto& node : _nodes)
			DrawDebugBox(node.Aabb, BOX_COLOR);
	}

	std::vector<int> BoundingVolumeHierarchy::GetBoundedObjectIds(const std::function<bool(const Node& node)>& testCollRoutine) const
	{
		auto objectIds = std::vector<int>{};
		if (_nodes.empty())
			return objectIds;

		// Initialize stack.
		auto nodeIDStack = std::stack<int>{};
		nodeIDStack.push(_rootID);

		// Traverse tree.
		while (!nodeIDStack.empty())
		{
			int nodeID = nodeIDStack.top();
			nodeIDStack.pop();

			// Invalid node; continue.
			if (nodeID == NO_VALUE)
				continue;

			const auto& node = _nodes[nodeID];

			// Test node collision.
			if (!testCollRoutine(node))
				continue;

			// Leaf node; collect object ID.
			if (node.IsLeaf())
			{
				objectIds.push_back(node.ObjectID);
			}
			// Inner node; push children onto stack for traversal.
			else
			{
				if (node.LeftChildID != NO_VALUE)
					nodeIDStack.push(node.LeftChildID);

				if (node.RightChildID != NO_VALUE)
					nodeIDStack.push(node.RightChildID);
			}
		}

		return objectIds;
	}

	int BoundingVolumeHierarchy::GetNewNodeID()
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
			nodeID = (int)_nodes.size();
			_nodes.emplace_back();
		}

		return nodeID;
	}

	int BoundingVolumeHierarchy::GetBestSiblingLeafID(int leafID) const
	{
		const auto& leaf = _nodes[leafID];

		// Branch and bound for best sibling leaf.
		int siblingID = _rootID;
		while (!_nodes[siblingID].IsLeaf())
		{
			const auto& sibling = _nodes[siblingID];
			int leftChildID = sibling.LeftChildID;
			int rightChildID = sibling.RightChildID;

			float area = Geometry::GetBoundingBoxArea(sibling.Aabb);
			float inheritCost = Geometry::GetBoundingBoxArea(leaf.Aabb) * 2;

			// Calculate cost of creating new parent for sibling and new leaf.
			auto mergedAabb = BoundingBox();
			BoundingBox::CreateMerged(mergedAabb, sibling.Aabb, leaf.Aabb);
			float mergedArea = Geometry::GetBoundingBoxArea(mergedAabb);
			float cost = mergedArea * 2;

			// Calculate cost of descending into left child.
			float leftCost = INFINITY;
			if (leftChildID != NO_VALUE)
			{
				const auto& leftChild = _nodes[leftChildID];
				auto aabb = BoundingBox();
				BoundingBox::CreateMerged(aabb, leftChild.Aabb, leaf.Aabb);
				float newArea = Geometry::GetBoundingBoxArea(aabb);

				leftCost = leftChild.IsLeaf() ?
					newArea + inheritCost :
					(newArea - Geometry::GetBoundingBoxArea(leftChild.Aabb)) + inheritCost;
			}

			// Calculate cost of descending into right child.
			float rightCost = INFINITY;
			if (rightChildID != NO_VALUE)
			{
				const auto& rightChild = _nodes[rightChildID];
				auto aabb = BoundingBox();
				BoundingBox::CreateMerged(aabb, rightChild.Aabb, leaf.Aabb);
				float newArea = Geometry::GetBoundingBoxArea(aabb);

				rightCost = rightChild.IsLeaf() ?
					newArea + inheritCost :
					(newArea - Geometry::GetBoundingBoxArea(rightChild.Aabb)) + inheritCost;
			}

			// Test if descent is worthwhile according to minimum cost.
			if (cost < leftCost && cost < rightCost)
				break;

			// Descend.
			siblingID = (leftCost < rightCost) ? leftChildID : rightChildID;
			if (siblingID == NO_VALUE)
			{
				TENLog("BoundingTree: Sibling leaf search failed.", LogLevel::Warning);
				break;
			}
		}

		return siblingID;
	}

	void BoundingVolumeHierarchy::InsertLeaf(int leafID)
	{
		// Create root if empty.
		if (_rootID == NO_VALUE)
		{
			auto& leaf = _nodes[leafID];
			leaf.Height = 0;

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
		parent.Height = sibling.Height + 1;
		parent.ParentID = prevParentID;
		parent.LeftChildID = siblingID;
		parent.RightChildID = leafID;
		sibling.ParentID = parentID;
		leaf.ParentID = parentID;

		if (prevParentID == NO_VALUE)
		{
			_rootID = parentID;
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
		}

		// Refit.
		RefitNode(leafID);

		// Store object-leaf association.
		_leafIDMap.insert({ leaf.ObjectID, leafID });

		//Validate(leafID);
	}

	void BoundingVolumeHierarchy::RemoveLeaf(int leafID)
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
				nodeID = NO_VALUE;
			}
		}
	}

	void BoundingVolumeHierarchy::RefitNode(int nodeID)
	{
		const auto& node = _nodes[nodeID];

		// Retread tree branch to refit AABBs.
		int parentID = node.ParentID;
		while (parentID != NO_VALUE)
		{
			// Balance node and get new subtree root.
			int newParentID = BalanceNode(parentID);
			auto& parent = _nodes[newParentID];

			if (parent.LeftChildID != NO_VALUE && parent.RightChildID != NO_VALUE)
			{
				const auto& leftChild = _nodes[parent.LeftChildID];
				const auto& rightChild = _nodes[parent.RightChildID];

				BoundingBox::CreateMerged(parent.Aabb, leftChild.Aabb, rightChild.Aabb);
				parent.Height = std::max(leftChild.Height, rightChild.Height) + 1;
			}
			else if (parent.LeftChildID != NO_VALUE)
			{
				const auto& leftChild = _nodes[parent.LeftChildID];

				parent.Aabb = leftChild.Aabb;
				parent.Height = leftChild.Height + 1;
			}
			else if (parent.RightChildID != NO_VALUE)
			{
				const auto& rightChild = _nodes[parent.RightChildID];

				parent.Aabb = rightChild.Aabb;
				parent.Height = rightChild.Height + 1;
			}

			int prevParentID = parentID;
			parentID = parent.ParentID;
		}
	}

	void BoundingVolumeHierarchy::RemoveNode(int nodeID)
	{
		auto& node = _nodes[nodeID];

		// Remove leaf from map.
		if (node.IsLeaf())
			_leafIDMap.erase(node.ObjectID);

		// Clear node and mark free.
		node = {};
		_freeNodeIds.push_back(nodeID);

		// Shrink capacity if empty to avoid memory bloat.
		if (_nodes.size() == _freeNodeIds.size())
			*this = {};
	}

	// Performs left or right tree rotation if input node is imbalanced.
	// Returns new subtree root ID.
	int BoundingVolumeHierarchy::BalanceNode(int nodeID)
	{
		if (nodeID == NO_VALUE)
			return nodeID;

		auto& nodeA = _nodes[nodeID];
		if (nodeA.IsLeaf() || nodeA.Height < 2)
			return nodeID;

		int nodeIDB = nodeA.LeftChildID;
		int nodeIDC = nodeA.RightChildID;
		if (nodeIDB == NO_VALUE || nodeIDC == NO_VALUE)
			return nodeID;

		auto& nodeB = _nodes[nodeIDB];
		auto& nodeC = _nodes[nodeIDC];

		// Calculate balance.
		int balance = nodeC.Height - nodeB.Height;

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
				auto& parent = _nodes[nodeC.ParentID];
				if (parent.LeftChildID == nodeID)
				{
					parent.LeftChildID = nodeIDC;
				}
				else
				{
					parent.RightChildID = nodeIDC;
				}
			}
			else
			{
				_rootID = nodeIDC;
			}

			// Rotate.
			if (nodeF.Height > nodeG.Height)
			{
				BoundingBox::CreateMerged(nodeA.Aabb, nodeB.Aabb, nodeG.Aabb);
				BoundingBox::CreateMerged(nodeC.Aabb, nodeA.Aabb, nodeF.Aabb);
				nodeA.Height = std::max(nodeB.Height, nodeG.Height) + 1;
				nodeC.Height = std::max(nodeA.Height, nodeF.Height) + 1;

				nodeG.ParentID = nodeID;
				nodeC.RightChildID = nodeIDF;
				nodeA.RightChildID = nodeIDG;
			}
			else
			{
				BoundingBox::CreateMerged(nodeA.Aabb, nodeB.Aabb, nodeF.Aabb);
				BoundingBox::CreateMerged(nodeC.Aabb, nodeA.Aabb, nodeG.Aabb);
				nodeA.Height = std::max(nodeB.Height, nodeF.Height) + 1;
				nodeC.Height = std::max(nodeA.Height, nodeG.Height) + 1;

				nodeF.ParentID = nodeID;
				nodeC.RightChildID = nodeIDG;
				nodeA.RightChildID = nodeIDF;
			}

			return nodeIDC;
		}
		// Rotate B up.
		else if (balance < -1)
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
				auto& parent = _nodes[nodeB.ParentID];
				if (parent.LeftChildID == nodeID)
				{
					parent.LeftChildID = nodeIDB;
				}
				else
				{
					parent.RightChildID = nodeIDB;
				}
			}
			else
			{
				_rootID = nodeIDB;
			}

			// Rotate.
			if (nodeD.Height > nodeE.Height)
			{
				BoundingBox::CreateMerged(nodeA.Aabb, nodeC.Aabb, nodeE.Aabb);
				BoundingBox::CreateMerged(nodeB.Aabb, nodeA.Aabb, nodeD.Aabb);
				nodeA.Height = std::max(nodeC.Height, nodeE.Height) + 1;
				nodeB.Height = std::max(nodeA.Height, nodeD.Height) + 1;

				nodeB.RightChildID = nodeIDD;
				nodeA.LeftChildID = nodeIDE;
				nodeE.ParentID = nodeID;
			}
			else
			{
				BoundingBox::CreateMerged(nodeA.Aabb, nodeC.Aabb, nodeD.Aabb);
				BoundingBox::CreateMerged(nodeB.Aabb, nodeA.Aabb, nodeE.Aabb);
				nodeA.Height = std::max(nodeC.Height, nodeD.Height) + 1;
				nodeB.Height = std::max(nodeA.Height, nodeE.Height) + 1;

				nodeB.RightChildID = nodeIDE;
				nodeA.LeftChildID = nodeIDD;
				nodeD.ParentID = nodeID;
			}

			return nodeIDB;
		}

		return nodeID;
	}

	void BoundingVolumeHierarchy::Build(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, BvhBuildStrategy strategy)
	{
		_nodes.reserve(objectIds.size());
		Build(objectIds, aabbs, 0, (int)objectIds.size(), strategy);
		_rootID = (int)_nodes.size() - 1;

		//Validate();
	}

	int BoundingVolumeHierarchy::Build(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs, int start, int end, BvhBuildStrategy strategy)
	{
		constexpr auto BALANCED_SPLIT_RANGE_MAX = 10;

		// FAILSAFE.
		if (start >= end)
			return NO_VALUE;

		// Create new node.
		auto node = Node{};

		// Combine AABBs.
		node.Aabb = aabbs[start];
		for (int i = (start + 1); i < end; i++)
			BoundingBox::CreateMerged(node.Aabb, node.Aabb, aabbs[i]);

		// Leaf node.
		if ((end - start) == 1)
		{
			node.ObjectID = objectIds[start];
			node.Height = 0;

			// Add new leaf.
			int leafID = (int)_nodes.size();
			_nodes.push_back(node);

			// Store object-leaf association.
			_leafIDMap.insert({ node.ObjectID, leafID });

			return leafID;
		}
		// Inner node.
		else
		{
			auto getBestSplit = [&]()
			{
				int bestSplit = (start + end) / 2;

				// Median split.
				if (strategy == BvhBuildStrategy::Fast)
					return bestSplit;

				// Surface area heuristic (SAH).
				int range = 0;
				if (strategy == BvhBuildStrategy::Balanced)
				{
					range = BALANCED_SPLIT_RANGE_MAX;
				}
				else if (strategy == BvhBuildStrategy::Accurate)
				{
					range = end - start;
				}

				float bestCost = INFINITY;
				for (int split = std::max(start + 1, bestSplit - range); split < std::min(end, bestSplit + range); split++)
				{
					// Calculate AABB 0.
					auto aabb0 = aabbs[start];
					for (int i = (start + 1); i < split; i++)
						BoundingBox::CreateMerged(aabb0, aabb0, aabbs[i]);

					// Calculate AABB 1.
					auto aabb1 = aabbs[split];
					for (int i = split; i < end; i++)
						BoundingBox::CreateMerged(aabb1, aabb1, aabbs[i]);

					// Calculate cost.
					float surfaceArea0 = Geometry::GetBoundingBoxArea(aabb0);
					float surfaceArea1 = Geometry::GetBoundingBoxArea(aabb1);
					float cost = (surfaceArea0 * (split - start)) + (surfaceArea1 * (end - split));

					// Track best split.
					if (cost < bestCost)
					{
						bestSplit = split;
						bestCost = cost;
					}
				}

				return bestSplit;
			};

			int bestSplit = getBestSplit();

			// Create children recursively.
			node.LeftChildID = Build(objectIds, aabbs, start, bestSplit, strategy);
			node.RightChildID = Build(objectIds, aabbs, bestSplit, end, strategy);

			// Set parent ID for children.
			int nodeID = (int)_nodes.size();
			if (node.LeftChildID != NO_VALUE)
				_nodes[node.LeftChildID].ParentID = nodeID;
			if (node.RightChildID != NO_VALUE)
				_nodes[node.RightChildID].ParentID = nodeID;

			// Add new inner node and set height.
			_nodes.push_back(node);
			_nodes[nodeID].Height = std::max(
				(node.LeftChildID != NO_VALUE) ? _nodes[node.LeftChildID].Height : 0, 
				(node.RightChildID != NO_VALUE) ? _nodes[node.RightChildID].Height : 0) + 1;

			return nodeID;
		}
	}

	void BoundingVolumeHierarchy::Validate() const
	{
		Validate(_rootID);

		// Validate unique object IDs.
		auto objectIds = GetBoundedObjectIds();
		for (int refObjectID : objectIds)
		{
			unsigned int count = 0;
			for (int objectID : objectIds)
			{
				if (refObjectID == objectID)
					count++;
			}

			TENAssert(count == 1, "BoundingTree: Duplicate object IDs contained.");
		}
	}

	void BoundingVolumeHierarchy::Validate(int nodeID) const
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
			TENAssert(node.Height == 0, "BoundingTree: Leaf node must have height of 0.");
		}
		// Validate inner node.
		else
		{
			TENAssert(node.ObjectID == NO_VALUE, "BoundingTree: Inner node cannot contain object ID.");
			TENAssert(node.Height != 0, "BoundingTree: Inner node cannot have height of 0.");
		}

		// Validate parent.
		if (nodeID != _rootID)
			TENAssert(node.ParentID != NO_VALUE, "BoundingTree: Non-root node must have parent.");

		// Validate parent of children.
		if (node.LeftChildID != NO_VALUE)
		{
			const auto& leftChild = _nodes[node.LeftChildID];
			TENAssert(leftChild.ParentID == nodeID, "BoundingTree: Left child has wrong parent.");
		}
		if (node.RightChildID != NO_VALUE)
		{
			const auto& rightChild = _nodes[node.RightChildID];
			TENAssert(rightChild.ParentID == nodeID, "BoundingTree: Right child has wrong parent.");
		}

		// Validate height.
		if (nodeID != _rootID)
		{
			const auto& parent = _nodes[node.ParentID];
			TENAssert(node.Height < parent.Height, "BoundingTree: Child height must be less than parent height.");
		}

		// Validate recursively.
		Validate(node.LeftChildID);
		Validate(node.RightChildID);
	}
}
