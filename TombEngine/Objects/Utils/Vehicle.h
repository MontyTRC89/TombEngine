#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	enum class VehicleType
	{
		// TR2:
		Skidoo,
		Speedboat,

		// TR3:
		BigGun,
		Kayak,
		Minecart,
		QuadBike,
		RubberBoat,
		UPV,

		// TR4:
		Jeep,
		Motorbike
	};

	struct VehicleControl
	{
		Vector3 Velocity			  = Vector3::Zero;
		float	LeftVerticalVelocity  = 0.0f;
		float	RightVerticalVelocity = 0.0f;

		EulerAngles TurnRate	   = EulerAngles::Zero;
		EulerAngles ExtraRotation  = EulerAngles::Zero;
		EulerAngles MomentumOrient = EulerAngles::Zero;
	};

	struct VehicleAttributes
	{
		float Radius = 0.0f;
		float Depth = 0.0f;
		float Width = 0.0f;
		float Height = 0.0f;

		int StepHeight = 0;
		int Bounce = 0;
		int Kick = 0;
		int MountDistance = 0;
		int DismountDistance = 0;
	};

	class AbstractVehicle
	{
	public:
		// Components
		VehicleControl		 Control = {};
		VehicleAttributes	 Attribute = {};
		std::vector<Vector3> CollisionPoints = {};

		// Core functionality
		virtual void Initialize(int itemNumber) = 0;
		virtual void CollidePlayer(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll) = 0;
		virtual bool Controller(ItemInfo& playerItem, CollisionInfo& coll) = 0;
		virtual void PlayerControl() = 0;
		virtual void Animate() = 0;

		// Utilities
		void SetPlayerVehicle(ItemInfo& item, const ItemInfo* vehicle);

		// Turn rate utilities
		void ModulateTurnRateX(short accelRate, short minTurnRate, short maxTurnRate, bool invert = true);
		void ModulateTurnRateY(short accelRate, short minTurnRate, short maxTurnRate, bool invert = false);
		void ResetTurnRateX(short decelRate);
		void ResetTurnRateY(short decelRate);

	private:
		// Helpers
		short ModulateTurnRate(short turnRate, short accelRate, short minTurnRate, short maxTurnRate, float axisCoeff, bool invert);
		short ResetTurnRate(short turnRate, short decelRate);
	};
}
