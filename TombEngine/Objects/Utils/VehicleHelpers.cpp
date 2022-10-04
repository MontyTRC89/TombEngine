#include "framework.h"
#include "Objects/Utils/VehicleHelpers.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/effects/simple_particle.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Game/room.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/prng.h"
#include "Specific/setup.h"

using namespace TEN::Input;

namespace TEN::Entities::Vehicles
{
	// Deprecated.
	int GetVehicleHeight(ItemInfo* vehicleItem, int forward, int right, bool clamp, Vector3Int& pos)
	{
		float sinX = phd_sin(vehicleItem->Pose.Orientation.x);
		float sinY = phd_sin(vehicleItem->Pose.Orientation.y);
		float cosY = phd_cos(vehicleItem->Pose.Orientation.y);
		float sinZ = phd_sin(vehicleItem->Pose.Orientation.z);

		pos.x = vehicleItem->Pose.Position.x + (forward * sinY) + (right * cosY);
		pos.y = vehicleItem->Pose.Position.y - (forward * sinX) + (right * sinZ);
		pos.z = vehicleItem->Pose.Position.z + (forward * cosY) - (right * sinY);

		// Get collision a bit higher to be able to detect bridges.
		auto probe = GetCollision(pos.x, pos.y - CLICK(2), pos.z, vehicleItem->RoomNumber);

		if (pos.y < probe.Position.Ceiling || probe.Position.Ceiling == NO_HEIGHT)
			return NO_HEIGHT;

		if (pos.y > probe.Position.Floor && clamp)
			pos.y = probe.Position.Floor;

		return probe.Position.Floor;
	}

	VehicleMountType GetVehicleMountType(ItemInfo* vehicleItem, ItemInfo* laraItem, CollisionInfo* coll, const vector<VehicleMountType>& allowedMountTypes, float maxDistance2D, float maxVerticalDistance)
	{
		auto* lara = GetLaraInfo(laraItem);

		// Assess vehicle usability.
		if (vehicleItem->Flags & IFLAG_INVISIBLE)
			return VehicleMountType::None;

		// Assess hand status.
		if (lara->Control.HandStatus != HandStatus::Free)
			return VehicleMountType::None;

		// Assess vertical distance to vehicle.
		if (abs(laraItem->Pose.Position.y - vehicleItem->Pose.Position.y) > maxVerticalDistance)
			return VehicleMountType::None;

		// Assess 2D distance to vehicle.
		float distance2D = Vector2::Distance(
			Vector2(laraItem->Pose.Position.x, laraItem->Pose.Position.z),
			Vector2(vehicleItem->Pose.Position.x, vehicleItem->Pose.Position.z));
		if (distance2D > maxDistance2D)
			return VehicleMountType::None;

		// Assess object collision.
		if (!TestBoundsCollide(vehicleItem, laraItem, coll->Setup.Radius) || !TestCollision(vehicleItem, laraItem))
			return VehicleMountType::None;

		bool hasInputAction = TrInput & IN_ACTION;

		short deltaHeadingAngle = vehicleItem->Pose.Orientation.y - laraItem->Pose.Orientation.y;
		short angleBetweenPositions = vehicleItem->Pose.Orientation.y - GetOrientBetweenPoints(laraItem->Pose.Position, vehicleItem->Pose.Position).y;
		bool onCorrectSide = abs(deltaHeadingAngle - angleBetweenPositions) < ANGLE(45.0f);

		// Assess mount types allowed for vehicle.
		for (auto mountType : allowedMountTypes)
		{
			switch (mountType)
			{
			// TODO: Holding FORWARD the moment a level loads, this mount will fail. Need a better method.
			case VehicleMountType::LevelStart:
				if (laraItem->Animation.Velocity == Vector3::Zero &&
					laraItem->Pose.Position == vehicleItem->Pose.Position)
				{
					break;
				}
				continue;

			case VehicleMountType::Front:
				if (hasInputAction && onCorrectSide &&
					deltaHeadingAngle > ANGLE(135.0f) && deltaHeadingAngle < ANGLE(-135.0f) &&
					!laraItem->Animation.IsAirborne)
				{
					break;
				}
				continue;

			case VehicleMountType::Back:
				if (hasInputAction && onCorrectSide &&
					abs(deltaHeadingAngle) < ANGLE(45.0f) &&
					!laraItem->Animation.IsAirborne)
				{
					break;
				}
				continue;

			case VehicleMountType::Left:
				if (hasInputAction && onCorrectSide &&
					deltaHeadingAngle > ANGLE(-135.0f) && deltaHeadingAngle < ANGLE(-45.0f) &&
					!laraItem->Animation.IsAirborne)
				{
					break;
				}
				continue;

			case VehicleMountType::Right:
				if (hasInputAction && onCorrectSide &&
					deltaHeadingAngle > ANGLE(45.0f) && deltaHeadingAngle < ANGLE(135.0f) &&
					!laraItem->Animation.IsAirborne)
				{
					break;
				}
				continue;

			case VehicleMountType::Jump:
				if (laraItem->Animation.IsAirborne &&
					laraItem->Animation.Velocity.y > 0.0f &&
					laraItem->Pose.Position.y < vehicleItem->Pose.Position.y)
				{
					break;
				}
				continue;

			default:
				return VehicleMountType::None;
			}

			return mountType;
		}

		return VehicleMountType::None;
	}

