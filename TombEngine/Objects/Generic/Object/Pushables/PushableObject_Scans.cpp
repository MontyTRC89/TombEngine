#include "framework.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Scans.h"

#include "Game/collision/collide_item.h"
#include "Game/Setup.h"
#include "Objects/Generic/Object/Pushables/PushableObject.h"
#include "Objects/Generic/Object/Pushables/PushableObject_BridgeCol.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Stack.h"
#include "Specific/Input/Input.h"

#include "Specific/level.h"

using namespace TEN::Input;

namespace TEN::Entities::Generic
{
	constexpr auto PUSHABLE_HEIGHT_TOLERANCE = 32;

	bool IsPushableValid(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		if (pushableItem.Status == ITEM_INVISIBLE || pushableItem.TriggerFlags < 0) //It requires a positive OCB to can interact with it.
			return false;

		auto collisionResult = CollisionResult{};

		if (pushable.UsesRoomCollision)
		{
			DeactivateClimbablePushableCollider(itemNumber);
			collisionResult = GetCollision(&pushableItem);
			ActivateClimbablePushableCollider(itemNumber);
		}
		else
		{
			collisionResult = GetCollision(&pushableItem);
		}

		// Check is pushable is in a wall.
		if (collisionResult.Block->IsWall(pushableItem.Pose.Position.x, pushableItem.Pose.Position.z))
			return false;

		// Check if pushable isn't on the floor.
		int heightDifference = abs(collisionResult.Position.Floor - pushableItem.Pose.Position.y);
		if ((heightDifference >= PUSHABLE_HEIGHT_TOLERANCE))
			return false;

		//Check if is far from Lara (happened in the stacked pushables, that Lara was trying use the upper one but detects the lower one).
		int LaraHeightDifference = abs(LaraItem->Pose.Position.y - pushableItem.Pose.Position.y);
		if ((LaraHeightDifference >= PUSHABLE_HEIGHT_TOLERANCE))
			return false;

		return true;
	}

	bool IsPushableObjectMoveAllowed(int itemNumber, Vector3i targetPos, int targetRoom)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		auto collisionResult = GetCollision(targetPos.x, targetPos.y, targetPos.z, targetRoom);

		// Check for wall.
		if (collisionResult.Block->IsWall(targetPos.x, targetPos.z))
			return false;
		
		// Check for floor slope.
		if (collisionResult.Position.FloorSlope)
			return false;

		// Check for diagonal floor.
		if (collisionResult.Position.DiagonalStep)
			return false;

		if ((collisionResult.Block->GetSurfaceSlope(0, true) != Vector2::Zero) || (collisionResult.Block->GetSurfaceSlope(1, true) != Vector2::Zero))
			return false;

		// Check for stopper flag.
		/*if (collisionResult.Block->Stopper)
		{
			if (collisionResult.Position.Floor <= pushableItem.Pose.Position.y)
			{
				return false;
			}
		}*/

		// Check for gap or step. (Can it fall down?) (Only available for pushing).
		int floorDifference = abs(collisionResult.Position.Floor - pushableItem.Pose.Position.y);
		if (pushable.CanFall && pushable.StackUpperItem == NO_ITEM)
		{
			if ((collisionResult.Position.Floor < pushableItem.Pose.Position.y) && (floorDifference >= PUSHABLE_HEIGHT_TOLERANCE))
				return false;
		}
		else
		{
			if (floorDifference >= PUSHABLE_HEIGHT_TOLERANCE)
				return false;
		}

		// Is ceiling (square or diagonal) high enough?
		int distanceToCeiling = abs(collisionResult.Position.Ceiling - collisionResult.Position.Floor);
		int blockHeight = CalculateStackHeight(itemNumber) - PUSHABLE_HEIGHT_TOLERANCE;
		if (distanceToCeiling < blockHeight)
			return false;

		// Is there any enemy or object?
		auto prevPos = pushableItem.Pose.Position;
		pushableItem.Pose.Position = targetPos;
		GetCollidedObjects(&pushableItem, BLOCK(0.25f), true, &CollidedItems[0], &CollidedMeshes[0], true);
		pushableItem.Pose.Position = prevPos;

		if (CollidedMeshes[0])
			return false;

