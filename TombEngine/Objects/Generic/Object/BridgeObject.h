#pragma once
#include "Math/Math.h"

struct ItemInfo;

using namespace TEN::Math;

namespace TEN::Entities::Generic
{
	class BridgeObject
	{
	private:
		// Members
		CollisionMesh _collisionMesh = CollisionMesh();

	public:
		Pose PrevPose = Pose::Zero;

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
