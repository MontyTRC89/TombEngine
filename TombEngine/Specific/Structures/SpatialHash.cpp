#include "framework.h"
#include "Specific/Structures/SpatialHash.h"

#include "Math/Math.h"
#include "Renderer/Renderer.h"

using namespace TEN::Math;

namespace TEN::Structures
{
	SpatialHash DebugSpatialHash;

	SpatialHash::Cell::Cell(const Vector3& center)
	{
		Aabb = BoundingBox(center, AABB_EXTENTS);
	}

	std::set<int> SpatialHash::GetBoundedObjectIds() const
	{
		auto objectIds = std::set<int>{};

		// Collect object IDs from all cells.
		for (const auto& [key, cell] : _cellMap)
			objectIds.insert(cell.ObjectIds.begin(), cell.ObjectIds.end());

		return objectIds;
	}

	std::set<int> SpatialHash::GetBoundedObjectIds(const Vector3& pos) const
	{
		// No cells; return early.
		if (_cellMap.empty())
			return {};

		// Get position's cell key.
		auto key = GetCellKey(pos);

		// Check if cell exists.
		auto it = _cellMap.find(key);
		if (it == _cellMap.end())
			return {};

		// Return object IDs from cell.
		const auto& cell = it->second;
		return cell.ObjectIds;
	}

	std::set<int> SpatialHash::GetBoundedObjectIds(const Ray& ray, float dist) const
	{
		auto objectIds = std::set<int>{};

		// No cells; return early.
		if (_cellMap.empty())
			return objectIds;

		// Collect object IDs from cells intersecting ray.
		auto keys = GetCellKeys(ray, dist);
		for (const auto& key : keys)
		{
			// Check if cell exists.
			auto it = _cellMap.find(key);
			if (it == _cellMap.end())
				continue;

			// Collect object IDs from cell.
			const auto& cell = it->second;
			objectIds.insert(cell.ObjectIds.begin(), cell.ObjectIds.end());
		}

		return objectIds;
	}

	std::set<int> SpatialHash::GetBoundedObjectIds(const BoundingBox& aabb) const
	{
		auto objectIds = std::set<int>{};

		// No cells; return early.
		if (_cellMap.empty())
			return objectIds;

		// Collect object IDs from cells intersecting AABB.
		auto keys = GetCellKeys(aabb);
		for (const auto& key : keys)
		{
			// Check if cell exists.
			auto it = _cellMap.find(key);
			if (it == _cellMap.end())
				continue;

			// Collect object IDs from cell.
			const auto& cell = it->second;
			objectIds.insert(cell.ObjectIds.begin(), cell.ObjectIds.end());
		}

		return objectIds;
	}

	std::set<int> SpatialHash::GetBoundedObjectIds(const BoundingOrientedBox& obb) const
	{
		auto objectIds = std::set<int>{};

		// No cells; return early.
		if (_cellMap.empty())
			return objectIds;

		// Collect object IDs from cells intersecting OBB.
		auto keys = GetCellKeys(obb);
		for (const auto& key : keys)
		{
			// Check if cell exists.
			auto it = _cellMap.find(key);
			if (it == _cellMap.end())
				continue;

			// Collect object IDs from cell.
			const auto& cell = it->second;
			objectIds.insert(cell.ObjectIds.begin(), cell.ObjectIds.end());
		}

		return objectIds;
	}

	std::set<int> SpatialHash::GetBoundedObjectIds(const BoundingSphere& sphere) const
	{
		auto objectIds = std::set<int>{};

		// No cells; return early.
		if (_cellMap.empty())
			return objectIds;

		// Collect object IDs from cells intersecting sphere.
		auto keys = GetCellKeys(sphere);
		for (const auto& key : keys)
		{
			// Check if cell exists.
			auto it = _cellMap.find(key);
			if (it == _cellMap.end())
				continue;

			// Collect object IDs from cell.
			const auto& cell = it->second;
			objectIds.insert(cell.ObjectIds.begin(), cell.ObjectIds.end());
		}

		return objectIds;
	}

	void SpatialHash::Update(int objectID, const BoundingBox& aabb, const BoundingBox& prevAabb)
	{
		Remove(objectID, prevAabb);
		Insert(objectID, aabb);
	}

	void SpatialHash::Update(int objectID, const BoundingOrientedBox& obb, BoundingOrientedBox& prevObb)
	{
		Remove(objectID, prevObb);
		Insert(objectID, obb);
	}