	VehicleDismountType GetVehicleDismountType(ItemInfo* vehicleItem, const vector<VehicleDismountType>& allowedDismountTypes, float distance, bool onLand)
	{
		if (!(TrInput & VEHICLE_IN_BRAKE))
			return VehicleDismountType::None;
		
		// Assess dismount types allowed for vehicle.
		for (auto dismountType : allowedDismountTypes)
		{
			short headingAngle;
			switch (dismountType)
			{
			case VehicleDismountType::Front:
				if (TrInput & IN_FORWARD)
				{
					headingAngle = ANGLE(0.0f);
					break;
				}

				continue;

			case VehicleDismountType::Back:
				if (TrInput & IN_BACK)
				{
					headingAngle = ANGLE(180.0f);
					break;
				}

				continue;

			case VehicleDismountType::Left:
				if (TrInput & IN_LEFT)
				{
					headingAngle = ANGLE(-90.0f);
					break;
				}

				continue;

			case VehicleDismountType::Right:
				if (TrInput & IN_RIGHT)
				{
					headingAngle = ANGLE(90.0f);
					break;
				}

				continue;
			}

			if (TestVehicleDismount(vehicleItem, dismountType, headingAngle, distance, onLand))
				return dismountType;
		}

		return VehicleDismountType::None;
	}

	bool TestVehicleDismount(ItemInfo* vehicleItem, VehicleDismountType dismountType, short headingAngle, float distance, bool onLand)
	{
		if (dismountType == VehicleDismountType::None)
			return false;
		auto probe = GetCollision(vehicleItem, headingAngle, distance, -CLICK(2));

		// Assess for walls. Check ceiling as well?
		if (probe.Position.Floor == NO_HEIGHT)
			return false;

		// Assess for slopes (if applicable).
		if (probe.Position.FloorSlope && onLand)
			return false;

		// Assess feasibility of dismount.
		if (abs(probe.Position.Floor - vehicleItem->Pose.Position.y) <= STEPUP_HEIGHT && // Floor is within highest step threshold.
			(probe.Position.Ceiling - vehicleItem->Pose.Position.y) > -LARA_HEIGHT &&	 // Gap is optically permissive.
			abs(probe.Position.Ceiling - probe.Position.Floor) > LARA_HEIGHT)			 // Space is not a clamp.
		{
			return true;
		}

		return false;
	}

	VehicleImpactDirection GetVehicleImpactDirection(ItemInfo* vehicleItem, const Vector3Int& prevPos)
	{
		auto direction = vehicleItem->Pose.Position - prevPos;

		if (direction.x || direction.z)
		{
			float sinY = phd_sin(vehicleItem->Pose.Orientation.y);
			float cosY = phd_cos(vehicleItem->Pose.Orientation.y);

			int front = (direction.x * sinY) + (direction.z * cosY);
			int side = (direction.x * cosY) - (direction.z * sinY);

			if (abs(front) > abs(side))
			{
				if (front > 0)
					return VehicleImpactDirection::Back;
				else
					return VehicleImpactDirection::Front;
			}
			else
			{
				if (side > 0)
					return VehicleImpactDirection::Left;
				else
					return VehicleImpactDirection::Right;
			}
		}

		return VehicleImpactDirection::None;
	}

