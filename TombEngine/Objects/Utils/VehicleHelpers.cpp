#include "framework.h"
#include "Objects/Utils/VehicleHelpers.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Specific/input.h"

namespace TEN::Entities::Vehicles
{
	VehicleMountType GetVehicleMountType(ItemInfo* vehicleItem, ItemInfo* laraItem, CollisionInfo* coll, vector<VehicleMountType> allowedMountTypes, float minDistance)
	{
		auto* lara = GetLaraInfo(laraItem);

		// Assess ACTION input and hand status.
		if (!(TrInput & IN_ACTION) || lara->Control.HandStatus != HandStatus::Free)
			return VehicleMountType::None;

		// Assess height difference.
		if (abs(laraItem->Pose.Position.y - laraItem->Pose.Position.y) > STEPUP_HEIGHT)
			return VehicleMountType::None;

		// Assess distance to vehicle.
		float distance = Vector3::Distance(laraItem->Pose.Position.ToVector3(), vehicleItem->Pose.Position.ToVector3());
		if (distance > minDistance)
			return VehicleMountType::None;

		// Assess object collision.
		if (!TestBoundsCollide(vehicleItem, laraItem, coll->Setup.Radius) || !TestCollision(vehicleItem, laraItem))
			return VehicleMountType::None;

		// Assess mount types allowed for vehicle.
		short deltaAngle = vehicleItem->Pose.Orientation.y - laraItem->Pose.Orientation.y;
		for (auto mountType : allowedMountTypes)
		{
			// TODO: Check cardinal direction of relative position. Lara can mount from the opposite side! @Sezz
			switch (mountType)
			{
			case VehicleMountType::LevelStart:
				if (abs(deltaAngle) < ANGLE(135.0f) &&
					!laraItem->Animation.VerticalVelocity &&
					laraItem->Pose.Position == vehicleItem->Pose.Position)
				{
					break;
				}

				continue;

			case VehicleMountType::Back:
				if (abs(deltaAngle) < ANGLE(35.0f) &&
					!laraItem->Animation.Airborne)
				{
					break;
				}

				continue;

			case VehicleMountType::Left:
				if (deltaAngle > -ANGLE(135.0f) && deltaAngle < -ANGLE(45.0f) &&
					!laraItem->Animation.Airborne)
				{
					break;
				}

				continue;

			case VehicleMountType::Right:
				if (deltaAngle > ANGLE(45.0f) && deltaAngle < ANGLE(135.0f) &&
					!laraItem->Animation.Airborne)
				{
					break;
				}

				continue;

			case VehicleMountType::Jump:
				if (abs(deltaAngle) < ANGLE(135.0f) &&
					laraItem->Animation.Airborne &&
					laraItem->Animation.VerticalVelocity > 0 &&
					laraItem->Pose.Position.y > vehicleItem->Pose.Position.y &&
					lara->Control.WaterStatus == WaterStatus::Dry)
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
}
