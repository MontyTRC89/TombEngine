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
		//Attractor	  _attractor	 = Attractor();

		Pose _prevPose		 = Pose::Zero;
		int	 _prevRoomNumber = 0;

	public:
		// Routines
		std::function<std::optional<int>(const ItemInfo& item, const Vector3i& pos)> GetFloorHeight	  = nullptr;
		std::function<std::optional<int>(const ItemInfo& item, const Vector3i& pos)> GetCeilingHeight = nullptr;
		std::function<int(const ItemInfo& item)> GetFloorBorder	  = nullptr;
		std::function<int(const ItemInfo& item)> GetCeilingBorder = nullptr;

		// Getters
		const BoundingBox&	 GetBox() const;
		const CollisionMesh& GetCollisionMesh() const;
		//const Attractor&	 GetAttractor() const;

		// Utilities
		void Initialize(const ItemInfo& item);
		void Update(const ItemInfo& item);
		void DeassignSectors(const ItemInfo& item) const;

	private:
		// Helpers
		void InitializeAttractor(const ItemInfo& item);
		void UpdateBox(const ItemInfo& item);
		void UpdateCollisionMesh(const ItemInfo& item);
		void UpdateAttractor(const ItemInfo& item);
		void AssignSectors(const ItemInfo& item);
	};

	const BridgeObject& GetBridgeObject(const ItemInfo& item);
	BridgeObject&		GetBridgeObject(ItemInfo& item);
}
