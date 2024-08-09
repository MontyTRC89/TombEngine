#pragma once

#include "Math/Math.h"

using namespace TEN::Math;

template <>
struct std::hash<Vector3i>
{
	size_t operator() (const Vector3i& vector) const
	{
		size_t value = 0;
		value ^= std::hash<int>()(vector.x) + 0x9e3779b9 + (value << 6) + (value >> 2);
		value ^= std::hash<int>()(vector.y) + 0x9e3779b9 + (value << 6) + (value >> 2);
		value ^= std::hash<int>()(vector.z) + 0x9e3779b9 + (value << 6) + (value >> 2);
		return value;
	}
};

namespace TEN::Structures
{
	class SpatialHash
	{
	private:
		struct Cell
		{
			static constexpr auto SIZE		   = BLOCK(0.5f);
			static constexpr auto AABB_EXTENTS = Vector3(SIZE / 2);

			std::set<int> ObjectIds = {};
			BoundingBox	  Aabb		= BoundingBox();

			Cell(const Vector3& center);
		};

		// Members

		std::unordered_map<Vector3i, Cell> _cellMap = {};

	public:
		// Getters

		std::set<int> GetBoundedObjectIds() const;
		std::set<int> GetBoundedObjectIds(const Vector3& pos) const;
		std::set<int> GetBoundedObjectIds(const Ray& ray, float dist) const;
		std::set<int> GetBoundedObjectIds(const BoundingBox& aabb) const;
		std::set<int> GetBoundedObjectIds(const BoundingOrientedBox& obb) const;
		std::set<int> GetBoundedObjectIds(const BoundingSphere& sphere) const;

		// Utilities

		void Update(int objectID, const BoundingBox& aabb, const BoundingBox& prevAabb);
		void Update(int objectID, const BoundingOrientedBox& obb, BoundingOrientedBox& prevObb);
		void Update(int objectID, const BoundingSphere& sphere, const BoundingSphere& prevSphere);

		void Insert(int objectID, const BoundingBox& aabb);
		void Insert(int objectID, const BoundingOrientedBox& obb);
		void Insert(int objectID, const BoundingSphere& sphere);

		void Remove(int objectID, const BoundingBox& prevAabb);
		void Remove(int objectID, const BoundingOrientedBox& prevObb);
		void Remove(int objectID, const BoundingSphere& prevSphere);

		// Debug

		void DrawDebug() const;

	private:
		// Getter helpers

		Vector3i			  GetCellKey(const Vector3& pos) const;
		std::vector<Vector3i> GetCellKeys(const Ray& ray, float dist) const;
		std::vector<Vector3i> GetCellKeys(const BoundingBox& aabb) const;
		std::vector<Vector3i> GetCellKeys(const BoundingOrientedBox& obb) const;
		std::vector<Vector3i> GetCellKeys(const BoundingSphere& sphere) const;

		// Utility helpers

		void Insert(int objectID, const std::vector<Vector3i>& keys);
		void Remove(int objectID, const std::vector<Vector3i>& keys);
	};

	extern SpatialHash DebugSpatialHash;
}
