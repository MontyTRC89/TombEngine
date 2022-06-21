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

		short deltaHeadingAngle = vehicleItem->Pose.Orientation.y - laraItem->Pose.Orientation.y;
		short angleBetweenPositions = GetOrientBetweenPoints(laraItem->Pose.Position, vehicleItem->Pose.Position).y;
		bool onCorrectSide = abs(deltaHeadingAngle - angleBetweenPositions) < abs(deltaHeadingAngle);

		// Assess mount types allowed for vehicle.
		for (auto mountType : allowedMountTypes)
		{
			switch (mountType)
			{
			case VehicleMountType::LevelStart:
				if (abs(deltaHeadingAngle) < ANGLE(135.0f) &&
					!laraItem->Animation.VerticalVelocity &&
					laraItem->Pose.Position == vehicleItem->Pose.Position)
				{
					break;
				}

				continue;

			case VehicleMountType::Back:
				if (abs(deltaHeadingAngle) <= ANGLE(35.0f) &&
					onCorrectSide &&
					!laraItem->Animation.Airborne)
				{
					break;
				}

				continue;

			case VehicleMountType::Left:
				if (deltaHeadingAngle > -ANGLE(135.0f) && deltaHeadingAngle < -ANGLE(45.0f) &&
					onCorrectSide &&
					!laraItem->Animation.Airborne)
				{
					break;
				}

				continue;

			case VehicleMountType::Right:
				if (deltaHeadingAngle > ANGLE(45.0f) && deltaHeadingAngle < ANGLE(135.0f) &&
					onCorrectSide &&
					!laraItem->Animation.Airborne)
				{
					break;
				}

				continue;

			case VehicleMountType::Jump:
				if (abs(deltaHeadingAngle) < ANGLE(135.0f) &&
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
