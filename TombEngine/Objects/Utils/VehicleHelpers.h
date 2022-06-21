#pragma once

using std::vector;

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Vehicles
{
	enum class VehicleMountType
	{
		None,
		LevelStart,
		Back,
		Left,
		Right,
		Jump
	};

	VehicleMountType GetVehicleMountType(ItemInfo* vehicleItem, ItemInfo* laraItem, CollisionInfo* coll, vector<VehicleMountType> allowedMountTypes, float minDistance);
}
