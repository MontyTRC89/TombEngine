#pragma once
#include "Specific/phd_global.h"
#include "Specific/trmath.h"

using std::vector;

struct CollisionInfo;
struct CollisionResult;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	constexpr int VEHICLE_VELOCITY_SCALE = 256; // TODO: Deal with this nonsense *immediately* post-beta. @Sezz 2022.06.25

	constexpr auto VEHICLE_SINK_SPEED = 15;
	constexpr auto VEHICLE_MAX_WATER_HEIGHT = CLICK(2.5f);
	constexpr auto VEHICLE_WATER_VEL_COEFFICIENT = 16.0f;
	constexpr auto VEHICLE_WATER_TURN_COEFFICIENT = 10.0f;
	constexpr auto VEHICLE_SWAMP_VEL_COEFFICIENT = 8.0f;
	constexpr auto VEHICLE_SWAMP_TURN_COEFFICIENT = 6.0f;

	enum class VehicleMountType
	{
		None,
		LevelStart,
		Front,
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
