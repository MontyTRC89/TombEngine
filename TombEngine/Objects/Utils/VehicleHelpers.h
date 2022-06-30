#pragma once
#include "Specific/phd_global.h"
#include "Specific/trmath.h"

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
	constexpr int VEHICLE_VELOCITY_SCALE = 256; // TODO: Deal with this nonsense *immediately* post-beta. @Sezz 2022.06.25

	constexpr int	VEHICLE_SINK_VELOCITY		  = 15;
	constexpr int	VEHICLE_WATER_HEIGHT_MAX	  = CLICK(2.5f);
	constexpr float VEHICLE_WATER_VELOCITY_COEFF  = 16.0f;
	constexpr float VEHICLE_WATER_TURN_RATE_COEFF = 10.0f;
	constexpr float VEHICLE_SWAMP_VELOCITY_COEFF  = 8.0f;
	constexpr float VEHICLE_SWAMP_TURN_RATE_COEFF = 6.0f;

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

	enum class VehicleImpactDirection
	{
		None,
		Front,
		Back,
		Left,
		Right
	};

	VehicleMountType GetVehicleMountType(ItemInfo* vehicleItem, ItemInfo* laraItem, CollisionInfo* coll, vector<VehicleMountType> allowedMountTypes, float maxDistance2D, float maxVerticalDistance = STEPUP_HEIGHT);
	VehicleImpactDirection GetVehicleImpactDirection(ItemInfo* vehicleItem, Vector3Int deltaPos);
	int GetVehicleHeight(ItemInfo* vehicleItem, int forward, int right, bool clamp, Vector3Int* pos);
	int GetVehicleWaterHeight(ItemInfo* vehicleItem, int forward, int right, bool clamp, Vector3Int* pos);

	void DoVehicleCollision(ItemInfo* vehicleItem, int radius);
	int	 DoVehicleDynamics(int height, int verticalVelocity, int minBounce, int maxKick, int* yPos);
	int  DoVehicleShift(ItemInfo* vehicleItem, Vector3Int* pos, Vector3Int* oldPos);
	int  DoVehicleWaterMovement(ItemInfo* vehicleItem, ItemInfo* laraItem, int currentVelocity, int radius, short* turnRate);
	void DoVehicleFlareDiscard(ItemInfo* laraItem);

	short ModulateVehicleTurnRate(short turnRate, short accelRate, short minTurnRate, short maxTurnRate, float axisCoeff, bool invert);
	void  ModulateVehicleTurnRateX(short* turnRate, short accelRate, short minTurnRate, short maxTurnRate, bool invert = true);
	void  ModulateVehicleTurnRateY(short* turnRate, short accelRate, short minTurnRate, short maxTurnRate, bool invert = false);
	void  ModulateVehicleLean(ItemInfo* vehicleItem, short baseRate, short maxAngle);
	void  ResetVehicleLean(ItemInfo* vehicleItem, float rate);
}