	void SpatialHash::Update(int objectID, const BoundingSphere& sphere, const BoundingSphere& prevSphere)
	{
		Remove(objectID, prevSphere);
		Insert(objectID, sphere);
	}

	void SpatialHash::Insert(int objectID, const BoundingBox& aabb)
	{
		// Insert object ID into cells intersecting AABB.
		auto keys = GetCellKeys(aabb);
		Insert(objectID, keys);
	}

	void SpatialHash::Insert(int objectID, const BoundingOrientedBox& obb)
	{
		// Insert object ID into cells intersecting OBB.
		auto keys = GetCellKeys(obb);
		Insert(objectID, keys);
	}

	void SpatialHash::Insert(int objectID, const BoundingSphere& sphere)
	{
		// Insert object ID into cells intersecting sphere.
		auto keys = GetCellKeys(sphere);
		Insert(objectID, keys);
	}

	void SpatialHash::Remove(int objectID, const BoundingBox& prevAabb)
	{
		// Remove object ID from cells intersecting previous AABB.
		auto keys = GetCellKeys(prevAabb);
		Remove(objectID, keys);
	}

	void SpatialHash::Remove(int objectID, const BoundingOrientedBox& prevObb)
	{
		// Remove object ID from cells intersecting previous OBB.
		auto keys = GetCellKeys(prevObb);
		Remove(objectID, keys);
	}

	void SpatialHash::Remove(int objectID, const BoundingSphere& prevSphere)
	{
		// Remove object ID from cells intersecting previous sphere.
		auto keys = GetCellKeys(prevSphere);
		Remove(objectID, keys);
	}

	void SpatialHash::DrawDebug() const
	{
		constexpr auto BOX_COLOR = Color(1.0f, 1.0f, 1.0f, 0.5f);

		PrintDebugMessage("SPATIAL HASH DEBUG");

		PrintDebugMessage("Cells: %d", _cellMap.size());
		for (const auto& [key, cell] : _cellMap)
			DrawDebugBox(cell.Aabb, BOX_COLOR);
	}

	Vector3i SpatialHash::GetCellKey(const Vector3& pos) const
	{
		constexpr auto OFFSET = Cell::SIZE / 2;

		// Calculate and return key.
		return Vector3i(
			RoundToStep(pos.x, Cell::SIZE) + OFFSET,
			RoundToStep(pos.y, Cell::SIZE) + OFFSET,
			RoundToStep(pos.z, Cell::SIZE) + OFFSET);
	}

	std::vector<Vector3i> SpatialHash::GetCellKeys(const Ray& ray, float dist) const
	{
		// Reserve minimum key vector.
		auto keys = std::vector<Vector3i>{};
		keys.reserve(int(dist / Cell::SIZE) + 1);

		// Calculate cell position.
		auto pos = Vector3(
			floor(ray.position.x / Cell::SIZE) * Cell::SIZE,
			floor(ray.position.y / Cell::SIZE) * Cell::SIZE,
			floor(ray.position.z / Cell::SIZE) * Cell::SIZE);

		// Calculate cell position step.
		auto posStep = Vector3(
			(ray.direction.x > 0) ? Cell::SIZE : -Cell::SIZE,
			(ray.direction.y > 0) ? Cell::SIZE : -Cell::SIZE,
			(ray.direction.z > 0) ? Cell::SIZE : -Cell::SIZE);

		// Calculate next intersection.
		auto nextIntersect = Vector3(
			((pos.x + ((posStep.x > 0) ? Cell::SIZE : 0)) - ray.position.x) / ray.direction.x,
			((pos.y + ((posStep.y > 0) ? Cell::SIZE : 0)) - ray.position.y) / ray.direction.y,
			((pos.z + ((posStep.z > 0) ? Cell::SIZE : 0)) - ray.position.z) / ray.direction.z);

		// Calculate ray step.
		auto rayStep = Vector3(
			Cell::SIZE / abs(ray.direction.x),
			Cell::SIZE / abs(ray.direction.y),
			Cell::SIZE / abs(ray.direction.z));

		// Traverse cells and collect keys.
		float currentDist = 0.0f;
		while (currentDist <= dist)
		{
			auto key = GetCellKey(pos);
			keys.push_back(key);

			// Determine which axis to step along.
			if (nextIntersect.x < nextIntersect.y)
			{
				if (nextIntersect.x < nextIntersect.z)
				{
					pos.x += posStep.x;
					currentDist = nextIntersect.x;
					nextIntersect.x += rayStep.x;
				}
				else
				{
					pos.z += posStep.z;
					currentDist = nextIntersect.z;
					nextIntersect.z += rayStep.z;
				}
			}
			else
			{
				if (nextIntersect.y < nextIntersect.z)
				{
					pos.y += posStep.y;
					currentDist = nextIntersect.y;
					nextIntersect.y += rayStep.y;
				}
				else
				{
					pos.z += posStep.z;
					currentDist = nextIntersect.z;
					nextIntersect.z += rayStep.z;
				}
			}
		}

		return keys;
	}