		for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
		{
			if (!CollidedItems[i])
				break;

			if (Objects[CollidedItems[i]->ObjectNumber].isPickup)
				continue;

			if (Objects[CollidedItems[i]->ObjectNumber].floor == nullptr) //??
				return false;

			auto& object = Objects[CollidedItems[i]->ObjectNumber];
			int collidedIndex = CollidedItems[i] - g_Level.Items.data(); // Index of CollidedItems[i].

			auto colPos = CollidedItems[i]->Pose.Position;

			// Check if floor function returns nullopt.
			if (object.floor(collidedIndex, colPos.x, colPos.y, colPos.z) == std::nullopt)
				return false;
		}
		return true;
	}

	bool IsValidForLara(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];

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

		auto laraDetectionPos = LaraItem->Pose.Position + playerOffset;
		auto collisionResult = GetCollision(laraDetectionPos.x, laraDetectionPos.y, laraDetectionPos.z, LaraItem->RoomNumber);

		//This collisionResult is the point where Lara would be at the end of the pushable pull.

		// If is a wall
		if (collisionResult.Block->IsWall(laraDetectionPos.x, laraDetectionPos.z))
			return false;

		// If floor is not flat
		int floorDifference = abs(collisionResult.Position.Floor - LaraItem->Pose.Position.y);
		if (floorDifference >= PUSHABLE_HEIGHT_TOLERANCE)
			return false;

		// Is ceiling (square or diagonal) high enough?
		int distanceToCeiling = abs(collisionResult.Position.Ceiling - collisionResult.Position.Floor);
		if (distanceToCeiling < LARA_HEIGHT)
			return false;

		// Is there any enemy or object?
		auto prevPos = LaraItem->Pose.Position;
		LaraItem->Pose.Position = laraDetectionPos;
		GetCollidedObjects(LaraItem, LARA_RADIUS, true, &CollidedItems[0], &CollidedMeshes[0], true);
		LaraItem->Pose.Position = prevPos;

		if (CollidedMeshes[0])
			return false;

		for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
		{
			if (!CollidedItems[i])
				break;

			if (CollidedItems[i] == &pushableItem) // If collided item is not pushblock in which lara embedded
				continue;

			if (Objects[CollidedItems[i]->ObjectNumber].isPickup) // If it isn't a picukp
				continue;

			if (Objects[CollidedItems[i]->ObjectNumber].floor == nullptr)
				return false;
			else
			{
				auto& object = Objects[CollidedItems[i]->ObjectNumber];
				int collidedIndex = CollidedItems[i] - g_Level.Items.data();
				int xCol = CollidedItems[i]->Pose.Position.x;
				int yCol = CollidedItems[i]->Pose.Position.y;
				int zCol = CollidedItems[i]->Pose.Position.z;

				if (object.floor(collidedIndex, xCol, yCol, zCol) == std::nullopt)
					return false;
			}
		}

		return true;
	}

	bool PushableIdleConditions(int itemNumber)
	{
		//If Lara is grabbing, check the push pull actions.
		if (LaraItem->Animation.ActiveState != LS_PUSHABLE_GRAB || 
			!TestLastFrame(LaraItem, LA_PUSHABLE_GRAB))
			return false;

		//First checks conditions.
		bool hasPushAction = IsHeld(In::Forward);
		bool hasPullAction = IsHeld(In::Back);

		//Cond 1: Is pressing Forward or Back?
		if (!hasPushAction && !hasPullAction)
			return false;

		//Cond 2: Can do the interaction with that side of the pushable?
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);
		auto& pushableSidesAttributes = pushable.SidesMap[quadrant]; //0 North, 1 East, 2 South or 3 West.

		if ((hasPushAction && !pushableSidesAttributes.Pushable) ||
			(hasPullAction && !pushableSidesAttributes.Pullable))
			return false;

		//Cond 3: Is its stacked pushables under the limit?
		if (!IsUnderStackLimit(itemNumber))
			return false;

		//Cond 4: Does it comply with the room collision conditions?.
		if (!PushableMovementConditions(itemNumber, hasPushAction, hasPullAction))
			return false;

		return true;
	}

	bool PushableMovementConditions(int itemNumber, bool hasPushAction, bool hasPullAction)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		//Is the pushable allowed in the target position?
		int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);
		auto targetPos = pushableItem.Pose.Position;

		int direction = (hasPushAction) ? 1 : -1;

		switch (quadrant)
		{
		case NORTH:
			targetPos.z = targetPos.z + direction * BLOCK(1);
			break;

		case EAST:
			targetPos.x = targetPos.x + direction * BLOCK(1);
			break;

		case SOUTH:
			targetPos.z = targetPos.z - direction * BLOCK(1);
			break;

		case WEST:
			targetPos.x = targetPos.x - direction * BLOCK(1);
			break;
		}

		auto targetRoom = pushableItem.RoomNumber;

		if (!IsPushableObjectMoveAllowed(itemNumber, targetPos, targetRoom))
			return false;

		//If player is pulling, is there space for Lara?
		if (hasPullAction && !IsValidForLara(itemNumber))
			return false;


		//Stopper flag
		//Put it in destiny
		SetPushableStopperFlag(true, targetPos, targetRoom);

		return true;
	}

	PushableEnvironemntState CheckPushableEnvironment(int itemNumber, int& floorHeight)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		PushableEnvironemntState result;

		DeactivateClimbablePushableCollider(itemNumber);
		auto pointColl = GetCollision(&pushableItem);
		ActivateClimbablePushableCollider(itemNumber);

		floorHeight = pointColl.Position.Floor; //Updates floorHeight reference for external use.
		
		if (TestEnvironment(ENV_FLAG_WATER, pushableItem.RoomNumber))
		{
			//Is in water, is it deep or shallow?
			int waterHeight = GetWaterHeight(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y, pushableItem.Pose.Position.z, pushableItem.RoomNumber);
			int distanceToSurface = abs(waterHeight - floorHeight);
			if (distanceToSurface > (GetPushableHeight(pushableItem) + 128))
			{
				result = PushableEnvironemntState::DeepWater;
			}
			else
			{
				result = PushableEnvironemntState::ShallowWater;
			}
			pushable.WaterSurfaceHeight = waterHeight;
		}
		else
		{
			//Is in dry, is it on ground or on air?
			if (floorHeight > (pushableItem.Pose.Position.y + pushableItem.Animation.Velocity.y) &&
				abs(pushableItem.Pose.Position.y - floorHeight) >= CLICK(1))
			{
				result = PushableEnvironemntState::Air;
			}
			else
			{
				if (pointColl.FloorTilt.x == 0 && pointColl.FloorTilt.y == 0)
				{
					//Is on a flat floor
					result = PushableEnvironemntState::Ground;
				}
				else
				{
					//Is on a slope floor
					result = PushableEnvironemntState::Slope;
				}
			}
			pushable.WaterSurfaceHeight = NO_HEIGHT;
		}
		return result;
	}

	void UpdateWaterHeight(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		pushable.WaterSurfaceHeight = 0;
	}
}