	VehiclePointCollision GetVehicleCollision(ItemInfo* vehicleItem, int forward, int right, bool clamp)
	{
		float sinX = phd_sin(vehicleItem->Pose.Orientation.x);
		float sinY = phd_sin(vehicleItem->Pose.Orientation.y);
		float cosY = phd_cos(vehicleItem->Pose.Orientation.y);
		float sinZ = phd_sin(vehicleItem->Pose.Orientation.z);

		auto point = Vector3Int(
			vehicleItem->Pose.Position.x + (forward * sinY) + (right * cosY),
			vehicleItem->Pose.Position.y - (forward * sinX) + (right * sinZ),
			vehicleItem->Pose.Position.z + (forward * cosY) - (right * sinY)
		);

		// Get collision a bit higher to be able to detect bridges.
		auto probe = GetCollision(point.x, point.y - CLICK(2), point.z, vehicleItem->RoomNumber);

		if (point.y < probe.Position.Ceiling || probe.Position.Ceiling == NO_HEIGHT)
			return VehiclePointCollision{ point, NO_HEIGHT, NO_HEIGHT};

		if (point.y > probe.Position.Floor && clamp)
			point.y = probe.Position.Floor;

		return VehiclePointCollision{ point, probe.Position.Floor, probe.Position.Ceiling };
	}
	
	int GetVehicleWaterHeight(ItemInfo* vehicleItem, int forward, int right, bool clamp, Vector3Int& pos)
	{
		auto world =
			Matrix::CreateFromYawPitchRoll(TO_RAD(vehicleItem->Pose.Orientation.y), TO_RAD(vehicleItem->Pose.Orientation.x), TO_RAD(vehicleItem->Pose.Orientation.z)) *
			Matrix::CreateTranslation(vehicleItem->Pose.Position.x, vehicleItem->Pose.Position.y, vehicleItem->Pose.Position.z);

		auto vec = Vector3(right, 0, forward);
		vec = Vector3::Transform(vec, world);

		pos = Vector3Int(vec);
		auto probe = GetCollision(pos.x, pos.y, pos.z, vehicleItem->RoomNumber);
		int probedRoomNumber = probe.RoomNumber;

		int height = GetWaterHeight(pos.x, pos.y, pos.z, probedRoomNumber);

		if (height == NO_HEIGHT)
		{
			height = probe.Position.Floor;
			if (height == NO_HEIGHT)
				return height;
		}

		height -= 5;

		if (pos.y > height && clamp)
			pos.y = height;

		return height;
	}

	void DoVehicleCollision(ItemInfo* vehicleItem, int radius)
	{
		CollisionInfo coll = {};
		coll.Setup.Radius = radius * 0.8f; // HACK: Most vehicles use radius larger than needed.
		coll.Setup.UpperCeilingBound = MAX_HEIGHT; // HACK: this needs to be set to prevent GCI result interference.
		coll.Setup.OldPosition = vehicleItem->Pose.Position;
		coll.Setup.EnableObjectPush = true;

		DoObjectCollision(vehicleItem, &coll);
	}

	// Motorbike and jeep had the values in their dynamics functions tweaked to make them feel heavier.
	// Using this unified function, they leap off hills as easily as the older vehicles for now. @Sezz 2022.06.30
	float DoVehicleDynamics(int height, float verticalVelocity, int minBounce, int maxKick, int* yPos, float weightMult)
	{
		// Grounded.
		if (height > *yPos)
		{
			*yPos += verticalVelocity;
			if (*yPos > (height - minBounce))
			{
				*yPos = height;
				verticalVelocity = 0.0f;
			}
			else
				verticalVelocity += GRAVITY * weightMult;
		}
		// Airborne.
		else
		{
			int kick = (height - *yPos) * 4;
			if (kick < maxKick)
				kick = maxKick;

			verticalVelocity += (kick - verticalVelocity) / 8;

			if (*yPos > height)
				*yPos = height;
		}

		return verticalVelocity;
	}

	// Temp. scaffolding function. Shifts need a rework.
	void CalculateVehicleShift(ItemInfo* vehicleItem, short* extraRot, const VehiclePointCollision& prevPoint, int height, int front, int side, int step, bool clamp)
	{
		auto point = GetVehicleCollision(vehicleItem, front, side, clamp);

		if (point.Floor < (prevPoint.Position.y - step) ||	   // Floor is beyond upper bound.
			//point.Ceiling > (prevPoint.Position.y - height) || // Ceiling is too low. Will get stuck on jumps that hit the ceiling.
			abs(point.Ceiling - point.Floor) <= height)		   // Floor and ceiling form a clamp.
		{
			*extraRot += DoVehicleShift(vehicleItem, point.Position, prevPoint.Position);
		}
	}

