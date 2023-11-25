#include "framework.h"
#include "Objects/Generic/Object/Pushable/PushableCollision.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/floordata.h"
#include "Game/Lara/lara.h"
#include "Game/Setup.h"
#include "Objects/Generic/Object/Pushable/PushableBridge.h"
#include "Objects/Generic/Object/Pushable/PushableObject.h"
#include "Objects/Generic/Object/Pushable/PushableStack.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Input;

namespace TEN::Entities::Generic
{
	constexpr auto PUSHABLE_FLOOR_HEIGHT_TOLERANCE = 32;

	bool IsPushableValid(ItemInfo& pushableItem)
	{
		auto& pushable = GetPushableInfo(pushableItem);

		// NOTE: Requires positive OCB value for interaction.
		if (pushableItem.Status == ITEM_INVISIBLE || pushableItem.TriggerFlags < 0)
			return false;

		auto pointColl = CollisionResult{};
		if (pushable.UseRoomCollision)
		{
			RemovePushableBridge(pushableItem);
			pointColl = GetCollision(&pushableItem);
			AddPushableBridge(pushableItem);
		}
		else
		{
			pointColl = GetCollision(&pushableItem);
		}

		// 1) Check for wall.
		if (pointColl.Block->IsWall(pushableItem.Pose.Position.x, pushableItem.Pose.Position.z))
			return false;

		// 2) Test height from floor.
		int heightFromFloor = abs(pointColl.Position.Floor - pushableItem.Pose.Position.y);
		if (heightFromFloor >= PUSHABLE_FLOOR_HEIGHT_TOLERANCE)
			return false;

		// 3) Test height from player. Prevents player from grabbing pushable at wrong elevation in stack.
		int heightFromPlayer = abs(LaraItem->Pose.Position.y - pushableItem.Pose.Position.y);
		if (heightFromPlayer >= PUSHABLE_FLOOR_HEIGHT_TOLERANCE)
			return false;

		return true;
	}

	bool IsPushableObjectMoveAllowed(ItemInfo& pushableItem, const Vector3i& targetPos, int targetRoomNumber)
	{
		auto& pushable = GetPushableInfo(pushableItem);

		pushable.IsOnEdge = false;

		auto pointColl = GetCollision(targetPos, targetRoomNumber);

		// 1) Check for wall.
		if (pointColl.Block->IsWall(targetPos.x, targetPos.z))
			return false;
		
		// 2) Check for gaps or steps.
		int heightFromFloor = abs(pointColl.Position.Floor - pushableItem.Pose.Position.y);
		if (heightFromFloor >= PUSHABLE_FLOOR_HEIGHT_TOLERANCE)
		{
			// Step.
			if (pointColl.Position.Floor < pushableItem.Pose.Position.y)
			{
				return false;
			}
			// Gap.
			else
			{
				pushable.IsOnEdge = true;
				if (!pushable.CanFall || pushable.Stack.ItemNumberAbove != NO_ITEM)
					return false;
			}
		}

		// 3) Check for slippery floor slope.
		if (pointColl.Position.FloorSlope)
			return false;

		// 4) Check for diagonal floor step.
		if (pointColl.Position.DiagonalStep)
			return false;

		// 5) Test floor slope. TODO: Check slope angles of normals directly.
		if ((pointColl.Block->GetSurfaceNormal(0, true) != -Vector3::UnitY) || (pointColl.Block->GetSurfaceNormal(1, true) != -Vector3::UnitY))
			return false;

		// Check for stopper flag.
		/*if (collisionResult.Block->Stopper)
		{
			if (collisionResult.Position.Floor <= pushableItem.Pose.Position.y)
				return false;
		}*/

		// Is ceiling (square or diagonal) high enough?
		int distFromCeil = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);
		int blockHeight = GetStackHeight(pushableItem.Index) - PUSHABLE_FLOOR_HEIGHT_TOLERANCE;
		if (distFromCeil < blockHeight)
			return false;