	std::vector<Vector3i> SpatialHash::GetCellKeys(const BoundingBox& aabb) const
	{
		// Calculate cell bounds.
		auto minCell = Vector3(
			FloorToStep(aabb.Center.x - aabb.Extents.x, Cell::SIZE),
			FloorToStep(aabb.Center.y - aabb.Extents.y, Cell::SIZE),
			FloorToStep(aabb.Center.z - aabb.Extents.z, Cell::SIZE));
		auto maxCell = Vector3(
			FloorToStep(aabb.Center.x + aabb.Extents.x, Cell::SIZE),
			FloorToStep(aabb.Center.y + aabb.Extents.y, Cell::SIZE),
			FloorToStep(aabb.Center.z + aabb.Extents.z, Cell::SIZE));

		// Reserve key vector.
		auto keys = std::vector<Vector3i>{};
		keys.reserve(
			(((maxCell.x - minCell.x) / Cell::SIZE) + 1) *
			(((maxCell.y - minCell.y) / Cell::SIZE) + 1) *
			(((maxCell.z - minCell.z) / Cell::SIZE) + 1));

		// Collect keys of cells intersecting AABB.
		for (float x = minCell.x; x <= maxCell.x; x += Cell::SIZE)
		{
			for (float y = minCell.y; y <= maxCell.y; y += Cell::SIZE)
			{
				for (float z = minCell.z; z <= maxCell.z; z += Cell::SIZE)
				{
					auto pos = Vector3(x, y, z);
					keys.push_back(GetCellKey(pos));
				}
			}
		}

		return keys;
	}

	std::vector<Vector3i> SpatialHash::GetCellKeys(const BoundingOrientedBox& obb) const
	{
		auto aabb = Geometry::GetBoundingBox(obb);

		// Collect keys of cells intersecting OBB.
		auto keys = GetCellKeys(aabb);
		for (auto it = keys.begin(); it != keys.end();)
		{
			const auto& key = *it;

			// Remove keys of cells not intersecting OBB.
			auto cellAabb = BoundingBox(key.ToVector3(), Cell::AABB_EXTENTS);
			if (!obb.Intersects(cellAabb))
			{
				it = keys.erase(it);
				continue;
			}

			it++;
		}

		return keys;
	}

	std::vector<Vector3i> SpatialHash::GetCellKeys(const BoundingSphere& sphere) const
	{
		auto aabb = BoundingBox(sphere.Center, Vector3(sphere.Radius));

		// Collect keys of cells intersecting sphere.
		auto keys = GetCellKeys(aabb);
		for (auto it = keys.begin(); it != keys.end();)
		{
			const auto& key = *it;

			// Remove keys of cells not intersecting OBB.
			auto cellAabb = BoundingBox(key.ToVector3(), Cell::AABB_EXTENTS);
			if (!sphere.Intersects(cellAabb))
			{
				it = keys.erase(it);
				continue;
			}

			it++;
		}

		return keys;
	}

	void SpatialHash::Insert(int objectID, const std::vector<Vector3i>& keys)
	{
		// Insert object ID into cells at keys.
		for (auto& key : keys)
		{
			// Get existing cell or insert new cell.
			auto [it, isInserted] = _cellMap.try_emplace(key, Cell(key.ToVector3()));

			// Insert object ID into cell.
			auto& cell = it->second;
			cell.ObjectIds.insert(objectID);
		}
	}

	void SpatialHash::Remove(int objectID, const std::vector<Vector3i>& keys)
	{
		// Remove object ID from cells at keys.
		for (const auto& key : keys)
		{
			// Check if cell exists.
			auto it = _cellMap.find(key);
			if (it == _cellMap.end())
				continue;

			// Remove object ID from cell.
			auto& cell = it->second;
			cell.ObjectIds.erase(objectID);

			// Remove cell if empty.
			if (cell.ObjectIds.empty())
				_cellMap.erase(it);
		}
	}
}
