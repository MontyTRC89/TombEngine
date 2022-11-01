#pragma once
#include "Math/Math.h"
#include "Math/Math.h"

struct CollisionInfo;
struct CollisionResult;
struct ItemInfo;

using std::vector;

//////////
//////////
// TODO: SAVEGAMES!
//////////
//////////
//////////

namespace TEN::Entities::Vehicles
{
	constexpr auto VEHICLE_VELOCITY_SCALE = 256; // TODO: Deal with this nonsense *immediately* post-beta. -- Sezz 2022.06.25

	constexpr auto VEHICLE_SINK_VELOCITY		 = 15.0f;
	constexpr auto VEHICLE_WATER_HEIGHT_MAX		 = CLICK(2.5f);
	constexpr auto VEHICLE_WATER_VELOCITY_COEFF  = 16.0f;
	constexpr auto VEHICLE_WATER_TURN_RATE_COEFF = 10.0f;
	constexpr auto VEHICLE_SWAMP_VELOCITY_COEFF  = 8.0f;
	constexpr auto VEHICLE_SWAMP_TURN_RATE_COEFF = 6.0f;

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

	enum class VehicleDismountType
	{
		None,
		Front,
		Back,
		Left,
		Right,
		Jump,
		Fall,
		Death
	};

	enum class VehicleImpactDirection
	{
		None,
		Front,
		Back,
		Left,
		Right
	};

	// Collision struct specific to vehicles. Used to determine point collision parameters
	// at wheels and around the base perimeter of a vehicle. May revise.
	struct VehiclePointCollision
	{
		Vector3i Position	   = Vector3i::Zero;
		int		 FloorHeight   = 0;
		int		 CeilingHeight = 0;
	};

	//-------------------

	int GetVehicleHeight(ItemInfo* vehicleItem, int forward, int right, bool clamp, Vector3i& pos);

	//-------------------
	
	VehicleMountType GetVehicleMountType(ItemInfo* vehicleItem, ItemInfo* laraItem, CollisionInfo* coll, const vector<VehicleMountType>& allowedMountTypes, float maxDistance2D, float maxVerticalDistance = STEPUP_HEIGHT);
	VehicleDismountType GetVehicleDismountType(ItemInfo* vehicleItem, const vector<VehicleDismountType>& allowedDismountTypes, float distance, bool onLand = true);
	bool TestVehicleDismount(ItemInfo* vehicleItem, VehicleDismountType dismountType, short headingAngle, float distance, bool onLand);
	VehicleImpactDirection GetVehicleImpactDirection(ItemInfo* vehicleItem, const Vector3i& prevPos);
	
	VehiclePointCollision GetVehicleCollision(ItemInfo* vehicleItem, int forward, int right, bool clamp);
	int GetVehicleWaterHeight(ItemInfo* vehicleItem, int forward, int right, bool clamp, Vector3i& pos);

	void  DoVehicleCollision(ItemInfo* vehicleItem, int radius);
	float DoVehicleDynamics(int height, float verticalVelocity, int minBounce, int maxKick, int& outYPos, float weightMult = 1.0f);
	void  CalculateVehicleShift(ItemInfo* vehcleItem, short& outExtraRot, const VehiclePointCollision& prevPoint, int height, int front, int side, int step, bool clamp);
	short DoVehicleShift(ItemInfo* vehicleItem, const Vector3i& pos, const Vector3i& prevPos);
	float DoVehicleWaterMovement(ItemInfo* vehicleItem, ItemInfo* laraItem, float currentVelocity, int radius, short& outTurnRate);
	void  DoVehicleFlareDiscard(ItemInfo* laraItem);

	short ModulateVehicleTurnRate(short turnRate, short accelRate, short minTurnRate, short maxTurnRate, float axisCoeff, bool invert);
	void  ModulateVehicleTurnRateX(short& turnRate, short accelRate, short minTurnRate, short maxTurnRate, bool invert = true);
	void  ModulateVehicleTurnRateY(short& turnRate, short accelRate, short minTurnRate, short maxTurnRate, bool invert = false);
	short ResetVehicleTurnRate(short turnRate, short decelRate);
	void  ResetVehicleTurnRateX(short& turnRate, short decelRate);
	void  ResetVehicleTurnRateY(short& turnRate, short decelRate);
	void  ModulateVehicleLean(ItemInfo* vehicleItem, short baseRate, short maxAngle);
	void  ResetVehicleLean(ItemInfo* vehicleItem, float alpha = 0.5f);
}