		// TODO: Don't modify item.
		// Test object collision.
		auto prevPos = pushableItem.Pose.Position;
		pushableItem.Pose.Position = targetPos;
		GetCollidedObjects(&pushableItem, BLOCK(0.25f), true, &CollidedItems[0], &CollidedMeshes[0], true);
		pushableItem.Pose.Position = prevPos;

		if (CollidedMeshes[0])
			return false;

		for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
		{
			if (CollidedItems[i] == nullptr)
				break;

			const auto& item = *CollidedItems[i];
			const auto& object = Objects[item.ObjectNumber];

			if (object.isPickup)
				continue;

			if (object.GetFloorHeight == nullptr)
				return false;

			if (!object.GetFloorHeight(item, item.Pose.Position).has_value())
				return false;
		}

		return true;
	}

	bool IsValidForPlayer(const ItemInfo& pushableItem)
	{
		auto playerOffset = Vector3i::Zero;
				
		int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);
		switch (quadrant)
		{
		case NORTH:
			playerOffset.z = GetBestFrame(*LaraItem).Offset.z - BLOCK(1);
			break;

		case EAST:
			playerOffset.x = GetBestFrame(*LaraItem).Offset.z - BLOCK(1);
			break;

		case SOUTH:
			playerOffset.z = -GetBestFrame(*LaraItem).Offset.z + BLOCK(1);
			break;

		case WEST:
			playerOffset.x = -GetBestFrame(*LaraItem).Offset.z + BLOCK(1);
			break;

		default:
			break;
		}

		// This collisionResult is the point where Lara would be at the end of the pushable pull.
		auto pointColl = GetCollision(LaraItem->Pose.Position + playerOffset, LaraItem->RoomNumber);

		// Test for wall.
		if (pointColl.Block->IsWall(pointColl.Coordinates.x, pointColl.Coordinates.z))
			return false;

		// Test for flat floor.
		int floorHeightDelta = abs(pointColl.Position.Floor - LaraItem->Pose.Position.y);
		if (floorHeightDelta >= PUSHABLE_FLOOR_HEIGHT_TOLERANCE)
			return false;

		// Test floor-to-ceiling height.
		int floorToCeilHeight = abs(pointColl.Position.Ceiling - pointColl.Position.Floor);
		if (floorToCeilHeight < LARA_HEIGHT)
			return false;

		// Collide with objects.
		auto prevPos = LaraItem->Pose.Position;
		LaraItem->Pose.Position = pointColl.Coordinates;
		GetCollidedObjects(LaraItem, LARA_RADIUS, true, &CollidedItems[0], &CollidedMeshes[0], true);
		LaraItem->Pose.Position = prevPos;

		if (CollidedMeshes[0])
			return false;

		for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
		{
			if (CollidedItems[i] == nullptr)
				break;

			const auto& item = *CollidedItems[i];
			const auto& object = Objects[item.ObjectNumber];

			if (&item == &pushableItem)
				continue;

			if (object.isPickup)
				continue;

			if (object.GetFloorHeight == nullptr)
			{
				return false;
			}
			else
			{
				if (!object.GetFloorHeight(item, item.Pose.Position).has_value())
					return false;
			}
		}

		return true;
	}

	bool PushableIdleConditions(ItemInfo& pushableItem)
	{
		auto& pushable = GetPushableInfo(pushableItem);

		// If player is grabbing, check push and pull actions.
		if (LaraItem->Animation.ActiveState != LS_PUSHABLE_GRAB || 
			!TestLastFrame(LaraItem, LA_PUSHABLE_GRAB))
		{
			return false;
		}

		bool hasPushAction = IsHeld(In::Forward);
		bool hasPullAction = IsHeld(In::Back);

		// Check for input actions.
		if (!hasPushAction && !hasPullAction)
			return false;

		int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);
		const auto& edgeAttribs = pushable.EdgeAttribs[quadrant];

		// Test edge attributes for quadrant.
		if ((hasPushAction && !edgeAttribs.IsPushable) ||
			(hasPullAction && !edgeAttribs.IsPullable))
		{
			return false;
		}

		// Test stack limit.
		if (!IsWithinStackLimit(pushableItem.Index))
			return false;

		// Test movement conditions.
		if (!TestPushableMovementConditions(pushableItem, hasPushAction, hasPullAction))
			return false;

		return true;
	}

	bool TestPushableMovementConditions(ItemInfo& pushableItem, bool hasPushAction, bool hasPullAction)
	{
		const auto& pushable = GetPushableInfo(pushableItem);

		int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);
		auto targetPos = pushableItem.Pose.Position;

		int dir = (hasPushAction) ? 1 : -1;

		switch (quadrant)
		{
		case NORTH:
			targetPos.z = targetPos.z + dir * BLOCK(1);
			break;

		case EAST:
			targetPos.x = targetPos.x + dir * BLOCK(1);
			break;

		case SOUTH:
			targetPos.z = targetPos.z - dir * BLOCK(1);
			break;

		case WEST:
			targetPos.x = targetPos.x - dir * BLOCK(1);
			break;
		}

		int targetRoomNumber = pushableItem.RoomNumber;

		if (!IsPushableObjectMoveAllowed(pushableItem, targetPos, targetRoomNumber))
			return false;

		if (hasPullAction && !IsValidForPlayer(pushableItem))
			return false;

		SetPushableStopperFlag(true, targetPos, targetRoomNumber);
		return true;
	}

	PushableCollisionData GetPushableCollision(ItemInfo& item)
	{
		auto& pushable = GetPushableInfo(item);

		auto pointColl = CollisionResult{};
		int waterHeight = NO_HEIGHT;

		if (pushable.UseBridgeCollision)
		{
			RemovePushableBridge(item);

			pointColl = GetCollision(item);
			waterHeight = GetWaterSurface(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, item.RoomNumber);

			AddPushableBridge(item);
		}
		else
		{
			pointColl = GetCollision(item);
			waterHeight = GetWaterSurface(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, item.RoomNumber);
		}

		auto pushableColl = PushableCollisionData{};
		pushableColl.FloorHeight = pointColl.Position.Floor;
		pushableColl.CeilingHeight = pointColl.Position.Ceiling;

		// Above water.
		if (TestEnvironment(ENV_FLAG_WATER, item.RoomNumber) ||
			TestEnvironment(ENV_FLAG_SWAMP, item.RoomNumber))
		{
			pushable.WaterSurfaceHeight = waterHeight;

			if (pushableColl.FloorHeight > (item.Pose.Position.y + item.Animation.Velocity.y) &&
				abs(item.Pose.Position.y - pushableColl.FloorHeight) >= PUSHABLE_FLOOR_HEIGHT_TOLERANCE)
			{
				pushableColl.EnvType = PushableEnvironmentType::Water;
			}
			else
			{
				pushableColl.EnvType = PushableEnvironmentType::WaterFloor;
			}
		}
		// Above dry ground.
		else
		{
			pushable.WaterSurfaceHeight = NO_HEIGHT;

			// Airborne.
			if (pushableColl.FloorHeight > (item.Pose.Position.y + item.Animation.Velocity.y) &&
				abs(item.Pose.Position.y - pushableColl.FloorHeight) >= PUSHABLE_FLOOR_HEIGHT_TOLERANCE)
			{
				pushableColl.EnvType = PushableEnvironmentType::Air;
			}
			// Grounded.
			else
			{
				auto floorSlopeAngle = Geometry::GetSurfaceSlopeAngle(pointColl.FloorNormal);

				if (floorSlopeAngle == 0)
				{
					pushableColl.EnvType = PushableEnvironmentType::FlatFloor;
				}
				else
				{
					pushableColl.EnvType = PushableEnvironmentType::SlopedFloor;
				}
			}
		}

		return pushableColl;
	}
}
