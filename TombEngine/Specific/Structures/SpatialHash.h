#pragma once

#include "Math/Math.h"

using namespace TEN::Math;

namespace TEN::Structures
{
	class SpatialHash
	{
	private:
		struct Cell
		{
			std::set<int> ObjectIds = {};
			BoundingBox	  Aabb		= BoundingBox();

			Cell(const BoundingBox& aabb);
		};

		// Members

		std::unordered_map<Vector3i, Cell> _cellMap = {};

		float	_cellSize		 = 0.0f;
		Vector3 _cellAabbExtents = Vector3::Zero;

	public:
		// Constructors
		
		SpatialHash(float cellSize);

		// Getters

		std::set<int> GetBoundedObjectIds() const;
		std::set<int> GetBoundedObjectIds(const Vector3& pos) const;
		std::set<int> GetBoundedObjectIds(const Ray& ray, float dist) const;
		std::set<int> GetBoundedObjectIds(const BoundingBox& aabb) const;
		std::set<int> GetBoundedObjectIds(const BoundingOrientedBox& obb) const;
		std::set<int> GetBoundedObjectIds(const BoundingSphere& sphere) const;

		// Utilities

		unsigned int Size() const;
		bool		 Empty() const;

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
}
