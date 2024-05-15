#pragma once
#include "Math/Math.h"

class Vector3i;
struct ItemInfo;

using namespace TEN::Math;

namespace TEN::Entities::Generic
{
	// TODO: Bridge properties.
	class BridgeObject
	{
	private:
		// Members
		CollisionMesh _collisionMesh = CollisionMesh();

	public:
		// Routines
		std::function<std::optional<int>(const ItemInfo& item, const Vector3i& pos)> GetFloorHeight	  = nullptr;
		std::function<std::optional<int>(const ItemInfo& item, const Vector3i& pos)> GetCeilingHeight = nullptr;
		std::function<int(const ItemInfo& item)> GetFloorBorder	  = nullptr;
		std::function<int(const ItemInfo& item)> GetCeilingBorder = nullptr;

		// Getters
		const CollisionMesh& GetCollisionMesh() const;

		// Utilities
		void Initialize(const ItemInfo& item);
		void Update(const ItemInfo& item);

	private:
		// Helpers
		void UpdateCollisionMesh(const ItemInfo& item);
	};

	const BridgeObject& GetBridgeObject(const ItemInfo& item);
	BridgeObject&		GetBridgeObject(ItemInfo& item);
}
