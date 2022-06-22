#pragma once

using std::vector;

struct CollisionInfo;
struct CollisionResult;
struct ItemInfo;
struct Vector3Int;

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
	int  GetVehicleHeight(ItemInfo* vehicleItem, int forward, int right, bool clamp, Vector3Int* pos);
	int  GetVehicleWaterHeight(ItemInfo* vehicleItem, int forward, int right, bool clamp, Vector3Int* pos);

	void DoVehicleCollision(ItemInfo* vehicleItem, int radius);
	int  DoVehicleWaterMovement(ItemInfo* vehicleItem, ItemInfo* laraItem, int currentVelocity, int radius, short* angle);
}
