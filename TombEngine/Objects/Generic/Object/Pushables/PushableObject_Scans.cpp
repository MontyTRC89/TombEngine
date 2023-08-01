#include "framework.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Scans.h"

#include "Game/collision/collide_item.h"
#include "Game/Setup.h"
#include "Objects/Generic/Object/Pushables/PushableObject.h"
#include "Objects/Generic/Object/Pushables/PushableObject_BridgeCol.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Stack.h"

#include "Specific/level.h"

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
		if (collisionResult.Block->Stopper)
		{
			if (collisionResult.Position.Floor <= pushableItem.Pose.Position.y)
			{
				return false;
			}
		}

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
		int blockHeight = CalculateStackHeight(itemNumber);
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
		if (collisionResult.Position.Floor != LaraItem->Pose.Position.y)
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

		auto targetRoom = pushableItem.RoomNumber; //DOUBT, What will happen in the room portals?

		if (!IsPushableObjectMoveAllowed(itemNumber, targetPos, targetRoom))
			return false;

		//If player is pulling, is there space for Lara?
		if (hasPullAction && !IsValidForLara(itemNumber))
			return false;

		return true;
	}
}
