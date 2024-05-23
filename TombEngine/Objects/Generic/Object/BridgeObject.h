#pragma once
#include "Math/Math.h"
#include "Physics/Physics.h"

struct ItemInfo;

using namespace TEN::Math;
using namespace TEN::Physics;

namespace TEN::Entities::Generic
{
	class BridgeObject
	{
	private:
		// Members
		BoundingBox	  _box			 = BoundingBox();
		CollisionMesh _collisionMesh = CollisionMesh();

	public:
		Pose PrevPose = Pose::Zero;

		// Routines
		std::function<std::optional<int>(const ItemInfo& item, const Vector3i& pos)> GetFloorHeight	  = nullptr;
		std::function<std::optional<int>(const ItemInfo& item, const Vector3i& pos)> GetCeilingHeight = nullptr;
		std::function<int(const ItemInfo& item)> GetFloorBorder	  = nullptr;
		std::function<int(const ItemInfo& item)> GetCeilingBorder = nullptr;

		// Getters
		const BoundingBox&	 GetBox() const;
		const CollisionMesh& GetCollisionMesh() const;

		// Utilities
		void Initialize(const ItemInfo& item);
		void Update(const ItemInfo& item);

	private:
		// Helpers
		void UpdateCollisionMesh(const ItemInfo& item);
		void UpdateBox(const ItemInfo& item);
	};

	const BridgeObject& GetBridgeObject(const ItemInfo& item);
	BridgeObject&		GetBridgeObject(ItemInfo& item);
}