	short DoVehicleShift(ItemInfo* vehicleItem, const Vector3Int& pos, const Vector3Int& prevPos)
	{
		auto alignedPos = pos / SECTOR(1);
		auto alignedPrevPos = prevPos / SECTOR(1);
		auto alignedShift = Vector3Int(
			pos.x & WALL_MASK,
			0,
			pos.z & WALL_MASK
		);

		if (alignedPos.x == alignedPrevPos.x)
		{
			if (alignedPos.z == alignedPrevPos.z)
			{
				vehicleItem->Pose.Position.x += (prevPos.x - pos.x);
				vehicleItem->Pose.Position.z += (prevPos.z - pos.z);
			}
			else if (alignedPos.z > alignedPrevPos.z)
			{
				vehicleItem->Pose.Position.z -= alignedShift.z + 1;
				return (pos.x - vehicleItem->Pose.Position.x);
			}
			else
			{
				vehicleItem->Pose.Position.z += SECTOR(1) - alignedShift.z;
				return (vehicleItem->Pose.Position.x - pos.x);
			}
		}
		else if (alignedPos.z == alignedPrevPos.z)
		{
			if (alignedPos.x > alignedPrevPos.x)
			{
				vehicleItem->Pose.Position.x -= alignedShift.x + 1;
				return (vehicleItem->Pose.Position.z - pos.z);
			}
			else
			{
				vehicleItem->Pose.Position.x += SECTOR(1) - alignedShift.x;
				return (pos.z - vehicleItem->Pose.Position.z);
			}
		}
		else
		{
			alignedPos = Vector3Int::Zero;

			auto probe = GetCollision(prevPos.x, pos.y, pos.z, vehicleItem->RoomNumber);
			if (probe.Position.Floor < (prevPos.y - CLICK(1)))
			{
				if (pos.z > prevPos.z)
					alignedPos.z = -alignedShift.z - 1;
				else
					alignedPos.z = SECTOR(1) - alignedShift.z;
			}

			probe = GetCollision(pos.x, pos.y, prevPos.z, vehicleItem->RoomNumber);
			if (probe.Position.Floor < (prevPos.y - CLICK(1)))
			{
				if (pos.x > prevPos.x)
					alignedPos.x = -alignedShift.x - 1;
				else
					alignedPos.x = SECTOR(1) - alignedShift.x;
			}

			// NOTE: Commented lines are skidoo-specific. Likely unnecessary but keeping for reference. @Sezz 2022.06.30
			if (alignedPos.x && alignedPos.z)
			{
				vehicleItem->Pose.Position.z += alignedPos.z;
				vehicleItem->Pose.Position.x += alignedPos.x;
				//vehicleItem->Animation.Velocity -= 50.0f;
			}
			else if (alignedPos.z)
			{
				vehicleItem->Pose.Position.z += alignedPos.z;
				//vehicleItem->Animation.Velocity -= 50.0f;

				if (alignedPos.z > 0)
					return (vehicleItem->Pose.Position.x - pos.x);
				else
					return (pos.x - vehicleItem->Pose.Position.x);
			}
			else if (alignedPos.x)
			{
				vehicleItem->Pose.Position.x += alignedPos.x;
				//vehicleItem->Animation.Velocity -= 50.0f;

				if (alignedPos.x > 0)
					return (pos.z - vehicleItem->Pose.Position.z);
				else
					return (vehicleItem->Pose.Position.z - pos.z);
			}
			else
			{
				vehicleItem->Pose.Position.z += prevPos.z - pos.z;
				vehicleItem->Pose.Position.x += prevPos.x - pos.x;
				//vehicleItem->Animation.Velocity -= 50.0f;
			}
		}

		return 0;
	}

	float DoVehicleWaterMovement(ItemInfo* vehicleItem, ItemInfo* laraItem, float currentVelocity, int radius, short* turnRate)
	{
		if (TestEnvironment(ENV_FLAG_WATER, vehicleItem) ||
			TestEnvironment(ENV_FLAG_SWAMP, vehicleItem))
		{
			float waterDepth = (float)GetWaterDepth(vehicleItem);
			float waterHeight = vehicleItem->Pose.Position.y - GetWaterHeight(vehicleItem);

			// HACK: Sometimes quadbike test position may end up under non-portal ceiling block.
			// GetWaterDepth returns DEEP_WATER constant in that case, which is too large for our needs.
			if (waterDepth == DEEP_WATER)
				waterDepth = VEHICLE_WATER_HEIGHT_MAX;

			if (waterDepth <= VEHICLE_WATER_HEIGHT_MAX)
			{
				bool isWater = TestEnvironment(ENV_FLAG_WATER, vehicleItem);

				if (currentVelocity != 0.0f)
				{
					float coeff = isWater ? VEHICLE_WATER_VELOCITY_COEFF : VEHICLE_SWAMP_VELOCITY_COEFF;
					currentVelocity -= std::copysign(currentVelocity * ((waterDepth / VEHICLE_WATER_HEIGHT_MAX) / coeff), currentVelocity);

					if (TEN::Math::Random::GenerateInt(0, 32) > 28)
						SoundEffect(SFX_TR4_LARA_WADE, &PHD_3DPOS(vehicleItem->Pose.Position), SoundEnvironment::Land, isWater ? 0.8f : 0.7f);

					if (isWater)
						TEN::Effects::TriggerSpeedboatFoam(vehicleItem, Vector3(0, -waterDepth / 2.0f, -radius));
				}

				if (*turnRate)
				{
					float coeff = isWater ? VEHICLE_WATER_TURN_RATE_COEFF : VEHICLE_SWAMP_TURN_RATE_COEFF;
					*turnRate -= *turnRate * ((waterDepth / VEHICLE_WATER_HEIGHT_MAX) / coeff);
				}
			}
			else
			{
				if (waterDepth > VEHICLE_WATER_HEIGHT_MAX && waterHeight > VEHICLE_WATER_HEIGHT_MAX)
					ExplodeVehicle(laraItem, vehicleItem);
				else if (TEN::Math::Random::GenerateInt(0, 32) > 25)
					Splash(vehicleItem);
			}
		}

		return currentVelocity;
	}

