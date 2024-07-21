#include "framework.h"
#include "Specific/Structures/BoundingTree.h"

#include "Math/Math.h"
#include "Renderer/Renderer.h"

using namespace TEN::Math;
using TEN::Renderer::g_Renderer;

// TODO: Add licence? Heavily referenced this implementation, which has an MIT licence and requests attribution:
// https://github.com/erincatto/box2d/blob/main/src/collision/b2_dynamic_tree.cpp

namespace TEN::Structures
{
	bool BoundingTree::Node::IsLeaf() const
	{
		return (Child0ID == NO_VALUE && Child1ID == NO_VALUE);
	}

	BoundingTree::BoundingTree(const std::vector<int>& objectIds, const std::vector<BoundingBox>& aabbs)
	{
		TENAssert(objectIds.size() == aabbs.size(), "BoundingTree(): Object ID and AABB counts must be equal.");

		// Debug
		_nodes.clear();
		for (int i = 0; i < objectIds.size(); i++)
			Insert(objectIds[i], aabbs[i]);

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
		int nodeID = GetNewNodeID();
		auto& node = _nodes[nodeID];

		node.ObjectID = objectID;
		node.Aabb = BoundingBox(aabb.Center, aabb.Extents + Vector3(boundary));

		InsertLeafNode(nodeID);
	}

	void BoundingTree::DrawDebug() const
	{
		constexpr auto BOX_COLOR = Color(1.0f, 1.0f, 1.0f);

		for (const auto& node : _nodes)
		{
			//if (node.IsLeaf())
			//	continue;

			DrawDebugBox(node.Aabb, BOX_COLOR);
		}
		//DrawDebugBox(_nodes[_rootID].Aabb, BOX_COLOR);

		PrintDebugMessage("%d", _rootID);
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

	std::vector<int> BoundingTree::GetBoundedObjectIds() const
	{
		auto objectIds = std::vector<int>{};
		if (_nodes.empty())
			return objectIds;

		// Collect all object IDs.
		for (const auto& node : _nodes)
		{
			if (!node.IsLeaf())
				continue;

			objectIds.push_back(node.ObjectID);
		}

		return objectIds;
	}

	std::vector<int> BoundingTree::GetBoundedObjectIds(const std::function<bool(const Node& node)>& testCollRoutine) const
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
				DrawDebugBox(node.Aabb, Color(1, 1, 1)); //debug
				objectIds.push_back(node.ObjectID);
			}
			else
			{
				traverse(node.Child0ID);
				traverse(node.Child1ID);
			}
		};

		// Traverse tree from root node.
		traverse(_rootID);

		return objectIds;
	}

	void BoundingTree::InsertLeafNode(int nodeID)
	{
		auto& node = _nodes[nodeID];

		// 1) Create root if empty.
		if (_rootID == NO_VALUE)
		{
			_rootID = nodeID;
			return;
		}

		// 2) Get best sibling node for new leaf.
		int siblingNodeID = GetBestSiblingNodeID(nodeID);
		auto& siblingNode = _nodes[siblingNodeID];

		// 3) Create new parent.
		int prevParentNodeID = siblingNode.ParentID;
		int newParentNodeID = GetNewNodeID();
		auto& newParentNode = _nodes[newParentNodeID];

		// TODO: Attempting to merge triggers an assertion. Tree structure is invalid? AABBs currupted? No idea.
		//BoundingBox::CreateMerged(newParentNode.Aabb, siblingNode.Aabb, node.Aabb);
		newParentNode.ParentID = prevParentNodeID;
		newParentNode.Child0ID = siblingNodeID;
		newParentNode.Child1ID = nodeID;
		siblingNode.ParentID = newParentNodeID;
		node.ParentID = newParentNodeID;

		// Set new root or update previous parent.
		if (prevParentNodeID == NO_VALUE)
		{
			_rootID = newParentNodeID;
		}
		else
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

		// 4) Restructure branch.
		RefitNode(nodeID);

		//Validate(prevParentNodeID);
	}

	int BoundingTree::GetNewNodeID()
	{
		int nodeID = 0;

		// Get existing empty node ID.
		if (!_freeIds.empty())
		{
			nodeID = _freeIds.back();
			_freeIds.pop_back();
		}
		// Allocate and get new empty node ID.
		else
		{
			_nodes.emplace_back();
			nodeID = int(_nodes.size() - 1);
		}

		return nodeID;
	}

	int BoundingTree::GetBestSiblingNodeID(int nodeID)
	{
		const auto& leafNode = _nodes[nodeID];
		int bestSiblingNodeID = _rootID;

		// Branch and bound for best sibling node.
		int prevBestSiblingNodeID = bestSiblingNodeID;
		while (!_nodes[bestSiblingNodeID].IsLeaf())
		{
			int child0ID = _nodes[bestSiblingNodeID].Child0ID;
			int child1ID = _nodes[bestSiblingNodeID].Child1ID;

			auto mergedAabb = BoundingBox();
			BoundingBox::CreateMerged(mergedAabb, _nodes[bestSiblingNodeID].Aabb, leafNode.Aabb);

			float area = Geometry::GetBoundingBoxArea(_nodes[bestSiblingNodeID].Aabb);
			float mergedArea = Geometry::GetBoundingBoxArea(mergedAabb);

			// Cost of creating new parent for current node and new leaf.
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

			if (bestSiblingNodeID == prevBestSiblingNodeID)
				break;

			prevBestSiblingNodeID = bestSiblingNodeID;
		}

		return bestSiblingNodeID;
	}

	void BoundingTree::RemoveNode(int nodeID)
	{
		// Clear node and mark free.
		auto& node = _nodes[nodeID];
		node = {};
		_freeIds.push_back(nodeID);

		// Shrink capacity if empty. NOTE: Prevents memory bloat, but may be slower.
		if (_nodes.size() == _freeIds.size())
			*this = {};
	}

	void BoundingTree::RefitNode(int nodeID)
	{
		const auto& node = _nodes[nodeID];

		// Retread tree branch to refit AABBs.
		int parentNodeID = node.ParentID;
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

	void BoundingTree::BalanceNode(int nodeID)
	{
		int parentNodeID = _nodes[nodeID].ParentID;
		int grandparentNodeID = _nodes[parentNodeID].ParentID;

		if (grandparentNodeID == NO_VALUE)
			return;

		auto& leafNode = _nodes[nodeID];
		auto& parentNode = _nodes[parentNodeID];
		auto& grandParentNode = _nodes[grandparentNodeID];

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

		if (parentNode.Child0ID == nodeID)
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
			grandParentNode.Child0ID = nodeID;
		}
		else
		{
			grandParentNode.Child1ID = nodeID;
		}
		grandParentNode.Aabb = mergedAabb;
	}

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
