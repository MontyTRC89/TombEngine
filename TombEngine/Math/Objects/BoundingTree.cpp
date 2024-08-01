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
		const auto& leaf = _nodes[leafID];

		// Previous expanded AABB contains current AABB within scale threshold; return early.
		if (leaf.Aabb.Contains(aabb) == ContainmentType::CONTAINS)
		{
			/*auto extentsThreshold = (leaf.Aabb.Extents + Vector3(boundary * 2)) / 2;
			if (!(aabb.Extents.x < extentsThreshold.x ||
				aabb.Extents.y < extentsThreshold.y ||
				aabb.Extents.z < extentsThreshold.z))*/
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

		for (const auto& node : _nodes)
			DrawDebugBox(node.Aabb, BOX_COLOR);
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

		std::function<void(int)> traverse = [&](int nodeID)
		{
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
			nodeID = int(_nodes.size() - 1);
		}

		return nodeID;
	}

	int BoundingTree::GetBestSiblingLeafID(int leafID) const
	{
		const auto& leaf = _nodes[leafID];

		// Branch and bound for best sibling.
		int siblingID = _rootID;
		while (!_nodes[siblingID].IsLeaf())
		{
			int child0ID = _nodes[siblingID].Child0ID;
			int child1ID = _nodes[siblingID].Child1ID;

			auto mergedAabb = BoundingBox();
			BoundingBox::CreateMerged(mergedAabb, _nodes[siblingID].Aabb, leaf.Aabb);

			float area = Geometry::GetBoundingBoxArea(_nodes[siblingID].Aabb);
			float mergedArea = Geometry::GetBoundingBoxArea(mergedAabb);

			// Calculate cost of creating new parent for prospective sibling and new leaf.
			float cost = mergedArea * 2;

			// Calculate minimum cost of pushing leaf further down tree.
			float inheritCost = (mergedArea - area) * 2;

			// Calculate cost of descending into child 0.
			float cost0 = INFINITY;
			if (child0ID != NO_VALUE)
			{
				const auto& child0 = _nodes[child0ID];

				auto aabb = BoundingBox();
				BoundingBox::CreateMerged(aabb, child0.Aabb, leaf.Aabb);

				if (child0.IsLeaf())
				{
					cost0 = Geometry::GetBoundingBoxArea(aabb) + inheritCost;
				}
				else
				{
					float prevArea = Geometry::GetBoundingBoxArea(child0.Aabb);
					float newArea = Geometry::GetBoundingBoxArea(aabb);
					cost0 = (newArea - prevArea) + inheritCost;
				}
			}

			// Calculate cost of descending into child 1.
			float cost1 = INFINITY;
			if (child1ID != NO_VALUE)
			{
				const auto& child1 = _nodes[child1ID];

				auto aabb = BoundingBox();
				BoundingBox::CreateMerged(aabb, child1.Aabb, leaf.Aabb);

				if (child1.IsLeaf())
				{
					cost1 = Geometry::GetBoundingBoxArea(aabb) + inheritCost;
				}
				else
				{
					float prevArea = Geometry::GetBoundingBoxArea(child1.Aabb);
					float newArea = Geometry::GetBoundingBoxArea(aabb);
					cost1 = newArea - prevArea + inheritCost;
				}
			}

			// Test if descent is worthwhile according to minimum cost.
			if (cost < cost0 && cost < cost1)
				break;

			// Descend.
			siblingID = (cost0 < cost1) ? child0ID : child1ID;
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
			return;
		}

		// Get sibling for new leaf.
		int siblingID = GetBestSiblingLeafID(leafID);
		auto& sibling = _nodes[siblingID];

		// Box is wrong?
		//auto aabb = BoundingBox();
		//BoundingBox::CreateMerged(aabb, sibling.Aabb, leaf.Aabb);

		// TODO: Data corruption?
		// Create new parent.
		int prevParentID = sibling.ParentID;
		int newParentID = GetNewNodeID();
		auto& newParent = _nodes[newParentID];

		//newParent.Aabb = aabb;
		//BoundingBox::CreateMerged(newParent.Aabb, sibling.Aabb, leaf.Aabb);
		newParent.ParentID = prevParentID;
		newParent.Child0ID = siblingID;
		newParent.Child1ID = leafID;
		sibling.ParentID = newParentID;
		leaf.ParentID = newParentID;

		// Set new root or update previous parent.
		if (newParent.ParentID == NO_VALUE)
		{
			_rootID = newParentID;
		}
		else
		{
			auto& prevParent = _nodes[prevParentID];
			if (prevParent.Child0ID == siblingID)
			{
				prevParent.Child0ID = newParentID;
			}
			else
			{
				prevParent.Child1ID = newParentID;
			}
		}

		// Refit.
		RefitNode(leafID);

		//Validate(prevParentID);
	}

	void BoundingTree::RemoveLeaf(int leafID)
	{
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

	// TODO: Blizzard guy's version is better.
	void BoundingTree::BalanceNode(int nodeID)
	{
		int parentID = _nodes[nodeID].ParentID;
		int grandparentID = _nodes[parentID].ParentID;

		if (grandparentID == NO_VALUE)
			return;

		auto& leaf = _nodes[nodeID];
		auto& parent = _nodes[parentID];
		auto& grandParent = _nodes[grandparentID];

		int rotatedSiblingID = NO_VALUE;
		if (grandParent.Child0ID != parentID && grandParent.Child1ID == NO_VALUE)
		{
			rotatedSiblingID = grandParent.Child0ID;
		}
		else if (grandParent.Child1ID != parentID && grandParent.Child0ID == NO_VALUE)
		{
			rotatedSiblingID = grandParent.Child0ID;
		}

		if (rotatedSiblingID == NO_VALUE)
			return;

		auto& rotatedSibling = _nodes[rotatedSiblingID];

		auto mergedAabb = BoundingBox();
		BoundingBox::CreateMerged(mergedAabb, rotatedSibling.Aabb, leaf.Aabb);

		// Rotation more less optimal; return early.
		if (Geometry::GetBoundingBoxArea(mergedAabb) > Geometry::GetBoundingBoxArea(parent.Aabb))
			return;

		if (parent.Child0ID == nodeID)
		{
			parent.Child0ID = NO_VALUE;
			if (parent.Child1ID != NO_VALUE)
				parent.Aabb = _nodes[parent.Child1ID].Aabb;
		}
		else
		{
			parent.Child1ID = NO_VALUE;
			if (parent.Child0ID != NO_VALUE)
				parent.Aabb = _nodes[parent.Child0ID].Aabb;
		}
		parent.Aabb = {};

		if (grandParent.Child0ID == NO_VALUE)
		{
			grandParent.Child0ID = nodeID;
		}
		else
		{
			grandParent.Child1ID = nodeID;
		}
		grandParent.Aabb = mergedAabb;
	}

	void BoundingTree::RefitNode(int nodeID)
	{
		const auto& node = _nodes[nodeID];

		// Retread tree branch to refit AABBs.
		int parentID = node.ParentID;
		while (parentID != NO_VALUE)
		{
			// TODO
			//BalanceNode(parentID);

			auto& parent = _nodes[parentID];

			if (parent.Child0ID != NO_VALUE && parent.Child1ID != NO_VALUE)
			{
				const auto& child0 = _nodes[parent.Child0ID];
				const auto& child1 = _nodes[parent.Child1ID];
				BoundingBox::CreateMerged(parent.Aabb, child0.Aabb, child1.Aabb);
			}
			else if (parent.Child0ID != NO_VALUE)
			{
				const auto& child0 = _nodes[parent.Child0ID];
				parent.Aabb = child0.Aabb;
			}
			else if (parent.Child1ID != NO_VALUE)
			{
				const auto& child1 = _nodes[parent.Child1ID];
				parent.Aabb = child1.Aabb;
			}

			int prevParentID = parentID;
			parentID = parent.ParentID;
		}
	}

	void BoundingTree::RemoveNode(int nodeID)
	{
		auto& node = _nodes[nodeID];

		// Remove leaf from map.//
		if (node.IsLeaf())
			_leafIDMap.erase(node.ObjectID);

		// Clear node and mark free.
		node = {};
		_freeNodeIds.push_back(nodeID);

		// Shrink capacity if empty to prevent memory bloat.
		if (_nodes.size() == _freeNodeIds.size())
			*this = {};
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