	// TODO: Allow flares on every vehicle. Boats have this already.
	void DoVehicleFlareDiscard(ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);

		if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
		{
			CreateFlare(laraItem, ID_FLARE_ITEM, 0);
			UndrawFlareMeshes(laraItem);
			lara->Control.Weapon.GunType = LaraWeaponType::None;
			lara->Control.Weapon.RequestGunType = LaraWeaponType::None;
			lara->Flare.ControlLeft = false;
		}
	}

	// TODO: Vehicle turn rates must be affected by speed for more tactile modulation. Slower speeds slow the turn rate.
	short ModulateVehicleTurnRate(short turnRate, short accelRate, short minTurnRate, short maxTurnRate, float axisCoeff, bool invert)
	{
		axisCoeff *= invert ? -1 : 1;
		int sign = std::copysign(1, axisCoeff);

		short minTurnRateNormalized = minTurnRate * abs(axisCoeff);
		short maxTurnRateNormalized = maxTurnRate * abs(axisCoeff);

		short newTurnRate = (turnRate + (accelRate * sign)) * sign;
		newTurnRate = std::clamp(newTurnRate, minTurnRateNormalized, maxTurnRateNormalized);
		return (newTurnRate * sign);
	}

	void ModulateVehicleTurnRateX(short* turnRate, short accelRate, short minTurnRate, short maxTurnRate, bool invert)
	{
		*turnRate = ModulateVehicleTurnRate(*turnRate, accelRate, minTurnRate, maxTurnRate, AxisMap[InputAxis::MoveVertical], invert);
	}

	void ModulateVehicleTurnRateY(short* turnRate, short accelRate, short minTurnRate, short maxTurnRate, bool invert)
	{
		*turnRate = ModulateVehicleTurnRate(*turnRate, accelRate, minTurnRate, maxTurnRate, AxisMap[InputAxis::MoveHorizontal], invert);
	}

	short ResetVehicleTurnRate(short turnRate, short decelRate)
	{
		int sign = std::copysign(1, turnRate);

		if (abs(turnRate) > decelRate)
			return (turnRate - (decelRate * sign));
		else
			return 0;
	}

	void ResetVehicleTurnRateX(short* turnRate, short decelRate)
	{
		*turnRate = ResetVehicleTurnRate(*turnRate, decelRate);
	}

	void ResetVehicleTurnRateY(short* turnRate, short decelRate)
	{
		*turnRate = ResetVehicleTurnRate(*turnRate, decelRate);
	}
	
	void ModulateVehicleLean(ItemInfo* vehicleItem, short baseRate, short maxAngle)
	{
		float axisCoeff = AxisMap[InputAxis::MoveHorizontal];
		int sign = copysign(1, axisCoeff);
		short maxAngleNormalized = maxAngle * axisCoeff;
		vehicleItem->Pose.Orientation.z += std::min<short>(baseRate, abs(maxAngleNormalized - vehicleItem->Pose.Orientation.z) / 3) * sign;
	}

	void ResetVehicleLean(ItemInfo* vehicleItem, float rate)
	{
		if (abs(vehicleItem->Pose.Orientation.z) > ANGLE(0.1f))
			vehicleItem->Pose.Orientation.z += vehicleItem->Pose.Orientation.z / -rate;
		else
			vehicleItem->Pose.Orientation.z = 0;
	}
}
